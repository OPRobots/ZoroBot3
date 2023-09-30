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

// Indica qué pared leen los sensores.
// Se usan en la máquina de estados de lectura de sensores
#define SENSOR_SIDE_LEFT_ID 0   // Este número NO DEBE CAMBIARSE
#define SENSOR_SIDE_RIGHT_ID 1  // Este número NO DEBE CAMBIARSE
#define SENSOR_FRONT_LEFT_ID 2  // Este número NO DEBE CAMBIARSE
#define SENSOR_FRONT_RIGHT_ID 3 // Este número NO DEBE CAMBIARSE

uint8_t *get_sensors();
uint8_t get_sensors_num();
void get_sensors_raw(uint16_t *on, uint16_t *off);
uint16_t get_sensor_raw(uint8_t pos, bool on);
float get_sensor_log(uint8_t pos);
float sensors_raw_log(uint16_t on, uint16_t off);
uint16_t get_sensor_raw_filter(uint8_t pos);

void filtro_sensores();

int sensor0_analog();
int sensor1_analog();
int sensor2_analog();
int sensor3_analog();

bool sensor0();
bool sensor1();
bool sensor2();
bool sensor3();

void sm_emitter_adc(void);
#endif