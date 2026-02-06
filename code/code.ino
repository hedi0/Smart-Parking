#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// WiFi credentials
const char* wifiName = "YOUR_WIFI_SSID";
const char* wifiPass = "YOUR_WIFI_PASSWORD";

// Firebase setup
#define DATABASE_URL "https://your-project-id.firebaseio.com"
#define API_KEY "YOUR_FIREBASE_API_KEY"

// Pin assignments
#define RFID_RST 22
#define RFID_SS 21
const int servoPin = 13;
const int openAngle = 90;
const int closeAngle = 0;
const int trigPins[3] = {32, 33, 25};
const int echoPins[3] = {26, 27, 14};
const int buzzer = 12;
const int red = 4;
const int green = 2;
const int blue = 15;
const int button = 0;
