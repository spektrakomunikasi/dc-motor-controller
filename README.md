# ESP32 DC Motor Controller with ACS712-30A

A production-ready firmware for an ESP32-based DC motor control system featuring real-time current monitoring, automatic relay protection, and a web-based dashboard.

## Overview

| Feature | Detail |
|---|---|
| Microcontroller | ESP32 DevKit V1 |
| Current Sensor | ACS712-30A (100 mV/A, 2.5 V zero offset) |
| Operating Range | 0 – 20 A (adjustable) |
| Input Voltage | 24 V DC |
| Display | OLED SSD1306 128×64 **or** LCD 1602 I2C (selectable) |
| User Input | Push button (manual motor restart) |
| Web Interface | AsyncWebServer on port 80 |
| Settings Storage | EEPROM (threshold persisted across resets) |

---

## File Structure

```
dc-motor-controller/
├── README.md
├── WIRING.md
├── platformio.ini
├── src/
│   ├── main.cpp               # Setup + main loop
│   ├── config.h               # All pin/WiFi/threshold settings
│   ├── motor_controller.cpp   # ACS712 reading, relay, button, EEPROM
│   ├── motor_controller.h
│   ├── display.cpp            # OLED / LCD driver
│   ├── display.h
│   ├── web_server.cpp         # Async REST API + static file serving
│   └── web_server.h
├── data/
│   └── index.html             # Real-time web dashboard
└── docs/
    └── COMPONENTS.md          # Bill of materials
```

---

## Hardware Requirements

- ESP32 DevKit V1 (or compatible)
- ACS712-30A current sensor module
- 5 V relay module (24 V coil compatible or use opto-isolated board)
- OLED SSD1306 128×64 I2C **or** LCD 1602 I2C
- Tactile push button
- 5 V / 3.3 V power supply (for logic)
- 24 V DC power supply (for motor)
- Decoupling capacitors (100 nF + 10 µF on ACS712 supply)

See [docs/COMPONENTS.md](docs/COMPONENTS.md) for the full bill of materials and [WIRING.md](WIRING.md) for wiring diagrams.

---

## ACS712-30A Calibration

```
Conversion: Current (A) = (V_sensor − V_offset) / Sensitivity
  V_offset    = 2.5 V  (at 5 V supply) / 1.65 V (at 3.3 V supply)
  Sensitivity = 0.1 V/A  (100 mV/A)
  ADC         = 12-bit (0–4095 counts), V_ref = 3.3 V
```

If the ACS712 is powered from 5 V and the ESP32 ADC only accepts ≤ 3.3 V,
you **must** add a voltage divider on the VOUT line. See WIRING.md.

---

## Software Installation

### PlatformIO (recommended)

1. Install [VS Code](https://code.visualstudio.com/) and the [PlatformIO](https://platformio.org/) extension.
2. Clone this repository:
   ```bash
   git clone https://github.com/spektrakomunikasi/dc-motor-controller.git
   cd dc-motor-controller
   ```
3. Open the folder in VS Code – PlatformIO detects `platformio.ini` automatically.

### Arduino IDE

1. Install board support: **File → Preferences → Additional Boards Manager URLs** → add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
2. Install libraries via **Sketch → Include Library → Manage Libraries**:
   - ESPAsyncWebServer (esphome fork)
   - ArduinoJson (≥ 7.x)
   - Adafruit SSD1306
   - Adafruit GFX Library
   - LiquidCrystal I2C (marcoschwartz)
3. Copy the `src/` files into a single `.ino` project folder.

---

## Configuration

Edit **`src/config.h`** before compiling:

```cpp
// WiFi credentials
#define WIFI_SSID      "YourNetworkName"
#define WIFI_PASSWORD  "YourPassword"

// GPIO pin assignments (change to match your wiring)
#define RELAY_PIN           26
#define BUTTON_PIN          27
#define CURRENT_SENSOR_PIN  34
#define I2C_SDA_PIN         21
#define I2C_SCL_PIN         22

// Display type: 0 = OLED SSD1306, 1 = LCD 1602 I2C
#define DISPLAY_TYPE        0

// Default over-current threshold (Amps)
#define DEFAULT_THRESHOLD_A  10.0f

// ACS712 supply voltage offset (2.5 V for 5 V supply, 1.65 V for 3.3 V)
#define ACS712_ZERO_OFFSET_V  2.5f
```

---

## Uploading Firmware

### PlatformIO

```bash
# Compile and upload firmware
pio run --target upload

# Upload web dashboard (SPIFFS)
pio run --target uploadfs

# Open serial monitor
pio device monitor --baud 115200
```

### Arduino IDE

1. Select **Tools → Board → ESP32 Dev Module**.
2. Select the correct COM port.
3. Click **Upload**.
4. For SPIFFS: use the **ESP32 Sketch Data Upload** plugin to upload `data/index.html`.

---

## Web Dashboard

After connecting to WiFi, the ESP32 will print its IP address to the serial monitor:

```
[WIFI] Connected! IP: 192.168.1.105
[MDNS] Registered as http://motor-ctrl.local
```

Open a browser and navigate to:
- `http://192.168.1.105` (IP address), or
- `http://motor-ctrl.local` (mDNS, most platforms)

### REST API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| `GET`  | `/api/status` | Current reading, relay state, fault, RSSI |
| `POST` | `/api/threshold` | Set over-current threshold (0.5 – 20 A) |
| `POST` | `/api/relay` | Manual relay ON / OFF |

**GET /api/status** example response:
```json
{
  "current":   12.34,
  "relay":     true,
  "running":   true,
  "threshold": 15.0,
  "faulted":   false,
  "rssi":      -65,
  "ip":        "192.168.1.105"
}
```

**POST /api/threshold** request body:
```json
{ "threshold": 12.5 }
```

**POST /api/relay** request body:
```json
{ "relay": true }
```

---

## Operation

1. Power on → ESP32 connects to WiFi, display shows "Booting…"
2. Motor relay is **OFF** at startup.
3. Press the **push button** to enable the relay (motor starts).
4. Current is monitored continuously:
   - When `current ≥ threshold` → relay trips, motor stops, fault LED blinks.
   - Press button **or** click **Relay ON** in dashboard to reset and restart.
5. Adjust the threshold via the slider in the dashboard and click **Set Threshold**.
6. Settings are saved to EEPROM and survive power cycles.

---

## Safety Notes

- The ACS712-30A hall-effect sensor is **galvanically isolated** from the ESP32.
- Always use an **opto-isolated relay module** to protect the ESP32 from switching transients.
- Add a **TVS diode** and **snubber** (RC network) across relay contacts for inductive loads.
- The firmware uses a **hysteresis** (0.5 A by default) to prevent relay chatter near the threshold.
- The push button has **50 ms software debounce**.
- All threshold changes are persisted to EEPROM with a magic-word check to detect corruption.

---

## Troubleshooting

| Symptom | Cause / Fix |
|---|---|
| Display not working | Check I2C address (0x3C / 0x27) and SDA/SCL wiring |
| Current always reads 0 | ACS712 VOUT < ADC threshold – check voltage divider |
| Current reads negative | Reverse ACS712 orientation or adjust `ACS712_ZERO_OFFSET_V` |
| WiFi won't connect | Check SSID/password in `config.h`; ESP32 only supports 2.4 GHz |
| Dashboard not loading | Ensure SPIFFS is uploaded (`pio run --target uploadfs`) |
| Relay chatters | Increase `THRESHOLD_HYSTERESIS_A` in `config.h` |

---

## License

MIT License – see [LICENSE](LICENSE) for details.
