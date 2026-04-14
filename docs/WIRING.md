# DC Motor Controller — Wiring Guide

## Components

| Component         | Purpose                        |
|-------------------|--------------------------------|
| ESP32 Dev Board   | Main controller                |
| ACS712-30A Module | Current sensing (0–30 A)       |
| LM2596 Module     | Step-down converter (24V → 5V) |
| SSD1306 OLED      | 128×64 I2C display             |
| 2× Relay Module   | Motor switching                |
| 2× NPN Transistor | Relay driver (e.g. BC547)      |
| Push Button       | Manual start/stop              |

---

## Power Supply

```
24V DC Input
    ↓
[LM2596 Buck Module]  (built-in capacitors)
    ├─ OUT+ → 5V
    └─ OUT- → GND

5V rail feeds:
    ├─ ESP32 VIN
    ├─ ACS712 VCC
    └─ OLED VCC (via I2C pull-up resistors)
```

---

## Pin Mapping

```
ESP32 GPIO  │ Connected To
────────────┼──────────────────────────────────────
GPIO 12     │ Relay 1 base (via 1 kΩ resistor + NPN) — Motor cutoff
GPIO 13     │ Relay 2 base (via 1 kΩ resistor + NPN) — Soft-start bypass
GPIO 14     │ Push button (other end → GND, INPUT_PULLUP)
GPIO 34     │ ACS712-30A VOUT (ADC input — do NOT exceed 3.3V!)
GPIO 21     │ OLED SDA (I2C)
GPIO 22     │ OLED SCL (I2C)
3V3         │ I2C pull-up resistors (4.7 kΩ to SDA & SCL)
GND         │ Common ground
```

> ⚠️ **Important**: GPIO 34 is ADC input-only (no internal pull-up).  
> ACS712 output is 0–5 V. Use a 2:3 voltage divider to limit to 3.3 V max.  
> Voltage divider: R1 = 20 kΩ (to VOUT), R2 = 10 kΩ (to GND) → ADC reads R2 voltage.

---

## Relay Driver Circuit (per relay)

```
ESP32 GPIO ──[1 kΩ]──┬── NPN Base (BC547)
                      │
                     GND
                     
NPN Emitter → GND  
NPN Collector → Relay coil (−)  
Relay coil (+) → 5V  
Flyback diode (1N4007) across relay coil (cathode to +)
```

---

## ACS712-30A Voltage Divider

```
ACS712 VOUT ──[20 kΩ]──┬──[10 kΩ]── GND
                         │
                      GPIO 34 (ADC)
                      
Output range: 0–3.3 V (safe for ESP32)
```

---

## OLED I2C (SSD1306 128×64)

```
OLED VCC  → 3V3
OLED GND  → GND
OLED SDA  → GPIO 21 (+ 4.7 kΩ pull-up to 3V3)
OLED SCL  → GPIO 22 (+ 4.7 kΩ pull-up to 3V3)
I2C Address: 0x3C
```

---

## Motor Control Logic

| State         | Relay 1 (GPIO 12) | Relay 2 (GPIO 13) | Description                  |
|---------------|:-----------------:|:-----------------:|------------------------------|
| STOPPED       | OFF               | OFF               | Motor disconnected           |
| SOFT_STARTING | ON                | OFF               | Motor ramping up (with R)    |
| RUNNING       | ON                | ON                | Full speed (bypass R)        |
| OVERCURRENT   | OFF               | OFF               | Protection tripped           |

> Relay 2 bypasses the soft-start resistor after the ramp-up period expires.

---

## Default Configuration

| Parameter            | Default  | Range      |
|----------------------|----------|------------|
| Current Limit        | 15.0 A   | 1–30 A     |
| Soft-Start Duration  | 3000 ms  | 1–10 000 ms|
| WiFi AP Password     | 12345678 | fixed      |
| WiFi Connect Timeout | 15 s     | fixed      |
