# Bill of Materials – ESP32 DC Motor Controller

## Core Components

| # | Component | Specification | Qty | Notes |
|---|-----------|---------------|-----|-------|
| 1 | ESP32 DevKit V1 | 240 MHz, 4 MB Flash, 38-pin | 1 | Any ESP32 module works |
| 2 | ACS712-30A Module | Hall-effect current sensor, 30 A, 100 mV/A | 1 | **Use 30A variant** for 0–20 A operation |
| 3 | Relay Module | 5 V coil, 30 A / 250 VAC or 30 A / 28 VDC contacts, opto-isolated | 1 | Opto-isolated preferred |
| 4 | OLED Display | SSD1306 128×64, I2C, 3.3 V / 5 V | 1 | **Option A** – or use LCD below |
| 5 | LCD 1602 I2C | 16×2 character, PCF8574 backpack, I2C | 1 | **Option B** – alternative to OLED |
| 6 | Push Button | Tactile switch, 6×6 mm, through-hole | 1 | Momentary NO |
| 7 | ESP32 Power Supply | 5 V / 2 A (buck converter from 24 V) | 1 | E.g., LM2596 or MP1584 module |

---

## Passive Components

| # | Component | Value | Qty | Purpose |
|---|-----------|-------|-----|---------|
| 8  | Resistor | 10 kΩ, 1/4 W | 1 | Voltage divider R1 (ACS712 VOUT → ADC) |
| 9  | Resistor | 20 kΩ, 1/4 W | 1 | Voltage divider R2 (ACS712 VOUT → ADC) |
| 10 | Capacitor | 100 nF ceramic (0.1 µF) | 2 | Decoupling (ACS712 supply, ADC input) |
| 11 | Capacitor | 10 µF electrolytic | 1 | Bulk decoupling on ACS712 supply |
| 12 | Capacitor | 100 nF ceramic | 1 | Button hardware debounce (optional) |

---

## Wiring & Connectors

| # | Component | Specification | Qty |
|---|-----------|---------------|-----|
| 13 | Wire (power) | 2.5 mm² / AWG 13, red & black | 50 cm |
| 14 | Wire (signal) | 0.5 mm² / AWG 20, multi-colour | 1 m |
| 15 | Automotive Fuse + holder | 25 A blade fuse | 1 |
| 16 | Terminal block | 2-pin screw terminal, 5 mm pitch, 30 A | 2 |
| 17 | Dupont connectors | 2.54 mm pitch | assorted |
| 18 | PCB breadboard | 170-point or 400-point | 1 |

---

## Optional / Recommended

| # | Component | Notes |
|---|-----------|-------|
| 19 | TVS Diode (SMBJ30A) | Transient protection across motor terminals |
| 20 | Snubber (RC) | 100 Ω + 100 nF in series, across relay contacts |
| 21 | Heat shrink tubing | Insulate solder joints on power lines |
| 22 | DIN rail enclosure | IP54-rated box for relay and motor connections |
| 23 | Flyback diode (1N4007) | Across relay coil pins to protect driver |

---

## Tools Required

- Soldering iron + solder
- Multimeter (for continuity and voltage checks)
- Wire strippers / crimping tool
- USB cable (micro-USB or USB-C depending on ESP32 module)
- Computer with Arduino IDE or PlatformIO

---

## Approximate Cost

| Section | Estimated Cost (USD) |
|---|---|
| ESP32 DevKit V1 | $4–8 |
| ACS712-30A module | $2–4 |
| Relay module (opto-isolated) | $2–5 |
| OLED SSD1306 128×64 | $3–6 |
| 5 V buck converter module | $1–3 |
| Push button | < $1 |
| Passives (R, C) | < $1 |
| Wire + connectors + fuse | $3–5 |
| **Total** | **~$16–33** |

---

## Datasheet References

- ACS712 datasheet: https://www.allegromicro.com/~/media/Files/Datasheets/ACS712-Datasheet.ashx
- ESP32 technical reference: https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf
- SSD1306 OLED datasheet: https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
