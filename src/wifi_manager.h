#pragma once

#include <Arduino.h>
#include <vector>
#include <Preferences.h>
#include <WiFi.h>
#include "config.h"

// =============================================================================
// WiFiManager
// Handles WiFi AP setup mode and STA (client) connection with NVS persistence.
// =============================================================================
class WiFiManager {
public:
    enum WiFiState {
        INITIALIZING,
        AP_MODE,      // Access Point mode — serving setup portal
        CONNECTING,   // Attempting connection to saved SSID
        CONNECTED,    // Connected to external WiFi
        DISCONNECTED  // Was connected but lost signal
    };

    WiFiManager();

    // Lifecycle
    void init();
    void update();

    // State
    WiFiState getState()    const { return _state; }
    bool      isConnected() const { return _state == CONNECTED; }
    bool      isAPMode()    const { return _state == AP_MODE; }

    // Network info
    String getAPName()  const { return _apName; }
    String getSSID()    const { return _savedSSID; }
    String getIP()      const;
    int8_t getRSSI()    const;

    // Credential management (persisted to NVS)
    void   setWiFiCredentials(const String& ssid, const String& password);
    String getSavedSSID()       const { return _savedSSID; }
    void   clearWiFiCredentials();

    // Network scanning
    void                  scanNetworks();
    std::vector<String>   getScannedNetworks() const { return _scannedNetworks; }

    // Schedule a reboot (used after saving credentials)
    void scheduleReboot() { _shouldReboot = true; _rebootScheduledMs = millis(); }

private:
    WiFiState           _state;
    String              _savedSSID;
    String              _savedPassword;
    String              _apName;
    std::vector<String> _scannedNetworks;
    uint32_t            _lastScanMs;
    uint32_t            _connectionAttemptMs;
    bool                _shouldReboot;
    uint32_t            _rebootScheduledMs;

    Preferences _prefs;

    void _startAPMode();
    void _attemptConnection();
    void _handleState();
    void _buildAPName();
    void _loadCredentials();
};
