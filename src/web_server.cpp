#include "web_server.h"
#include <SPIFFS.h>
#include <AsyncJson.h>
#include <esp_system.h>

// =============================================================================
// WebServerController Implementation
// =============================================================================

WebServerController::WebServerController(MotorController& motor)
    : _server(80)
    , _motor(motor)
{
}

void WebServerController::init()
{
    if (!SPIFFS.begin(true)) {
        Serial.println(F("[WEB] SPIFFS mount failed"));
    }

    _setupRoutes();
    _server.begin();
    Serial.println(F("[WEB] AsyncWebServer started on port 80"));
}

void WebServerController::update()
{
    // ESPAsyncWebServer handles requests asynchronously — nothing to do here
}

// -----------------------------------------------------------------------------
// Route setup
// -----------------------------------------------------------------------------
void WebServerController::_setupRoutes()
{
    // ----- Serve web dashboard from SPIFFS -----
    _server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    // ----- GET /api/status -----
    _server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* req) {
        JsonDocument doc;
        doc["current"]             = _motor.getCurrentFiltered();
        doc["threshold"]           = _motor.getCurrentThreshold();
        doc["relay1"]              = _motor.getRelayMotorState();
        doc["relay2"]              = _motor.getRelaySoftStartState();
        doc["soft_start_progress"] = _motor.getSoftStartProgress();
        doc["soft_start_time"]     = _motor.getSoftStartDuration();
        doc["uptime_ms"]           = millis();

        switch (_motor.getMotorState()) {
            case MotorController::MotorState::STOPPED:
                doc["motor_state"] = "STOPPED";  break;
            case MotorController::MotorState::SOFT_START_RAMP:
                doc["motor_state"] = "RAMP";     break;
            case MotorController::MotorState::RUNNING:
                doc["motor_state"] = "RUNNING";  break;
            case MotorController::MotorState::OVERCURRENT_CUTOFF:
                doc["motor_state"] = "OVERCURRENT"; break;
            default:
                doc["motor_state"] = "UNKNOWN";  break;
        }

        String out;
        serializeJson(doc, out);
        req->send(200, "application/json", out);
    });

    // ----- POST /api/threshold -----
    AsyncCallbackJsonWebHandler* threshHandler =
        new AsyncCallbackJsonWebHandler("/api/threshold",
            [this](AsyncWebServerRequest* req, JsonVariant& json) {
                if (!json.is<JsonObject>()) {
                    req->send(400, "application/json", "{\"error\":\"invalid JSON\"}");
                    return;
                }
                JsonObject body = json.as<JsonObject>();
                if (!body["value"].is<float>()) {
                    req->send(400, "application/json", "{\"error\":\"missing value\"}");
                    return;
                }
                float val = body["value"].as<float>();
                _motor.setCurrentThreshold(val);
                _motor.saveSettings();

                JsonDocument resp;
                resp["success"]   = true;
                resp["threshold"] = _motor.getCurrentThreshold();
                String out;
                serializeJson(resp, out);
                req->send(200, "application/json", out);
            });
    _server.addHandler(threshHandler);

    // ----- POST /api/soft-start-time -----
    AsyncCallbackJsonWebHandler* ssHandler =
        new AsyncCallbackJsonWebHandler("/api/soft-start-time",
            [this](AsyncWebServerRequest* req, JsonVariant& json) {
                if (!json.is<JsonObject>()) {
                    req->send(400, "application/json", "{\"error\":\"invalid JSON\"}");
                    return;
                }
                JsonObject body = json.as<JsonObject>();
                if (!body["value"].is<int>()) {
                    req->send(400, "application/json", "{\"error\":\"missing value\"}");
                    return;
                }
                uint8_t val = (uint8_t)body["value"].as<int>();
                _motor.setSoftStartDuration(val);
                _motor.saveSettings();

                JsonDocument resp;
                resp["success"]        = true;
                resp["soft_start_time"] = _motor.getSoftStartDuration();
                String out;
                serializeJson(resp, out);
                req->send(200, "application/json", out);
            });
    _server.addHandler(ssHandler);

    // ----- POST /api/relay/motor -----
    AsyncCallbackJsonWebHandler* relayHandler =
        new AsyncCallbackJsonWebHandler("/api/relay/motor",
            [this](AsyncWebServerRequest* req, JsonVariant& json) {
                if (!json.is<JsonObject>()) {
                    req->send(400, "application/json", "{\"error\":\"invalid JSON\"}");
                    return;
                }
                JsonObject body = json.as<JsonObject>();
                if (!body["state"].is<bool>()) {
                    req->send(400, "application/json", "{\"error\":\"missing state\"}");
                    return;
                }
                bool state = body["state"].as<bool>();
                if (state) {
                    _motor.startSoftStart(_motor.getSoftStartDuration());
                } else {
                    _motor.stopMotor();
                }

                JsonDocument resp;
                resp["success"] = true;
                resp["state"]   = state;
                String out;
                serializeJson(resp, out);
                req->send(200, "application/json", out);
            });
    _server.addHandler(relayHandler);

    // ----- POST /api/restart -----
    _server.on("/api/restart", HTTP_POST, [](AsyncWebServerRequest* req) {
        JsonDocument resp;
        resp["success"] = true;
        String out;
        serializeJson(resp, out);
        // Send response before restarting so the client receives it
        AsyncWebServerResponse* response = req->beginResponse(200, "application/json", out);
        response->addHeader("Connection", "close");
        req->send(response);
        // Restart after the current event-loop iteration finishes
        esp_restart();
    });

    // ----- 404 handler -----
    _server.onNotFound([](AsyncWebServerRequest* req) {
        req->send(404, "application/json", "{\"error\":\"not found\"}");
    });
}
