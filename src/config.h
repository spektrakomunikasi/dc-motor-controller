#pragma once

// =============================================================================
// DC Motor Controller - Configuration
// ESP32 + ACS712-30A + OLED + 2x Relay + WiFi AP Setup Mode
// =============================================================================

// -----------------------------------------------------------------------------
// Firmware Version
// -----------------------------------------------------------------------------
#define FIRMWARE_VERSION "1.0.0"
#define DEVICE_NAME      "DC Motor Controller"

// -----------------------------------------------------------------------------
// Pin Definitions
// -----------------------------------------------------------------------------

// Relay outputs (via NPN transistor drivers)
#define RELAY_CUTOFF_PIN      GPIO_NUM_12   // D12 - Relay 1: Main motor cutoff
#define RELAY_SOFTSTART_PIN   GPIO_NUM_13   // D13 - Relay 2: Soft-start bypass

// User input
#define PUSH_BUTTON_PIN       GPIO_NUM_14   // D14 - Push button (Start/Restart)

// Current sensor
#define ACS712_PIN            GPIO_NUM_34   // GPIO34 - ACS712-30A analog output (ADC1_CH6)

// OLED display (I2C)
#define I2C_SDA               GPIO_NUM_21   // D21 - I2C SDA
#define I2C_SCL               GPIO_NUM_22   // D22 - I2C SCL
#define OLED_ADDRESS          0x3C

// -----------------------------------------------------------------------------
// Motor Controller Settings
// -----------------------------------------------------------------------------
#define DEFAULT_CURRENT_LIMIT_A     15.0f   // Default overcurrent threshold (Amperes)
#define MIN_CURRENT_LIMIT_A          1.0f
#define MAX_CURRENT_LIMIT_A         30.0f

#define DEFAULT_SOFT_START_MS     3000      // Default soft-start ramp duration (ms)
#define MIN_SOFT_START_MS         1000      // 1 second
#define MAX_SOFT_START_MS        10000      // 10 seconds

// ACS712-30A calibration
// Vcc = 5V, sensitivity = 66 mV/A, zero offset = Vcc/2 = 2.5V
// ADC reference on ESP32 = 3.3V, 12-bit = 4095 counts
// Note: ACS712 output max 5V but ESP32 ADC max 3.3V — use voltage divider (2:3)
#define ACS712_SENSITIVITY_MV_A   66.0f    // mV per Ampere for 30A module
#define ACS712_VREF_MV          3300.0f    // ESP32 ADC reference voltage (mV)
#define ACS712_ADC_BITS           12       // 12-bit ADC
#define ACS712_ADC_MAX          4095.0f
// Zero-current ADC reading (calibrate to ~midpoint after voltage divider)
#define ACS712_ZERO_OFFSET       2048      // Adjust after hardware calibration

#define CURRENT_SAMPLE_COUNT        10     // Samples to average per reading
#define CURRENT_READ_INTERVAL_MS    50     // ms between readings

// Debounce for push button
#define BUTTON_DEBOUNCE_MS          50

// Soft-start relay timing
#define SOFTSTART_RELAY_STEP_MS    100     // Step interval during ramp-up

// -----------------------------------------------------------------------------
// Display Settings
// -----------------------------------------------------------------------------
#define DISPLAY_UPDATE_INTERVAL_MS  200    // OLED refresh rate
#define OLED_SCREEN_WIDTH           128
#define OLED_SCREEN_HEIGHT           64

// -----------------------------------------------------------------------------
// WiFi AP Setup Mode
// -----------------------------------------------------------------------------
#define WIFI_AP_SSID_PREFIX        "MOTOR_CTRL_"   // Appends last 4 MAC bytes
#define WIFI_AP_PASSWORD           "12345678"       // AP password (min 8 chars)
#define WIFI_CONNECTION_TIMEOUT_MS  15000           // 15 seconds to connect
#define WIFI_RECONNECT_INTERVAL_MS  30000           // Retry interval after failure
#define WIFI_SCAN_INTERVAL_MS       30000           // Network scan cache TTL

// NVS (Non-Volatile Storage) keys — stored via Preferences library
#define NVS_NAMESPACE          "motor_ctrl"
#define PREF_KEY_WIFI_SSID     "wifi_ssid"
#define PREF_KEY_WIFI_PASSWORD "wifi_password"
#define PREF_KEY_CURRENT_LIMIT "cur_limit"
#define PREF_KEY_SOFT_START    "soft_start"

// -----------------------------------------------------------------------------
// Web Server
// -----------------------------------------------------------------------------
#define WEB_SERVER_PORT     80
#define API_JSON_CAPACITY  512
