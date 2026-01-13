#pragma once

// === 执行器引脚（原来那几行你保持不变） ===
const int PIN_FAN    = 5;
const int PIN_BUZZER = 6;
const int PIN_SERVO  = 7;

// === MLX90640：人体检测参数 ===

// 认为“像素属于人体”的温度区间
const float MLX_HUMAN_MIN_TEMP   = 28.0;   // ℃ 下限
const float MLX_HUMAN_MAX_TEMP   = 37.5;   // ℃ 上限（略高一点防抖）

// 一帧中，温度落在上述区间的像素 ≥ 多少个，就认为“这一帧有人”
const int   MLX_MIN_HUMAN_PIXELS = 4;      // 你可以后面根据实际调 3~10

// 多帧平滑：至少连续多少帧有人 → occupied = true
const int   MLX_FRAMES_ON        = 2;      // 2 帧就判定有人（比较敏感）

// 至少连续多少帧没人 → occupied = false
const int   MLX_FRAMES_OFF       = 5;      // 5 帧都没人才判定为空
