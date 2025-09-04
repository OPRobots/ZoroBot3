#include <maze.h>

#define MAZE_ROWS_HOME 6
#define MAZE_COLUMNS_HOME 6

#define MAZE_ROWS_COMPETITION 16
#define MAZE_COLUMNS_COMPETITION 16

static struct cells_stack goals;

static void add_goal(uint8_t x, uint8_t y) {
  goals.stack[goals.size++] = (x - 1) + (y - 1) * maze_get_columns();
}

uint16_t maze_get_rows() {
#ifdef MMSIM_ENABLED
  return API_mazeHeight();
#endif

  switch (menu_run_get_maze_type()) {
    case MAZE_HOME:
      return MAZE_ROWS_HOME;
      break;
    case MAZE_COMPETITION:
      return MAZE_ROWS_COMPETITION;
      break;
  }
  return 0;
}

uint16_t maze_get_columns() {
#ifdef MMSIM_ENABLED
  return API_mazeWidth();
#endif

  switch (menu_run_get_maze_type()) {
    case MAZE_HOME:
      return MAZE_COLUMNS_HOME;
      break;
    case MAZE_COMPETITION:
      return MAZE_COLUMNS_COMPETITION;
      break;
  }
  return 0;
}

uint16_t maze_get_cells() {
  return maze_get_rows() * maze_get_columns();
}

struct cells_stack *maze_get_goals(void) {
  goals.size = 0;

#ifdef MMSIM_ENABLED
  add_goal(8, 8);
  add_goal(8, 9);
  add_goal(9, 8);
  add_goal(9, 9);
  return &goals;
#endif

  switch (menu_run_get_maze_type()) {
    case MAZE_HOME:
      add_goal(6, 6);
      add_goal(6, 5);
      add_goal(5, 6);
      add_goal(5, 5);
      break;
    case MAZE_COMPETITION:
      add_goal(8, 8);
      add_goal(8, 9);
      add_goal(9, 8);
      add_goal(9, 9);
      break;
  }

  return &goals;
}