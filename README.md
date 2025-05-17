# ESP32-CAM Robot Car: Complete Project Guide

Build a Wi-Fi controlled smart robot car with live camera streaming using the ESP32-CAM module.

---

## 🧰 Required Hardware

| Module               | Quantity | Purpose                          |
|----------------------|----------|----------------------------------|
| ESP32-CAM            | 1        | Main controller with camera      |
| L298N Motor Driver   | 1        | Controls 4 motors                |
| DC Gear Motor (6V)   | 4        | Movement of wheels               |
| 18650 Battery (3.7V) | 2        | Power source (total 7.4V)        |
| FTDI Programmer      | 1        | Upload code to ESP32-CAM         |
| Jumper Wires         | 20+      | All electrical connections       |

---

## 🔌 Wiring Instructions

### 1. ESP32-CAM to FTDI (for programming)

ESP32-CAM    →    FTDI GND          →    GND
5V           →    VCC (5V)
U0R          →    TX
U0T          →    RX
GPIO0        →    GND (to enable flash mode)

### 2. ESP32-CAM to L298N Motor Driver

ESP32-CAM    →    L298N GPIO12       →    ENA (PWM for left motors)
GPIO13       →    IN1 (Left Forward)
GPIO15       →    IN2 (Left Backward)
GPIO14       →    ENB (PWM for right motors)
GPIO2        →    IN3 (Right Forward)
GPIO4        →    IN4 (Right Backward)

### 3. Motor Connections

Left motors  → OUT1 & OUT2 (in parallel)
Right motors → OUT3 & OUT4 (in parallel)

### 4. Power Supply

Battery +ve  → L298N +12V Input
Battery -ve  → L298N GND & ESP32-CAM GND

---

## ⚙️ Uploading Code to ESP32-CAM

### Step 1: Install Arduino IDE

Download from [https://www.arduino.cc/en/software](https://www.arduino.cc/en/software)

### Step 2: Add ESP32 Board Support

- Open `File > Preferences`
- Add this URL in **Additional Board Manager URLs**:

https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

- Go to `Tools > Board > Board Manager`, search for **ESP32**, and install it.

### Step 3: Upload Code

1. Connect ESP32-CAM with FTDI as per wiring above.
2. Connect **GPIO0 to GND**.
3. In Arduino IDE:
 - Board: **AI Thinker ESP32-CAM**
 - Port: Select correct COM port
 - Paste your code and click **Upload**
4. Once upload is complete:
 - Disconnect GPIO0 from GND
 - Press the **Reset button** on the ESP32-CAM

---

## 🎮 Web Control Interface

### Step-by-Step Usage

1. Power the ESP32-CAM
2. Connect your device to the Wi-Fi network:
 - **SSID:** `SmartCar_CAM`
 - **Password:** `12345678`
3. Open your browser and go to:

http://192.168.4.1

### Web Features

- **Live camera stream**
- **Speed slider (0-255)**
- **Directional buttons** (Forward, Backward, Left, Right)
- **Emergency Stop** button

---

## 🧠 Advanced Controls

### Touch Control

- Hold buttons to move, release to stop instantly
