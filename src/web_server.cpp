#include "web_server.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>

// =============================================================================
// WebServerController Implementation
// =============================================================================

WebServerController::WebServerController(WiFiManager& wifiMgr, MotorController& motor)
    : _server(WEB_SERVER_PORT)
    , _wifiMgr(wifiMgr)
    , _motor(motor)
{}

void WebServerController::init() {
    _setupRoutes();
    _server.begin();
    Serial.println("[WebServer] Started on port 80");
}

void WebServerController::update() {
    // AsyncWebServer is interrupt-driven; nothing to poll
}

// -----------------------------------------------------------------------------
// Routes
// -----------------------------------------------------------------------------

void WebServerController::_setupRoutes() {
    // Root: redirect to motor dashboard or setup based on WiFi state
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest* req) {
        if (_wifiMgr.isAPMode()) {
            req->redirect("/setup.html");
        } else {
            req->redirect("/index.html");
        }
    });

    // Serve static files from SPIFFS
    _server.on("/index.html", HTTP_GET, [this](AsyncWebServerRequest* req) {
        if (_wifiMgr.isAPMode()) {
            req->redirect("/setup.html");
            return;
        }
        if (SPIFFS.exists("/index.html")) {
            req->send(SPIFFS, "/index.html", "text/html");
        } else {
            req->send(404, "text/plain", "index.html not found — upload SPIFFS data");
        }
    });

    _server.on("/setup.html", HTTP_GET, [](AsyncWebServerRequest* req) {
        if (SPIFFS.exists("/setup.html")) {
            req->send(SPIFFS, "/setup.html", "text/html");
        } else {
            req->send(404, "text/plain", "setup.html not found — upload SPIFFS data");
        }
    });

    // ------------------------------------------------------------------
    // Motor Control API
    // ------------------------------------------------------------------

    // GET /api/status — combined motor + WiFi status
    _server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* req) {
        JsonDocument doc;
        doc["motor_state"]    = _motor.getStateString();
        doc["current_a"]      = _motor.getCurrent();
        doc["current_limit"]  = _motor.getCurrentLimit();
        doc["soft_start_ms"]  = _motor.getSoftStartMs();
        doc["relay1"]         = _motor.isRelay1On();
        doc["relay2"]         = _motor.isRelay2On();
        doc["wifi_state"]     = _wifiMgr.isConnected() ? "CONNECTED" : "AP_MODE";
        doc["wifi_ssid"]      = _wifiMgr.getSSID();
        doc["wifi_ip"]        = _wifiMgr.getIP();
        doc["wifi_rssi"]      = _wifiMgr.getRSSI();
        doc["uptime_ms"]      = millis();

        String body;
        serializeJson(doc, body);
        req->send(200, "application/json", body);
    });

    // POST /api/motor/start
    _server.on("/api/motor/start", HTTP_POST, [this](AsyncWebServerRequest* req) {
        _motor.start();
        req->send(200, "application/json", "{\"success\":true,\"state\":\"SOFT_START\"}");
    });

    // POST /api/motor/stop
    _server.on("/api/motor/stop", HTTP_POST, [this](AsyncWebServerRequest* req) {
        _motor.stop();
        req->send(200, "application/json", "{\"success\":true,\"state\":\"STOPPED\"}");
    });

    // POST /api/motor/reset
    _server.on("/api/motor/reset", HTTP_POST, [this](AsyncWebServerRequest* req) {
        _motor.reset();
        req->send(200, "application/json", "{\"success\":true,\"state\":\"STOPPED\"}");
    });

    // POST /api/motor/settings  { "current_limit": 15.0, "soft_start_ms": 3000 }
    _server.on("/api/motor/settings", HTTP_POST,
        [this](AsyncWebServerRequest* req) {
            if (req->_tempObject) {
                String* resp = reinterpret_cast<String*>(req->_tempObject);
                int code = resp->startsWith("{\"success\":true") ? 200 : 400;
                req->send(code, "application/json", *resp);
                delete resp;
                req->_tempObject = nullptr;
            } else {
                req->send(400, "application/json", "{\"success\":false,\"message\":\"Empty body\"}");
            }
        },
        nullptr,
        [this](AsyncWebServerRequest* req, uint8_t* data, size_t len,
               size_t index, size_t total) {
            if (index == 0) {
                req->_tempObject = new String();
            }
            if (index + len == total) {
                String body((char*)data, len);
                JsonDocument doc;
                String* resp = reinterpret_cast<String*>(req->_tempObject);
                if (deserializeJson(doc, body)) {
                    *resp = "{\"success\":false,\"message\":\"Invalid JSON\"}";
                    return;
                }

                if (doc["current_limit"].is<float>()) {
                    _motor.setCurrentLimit(doc["current_limit"].as<float>());
                }
                if (doc["soft_start_ms"].is<uint32_t>()) {
                    _motor.setSoftStartDuration(doc["soft_start_ms"].as<uint32_t>());
                }

                *resp = "{\"success\":true}";
            }
        }
    );

    // GET /api/setup/status (forwarded for convenience from normal mode)
    _server.on("/api/setup/status", HTTP_GET, [this](AsyncWebServerRequest* req) {
        JsonDocument doc;
        doc["state"]     = _wifiMgr.isConnected() ? "CONNECTED" : "AP_MODE";
        doc["ssid"]      = _wifiMgr.getSSID();
        doc["ip"]        = _wifiMgr.getIP();
        doc["ap_name"]   = _wifiMgr.getAPName();
        doc["uptime_ms"] = millis();

        String body;
        serializeJson(doc, body);
        req->send(200, "application/json", body);
    });

    // POST /api/setup/clear — clear WiFi credentials and reboot into AP mode
    _server.on("/api/setup/clear", HTTP_POST, [this](AsyncWebServerRequest* req) {
        _wifiMgr.clearWiFiCredentials();
        req->send(200, "application/json", "{\"success\":true,\"message\":\"Cleared. Rebooting...\"}");
        _wifiMgr.scheduleReboot();
    });

    // 404 fallback
    _server.onNotFound([](AsyncWebServerRequest* req) {
        req->send(404, "text/plain", "Not found");
    });
}
