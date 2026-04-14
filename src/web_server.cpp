#include "web_server.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <SPIFFS.h>

// =============================================================================
// WebServerController implementation
// =============================================================================

WebServerController::WebServerController(MotorController& motor)
    : _motor(motor)
    , _server(WEB_SERVER_PORT)
{
}

// ----------------------------------------------------------------------------
void WebServerController::begin()
{
    setupRoutes();
    _server.begin();
    Serial.printf("[WEB] Server started on port %d\n", WEB_SERVER_PORT);
}

// ----------------------------------------------------------------------------
void WebServerController::addCorsHeaders(AsyncWebServerResponse* response)
{
    response->addHeader("Access-Control-Allow-Origin",  "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
}

// ----------------------------------------------------------------------------
void WebServerController::setupRoutes()
{
    // ---- Serve dashboard from SPIFFS (/data/index.html) ----
    _server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    // ---- OPTIONS pre-flight for CORS ----
    _server.on("/*", HTTP_OPTIONS, [](AsyncWebServerRequest* request) {
        AsyncWebServerResponse* resp =
            request->beginResponse(204, "text/plain", "");
        addCorsHeaders(resp);
        request->send(resp);
    });

    // ---- GET /api/status ----
    _server.on("/api/status", HTTP_GET,
        [this](AsyncWebServerRequest* request) {
            handleGetStatus(request);
        });

    // ---- POST /api/threshold ----
    _server.on("/api/threshold", HTTP_POST,
        // onRequest (headers only)
        [](AsyncWebServerRequest* request) {},
        // onUpload
        nullptr,
        // onBody
        [this](AsyncWebServerRequest* request,
               uint8_t* data, size_t len,
               size_t index, size_t total) {
            handleSetThreshold(request, data, len, index, total);
        });

    // ---- POST /api/relay ----
    _server.on("/api/relay", HTTP_POST,
        [](AsyncWebServerRequest* request) {},
        nullptr,
        [this](AsyncWebServerRequest* request,
               uint8_t* data, size_t len,
               size_t index, size_t total) {
            handleRelayControl(request, data, len, index, total);
        });

    // ---- 404 fallback ----
    _server.onNotFound([](AsyncWebServerRequest* request) {
        request->send(404, "application/json",
                      "{\"error\":\"Not found\"}");
    });
}

// ----------------------------------------------------------------------------
// GET /api/status
// Response JSON:
// {
//   "current":   12.34,
//   "relay":     true,
//   "running":   true,
//   "threshold": 15.0,
//   "faulted":   false,
//   "rssi":      -65,
//   "ip":        "192.168.1.100"
// }
// ----------------------------------------------------------------------------
void WebServerController::handleGetStatus(AsyncWebServerRequest* request)
{
    JsonDocument doc;
    doc["current"]   = static_cast<int>(_motor.getCurrent() * 100) / 100.0;
    doc["relay"]     = _motor.isRelayOn();
    doc["running"]   = _motor.isMotorRunning();
    doc["threshold"] = _motor.getThreshold();
    doc["faulted"]   = _motor.isFaulted();
    doc["rssi"]      = WiFi.RSSI();
    doc["ip"]        = WiFi.localIP().toString();

    String body;
    serializeJson(doc, body);

    AsyncWebServerResponse* resp =
        request->beginResponse(200, "application/json", body);
    addCorsHeaders(resp);
    request->send(resp);
}

// ----------------------------------------------------------------------------
// POST /api/threshold
// Body JSON: { "threshold": 12.5 }
// Response:  { "success": true, "threshold": 12.5 }
// ----------------------------------------------------------------------------
void WebServerController::handleSetThreshold(AsyncWebServerRequest* request,
                                              uint8_t* data, size_t len,
                                              size_t /*index*/, size_t /*total*/)
{
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data, len);

    if (err || !doc["threshold"].is<float>()) {
        AsyncWebServerResponse* resp =
            request->beginResponse(400, "application/json",
                                   "{\"error\":\"Invalid JSON or missing threshold\"}");
        addCorsHeaders(resp);
        request->send(resp);
        return;
    }

    float newThreshold = doc["threshold"].as<float>();
    _motor.setThreshold(newThreshold);
    _motor.saveSettings();

    JsonDocument out;
    out["success"]   = true;
    out["threshold"] = _motor.getThreshold();

    String body;
    serializeJson(out, body);

    AsyncWebServerResponse* resp =
        request->beginResponse(200, "application/json", body);
    addCorsHeaders(resp);
    request->send(resp);
}

// ----------------------------------------------------------------------------
// POST /api/relay
// Body JSON: { "relay": true }   → turn relay ON  (resets fault)
//            { "relay": false }  → turn relay OFF
// Response:  { "success": true, "relay": true }
// ----------------------------------------------------------------------------
void WebServerController::handleRelayControl(AsyncWebServerRequest* request,
                                              uint8_t* data, size_t len,
                                              size_t /*index*/, size_t /*total*/)
{
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data, len);

    if (err || !doc["relay"].is<bool>()) {
        AsyncWebServerResponse* resp =
            request->beginResponse(400, "application/json",
                                   "{\"error\":\"Invalid JSON or missing relay field\"}");
        addCorsHeaders(resp);
        request->send(resp);
        return;
    }

    bool wantOn = doc["relay"].as<bool>();
    if (wantOn) {
        _motor.relayOn();
    } else {
        _motor.relayOff();
    }

    JsonDocument out;
    out["success"] = true;
    out["relay"]   = _motor.isRelayOn();

    String body;
    serializeJson(out, body);

    AsyncWebServerResponse* resp =
        request->beginResponse(200, "application/json", body);
    addCorsHeaders(resp);
    request->send(resp);
}
