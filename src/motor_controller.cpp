#include "motor_controller.h"
#include <EEPROM.h>

// =============================================================================
// MotorController implementation
// =============================================================================

MotorController::MotorController()
    : _currentA(0.0f)
    , _thresholdA(DEFAULT_THRESHOLD_A)
    , _relayOn(false)
    , _faulted(false)
    , _lastButtonState(HIGH)
    , _buttonState(HIGH)
    , _lastDebounceTime(0)
    , _lastSensorTime(0)
    , _lastRelayTime(0)
{
}

// ----------------------------------------------------------------------------
void MotorController::begin()
{
    // Configure GPIO
    pinMode(RELAY_PIN,          OUTPUT);
    pinMode(BUTTON_PIN,         INPUT_PULLUP);
    pinMode(STATUS_LED_PIN,     OUTPUT);

    // ADC attenuation: 11 dB → ~0–3.6 V input range on ESP32
    analogSetAttenuation(ADC_11db);
    analogReadResolution(ADC_RESOLUTION_BITS);

    // Safe default – relay off until user enables
    setRelay(false);
    _faulted = false;

    // Initialise EEPROM and load saved settings
    EEPROM.begin(EEPROM_SIZE);
    loadSettings();

    // Seed the EMA filter with the first reading
    _currentA = readRawCurrent();
}

// ----------------------------------------------------------------------------
void MotorController::update()
{
    unsigned long now = millis();

    // Read and filter current sensor
    if (now - _lastSensorTime >= SENSOR_READ_INTERVAL_MS) {
        _lastSensorTime = now;
        float raw = readRawCurrent();
        applyFilter(raw);
    }

    // Evaluate relay / fault logic
    if (now - _lastRelayTime >= RELAY_CHECK_INTERVAL_MS) {
        _lastRelayTime = now;

        if (!_faulted && _relayOn) {
            // Trip if current meets or exceeds threshold
            if (_currentA >= _thresholdA) {
                _faulted = true;
                setRelay(false);
                Serial.printf("[MOTOR] OVER-CURRENT trip: %.2f A >= %.2f A\n",
                              _currentA, _thresholdA);
            }
        }
    }

    // Handle push button (reset fault / toggle relay)
    handleButton();

    // Status LED mirrors relay state
    digitalWrite(STATUS_LED_PIN, _relayOn ? HIGH : LOW);
}

// ----------------------------------------------------------------------------
// ACS712-30A current reading
// Formula: I = (V_adc - V_offset) / sensitivity
// V_offset is the sensor zero-current output (2.5 V for 5 V supply).
// If a voltage divider is used between sensor and ADC, the raw ADC voltage
// must be divided by ACS712_DIVIDER_RATIO to recover the actual sensor voltage.
// ----------------------------------------------------------------------------
float MotorController::readRawCurrent()
{
    // Average multiple samples to reduce ADC noise
    long sum = 0;
    for (int i = 0; i < ADC_SAMPLES; ++i) {
        sum += analogRead(CURRENT_SENSOR_PIN);
    }
    float adcValue  = static_cast<float>(sum) / ADC_SAMPLES;

    // Convert ADC counts → voltage at the ADC pin
    float adcVoltage = (adcValue / ADC_MAX_VALUE) * ADC_VREF;

    // Recover actual sensor output voltage (account for voltage divider)
    float sensorVoltage = (ACS712_DIVIDER_RATIO > 0.0f)
                          ? adcVoltage / ACS712_DIVIDER_RATIO
                          : adcVoltage;

    // Convert to current
    float current = (sensorVoltage - ACS712_ZERO_OFFSET_V)
                    / ACS712_SENSITIVITY_V_PER_A;

    // Clamp to 0 A minimum (no negative current expected in DC motor app)
    if (current < 0.0f) current = 0.0f;

    return current;
}

// ----------------------------------------------------------------------------
void MotorController::applyFilter(float raw)
{
    // Exponential Moving Average: y[n] = α * x[n] + (1 - α) * y[n-1]
    _currentA = EMA_ALPHA * raw + (1.0f - EMA_ALPHA) * _currentA;
}

// ----------------------------------------------------------------------------
void MotorController::handleButton()
{
    bool reading = digitalRead(BUTTON_PIN);  // LOW when pressed (INPUT_PULLUP)

    // Reset debounce timer on any change
    if (reading != _lastButtonState) {
        _lastDebounceTime = millis();
    }

    if ((millis() - _lastDebounceTime) >= BUTTON_DEBOUNCE_MS) {
        // Stable reading – check for falling edge (press event)
        if (reading != _buttonState) {
            _buttonState = reading;

            if (_buttonState == LOW) {
                // Button pressed – reset fault and re-enable relay
                Serial.println("[MOTOR] Button pressed – resetting fault, relay ON");
                _faulted = false;
                setRelay(true);
            }
        }
    }

    _lastButtonState = reading;
}

// ----------------------------------------------------------------------------
void MotorController::setRelay(bool on)
{
    _relayOn = on;
    digitalWrite(RELAY_PIN, on ? HIGH : LOW);
    Serial.printf("[MOTOR] Relay %s\n", on ? "ON" : "OFF");
}

// ----------------------------------------------------------------------------
void MotorController::relayOn()
{
    _faulted = false;
    setRelay(true);
}

void MotorController::relayOff()
{
    setRelay(false);
}

// ----------------------------------------------------------------------------
void MotorController::setThreshold(float amperes)
{
    if (amperes < MIN_THRESHOLD_A) amperes = MIN_THRESHOLD_A;
    if (amperes > MAX_THRESHOLD_A) amperes = MAX_THRESHOLD_A;
    _thresholdA = amperes;
    Serial.printf("[MOTOR] Threshold set to %.2f A\n", _thresholdA);
}

// ----------------------------------------------------------------------------
// EEPROM layout:
//   [0..3]  float   threshold value
//   [4..7]  uint32_t magic word (0xDEADBEEF)
// ----------------------------------------------------------------------------
void MotorController::saveSettings()
{
    EEPROM.put(EEPROM_ADDR_THRESHOLD, _thresholdA);
    uint32_t magic = EEPROM_MAGIC_VALUE;
    EEPROM.put(EEPROM_MAGIC_ADDR, magic);
    EEPROM.commit();
    Serial.printf("[MOTOR] Settings saved: threshold=%.2f A\n", _thresholdA);
}

void MotorController::loadSettings()
{
    uint32_t magic = 0;
    EEPROM.get(EEPROM_MAGIC_ADDR, magic);

    if (magic == EEPROM_MAGIC_VALUE) {
        float stored = 0.0f;
        EEPROM.get(EEPROM_ADDR_THRESHOLD, stored);

        if (stored >= MIN_THRESHOLD_A && stored <= MAX_THRESHOLD_A) {
            _thresholdA = stored;
            Serial.printf("[MOTOR] Loaded threshold from EEPROM: %.2f A\n", _thresholdA);
            return;
        }
    }

    // Fallback to default
    _thresholdA = DEFAULT_THRESHOLD_A;
    Serial.printf("[MOTOR] EEPROM invalid – using default threshold: %.2f A\n", _thresholdA);
}
