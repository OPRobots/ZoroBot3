#ifndef __EEPROM_H
#define __EEPROM_H

#include <libopencm3/stm32/flash.h>

#include <delay.h>
#include <leds.h>
#include <menu.h>
#include <sensors.h>

#define EEPROM_SECTOR 11
#define EEPROM_BASE_ADDRESS (0x080E0000)

#define DATA_LENGTH (2 + NUM_SENSORES + (2*SENSOR_FRONT_CALIBRATION_READINGS))

#define DATA_INDEX_GYRO_Z 0
#define DATA_INDEX_SENSORS_OFFSETS (DATA_INDEX_GYRO_Z + 2)
#define DATA_INDEX_FRONT_SENSORS_CALIBRATION (DATA_INDEX_SENSORS_OFFSETS + NUM_SENSORES)

void eeprom_save(void);
void eeprom_load(void);
void eeprom_clear(void);
void eeprom_set_data(uint16_t index, uint16_t *data, uint16_t length);
uint16_t *eeprom_get_data(void);

#endif
