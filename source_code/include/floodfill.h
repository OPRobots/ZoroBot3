#ifndef FLOODFILL_H
#define FLOODFILL_H

#include <config.h>
#include <constants.h>
#include <usart.h>
#include <move.h>
#include <maze.h>

#define VISITED_BIT 1
#define EAST_BIT 2
#define SOUTH_BIT 4
#define WEST_BIT 8
#define NORTH_BIT 16

#define MAX_TARGETS 10

enum compass_direction {
  EAST = 1,
  SOUTH = -MAZE_COLUMNS,
  WEST = -1,
  NORTH = MAZE_COLUMNS,
};

struct compass_direction_values{
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

struct cells_queue {
  uint8_t queue[MAZE_CELLS];
  uint8_t head;
  uint8_t tail;
};

struct cells_stack {
  uint8_t stack[MAX_TARGETS];
  uint8_t size;
};

void floodfill_set_time_limit(uint32_t ms);
void floodfill_maze_print(void);
void floodfill_load_maze(void);
void floodfill_start_explore(void);
void floodfill_start_run(void);
void floodfill_loop(void);

#endif