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

void grantAccess(String card) {
  Serial.println("Access approved");
  shortBeep();
  
  if (freeSlots > 0) {
    openGate();
    vehicleCount++;
    lastEntry = getTime();
    
    if (firebaseConnected) {
      Firebase.RTDB.setInt(&dataStream, "/stats/authorized_entries", vehicleCount);
      Firebase.RTDB.setString(&dataStream, "/logs/last_valid", card + " @ " + lastEntry);
    }
    
    screen.clear();
    screen.print("Welcome!");
    screen.setCursor(0, 1);
    screen.print("Gate opening");
    delay(1500);
  } else {
    screen.clear();
    screen.print("Valid card");
    screen.setCursor(0, 1);
    screen.print("NO SPACE LEFT");
    errorBeep();
    delay(2000);
  }
}

void denyAccess(String card) {
  Serial.println("Access denied");
  errorBeep();
  badAttempts++;
  
  if (firebaseConnected) {
    Firebase.RTDB.setInt(&dataStream, "/stats/bad_tries", badAttempts);
    Firebase.RTDB.setString(&dataStream, "/logs/invalid_try", card + " @ " + getTime());
  }
  
  for (int i = 0; i < 3; i++) {
    digitalWrite(red, LOW);
    delay(200);
    digitalWrite(red, HIGH);
    delay(200);
  }
  
  screen.clear();
  screen.print("Card not recognized");
  screen.setCursor(0, 1);
  screen.print("Access blocked");
  delay(2000);
}

void checkSlots() {
  freeSlots = 0;
  
  for (int i = 0; i < 3; i++) {
    float dist = measureDistance(i);
    
    if (dist > 0 && dist < 25.0) {
      slots[i] = true;
    } else {
      slots[i] = false;
      freeSlots++;
    }
    
    Serial.print("Spot ");
    Serial.print(slotIDs[i]);
    Serial.print(": ");
    if (dist < 0) {
      Serial.print("No signal");
    } else {
      Serial.print(dist);
      Serial.print("cm");
    }
    Serial.println(slots[i] ? " [TAKEN]" : " [EMPTY]");
  }
  
  Serial.print("Free spots: ");
  Serial.print(freeSlots);
  Serial.println("/3");
  
  if (freeSlots == 0) {
    digitalWrite(red, HIGH);
    digitalWrite(green, LOW);
  } else {
    digitalWrite(green, HIGH);
  }
}

float measureDistance(int sensor) {
  digitalWrite(trigPins[sensor], LOW);
  delayMicroseconds(2);
  digitalWrite(trigPins[sensor], HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPins[sensor], LOW);
  
  long time = pulseIn(echoPins[sensor], HIGH, 30000);
  
  if (time <= 0) return -1.0;
  
  float distance = time * 0.0343 / 2;
  return distance;
}

void openGate() {
  if (!gateIsOpen) {
    gateMotor.write(openAngle);
    gateIsOpen = true;
    gateTimer = millis();
    
    digitalWrite(green, HIGH);
    digitalWrite(red, LOW);
    
    Serial.println("Gate is open");
  }
}

void shutGate() {
  if (gateIsOpen) {
    gateMotor.write(closeAngle);
    gateIsOpen = false;
    
    digitalWrite(green, LOW);
    digitalWrite(red, HIGH);
    
    Serial.println("Gate closed");
  }
}

void updateScreen() {
  screen.clear();
  screen.setCursor(0, 0);
  
  screen.print("Available:");
  screen.setCursor(11, 0);
  screen.print(freeSlots);
  screen.print("/3");
  
  screen.setCursor(0, 1);
  if (freeSlots == 0) {
    screen.print("FULL - WAIT");
  } else if (gateIsOpen) {
    screen.print("ENTER NOW");
  } else {
    screen.print("SCAN CARD");
  }
}

