#include "motor_controller.h"

// =============================================================================
// MotorController Implementation
// =============================================================================

MotorController::MotorController()
    : _state(STOPPED)
    , _currentAmps(0.0f)
    , _currentLimitAmps(DEFAULT_CURRENT_LIMIT_A)
    , _softStartMs(DEFAULT_SOFT_START_MS)
    , _relay1On(false)
    , _relay2On(false)
    , _softStartBeginMs(0)
    , _lastCurrentReadMs(0)
    , _lastButtonMs(0)
    , _buttonWasPressed(false)
{}

void MotorController::init() {
    // Configure relay pins as outputs (active HIGH via transistor driver)
    pinMode(RELAY_CUTOFF_PIN,    OUTPUT);
    pinMode(RELAY_SOFTSTART_PIN, OUTPUT);
    digitalWrite(RELAY_CUTOFF_PIN,    LOW);
    digitalWrite(RELAY_SOFTSTART_PIN, LOW);
    _relay1On = false;
    _relay2On = false;

    // Configure push button as input with internal pull-up
    pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);

    // Configure ADC pin (GPIO34 is input-only)
    pinMode(ACS712_PIN, INPUT);
    analogReadResolution(ACS712_ADC_BITS);
    analogSetAttenuation(ADC_11db); // 0–3.3V range

    // Load persistent settings
    _loadSettings();

    Serial.println("[Motor] Initialized");
    Serial.printf("[Motor] Current limit: %.1f A, Soft-start: %u ms\n",
                  _currentLimitAmps, _softStartMs);
}

void MotorController::update() {
    uint32_t now = millis();

    // Read current sensor periodically
    if (now - _lastCurrentReadMs >= CURRENT_READ_INTERVAL_MS) {
        _lastCurrentReadMs = now;
        _readCurrent();
    }

    // Handle push button
    _handleButton();

    // State machine
    switch (_state) {
        case STOPPED:
        case FAULT:
            // Relays off
            _relay1On = false;
            _relay2On = false;
            break;

        case SOFT_STARTING: {
            // Relay 1 on, Relay 2 off during soft-start
            _relay1On = true;
            _relay2On = false;

            uint32_t elapsed = now - _softStartBeginMs;
            if (elapsed >= _softStartMs) {
                // Soft-start complete — bypass with Relay 2
                _state    = RUNNING;
                _relay2On = true;
                Serial.println("[Motor] Soft-start complete → RUNNING");
            }
            // Overcurrent check during ramp
            if (_currentAmps > _currentLimitAmps) {
                Serial.printf("[Motor] Overcurrent during soft-start: %.2f A > %.1f A\n",
                              _currentAmps, _currentLimitAmps);
                _state = OVERCURRENT;
            }
            break;
        }

        case RUNNING:
            _relay1On = true;
            _relay2On = true;
            // Overcurrent protection
            if (_currentAmps > _currentLimitAmps) {
                Serial.printf("[Motor] OVERCURRENT: %.2f A > %.1f A — CUTOFF\n",
                              _currentAmps, _currentLimitAmps);
                _state = OVERCURRENT;
            }
            break;

        case OVERCURRENT:
            // Cut all relays immediately
            _relay1On = false;
            _relay2On = false;
            break;
    }

    _updateRelays();
}

void MotorController::start() {
    if (_state == STOPPED || _state == OVERCURRENT || _state == FAULT) {
        _state            = SOFT_STARTING;
        _softStartBeginMs = millis();
        Serial.printf("[Motor] Starting (soft-start %u ms)\n", _softStartMs);
    }
}

void MotorController::stop() {
    _state = STOPPED;
    Serial.println("[Motor] Stopped");
}

void MotorController::reset() {
    stop();
    _state = STOPPED;
    Serial.println("[Motor] Reset");
}

String MotorController::getStateString() const {
    switch (_state) {
        case STOPPED:       return "STOPPED";
        case SOFT_STARTING: return "SOFT_START";
        case RUNNING:       return "RUNNING";
        case OVERCURRENT:   return "OVERCURRENT";
        case FAULT:         return "FAULT";
        default:            return "UNKNOWN";
    }
}

