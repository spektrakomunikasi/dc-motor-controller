# dc-motor-controller
# ESP32 DC Motor Controller v5.0

Professional DC Motor Control System with WiFi Dashboard, Real-time Monitoring, and Safety Features.

## 🎯 Features

### Core Functionality
- **2 Push Buttons**: S1 (START GPIO14) / S2 (STOP GPIO27)
- **Current Monitoring**: 0-30A with ACS712-30A sensor
- **Voltage Monitoring**: 24V ±5% with real-time color-coded status
- **Soft Start Control**: 0-10 seconds adjustable ramp
- **Current Limit**: 0-30A with 0.1A step resolution
- **Motor State Machine**: STOPPED → RAMP → RUNNING / OVERCURRENT

### User Interface
- **128x64 OLED Display**: Real-time status with optimized layout
- **Web Dashboard**: Responsive design @ 192.168.4.1
- **WiFi AP Mode**: Direct connection (MOTOR_CTRL_XXXXXX)
- **Live Sliders**: Adjust current limit and soft start time
- **Status Indicators**: Voltage, current, relay status

### Safety & Storage
- **Zener 3.3V Protection**: GPIO safeguard against overvoltage
- **Overcurrent Protection**: Automatic motor cutoff
- **NVS Storage**: Settings persist across reboots
- **State Machine**: Safe motor state transitions
- **Serial Debugging**: Comprehensive logging

---

## 📋 Hardware Requirements

### ESP32 Module
- ESP32 Dev Board (any variant with 36 GPIO pins)
- USB Type-C or Micro-USB for programming

### Sensors
- **ACS712-30A**: Current sensor (GPIO34)
- **24V Voltage Divider**: R1=100kΩ, R2=10kΩ (GPIO35)
- **Zener Diode 3.3V (1N4728A)**: GPIO protection

### Displays & Control
- **128x64 OLED Display**: I2C @ 0x3C (GPIO21/SDA, GPIO22/SCL)
- **2x Push Buttons**: Normally open (GPIO14 S1, GPIO27 S2)
- **2x Relay Modules**: 5V relay (GPIO12 Cutoff, GPIO13 SoftStart)

### Power Supply
- **24V DC Power Supply**: ±5% tolerance (22.8V - 25.2V)
- **Capacitor (100nF)**: Voltage filter

### Motor Load
- DC Motor: Any 24V DC motor with reasonable current draw

---

## 🔌 Wiring Diagram

