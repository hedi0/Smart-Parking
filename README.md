# 🚗 Smart Parking System - ESP32 & Firebase

A complete IoT-based smart parking system using an ESP32 microcontroller with RFID authentication, ultrasonic sensors, a servo‑controlled gate, and Firebase Realtime Database integration for real‑time monitoring.

![ESP32 Smart Parking System](https://img.shields.io/badge/Platform-ESP32_Arduino-blue)
![Firebase Integration](https://img.shields.io/badge/Cloud-Firebase_Realtime_DB-orange)
![License](https://img.shields.io/badge/License-MIT-green)
![Version](https://img.shields.io/badge/Version-2.0.0-blue)

![Project Cover](/images/smart_parking_cover.png)

&#x20; &#x20;
## 📋 Table of Contents
- [Features](#-features)
- [Hardware Requirements](#-hardware-requirements)
- [Schematic Diagram](#-schematic-diagram)
- [Software Setup](#-software-setup)
- [Configuration](#-configuration)
- [Firebase Setup](#-firebase-setup)
- [Installation](#-installation)
- [Usage](#-usage)
- [Troubleshooting](#-troubleshooting)
- [Project Structure](#-project-structure)

## ✨ Features

### 🎯 Core Functionality
- **RFID Card Authentication** - Secure access control with whitelisted cards
- **Automatic Slot Detection** - 3 ultrasonic sensors monitor parking spot occupancy
- **Servo-Controlled Gate** - Automatic gate opening/closing with safety timers
- **Real-time Display** - 16x2 LCD shows available spots and system status
- **Cloud Connectivity** - Firebase Realtime Database for remote monitoring
- **Multi-feedback System** - LED indicators, buzzer sounds, and visual display

### 📊 Monitoring & Control
- **Live Slot Status** - Track occupied/available spots in real-time
- **Entry Statistics** - Count authorized entries and failed attempts
- **Remote Commands** - Open gate or lock system via Firebase
- **Activity Logging** - Timestamped records of all access attempts
- **Offline Operation** - Local functionality during internet outages

## 🛠️ Hardware Requirements

### Components List

| Component                 | Quantity | Purpose             |
| ------------------------- | -------- | ------------------- |
| ESP32 Dev Board           | 1        | Main controller     |
| MFRC522 RFID Module       | 1        | Card authentication |
| HC‑SR04 Ultrasonic Sensor | 3        | Slot detection      |
| SG90 Servo Motor          | 1        | Gate control        |
| 16×2 I2C LCD              | 1        | User interface      |
| RGB LED                   | 1        | Status indicator    |
| Buzzer                    | 1        | Audio feedback      |
| Push Button               | 1        | Manual override     |
| Jumper Wires              | —        | Connections         |
| Breadboard                | 1        | Prototyping         |
| 5V Power Supply           | 1        | Servo power         |

### 🔌 Pin Connections

| ESP32 Pin   | Component       | Description      |
| ----------- | --------------- | ---------------- |
| GPIO21      | RFID SDA (SS)   | MFRC522 SDA      |
| GPIO22      | RFID RST        | MFRC522 RST      |
| GPIO18      | RFID SCK        | SPI Clock        |
| GPIO19      | RFID MISO       | SPI MISO         |
| GPIO23      | RFID MOSI       | SPI MOSI         |
| GPIO13      | Servo           | Control signal   |
| GPIO32 / 26 | Ultrasonic 1    | Trig / Echo      |
| GPIO33 / 27 | Ultrasonic 2    | Trig / Echo      |
| GPIO25 / 14 | Ultrasonic 3    | Trig / Echo      |
| GPIO4       | RGB LED (Red)   | 220Ω resistor    |
| GPIO2       | RGB LED (Green) | 220Ω resistor    |
| GPIO15      | RGB LED (Blue)  | 220Ω resistor    |
| GPIO12      | Buzzer          | Positive pin     |
| GPIO0       | Push Button     | Pull‑down to GND |
| 3.3V        | MFRC522 VCC     | Power            |
| GND         | All             | Common ground    |

> ⚠️ **Note:** Power the servo with an external 5V supply and connect all grounds together.

## 📐 Schematic Diagram

            ┌─────────────────────────────────────────┐
            │               ESP32 Dev Board           │
            │                                         │
            │  +-------------------+                  │
            │  |                   |                  │
            │  |   3.3V─────┬─────┐│                  │
            │  |   GND──────┼─────┤│                  │
            │  |   GPIO21───┼─────┘│                  │
            │  |   GPIO22───┤      │                  │
            │  |   GPIO18───┤      │                  │
            │  |   GPIO19───┤      │                  │
            │  |   GPIO23───┤      │                  │
            │  |            │      │                  │
            │  +------------│------+                  │
            │               │                         │
            │         ┌─────┴─────┐                   │
            │         │  MFRC522  │                   │
            │         │  RFID     │                   │
            │         └───────────┘                   │
            │                                         │
            │  Servo: GPIO13   LCD: SDA/SCL           │
            │  Sensors: 32-26, 33-27, 25-14           │
            │  LEDs: 4(R), 2(G), 15(B)                │
            │  Buzzer: GPIO12   Button: GPIO0         │
            └─────────────────────────────────────────┘


## ⚙️ Software Setup

### 1. Install Arduino IDE
- Download from [Arduino official website](https://www.arduino.cc/en/software)
- Install ESP32 board support:
  1. Open **File → Preferences**
  2. Add to Additional Boards Manager URLs:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
  3. Go to **Tools → Board → Boards Manager**
  4. Search for "**ESP32 by Espressif Systems**" and install

### 2. Install Required Libraries
Open Arduino IDE and install these libraries via **Sketch → Include Library → Manage Libraries**:

| Library | Version | Purpose |
|---------|---------|---------|
| **Firebase ESP Client** | ≥4.0.0 | Firebase cloud integration |
| **LiquidCrystal I2C** | ≥1.1.2 | LCD display control |
| **ESP32Servo** | ≥0.12.0 | Servo motor control |
| **MFRC522** | ≥1.4.10 | RFID module communication |
| **WiFi** | (Built-in) | WiFi connectivity |

### 3. Clone Repository
```bash
git clone https://github.com/hedi0/Smart-Parking.git
cd Smart-Parking
```

## 🔧 Configuration

### 1. WiFi Credentials
Edit the following lines in the code:
```cpp
const char* wifiName = "YOUR_WIFI_SSID";
const char* wifiPass = "YOUR_WIFI_PASSWORD";
```
### 2. Firebase Configuration
```cpp
#define DATABASE_URL "https://YOUR-PROJECT-ID.firebaseio.com"
#define API_KEY "YOUR_FIREBASE_WEB_API_KEY"
```
### 3. RFID Card Setup
Add your RFID card UIDs to the allowed list:
```cpp
String allowedCards[] = {
    "04 5A 1C 93 42 68 80",  // Replace with your card 1
    "04 B5 1C 93 42 68 80",  // Replace with your card 2
    "04 64 1C 93 42 68 80"   // Replace with your card 3
};
```
#### To get your card UID:
- Upload a simple RFID reading sketch
- Open Serial Monitor (115200 baud)
- Scan your RFID card/tag
- Copy the UID shown

### 4. Parking Spot Configuration
```cpp
// Adjust these values based on your setup
const int openAngle = 90;    // Gate open position
const int closeAngle = 0;    // Gate closed position
const unsigned long autoClose = 10000;  // Auto-close after 10 seconds

// Parking spot IDs (for Firebase)
String slotIDs[3] = {"A1", "B2", "C3"};
```

## ☁️ Firebase Setup

### 1. Create Firebase Project
- Go to [**Firebase Console**](https://console.firebase.google.com/)            
- Click "Add project"
- Enter project name (e.g., "smart-parking")
- Enable Google Analytics (optional)
- Click "Create project"

### 2. Setup Realtime Database
- From left menu, go to Build → Realtime Database
- Click "Create database"
- Choose location closest to you
- Start in test mode (for development)
- Click "Enable"

### 3. Get Database URL & API Key
- Click project settings ⚙️ → Project settings
- Under General tab, copy:
     - Project ID (for DATABASE_URL)
     - Web API Key (for API_KEY)

### 4. Database Rules (Security)
Go to Rules tab and set:
```json
{
  "rules": {
    ".read": "auth != null",
    ".write": "auth != null"
  }
}
```
> ⚠️ Secure your rules before production use.
For production, implement proper authentication

## 📥 Installation

### 1. Upload Code to ESP32
```bash
# Using Arduino IDE:
1. Open `code.ino` in Arduino IDE
2. Select Board: "ESP32 Dev Module"
3. Select correct COM port
4. Click Upload (→ button)
```

### 2. Initial Setup Process
After uploading:
   1- Open Serial Monitor (Tools → Serial Monitor, 115200 baud)
   2- Watch boot process:
   ```txt
    Starting Smart Parking System...
    Joining WiFi...
    Connected!
    My IP: 192.168.1.100
    Connecting to Firebase...
    Connected!
    System is running
   ```
   3- System is ready when LCD shows "Ready to scan"
### 3. Hardware Assembly Tips
1 - Power Connections:
- Servo motor needs 5V (don't power from ESP32 3.3V)
- Use external 5V supply for servo
-Connect all GNDs together

2 - Sensor Placement:
- Mount ultrasonic sensors 30-50cm above ground
- Angle slightly downward (15-20°)
- Test with actual vehicle placement

3 - RFID Reader:
- Place near entrance (5-10cm range)
- Protect from weather if outdoors
- Avoid metal interference

## 🚀 Usage

### System Operation Modes

| Mode | LED Color | Buzzer | LCD Display |
|------|-----------|--------|-------------|
| **Ready** | Blue | - | "Ready to scan" |
| **Valid Card** | Green | Short beep | "Welcome! Gate opening" |
| **Invalid Card** | Red flashing | Long beep | "Card not recognized" |
| **Parking Full** | Red solid | Error beep | "NO SPACE LEFT" |
| **Gate Open** | Green solid | - | "ENTER NOW" |
| **No WiFi** | Blue off | - | "Local mode only" |

### Manual Controls
- **RFID Card**: Scan to open gate (if spots available)
- **Button Press**: Manual gate override (hold for 2 seconds)
- **Serial Commands**: Send commands via Serial Monitor

### Firebase Monitoring
Access your Firebase console to monitor:
- **`/spots/A1`**, **`/spots/B2`**, **`/spots/C3`** - Individual slot status
- **`/status/free`** - Number of available spots
- **`/gate/open`** - Gate status (true/false)
- **`/stats/authorized_entries`** - Successful entries count
- **`/logs/last_valid`** - Last valid entry timestamp

### Remote Commands via Firebase
To control system remotely:
1. Go to Firebase Console → Realtime Database
2. Set these paths:
   - **`/remote/open_gate`** = `true` (opens gate immediately)
   - **`/remote/lock_system`** = `true` (disables system)

## 🔍 Troubleshooting

### Common Issues & Solutions

| Problem | Possible Cause | Solution |
|---------|---------------|----------|
| **No WiFi Connection** | Wrong credentials | Check SSID/password |
| | Weak signal | Move closer to router |
| **RFID Not Reading** | Card not in whitelist | Add card UID to allowedCards[] |
| | Distance too far | Keep card within 5cm |
| **Sensors Show Wrong** | Incorrect wiring | Check trigger/echo pins |
| | Objects too close | Ensure 20cm clearance |
| **Gate Not Moving** | Servo not powered | Connect servo to 5V external supply |
| | Wrong GPIO pin | Verify servo connected to GPIO13 |
| **Firebase Error** | Wrong API key | Regenerate Firebase Web API Key |
| | Database rules | Set rules to public for testing |

### Serial Debug Commands
Monitor these messages in Serial Monitor (115200 baud):
- `Card detected: XX XX XX XX` - Shows scanned RFID UID
- `Spot A1: 15.3cm [TAKEN]` - Ultrasonic sensor readings
- `Free spots: 2/3` - Current availability
- `WiFi dropped, reconnecting...` - Network issues

## 📁 Project Structure

```
Smart-Parking/
├── code.ino # Main Arduino sketch
├── LICENSE # License file 
└── README.md # This documentation
```

## 🔄 Future Enhancements

### Planned Features
- [ ] **Mobile App** - Real-time notifications and remote control
- [ ] **Payment Integration** - Automated billing system
- [ ] **License Plate Recognition** - Camera-based entry
- [ ] **Solar Power** - Self-sufficient energy system
- [ ] **Multi-language Support** - LCD display in multiple languages

### Current Limitations
- Maximum 3 parking spots (expandable with multiplexers)
- No battery backup (power outage resets system)
- Basic security (enhance with encryption)
- Local network only (consider port forwarding for remote)

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit changes (`git commit -m 'Add AmazingFeature'`)
4. Push to branch (`git push origin feature/AmazingFeature`)
 Oeu e
   dNvf
