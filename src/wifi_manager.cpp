#include "wifi_manager.h"

// =============================================================================
// WiFiManager Implementation
// =============================================================================

WiFiManager::WiFiManager()
    : _state(INITIALIZING)
    , _lastScanMs(0)
    , _connectionAttemptMs(0)
    , _shouldReboot(false)
    , _rebootScheduledMs(0)
{}

void WiFiManager::init() {
    _buildAPName();
    _loadCredentials();

    Serial.printf("[WiFi] AP name will be: %s\n", _apName.c_str());

    if (_savedSSID.isEmpty()) {
        Serial.println("[WiFi] No saved SSID — starting AP mode");
        _startAPMode();
    } else {
        Serial.printf("[WiFi] Saved SSID found: %s — attempting connection\n", _savedSSID.c_str());
        _attemptConnection();
    }
}

void WiFiManager::update() {
    _handleState();

    // Reboot after a short delay so the HTTP response can be sent first
    if (_shouldReboot && (millis() - _rebootScheduledMs > 1500)) {
        Serial.println("[WiFi] Rebooting...");
        ESP.restart();
    }
}

String WiFiManager::getIP() const {
    if (_state == AP_MODE) {
        return WiFi.softAPIP().toString();
    }
    if (_state == CONNECTED) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

int8_t WiFiManager::getRSSI() const {
    if (_state == CONNECTED) {
        return WiFi.RSSI();
    }
    return 0;
}

void WiFiManager::setWiFiCredentials(const String& ssid, const String& password) {
    _savedSSID     = ssid;
    _savedPassword = password;

    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.putString(PREF_KEY_WIFI_SSID,     ssid);
    _prefs.putString(PREF_KEY_WIFI_PASSWORD, password);
    _prefs.end();

    Serial.printf("[WiFi] Credentials saved: SSID=%s\n", ssid.c_str());
}

void WiFiManager::clearWiFiCredentials() {
    _savedSSID     = "";
    _savedPassword = "";

    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.remove(PREF_KEY_WIFI_SSID);
    _prefs.remove(PREF_KEY_WIFI_PASSWORD);
    _prefs.end();

    Serial.println("[WiFi] Credentials cleared");
}

void WiFiManager::scanNetworks() {
    // Non-blocking scan request
    int n = WiFi.scanNetworks(false, false); // sync scan
    _scannedNetworks.clear();
    if (n > 0) {
        for (int i = 0; i < n; i++) {
            _scannedNetworks.push_back(WiFi.SSID(i));
        }
    }
    _lastScanMs = millis();
    Serial.printf("[WiFi] Scanned %d networks\n", n);
}

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------

void WiFiManager::_buildAPName() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char suffix[9];
    snprintf(suffix, sizeof(suffix), "%02X%02X%02X%02X", mac[2], mac[3], mac[4], mac[5]);
    _apName = String(WIFI_AP_SSID_PREFIX) + suffix;
}

void WiFiManager::_loadCredentials() {
    _prefs.begin(NVS_NAMESPACE, true);
    _savedSSID     = _prefs.getString(PREF_KEY_WIFI_SSID,     "");
    _savedPassword = _prefs.getString(PREF_KEY_WIFI_PASSWORD, "");
    _prefs.end();
}

void WiFiManager::_startAPMode() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);

    bool ok = WiFi.softAP(_apName.c_str(), WIFI_AP_PASSWORD);
    if (ok) {
        _state = AP_MODE;
        Serial.printf("[WiFi] AP started: SSID=%s, IP=%s\n",
                      _apName.c_str(), WiFi.softAPIP().toString().c_str());
        // Kick off initial scan so setup page has networks ready
        delay(100); // brief settle
        scanNetworks();
    } else {
        Serial.println("[WiFi] Failed to start AP!");
        _state = INITIALIZING;
    }
}

void WiFiManager::_attemptConnection() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(_savedSSID.c_str(), _savedPassword.c_str());
    _state               = CONNECTING;
    _connectionAttemptMs = millis();
    Serial.printf("[WiFi] Connecting to: %s\n", _savedSSID.c_str());
}

void WiFiManager::_handleState() {
    switch (_state) {
        case CONNECTING: {
            if (WiFi.status() == WL_CONNECTED) {
                _state = CONNECTED;
                Serial.printf("[WiFi] Connected! IP: %s, RSSI: %d dBm\n",
                              WiFi.localIP().toString().c_str(), WiFi.RSSI());
            } else if (millis() - _connectionAttemptMs > WIFI_CONNECTION_TIMEOUT_MS) {
                Serial.printf("[WiFi] Connection timeout after %d ms — falling back to AP mode\n",
                              WIFI_CONNECTION_TIMEOUT_MS);
                _startAPMode();
            }
            break;
        }

        case CONNECTED: {
            if (WiFi.status() != WL_CONNECTED) {
                _state = DISCONNECTED;
                Serial.println("[WiFi] Connection lost!");
            }
            break;
        }

        case DISCONNECTED: {
            // Retry connection after interval
            if (millis() - _connectionAttemptMs > WIFI_RECONNECT_INTERVAL_MS) {
                Serial.println("[WiFi] Retrying connection...");
                _attemptConnection();
            }
            break;
        }

        case AP_MODE:
            // Periodic re-scan of available networks
            if (millis() - _lastScanMs > WIFI_SCAN_INTERVAL_MS) {
                scanNetworks();
            }
            break;

        default:
            break;
    }
}
