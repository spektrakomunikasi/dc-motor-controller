/**
 * DC Motor Controller — Main Entry Point
 * ESP32 + ACS712-30A + 2× Relay + OLED + WiFi AP Setup Mode
 *
 * Pin assignments (see config.h):
 *   D12  → Relay 1 (motor cutoff)
 *   D13  → Relay 2 (soft-start bypass)
 *   D14  → Push button (Start / Stop)
 *   GPIO34 → ACS712-30A VOUT (current sensor)
 *   GPIO21/22 → OLED I2C (SDA/SCL)
 */

#include <Arduino.h>
#include <SPIFFS.h>
#include "config.h"
#include "wifi_manager.h"
#include "motor_controller.h"
#include "display.h"
#include "web_server.h"
#include "setup_server.h"

// =============================================================================
// Global objects
// =============================================================================
WiFiManager       wifiManager;
MotorController   motorController;
DisplayController display(wifiManager, motorController);

// Web servers — only one is active at a time
// SetupWebServer is started in AP mode; WebServerController in STA mode.
SetupWebServer*      setupServer  = nullptr;
WebServerController* webServer    = nullptr;

// Track which server is currently running
static bool _setupServerRunning = false;
static bool _webServerRunning   = false;

// =============================================================================
// setup()
// =============================================================================
void setup() {
    Serial.begin(115200);
    Serial.println("\n\n=== DC Motor Controller ===");
    Serial.println("Firmware v" FIRMWARE_VERSION);
    Serial.println("===========================\n");

    // Mount SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("[SPIFFS] Mount failed — web UI unavailable");
    } else {
        Serial.println("[SPIFFS] Mounted OK");
    }

    // Initialize subsystems
    motorController.init();
    display.init();
    wifiManager.init();

    // Web server will be started in loop() once WiFi state is known
}

// =============================================================================
// loop()
// =============================================================================
void loop() {
    // Update all subsystems
    wifiManager.update();
    motorController.update();
    display.update();

    // -----------------------------------------------------------------
    // Lazy-start the appropriate web server based on WiFi state
    // -----------------------------------------------------------------
    if (wifiManager.isAPMode() && !_setupServerRunning) {
        Serial.println("[Main] WiFi in AP mode — starting SetupWebServer");
        setupServer = new SetupWebServer(wifiManager, motorController);
        setupServer->init();
        _setupServerRunning = true;
        _webServerRunning   = false;
    } else if (wifiManager.isConnected() && !_webServerRunning) {
        Serial.println("[Main] WiFi connected — starting WebServerController");
        // If setup server was running, it needs to be stopped first.
        // Since AsyncWebServer doesn't have a stop() method in all versions,
        // we rely on the fact that a reboot happens after credentials are saved,
        // so we only ever start one server per boot.
        webServer = new WebServerController(wifiManager, motorController);
        webServer->init();
        _webServerRunning   = true;
        _setupServerRunning = false;
    }

    // Update active server (no-op for AsyncWebServer but keeps the API consistent)
    if (_setupServerRunning && setupServer) {
        setupServer->update();
    }
    if (_webServerRunning && webServer) {
        webServer->update();
    }
}
