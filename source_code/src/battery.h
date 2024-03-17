#ifndef __BATTERY_H
#define __BATTERY_H

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>

#include "setup.h"
#include "delay.h"
#include "leds.h"

float get_battery_voltage(void);
void show_battery_level(void);

#endif /* __BATTERY_H */
