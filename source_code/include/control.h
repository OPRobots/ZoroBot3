#ifndef __CONTROL_H
#define __CONTROL_H

#include <battery.h>
#include <config.h>
#include <constants.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <macroarray.h>
#include <math.h>

bool is_competicion_iniciada(void);
void set_competicion_iniciada(bool state);
int8_t check_iniciar_competicion(void);

void set_side_sensors_close_correction(bool enabled);
void set_side_sensors_far_correction(bool enabled);
void set_front_sensors_correction(bool enabled);

void set_target_linear_speed(int32_t linear_speed);
int32_t get_ideal_linear_speed(void);
void set_ideal_angular_speed(float angular_speed);

void control_loop(void);

#endif