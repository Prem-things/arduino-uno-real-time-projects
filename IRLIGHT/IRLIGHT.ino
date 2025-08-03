#include <IRremote.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

// === PIN DEFINITIONS ===
#define RECV_PIN 2
#define LED_PIN 6
#define RGB_PIN 4
#define NUM_LEDS 120

// === SYSTEM STATE ===
bool systemOn = false;
Adafruit_NeoPixel strip(NUM_LEDS, RGB_PIN, NEO_GRB + NEO_KHZ800);

// === FUNCTION PROTOTYPES ===
void runMultiPatterns();
void runExtraPattern(uint8_t code);
bool interruptibleColorFadeLoop();
bool interruptibleBreathingWhite();
bool interruptiblePastelRainbowDrift(uint8_t wait);
bool checkForSystemOff();
void colorWipe(uint32_t color, int delayTime);
void rainbowCycle(uint8_t wait);
void theaterChase(uint32_t color, int wait);
void blinkRandom(int times);
void blinkColor(uint32_t color);
void breathingColor(uint32_t color);
uint32_t Wheel(byte WheelPos);

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  strip.begin();
  strip.show(); // Initialize LEDs to off
}

void loop() {
  if (IrReceiver.decode()) {
    uint8_t code = IrReceiver.decodedIRData.command;
    Serial.print("IR Code Received: ");
    Serial.println(code, HEX);

    switch (code) {
      case 0x12:  // ✅ SYSTEM ON
        systemOn = true;
        digitalWrite(LED_PIN, HIGH);
        Serial.println("System ON");
        break;

      case 0x1E:  // 🔴 SYSTEM OFF
        systemOn = false;
        digitalWrite(LED_PIN, LOW);
        strip.clear();
        strip.show();
        Serial.println("System OFF");
        break;

      case 0x1A:  // 🌈 MULTI MODE
        if (systemOn) {
          Serial.println("Multi RGB Mode");
          runMultiPatterns();
        }
        break;

      default:
        if (systemOn) {
          Serial.println("Extra RGB Pattern");
          runExtraPattern(code);
        }
        break;
    }

    IrReceiver.resume();  // Ready for next command
  }
}

void runMultiPatterns() {
  if (!interruptibleColorFadeLoop()) return;
  if (!interruptibleBreathingWhite()) return;
  if (!interruptiblePastelRainbowDrift(20)) return;
}

bool interruptibleColorFadeLoop() {
  for (int i = 0; i < 255; i++) {
    if (checkForSystemOff()) return false;
    strip.fill(strip.Color(i, 255 - i, 0));
    strip.show();
    delay(10);
  }
  for (int i = 0; i < 255; i++) {
    if (checkForSystemOff()) return false;
    strip.fill(strip.Color(255 - i, 0, i));
    strip.show();
    delay(10);
  }
  for (int i = 0; i < 255; i++) {
    if (checkForSystemOff()) return false;
    strip.fill(strip.Color(0, i, 255 - i));
    strip.show();
    delay(10);
  }
  return true;
}

bool interruptibleBreathingWhite() {
  for (int i = 0; i < 256; i++) {
    if (checkForSystemOff()) return false;
    uint8_t val = (exp(sin(i * 3.14 / 128)) - 0.36787944) * 108;
    strip.fill(strip.Color(val, val, val));
    strip.show();
    delay(15);
  }
  return true;
}

bool interruptiblePastelRainbowDrift(uint8_t wait) {
  for (uint16_t j = 0; j < 256; j++) {
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      if (checkForSystemOff()) return false;
      uint8_t red = (sin((i + j) * 0.1) + 1) * 127;
      uint8_t green = (sin((i + j) * 0.1 + 2) + 1) * 127;
      uint8_t blue = (sin((i + j) * 0.1 + 4) + 1) * 127;
      strip.setPixelColor(i, strip.Color(red, green, blue));
    }
    strip.show();
    delay(wait);
  }
  return true;
}

bool checkForSystemOff() {
  if (IrReceiver.decode()) {
    uint8_t code = IrReceiver.decodedIRData.command;
    if (code == 0x1E) {  // OFF command
      systemOn = false;
      digitalWrite(LED_PIN, LOW);
      strip.clear();
      strip.show();
      Serial.println("System OFF (Interrupt)");
      IrReceiver.resume();
      return true;
    }
    IrReceiver.resume();  // Resume listening
  }
  return false;
}

void runExtraPattern(uint8_t code) {
  strip.clear();

  switch (code) {
    case 0x0A:
      blinkColor(strip.Color(255, 100, 0));
      break;
    case 0x1B:
      theaterChase(strip.Color(0, 255, 255), 50);
      break;
    case 0x1F:
      rainbowCycle(1);
      break;
    case 0x0C:
      colorWipe(strip.Color(255, 255, 255), 5);
      break;
    case 0x0D:
      blinkRandom(4);
      break;
    case 0x0E:
      colorWipe(strip.Color(255, 0, 255), 20);
      break;
    case 0x00:
      colorWipe(strip.Color(0, 255, 125), 20);
      break;
    case 0x0F:
      breathingColor(strip.Color(255, 0, 0)); // 🔴 Smooth Red
      break;
    default:
      blinkColor(strip.Color(10, 10, 10));  // fallback dim blink
      break;
  }

  strip.show();
}

void colorWipe(uint32_t color, int delayTime) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(delayTime);
  }
}

void rainbowCycle(uint8_t wait) {
  for (int j = 0; j < 256; j++) {
    for (int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i * 256 / strip.numPixels() + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void theaterChase(uint32_t color, int wait) {
  for (int j = 0; j < 10; j++) {
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < strip.numPixels(); i += 3) {
        strip.setPixelColor(i + q, color);
      }
      strip.show();
      delay(wait);
      for (int i = 0; i < strip.numPixels(); i += 3) {
        strip.setPixelColor(i + q, 0);
      }
    }
  }
}

void blinkRandom(int times) {
  for (int i = 0; i < times; i++) {
    for (int j = 0; j < strip.numPixels(); j++) {
      strip.setPixelColor(j, strip.Color(random(255), random(255), random(255)));
    }
    strip.show();
    delay(200);
    strip.clear();
    strip.show();
    delay(200);
  }
}

void blinkColor(uint32_t color) {
  for (int i = 0; i < 3; i++) {
    strip.fill(color);
    strip.show();
    delay(250);
    strip.clear();
    strip.show();
    delay(250);
  }
}

void breathingColor(uint32_t color) {
  for (int i = 0; i < 256; i++) {
    uint8_t val = (exp(sin(i * 3.14 / 128)) - 0.36787944) * 108;
    uint8_t r = ((color >> 16) & 0xFF) * val / 255;
    uint8_t g = ((color >> 8) & 0xFF) * val / 255;
    uint8_t b = (color & 0xFF) * val / 255;
    strip.fill(strip.Color(r, g, b));
    strip.show();
    delay(15);
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}