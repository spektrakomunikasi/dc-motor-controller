#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "config.h"
#include "motor_controller.h"

// =============================================================================
// WebServerController
// Serves a static HTML dashboard and exposes a JSON REST API.
// =============================================================================

class WebServerController {
public:
    // Takes references so it can read/write motor state
    WebServerController(MotorController& motor);

    // Set up routes and start listening on WEB_SERVER_PORT
    void begin();

    // Nothing to poll – AsyncWebServer is interrupt-driven
    // (kept for API symmetry)
    void update() {}

private:
    MotorController&  _motor;
    AsyncWebServer    _server;

    void setupRoutes();

    // Route handlers
    void handleGetStatus(AsyncWebServerRequest* request);
    void handleSetThreshold(AsyncWebServerRequest* request,
                            uint8_t* data, size_t len,
                            size_t index, size_t total);
    void handleRelayControl(AsyncWebServerRequest* request,
                            uint8_t* data, size_t len,
                            size_t index, size_t total);

    // Add CORS headers to every response
    static void addCorsHeaders(AsyncWebServerResponse* response);
};
