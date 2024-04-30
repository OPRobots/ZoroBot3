#ifndef __ENCODERS_H
#define __ENCODERS_H

#include <libopencm3/stm32/timer.h>

#include <math.h>
#include <stdint.h>

#include "config.h"
#include "constants.h"
#include "setup.h"

float get_micrometers_per_tick(void);
void set_micrometers_per_tick(float value);
float get_wheels_separation(void);
void set_wheels_separation(float value);
int32_t get_encoder_total_left_ticks(void);
int32_t get_encoder_total_right_ticks(void);
int32_t get_encoder_total_left_micrometers(void);
int32_t get_encoder_total_right_micrometers(void);
int32_t get_encoder_total_average_micrometers(void);
int32_t get_encoder_total_left_millimeters(void);
int32_t get_encoder_total_right_millimeters(void);
int32_t get_encoder_total_average_millimeters(void);

float get_encoder_left_speed(void);
float get_encoder_right_speed(void);
float get_encoder_avg_speed(void);
float get_encoder_angular_speed(void);

float get_encoder_avg_micrometers(void);
float get_encoder_avg_millimeters(void);
float get_encoder_curernt_angle(void);

int32_t max_likelihood_counter_diff(uint16_t now, uint16_t before);
void update_encoder_readings(void);

#endif
