// ===== Light Sensor v1.1 Threshold Test (MKR1010) =====
// All comments in English only.

const int LIGHT_SENSOR_PIN = A1;
const unsigned long READ_INTERVAL_MS = 500;

unsigned long lastRead = 0;

// You will tune these after you see real values
int thresholdLow  = 340;   // Below this = DARK
int thresholdHigh = 650;   // Above this = BRIGHT

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("=== Light Sensor Threshold Test ===");
  Serial.println("Columns: time_ms, value, state");
}

void loop() {
  unsigned long now = millis();

  if (now - lastRead >= READ_INTERVAL_MS) {
    lastRead = now;

    int value = analogRead(LIGHT_SENSOR_PIN);

    const char* state = "MID";

    if (value < thresholdLow) {
      state = "DARK";
    } else if (value > thresholdHigh) {
      state = "BRIGHT";
    }

    Serial.print(now);
    Serial.print(", ");
    Serial.print(value);
    Serial.print(", ");
    Serial.println(state);
  }
}
