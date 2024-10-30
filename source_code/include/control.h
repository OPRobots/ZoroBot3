#ifndef __CONTROL_H
#define __CONTROL_H

#include <battery.h>
#include <config.h>
#include <constants.h>
#include <macroarray.h>
#include <menu.h>
#include <move.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <math.h>

bool is_race_started(void);
void set_race_started(bool state);
int8_t check_start_run(void);
int8_t check_side_front_sensors(void);
void check_start_module_run(void);
void set_control_debug(bool state);

void set_side_sensors_close_correction(bool enabled);
void set_side_sensors_far_correction(bool enabled);
void set_front_sensors_correction(bool enabled);
void set_front_sensors_diagonal_correction(bool enabled);
void disable_sensors_correction(void);
void reset_control_errors(void);
void reset_control_speed(void);
void reset_control_all(void);

void set_target_linear_speed(int32_t linear_speed);
void force_target_linear_speed(int32_t linear_speed);
int32_t get_ideal_linear_speed(void);
void set_ideal_angular_speed(float angular_speed);

void set_target_fan_speed(int32_t fan_speed, int32_t ms);

void control_loop(void);

#endif