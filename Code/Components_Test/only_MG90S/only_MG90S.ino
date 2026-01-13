#include <Servo.h>

const int SERVO_PIN = 6;   // Signal wire connected to D5

Servo myServo;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("=== MG90S Servo Test (MKR1010) ===");

  // Attach servo, use extended pulse range for MG90S
  myServo.attach(SERVO_PIN, 500, 2500);  // minPulse, maxPulse in microseconds

  // Move to a known start position
  myServo.write(90);   // center
  delay(1000);
}

void loop() {
  Serial.println("Angle 0");
  myServo.write(0);
  delay(2000);

  Serial.println("Angle 90");
  myServo.write(90);
  delay(2000);

  Serial.println("Angle 180");
  myServo.write(180);
  delay(2000);
}
