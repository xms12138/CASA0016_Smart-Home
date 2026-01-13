#include "actuators.h"

void actuatorsInit() {
  pinMode(PIN_FAN, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_SERVO, OUTPUT);

  digitalWrite(PIN_FAN, LOW);
  digitalWrite(PIN_BUZZER, LOW);
}

void applyActuators(const SystemState &st, unsigned long now) {
  (void)now;

  // 现在先只打印，不认真控制舵机
  Serial.print("[ACT] fan=");
  Serial.print(st.fanOn);
  Serial.print(" buzzer=");
  Serial.print(st.buzzerOn);
  Serial.print(" alarm=");
  Serial.print(st.alarmOn);
  Serial.print(" curtain=");
  Serial.println(st.curtainAngle);

  digitalWrite(PIN_FAN,    st.fanOn    ? HIGH : LOW);
  digitalWrite(PIN_BUZZER, st.buzzerOn ? HIGH : LOW);
}
