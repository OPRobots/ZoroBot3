#ifndef __CONTROL_H
#define __CONTROL_H

#ifndef MMSIM_ENABLED
#include "battery.h"
#include "config.h"
#include "constants.h"
#include "debug.h"
#include "macroarray.h"
#include "menu.h"
#endif
#include "move.h"

#ifndef MMSIM_ENABLED
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <math.h>
#else
#include <stdbool.h>
#include <stdint.h>
#endif

bool is_race_started(void);
void set_race_started(bool state);
int8_t check_start_run(void);
void set_control_debug(bool state);

void set_side_sensors_close_correction(bool enabled);
void set_side_sensors_far_correction(bool enabled);
void set_front_sensors_correction(bool enabled);
bool is_front_sensors_correction_enabled(void);
void set_front_sensors_diagonal_correction(bool enabled);
void disable_sensors_correction(void);
void reset_control_errors(void);
void reset_control_speed(void);
void reset_control_all(void);

void set_target_linear_speed(int32_t linear_speed);
int32_t get_ideal_linear_speed(void);
void set_ideal_angular_speed(float angular_speed);
float get_ideal_angular_speed(void);

void set_target_fan_speed(int32_t fan_speed, int32_t ms);

void control_loop(void);

void keep_z_angle(void);

#endif