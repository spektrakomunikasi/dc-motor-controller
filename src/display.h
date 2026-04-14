#pragma once

#include <Arduino.h>
#include "config.h"

// =============================================================================
// DisplayController
// Supports OLED SSD1306 128x64 I2C and LCD 1602 I2C (runtime selectable via
// DISPLAY_TYPE in config.h).
// =============================================================================

class DisplayController {
public:
    DisplayController();

    // Initialise display hardware; returns true on success
    bool begin();

    // Refresh display with latest values
    void update(float currentA,
                bool  relayOn,
                bool  motorRunning,
                float thresholdA,
                int   rssi);

    // Show a temporary splash/boot screen
    void showSplash();

    // Show a brief message (used for fault indication)
    void showMessage(const char* line1, const char* line2 = nullptr);

private:
    bool _initialized;

    // OLED helpers
    void updateOLED(float currentA, bool relayOn, bool motorRunning,
                    float thresholdA, int rssi);
    // LCD helpers
    void updateLCD(float currentA, bool relayOn, bool motorRunning,
                   float thresholdA, int rssi);
};
