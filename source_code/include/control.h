#ifndef __CONTROL_H
#define __CONTROL_H

#include <battery.h>
#include <config.h>
#include <constants.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <math.h>


#define CONTROL_DEBUG_LENGTH 1000
#define CONTROL_DEBUG_SIZE 8
enum CONTROL_DEBUG {
  TARGET_LINEAR_SPEED = 0,
  IDEAL_LINEAR_SPEED = 1,
  MEASURED_LINEAR_SPEED = 2,
  IDEAL_ANGULAR_SPEED = 3,
  MEASURED_ANGULAR_SPEED = 4,
  PWM_LEFT = 5,
  PWM_RIGHT = 6,
  BATTERY_LEVEL = 7
};

bool is_competicion_iniciada(void);
void set_competicion_iniciada(bool state);

void set_target_linear_speed(int32_t linear_speed);
void set_ideal_angular_speed(float angular_speed);

void control_loop(void);
void control_debug(void);

#endif