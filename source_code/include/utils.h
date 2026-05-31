#ifndef __UTILS_H
#define __UTILS_H

#include <stdint.h>

#include "constants.h"

#define LOG_LINEARIZATION_TABLE_STEP 4
#define LOG_LINEARIZATION_TABLE_SIZE (ADC_RESOLUTION / LOG_LINEARIZATION_TABLE_STEP)

float get_ln_value(uint16_t index);
float map(float x, float in_min, float in_max, float out_min, float out_max);
float constrain(float x, float min, float max);

#endif