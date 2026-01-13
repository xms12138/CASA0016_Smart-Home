const int FLAME_PIN = 2;  // DO pin

void setup() {
  Serial.begin(115200);
  pinMode(FLAME_PIN, INPUT);
  Serial.println("Flame sensor test (digital DO)");
}

void loop() {
  int v = digitalRead(FLAME_PIN);

  if (v == HIGH) { 
    Serial.println("🔥 Flame detected!");
  } else {
    Serial.println("No flame");
  }

  delay(300);
}
