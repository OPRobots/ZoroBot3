#ifndef __BASIC_ALGORITHM_H
#define __BASIC_ALGORITHM_H

#include <config.h>
#include <buttons.h>
#include <motors.h>
#include <usart.h>
#include <libopencm3/stm32/gpio.h>

#define BASIC_ALGORITHM_KP 0.005
#define BASIC_ALGORITHM_KI 0
#define BASIC_ALGORITHM_KD 0.030

#define FRONT_SENSOR_THRESHOLD 900

void basic_algorithm_init();
void basic_algorithm_loop();

#endif