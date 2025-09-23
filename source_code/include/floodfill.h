#ifndef FLOODFILL_H
#define FLOODFILL_H

#include "config.h"
#include "constants.h"
#include "floodfill_weigths.h"
#include "maze.h"
#include "move.h"
#include "sensors.h"

#ifndef MMSIM_ENABLED
#include <usart.h>
#else
#include "mmsim_api.h"
#include "menu_run.h"
#endif

#define VISITED_BIT 1
#define EAST_BIT 2
#define SOUTH_BIT 4
#define WEST_BIT 8
#define NORTH_BIT 16

#define MAX_TARGETS 10

enum compass_direction {
  TARGET = 0,
  EAST = 1,
  SOUTH_EAST = 1 - MAZE_COLUMNS,
  SOUTH = -MAZE_COLUMNS,
  SOUTH_WEST = -1 - MAZE_COLUMNS,
  WEST = -1,
  NORTH_WEST = -1 + MAZE_COLUMNS,
  NORTH = MAZE_COLUMNS,
  NORTH_EAST = 1 + MAZE_COLUMNS,
};

struct compass_direction_values {
  int8_t EAST;
  int8_t SOUTH;
  int8_t WEST;
  int8_t NORTH;
};

enum step_direction {
  NONE = -1,
  FRONT = 0,
  LEFT = 1,
  RIGHT = 2,
  BACK = 3,
};

struct queue_cell {
  uint8_t cell;
  enum compass_direction direction;
  enum compass_direction last_step;
  uint8_t count;
};

struct cells_queue {
  struct queue_cell queue[MAZE_CELLS];
  uint8_t head;
  uint8_t tail;
};

struct cells_stack {
  uint8_t stack[MAX_TARGETS];
  uint8_t size;
};

void floodfill_set_time_limit(uint32_t ms);
void floodfill_set_reset_maze_on_start_explore(bool reset);
bool floodfill_is_reset_maze_on_start_explore(void);
void floodfill_maze_print(void);
void floodfill_load_maze(void);
void floodfill_start_explore(void);
void floodfill_start_run(void);
void floodfill_loop(void);

#endif