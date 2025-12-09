#pragma once
#include <Arduino.h>
#include "types.h"

// 初始化所有传感器（现在先只管 MLX90640）
void sensorsInit();

// 读取所有传感器，把结果写入 data
void sensorsRead(SensorData &data);
