#ifndef LSM6DSR_H
#define LSM6DSR_H

#include <stdint.h>

#ifndef MMSIM_ENABLED
#include <lsm6dsr_reg.h>

#include "buttons.h"
#include "delay.h"
#include "eeprom.h"
#include "motors.h"
#include "setup.h"
#else
#define LSM6DSR_1000dps 8
#define LSM6DSR_2000dps 12
#define LSM6DSR_4000dps 1
#endif

#include "constants.h"
#include "math.h"

#define MPU_FULL_SCALE_COUNT 3
enum MPU_FULL_SCALE_DPS {
  MPU_FULL_SCALE_1000DPS = LSM6DSR_1000dps,
  MPU_FULL_SCALE_2000DPS = LSM6DSR_2000dps,
  MPU_FULL_SCALE_4000DPS = LSM6DSR_4000dps,
};
enum MPU_SENSITIVITY_DPS {
  MPU_SENSITIVITY_1000DPS = 35,
  MPU_SENSITIVITY_2000DPS = 70,
  MPU_SENSITIVITY_4000DPS = 140,
};

#define MPU_DATA_LENGTH MPU_FULL_SCALE_COUNT

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