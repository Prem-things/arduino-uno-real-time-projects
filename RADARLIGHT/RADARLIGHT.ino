const int radarPin = 2;  // Radar sensor output pin
const int relayPin = 3;  // Relay control pin

void setup() {
  pinMode(radarPin, INPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);  // Relay initially OFF
  Serial.begin(9600);
}

void loop() {
  int radarState = digitalRead(radarPin);

  if (radarState == HIGH) {
    digitalWrite(relayPin, HIGH);  // Turn ON relay
    Serial.println("Motion Detected: Relay ON");
  } else if(radarState == LOW) {
    digitalWrite(relayPin, LOW);   // Turn OFF relay
    Serial.println("No Motion: Relay OFF");
  }

  delay(200);  // Short delay to avoid rapid switching
}
