#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

// =============================================================================
// MotorController — Manages relays, ACS712 current sensing, and soft-start
// =============================================================================

class MotorController {
public:
    // Motor operating state
    enum class MotorState : uint8_t {
        STOPPED,
        SOFT_START_RAMP,
        RUNNING,
        OVERCURRENT_CUTOFF
    };

    MotorController();

    // Lifecycle
    void init();
    void update();   // Call every loop iteration

    // -------------------------------------------------------------------------
    // Current sensing
    // -------------------------------------------------------------------------
    float readCurrentRaw();        // Raw single ADC sample, in Amps
    float getCurrentFiltered();    // EMA-filtered current in Amps

    // -------------------------------------------------------------------------
    // Relay control
    // -------------------------------------------------------------------------
    void setRelayMotor(bool on);
    void setRelaySoftStart(bool on);
    bool getRelayMotorState() const    { return _relayMotorOn; }
    bool getRelaySoftStartState() const { return _relaySoftStartOn; }

    // -------------------------------------------------------------------------
    // Push button
    // -------------------------------------------------------------------------
    bool isPushButtonPressed();    // Returns true on debounced press

    // -------------------------------------------------------------------------
    // Soft start
    // -------------------------------------------------------------------------
    void startSoftStart(uint8_t durationSec);
    bool isSoftStartActive() const { return _state == MotorState::SOFT_START_RAMP; }
    uint8_t getSoftStartProgress() const;  // 0 – 100 %
    uint8_t getSoftStartDuration() const   { return _softStartDuration; }
    void setSoftStartDuration(uint8_t sec);

    // -------------------------------------------------------------------------
    // Motor stop
    // -------------------------------------------------------------------------
    void stopMotor();

    // -------------------------------------------------------------------------
    // Current threshold
    // -------------------------------------------------------------------------
    void  setCurrentThreshold(float amps);
    float getCurrentThreshold() const { return _currentThreshold; }

    // -------------------------------------------------------------------------
    // State
    // -------------------------------------------------------------------------
    MotorState getMotorState() const { return _state; }

    // -------------------------------------------------------------------------
    // Persistent settings (Preferences / NVS)
    // -------------------------------------------------------------------------
    void saveSettings();
    void loadSettings();

private:
    // State
    MotorState _state;
    bool       _relayMotorOn;
    bool       _relaySoftStartOn;

    // Current sensing
    float _currentFiltered;
    float _currentThreshold;

    // Soft start
    uint8_t  _softStartDuration;    // seconds
    uint32_t _softStartStartMs;     // millis() when soft start began

    // Button debounce
    bool     _lastButtonReading;
    uint32_t _lastDebounceMs;
    bool     _buttonState;

    // Update timers
    uint32_t _lastMonitorMs;

    // NVS
    Preferences _prefs;

    // Helpers
    void _handleSoftStartRamp();
    void _handleOvercurrentCheck();
};
