#include "sensors.h"
#include "config.h"

#include <Wire.h>
#include <Adafruit_MLX90640.h>

static Adafruit_MLX90640 mlx;
static float mlxFrame[32 * 24];
// 占用状态和平滑用的计数器
static bool occupiedState       = false;
static int  framesWithHuman     = 0;
static int  framesWithoutHuman  = 0;
void sensorsInit() {
  Serial.println("[SENS] Init start");

  Wire.begin();
  Wire.setClock(400000);

  if (!mlx.begin(0x33, &Wire)) {
    Serial.println("[SENS] ❌ MLX90640 not found!");
  } else {
    Serial.println("[SENS] ✅ MLX90640 detected.");
    mlx.setMode(MLX90640_INTERLEAVED);
    mlx.setResolution(MLX90640_ADC_18BIT);
    mlx.setRefreshRate(MLX90640_4_HZ);
  }

  Serial.println("[SENS] Init done");
}

void sensorsRead(SensorData &data) {
  // 默认值，防止本帧失败
  data.occupied = occupiedState;
  data.maxTempC = 0.0f;
  data.avgTempC = 0.0f;

  int status = mlx.getFrame(mlxFrame);
  if (status != 0) {
    Serial.print("[SENS] MLX90640 getFrame error: ");
    Serial.println(status);
    // 读失败时沿用上一帧的 occupied 状态
    return;
  }

  // 1. 先算 max / avg（方便你观察环境情况，也可以用于以后其它逻辑）
  float sumT = 0.0f;
  float maxT = -1000.0f;

  for (int i = 0; i < 32 * 24; ++i) {
    float t = mlxFrame[i];
    sumT += t;
    if (t > maxT) maxT = t;
  }

  float avgT = sumT / (32.0f * 24.0f);
  data.maxTempC = maxT;
  data.avgTempC = avgT;

  // 2. 数“人体像素”：温度在 [MLX_HUMAN_MIN_TEMP, MLX_HUMAN_MAX_TEMP] 区间内
  int humanPixelCount = 0;

  for (int i = 0; i < 32 * 24; ++i) {
    float t = mlxFrame[i];
    if (t >= MLX_HUMAN_MIN_TEMP && t <= MLX_HUMAN_MAX_TEMP) {
      humanPixelCount++;
    }
  }

  // 3. 判断这一帧是否“有人”
  bool thisFrameHasHuman = (humanPixelCount >= MLX_MIN_HUMAN_PIXELS);

  // 4. 多帧平滑：更新计数，更新 occupiedState
  if (thisFrameHasHuman) {
    framesWithHuman++;
    framesWithoutHuman = 0;

    if (!occupiedState && framesWithHuman >= MLX_FRAMES_ON) {
      occupiedState = true;
    }
  } else {
    framesWithoutHuman++;
    framesWithHuman = 0;

    if (occupiedState && framesWithoutHuman >= MLX_FRAMES_OFF) {
      occupiedState = false;
    }
  }

  data.occupied = occupiedState;

  // 5. 串口调试输出
  Serial.print("[SENS] maxT=");
  Serial.print(maxT, 2);
  Serial.print(" avgT=");
  Serial.print(avgT, 2);
  Serial.print(" humanPix=");
  Serial.print(humanPixelCount);
  Serial.print(" framesOn=");
  Serial.print(framesWithHuman);
  Serial.print(" framesOff=");
  Serial.print(framesWithoutHuman);
  Serial.print(" occupied=");
  Serial.println(occupiedState ? "YES" : "NO");
}

