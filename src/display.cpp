#include "display.h"

// =============================================================================
// DisplayController Implementation
// =============================================================================

DisplayController::DisplayController()
    : _display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET_PIN)
    , _blinkState(false)
    , _lastBlinkMs(0)
{
}

void DisplayController::init()
{
    Wire.begin(I2C_SDA, I2C_SCL);

    if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println(F("[OLED] SSD1306 init failed — check wiring"));
        return;
    }

    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);
    _display.setTextSize(1);

    // Splash screen
    _display.setCursor(10, 20);
    _display.println(F("MOTOR CTRL v1.0"));
    _display.setCursor(20, 36);
    _display.println(F("Initializing..."));
    _display.display();
    delay(1500);
    _display.clearDisplay();
    _display.display();
}

void DisplayController::update(float currentAmps,
                                float limitAmps,
                                MotorController::MotorState motorState,
                                uint8_t softStartProgress,
                                bool relay1On,
                                bool relay2On,
                                bool wifiConnected,
                                int8_t rssi)
{
    // Blink timer for OVERCURRENT state
    uint32_t now = millis();
    if (now - _lastBlinkMs >= 400) {
        _blinkState  = !_blinkState;
        _lastBlinkMs = now;
    }

    _display.clearDisplay();

    // ---------- Header ----------
    _display.setTextSize(1);
    _display.setCursor(0, 0);
    _display.print(F("MOTOR CONTROL v1.0"));
    // Horizontal rule (simulated with dashes)
    _display.setCursor(0, 9);
    _display.print(F("--------------------"));

    // ---------- Current ----------
    _display.setCursor(0, 18);
    _display.print(F("Current:"));
    _display.setCursor(54, 18);
    _display.print(currentAmps, 2);
    _display.print(F(" A"));

    // ---------- Limit ----------
    _display.setCursor(0, 27);
    _display.print(F("Limit:  "));
    _display.setCursor(54, 27);
    _display.print(limitAmps, 1);
    _display.print(F(" A"));

    // ---------- Motor state ----------
    _display.setCursor(0, 36);
    _display.print(F("Motor:  "));
    _drawStatus(motorState, softStartProgress, _blinkState);

    // ---------- Relay states ----------
    _display.setCursor(0, 45);
    _display.print(F("Relay1:"));
    _display.setCursor(48, 45);
    _display.print(relay1On ? F("[ON] ") : F("[OFF]"));

    _display.setCursor(72, 45);
    _display.print(F("R2:"));
    _display.setCursor(90, 45);
    _display.print(relay2On ? F("[ON] ") : F("[OFF]"));

    // ---------- WiFi status ----------
    _display.setCursor(0, 54);
    _display.print(F("WiFi:"));
    if (!wifiConnected) {
        _display.setCursor(36, 54);
        _display.print(F("Connecting..."));
    } else {
        // Draw signal-strength bars (5 bars, rssi -100 to -50 dBm typical)
        int8_t bars = 0;
        if (rssi >= -55)      bars = 5;
        else if (rssi >= -65) bars = 4;
        else if (rssi >= -70) bars = 3;
        else if (rssi >= -78) bars = 2;
        else if (rssi >= -87) bars = 1;

        _display.setCursor(36, 54);
        for (int8_t i = 0; i < 5; i++) {
            _display.print(i < bars ? F("*") : F("."));
        }
        // Show IP hint
        _display.setCursor(66, 54);
        _display.print(F(" OK"));
    }

    _display.display();
}

void DisplayController::_drawStatus(MotorController::MotorState state,
                                    uint8_t progress,
                                    bool blink)
{
    _display.setCursor(54, 36);
    switch (state) {
        case MotorController::MotorState::STOPPED:
            _display.print(F("STOPPED  "));
            break;

        case MotorController::MotorState::SOFT_START_RAMP:
            _display.print(F("RAMP "));
            _display.print(progress);
            _display.print(F("% "));
            break;

        case MotorController::MotorState::RUNNING:
            _display.print(F("RUNNING  "));
            break;

        case MotorController::MotorState::OVERCURRENT_CUTOFF:
            if (blink) {
                _display.print(F("OVERCURRENT"));
            } else {
                _display.print(F("!!!!!!!!!"));
            }
            break;
    }
}
