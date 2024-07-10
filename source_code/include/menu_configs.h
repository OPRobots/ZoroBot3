#ifndef __MENU_CONFIGS_H
#define __MENU_CONFIGS_H

#include <buttons.h>
#include <calibrations.h>
#include <config.h>
#include <debug.h>
#include <leds.h>


bool check_menu_button(void);

bool in_debug_mode(void);
void reset_menu_mode(void);
void menu_reset(void);

#endif