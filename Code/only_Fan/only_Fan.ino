const int FAN_PIN = 7;   // 换成你实际接的脚

void setup() {
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);   // 先关
}

void loop() {
  digitalWrite(FAN_PIN, HIGH);  // 开
  delay(3000);

  digitalWrite(FAN_PIN, LOW);   // 关
  delay(3000);
}
