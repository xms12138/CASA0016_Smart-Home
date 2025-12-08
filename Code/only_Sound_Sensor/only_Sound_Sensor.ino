// ===== MKR1010 Noise Exposure Alarm (5s Majority, A1) =====
// Triggers alarm only if most of the last 5 seconds are loud.
// Short quiet gaps in music will be ignored.

const int SOUND_PIN = A1;

// Baseline drift factor
float baselineAlpha = 0.98;

// Threshold for a single loud event (you tuned this to ~20)
int loudThreshold = 10;

// 5-second window settings
const int SAMPLE_INTERVAL_MS      = 100;   // one check every 100 ms
const int WINDOW_5S_SAMPLES       = 50;    // 5 seconds / 0.1s = 50 samples
const float NOISE_FRACTION_THRESH = 0.40;  // at least 60% of time must be loud

float baseline = 0;

// History of loud events over the last 5 seconds
bool loudHistory[WINDOW_5S_SAMPLES];
int  loudSamples5s = 0;
int  historyIndex  = 0;

unsigned long lastSampleTime = 0;

bool alarmActive = false;  // true when in "noise" state


int readFastAverage(int samples = 20) {
  long s = 0;
  for (int i = 0; i < samples; i++) {
    s += analogRead(SOUND_PIN);
    delay(2);
  }
  return (int)(s / samples);
}


void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("=== 5s Majority Noise Alarm (A1) ===");

  // Init baseline
  baseline = readFastAverage(50);

  // Init history
  for (int i = 0; i < WINDOW_5S_SAMPLES; i++) {
    loudHistory[i] = false;
  }
}


void loop() {
  unsigned long now = millis();

  if (now - lastSampleTime >= SAMPLE_INTERVAL_MS) {
    lastSampleTime = now;

    // ----- Measure sound and compute level -----
    int current = readFastAverage(20);
    baseline = baselineAlpha * baseline + (1 - baselineAlpha) * current;
    int level = current - baseline;
    bool isLoud = (level > loudThreshold);

    // ----- Update 5s loud history (sliding window) -----
    // Remove the oldest sample from count
    if (loudHistory[historyIndex]) {
      loudSamples5s--;
    }

    // Store new sample
    loudHistory[historyIndex] = isLoud;
    if (isLoud) {
      loudSamples5s++;
    }

    // Move index (circular buffer)
    historyIndex++;
    if (historyIndex >= WINDOW_5S_SAMPLES) {
      historyIndex = 0;
    }

    // ----- Compute loud fraction over last 5 seconds -----
    float loudFraction = (float)loudSamples5s / (float)WINDOW_5S_SAMPLES;
    bool noiseNow = (loudFraction >= NOISE_FRACTION_THRESH);

    // ----- State change detection & messages -----
    if (noiseNow && !alarmActive) {
      // Just entered sustained noise state
      alarmActive = true;
      Serial.println("ALARM: sustained noise >5s (60%+ loud)");
    } else if (!noiseNow && alarmActive) {
      // Just returned to normal
      alarmActive = false;
      Serial.println("NORMAL again");
    }

    // If you want to debug, you can temporarily uncomment:
    // Serial.print("Loud fraction: ");
    // Serial.println(loudFraction, 2);
  }
}
