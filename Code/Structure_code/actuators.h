#pragma once
#include <Arduino.h>
#include "types.h"
#include "config.h"

void actuatorsInit();
void applyActuators(const SystemState &st, unsigned long now);
