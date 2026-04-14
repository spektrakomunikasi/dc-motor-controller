# DC Motor Controller

Adjustable-current DC motor controller for ESP32 with:
- Real-time ACS712-30A current sensing
- Overcurrent protection (1–30 A, adjustable)
- Soft-start ramp (1–10 s, adjustable)
- WiFi AP setup mode (no hardcoded credentials)
- OLED status display (128×64 SSD1306)
- Web dashboard for monitoring and control

---

## Quick Start

### 1. Install Toolchain
- Install [PlatformIO](https://platformio.org/) (VS Code extension or CLI)

### 2. Clone & Build
```bash
git clone <repo-url>
cd dc-motor-controller
pio run
```

### 3. Upload Firmware & Filesystem
```bash
# Upload firmware
pio run --target upload

# Upload SPIFFS web files (do this once, or whenever data/ changes)
pio run --target uploadfs
```

### 4. First Boot — WiFi Setup

On first boot (no saved credentials), the ESP32 creates an access point:

| Setting  | Value                             |
|----------|-----------------------------------|
| SSID     | `MOTOR_CTRL_XXXXXXXX` (MAC-based) |
| Password | `12345678`                        |
| IP       | `192.168.4.1`                     |

1. Connect your phone or laptop to the `MOTOR_CTRL_...` WiFi network.
2. Open a browser and go to **http://192.168.4.1/setup.html**
3. Select your home/office WiFi network and enter the password.
4. Click **Save & Reboot**.
5. The device reboots and connects to your WiFi. The new IP is shown on the OLED.

### 5. Motor Dashboard

Once connected to your WiFi, open the IP shown on the OLED in a browser:

```
http://<device-ip>/
```

Features:
- Live current gauge and 60-second history graph
- Start / Stop / Reset buttons
- Current limit slider (1–30 A)
- Soft-start duration slider (1–10 s)
- WiFi status and RSSI

---

## WiFi Setup Anytime

Navigate to `http://<device-ip>/setup.html` to reconfigure WiFi or clear credentials.

---

## API Reference

### Motor Control (normal mode)

| Method | Endpoint                | Description               |
|--------|-------------------------|---------------------------|
| GET    | `/api/status`           | Full status JSON          |
| POST   | `/api/motor/start`      | Start motor               |
| POST   | `/api/motor/stop`       | Stop motor                |
| POST   | `/api/motor/reset`      | Reset after fault         |
| POST   | `/api/motor/settings`   | Set `current_limit`, `soft_start_ms` |

### WiFi Setup (AP mode + normal mode)

| Method | Endpoint                | Description               |
|--------|-------------------------|---------------------------|
| GET    | `/api/setup/status`     | WiFi state, IP, AP name   |
| GET    | `/api/setup/networks`   | Scanned network list      |
| POST   | `/api/setup/wifi`       | Save credentials & reboot |
| POST   | `/api/setup/reboot`     | Reboot device             |
| POST   | `/api/setup/clear`      | Clear credentials & reboot|

---

## Hardware

See [docs/WIRING.md](docs/WIRING.md) for detailed wiring instructions and schematic.

### Pin Summary

| GPIO | Function                        |
|------|---------------------------------|
| 12   | Relay 1 — motor cutoff          |
| 13   | Relay 2 — soft-start bypass     |
| 14   | Push button (START/STOP)        |
| 34   | ACS712-30A VOUT (ADC)           |
| 21   | OLED SDA (I2C)                  |
| 22   | OLED SCL (I2C)                  |

---

## OLED Display States

```
AP MODE:                     CONNECTED:
┌──────────────────────┐     ┌──────────────────────┐
│ MOTOR CTRL v1.0      │     │ MOTOR CTRL v1.0      │
├──────────────────────┤     ├──────────────────────┤
│ WiFi: AP MODE        │     │ WiFi: ***            │
│ SSID: MOTOR_CTRL_123 │     │ SSID: MyWiFi         │
│ IP: 192.168.4.1      │     │ IP: 192.168.1.100    │
├──────────────────────┤     ├──────────────────────┤
│ Visit http://        │     │ Cur:12.45A Lim:15A   │
│ 192.168.4.1/setup.h  │     │ Motor: RUNNING       │
└──────────────────────┘     └──────────────────────┘
```

---

## License

MIT
