#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "config.h"
#include "motor_controller.h"

// =============================================================================
// WebServer — ESPAsyncWebServer REST API + SPIFFS dashboard
// =============================================================================

class WebServerController {
public:
    explicit WebServerController(MotorController& motor);

    // Call once after WiFi is connected
    void init();

    // Not needed for async server, but provided for symmetry
    void update();

private:
    AsyncWebServer _server;
    MotorController& _motor;

    // Route handlers
    void _setupRoutes();
};
