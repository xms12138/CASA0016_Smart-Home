#include <Arduino.h>
#include "logic.h"

void logicInit() {
  // 现在暂时没有内部状态
}

void logicUpdate(const SensorData &s, SystemState &st, unsigned long now) {
  (void)now;

  st.fanOn        = false;
  st.buzzerOn     = false;
  st.alarmOn      = false;
  st.curtainAngle = 0;
  st.justReturned = false;

  Serial.print("[LOGIC] occ=");
  Serial.print(s.occupied ? "Y" : "N");
  Serial.print(" maxT=");
  Serial.print(s.maxTempC, 2);
  Serial.print("C avgT=");
  Serial.print(s.avgTempC, 2);
  Serial.println("C");
}