void MotorController::setCurrentLimit(float amps) {
    if (amps < MIN_CURRENT_LIMIT_A) amps = MIN_CURRENT_LIMIT_A;
    if (amps > MAX_CURRENT_LIMIT_A) amps = MAX_CURRENT_LIMIT_A;
    _currentLimitAmps = amps;
    _saveSettings();
    Serial.printf("[Motor] Current limit set: %.1f A\n", amps);
}

void MotorController::setSoftStartDuration(uint32_t ms) {
    if (ms < MIN_SOFT_START_MS) ms = MIN_SOFT_START_MS;
    if (ms > MAX_SOFT_START_MS) ms = MAX_SOFT_START_MS;
    _softStartMs = ms;
    _saveSettings();
    Serial.printf("[Motor] Soft-start duration set: %u ms\n", ms);
}

// -----------------------------------------------------------------------------
// Private helpers
// -----------------------------------------------------------------------------

void MotorController::_readCurrent() {
    long sum = 0;
    for (int i = 0; i < CURRENT_SAMPLE_COUNT; i++) {
        sum += analogRead(ACS712_PIN);
        // Brief delay for ADC input capacitor settling between samples
        delayMicroseconds(100);
    }
    int avg = (int)(sum / CURRENT_SAMPLE_COUNT);
    _currentAmps = _adcToCurrent(avg);
    if (_currentAmps < 0.0f) _currentAmps = 0.0f;
}

float MotorController::_adcToCurrent(int adcValue) const {
    // Convert ADC count to voltage (mV)
    float voltageMv = ((float)adcValue / ACS712_ADC_MAX) * ACS712_VREF_MV;
    // Zero-current voltage (midpoint)
    float zeroMv    = ((float)ACS712_ZERO_OFFSET / ACS712_ADC_MAX) * ACS712_VREF_MV;
    // Current = (V - Vzero) / sensitivity
    return (voltageMv - zeroMv) / ACS712_SENSITIVITY_MV_A;
}

void MotorController::_handleButton() {
    bool pressed = (digitalRead(PUSH_BUTTON_PIN) == LOW);
    uint32_t now = millis();

    if (pressed && !_buttonWasPressed && (now - _lastButtonMs > BUTTON_DEBOUNCE_MS)) {
        _lastButtonMs    = now;
        _buttonWasPressed = true;

        if (_state == STOPPED || _state == OVERCURRENT || _state == FAULT) {
            start();
        } else {
            stop();
        }
    }
    if (!pressed) {
        _buttonWasPressed = false;
    }
}

void MotorController::_updateRelays() {
    digitalWrite(RELAY_CUTOFF_PIN,    _relay1On ? HIGH : LOW);
    digitalWrite(RELAY_SOFTSTART_PIN, _relay2On ? HIGH : LOW);
}

void MotorController::_loadSettings() {
    _prefs.begin(NVS_NAMESPACE, true); // read-only
    _currentLimitAmps = _prefs.getFloat(PREF_KEY_CURRENT_LIMIT, DEFAULT_CURRENT_LIMIT_A);
    _softStartMs      = _prefs.getUInt(PREF_KEY_SOFT_START,    DEFAULT_SOFT_START_MS);
    _prefs.end();

    // Clamp to valid range
    if (_currentLimitAmps < MIN_CURRENT_LIMIT_A || _currentLimitAmps > MAX_CURRENT_LIMIT_A)
        _currentLimitAmps = DEFAULT_CURRENT_LIMIT_A;
    if (_softStartMs < MIN_SOFT_START_MS || _softStartMs > MAX_SOFT_START_MS)
        _softStartMs = DEFAULT_SOFT_START_MS;
}

void MotorController::_saveSettings() {
    _prefs.begin(NVS_NAMESPACE, false); // read-write
    _prefs.putFloat(PREF_KEY_CURRENT_LIMIT, _currentLimitAmps);
    _prefs.putUInt(PREF_KEY_SOFT_START,     _softStartMs);
    _prefs.end();
}
