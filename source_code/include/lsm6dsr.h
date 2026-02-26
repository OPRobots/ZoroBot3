#ifndef LSM6DSR_H
#define LSM6DSR_H

#include <stdint.h>

#include "buttons.h"
#include "constants.h"
#include "delay.h"
#include "eeprom.h"
#include "math.h"
#include "motors.h"
#include "setup.h"
#include <lsm6dsr_reg.h>

#define GYRO_SENSITIVITY_1000DPS 35.0f
#define GYRO_SENSITIVITY_2000DPS 70.0f
#define GYRO_SENSITIVITY_4000DPS 140.0f

void lsm6dsr_init(void);
void lsm6dsr_reload_config(void);

uint8_t lsm6dsr_who_am_i(void);

void lsm6dsr_gyro_z_calibration(void);
void lsm6dsr_load_eeprom(void);

void lsm6dsr_update(void);
float lsm6dsr_get_gyro_z_raw(void);
float lsm6dsr_get_gyro_z_radps(void);
float lsm6dsr_get_gyro_z_dps(void);
float lsm6dsr_get_gyro_z_degrees(void);
void lsm6dsr_set_gyro_z_degrees(float deg);

float get_offset_z(void);
uint8_t get_current_full_scale_dps(void);

#endif // LSM6DSR_H