#ifndef __CONTROL_H
#define __CONTROL_H

#include <config.h>
#include <battery.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <math.h>

#define BASE_LINEAR_ACCEL 1000

#define KP_LINEAR 0.2
#define KD_LINEAR 2
#define KP_ANGULAR 0
#define KD_ANGULAR 0


bool is_competicion_iniciada(void);
void set_competicion_iniciada(bool state);

void set_target_linear_speed(int32_t linear_speed);
void set_ideal_angular_speed(float angular_speed);

void control_loop(void);
void control_debug(void);

#endif