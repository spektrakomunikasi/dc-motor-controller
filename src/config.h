#pragma once

#include <Arduino.h>

// =============================================================================
// GPIO Pin Definitions (CONFIRMED FINAL)
// =============================================================================

// Motor Control (via NPN Transistor - Active HIGH)
#define RELAY_CUTOFF_PIN    GPIO_NUM_12   // Relay 1 - Main Motor ON/OFF
#define RELAY_SOFTSTART_PIN GPIO_NUM_13   // Relay 2 - Soft Start NTC Bypass

// User Input
#define PUSH_BUTTON_PIN     GPIO_NUM_14   // Push Button (Start/Restart) INPUT_PULLUP, active LOW

// Sensors & Communication
#define ACS712_PIN          GPIO_NUM_34   // ACS712-30A Current Sensor (ADC1_CH6)
#define I2C_SDA             GPIO_NUM_21   // OLED SDA
#define I2C_SCL             GPIO_NUM_22   // OLED SCL

// =============================================================================
// WiFi Configuration
// Copy src/credentials.h.example → src/credentials.h and fill in your SSID
// and password. credentials.h is excluded from version control (.gitignore).
// =============================================================================
#if __has_include("credentials.h")
  #include "credentials.h"
#else
  #define WIFI_SSID     "YOUR_SSID"
  #define WIFI_PASSWORD "YOUR_PASSWORD"
#endif

// =============================================================================
// Soft Start Configuration
// =============================================================================
#define SOFT_START_MIN_TIME     1    // Minimum ramp-up time (seconds)
#define SOFT_START_MAX_TIME     10   // Maximum ramp-up time (seconds)
#define SOFT_START_DEFAULT_TIME 5    // Default ramp-up time (seconds)

// =============================================================================
// Current Sensor Calibration (ACS712-30A)
// ACS712-30A: 100 mV/A sensitivity, 2.5V offset at 5V supply
// =============================================================================
#define ACS712_SENSITIVITY       100.0f  // mV per Ampere
#define ACS712_ZERO_OFFSET_MV   2500.0f  // Zero-current output voltage (mV) at 5V supply
#define ADC_MAX_VALUE            4095    // 12-bit ADC
#define ADC_REFERENCE_VOLTAGE    3.3f    // ESP32 ADC reference (V)
#define ACS712_VCC               5.0f    // ACS712 supply voltage (V)

// ADC calibration: ACS712 output 0-5V, ESP32 ADC 0-3.3V
// ACS712 VOUT is scaled to ESP32 range (direct connection, GPIO 34 tolerates up to 3.6V)
// At zero current: VOUT = 2.5V → ADC reading = 2.5 / 3.3 * 4095 ≈ 3102
// Each ADC step = 3.3V / 4095 ≈ 0.806mV
#define ADC_MV_PER_STEP (ADC_REFERENCE_VOLTAGE * 1000.0f / ADC_MAX_VALUE)

// =============================================================================
// Current Limit Configuration
// =============================================================================
#define CURRENT_MIN         0.0f    // Minimum threshold (A)
#define CURRENT_MAX         30.0f   // Maximum threshold (A) — ACS712-30A rated
#define CURRENT_DEFAULT     15.0f   // Default cutoff threshold (A)
#define CURRENT_HYSTERESIS  0.5f    // Hysteresis to prevent relay chatter (A)

// EMA (Exponential Moving Average) filter coefficient (0.0 - 1.0)
// Lower = more smoothing, higher = faster response
#define CURRENT_EMA_ALPHA   0.1f

// =============================================================================
// Button Debounce
// =============================================================================
#define BUTTON_DEBOUNCE_MS 50

// =============================================================================
// OLED Configuration
// =============================================================================
#define OLED_ADDR        0x3C  // I2C address (0x3C or 0x3D)
#define OLED_WIDTH       128
#define OLED_HEIGHT      64
#define OLED_RESET_PIN   -1    // No dedicated reset pin

// =============================================================================
// EEPROM / Preferences Configuration
// =============================================================================
#define PREF_NAMESPACE          "motor_ctrl"
#define PREF_KEY_THRESHOLD      "threshold"
#define PREF_KEY_SOFT_START_T   "soft_start_t"

// =============================================================================
// Timing
// =============================================================================
#define DISPLAY_UPDATE_MS   250    // OLED refresh interval
#define MONITOR_UPDATE_MS   100    // Current monitoring interval
#define WIFI_TIMEOUT_MS     10000  // WiFi connection timeout
