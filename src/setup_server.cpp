#include "setup_server.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>

// =============================================================================
// SetupWebServer Implementation
// =============================================================================

SetupWebServer::SetupWebServer(WiFiManager& wifiMgr, MotorController& motor)
    : _server(WEB_SERVER_PORT)
    , _wifiMgr(wifiMgr)
    , _motor(motor)
{}

void SetupWebServer::init() {
    _setupRoutes();
    _server.begin();
    Serial.println("[SetupServer] Started on port 80");
}

void SetupWebServer::update() {
    // AsyncWebServer is interrupt-driven; nothing to poll
}

// -----------------------------------------------------------------------------
// Routes
// -----------------------------------------------------------------------------

void SetupWebServer::_setupRoutes() {
    // Serve setup dashboard HTML
    _server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->redirect("/setup.html");
    });

    _server.on("/setup.html", HTTP_GET, [](AsyncWebServerRequest* req) {
        if (SPIFFS.exists("/setup.html")) {
            req->send(SPIFFS, "/setup.html", "text/html");
        } else {
            req->send(404, "text/plain", "setup.html not found — upload SPIFFS data");
        }
    });

    // GET /api/setup/status
    _server.on("/api/setup/status", HTTP_GET, [this](AsyncWebServerRequest* req) {
        JsonDocument doc;

        const char* stateStr = "UNKNOWN";
        switch (_wifiMgr.getState()) {
            case WiFiManager::AP_MODE:     stateStr = "AP_MODE";     break;
            case WiFiManager::CONNECTING:  stateStr = "CONNECTING";  break;
            case WiFiManager::CONNECTED:   stateStr = "CONNECTED";   break;
            case WiFiManager::DISCONNECTED:stateStr = "DISCONNECTED";break;
            default:                       stateStr = "INITIALIZING";break;
        }

        doc["state"]     = stateStr;
        doc["ssid"]      = _wifiMgr.getSSID();
        doc["ip"]        = _wifiMgr.getIP();
        doc["ap_name"]   = _wifiMgr.getAPName();
        doc["uptime_ms"] = millis();

        String body;
        serializeJson(doc, body);
        req->send(200, "application/json", body);
    });

    // GET /api/setup/networks
    _server.on("/api/setup/networks", HTTP_GET, [this](AsyncWebServerRequest* req) {
        JsonDocument doc;
        JsonArray arr = doc["networks"].to<JsonArray>();

        for (const String& net : _wifiMgr.getScannedNetworks()) {
            arr.add(net);
        }

        String body;
        serializeJson(doc, body);
        req->send(200, "application/json", body);
    });

    // POST /api/setup/wifi — save credentials and schedule reboot
    // Body handler accumulates JSON; request handler sends the response.
    _server.on("/api/setup/wifi", HTTP_POST,
        [this](AsyncWebServerRequest* req) {
            // _tempObject holds the pre-built response string from the body handler
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
        nullptr, // upload handler (not used)
        [this](AsyncWebServerRequest* req, uint8_t* data, size_t len,
               size_t index, size_t total) {
            // Accumulate body across chunks, then parse on last chunk
            if (index == 0) {
                // First chunk — allocate response string (will be filled on last chunk)
                req->_tempObject = new String();
            }

            if (index + len == total) {
                // Last chunk — parse JSON body
                String body((char*)data, len);
                JsonDocument doc;
                DeserializationError err = deserializeJson(doc, body);

                String* resp = reinterpret_cast<String*>(req->_tempObject);
                if (err) {
                    *resp = "{\"success\":false,\"message\":\"Invalid JSON\"}";
                    return;
                }

                String ssid     = doc["ssid"]     | "";
                String password = doc["password"] | "";

                if (ssid.isEmpty()) {
                    *resp = "{\"success\":false,\"message\":\"SSID required\"}";
                    return;
                }

                _wifiMgr.setWiFiCredentials(ssid, password);
                _wifiMgr.scheduleReboot();
                *resp = "{\"success\":true,\"message\":\"Saved. Rebooting...\"}";
            }
        }
    );

    // POST /api/setup/reboot
    _server.on("/api/setup/reboot", HTTP_POST, [this](AsyncWebServerRequest* req) {
        req->send(200, "application/json", "{\"success\":true}");
        _wifiMgr.scheduleReboot();
    });

    // POST /api/setup/clear — clear credentials and reboot into AP mode
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
