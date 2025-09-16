#ifndef __BATTERY_H
#define __BATTERY_H

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>

#include "constants.h"
#include "delay.h"
#include "leds.h"
#include "sensors.h"
#include "setup.h"

#define BATTERY_VOLTAGE_LOW_PASS_FILTER_ALPHA 0.1

void set_battery_volt_div_factor(uint16_t version);
void update_battery_voltage(void);
float get_battery_voltage(void);
float get_battery_high_limit_voltage(void);
void show_battery_level(void);

#endif /* __BATTERY_H */
