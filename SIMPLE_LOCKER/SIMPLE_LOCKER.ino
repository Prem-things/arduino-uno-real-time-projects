#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address may be 0x3F on some models

// Keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {6, 7, 8, 9};
byte colPins[COLS] = {10, 11, 12, 13};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Pins
#define RELAY_PIN 2
#define BUZZER_PIN 3

// Logic
const String correctPassword = "1234";
const String resetCode = "****";
String inputCode = "";
int attempts = 0;
bool lockedOut = false;

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // Lock initially

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Booting...");
  delay(1500);
  showReadyScreen();
}

void loop() {
  char key = keypad.getKey();

  if (key && !lockedOut) {
    beep(1);
    inputCode += key;
    showInput(inputCode);

    if (inputCode.length() == 4) {
      if (inputCode == correctPassword) {
        successUnlock();
      } else {
        attempts++;
        wrongAttempt();
        if (attempts >= 3) {
          lockOut();
        }
      }
      inputCode = "";
    }
  }

  // Allow reset in locked out mode
  if (key && lockedOut) {
    beep(1);
    inputCode += key;
    showInput(inputCode);

    if (inputCode.length() == 4) {
      if (inputCode == resetCode) {
        resetSystem();
      } else {
        beep(2);
        showMessage("Reset Code ??");
        delay(1500);
      }
      inputCode = "";
    }
  }
}

// ------------ HELPER FUNCTIONS ------------

void beep(int times) {
  for (int i = 0; i < times; i++) {
    tone(BUZZER_PIN, 1000, 100);
    delay(150);
  }
}

void successUnlock() {
  Serial.println("Correct PIN – Unlocked!");
  showMessage("Access Granted");
  playSuccessTune();

  digitalWrite(RELAY_PIN, LOW); // Unlock
  showMessage("Door Opened");
  delay(5000);
  digitalWrite(RELAY_PIN, HIGH);  // Lock again
  showReadyScreen();
}

void wrongAttempt() {
  Serial.println("Wrong PIN!");
  tone(BUZZER_PIN, 400, 300);
  showMessage("Wrong Password");
  delay(1000);
}

void lockOut() {
  Serial.println("LOCKED OUT");
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 300, 200);
    delay(300);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LOCKED OUT!!!");
  lcd.setCursor(0, 1);
  lcd.print("Use Reset Code");
  lockedOut = true;
}

void resetSystem() {
  Serial.println("Reset Success");
  tone(BUZZER_PIN, 2000, 500);
  attempts = 0;
  lockedOut = false;
  showMessage("System Reset");
  delay(1500);
  showReadyScreen();
}

void playSuccessTune() {
  int melody[] = { 523, 659, 784, 1046 };
  int duration = 150;
  for (int i = 0; i < 4; i++) {
    tone(BUZZER_PIN, melody[i], duration);
    delay(duration + 50);
  }
}

void showInput(String code) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(lockedOut ? "Reset Mode" : "Enter PIN:");
  lcd.setCursor(0, 1);
  lcd.print(code);
}

void showMessage(String msg) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(msg);
}

void showReadyScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter 4digit PIN");
}
