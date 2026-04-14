# Wiring Diagram — DC Motor Controller

## CONFIRMED Final Pin Mapping

```
ESP32 DevKit V1
┌──────────────────────────────────────────────────────────────┐
│                                                              │
│  GPIO 12 (D12) ──[R 1kΩ]── Base NPN Q1 ── Relay 1 coil     │
│                                            (Motor Cutoff)   │
│                                            LED1 ∥ coil      │
│                                                              │
│  GPIO 13 (D13) ──[R 1kΩ]── Base NPN Q2 ── Relay 2 coil     │
│                                            (Soft Start)     │
│                                            LED2 ∥ coil      │
│                                                              │
│  GPIO 14 (D14) ──[Push Button]── GND      (INPUT_PULLUP)    │
│                                                              │
│  GPIO 34 (A6)  ──────────────── ACS712-30A VOUT             │
│                                                              │
│  GPIO 21 (D21) ──────────────── OLED SDA  (I2C)             │
│  GPIO 22 (D22) ──────────────── OLED SCL  (I2C)             │
│                                                              │
│  VIN (5V)      ──────────────── LM2596 OUT+                 │
│                                 ACS712 VCC (5V)             │
│                                 OLED VCC   (3.3V or 5V)     │
│  GND           ──────────────── Common GND                  │
└──────────────────────────────────────────────────────────────┘
```

---

## Power Supply

```
24V DC Input
  │
  ├──[470μF 50V cap — built-in on LM2596 module]
  │
  ▼
LM2596 Buck Converter Module
  ├─ IN+  → 24V DC
  ├─ IN−  → GND
  ├─ OUT+ → 5V (trimmer adjusted)   ──→ ESP32 VIN
  │                                  ──→ ACS712-30A VCC
  │                                  ──→ OLED VCC
  └─ OUT− → GND
  │
  └──[100μF 25V cap — built-in on LM2596 module]
```

> **Note:** Measure LM2596 output with a multimeter and adjust the trimmer pot until output = **5.0 V** before connecting to ESP32.

---

## Relay Driver (per relay)

```
GPIO 12/13
   │
[R 1kΩ]
   │
   ├──→ Base  NPN transistor (e.g. BC547, 2N2222)
   │
GND ──→ Emitter

Collector ──→ Relay coil (−)
Relay coil (+) ──→ 5V

Flyback diode (1N4007) across relay coil: Anode→Collector, Cathode→5V

LED indicator wired in parallel with relay coil (with series resistor 470Ω)
```

---

## ACS712-30A Connection

```
ACS712-30A Module
  ├─ VCC  → 5V
  ├─ GND  → GND
  └─ VOUT → GPIO 34 (ADC1_CH6)

Motor power path:
  24V DC → IP+ (ACS712 terminal 1) → Motor Load → IP− (ACS712 terminal 2) → GND
```

> **Important:** At zero current VOUT = 2.5 V, sensitivity = 100 mV/A.
> The ACS712 VOUT is connected directly to GPIO 34 (ESP32 ADC range: 0–3.3 V).
> The ADC can reliably measure voltages from 0 V up to ~3.1 V, so the usable
> current range is approximately 0–6 A in the positive direction before clipping.
> For the full 0–30 A range, add a voltage divider (10 kΩ / 10 kΩ) to scale
> 0–5 V → 0–2.5 V at GPIO 34, then update `ACS712_ZERO_OFFSET_MV` to 1250 in `config.h`.

---

## OLED SSD1306 128×64

```
OLED Module
  ├─ VCC → 3.3V (or 5V if module has onboard regulator)
  ├─ GND → GND
  ├─ SDA → GPIO 21
  └─ SCL → GPIO 22

I2C address: 0x3C (default) — adjust OLED_ADDR in config.h if different
```

---

## Push Button

```
GPIO 14 ──┐
          [Push Button]
          │
         GND

Internal pull-up enabled (INPUT_PULLUP).
Button pressed → GPIO 14 reads LOW → Start/Restart motor sequence.
```

---

## Soft Start NTC Circuit

```
Relay 2 contact (Normally Open):
  Motor supply line → [NTC Thermistor R5] → Motor

When Relay 2 is OFF:  current flows through NTC (current limiting during ramp-up)
When Relay 2 is ON:   NTC bypassed — motor runs at full power
```

---

## Summary Table

| Signal | ESP32 Pin | Direction | Notes |
|---|---|---|---|
| Relay 1 (Motor Cutoff) | GPIO 12 | OUTPUT | Active HIGH via NPN + 1kΩ base resistor |
| Relay 2 (Soft Start) | GPIO 13 | OUTPUT | Active HIGH via NPN + 1kΩ base resistor |
| Push Button | GPIO 14 | INPUT_PULLUP | Active LOW |
| ACS712 VOUT | GPIO 34 | ADC Input | 0–3.3 V range |
| OLED SDA | GPIO 21 | I2C | 400 kHz |
| OLED SCL | GPIO 22 | I2C | 400 kHz |
| 5V Power | VIN | Power | From LM2596 |
| GND | GND | Power | Common ground |
