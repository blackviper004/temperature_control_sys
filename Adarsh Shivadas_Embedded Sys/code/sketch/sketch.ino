#include <WiFi.h>
#include "ThingSpeak.h"
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 15
#define DHTTYPE DHT22

#define FAN_PIN 26
#define HEATER_PIN 27
#define BUZZER_PIN 25

const char* ssid = "Wokwi-GUEST";
const char* password = "";

unsigned long channelID = 3231495;        // Your Channel ID
const char* writeAPIKey = "Y7REKR83NKN5AVEN"; // Your Write API Key

WiFiClient client;
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

float setTemp = 30.0;

void setup() {
  Serial.begin(115200);

  pinMode(FAN_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();
  dht.begin();

  WiFi.begin(ssid, password);
  lcd.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  lcd.print("WiFi Connected");

  ThingSpeak.begin(client);
}

void loop() {
  float temp = dht.readTemperature();

  if (isnan(temp)) return;

  // Determine operating mode
  int operatingMode;
  if (temp > setTemp+2) {
    digitalWrite(FAN_PIN, HIGH);
    digitalWrite(HEATER_PIN, LOW);
    operatingMode = -1; // Cooling
  } else if (temp < setTemp-2) {
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(HEATER_PIN, HIGH);
    operatingMode = 1; // Heating
  } else {
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(HEATER_PIN, LOW);
    operatingMode = 0; // Idle
  }

  // Fan and Heater status for ThingSpeak
  int fanStatus = digitalRead(FAN_PIN);
  int heaterStatus = digitalRead(HEATER_PIN);
  int alarmStatus = digitalRead(BUZZER_PIN); // Assuming buzzer is the alarm

  // Display on LCD
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print(" C   ");

  lcd.setCursor(0, 1);
  lcd.print("Uploading...   ");

  // Send all fields to ThingSpeak
  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, fanStatus);
  ThingSpeak.setField(3, heaterStatus);
  ThingSpeak.setField(4, alarmStatus);
  ThingSpeak.setField(5, operatingMode);

  int x = ThingSpeak.writeFields(channelID, writeAPIKey);

  if (x == 200) {
    Serial.println("Update successful.");
  } else {
    Serial.print("ThingSpeak error: ");
    Serial.println(x);
  }

  delay(15000); // ThingSpeak minimum interval
}
