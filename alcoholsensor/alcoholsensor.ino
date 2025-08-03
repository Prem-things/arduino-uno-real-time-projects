#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int sensorPin = A0;  // Flying Fish sensor analog out
const int buzzerPin = 3;   // Buzzer on pin D3

LiquidCrystal_I2C lcd(0x27, 16, 2); // Change to 0x3F if needed

void setup() {
  pinMode(buzzerPin, OUTPUT);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(1000);
  lcd.clear();
}

void loop() {
  int analogValue = analogRead(sensorPin);

  // Simulated BAC value: 0.00 to 0.20%
  float bac = (analogValue / 1023.0) * 0.20;
  
  // Easy percentage: map analog 200–900 to 0–100%
  int alcoholPercent = map(analogValue, 200, 900, 0, 100);
  alcoholPercent = constrain(alcoholPercent, 0, 100);

  // Display BAC on first line
  lcd.setCursor(0, 0);
  lcd.print("BAC: ");
  lcd.print(bac, 3);  // 3 decimal places
  lcd.print(" %   ");

  // Display % alcohol on second line
  lcd.setCursor(0, 1);
  lcd.print("Alcohol: ");
  lcd.print(alcoholPercent);
  lcd.print(" %    ");

  // Buzzer alert if alcohol > 60%
  if (alcoholPercent >= 60) {
    digitalWrite(buzzerPin, HIGH);
  } else {
    digitalWrite(buzzerPin, LOW);
  }

  delay(1000);
}
