#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

// =============================================================================
// MotorController
// Manages relay outputs, ACS712 current sensing, soft-start logic,
// overcurrent protection, and persistent settings (NVS).
// =============================================================================
class MotorController {
public:
    enum MotorState {
        STOPPED,
        SOFT_STARTING,
        RUNNING,
        OVERCURRENT,
        FAULT
    };

    MotorController();

    // Lifecycle
    void init();
    void update();

    // Motor control
    void start();
    void stop();
    void reset();

    // State accessors
    MotorState getState()        const { return _state; }
    bool       isRunning()       const { return _state == RUNNING || _state == SOFT_STARTING; }
    float      getCurrent()      const { return _currentAmps; }
    float      getCurrentLimit() const { return _currentLimitAmps; }
    uint32_t   getSoftStartMs()  const { return _softStartMs; }
    String     getStateString()  const;

    // Configuration (persisted to NVS)
    void  setCurrentLimit(float amps);
    void  setSoftStartDuration(uint32_t ms);

    // Relay state helpers (for display)
    bool isRelay1On() const { return _relay1On; }
    bool isRelay2On() const { return _relay2On; }

private:
    MotorState _state;
    float      _currentAmps;
    float      _currentLimitAmps;
    uint32_t   _softStartMs;

    bool     _relay1On;
    bool     _relay2On;

    uint32_t _softStartBeginMs;
    uint32_t _lastCurrentReadMs;
    uint32_t _lastButtonMs;
    bool     _buttonWasPressed;

    Preferences _prefs;

    void _readCurrent();
    void _handleButton();
    void _updateRelays();
    void _loadSettings();
    void _saveSettings();
    float _adcToCurrent(int adcValue) const;
};
