#ifndef __BASIC_ALGORITHM_H
#define __BASIC_ALGORITHM_H

#include <config.h>
#include <buttons.h>
#include <motors.h>
#include <usart.h>
#include <libopencm3/stm32/gpio.h>


void basic_algorithm_config();
void start_from_front_sensor();
void start_from_ms();
void basic_algorithm_loop();

#endif