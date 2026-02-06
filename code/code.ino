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
const char* wifiName = "PASTE_YOUR_WIFI_SSID";
const char* wifiPass = "PASTE_YOUR_WIFI_PASSWORD";

// Firebase setup
#define DATABASE_URL "https://your-project-id.firebaseio.com"
#define API_KEY "PASTE_YOUR_FIREBASE_API_KEY"

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

// Firebase objects
FirebaseData dataStream;
FirebaseAuth auth;
FirebaseConfig config;

// Hardware objects
LiquidCrystal_I2C screen(0x27, 16, 2);
Servo gateMotor;
MFRC522 rfid(RFID_SS, RFID_RST);

// System state
bool slots[3] = {false, false, false};
int freeSlots = 3;
String slotIDs[3] = {"A1", "B2", "C3"};
bool gateIsOpen = false;
bool systemActive = true;
unsigned long gateTimer = 0;
const unsigned long autoClose = 10000;
String allowedCards[] = {"04 5A 1C 93 42 68 80", "04 B5 1C 93 42 68 80", "04 64 1C 93 42 68 80"};
const int cardCount = 3;
unsigned long lastSensorCheck = 0;
const unsigned long sensorDelay = 2000;
unsigned long lastCardCheck = 0;
const unsigned long cardDelay = 500;
int vehicleCount = 0;
int badAttempts = 0;
String lastEntry = "";
bool firebaseConnected = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting Smart Parking System...");
  
  Wire.begin();
  screen.init();
  screen.backlight();
  screen.clear();
  screen.setCursor(0, 0);
  screen.print("Booting System");
  screen.setCursor(0, 1);
  screen.print("Please wait...");
  
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  
  gateMotor.attach(servoPin);
  gateMotor.write(closeAngle);
  
  for (int i = 0; i < 3; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
    digitalWrite(trigPins[i], LOW);
  }
  
  SPI.begin(18, 19, 23, RFID_SS);
  rfid.PCD_Init();
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);
  
  setupWiFi();
  setupFirebase();
  
  digitalWrite(red, HIGH);
  digitalWrite(green, LOW);
  digitalWrite(blue, HIGH);
  
  shortBeep();
  screen.clear();
  screen.print("Ready to scan");
  screen.setCursor(0, 1);
  screen.print("Place card here");
  Serial.println("System is running");
}

void loop() {
  unsigned long now = millis();
  
  if (now - lastCardCheck >= cardDelay) {
    lastCardCheck = now;
    scanRFID();
  }
  
  if (now - lastSensorCheck >= sensorDelay) {
    lastSensorCheck = now;
    checkSlots();
    sendToCloud();
    updateScreen();
    
    if (gateIsOpen && (now - gateTimer >= autoClose)) {
      shutGate();
    }
  }
  
  checkButton();
  readCloudCommands();
  
  if (WiFi.status() != WL_CONNECTED) {
    fixWiFi();
  }
}

void scanRFID() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;
  
  String cardCode = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    cardCode += (rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    cardCode += String(rfid.uid.uidByte[i], HEX);
  }
  cardCode.toUpperCase();
  cardCode.trim();
  
  Serial.print("Card detected: ");
  Serial.println(cardCode);
  
  bool validCard = false;
  for (int i = 0; i < cardCount; i++) {
    if (cardCode == allowedCards[i]) {
      validCard = true;
      break;
    }
  }
  
  if (validCard) {
    grantAccess(cardCode);
  } else {
    denyAccess(cardCode);
  }
  
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

