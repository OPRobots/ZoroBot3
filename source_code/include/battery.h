#ifndef __BATTERY_H
#define __BATTERY_H

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>

#include "delay.h"
#include "leds.h"
#include "setup.h"
#include "constants.h"

#define BATTERY_VOLTAGE_LOW_PASS_FILTER_ALPHA 0.1

void update_battery_voltage(void);
float get_battery_voltage(void);
void show_battery_level(void);

#endif /* __BATTERY_H */
