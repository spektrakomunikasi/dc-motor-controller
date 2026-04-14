#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>

#include "config.h"
#include "motor_controller.h"
#include "display.h"
#include "web_server.h"

// =============================================================================
// Global objects
// =============================================================================
MotorController    motor;
DisplayController  display;
WebServerController webServer(motor);

// Display refresh timer
static unsigned long lastDisplayUpdate = 0;

// =============================================================================
// WiFi setup
// =============================================================================
static void setupWiFi()
{
    Serial.printf("[WIFI] Connecting to %s", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > WIFI_TIMEOUT_MS) {
            Serial.println("\n[WIFI] Connection timeout! Continuing without WiFi.");
            return;
        }
        delay(500);
        Serial.print(".");
    }

    Serial.printf("\n[WIFI] Connected! IP: %s\n",
                  WiFi.localIP().toString().c_str());

    // mDNS: accessible as http://<HOSTNAME>.local
    if (MDNS.begin(HOSTNAME)) {
        MDNS.addService("http", "tcp", WEB_SERVER_PORT);
        Serial.printf("[MDNS] Registered as http://%s.local\n", HOSTNAME);
    }
}

// =============================================================================
// setup()
// =============================================================================
void setup()
{
    Serial.begin(115200);
    delay(200);
    Serial.println("\n\n=== ESP32 DC Motor Controller ===");
    Serial.println("ACS712-30A | Relay | OLED/LCD | Web UI");
    Serial.println("=================================\n");

    // Mount SPIFFS for web dashboard
    if (!SPIFFS.begin(true)) {
        Serial.println("[SPIFFS] Mount failed – web UI unavailable");
    } else {
        Serial.println("[SPIFFS] Mounted OK");
    }

    // Initialise motor controller (GPIO, EEPROM, ADC)
    motor.begin();

    // Initialise display (I2C, SSD1306 / LCD1602)
    if (!display.begin()) {
        Serial.println("[DISPLAY] Init failed – continuing without display");
    }

    // Connect to WiFi
    setupWiFi();

    // Start web server
    webServer.begin();

    Serial.println("\n[MAIN] System ready.");
    Serial.printf("[MAIN] Current threshold: %.2f A\n", motor.getThreshold());
}

// =============================================================================
// loop()
// =============================================================================
void loop()
{
    // Update motor controller (sensor read, relay logic, button)
    motor.update();

    // Refresh display at configured interval
    unsigned long now = millis();
    if (now - lastDisplayUpdate >= DISPLAY_REFRESH_INTERVAL_MS) {
        lastDisplayUpdate = now;
        display.update(
            motor.getCurrent(),
            motor.isRelayOn(),
            motor.isMotorRunning(),
            motor.getThreshold(),
            WiFi.RSSI()
        );
    }

    // AsyncWebServer is interrupt-driven; no polling needed
    webServer.update();
}
