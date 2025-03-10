#ifndef CALIBRATIONS_H
#define CALIBRATIONS_H

#include <stdint.h>

#include <delay.h>
#include <eeprom.h>
#include <mpu6500.h>
#include <move.h>
#include <sensors.h>

#define CALIBRATE_NONE 0
#define CALIBRATE_GYRO_Z 1
#define CALIBRATE_SIDE_SENSORS_OFFSET 2
#define CALIBRATE_FRONT_SENSORS 3
#define CALIBRATE_STORE_EEPROM 4

void calibrate_from_config(uint8_t type);

#endif