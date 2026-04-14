#pragma once

#include <Arduino.h>
#include "config.h"

// =============================================================================
// MotorController
// Reads current from ACS712-30A, controls relay, handles push button.
// =============================================================================

class MotorController {
public:
    MotorController();

    // Call once during setup()
    void begin();

    // Call every loop iteration
    void update();

    // ---- Accessors ----

    // Smoothed current reading in Amperes
    float getCurrent() const { return _currentA; }

    // true = relay energised (motor allowed to run)
    bool isRelayOn() const { return _relayOn; }

    // true = motor is considered running (relay on AND current > ~0.1 A)
    bool isMotorRunning() const { return _relayOn && (_currentA > 0.1f); }

    // Current over-current trip threshold in Amperes
    float getThreshold() const { return _thresholdA; }

    // Trip condition active (latched until button pressed or threshold raised)
    bool isFaulted() const { return _faulted; }

    // ---- Settings ----

    // Set a new threshold (clamped to [MIN_THRESHOLD_A, MAX_THRESHOLD_A])
    void setThreshold(float amperes);

    // Manually command relay ON (resets fault latch)
    void relayOn();

    // Manually command relay OFF
    void relayOff();

    // Save current threshold to EEPROM
    void saveSettings();

    // Load threshold from EEPROM (called from begin())
    void loadSettings();

private:
    // ADC / sensor
    float readRawCurrent();     // Returns instantaneous current (A)
    void  applyFilter(float raw);

    // Relay
    void  setRelay(bool on);

    // Button
    void  handleButton();

    // State
    float    _currentA;         // EMA-filtered current reading
    float    _thresholdA;       // Over-current trip threshold
    bool     _relayOn;          // Physical relay state
    bool     _faulted;          // Over-current latch flag

    // Button debounce state
    bool     _lastButtonState;
    bool     _buttonState;
    unsigned long _lastDebounceTime;

    // Timing
    unsigned long _lastSensorTime;
    unsigned long _lastRelayTime;
};
