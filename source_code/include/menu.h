#ifndef MENU_H
#define MENU_H

#include <buttons.h>
#include <delay.h>
#include <leds.h>
#include <menu_configs.h>
#include <menu_run.h>
#include <usart.h>

void menu_handler(void);
void menu_reset(void);

void menu_rc5_mode_change(void);
void menu_rc5_up(void);
void menu_rc5_down(void);

#endif