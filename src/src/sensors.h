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


#define SENSOR_SIDE_LEFT_ID 0
#define SENSOR_SIDE_RIGHT_ID 1
#define SENSOR_FRONT_LEFT_ID 2
#define SENSOR_FRONT_RIGHT_ID 3

uint8_t *get_sensors();
uint8_t get_sensors_num();
volatile uint16_t *get_sensors_raw();
uint16_t get_sensor_raw(uint8_t pos, bool on);

void sm_emitter_adc(void);
#endif