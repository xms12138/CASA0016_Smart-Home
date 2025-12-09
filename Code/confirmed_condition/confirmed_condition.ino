#include <Wire.h>
#include <Adafruit_MLX90640.h>

Adafruit_MLX90640 mlx;
float frame[32 * 24];   // 768 个像素

// ----------- 人体检测参数，可以之后微调 -----------
const float TEMP_DELTA_THRESHOLD = 3.5;  // 阈值：高于平均温度多少度算“热”
const int   MIN_HOT_PIXELS       = 8;    // 至少多少个“热像素”才算有人
// -------------------------------------------------

// 检测当前帧是否有人
bool detectHuman(const float *f) {
  float sum = 0.0;
  float minT = 1000.0;
  float maxT = -1000.0;

  // 先遍历一遍，求平均 / 最小 / 最大温度
  for (int i = 0; i < 32 * 24; i++) {
    float t = f[i];
    sum += t;
    if (t < minT) minT = t;
    if (t > maxT) maxT = t;
  }

  float avg = sum / (32.0 * 24.0);       // 平均温度
  float threshold = avg + TEMP_DELTA_THRESHOLD;

  // 再遍历一遍，统计高于阈值的像素
  int hotPixels = 0;
  for (int i = 0; i < 32 * 24; i++) {
    if (f[i] > threshold) {
      hotPixels++;
    }
  }

  bool presence = (hotPixels >= MIN_HOT_PIXELS) &&
                  ((maxT - avg) > TEMP_DELTA_THRESHOLD);

  // 为了调试，打印一些信息
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

  Wire.begin();              // MKR1010 默认 SDA/SCL
  Wire.setClock(400000);     // 400kHz I2C

  if (!mlx.begin(0x33, &Wire)) {
    Serial.println("❌ MLX90640 not found! Check wiring.");
    while (1) delay(1000);
  }

  Serial.println("✅ MLX90640 detected!");

  mlx.setMode(MLX90640_INTERLEAVED);
  mlx.setResolution(MLX90640_ADC_18BIT);
  mlx.setRefreshRate(MLX90640_4_HZ);  // 4Hz 刷新
}

void loop() {
  int status = mlx.getFrame(frame);

  if (status != 0) {
    Serial.print("getFrame error: ");
    Serial.println(status);
    return;
  }

  // 调用检测函数
  bool human = detectHuman(frame);

  // 如果你还想保留原来的矩阵输出，可以把下面这段注释打开
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

  // 稍微等一下，再采下一帧
  delay(250);  // 约 4 帧/秒
}
