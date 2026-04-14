# Bill of Materials

## DC Motor Controller — ESP32 with ACS712-30A

| # | Component | Value / Part No. | Qty | Notes |
|---|---|---|---|---|
| 1 | Microcontroller | ESP32 DevKit V1 (38-pin) | 1 | Main controller |
| 2 | Current Sensor | ACS712-30A Module | 1 | 100 mV/A, ±30 A |
| 3 | OLED Display | SSD1306 128×64 I2C | 1 | 0.96″, I2C address 0x3C |
| 4 | Buck Converter | LM2596 Module (24V→5V) | 1 | Built-in 470μF in, 100μF out caps |
| 5 | Relay Module | 5V Relay × 2 | 2 | Rated for motor load voltage/current |
| 6 | NPN Transistor | BC547 or 2N2222 | 2 | Relay coil driver |
| 7 | Resistor | 1 kΩ 1/4 W | 2 | Transistor base resistors (Q1, Q2) |
| 8 | Resistor | 470 Ω 1/4 W | 2 | LED indicator series resistors |
| 9 | LED | 5 mm Red/Green | 2 | Relay status indicators |
| 10 | Flyback Diode | 1N4007 | 2 | Relay coil protection |
| 11 | NTC Thermistor | 5–10 Ω (R5) | 1 | Soft-start inrush current limiter |
| 12 | Push Button | Momentary NO | 1 | Start / Restart |
| 13 | DC Jack / Terminal | 24V DC input | 1 | Power input connector |
| 14 | Enclosure | ABS / DIN rail | 1 | Optional |

---

## Notes

### LM2596 Module
- Input range: 4.5–40 V DC
- Output: adjustable 1.23–37 V; set to **5.0 V** via trimmer pot
- Built-in capacitors: 470 μF 50 V (input), 100 μF 25 V (output) — no additional caps needed
- Efficiency: ~85–92% at 24 V → 5 V conversion
- Max output current: 3 A (sufficient for ESP32 + ACS712 + OLED ≈ 300 mA total)

### ACS712-30A
- Supply voltage: 5 V DC
- Output at zero current: 2.5 V
- Sensitivity: 100 mV/A
- Current measurement range: ±30 A
- Connect motor power wires through IP+/IP− terminals

### Relay Modules
- Coil voltage: 5 V
- Drive via NPN transistor (BC547/2N2222) with 1 kΩ base resistor from ESP32 GPIO
- LED indicator wired in parallel with relay coil
- Contact rating must exceed motor operating voltage and stall current

### ESP32 DevKit V1
- GPIO 34 is input-only (no pull-up/down, no output)
- All GPIO outputs are 3.3 V logic (drives NPN transistor base through 1 kΩ resistor)
- Power via VIN (5 V) from LM2596 output
