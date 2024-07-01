#ifndef FLOODFILL_H
#define FLOODFILL_H

#include <config.h>
#include <constants.h>
#include <usart.h>


#define VISITED_BIT 1
#define EAST_BIT 2
#define SOUTH_BIT 4
#define WEST_BIT 8
#define NORTH_BIT 16

enum compass_direction {
  EAST = 1,
  SOUTH = -MAZE_COLUMNS,
  WEST = -1,
  NORTH = MAZE_COLUMNS,
};

enum step_direction {
  NONE = -1,
  FRONT = 0,
  LEFT = 1,
  RIGHT = 2,
  BACK = 3,
};


void floodfill_use_left_hand(void);
void floodfill_use_right_hand(void);
void floodfill_set_time_limit(uint32_t ms);
void floodfill_maze_print(void);
void floodfill_save_maze(void);
void floodfill_load_maze(void);
void floodfill_start(void);
void floodfill_loop(void);


void floodfill_debug_update_walls(uint8_t position, bool east, bool south, bool west, bool north);

#endif