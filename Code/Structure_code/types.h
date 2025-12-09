#pragma once

// 所有传感器读出来的数据，都塞进这个结构体
struct SensorData {
  // 先只放 MLX90640 相关的
  bool  occupied;      // 是否有人
  float maxTempC;      // 房间最高温（所有像素里的最大值）
  float avgTempC;      // 平均温度
};

// 所有执行器需要的“目标状态”
struct SystemState {
  bool fanOn;          // 电风扇
  bool buzzerOn;       // 普通提示蜂鸣
  bool alarmOn;        // 严重报警（火灾等）
  int  curtainAngle;   // 窗帘舵机角度
  bool justReturned;   // “刚刚有人回来了”的标记
};
