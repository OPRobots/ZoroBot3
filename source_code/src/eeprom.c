#include "eeprom.h"

static int16_t eeprom_data[DATA_TOTAL_WORDS];

static int16_t eeprom_compute_checksum(void) {
  int16_t sum = 0;
  for (uint16_t i = 0; i < DATA_LENGTH; i++) {
    sum += eeprom_data[i];
  }
  return -sum;
}

static bool eeprom_validate_checksum(void) {
  bool empty = true;
  int16_t sum = 0;
  for (uint16_t i = 0; i < DATA_TOTAL_WORDS; i++) {
    if (eeprom_data[i] != 0 && empty) {
      empty = false;
    }
    sum += eeprom_data[i];
  }
  return sum == 0 && !empty;
}

void eeprom_save(void) {
  uint32_t addr = EEPROM_BASE_ADDRESS;
  set_status_led(false);
  for (uint16_t i = 0; i < 2; i++) {
    toggle_status_led();
    delay_us(50000);
    toggle_status_led();
    delay_us(50000);
  }

  set_status_led(true);
  eeprom_data[DATA_CHECKSUM_INDEX] = eeprom_compute_checksum();
  flash_unlock();
  flash_erase_sector(EEPROM_SECTOR, FLASH_CR_PROGRAM_X16);
  for (uint16_t i = 0; i < DATA_TOTAL_WORDS; i++) {
    flash_program_word(addr, eeprom_data[i]);
    addr += 4;
  }
  flash_lock();
  set_status_led(false);
}

void eeprom_load(void) {
  uint32_t addr = EEPROM_BASE_ADDRESS;
  for (uint16_t i = 0; i < DATA_TOTAL_WORDS; i++) {
    eeprom_data[i] = MMIO32(addr);
    addr += 4;
  }

  if (!eeprom_validate_checksum()) {
    warning_eeprom();
  }

  uint16_t used_bytes = DATA_TOTAL_WORDS * 4;
  printf("EEPROM usage: %lu/%lu bytes (%.1f%%)\n",
         (unsigned long)used_bytes,
         (unsigned long)EEPROM_SECTOR_SIZE,
         (float)used_bytes * 100.0f / (float)EEPROM_SECTOR_SIZE);

  lsm6dsr_load_eeprom();
  sensors_load_eeprom();
  floodfill_load_maze();
  menu_run_load_values();
  rc5_load_eeprom();
}

void eeprom_clear(void) {
  flash_unlock();
  flash_erase_sector(EEPROM_SECTOR, FLASH_CR_PROGRAM_X16);
  flash_lock();
}

void eeprom_backup(void) {
  printf("int16_t eeprom_backup[DATA_TOTAL_WORDS] = {");
  for (uint16_t i = 0; i < DATA_TOTAL_WORDS; i++) {
    printf("%d,", eeprom_data[i]);
  }
  printf("};\n");
}

// clang-format off
void eeprom_restore(void){
  int16_t eeprom_backup[DATA_LENGTH] = {1,56,0,0,2,17,520,85,325,45,15,28,4,20,20,6,9,21,17,21,5,2,11,12,4,6,11,10,11,24,16,2,11,10,11,13,7,26,9,2,25,19,25,21,17,19,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,};

  for (uint16_t i = 0; i < DATA_LENGTH; i++) {
    eeprom_data[i] = eeprom_backup[i];
  }
}
// clang-format on

void eeprom_set_data(uint16_t index, int16_t *data, uint16_t length) {
  for (uint16_t i = index; i < index + length; i++) {
    eeprom_data[i] = data[i - index];
  }
}

int16_t *eeprom_get_data(void) {
  return eeprom_data;
}