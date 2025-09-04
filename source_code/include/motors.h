#ifndef __MOTORS_H
#define __MOTORS_H

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include <config.h>
#include <constants.h>
#include <delay.h>
#include <utils.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

void set_motors_enable(bool enabled);
void set_motors_speed(float velI, float velD);
void set_motors_brake(void);
void set_motors_pwm(int32_t pwm_left, int32_t pwm_right);
void set_fan_speed(uint8_t vel);

void reset_motors_saturated(void);
bool is_motor_saturated(void);
bool is_motor_pwm_saturated(void);
bool is_motor_angle_saturated(void);
uint32_t get_motors_saturated_ms(void);
void set_check_motors_saturated_enabled(bool enabled);

#endif