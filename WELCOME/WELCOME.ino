#include <SoftwareSerial.h>

#define TRIG_PIN A5
#define ECHO_PIN A4


SoftwareSerial mp3(8, 9); // TX, RX

unsigned long lastPlayTime = 0;
bool isPlaying = false;

void sendMP3Command(uint8_t cmd, const uint8_t *data, uint8_t len) {
  uint8_t pkt[5 + len];
  pkt[0] = 0xAA;
  pkt[1] = cmd;
  pkt[2] = len;
  for (uint8_t i = 0; i < len; i++)
    pkt[3 + i] = data[i];
  uint16_t s = 0;
  for (uint8_t i = 0; i < 3 + len; i++)
    s += pkt[i];
  pkt[3 + len] = s & 0xFF;
  mp3.write(pkt, 4 + len);
}

void setup() {
  Serial.begin(9600);
  mp3.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  delay(200);

  // 1. Switch to SD
  uint8_t drv[] = { 0x01 };
  sendMP3Command(0x0B, drv, 1);
  delay(200);

  // 2. Set volume to 30
  uint8_t vol[] = { 0x1E };
  sendMP3Command(0x13, vol, 1);
  delay(200);
}

void loop() {
  long distance = getDistance();

  if (distance > 0 && distance < 40 && !isPlaying) {
    playTrack2();
    lastPlayTime = millis();
    isPlaying = true;
  }

  // Reset after 5 seconds
  if (isPlaying && (millis() - lastPlayTime >= 5000)) {
    isPlaying = false;
  }
}

// Function to play track 2
void playTrack2() {
  uint8_t track2[] = { 0x00, 0x02 };
  sendMP3Command(0x07, track2, 2);
  delay(100);
  sendMP3Command(0x02, nullptr, 0); // Play
  Serial.println("Playing track 2");
}

// Function to get distance in cm
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 20000); // Timeout after 20ms
  long cm = duration * 0.034 / 2;
  return (duration == 0) ? -1 : cm;
}