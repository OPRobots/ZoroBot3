#ifndef MENU_RUN_H
#define MENU_RUN_H

#ifndef MMSIM_ENABLED
#include <buttons.h>
#include <delay.h>
#include <leds.h>
#else
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#endif

#define MENU_RUN_NUM_MODES 5

enum speed_strategy {
  SPEED_EXPLORE = 0,
  SPEED_NORMAL = 1,
  SPEED_MEDIUM = 2,
  SPEED_FAST = 3,
  SPEED_SUPER = 4,
  SPEED_HAKI = 5,
};

enum maze_type {
  MAZE_HOME = 0,
  MAZE_COMPETITION = 1,
};

enum solve_strategy {
  SOLVE_STANDARD = 0,
  SOLVE_DIAGONALS = 1,
};

enum explore_algorithm {
  EXPLORE_HANDWALL = 0,
  EXPLORE_FLOODFILL = 1,
  EXPLORE_TIME_TRIAL = 2,
};

bool menu_run_handler(void);
void menu_run_reset(void);

void menu_run_load_values(void);

bool menu_run_can_start(void);

void menu_run_mode_change(void);
void menu_run_up(void);
void menu_run_down(void);

int16_t *get_menu_run_values(void);
enum speed_strategy menu_run_get_speed(void);
enum maze_type menu_run_get_maze_type(void);
enum solve_strategy menu_run_get_solve_strategy(void);
enum explore_algorithm menu_run_get_explore_algorithm(void);

#endif