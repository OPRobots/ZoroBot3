#ifndef __CONTROL_H
#define __CONTROL_H

#include <config.h>
#include <battery.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <math.h>

#define BASE_LINEAR_ACCEL 3000

// #define KP_LINEAR 0.0000
// #define KI_LINEAR 0.0009
// #define KD_LINEAR 0.0050
#define KP_LINEAR 0.0002
#define KI_LINEAR 0.0007
#define KD_LINEAR 0

#define KP_ANGULAR 0.2
#define KI_ANGULAR 0.007
#define KD_ANGULAR 0.00


bool is_competicion_iniciada(void);
void set_competicion_iniciada(bool state);

void set_target_linear_speed(int32_t linear_speed);
void set_ideal_angular_speed(float angular_speed);

void control_loop(void);
void control_debug(void);

#endif