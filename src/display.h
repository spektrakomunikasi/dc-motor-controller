#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "wifi_manager.h"
#include "motor_controller.h"
#include "config.h"

// =============================================================================
// DisplayController
// Updates 128×64 OLED display with motor status and WiFi info.
// =============================================================================
class DisplayController {
public:
    DisplayController(WiFiManager& wifiMgr, MotorController& motor);

    void init();
    void update();

private:
    Adafruit_SSD1306 _display;
    WiFiManager&     _wifiMgr;
    MotorController& _motor;
    uint32_t         _lastUpdateMs;

    void _drawAPMode();
    void _drawConnecting();
    void _drawConnected();
    void _drawDisconnected();
};
