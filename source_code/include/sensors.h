#ifndef __SENSORS_H
#define __SENSORS_H

#include <config.h>
#include <delay.h>
#include <eeprom.h>
#include <encoders.h>
#include <leds.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <stdint.h>

#define NUM_AUX_ADC_CHANNELS 4
#define AUX_BATTERY_ID 2
#define AUX_CURRENT_LEFT_ID 0
#define AUX_CURRENT_RIGHT_ID 3
#define AUX_MENU_BTN_ID 1


#define NUM_SENSORES 4
#define SENSOR_FRONT_LEFT_WALL_ID 0
#define SENSOR_FRONT_RIGHT_WALL_ID 1
#define SENSOR_SIDE_LEFT_WALL_ID 2
#define SENSOR_SIDE_RIGHT_WALL_ID 3

struct walls {
  bool front;
  bool left;
  bool right;
};

struct front_sensors_distance_calibration {
  int16_t close_offset;
  float close_slope;
  int16_t close_intercept;
  int16_t close_low_raw;
  int16_t close_high_raw;
  int16_t far_offset;
  float far_slope;
  int16_t far_intercept;
  int16_t far_low_raw;
  int16_t far_high_raw;
  int16_t offset;
  float slope;
  int16_t intercept;
  int16_t low_raw;
  int16_t high_raw;
};

struct side_sensors_distance_calibration {
  int16_t offset;
  float slope;
  int16_t intercept;
  int16_t low_linearized;
  int16_t high_linearized;
};

uint8_t *get_aux_adc_channels(void);
uint8_t get_aux_adc_channels_num(void);
volatile uint16_t *get_aux_adc_raw(void);
uint16_t get_aux_raw(uint8_t pos);


uint8_t *get_sensors(void);
uint8_t get_sensors_num(void);
void get_sensors_raw(uint16_t *on, uint16_t *off);
void sm_emitter_adc(void);

void front_sensors_calibration(void);
void side_sensors_calibration(void);
void sensors_load_eeprom(void);

uint16_t get_sensor_raw(uint8_t pos, bool on);
uint16_t get_sensor_raw_filter(uint8_t pos);

bool left_wall_detection(void);
bool right_wall_detection(void);
bool front_wall_detection(void);
struct walls get_walls(void);

void update_sensors_magics(void);
void update_side_sensors_leds(void);
uint16_t get_sensor_filtered(uint8_t pos);
uint16_t get_sensor_linearized(uint8_t pos);
uint16_t get_sensor_distance(uint8_t pos);
uint16_t get_front_wall_distance(void);

int16_t get_side_sensors_close_error(void);
int16_t get_side_sensors_far_error(void);
int16_t get_front_sensors_angle_error(void);
int16_t get_front_sensors_diagonal_error(void);

#endif