void setupFirebase() {
  Serial.print("Connecting to cloud...");
  
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = "";
  auth.user.password = "";
  
  Firebase.reconnectWiFi(true);
  dataStream.setBSSLBufferSize(4096, 1024);
  dataStream.setResponseSize(4096);
  config.token_status_callback = tokenStatusCallback;
  
  Firebase.begin(&config, &auth);
  
  Serial.print("Getting token...");
  while ((auth.token.uid) == "") {
    Serial.print(".");
    delay(1000);
  }
  
  firebaseConnected = true;
  Serial.println(" Connected!");
  
  if (Firebase.RTDB.setString(&dataStream, "/system/online", "yes")) {
    Serial.println("Status updated");
  } else {
    Serial.print("Problem: ");
    Serial.println(dataStream.errorReason());
  }
  
  Firebase.RTDB.setString(&dataStream, "/system/boot_time", getTime());
  Firebase.RTDB.setString(&dataStream, "/system/device", WiFi.macAddress());
}

void sendToCloud() {
  if (!firebaseConnected) return;
  
  for (int i = 0; i < 3; i++) {
    String path = "/spots/" + slotIDs[i];
    Firebase.RTDB.setBool(&dataStream, path + "/taken", slots[i]);
    Firebase.RTDB.setString(&dataStream, path + "/checked", getTime());
  }
  
  Firebase.RTDB.setInt(&dataStream, "/status/free", freeSlots);
  Firebase.RTDB.setInt(&dataStream, "/status/total", 3);
  Firebase.RTDB.setBool(&dataStream, "/gate/open", gateIsOpen);
  Firebase.RTDB.setString(&dataStream, "/system/ping", getTime());
}

void readCloudCommands() {
  if (!firebaseConnected) return;
  
  if (Firebase.RTDB.getBool(&dataStream, "/remote/open_gate")) {
    if (dataStream.boolData()) {
      openGate();
      Firebase.RTDB.setBool(&dataStream, "/remote/open_gate", false);
      Serial.println("Remote gate open");
    }
  }
  
  if (Firebase.RTDB.getBool(&dataStream, "/remote/lock_system")) {
    systemActive = dataStream.boolData();
    digitalWrite(red, systemActive ? HIGH : LOW);
  }
}

void setupWiFi() {
  Serial.print("Joining WiFi...");
  screen.clear();
  screen.print("Network:");
  screen.print(wifiName);
  
  WiFi.begin(wifiName, wifiPass);
  int tries = 0;
  
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    Serial.print(".");
    screen.print(".");
    tries++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" Connected");
    Serial.print("My IP: ");
    Serial.println(WiFi.localIP());
    
    screen.setCursor(0, 1);
    screen.print(WiFi.localIP());
    delay(2000);
    
    digitalWrite(blue, HIGH);
  } else {
    Serial.println(" Failed");
    screen.clear();
    screen.print("No internet");
    screen.setCursor(0, 1);
    screen.print("Local mode only");
    delay(2000);
  }
}

void fixWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi dropped, reconnecting...");
    WiFi.disconnect();
    delay(1000);
    WiFi.begin(wifiName, wifiPass);
    
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 10) {
      delay(500);
      Serial.print(".");
      retries++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nBack online!");
      digitalWrite(blue, HIGH);
    } else {
      Serial.println("\nStill offline");
      digitalWrite(blue, LOW);
    }
  }
}

void checkButton() {
  if (digitalRead(button) == LOW) {
    delay(50);
    if (digitalRead(button) == LOW) {
      Serial.println("Button pressed");
      
      if (freeSlots > 0) {
        openGate();
        shortBeep();
        screen.clear();
        screen.print("Manual control");
        screen.setCursor(0, 1);
        screen.print("Opening gate");
        delay(1000);
      } else {
        errorBeep();
        screen.clear();
        screen.print("Manual control");
        screen.setCursor(0, 1);
        screen.print("Lot is full");
        delay(2000);
      }
      
      while (digitalRead(button) == LOW) {
        delay(10);
      }
    }
  }
}

void shortBeep() {
  tone(buzzer, 1000, 100);
  delay(150);
  tone(buzzer, 1500, 100);
  noTone(buzzer);
}

void errorBeep() {
  tone(buzzer, 300, 500);
  delay(500);
  noTone(buzzer);
}

String getTime() {
  unsigned long seconds = millis() / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  
  char timeString[30];
  sprintf(timeString, "Day %lu, %02lu:%02lu:%02lu", 
          days, hours % 24, minutes % 60, seconds % 60);
  return String(timeString);
}
