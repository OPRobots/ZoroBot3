#ifndef __MENU_H
#define __MENU_H

#include <config.h>
#include <debug.h>
#include <calibrations.h>
#include <buttons.h>
#include <leds.h>

void check_menu_button(void);

bool in_debug_mode(void);
void reset_menu_mode(void);

#endif