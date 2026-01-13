const int FAN_PIN = 7; 

void setup() {
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);   
}

void loop() {
  digitalWrite(FAN_PIN, HIGH); 
  delay(3000);

  digitalWrite(FAN_PIN, LOW);   
  delay(3000);
}
