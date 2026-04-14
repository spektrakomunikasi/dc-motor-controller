#include "display.h"

#if DISPLAY_TYPE == 0
  // ---- OLED SSD1306 ----
  #include <Wire.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
  static Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

#elif DISPLAY_TYPE == 1
  // ---- LCD 1602 I2C ----
  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>
  static LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, 16, 2);

#endif

// =============================================================================
// DisplayController implementation
// =============================================================================

DisplayController::DisplayController()
    : _initialized(false)
{
}

// ----------------------------------------------------------------------------
bool DisplayController::begin()
{
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

#if DISPLAY_TYPE == 0
    if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
        Serial.println("[DISPLAY] SSD1306 OLED not found!");
        return false;
    }
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    _initialized = true;
    Serial.println("[DISPLAY] OLED SSD1306 initialised");

#elif DISPLAY_TYPE == 1
    lcd.init();
    lcd.backlight();
    _initialized = true;
    Serial.println("[DISPLAY] LCD 1602 I2C initialised");

#endif

    showSplash();
    return _initialized;
}

// ----------------------------------------------------------------------------
void DisplayController::showSplash()
{
    if (!_initialized) return;

#if DISPLAY_TYPE == 0
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.println("  DC Motor Ctrl");
    oled.println("  ESP32 + ACS712");
    oled.println("");
    oled.println("  Booting...");
    oled.display();

#elif DISPLAY_TYPE == 1
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DC Motor Ctrl");
    lcd.setCursor(0, 1);
    lcd.print("Booting...");

#endif

    delay(1500);
}

// ----------------------------------------------------------------------------
void DisplayController::showMessage(const char* line1, const char* line2)
{
    if (!_initialized) return;

#if DISPLAY_TYPE == 0
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setCursor(0, 10);
    oled.println(line1 ? line1 : "");
    oled.setCursor(0, 24);
    oled.println(line2 ? line2 : "");
    oled.display();

#elif DISPLAY_TYPE == 1
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1 ? line1 : "");
    if (line2) {
        lcd.setCursor(0, 1);
        lcd.print(line2);
    }

#endif
}

// ----------------------------------------------------------------------------
void DisplayController::update(float currentA, bool relayOn, bool motorRunning,
                                float thresholdA, int rssi)
{
    if (!_initialized) return;

#if DISPLAY_TYPE == 0
    updateOLED(currentA, relayOn, motorRunning, thresholdA, rssi);
#elif DISPLAY_TYPE == 1
    updateLCD(currentA, relayOn, motorRunning, thresholdA, rssi);
#endif
}

// ----------------------------------------------------------------------------
// OLED layout (128×64 pixels, 8 px per text row at size 1)
//
//  Row 0 (y=0):   "Current:  XX.XX A"
//  Row 1 (y=10):  "Relay: ON   Motor: RUN"
//  Row 2 (y=20):  "Set:   XX.XX A"
//  Row 3 (y=32):  WiFi signal bar  "WiFi: -XXdBm"
//  Row 4 (y=44):  [current bar graph]
// ----------------------------------------------------------------------------
void DisplayController::updateOLED(float currentA, bool relayOn, bool motorRunning,
                                    float thresholdA, int rssi)
{
#if DISPLAY_TYPE == 0
    char buf[32];

    oled.clearDisplay();

    // -- Title bar --
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.print("DC Motor Controller");
    oled.drawLine(0, 9, 127, 9, SSD1306_WHITE);

    // -- Current (large) --
    oled.setTextSize(2);
    oled.setCursor(0, 13);
    snprintf(buf, sizeof(buf), "%.2fA", currentA);
    oled.print(buf);

    // -- Status row --
    oled.setTextSize(1);
    oled.setCursor(0, 33);
    oled.print("Relay:");
    oled.print(relayOn ? "ON " : "OFF");
    oled.print(" Motor:");
    oled.print(motorRunning ? "RUN" : "STP");

    // -- Threshold --
    oled.setCursor(0, 43);
    snprintf(buf, sizeof(buf), "Set: %.1fA", thresholdA);
    oled.print(buf);

    // -- WiFi RSSI --
    oled.setCursor(68, 43);
    snprintf(buf, sizeof(buf), "WiFi:%ddBm", rssi);
    oled.print(buf);

    // -- Current bar graph (bottom row) --
    // Full width = 128 px maps to MAX_THRESHOLD_A
    int barWidth = static_cast<int>(
        (currentA / MAX_THRESHOLD_A) * 124.0f);
    if (barWidth > 124) barWidth = 124;
    if (barWidth > 0) {
        oled.fillRect(0, 55, barWidth, 8, SSD1306_WHITE);
    }
    oled.drawRect(0, 55, 124, 8, SSD1306_WHITE);

    oled.display();
#endif
}

// ----------------------------------------------------------------------------
// LCD 1602 layout (16 columns × 2 rows):
//
//  Row 0: "I:XX.XXA S:XX.XA"
//  Row 1: "Rly:ON  Mtr:RUN "
// ----------------------------------------------------------------------------
void DisplayController::updateLCD(float currentA, bool relayOn, bool motorRunning,
                                   float thresholdA, int rssi)
{
#if DISPLAY_TYPE == 1
    char buf[17];

    // Row 0: current and setpoint
    snprintf(buf, sizeof(buf), "I:%5.2fA S:%4.1fA", currentA, thresholdA);
    lcd.setCursor(0, 0);
    lcd.print(buf);

    // Row 1: relay and motor status
    snprintf(buf, sizeof(buf), "Rly:%-3s  Mtr:%-3s",
             relayOn     ? "ON"  : "OFF",
             motorRunning ? "RUN" : "STP");
    lcd.setCursor(0, 1);
    lcd.print(buf);
#endif
}
