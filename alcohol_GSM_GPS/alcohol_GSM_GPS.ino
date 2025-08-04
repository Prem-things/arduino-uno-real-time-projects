#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "DFRobot_Alcohol.h"

// === Pins ===
SoftwareSerial mySerial(9, 8); // EC200U TX, RX
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int alcoholPin = 12;     
const int ledPin = 5;
const int buzzer = 6;
const int lock = 2;

#define COLLECT_NUMBER        50
#define ALCOHOL_I2C_ADDRESS   0x75
DFRobot_Alcohol_I2C Alcohol(&Wire, ALCOHOL_I2C_ADDRESS);

bool alcoholState = HIGH;
bool alertSent = false;

void setup() {
  mySerial.begin(115200);
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  pinMode(alcoholPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(lock, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(ledPin, LOW);

 /* lcd.setCursor(0, 0);
  lcd.print("Welcome To");
  lcd.setCursor(0, 1);
  lcd.print("Science Utsav");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DESIGN By :-");
  lcd.setCursor(0, 1);
  lcd.print("SUJAN");
  delay(3000);
  lcd.clear();*/

  if (!Alcohol.begin()) {
    Serial.println("Alcohol Sensor ERROR");
    lcd.setCursor(0, 0);
    lcd.print("Sensor ERROR!!!");
    while (1);
  }
  Alcohol.setModes(MEASURE_MODE_AUTOMATIC);
}

String getGPSLocation() {
  String gpsData = "";
  Serial.println("Getting GPS location...");
  mySerial.println("AT+QGPS=1");  // Start GNSS
  delay(2000);

  mySerial.println("AT+QGPSLOC=2");
  delay(2000);

  while (mySerial.available()) {
    gpsData += mySerial.readString();
  }
  mySerial.println("AT+QGPSEND"); // Stop GNSS
  delay(1000);

  Serial.println("GPS Data: " + gpsData);

  // Parse latitude & longitude from gpsData
  int index = gpsData.indexOf("+QGPSLOC:");
  if (index != -1) {
    String loc = gpsData.substring(index + 9, gpsData.indexOf("\n", index));
    loc.trim();
    String lat = loc.substring(0, loc.indexOf(","));
    loc = loc.substring(loc.indexOf(",") + 1);
    String lon = loc.substring(0, loc.indexOf(","));
    return "https://maps.google.com/?q=" + lat + "," + lon;
  } else {
    return "Location Unavailable";
  }
}

void sendSMS(String message) {
  Serial.println("Sending SMS...");
  mySerial.println("AT+CMGF=1");
  delay(1000);
  mySerial.println("AT+CMGS=\"+919305387434\"\r");
  delay(1000);
  mySerial.println(message);
  delay(500);
  mySerial.println((char)26); // Ctrl+Z
  delay(5000);
  Serial.println("SMS Sent.");
}

void makeCall(String phoneNumber) {
  Serial.println("Making Call...");
  mySerial.println("ATD" + phoneNumber + ";");
  delay(20000); // Call duration (20s)
  mySerial.println("ATH"); // Hang up
  delay(1000);
  Serial.println("Call Ended.");
}

void loop() {
  float ppm = Alcohol.readAlcoholData(COLLECT_NUMBER);
  Serial.print("Alcohol: ");
  Serial.print(ppm);
  Serial.println(" PPM");

  if (ppm < 1.00) {
    alcoholState = HIGH;

    digitalWrite(ledPin, LOW);
    digitalWrite(lock, LOW);
    digitalWrite(buzzer, LOW);
    lcd.setCursor(0, 0);
    lcd.print("Air Quality OK ");
    lcd.setCursor(0, 1);
    lcd.print("PPM: ");
    lcd.print(ppm, 2);
    alertSent = false;
  } else {
    alcoholState = LOW;
    digitalWrite(lock, HIGH);
    digitalWrite(ledPin, HIGH);
    digitalWrite(buzzer, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ALCOHOL ALERT");
    lcd.setCursor(0, 1);
    lcd.print("PPM: ");
    lcd.print(ppm, 2);

    if (!alertSent) {
      String gpsLink = getGPSLocation();
      sendSMS("ALCOHOL Detected! PPM: " + String(ppm, 2) + "\nLocation: " + gpsLink);
      makeCall("+919305387434");
      alertSent = true;
    }
  }
  delay(2000);
}
