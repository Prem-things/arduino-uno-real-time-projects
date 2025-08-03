#include <Wire.h>
#include <VL53L1X.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

VL53L1X sensor;
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define SETUP_BUTTON_PIN 5
#define MEASURE_BUTTON_PIN 6
#define BUZZER_PIN 7
#define LED_PIN 2
#define EEPROM_ADDR 0

float initialHeight = 0.0;
bool setupMode = false;

enum MeasureState { MS_IDLE, MS_PREPARE, MS_MEASURE, MS_SHOW_RESULT } measureState = MS_IDLE;
unsigned long stateStartTime;
const unsigned long prepareTime = 5000;
const unsigned long measureTime = 5000;
const unsigned long displayTime = 5000;

float heightReadings[50];
int readingCount;

void showWelcome() {
  lcd.clear();
  lcd.print("Height Measure");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");
  delay(2000);
  showReadyDisplay();
}

void showReadyDisplay() {
  
  lcd.clear();
  lcd.print("Sytem Ready...");
  lcd.setCursor(0, 1);
  lcd.print("measure height");
}

void startMeasurement() {
  measureState = MS_PREPARE;
  stateStartTime = millis();
  readingCount = 0;
  
  lcd.clear();
  lcd.print("Prepare to");
  lcd.setCursor(0, 1);
  lcd.print("measure...");
  delay(1000);
  lcd.clear();
  lcd.print("Stand below the ");
  lcd.setCursor(0, 1);
  lcd.print("Sensor...");
  
  for (int i = 0; i < 5; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
}

void startMeasuring() {
  measureState = MS_MEASURE;
  stateStartTime = millis();
  lcd.clear();
  lcd.print("Measuring...");
}

void finishMeasuring() {
  measureState = MS_SHOW_RESULT;
  stateStartTime = millis();
  
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);

  float finalHeight = calculateHeight();
  lcd.clear();
  lcd.print("Your Height:");
  lcd.setCursor(0, 1);
  
  if (finalHeight <= 0) {
    lcd.print("No one detected");
  } else {
    displayFeetInches(finalHeight);
  }
  digitalWrite(LED_PIN,LOW);
}

void takeMeasurement() {
  uint16_t distance = sensor.read();
  if (distance > 0 && distance < 4000) {
    float currentHeight = initialHeight - distance;
    if (currentHeight > 0) { // Only store positive heights
      heightReadings[readingCount++] = currentHeight;
      if (readingCount >= 50) readingCount = 49;
    }
  }
}

void showPrepareCountdown() {
  unsigned long remaining = (prepareTime - (millis() - stateStartTime)) / 1000 + 1;
  lcd.setCursor(13, 1);
  lcd.print(remaining);
  lcd.print("s ");
}

void showMeasureCountdown() {
  unsigned long remaining = (measureTime - (millis() - stateStartTime)) / 1000 + 1;
  lcd.setCursor(13, 1);
  lcd.print(remaining);
  lcd.print("s ");
}

void showSetupDistance() {
  static unsigned long lastUpdate;
  if (millis() - lastUpdate > 300) {
    uint16_t distance = sensor.read();
    lcd.clear();
    lcd.print("Current Height:");
    lcd.setCursor(0, 1);
    if (distance > 0 && distance < 4000) {
      displayFeet(distance);
    } else {
      lcd.print("Invalid reading");
    }
    lastUpdate = millis();
  }
}

void displayFeet(float mm) {
  float inches = mm / 25.4;
  float feet = inches / 12;
  lcd.print(feet, 2);
  lcd.print(" feet");
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  lcd.init();
  lcd.backlight();
  
  if (!sensor.init()) {
    lcd.print("Sensor Error!");
    while(1);
  }
  sensor.setDistanceMode(VL53L1X::Long);
  sensor.startContinuous(50);

  pinMode(SETUP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MEASURE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  initialHeight = readFloatFromEEPROM(EEPROM_ADDR);
  showWelcome();
}

void loop() {
  handleButtons();
  updateMeasurementProcess();
}

void handleButtons() {
  if (digitalRead(SETUP_BUTTON_PIN) == LOW) {
    delay(50);
    if (digitalRead(SETUP_BUTTON_PIN) == LOW) {
      toggleSetupMode();
      //showReadyDisplay();
      while(digitalRead(SETUP_BUTTON_PIN) == LOW);
     showReadyDisplay();
    }
  }

  if (measureState == MS_IDLE && digitalRead(MEASURE_BUTTON_PIN) == LOW) {
    delay(50);
    if (digitalRead(MEASURE_BUTTON_PIN) == LOW) {
      digitalWrite(LED_PIN,HIGH);
      startMeasurement();
      while(digitalRead(MEASURE_BUTTON_PIN) == LOW);
    }
  }
}

void updateMeasurementProcess() {
  switch(measureState) {
    case MS_PREPARE:
      if (millis() - stateStartTime >= prepareTime) {
        startMeasuring();
      } else {
        showPrepareCountdown();
      }
      break;
      
    case MS_MEASURE:
      if (millis() - stateStartTime >= measureTime) {
        finishMeasuring();
      } else {
        takeMeasurement();
        showMeasureCountdown();
      }
      break;
      
    case MS_SHOW_RESULT:
      if (millis() - stateStartTime >= displayTime) {
        measureState = MS_IDLE;
        showReadyDisplay();
      }
      break;
      
    case MS_IDLE:
      if (setupMode) {
        showSetupDistance();
      }
      break;
  }
}

void toggleSetupMode() {
  setupMode = !setupMode;
  digitalWrite(LED_PIN, setupMode);
  
  if (setupMode) {
    lcd.clear();
    lcd.print("Setup Mode");
    delay(1000);
  } else {
    uint16_t distance = sensor.read();
    if (distance > 0 && distance < 4000) {
      initialHeight = distance;
      writeFloatToEEPROM(EEPROM_ADDR, initialHeight);
      lcd.clear();
      lcd.print("Height Set To:");
      lcd.setCursor(0, 1);
      displayFeet(initialHeight);
      delay(2000);
    }
  }
}

float calculateHeight() {
  if (readingCount == 0) return 0;
  
  // Sort readings in descending order
  for (int i = 0; i < readingCount-1; i++) {
    for (int j = i+1; j < readingCount; j++) {
      if (heightReadings[i] < heightReadings[j]) {
        float temp = heightReadings[i];
        heightReadings[i] = heightReadings[j];
        heightReadings[j] = temp;
      }
    }
  }
  
  // Take average of top 10 readings (or all if less than 10)
  int count = min(10, readingCount);
  float sum = 0;
  for (int i = 0; i < count; i++) {
    sum += heightReadings[i];
  }
  
  return sum / count;
}

void displayFeetInches(float mm) {
  float inches = mm / 25.4;
  int feet = inches / 12;
  float remainingInches = inches - (feet * 12);
  
  lcd.print(feet);
  lcd.print("' ");
  lcd.print(remainingInches, 1);
  lcd.print("\"");
}

void writeFloatToEEPROM(int addr, float val) {
  EEPROM.put(addr, val);
}

float readFloatFromEEPROM(int addr) {
  float val = 0.0;
  EEPROM.get(addr, val);
  return val;
}