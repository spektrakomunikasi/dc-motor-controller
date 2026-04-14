#include "motor_controller.h"

// =============================================================================
// MotorController Implementation
// =============================================================================

MotorController::MotorController()
    : _state(MotorState::STOPPED)
    , _relayMotorOn(false)
    , _relaySoftStartOn(false)
    , _currentFiltered(0.0f)
    , _currentThreshold(CURRENT_DEFAULT)
    , _softStartDuration(SOFT_START_DEFAULT_TIME)
    , _softStartStartMs(0)
    , _lastButtonReading(HIGH)
    , _lastDebounceMs(0)
    , _buttonState(HIGH)
    , _lastMonitorMs(0)
{
}

void MotorController::init()
{
    // Configure relay output pins (Active HIGH via NPN transistor)
    pinMode(RELAY_CUTOFF_PIN,    OUTPUT);
    pinMode(RELAY_SOFTSTART_PIN, OUTPUT);
    digitalWrite(RELAY_CUTOFF_PIN,    LOW);
    digitalWrite(RELAY_SOFTSTART_PIN, LOW);

    // Configure push button with internal pull-up
    pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);

    // Configure ADC for ACS712 (GPIO 34 is input-only, no pull config needed)
    // ADC_11db: 0 – 3.3 V full-scale range, 12-bit resolution
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    // Pre-fill EMA filter with current reading
    _currentFiltered = readCurrentRaw();

    // Load saved settings
    loadSettings();
}

// -----------------------------------------------------------------------------
// Main update — call every loop iteration
// -----------------------------------------------------------------------------
void MotorController::update()
{
    uint32_t now = millis();

    if (now - _lastMonitorMs >= MONITOR_UPDATE_MS) {
        _lastMonitorMs = now;

        // Update EMA filter
        float raw = readCurrentRaw();
        _currentFiltered = CURRENT_EMA_ALPHA * raw
                         + (1.0f - CURRENT_EMA_ALPHA) * _currentFiltered;

        // Drive state machine
        switch (_state) {
            case MotorState::SOFT_START_RAMP:
                _handleSoftStartRamp();
                break;

            case MotorState::RUNNING:
                _handleOvercurrentCheck();
                break;

            default:
                break;
        }
    }
}

// -----------------------------------------------------------------------------
// Current sensing
// -----------------------------------------------------------------------------
float MotorController::readCurrentRaw()
{
    // Average 16 samples for noise reduction
    uint32_t sum = 0;
    for (uint8_t i = 0; i < 16; i++) {
        sum += analogRead(ACS712_PIN);
    }
    float adcVal = sum / 16.0f;

    // Convert ADC reading to voltage at the GPIO 34 pin (mV).
    // With ADC_11db attenuation, full scale = 3.3 V (0–4095 → 0–3300 mV).
    //
    // DIRECT CONNECTION (default — no voltage divider):
    //   ACS712 VOUT → GPIO 34 directly.
    //   Zero-current VOUT = 2.5 V  → ACS712_ZERO_OFFSET_MV = 2500 mV
    //   Usable positive range: (3300 - 2500) / 100 mV/A ≈ 8 A
    //   Usable negative range: (2500 -   0) / 100 mV/A ≈ 25 A
    //
    // WITH VOLTAGE DIVIDER (10kΩ / 10kΩ, for full ±30 A range):
    //   ACS712 VOUT is halved: 0–5 V → 0–2.5 V at GPIO 34
    //   Update ACS712_ZERO_OFFSET_MV to 1250 in config.h
    //   Update ACS712_SENSITIVITY to 50 in config.h  (100 mV/A ÷ 2)
    float voltageMv = adcVal * ADC_MV_PER_STEP;

    // Zero-offset at the ADC pin equals ACS712_ZERO_OFFSET_MV for direct
    // connection (2500 mV), or half that (1250 mV) when a 1:1 divider is used.
    float currentAmps = (voltageMv - ACS712_ZERO_OFFSET_MV) / ACS712_SENSITIVITY;

    // Clamp to valid range
    if (currentAmps < 0.0f) currentAmps = 0.0f;

    return currentAmps;
}

float MotorController::getCurrentFiltered()
{
    return _currentFiltered;
}

// -----------------------------------------------------------------------------
// Relay control
// -----------------------------------------------------------------------------
void MotorController::setRelayMotor(bool on)
{
    _relayMotorOn = on;
    digitalWrite(RELAY_CUTOFF_PIN, on ? HIGH : LOW);
}

void MotorController::setRelaySoftStart(bool on)
{
    _relaySoftStartOn = on;
    digitalWrite(RELAY_SOFTSTART_PIN, on ? HIGH : LOW);
}

