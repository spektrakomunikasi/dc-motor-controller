#include <Arduino.h>
#include <WiFi.h>
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

// Timers
static uint32_t lastDisplayMs = 0;

// =============================================================================
// WiFi connect helper
// =============================================================================
static void connectWiFi()
{
    Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > WIFI_TIMEOUT_MS) {
            Serial.println(F("\n[WiFi] Connection timed out — running without WiFi"));
            return;
        }
        delay(200);
        Serial.print('.');
    }
    Serial.print(F("\n[WiFi] Connected! IP: "));
    Serial.println(WiFi.localIP());
}

// =============================================================================
// setup()
// =============================================================================
void setup()
{
    Serial.begin(115200);
    Serial.println(F("[BOOT] ESP32 DC Motor Controller v1.0"));

    // Init motor controller (pins, ADC, NVS settings)
    motor.init();

    // Init OLED
    display.init();

    // Connect WiFi
    connectWiFi();

    // Start web server (SPIFFS + REST API)
    if (WiFi.status() == WL_CONNECTED) {
        webServer.init();
        Serial.print(F("[WEB] Dashboard: http://"));
        Serial.println(WiFi.localIP());
    }

    Serial.println(F("[BOOT] Ready"));
}

// =============================================================================
// loop()
// =============================================================================
void loop()
{
    // Update motor state machine (current reading, soft start, overcurrent)
    motor.update();

    // Check push button — start/restart soft start sequence
    if (motor.isPushButtonPressed()) {
        MotorController::MotorState s = motor.getMotorState();
        if (s == MotorController::MotorState::STOPPED ||
            s == MotorController::MotorState::OVERCURRENT_CUTOFF) {
            Serial.println(F("[BTN] Start button pressed — initiating soft start"));
            motor.startSoftStart(motor.getSoftStartDuration());
        }
    }

    // Update OLED at a limited rate
    uint32_t now = millis();
    if (now - lastDisplayMs >= DISPLAY_UPDATE_MS) {
        lastDisplayMs = now;

        bool wifiOk = (WiFi.status() == WL_CONNECTED);
        int8_t rssi = wifiOk ? (int8_t)WiFi.RSSI() : -100;

        display.update(
            motor.getCurrentFiltered(),
            motor.getCurrentThreshold(),
            motor.getMotorState(),
            motor.getSoftStartProgress(),
            motor.getRelayMotorState(),
            motor.getRelaySoftStartState(),
            wifiOk,
            rssi
        );
    }
}
