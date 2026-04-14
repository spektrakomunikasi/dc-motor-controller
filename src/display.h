#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "motor_controller.h"

// =============================================================================
// DisplayController — Drives the SSD1306 OLED 128×64 via I2C
// =============================================================================

class DisplayController {
public:
    DisplayController();

    // Lifecycle
    void init();

    // Update display — pass in current motor state data
    void update(float currentAmps,
                float limitAmps,
                MotorController::MotorState motorState,
                uint8_t softStartProgress,
                bool relay1On,
                bool relay2On,
                bool wifiConnected,
                int8_t rssi);

private:
    Adafruit_SSD1306 _display;
    bool             _blinkState;
    uint32_t         _lastBlinkMs;

    void _drawStatus(MotorController::MotorState state,
                     uint8_t progress,
                     bool blink);
};
