#ifndef __EEPROM_H
#define __EEPROM_H

#include <libopencm3/stm32/flash.h>

#include <delay.h>
#include <leds.h>
#include <sensors.h>
#include <floodfill.h>
#include <menu_run.h>

#define EEPROM_SECTOR 11
#define EEPROM_BASE_ADDRESS (0x080E0000)

#define DATA_LENGTH (2 + NUM_SENSORES + 4 + MAZE_CELLS + MENU_RUN_NUM_MODES)

#define DATA_INDEX_GYRO_Z 0
#define DATA_INDEX_SENSORS_OFFSETS (DATA_INDEX_GYRO_Z + 2)
#define DATA_INDEX_FRONT_SENSORS_CALIBRATION (DATA_INDEX_SENSORS_OFFSETS + NUM_SENSORES)
#define DATA_INDEX_MAZE (DATA_INDEX_FRONT_SENSORS_CALIBRATION + 4)
#define DATA_INDEX_MENU_RUN (DATA_INDEX_MAZE + MAZE_CELLS)

void eeprom_save(void);
void eeprom_load(void);
void eeprom_clear(void);
void eeprom_set_data(uint16_t index, uint16_t *data, uint16_t length);
uint16_t *eeprom_get_data(void);

#endif
