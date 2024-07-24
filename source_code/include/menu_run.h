#ifndef MENU_RUN_H
#define MENU_RUN_H

#include <buttons.h>
#include <delay.h>
#include <leds.h>

#define MODE_STRATEGY_HANDWALL 0
#define MODE_STRATEGY_FLOODFILL 1

enum speed_strategy {
  SPEED_EXPLORE = 0,
  SPEED_NORMAL = 1,
  SPEED_FAST = 2,
  SPEED_DIAGONALS = 3,
};

enum maze_type {
  MAZE_HOME = 0,
  MAZE_COMPETITION = 1,
};

bool menu_run_handler(void);
void menu_run_reset(void);

bool menu_run_can_start(void);
enum speed_strategy menu_run_get_speed(void);
enum maze_type menu_run_get_maze_type(void);
uint8_t menu_run_get_strategy(void);

#endif