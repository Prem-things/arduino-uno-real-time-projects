#include <Wire.h>
#include <VL53L1X.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

VL53L1X sensor;
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define SETUP_BUTTON_PIN 5    // Button to set reference height
#define MEASURE_BUTTON_PIN 6  // Button to initiate measurement
#define BUZZER_PIN 4          // Buzzer for feedback
#define LED_PIN 7            // LED indicator
#define EEPROM_ADDR 0         // EEPROM address

float initialHeight = 0.0;
bool setupMode = false;
unsigned long lastDisplayUpdate = 0;
const long displayInterval = 1000;

// Measurement variables
bool measuring = false;
unsigned long measureStartTime = 0;
const long measureDuration = 5000; // 5 second measurement window
float heightReadings[50];          // Array to store more readings
int readingCount = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  lcd.init();
  lcd.backlight();
  lcd.print("Initializing...");
  
  if (!sensor.init()) {
    lcd.clear();
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
  
  lcd.clear();
  lcd.print("System Ready");
  lcd.setCursor(0, 1);
  lcd.print("Set:Btn1  Meas:Btn2");
  delay(2000);
}

void loop() {
  // Handle setup button
  if (digitalRead(SETUP_BUTTON_PIN) == LOW) {
    delay(50);
    if (digitalRead(SETUP_BUTTON_PIN) == LOW) {
      handleSetupButton();
      while(digitalRead(SETUP_BUTTON_PIN) == LOW);
      delay(200);
    }
  }

  // Handle measure button
  if (digitalRead(MEASURE_BUTTON_PIN) == LOW && !measuring) {
    delay(50);
    if (digitalRead(MEASURE_BUTTON_PIN) == LOW) {
      startMeasurement();
      while(digitalRead(MEASURE_BUTTON_PIN) == LOW);
      delay(200);
    }
  }

  // Update display based on mode
  if (millis() - lastDisplayUpdate >= displayInterval) {
    if (setupMode) {
      showSetupDisplay();
    } 
    else if (!measuring) {
      showReadyDisplay();
    }
    lastDisplayUpdate = millis();
  }

  // Measurement process
  if (measuring) {
    takeMeasurement();
  }
}

void showSetupDisplay() {
  uint16_t distance = sensor.read();
  lcd.clear();
  lcd.print("Raw:");
  lcd.print(distance);
  lcd.print("mm");
  
  lcd.setCursor(0, 1);
  lcd.print("Press to set");
}

void showReadyDisplay() {
  lcd.clear();
  lcd.print("Press Btn2 to");
  lcd.setCursor(0, 1);
  lcd.print("measure height");
}

void startMeasurement() {
  measuring = true;
  measureStartTime = millis();
  readingCount = 0;
  
  lcd.clear();
  lcd.print("Please stand still");
  lcd.setCursor(0, 1);
  lcd.print("Measuring...");
  
  // Beep 5 times
  for (int i = 0; i < 5; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
}

void takeMeasurement() {
  uint16_t distance = sensor.read();
  
  // Store readings during measurement window
  if (millis() - measureStartTime < measureDuration) {
    if (distance > 0 && distance < 4000) {
      heightReadings[readingCount] = initialHeight - distance;
      readingCount = min(readingCount + 1, 49);
      
      // Show countdown
      lcd.setCursor(13, 1);
      lcd.print((measureDuration - (millis() - measureStartTime))/1000);
      lcd.print("s ");
    }
    return;
  }

  // Measurement complete
  measuring = false;
  
  // Beep to signal completion
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);

  // Calculate final height
  float finalHeight = calculateHeight();
  
  // Display result
  lcd.clear();
  lcd.print("Your Height:");
  lcd.setCursor(0, 1);
  displayFeetInches(finalHeight);
  
  // Keep displaying for 8 seconds
  delay(8000);
}

float calculateHeight() {
  // Calculate average of readings within 10mm of median
  float sum = 0;
  int count = 0;
  float median = heightReadings[readingCount/2];
  
  for (int i = 0; i < readingCount; i++) {
    if (abs(heightReadings[i] - median) < 10) {
      sum += heightReadings[i];
      count++;
    }
  }
  
  return count > 0 ? sum/count : 0;
}

void handleSetupButton() {
  if (setupMode) {
    uint16_t distance = sensor.read();
    if (distance > 0 && distance < 4000) {
      initialHeight = distance;
      writeFloatToEEPROM(EEPROM_ADDR, initialHeight);
      lcd.clear();
      lcd.print("Height Set To:");
      lcd.setCursor(0, 1);
      lcd.print(initialHeight);
      lcd.print("mm");
      delay(2000);
    }
    setupMode = false;
    digitalWrite(LED_PIN, LOW);
  } else {
    setupMode = true;
    digitalWrite(LED_PIN, HIGH);
    lcd.clear();
    lcd.print("Setup Mode");
    lcd.setCursor(0, 1);
    lcd.print("Show raw distance");
    delay(1000);
  }
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