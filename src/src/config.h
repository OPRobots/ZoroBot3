#ifndef __CONFIG_H
#define __CONFIG_H

#include <buttons.h>
#include <stdio.h>


/** Calibration constants for sensors */
#define SENSOR_SIDE_LEFT_A 2.806
#define SENSOR_SIDE_LEFT_B 0.287
#define SENSOR_SIDE_RIGHT_A 2.327
#define SENSOR_SIDE_RIGHT_B 0.231
#define SENSOR_FRONT_LEFT_A 2.609
#define SENSOR_FRONT_LEFT_B 0.242
#define SENSOR_FRONT_RIGHT_A 2.713
#define SENSOR_FRONT_RIGHT_B 0.258

/** Maze dimensions */
#define CELL_DIMENSION 0.18
#define CELL_DIAGONAL 0.1273
#define WALL_WIDTH 0.012
#define MIDDLE_MAZE_DISTANCE ((CELL_DIMENSION - WALL_WIDTH) / 2.)

/** Constantes matemáticas */
#define PI 3.1415

/** Constantes del STM32F4 */
#define SYSCLK_FREQUENCY_HZ 168000000
#define SYSTICK_FREQUENCY_HZ 1000
#define MICROMETERS_PER_METER 1000000
#define MICROSECONDS_PER_SECOND 1000000

/** Constantes de PWM */
#define LEDS_MAX_PWM 1024
#define MOTORES_MAX_PWM 1024

/** Constantes del Robot */
#define MICROMETERS_PER_TICK 9.09
#define WHEELS_SEPARATION 0.1169

/** Constantes ADC */
#define ADC_RESOLUTION 4096
#define ADC_LSB (3.3 / ADC_RESOLUTION)

/** Constantes del Divisor de Voltage */
#define VOLT_DIV_FACTOR 2.781 // 2.74
#define BATTERY_HIGH_LIMIT_VOLTAGE 8.4
#define BATTERY_LOW_LIMIT_VOLTAGE 7.4

/** Constantes de Modo RUN */
#define CONFIG_RUN_RACE 1
#define CONFIG_RUN_DEBUG 0

void set_all_configs(void);
uint16_t get_config_run(void);

#endif