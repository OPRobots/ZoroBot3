#ifndef __BASIC_ALGORITHM_H
#define __BASIC_ALGORITHM_H

#include <config.h>
#include <buttons.h>
#include <motors.h>
#include <usart.h>
#include <libopencm3/stm32/gpio.h>

#define BASIC_ALGORITHM_KP 0.005
#define BASIC_ALGORITHM_KI 0
#define BASIC_ALGORITHM_KD 0.080

#define FRONT_SENSOR_THRESHOLD 1470

#define MIN_U_TURN_CECKES 100

// Indica los millis a partir de los cuales realiza un cambio de pared de referencia (0 -> sin cambio)
#define REFERENCE_WALL_CHANGE_LENGTH 0
#define MILLIS_REFERENCE_WALL_CHANGE_1 5000
#define MILLIS_REFERENCE_WALL_CHANGE_2 MILLIS_REFERENCE_WALL_CHANGE_1 + 5000
#define MILLIS_REFERENCE_WALL_CHANGE_3 MILLIS_REFERENCE_WALL_CHANGE_2 + 5000

void check_start_front_sensor();
void basic_algorithm_init();
void basic_algorithm_loop();

#endif