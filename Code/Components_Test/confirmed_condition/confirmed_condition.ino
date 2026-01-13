#include <Wire.h>
#include <Adafruit_MLX90640.h>

Adafruit_MLX90640 mlx;
float frame[32 * 24];

const float TEMP_DELTA_THRESHOLD = 3.5;  
const int   MIN_HOT_PIXELS       = 8;    
// -------------------------------------------------


bool detectHuman(const float *f) {
  float sum = 0.0;
  float minT = 1000.0;
  float maxT = -1000.0;


  for (int i = 0; i < 32 * 24; i++) {
    float t = f[i];
    sum += t;
    if (t < minT) minT = t;
    if (t > maxT) maxT = t;
  }

  float avg = sum / (32.0 * 24.0);     
  float threshold = avg + TEMP_DELTA_THRESHOLD;


  int hotPixels = 0;
  for (int i = 0; i < 32 * 24; i++) {
    if (f[i] > threshold) {
      hotPixels++;
    }
  }

  bool presence = (hotPixels >= MIN_HOT_PIXELS) &&
                  ((maxT - avg) > TEMP_DELTA_THRESHOLD);

  Serial.print("avg=");
  Serial.print(avg, 2);
  Serial.print("  max=");
  Serial.print(maxT, 2);
  Serial.print("  hotPixels=");
  Serial.print(hotPixels);
  Serial.print("  presence=");
  Serial.println(presence ? "YES" : "NO");

  return presence;
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("MLX90640 Human Presence Test (MKR1010)");

  Wire.begin();            
  Wire.setClock(400000);     // 400kHz I2C

  if (!mlx.begin(0x33, &Wire)) {
    Serial.println("❌ MLX90640 not found! Check wiring.");
    while (1) delay(1000);
  }

  Serial.println("✅ MLX90640 detected!");

  mlx.setMode(MLX90640_INTERLEAVED);
  mlx.setResolution(MLX90640_ADC_18BIT);
  mlx.setRefreshRate(MLX90640_4_HZ);  
}

void loop() {
  int status = mlx.getFrame(frame);

  if (status != 0) {
    Serial.print("getFrame error: ");
    Serial.println(status);
    return;
  }


  bool human = detectHuman(frame);

  /*
  Serial.println("FRAME_START");
  for (int y = 0; y < 24; y++) {
    for (int x = 0; x < 32; x++) {
      Serial.print(frame[y * 32 + x], 1);
      if (x < 31) Serial.print(", ");
    }
    Serial.println();
  }
  Serial.println("FRAME_END");
  */

  delay(250);  // 约 4 帧/秒
}
