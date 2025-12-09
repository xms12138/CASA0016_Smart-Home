#pragma once
#include "types.h"

void logicInit();
void logicUpdate(const SensorData &s, SystemState &st, unsigned long now);
