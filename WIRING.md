# Wiring Diagram – ESP32 DC Motor Controller

## Component Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                        24 V DC Input                            │
│                             │                                   │
│              ┌──────────────┴──────────────┐                   │
│              │         Motor Load           │                   │
│              │  (DC Motor, 0–20 A)         │                   │
│         ┌────┴────┐                    ┌───┴───┐               │
│         │ ACS712  │                    │ Relay  │               │
│         │  30A    │                    │ Module │               │
│         └────┬────┘                    └───┬───┘               │
│              │ VOUT (analog)          IN ──┤                    │
│              └──────────────┬─────────────┘                    │
│                             │                                   │
│                       ┌─────┴─────┐                            │
│                       │   ESP32   │                             │
│                       │ DevKit V1 │                             │
│                       └───────────┘                            │
└─────────────────────────────────────────────────────────────────┘
```

---

## 1. ESP32 Pinout Reference

```
                    ESP32 DevKit V1
                  ┌─────────────────┐
               3V3│1             38│GND
               GND│2             37│GPIO23
               GND│3             36│GPIO22  ← I2C SCL
         3V3 / VCC│4             35│GPIO01  (TX0)
              GPIO│5  (EN)       34│GPIO03  (RX0)
             GPIO0│6  (BOOT)     33│GPIO21  ← I2C SDA
            GPIO36│7  (SVP)      32│GPIO19
            GPIO39│8  (SVN)      31│GPIO18
            GPIO34│9  ← ACS712   30│GPIO5
            GPIO35│10            29│GPIO17
            GPIO32│11            28│GPIO16
            GPIO33│12            27│GPIO4
            GPIO25│13            26│GPIO0
            GPIO26│14 ← RELAY    25│GPIO2   ← LED
            GPIO27│15 ← BUTTON   24│GPIO15
            GPIO14│16            23│GPIO8
            GPIO12│17            22│GPIO7
               GND│18            21│GPIO6
            GPIO13│19            20│GPIO11
                  └─────────────────┘
```

---

## 2. ACS712-30A Current Sensor

The ACS712-30A module has three connections:

| ACS712 Pin | Connect To | Notes |
|---|---|---|
| VCC | 5 V (external) | Use regulated 5 V supply |
| GND | Common GND | Share with ESP32 GND |
| VOUT | Voltage divider → GPIO 34 | See note below |

### Voltage Divider (required when ACS712 runs on 5 V)

The ACS712 outputs 0 – 5 V; ESP32 ADC tolerates max 3.3 V.
A resistor divider scales the output down:

```
VOUT ──┬── R1 (10 kΩ) ──┬── R2 (20 kΩ) ── GND
       │                 │
       │               GPIO34
       │           (ADC input)
```

Divider ratio: R2 / (R1 + R2) = 20 k / 30 k = 0.667
Maximum voltage at GPIO34: 5 V × 0.667 = 3.33 V ≈ 3.3 V ✓

Update `config.h`:
```cpp
#define ACS712_DIVIDER_RATIO  0.667f
#define ACS712_ZERO_OFFSET_V  2.5f    // sensor midpoint at 5 V supply
```

**Alternative:** Power ACS712 from 3.3 V (some modules support this):
```cpp
#define ACS712_DIVIDER_RATIO  1.0f    // no divider needed
#define ACS712_ZERO_OFFSET_V  1.65f   // midpoint at 3.3 V supply
```

Add decoupling capacitors close to the ACS712:
- 100 nF ceramic between VCC and GND
- 10 µF electrolytic between VCC and GND

---

## 3. Relay Module

Use a 5 V opto-isolated relay module rated ≥ 24 V / 30 A on the load side.

| Relay Module Pin | Connect To | Notes |
|---|---|---|
| VCC | 5 V | Relay coil power |
| GND | Common GND | |
| IN (signal) | GPIO 26 (ESP32) | HIGH = relay ON |
| COM (load) | 24 V+ (motor supply) | |
| NO (Normally Open) | Motor terminal A | Circuit opens on trip |
| NC (Normally Closed) | (not used) | |

> **Note:** The motor return (negative) connects directly from the motor terminal B back to the 24 V supply negative.

---

## 4. Push Button

```
ESP32 GPIO 27 ──┬── Button ── GND
                │
            (internal PULLUP enabled in firmware)
```

When button is pressed, GPIO27 reads LOW.
Firmware enables relay and clears any over-current fault.

Use a 100 nF capacitor between GPIO27 and GND as hardware debounce (optional – firmware handles 50 ms software debounce).

---

## 5. OLED SSD1306 128×64 I2C

| OLED Pin | Connect To |
|---|---|
| VCC | 3.3 V or 5 V (check module) |
| GND | GND |
| SDA | GPIO 21 (ESP32) |
| SCL | GPIO 22 (ESP32) |

I2C address: **0x3C** (or 0x3D – check your module).
Update `config.h` if needed: `#define OLED_I2C_ADDRESS 0x3C`

---

## 6. LCD 1602 I2C (PCF8574 backpack)

| LCD Pin | Connect To |
|---|---|
| VCC | 5 V |
| GND | GND |
| SDA | GPIO 21 (ESP32) |
| SCL | GPIO 22 (ESP32) |

I2C address: **0x27** (or 0x3F – check your module).
Update `config.h` if needed:
```cpp
#define DISPLAY_TYPE     1       // 1 = LCD 1602
#define LCD_I2C_ADDRESS  0x27
```

---

## 7. Power Supply Diagram

```
  24 V DC Supply
       │
       ├── (+) 24 V ──────────── Relay COM  ─── Motor(+)
       │                                            │
       │                         Relay NO  ─────── (motor circuit)
       │
       └── (-) GND ──────────── Common GND
                                     │
                              5 V Buck Converter
                                  │       │
                                 5V      GND
                                  │
                        ┌─────────┴──────────┐
                        │   ACS712 VCC       │
                        │   Relay VCC        │
                        │   LCD VCC          │
                        └─────────┬──────────┘
                                  │
                             3.3 V LDO (on ESP32)
                                  │
                        ┌─────────┴──────────┐
                        │   ESP32 3V3        │
                        │   OLED VCC         │
                        └────────────────────┘
```

---

## 8. Safety Notes

1. **Current rating**: ACS712-30A is rated for 30 A continuous. Use 20 A as operating limit.
2. **Relay contacts**: Use relay rated for ≥ 30 A / 24 V DC. Add a flyback diode on the coil side.
3. **Wire gauge**: Use at least 2.5 mm² (AWG 13) wire on the 24 V / motor side for 20 A.
4. **Fusing**: Install a 25 A automotive fuse on the 24 V line.
5. **Earthing**: Ground the motor chassis and relay enclosure.
6. **Isolation**: Keep the 24 V power side physically separated from the ESP32 signal wiring.
7. **ADC accuracy**: ESP32 ADC has non-linearity near the rails. Avoid readings below 100 mV or above 3.0 V for best accuracy.
