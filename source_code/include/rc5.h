#ifndef RC5_H
#define RC5_H

#include <libopencm3/stm32/gpio.h>

// #include <control.h>
#include <buttons.h>
#include <delay.h>
#include <menu_run.h>
#include <stdint.h>
#include <usart.h>
// #include <eeprom.h>

enum RC5_TRIGGER {
  RC5_TRIGGER_FALLING,
  RC5_TRIGGER_RISING,
};

#define RC5_DATA_LENGTH 5

void rc5_load_eeprom(void);
void rc5_register(enum RC5_TRIGGER trigger);

#endif