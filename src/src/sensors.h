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

uint8_t *get_sensors();
uint8_t get_sensors_num();
volatile uint16_t *get_sensors_raw();
uint16_t get_sensor_raw(uint8_t pos);
#endif