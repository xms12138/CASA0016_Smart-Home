const int FAN_PIN = 7;

void setup() {
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);
}

void loop() {
  digitalWrite(FAN_PIN, HIGH);  

  digitalWrite(FAN_PIN, LOW);  
  delay(3000);
}
