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

#define SENSOR_SIDE_LEFT_ID 0 //1
#define SENSOR_SIDE_RIGHT_ID 1 //4
#define SENSOR_FRONT_LEFT_ID 2 //2
#define SENSOR_FRONT_RIGHT_ID 3 //3

uint8_t *get_sensors();
uint8_t get_sensors_num();
void get_sensors_raw(uint16_t *on, uint16_t *off);
uint16_t get_sensor_raw(uint8_t pos, bool on);
float get_sensor_log(uint8_t pos);
float sensors_raw_log(uint16_t on, uint16_t off);
uint16_t get_sensor_raw_filter(uint8_t pos);

void sm_emitter_adc(void);
#endif