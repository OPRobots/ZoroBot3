#include <maze.h>

#define MAZE_ROWS_HOME 6
#define MAZE_COLUMNS_HOME 6

#define MAZE_ROWS_COMPETITION 16
#define MAZE_COLUMNS_COMPETITION 16

static struct cells_stack goals;

#ifdef MAZE_SIMULATOR
// Variables para simulador
static uint16_t sim_rows = 16;
static uint16_t sim_cols = 16;

void maze_simulator_set_size(uint16_t rows, uint16_t cols) {
  sim_rows = rows;
  sim_cols = cols;
}
#endif

static void add_goal(uint8_t x, uint8_t y) {
  goals.stack[goals.size++] = (x - 1) + (y - 1) * maze_get_columns();
}

uint16_t maze_get_rows() {
#ifdef MAZE_SIMULATOR
  return sim_rows;
#endif

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
#ifdef MAZE_SIMULATOR
  return sim_cols;
#endif

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

#ifdef MAZE_SIMULATOR
  // Para 16x16 el centro es (8,8), (8,9), (9,8), (9,9)
  if (sim_rows == 16 && sim_cols == 16) {
    add_goal(8, 8);
    add_goal(8, 9);
    add_goal(9, 8);
    add_goal(9, 9);
  } else {
    // Centro genérico
    add_goal(sim_cols / 2, sim_rows / 2);
    add_goal(sim_cols / 2 + 1, sim_rows / 2);
    add_goal(sim_cols / 2, sim_rows / 2 + 1);
    add_goal(sim_cols / 2 + 1, sim_rows / 2 + 1);
  }
  return &goals;
#endif

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