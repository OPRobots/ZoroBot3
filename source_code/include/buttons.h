#ifndef __BUTTONS_H
#define __BUTTONS_H

#include "config.h"
#include "control.h"
#include "delay.h"
#include <libopencm3/stm32/gpio.h>
#include <math.h>
#include <stdio.h>

#define MENU_VOLTAGE_LOW_PASS_FILTER_ALPHA 0.1

void check_buttons(void);
bool get_start_btn(void);
bool get_menu_up_btn(void);
bool get_menu_down_btn(void);
bool get_menu_mode_btn(void);

#endif