#pragma once

#include "stdlib.h"
#include "stdint.h"

void FIRInit(void);
float FIR_Filter(float in);
int heartbeat_check(float value);
double calc_heartbeat_rate(int is_heartbeat);
