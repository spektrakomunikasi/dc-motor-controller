#include "display.h"

// =============================================================================
// DisplayController Implementation
// =============================================================================

DisplayController::DisplayController(WiFiManager& wifiMgr, MotorController& motor)
    : _display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, -1)
    , _wifiMgr(wifiMgr)
    , _motor(motor)
    , _lastUpdateMs(0)
{}

void DisplayController::init() {
    Wire.begin(I2C_SDA, I2C_SCL);

    if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println("[Display] SSD1306 not found — check wiring");
        return;
    }

    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(0, 0);
    _display.println(DEVICE_NAME);
    _display.println("  v" FIRMWARE_VERSION);
    _display.println("  Initializing...");
    _display.display();
    Serial.println("[Display] Initialized");
}

void DisplayController::update() {
    uint32_t now = millis();
    if (now - _lastUpdateMs < DISPLAY_UPDATE_INTERVAL_MS) return;
    _lastUpdateMs = now;

    _display.clearDisplay();

    switch (_wifiMgr.getState()) {
        case WiFiManager::AP_MODE:
            _drawAPMode();
            break;
        case WiFiManager::CONNECTING:
            _drawConnecting();
            break;
        case WiFiManager::CONNECTED:
            _drawConnected();
            break;
        case WiFiManager::DISCONNECTED:
            _drawDisconnected();
            break;
        default:
            _display.setTextSize(1);
            _display.setCursor(0, 0);
            _display.println("Initializing...");
            break;
    }

    _display.display();
}

// -----------------------------------------------------------------------------
// Screen layouts
// -----------------------------------------------------------------------------

// AP MODE screen
// ┌──────────────────────┐
// │ MOTOR CONTROL v1.0   │
// │ WiFi: AP MODE        │
// │ SSID: MOTOR_CTRL_XX  │
// │ IP: 192.168.4.1      │
// │                      │
// │ http://192.168.4.1   │
// │ to setup WiFi        │
// └──────────────────────┘
void DisplayController::_drawAPMode() {
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);

    // Header
    _display.setCursor(0, 0);
    _display.println("MOTOR CTRL v" FIRMWARE_VERSION);

    _display.drawLine(0, 9, OLED_SCREEN_WIDTH - 1, 9, SSD1306_WHITE);

    _display.setCursor(0, 12);
    _display.println("WiFi: AP MODE");

    _display.setCursor(0, 22);
    String ap = _wifiMgr.getAPName();
    if (ap.length() > 16) ap = ap.substring(0, 16);
    _display.print("SSID: ");
    _display.println(ap);

    _display.setCursor(0, 32);
    _display.print("IP: ");
    _display.println(_wifiMgr.getIP());

    _display.drawLine(0, 42, OLED_SCREEN_WIDTH - 1, 42, SSD1306_WHITE);

    _display.setCursor(0, 45);
    _display.println("Visit http://");
    _display.setCursor(0, 55);
    _display.print(_wifiMgr.getIP());
    _display.println("/setup.html");
}

// CONNECTING screen
// │ MOTOR CONTROL v1.0   │
// │ WiFi: Connecting...  │
// │ SSID: MyWiFi         │
void DisplayController::_drawConnecting() {
    _display.setTextSize(1);
    _display.setCursor(0, 0);
    _display.println("MOTOR CTRL v" FIRMWARE_VERSION);
    _display.drawLine(0, 9, OLED_SCREEN_WIDTH - 1, 9, SSD1306_WHITE);

    _display.setCursor(0, 12);
    _display.println("WiFi: Connecting...");

    _display.setCursor(0, 22);
    String ssid = _wifiMgr.getSSID();
    if (ssid.length() > 16) ssid = ssid.substring(0, 16);
    _display.print("SSID: ");
    _display.println(ssid);

    // Animate dots based on time
    uint8_t dots = (millis() / 500) % 4;
    _display.setCursor(0, 34);
    _display.print("Please wait");
    for (uint8_t i = 0; i < dots; i++) _display.print(".");
}

// CONNECTED screen
// │ MOTOR CONTROL v1.0   │
// │ WiFi: Connected ●●   │
// │ SSID: MyWiFi         │
// │ IP: 192.168.1.100    │
// │ Current: 12.45 A     │
// │ Limit:   15.0 A      │
// │ Motor: RUNNING       │
void DisplayController::_drawConnected() {
    _display.setTextSize(1);
    _display.setCursor(0, 0);
    _display.println("MOTOR CTRL v" FIRMWARE_VERSION);
    _display.drawLine(0, 9, OLED_SCREEN_WIDTH - 1, 9, SSD1306_WHITE);

    // WiFi section
    _display.setCursor(0, 12);
    int8_t rssi = _wifiMgr.getRSSI();
    String bars = (rssi > -60) ? "***" : (rssi > -75) ? "**" : "*";
    _display.print("WiFi: ");
    _display.println(bars);

    _display.setCursor(0, 22);
    String ssid = _wifiMgr.getSSID();
    if (ssid.length() > 16) ssid = ssid.substring(0, 16);
    _display.print("SSID: ");
    _display.println(ssid);

    _display.setCursor(0, 32);
    _display.print("IP: ");
    _display.println(_wifiMgr.getIP());

    _display.drawLine(0, 42, OLED_SCREEN_WIDTH - 1, 42, SSD1306_WHITE);

    // Motor section
    _display.setCursor(0, 45);
    _display.printf("Cur:%.2fA Lim:%.0fA",
                    _motor.getCurrent(), _motor.getCurrentLimit());

    _display.setCursor(0, 55);
    _display.print("Motor: ");
    _display.println(_motor.getStateString());
}

// DISCONNECTED screen
void DisplayController::_drawDisconnected() {
    _display.setTextSize(1);
    _display.setCursor(0, 0);
    _display.println("MOTOR CTRL v" FIRMWARE_VERSION);
    _display.drawLine(0, 9, OLED_SCREEN_WIDTH - 1, 9, SSD1306_WHITE);

    _display.setCursor(0, 14);
    _display.println("WiFi: DISCONNECTED");
    _display.setCursor(0, 26);
    _display.println("Reconnecting...");

    _display.drawLine(0, 38, OLED_SCREEN_WIDTH - 1, 38, SSD1306_WHITE);

    _display.setCursor(0, 42);
    _display.printf("Cur:%.2fA Lim:%.0fA",
                    _motor.getCurrent(), _motor.getCurrentLimit());
    _display.setCursor(0, 52);
    _display.print("Motor: ");
    _display.println(_motor.getStateString());
}
