#ifndef __MOTORS_H
#define __MOTORS_H

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include <config.h>
#include <delay.h>
#include <utils.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

void set_motors_speed(float velI, float velD);
void set_fan_speed(uint8_t vel);

#endif