// -----------------------------------------------------------------------------
// Push button (debounced)
// -----------------------------------------------------------------------------
bool MotorController::isPushButtonPressed()
{
    bool reading = digitalRead(PUSH_BUTTON_PIN);
    uint32_t now = millis();

    if (reading != _lastButtonReading) {
        _lastDebounceMs = now;
    }
    _lastButtonReading = reading;

    if ((now - _lastDebounceMs) >= BUTTON_DEBOUNCE_MS) {
        bool prev = _buttonState;
        _buttonState = reading;
        // Detect falling edge (button pressed, active LOW)
        if (prev == HIGH && _buttonState == LOW) {
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
// Soft start
// -----------------------------------------------------------------------------
void MotorController::startSoftStart(uint8_t durationSec)
{
    if (durationSec < SOFT_START_MIN_TIME) durationSec = SOFT_START_MIN_TIME;
    if (durationSec > SOFT_START_MAX_TIME) durationSec = SOFT_START_MAX_TIME;
    _softStartDuration = durationSec;

    _softStartStartMs = millis();
    _state = MotorState::SOFT_START_RAMP;

    // Relay 1 ON (main motor enabled), Relay 2 OFF (current flows via NTC R5)
    setRelayMotor(true);
    setRelaySoftStart(false);
}

uint8_t MotorController::getSoftStartProgress() const
{
    if (_state != MotorState::SOFT_START_RAMP) {
        return (_state == MotorState::RUNNING) ? 100 : 0;
    }
    uint32_t elapsed = millis() - _softStartStartMs;
    uint32_t total   = (uint32_t)_softStartDuration * 1000UL;
    if (elapsed >= total) return 100;
    return (uint8_t)((elapsed * 100UL) / total);
}

void MotorController::setSoftStartDuration(uint8_t sec)
{
    if (sec < SOFT_START_MIN_TIME) sec = SOFT_START_MIN_TIME;
    if (sec > SOFT_START_MAX_TIME) sec = SOFT_START_MAX_TIME;
    _softStartDuration = sec;
}

// -----------------------------------------------------------------------------
// Stop motor
// -----------------------------------------------------------------------------
void MotorController::stopMotor()
{
    setRelayMotor(false);
    setRelaySoftStart(false);
    _state = MotorState::STOPPED;
}

// -----------------------------------------------------------------------------
// Current threshold
// -----------------------------------------------------------------------------
void MotorController::setCurrentThreshold(float amps)
{
    if (amps < CURRENT_MIN) amps = CURRENT_MIN;
    if (amps > CURRENT_MAX) amps = CURRENT_MAX;
    _currentThreshold = amps;
}

// -----------------------------------------------------------------------------
// NVS settings
// -----------------------------------------------------------------------------
void MotorController::saveSettings()
{
    _prefs.begin(PREF_NAMESPACE, false);
    _prefs.putFloat(PREF_KEY_THRESHOLD,    _currentThreshold);
    _prefs.putUChar(PREF_KEY_SOFT_START_T, _softStartDuration);
    _prefs.end();
}

void MotorController::loadSettings()
{
    _prefs.begin(PREF_NAMESPACE, true);
    _currentThreshold  = _prefs.getFloat(PREF_KEY_THRESHOLD,    CURRENT_DEFAULT);
    _softStartDuration = _prefs.getUChar(PREF_KEY_SOFT_START_T, SOFT_START_DEFAULT_TIME);
    _prefs.end();

    // Sanitise loaded values
    if (_currentThreshold < CURRENT_MIN || _currentThreshold > CURRENT_MAX)
        _currentThreshold = CURRENT_DEFAULT;
    if (_softStartDuration < SOFT_START_MIN_TIME || _softStartDuration > SOFT_START_MAX_TIME)
        _softStartDuration = SOFT_START_DEFAULT_TIME;
}

// -----------------------------------------------------------------------------
// Private helpers
// -----------------------------------------------------------------------------
void MotorController::_handleSoftStartRamp()
{
    uint8_t progress = getSoftStartProgress();

    // Check overcurrent during ramp-up
    if (_currentFiltered >= _currentThreshold) {
        stopMotor();
        _state = MotorState::OVERCURRENT_CUTOFF;
        return;
    }

    if (progress >= 100) {
        // Ramp complete — bypass NTC by switching Relay 2 ON
        setRelaySoftStart(true);
        _state = MotorState::RUNNING;
    }
}

void MotorController::_handleOvercurrentCheck()
{
    if (_currentFiltered >= _currentThreshold) {
        stopMotor();
        _state = MotorState::OVERCURRENT_CUTOFF;
    }
}
