# 🚗 Smart Parking System - ESP32 & Firebase

![ESP32 Smart Parking System](https://img.shields.io/badge/Platform-ESP32_Arduino-blue)
![Firebase Integration](https://img.shields.io/badge/Cloud-Firebase_Realtime_DB-orange)
![License](https://img.shields.io/badge/License-MIT-green)
![Version](https://img.shields.io/badge/Version-2.0.0-blue)

A complete IoT-based smart parking system using ESP32 microcontroller with RFID authentication, ultrasonic sensors, servo-controlled gate, and Firebase cloud integration for real-time monitoring.

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
| Component | Quantity | Purpose |
|-----------|----------|---------|
| **ESP32 Dev Board** | 1 | Main microcontroller |
| **MFRC522 RFID Module** | 1 | Card authentication |
| **HC-SR04 Ultrasonic Sensors** | 3 | Distance measurement |
| **SG90 Servo Motor** | 1 | Gate control |
| **16x2 I2C LCD Display** | 1 | User interface |
| **RGB LED** | 1 | Status indicator |
| **Buzzer** | 1 | Audio feedback |
| **Push Button** | 1 | Manual override |
| **Jumper Wires** | - | Connections |
| **Breadboard** | 1 | Prototyping |
| **5V Power Supply** | 1 | System power |

### 🔌 Pin Connections
| ESP32 Pin | Component | Connection |
|-----------|-----------|------------|
| **GPIO21** | RFID SS (SDA) | Pin 3 on MFRC522 |
| **GPIO22** | RFID RST | Pin 9 on MFRC522 |
| **GPIO18** | RFID SCK | Pin 5 on MFRC522 |
| **GPIO19** | RFID MISO | Pin 6 on MFRC522 |
| **GPIO23** | RFID MOSI | Pin 4 on MFRC522 |
| **GPIO13** | Servo Motor | Signal (orange wire) |
| **GPIO32** | Ultrasonic 1 Trigger | HC-SR04 Trig |
| **GPIO26** | Ultrasonic 1 Echo | HC-SR04 Echo |
| **GPIO33** | Ultrasonic 2 Trigger | HC-SR04 Trig |
| **GPIO27** | Ultrasonic 2 Echo | HC-SR04 Echo |
| **GPIO25** | Ultrasonic 3 Trigger | HC-SR04 Trig |
| **GPIO14** | Ultrasonic 3 Echo | HC-SR04 Echo |
| **GPIO4** | RGB LED Red | Anode (via 220Ω resistor) |
| **GPIO2** | RGB LED Green | Anode (via 220Ω resistor) |
| **GPIO15** | RGB LED Blue | Anode (via 220Ω resistor) |
| **GPIO12** | Buzzer | Positive terminal |
| **GPIO0** | Push Button | One terminal (other to GND) |
| **3.3V** | MFRC522 VCC | Pin 8 on MFRC522 |
| **GND** | All components | Common ground |

*Note: Connect all GND pins together. Servo motor requires 5V power.*

## 📐 Schematic Diagram
