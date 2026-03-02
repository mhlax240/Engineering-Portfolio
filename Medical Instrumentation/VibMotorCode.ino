int motorPin = 3;  // Motor connected directly to digital pin D3

void setup() {
  pinMode(motorPin, OUTPUT);
}

void loop() {
  digitalWrite(motorPin, HIGH);
  delay(300);
  digitalWrite(motorPin, LOW);
  delay(1000);
}
