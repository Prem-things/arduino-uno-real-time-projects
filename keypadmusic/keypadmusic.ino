#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int sensorPin = A0;
const int buzzerPin = 3;
const float alertThreshold = 0.04;  // 0.04% BAC threshold

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  pinMode(sensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Alcohol Sensor");
  delay(1500);
  lcd.clear();
}

void loop() {
  int rawValue = analogRead(sensorPin);

  // Simple approximation: map 0–1023 to 0.00%–0.10% BAC
  float alcoholPercent = map(rawValue, 0, 1023, 0, 1000) / 10000.0;

  lcd.setCursor(0, 0);
  lcd.print("BAC: ");
  lcd.print(alcoholPercent, 3); // Show 3 decimal places
  lcd.print(" %   ");

  if (alcoholPercent > alertThreshold) {
    lcd.setCursor(0, 1);
    lcd.print("ALERT! HIGH LEVEL ");
    tone(buzzerPin, 1000); // Beep warning
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Safe              ");
    noTone(buzzerPin);
  }

  delay(1000);
}

byte rowPins[ROWS] = {6, 7, 8, 9};
byte colPins[COLS] = {10, 11, 12, 13};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int buzzer1 = 2;
const int buzzer2 = 3;

void setup() {
  pinMode(buzzer1, OUTPUT);
  pinMode(buzzer2, OUTPUT);
}

void playNote(int buzzerPin, int freq, int duration) {
  tone(buzzerPin, freq, duration);
  delay(duration);
  noTone(buzzerPin);
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    switch (key) {
      case '1':
        playNote(buzzer1, 262, 200); // C4
        break;
      case '2':
        playNote(buzzer2, 294, 200); // D4
        break;
      case '3':
        playNote(buzzer1, 330, 100); // E4
        delay(50);
        playNote(buzzer2, 392, 100); // G4
        break;
      case '4':
        tone(buzzer1, 349);
        delay(100);
        tone(buzzer2, 440);
        delay(100);
        noTone(buzzer1);
        noTone(buzzer2);
        break;
      case '5':
        playNote(buzzer1, 392, 150);
        delay(50);
        playNote(buzzer2, 523, 150);
        break;
      case '6':
        playNote(buzzer1, 440, 200);
        break;
      case '7':
        playNote(buzzer2, 494, 200);
        break;
      case '8':
        playNote(buzzer1, 523, 200);
        break;
      case '9':
        playNote(buzzer2, 587, 200);
        break;
      case '0':
        playNote(buzzer1, 659, 120);
        delay(50);
        playNote(buzzer2, 784, 120);
        break;
      case 'A':
        tone(buzzer1, 698, 150);
        tone(buzzer2, 880, 150);
        delay(150);
        noTone(buzzer1);
        noTone(buzzer2);
        break;
      case 'B':
        playNote(buzzer1, 784, 100);
        playNote(buzzer2, 988, 100);
        break;
      case 'C':
        playNote(buzzer2, 1047, 200);
        break;
      case 'D':
        playNote(buzzer1, 1175, 200);
        break;
      case '*':
        playNote(buzzer2, 1319, 200);
        break;
      case '#':
        tone(buzzer1, 1397);
        delay(200);
        noTone(buzzer1);
        break;
    }
  }
}
