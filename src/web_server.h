#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "wifi_manager.h"
#include "motor_controller.h"

// =============================================================================
// WebServerController
// Normal-mode web server — served when the device is connected to WiFi.
// Provides REST API for motor control and serves the dashboard HTML.
// =============================================================================
class WebServerController {
public:
    explicit WebServerController(WiFiManager& wifiMgr, MotorController& motor);

    void init();
    void update();

private:
    AsyncWebServer   _server;
    WiFiManager&     _wifiMgr;
    MotorController& _motor;

    void _setupRoutes();
};
