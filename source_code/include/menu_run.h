#ifndef MENU_RUN_H
#define MENU_RUN_H

#include <buttons.h>
#include <delay.h>
#include <leds.h>

#define MODE_STRATEGY_HANDWALL 0
#define MODE_STRATEGY_FLOODFILL 1

bool menu_run_handler(void);
void menu_run_reset(void);

bool menu_run_can_start(void);
uint8_t menu_run_get_strategy(void);

#endif