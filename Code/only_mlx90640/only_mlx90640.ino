#include <Wire.h>
#include <Adafruit_MLX90640.h>

Adafruit_MLX90640 mlx;
float frame[32 * 24];

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("MLX90640 + MKR1010 Matrix Output Test");

  Wire.begin();               // MKR1010 的 SDA/SCL 自动分配
  Wire.setClock(400000);      // 400kHz I2C，更稳定

  if (!mlx.begin(0x33, &Wire)) {
    Serial.println("❌ MLX90640 not found! Check wiring.");
    while (1) delay(1000);
  }
  Serial.println("✅ MLX90640 detected!");

  mlx.setMode(MLX90640_INTERLEAVED);
  mlx.setResolution(MLX90640_ADC_18BIT);
  mlx.setRefreshRate(MLX90640_4_HZ);  // 稳定刷新率
}

void loop() {
  int status = mlx.getFrame(frame);

  if (status != 0) {
    Serial.print("getFrame error: ");
    Serial.println(status);
    return;  // 出错就跳过当前帧
  }

  // 输出 24x32 的温度矩阵
  Serial.println("FRAME_START");
  for (int y = 0; y < 24; y++) {
    for (int x = 0; x < 32; x++) {
      Serial.print(frame[y * 32 + x], 1);  // 取一位小数
      if (x < 31) Serial.print(",");
    }
    Serial.println();
  }
  Serial.println("FRAME_END\n");

  delay(250); // 对应 4 FPS，可调
}
