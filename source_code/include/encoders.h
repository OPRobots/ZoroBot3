#ifndef __ENCODERS_H
#define __ENCODERS_H

#include <libopencm3/stm32/timer.h>

#include <math.h>
#include <stdint.h>

#include "config.h"
#include "constants.h"
#include "setup.h"

int32_t get_encoder_left_ticks(void);
int32_t get_encoder_right_ticks(void);

int32_t get_encoder_left_micrometers(void);
int32_t get_encoder_right_micrometers(void);
int32_t get_encoder_avg_micrometers(void);

int32_t get_encoder_left_millimeters(void);
int32_t get_encoder_right_millimeters(void);
int32_t get_encoder_avg_millimeters(void);

float get_encoder_left_speed(void);
float get_encoder_right_speed(void);
float get_encoder_avg_speed(void);

float get_encoder_angular_speed(void);

int32_t max_likelihood_counter_diff(uint16_t now, uint16_t before);
void update_encoder_readings(void);

#endif
