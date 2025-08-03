#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

// RFID on SoftwareSerial
#define RFID_RX 9
#define RFID_TX 8
SoftwareSerial rfidSerial(RFID_RX, RFID_TX);  // RX, TX

// 16x4 LCD
LiquidCrystal_I2C lcd(0x27, 16, 4);

// Constants
const int UID_LENGTH = 12;
const int MAX_STORED_UIDS = 10;

// Known student tags
const String knownTags[] = {
  "5000DB8FF3F7",
  "5000DB813339",
  "5000DB97EFF3"
};

const String studentNames[] = {
  "Rahul Sharma",
  "Priya Verma",
  "Amit Mehta"
};

const int totalStudents = sizeof(knownTags) / sizeof(knownTags[0]);
int attendanceCount = 0;

void setup() {
  Serial.begin(9600);
  rfidSerial.begin(9600);
  lcd.init();
  lcd.backlight();

  // Uncomment below once to clear EEPROM, then re-upload without it
  
  for (int i = 0; i < MAX_STORED_UIDS * UID_LENGTH; i++) {
    EEPROM.write(i, 0xFF);
 }
  

  displayWelcomeScreen();
}

void loop() {
  if (rfidSerial.available()) {
    String tag = "";
    unsigned long startTime = millis();

    // Read exactly 12 characters or timeout after 1 second
    while (tag.length() < UID_LENGTH && (millis() - startTime) < 1000) {
      if (rfidSerial.available()) {
        char c = rfidSerial.read();
        if (isPrintable(c)) {
          tag += c;
        }
      }
    }

    if (tag.length() != UID_LENGTH) return; // Invalid or partial UID

    Serial.println("Scanned UID: " + tag);
    int studentIndex = findTagIndex(tag);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ABC Public School");
    lcd.setCursor(0, 1);
    lcd.print("Class 10 - A    ");

    if (studentIndex != -1) {
      if (isAlreadyScanned(tag)) {
        lcd.setCursor(-4, 2);
        lcd.print("Already Scanned ");
        lcd.setCursor(-4, 3);
        lcd.print(studentNames[studentIndex]);
      } else {
        saveToEEPROM(tag);
        attendanceCount++;
        lcd.setCursor(-4, 2);
        lcd.print("Attendance: ");
        lcd.print(attendanceCount);
        lcd.print(" ");
        lcd.setCursor(-4, 3);
        lcd.print(studentNames[studentIndex]);
      }
    } else {
      lcd.setCursor(-4, 2);
      lcd.print("Access Denied! ");
      lcd.setCursor(-4, 3);
      lcd.print("Unknown Card   ");
    }

    delay(3000);
    displayWelcomeScreen();
  }
}

void displayWelcomeScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ABC Public School");
  lcd.setCursor(0, 1);
  lcd.print("Class 10 - A    ");
  lcd.setCursor(-4, 2);
  lcd.print("Attendance: ");
  lcd.print(attendanceCount);
  lcd.setCursor(-4, 3);
  lcd.print("Scan Your Card...");
}

int findTagIndex(String tag) {
  for (int i = 0; i < totalStudents; i++) {
    if (tag == knownTags[i]) {
      return i;
    }
  }
  return -1;
}

bool isAlreadyScanned(String uid) {
  for (int i = 0; i < MAX_STORED_UIDS; i++) {
    int addr = i * UID_LENGTH;
    String stored = readUIDFromEEPROM(addr);
    if (stored == uid) return true;
  }
  return false;
}

void saveToEEPROM(String uid) {
  for (int i = 0; i < MAX_STORED_UIDS; i++) {
    int addr = i * UID_LENGTH;
    String stored = readUIDFromEEPROM(addr);
    if (stored == "") {
      for (int j = 0; j < UID_LENGTH; j++) {
        EEPROM.write(addr + j, uid[j]);
      }
      break;
    }
  }
}

String readUIDFromEEPROM(int startAddr) {
  String result = "";
  for (int i = 0; i < UID_LENGTH; i++) {
    char c = EEPROM.read(startAddr + i);
    if (!isPrintable(c)) return "";  // Empty or corrupt slot
    result += c;
  }
  return result;
}
