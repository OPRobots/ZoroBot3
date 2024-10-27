#include "eeprom.h"

static int16_t eeprom_data[DATA_LENGTH];
static uint32_t millis_save = 0;

void eeprom_save(void) {
  uint32_t addr = EEPROM_BASE_ADDRESS;
  millis_save = get_clock_ticks();
  while(get_clock_ticks() - millis_save < 200){
    warning_status_led(50);
  }
  set_status_led(true);
  flash_unlock();
  flash_erase_sector(EEPROM_SECTOR, FLASH_CR_PROGRAM_X16);
  for (uint16_t i = 0; i < DATA_LENGTH; i++) {
    flash_program_word(addr, eeprom_data[i]);
    addr += 4;
  }
  flash_lock();
  set_status_led(false);
}

void eeprom_load(void) {
  uint32_t addr = EEPROM_BASE_ADDRESS;
  for (uint16_t i = 0; i < DATA_LENGTH; i++) {
    eeprom_data[i] = MMIO32(addr);
    addr += 4;
  }

  // eeprom_restore();

  mpu_load_eeprom();
  sensors_load_eeprom();
  floodfill_load_maze();
  menu_run_load_values();
}

void eeprom_clear(void) {
  flash_unlock();
  flash_erase_sector(EEPROM_SECTOR, FLASH_CR_PROGRAM_X16);
  flash_lock();
}

void eeprom_backup(void){
  printf("int16_t eeprom_backup[DATA_LENGTH] = {");
  for (uint16_t i = 0; i < DATA_LENGTH; i++) {
    printf("%d,", eeprom_data[i]);
  }
  printf("};\n");
}

void eeprom_restore(void){
  int16_t eeprom_backup[DATA_LENGTH] = {1,56,0,0,2,17,520,85,325,45,15,28,4,20,20,6,9,21,17,21,5,2,11,12,4,6,11,10,11,24,16,2,11,10,11,13,7,26,9,2,25,19,25,21,17,19,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,};

  for (uint16_t i = 0; i < DATA_LENGTH; i++) {
    eeprom_data[i] = eeprom_backup[i];
  }  
}

void eeprom_set_data(uint16_t index, int16_t *data, uint16_t length) {
  for (uint16_t i = index; i < index + length; i++) {
    eeprom_data[i] = data[i - index];
  }
}

int16_t *eeprom_get_data(void){
  return eeprom_data;
}