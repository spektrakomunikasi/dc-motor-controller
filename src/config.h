#pragma once

// =============================================================================
// ESP32 DC Motor Controller - Configuration
// Hardware: ESP32 + ACS712-30A + Relay + OLED/LCD1602 I2C + Push Button
// =============================================================================

// -----------------------------------------------------------------------------
// GPIO Pin Definitions
// -----------------------------------------------------------------------------

// Relay control pin (active HIGH - relay ON means motor running)
#define RELAY_PIN           26

// Push button pin (INPUT_PULLUP - LOW when pressed)
#define BUTTON_PIN          27

// ACS712-30A analog output → ESP32 ADC1 channel (GPIO 34 is input-only)
#define CURRENT_SENSOR_PIN  34

// I2C pins (default ESP32: SDA=21, SCL=22)
#define I2C_SDA_PIN         21
#define I2C_SCL_PIN         22

// Optional: onboard LED for status indication
#define STATUS_LED_PIN      2

// -----------------------------------------------------------------------------
// WiFi Credentials
// -----------------------------------------------------------------------------
#define WIFI_SSID           "YOUR_WIFI_SSID"
#define WIFI_PASSWORD       "YOUR_WIFI_PASSWORD"

// Hostname for mDNS (access via http://motor-ctrl.local)
#define HOSTNAME            "motor-ctrl"

// WiFi connection timeout in milliseconds
#define WIFI_TIMEOUT_MS     15000

// -----------------------------------------------------------------------------
// Current Thresholds (Amperes)
// -----------------------------------------------------------------------------

// Default over-current trip threshold (adjustable via web / button)
#define DEFAULT_THRESHOLD_A     10.0f

// Minimum and maximum allowed threshold values
#define MIN_THRESHOLD_A          0.5f
#define MAX_THRESHOLD_A         20.0f

// Hysteresis to prevent relay chatter around the threshold
// Relay trips at threshold; re-enables when current drops below (threshold - hysteresis)
#define THRESHOLD_HYSTERESIS_A   0.5f

// -----------------------------------------------------------------------------
// Display Selection
// 0 = OLED SSD1306 128x64 I2C
// 1 = LCD 1602 I2C (PCF8574 backpack)
// -----------------------------------------------------------------------------
#define DISPLAY_TYPE        0

// I2C addresses
#define OLED_I2C_ADDRESS    0x3C    // Common address for SSD1306 128x64
#define LCD_I2C_ADDRESS     0x27    // Common address for PCF8574 backpack

// OLED display dimensions
#define OLED_WIDTH          128
#define OLED_HEIGHT         64

// Display update interval in milliseconds
#define DISPLAY_UPDATE_MS   500

// -----------------------------------------------------------------------------
// ACS712-30A Sensor Calibration
// -----------------------------------------------------------------------------

// ESP32 ADC resolution
#define ADC_RESOLUTION_BITS     12
#define ADC_MAX_VALUE           4095    // 2^12 - 1

// ESP32 ADC reference voltage (volts)
#define ADC_VREF                3.3f

// ACS712-30A sensitivity: 100 mV/A (0.1 V/A)
#define ACS712_SENSITIVITY_V_PER_A  0.100f

// ACS712 output at zero current with 5 V supply = 2.5 V
// Because ESP32 ADC max is 3.3 V, add a voltage divider if sensor runs on 5 V
// For 3.3 V supply: zero-current midpoint ≈ 1.65 V
// Set this to match your actual hardware:
//   5 V supply → 2.5 V offset
//   3.3 V supply → 1.65 V offset
#define ACS712_ZERO_OFFSET_V    2.5f

// If using a voltage divider between ACS712 VOUT and ESP32 ADC, set the ratio
// divider_ratio = R2 / (R1 + R2)  (set to 1.0 if no divider)
#define ACS712_DIVIDER_RATIO    1.0f

// Exponential Moving Average (EMA) filter coefficient (0 < α ≤ 1)
// Lower = more smoothing / slower response; Higher = less smoothing / faster response
#define EMA_ALPHA               0.1f

// Number of ADC samples averaged per reading
#define ADC_SAMPLES             10

// -----------------------------------------------------------------------------
// Push Button Debounce
// -----------------------------------------------------------------------------
#define BUTTON_DEBOUNCE_MS      50

// -----------------------------------------------------------------------------
// EEPROM Settings
// -----------------------------------------------------------------------------
#define EEPROM_SIZE             64      // bytes allocated
#define EEPROM_ADDR_THRESHOLD   0       // float (4 bytes) starting at address 0
#define EEPROM_MAGIC_ADDR       4       // uint32_t magic word to validate data
#define EEPROM_MAGIC_VALUE      0xDEADBEEF

// -----------------------------------------------------------------------------
// Web Server
// -----------------------------------------------------------------------------
#define WEB_SERVER_PORT         80

// -----------------------------------------------------------------------------
// Timing intervals (milliseconds)
// -----------------------------------------------------------------------------
#define SENSOR_READ_INTERVAL_MS     100     // Read current sensor every 100 ms
#define RELAY_CHECK_INTERVAL_MS     100     // Evaluate relay state every 100 ms
#define DISPLAY_REFRESH_INTERVAL_MS 500     // Refresh display every 500 ms
