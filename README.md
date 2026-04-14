# DC Motor Controller — ESP32 with ACS712-30A

Adjustable-current DC motor controller with ESP32, ACS712-30A sensor, soft-start via NTC thermistor, OLED status display, and a responsive web dashboard served directly from the ESP32.

---

## Features

| Feature | Detail |
|---|---|
| **Current Sensing** | ACS712-30A, EMA-filtered, reads every 100 ms |
| **Soft Start** | 1–10 s adjustable ramp-up through NTC thermistor (R5) |
| **Overcurrent Protection** | Configurable threshold (1–30 A), 0.5 A hysteresis |
| **Web Dashboard** | Async HTTP server, live gauge, sliders, current graph |
| **OLED Display** | 128×64 SSD1306, real-time current / state / relay status |
| **Settings Persistence** | Stored in ESP32 NVS (survives power cycle) |
| **Push Button** | Debounced (50 ms) Start / Restart from STOPPED / OVERCURRENT state |

---

## Hardware

See [WIRING.md](WIRING.md) for full wiring diagram and [docs/COMPONENTS.md](docs/COMPONENTS.md) for the bill of materials.

---

## Quick-Start

### 1. Prerequisites

- [PlatformIO](https://platformio.org/) CLI or IDE extension
- ESP32 DevKit V1 board
- USB cable for flashing

### 2. Clone & Configure

```bash
git clone https://github.com/spektrakomunikasi/dc-motor-controller.git
cd dc-motor-controller
```

Open `src/config.h` and update the WiFi credentials:

```cpp
#define WIFI_SSID     "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASSWORD"
```

### 3. Build & Upload

```bash
# Build firmware
pio run

# Upload firmware
pio run --target upload

# Upload web dashboard (SPIFFS)
pio run --target uploadfs

# Monitor serial output
pio device monitor --baud 115200
```

### 4. Find ESP32 IP Address

After boot the serial monitor will print:

```
[WiFi] Connected! IP: 192.168.x.x
[WEB] Dashboard: http://192.168.x.x
```

The OLED will also show the WiFi connection status.

### 5. Open Web Dashboard

Navigate to `http://<ESP32-IP>` in any browser.

---

## Web API Endpoints

| Method | Path | Description |
|---|---|---|
| `GET` | `/api/status` | JSON snapshot of all sensor and state data |
| `POST` | `/api/threshold` | `{"value": 15.5}` — Set overcurrent threshold (A) |
| `POST` | `/api/soft-start-time` | `{"value": 7}` — Set soft-start duration (s) |
| `POST` | `/api/relay/motor` | `{"state": true}` — Start (true) or Stop (false) motor |
| `POST` | `/api/restart` | Reboot ESP32 |
| `GET` | `/` | Serve web dashboard (`data/index.html` from SPIFFS) |

---

## OLED Display Layout

```
MOTOR CONTROL v1.0
--------------------
Current: 12.45 A
Limit:   15.0 A
Motor:   RAMP 45%
Relay1: [ON]  R2:[OFF]
WiFi:  *** OK
```

States shown: `STOPPED`, `RAMP X%`, `RUNNING`, `OVERCURRENT` (flashing).

---

## Motor Control Logic

```
Push button (GPIO 14) pressed
  │
  ▼
GPIO 12 → HIGH  (Relay 1 ON — main motor enabled)
GPIO 13 → LOW   (Relay 2 OFF — current flows via NTC R5)
  │
  ▼ Soft-start ramp (1–10 s, adjustable)
  │   Monitor ACS712 continuously
  │   If current ≥ threshold → GPIO 12 LOW (emergency cutoff)
  │
  ▼ Ramp complete
GPIO 13 → HIGH  (Relay 2 ON — NTC bypassed)
  │
  ▼ Running
  │   If current ≥ threshold → GPIO 12 LOW (cutoff)
  │
  ▼ OVERCURRENT state
    Press button to restart sequence
```

---

## Configuration Reference (`src/config.h`)

| Constant | Default | Description |
|---|---|---|
| `RELAY_CUTOFF_PIN` | GPIO 12 | Relay 1 — motor ON/OFF |
| `RELAY_SOFTSTART_PIN` | GPIO 13 | Relay 2 — NTC bypass |
| `PUSH_BUTTON_PIN` | GPIO 14 | Start/Restart button |
| `ACS712_PIN` | GPIO 34 | ACS712-30A ADC input |
| `I2C_SDA` | GPIO 21 | OLED SDA |
| `I2C_SCL` | GPIO 22 | OLED SCL |
| `SOFT_START_DEFAULT_TIME` | 5 s | Default ramp-up duration |
| `CURRENT_DEFAULT` | 15.0 A | Default overcurrent threshold |
| `CURRENT_HYSTERESIS` | 0.5 A | Relay chatter prevention |
| `ACS712_SENSITIVITY` | 100 mV/A | ACS712-30A spec |
| `ACS712_ZERO_OFFSET_MV` | 2500 mV | Zero-current output at 5 V supply |

---

## License

MIT
