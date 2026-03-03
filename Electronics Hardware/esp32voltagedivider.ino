int sensorpin = 6;
int filterpin = 7;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  int value = analogRead(sensorpin);
  int fv = analogRead(filterpin);
  Serial.print("Without filter:");
  Serial.print(value);
  Serial.print(" With filter:");
  Serial.println (fv);
  delay(300);
}
