#ifndef __SENSORS_H
#define __SENSORS_H

#include <config.h>
#include <delay.h>
#include <encoders.h>
#include <leds.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <stdint.h>

#define NUM_SENSORES 4

#define SENSOR_FRONT_LEFT_WALL_ID 0
#define SENSOR_FRONT_RIGHT_WALL_ID 1
#define SENSOR_SIDE_LEFT_WALL_ID 2
#define SENSOR_SIDE_RIGHT_WALL_ID 3

uint8_t *get_sensors(void);
uint8_t get_sensors_num(void);
void get_sensors_raw(uint16_t *on, uint16_t *off);
void sm_emitter_adc(void);

void sensors_calibration(void);

uint16_t get_sensor_raw(uint8_t pos, bool on);
uint16_t get_sensor_raw_filter(uint8_t pos);

void update_sensors_magics(void);
uint16_t get_sensor_filtered(uint8_t pos);
uint16_t get_sensor_linearized(uint8_t pos);
uint16_t get_sensor_distance(uint8_t pos);

#endif