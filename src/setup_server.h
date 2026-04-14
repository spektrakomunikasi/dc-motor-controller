#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "wifi_manager.h"
#include "motor_controller.h"

// =============================================================================
// SetupWebServer
// Served only when the device is in AP mode.
// Provides endpoints to scan networks, save credentials, and reboot.
// =============================================================================
class SetupWebServer {
public:
    explicit SetupWebServer(WiFiManager& wifiMgr, MotorController& motor);

    void init();
    void update();

private:
    AsyncWebServer  _server;
    WiFiManager&    _wifiMgr;
    MotorController& _motor;

    void _setupRoutes();
};
