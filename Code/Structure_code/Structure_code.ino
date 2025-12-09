#include <Arduino.h>
#include "types.h"
#include "config.h"
#include "sensors.h"
#include "logic.h"
#include "actuators.h"

SensorData  gSensors;
SystemState gState;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== SmartHome + MLX90640 Start ===");

  sensorsInit();
  logicInit();
  actuatorsInit();
}

void loop() {
  unsigned long now = millis();

  // 1. 读所有传感器（现在只有 MLX90640）
  sensorsRead(gSensors);

  // 2. 根据传感器数据，更新系统状态（现在只打印）
  logicUpdate(gSensors, gState, now);

  // 3. 把状态应用到执行器（现在也只是打印）
  applyActuators(gState, now);

  delay(500);
}
