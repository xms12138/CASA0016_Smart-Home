#include <Wire.h>
#include <Adafruit_MLX90640.h>

Adafruit_MLX90640 mlx;
float frame[32 * 24];          // 768 pixels (24x32)

// ----------- Advanced presence detection parameters (tunable) -----------
const float Z_THRESHOLD          = 2.3f;   // z-score threshold: higher = more conservative
const float MIN_GLOBAL_DELTA     = 3.0f;   // (maxT - mean) difference required to consider heat significant
const int   MIN_PIXELS_PER_BLOCK = 2;      // minimum hot pixels required within each 4x4 block
const int   MIN_HOT_BLOCKS       = 3;      // minimum number of blocks meeting criteria to count as presence
// Temporal smoothing
const int   MAX_SCORE            = 6;      // upper bound on stability score
const int   STABLE_SCORE         = 3;      // threshold above which presence is confirmed
// -----------------------------------------------------------------------

int presenceScore = 0;           // score accumulator for temporal smoothing

// Image size and block partitioning
const int IMG_W = 32;
const int IMG_H = 24;
const int BLOCK_W = 8;
const int BLOCK_H = 6;
const int BLOCK_SIZE_X = IMG_W / BLOCK_W;  // 4 pixels per block (horizontal)
const int BLOCK_SIZE_Y = IMG_H / BLOCK_H;  // 4 pixels per block (vertical)

// Single-frame advanced human detection
bool detectHumanFrame(const float *f) {
  // 1. Compute mean, standard deviation, and max temperature
  float sum  = 0.0f;
  float var  = 0.0f;
  float maxT = -1000.0f;

  for (int i = 0; i < IMG_W * IMG_H; i++) {
    float t = f[i];
    sum += t;
    if (t > maxT) maxT = t;
  }

  float mean = sum / (IMG_W * IMG_H);

  // Compute variance
  for (int i = 0; i < IMG_W * IMG_H; i++) {
    float diff = f[i] - mean;
    var += diff * diff;
  }

  float stdDev = sqrtf(var / (IMG_W * IMG_H) + 1e-6f);

  // 2. Mark candidate "hot pixels" based on z-score
  static uint8_t hotMask[IMG_W * IMG_H];  // 768 bytes OK for MKR1010
  int totalHotPixels = 0;

  for (int i = 0; i < IMG_W * IMG_H; i++) {
    float z = (f[i] - mean) / stdDev;
    if (z > Z_THRESHOLD) {
      hotMask[i] = 1;
      totalHotPixels++;
    } else {
      hotMask[i] = 0;
    }
  }

  // 3. Spatial clustering using 4x4 blocks
  int hotBlocks = 0;

  for (int by = 0; by < BLOCK_H; by++) {
    for (int bx = 0; bx < BLOCK_W; bx++) {
      int blockHot = 0;

      // Iterate through all pixels in a block
      for (int dy = 0; dy < BLOCK_SIZE_Y; dy++) {
        for (int dx = 0; dx < BLOCK_SIZE_X; dx++) {
          int x = bx * BLOCK_SIZE_X + dx;
          int y = by * BLOCK_SIZE_Y + dy;
          int idx = y * IMG_W + x;
          if (hotMask[idx]) {
            blockHot++;
          }
        }
      }

      if (blockHot >= MIN_PIXELS_PER_BLOCK) {
        hotBlocks++;
      }
    }
  }

  // 4. Global sanity check: prevents false positives when entire room warms evenly
  bool framePresence = (hotBlocks >= MIN_HOT_BLOCKS) &&
                       ((maxT - mean) > MIN_GLOBAL_DELTA);

  // Debug output
  Serial.print("mean=");
  Serial.print(mean, 2);
  Serial.print(" max=");
  Serial.print(maxT, 2);
  Serial.print(" hotPixels=");
  Serial.print(totalHotPixels);
  Serial.print(" hotBlocks=");
  Serial.print(hotBlocks);
  Serial.print(" framePresence=");
  Serial.println(framePresence ? "YES" : "NO");

  return framePresence;
}

// Multi-frame temporal smoothing + final decision
bool updatePresence() {
  int status = mlx.getFrame(frame);
  if (status != 0) {
    Serial.print("getFrame error: ");
    Serial.println(status);
    return false;
  }

  bool framePresence = detectHumanFrame(frame);

  // Smoothing: ramp score up/down based on consecutive detections
  if (framePresence) {
    if (presenceScore < MAX_SCORE) presenceScore++;
  } else {
    if (presenceScore > 0) presenceScore--;
  }

  bool presence = (presenceScore >= STABLE_SCORE);

  Serial.print("presenceScore=");
  Serial.print(presenceScore);
  Serial.print(" -> presence=");
  Serial.println(presence ? "YES" : "NO");

  return presence;
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("MLX90640 Advanced Human Presence (MKR1010)");

  Wire.begin();
  Wire.setClock(400000);       // high-speed I2C

  if (!mlx.begin(0x33, &Wire)) {
    Serial.println("MLX90640 not found! Check wiring.");
    while (1) delay(1000);
  }

  Serial.println("MLX90640 detected.");

  // Sensor config
  mlx.setMode(MLX90640_INTERLEAVED);
  mlx.setResolution(MLX90640_ADC_18BIT);
  mlx.setRefreshRate(MLX90640_4_HZ);  // approx. 4 FPS
}

void loop() {
  bool human = updatePresence();

  // Hook output here (fan / curtain / MQTT etc.)
  // if (human) { ... } else { ... }

  delay(250);  // matches ~4 FPS sensor rate
}
