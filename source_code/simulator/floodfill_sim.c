// floodfill_sim.c
// Implementación de floodfill para el simulador standalone
// Este archivo habilita MAZE_SIMULATOR para incluir todo el código de debug y simulación

#define MAZE_SIMULATOR 1
#include "floodfill_sim.h"
#include <stdlib.h>  // Para exit()

static uint32_t start_ms = 0;
static uint32_t time_limit = 0;

static float floodfill[MAZE_CELLS];
static int16_t maze[MAZE_CELLS];
static bool reset_maze_on_start_explore = true;

static enum compass_direction initial_direction = NORTH;

static uint8_t current_position = 0;
static enum compass_direction current_direction = NORTH;

static uint8_t maze_goal_position = 0;
static enum compass_direction maze_goal_direction = NORTH;

static struct cell_weigth straight_weights[15];
static uint16_t straight_weights_count = 0;
static struct cell_weigth diagonal_weights[15];
static uint16_t diagonal_weights_count = 0;

static struct cells_queue cells_queue;

static struct cells_stack target_cells;
static struct cells_stack goal_cells;

static bool race_mode = false;
static char run_sequence[MAZE_CELLS + 3];
static enum movement run_sequence_movements[MAZE_CELLS + 3];
static struct compass_direction_values directions_values = {
    .EAST = 1,
    .SOUTH = -MAZE_COLUMNS,
    .WEST = -1,
    .NORTH = MAZE_COLUMNS,
};

// ============== SIMULATION MODE ==============
// SIM_MODE activo para el simulador
#define SIM_MODE 1

// Variables del simulador
static int16_t true_maze[MAZE_CELLS];  // El laberinto "real" (input del usuario)
static uint8_t sim_maze_rows = 16;
static uint8_t sim_maze_cols = 16;
static uint8_t cell_colors[MAZE_CELLS];  // 0=normal, 1=visitado, 2=ruta final (azul)

static void initialize_directions_values(void) {
  directions_values.EAST = 1;
  directions_values.SOUTH = -maze_get_columns();
  directions_values.WEST = -1;
  directions_values.NORTH = maze_get_columns();
}

static void initialize_maze(void) {
  for (uint16_t i = 0; i < MAZE_CELLS; i++) {
    maze[i] = 0;
    floodfill[i] = MAZE_MAX_DISTANCE;
  }

  for (uint16_t i = 0; i < maze_get_rows(); i++) {
    maze[(maze_get_columns() - 1) + i * (maze_get_columns())] |= EAST_BIT;
    maze[i * maze_get_columns()] |= WEST_BIT;

#ifdef MMSIM_ENABLED
    API_setWall(maze_get_columns() - 1, i, 'e');
    API_setWall(0, i, 'w');
#endif
  }

  for (uint16_t i = 0; i < maze_get_columns(); i++) {
    maze[i] |= SOUTH_BIT;
    maze[(maze_get_rows() - 1) * maze_get_columns() + i] |= NORTH_BIT;
#ifdef MMSIM_ENABLED
    API_setWall(i, maze_get_rows() - 1, 'n');
    API_setWall(i, 0, 's');
#endif
  }
}

static void set_initial_state(void) {
  current_position = 0;
  current_direction = initial_direction;
}

static void set_goals_from_maze(void) {
  goal_cells.size = 0;
  struct cells_stack *goals = maze_get_goals();
  for (uint8_t i = 0; i < goals->size; i++) {
    goal_cells.stack[goal_cells.size++] = goals->stack[i];
  }
}

static void add_target(uint8_t cell) {
  target_cells.stack[target_cells.size++] = cell;
}

static void set_target(uint8_t cell) {
  target_cells.size = 0;
  add_target(cell);
}

static void set_goal_as_target(void) {
  target_cells.size = 0;
  for (uint8_t i = 0; i < goal_cells.size; i++) {
    add_target(goal_cells.stack[i]);
  }
}

static enum compass_direction get_next_direction(enum step_direction step) {
  if (step == LEFT) {
    if (current_direction == EAST) {
      return NORTH;
    }
    if (current_direction == SOUTH) {
      return EAST;
    }
    if (current_direction == WEST) {
      return SOUTH;
    }
    if (current_direction == NORTH) {
      return WEST;
    }
  }
  if (step == RIGHT) {
    if (current_direction == EAST) {
      return SOUTH;
    }
    if (current_direction == SOUTH) {
      return WEST;
    }
    if (current_direction == WEST) {
      return NORTH;
    }
    if (current_direction == NORTH) {
      return EAST;
    }
  }
  if (step == FRONT) {
    return current_direction;
  }
  return -current_direction;
}

static int8_t get_direction_value(enum compass_direction direction) {
  switch (direction) {
    case EAST:
      return directions_values.EAST;
    case SOUTH:
      return directions_values.SOUTH;
    case WEST:
      return directions_values.WEST;
    case NORTH:
      return directions_values.NORTH;
    default:
      return 0;
  }
}

static uint8_t get_next_position(enum step_direction step) {
  return current_position + get_direction_value(get_next_direction(step));
}

static void update_position(enum step_direction step) {
  current_position = get_next_position(step);
  current_direction = get_next_direction(step);
}

static bool wall_exists(uint8_t position, uint8_t wall_bit) {
  return maze[position] & wall_bit;
}

static void set_wall(uint8_t wall_bit) {
  if (!wall_exists(current_position, wall_bit)) {
    maze[current_position] |= wall_bit;
    switch (wall_bit) {
      case EAST_BIT:
        if (current_position % maze_get_columns() == maze_get_columns() - 1) {
          break;
        }
        maze[current_position + get_direction_value(EAST)] |= WEST_BIT;
        break;
      case SOUTH_BIT:
        if (current_position / maze_get_columns() == 0) {
          break;
        }
        maze[current_position + get_direction_value(SOUTH)] |= NORTH_BIT;
        break;
      case WEST_BIT:
        if (current_position % maze_get_columns() == 0) {
          break;
        }
        maze[current_position + get_direction_value(WEST)] |= EAST_BIT;
        break;
      case NORTH_BIT:
        if (current_position / maze_get_columns() == maze_get_rows() - 1) {
          break;
        }
        maze[current_position + get_direction_value(NORTH)] |= SOUTH_BIT;
        break;
    }
  }
}

static struct walls get_current_stored_walls(void) {
  struct walls walls;
  uint8_t cell = maze[current_position];
  switch (current_direction) {
    case EAST:
      walls.left = (bool)(cell & NORTH_BIT);
      walls.front = (bool)(cell & EAST_BIT);
      walls.right = (bool)(cell & SOUTH_BIT);
      break;
    case SOUTH:
      walls.left = (bool)(cell & EAST_BIT);
      walls.front = (bool)(cell & SOUTH_BIT);
      walls.right = (bool)(cell & WEST_BIT);
      break;
    case WEST:
      walls.left = (bool)(cell & SOUTH_BIT);
      walls.front = (bool)(cell & WEST_BIT);
      walls.right = (bool)(cell & NORTH_BIT);
      break;
    case NORTH:
      walls.left = (bool)(cell & WEST_BIT);
      walls.front = (bool)(cell & NORTH_BIT);
      walls.right = (bool)(cell & EAST_BIT);
      break;
    default:
      break;
  }

  return walls;
}

static struct virtual_walls get_current_stored_virtual_walls(void) {
  struct virtual_walls walls;
  uint8_t cell = maze[current_position];
  switch (current_direction) {
    case EAST:
      walls.left = (bool)(cell & NORTH_BIT);
      walls.front = (bool)(cell & EAST_BIT);
      walls.right = (bool)(cell & SOUTH_BIT);
      walls.back = (bool)(cell & WEST_BIT);
      break;
    case SOUTH:
      walls.left = (bool)(cell & EAST_BIT);
      walls.front = (bool)(cell & SOUTH_BIT);
      walls.right = (bool)(cell & WEST_BIT);
      walls.back = (bool)(cell & NORTH_BIT);
      break;
    case WEST:
      walls.left = (bool)(cell & SOUTH_BIT);
      walls.front = (bool)(cell & WEST_BIT);
      walls.right = (bool)(cell & NORTH_BIT);
      walls.back = (bool)(cell & EAST_BIT);
      break;
    case NORTH:
      walls.left = (bool)(cell & WEST_BIT);
      walls.front = (bool)(cell & NORTH_BIT);
      walls.right = (bool)(cell & EAST_BIT);
      walls.back = (bool)(cell & SOUTH_BIT);
      break;
    default:
      break;
  }

  return walls;
}

static void set_visited(void) {
  maze[current_position] |= VISITED_BIT;
}

static bool is_visited(uint8_t position) {
  return maze[position] & VISITED_BIT;
}

static bool current_cell_is_visited(void) {
  return maze[current_position] & VISITED_BIT;
}

static bool current_cell_is_goal(void) {
  for (uint8_t i = 0; i < goal_cells.size; i++) {
    if (current_position == goal_cells.stack[i]) {
      return true;
    }
  }
  return false;
}

static void update_walls(struct walls walls) {
  bool cell_walls[4] = {false, false, false, false};
  switch (current_direction) {
    case EAST:
      cell_walls[0] = walls.front;
      cell_walls[1] = walls.right;
      cell_walls[2] = false;
      cell_walls[3] = walls.left;
      break;
    case SOUTH:
      cell_walls[0] = walls.left;
      cell_walls[1] = walls.front;
      cell_walls[2] = walls.right;
      cell_walls[3] = false;
      break;
    case WEST:
      cell_walls[0] = false;
      cell_walls[1] = walls.left;
      cell_walls[2] = walls.front;
      cell_walls[3] = walls.right;
      break;
    case NORTH:
      cell_walls[0] = walls.right;
      cell_walls[1] = false;
      cell_walls[2] = walls.left;
      cell_walls[3] = walls.front;
      break;
    default:
      break;
  }

  if (cell_walls[0]) {
    set_wall(EAST_BIT);
#ifdef MMSIM_ENABLED
    API_setWall(current_position % maze_get_columns(), current_position / maze_get_columns(), 'e');
#endif
  }
  if (cell_walls[1]) {
    set_wall(SOUTH_BIT);
#ifdef MMSIM_ENABLED
    API_setWall(current_position % maze_get_columns(), current_position / maze_get_columns(), 's');
#endif
  }
  if (cell_walls[2]) {
    set_wall(WEST_BIT);
#ifdef MMSIM_ENABLED
    API_setWall(current_position % maze_get_columns(), current_position / maze_get_columns(), 'w');
#endif
  }
  if (cell_walls[3]) {
    set_wall(NORTH_BIT);
#ifdef MMSIM_ENABLED
    API_setWall(current_position % maze_get_columns(), current_position / maze_get_columns(), 'n');
#endif
  }

  set_visited();
}

static void reset_floodfill_and_queue(void) {
  for (uint16_t i = 0; i < MAZE_CELLS; i++) {
    floodfill[i] = MAZE_MAX_DISTANCE;
  }
  cells_queue.head = 0;
  cells_queue.tail = 0;
}

static void queue_push(uint8_t position, enum compass_direction direction, enum compass_direction step, uint8_t count) {
  struct queue_cell queue = {
      .cell = position,
      .direction = direction,
      .last_step = step,
      .count = count,
  };

  float value = floodfill[position];
  int16_t i = cells_queue.head;

  while (i > 0 && floodfill[cells_queue.queue[i - 1].cell] > value) {
    cells_queue.queue[i] = cells_queue.queue[i - 1];
    i--;
  }

  cells_queue.queue[i] = queue;
  cells_queue.head++;
}

static struct queue_cell queue_pop(void) {
  return cells_queue.queue[cells_queue.tail++];
}

static enum step_direction get_next_floodfill_step(struct walls walls) {
  float floodfill_value = floodfill[current_position];
  enum step_direction next_step = BACK;
  if (menu_run_get_floodfill_type() == FLOODFILL_TYPE_BASIC) {
    if (!walls.front && floodfill[get_next_position(FRONT)] < floodfill_value) {
      floodfill_value = floodfill[get_next_position(FRONT)];
      next_step = FRONT;
    }
  }
  if (!walls.right && floodfill[get_next_position(RIGHT)] < floodfill_value) {
    floodfill_value = floodfill[get_next_position(RIGHT)];
    next_step = RIGHT;
  }
  if (!walls.left && floodfill[get_next_position(LEFT)] < floodfill_value) {
    floodfill_value = floodfill[get_next_position(LEFT)];
    next_step = LEFT;
  }
  if (menu_run_get_floodfill_type() != FLOODFILL_TYPE_BASIC) {
    if (!walls.front && floodfill[get_next_position(FRONT)] < floodfill_value) {
      floodfill_value = floodfill[get_next_position(FRONT)];
      next_step = FRONT;
    }
  }
  return next_step;
}

static enum step_direction get_next_floodfill_virtual_step(struct virtual_walls walls) {
  float floodfill_value = floodfill[current_position];
  enum step_direction next_step = NONE;

  if (menu_run_get_floodfill_type() == FLOODFILL_TYPE_BASIC) {
    if (!walls.front && floodfill[get_next_position(FRONT)] < floodfill_value) {
      floodfill_value = floodfill[get_next_position(FRONT)];
      next_step = FRONT;
    }
  }
  if (!walls.right && floodfill[get_next_position(RIGHT)] < floodfill_value) {
    floodfill_value = floodfill[get_next_position(RIGHT)];
    next_step = RIGHT;
  }
  if (!walls.left && floodfill[get_next_position(LEFT)] < floodfill_value) {
    floodfill_value = floodfill[get_next_position(LEFT)];
    next_step = LEFT;
  }
  if (menu_run_get_floodfill_type() != FLOODFILL_TYPE_BASIC) {
    if (!walls.front && floodfill[get_next_position(FRONT)] < floodfill_value) {
      floodfill_value = floodfill[get_next_position(FRONT)];
      next_step = FRONT;
    }
  }
  if (!walls.back && floodfill[get_next_position(BACK)] < floodfill_value && next_step == NONE) {
    floodfill_value = floodfill[get_next_position(BACK)];
    next_step = BACK;
  }
  return next_step;
}

static float get_next_floodfill_distance(float distance, enum compass_direction from_direction, enum compass_direction to_direction, uint8_t count, uint8_t last_count) {
  switch (menu_run_get_floodfill_type()) {
    case FLOODFILL_TYPE_BASIC:
      return distance + 1.0f;
    case FLOODFILL_TYPE_DIAGONAL:
      switch (to_direction) {
        case EAST:
        case SOUTH:
        case WEST:
        case NORTH:
          return distance + 1.0f;
        default:
          return distance + 0.7f;
      }
    case FLOODFILL_TYPE_TIME:
    case FLOODFILL_TYPE_TIMEv2: {
      bool from_orthogonal = false;
      bool from_diagonal = false;
      bool to_orthogonal = false;
      bool to_diagonal = false;
      float next_distance = 0.0f;
      switch (from_direction) {
        case TARGET:
        case EAST:
        case SOUTH:
        case WEST:
        case NORTH:
          from_orthogonal = true;
          break;
        case SOUTH_EAST:
        case SOUTH_WEST:
        case NORTH_WEST:
        case NORTH_EAST:
          from_diagonal = true;
          break;
      }
      switch (to_direction) {
        case TARGET:
        case EAST:
        case SOUTH:
        case WEST:
        case NORTH:
          to_orthogonal = true;
          break;
        case SOUTH_EAST:
        case SOUTH_WEST:
        case NORTH_WEST:
        case NORTH_EAST:
          to_diagonal = true;
          break;
      }
      if (from_orthogonal && to_orthogonal) {
        if (count < straight_weights_count) {
          next_distance += (float)(straight_weights[count].time);
        } else {
          next_distance += (float)straight_weights[straight_weights_count - 1].time;
        }
      } else if (from_diagonal && to_diagonal) {
        if (from_direction == to_direction) {
          if (count < diagonal_weights_count) {
            next_distance += (float)diagonal_weights[count].time;
          } else {
            next_distance += (float)diagonal_weights[diagonal_weights_count - 1].time;
          }
        } else {
          next_distance += (float)diagonal_weights[0].time;
          if (last_count < diagonal_weights_count) {
            next_distance += (float)diagonal_weights[last_count].penalty;
          } else {
            next_distance += (float)diagonal_weights[diagonal_weights_count - 1].penalty;
          }
        }
      } else if (from_orthogonal && to_diagonal) {
        next_distance += (float)diagonal_weights[0].time;
        if (last_count < straight_weights_count) {
          next_distance += (float)straight_weights[last_count].penalty;
        } else {
          next_distance += (float)straight_weights[straight_weights_count - 1].penalty;
        }
      } else if (from_diagonal && to_orthogonal) {
        next_distance += (float)straight_weights[0].time;
        if (last_count < diagonal_weights_count) {
          next_distance += (float)diagonal_weights[last_count].penalty;
        } else {
          next_distance += (float)diagonal_weights[diagonal_weights_count - 1].penalty;
        }
      }
      return distance + next_distance;
    }
    default:
      return distance + 1.0f;
  }
}

static uint8_t get_next_floodfill_count(enum compass_direction from_direction, enum compass_direction to_direction, uint8_t count) {
  if (from_direction == to_direction /* || from_direction == TARGET */) {
    return count + 1;
  } else {
    return 0;
  }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
static enum compass_direction get_next_floodfill_direction(enum compass_direction from_direction, enum compass_direction from_step, enum compass_direction to_step) {
  switch (from_direction) {
    case TARGET:
      return to_step;
    case EAST:
      switch (to_step) {
        case NORTH:
          return NORTH_EAST;
        case SOUTH:
          return SOUTH_EAST;
        default:
          return to_step;
      }
      break;
    case SOUTH_EAST:
      switch (to_step) {
        case NORTH:
          return NORTH_EAST;
        case SOUTH:
          switch (from_step) {
            case SOUTH:
              return SOUTH;
            case EAST:
              return SOUTH_EAST;
          }
          break;
        case EAST:
          switch (from_step) {
            case SOUTH:
              return SOUTH_EAST;
            case EAST:
              return EAST;
          }
          break;
        case WEST:
          return SOUTH_WEST;
        default:
          return to_step;
      }
      break;
    case SOUTH:
      switch (to_step) {
        case EAST:
          return SOUTH_EAST;
        case WEST:
          return SOUTH_WEST;
        default:
          return to_step;
      }
      break;
    case SOUTH_WEST:
      switch (to_step) {
        case NORTH:
          return NORTH_WEST;
        case SOUTH:
          switch (from_step) {
            case SOUTH:
              return SOUTH;
            case WEST:
              return SOUTH_WEST;
          }
          break;
        case EAST:
          return SOUTH_EAST;
        case WEST:
          switch (from_step) {
            case SOUTH:
              return SOUTH_WEST;
            case WEST:
              return WEST;
          }
          break;
        default:
          return to_step;
      }
      break;
    case WEST:
      switch (to_step) {
        case NORTH:
          return NORTH_WEST;
        case SOUTH:
          return SOUTH_WEST;
        default:
          return to_step;
      }
      break;
    case NORTH_WEST:
      switch (to_step) {
        case NORTH:
          switch (from_step) {
            case NORTH:
              return NORTH;
            case WEST:
              return NORTH_WEST;
          }
          break;
        case EAST:
          return NORTH_EAST;
        case SOUTH:
          return SOUTH_WEST;
        case WEST:
          switch (from_step) {
            case NORTH:
              return NORTH_WEST;
            case WEST:
              return WEST;
          }
          break;
        default:
          return to_step;
      }
      break;
    case NORTH:
      switch (to_step) {
        case EAST:
          return NORTH_EAST;
        case WEST:
          return NORTH_WEST;
        default:
          return to_step;
      }
      break;
    case NORTH_EAST:
      switch (to_step) {
        case NORTH:
          switch (from_step) {
            case NORTH:
              return NORTH;
            case EAST:
              return NORTH_EAST;
          }
          break;
        case SOUTH:
          return SOUTH_EAST;
        case WEST:
          return NORTH_WEST;
          break;
        case EAST:
          switch (from_step) {
            case NORTH:
              return NORTH_EAST;
            case EAST:
              return EAST;
          }
          break;
        default:
          return to_step;
      }
      break;
  }
  return to_step;
}
#pragma GCC diagnostic pop

static void update_floodfill(void) {
#ifdef MAZE_SIMULATOR
  static int update_call_count = 0;
  update_call_count++;
  if (update_call_count <= 3) {
    fprintf(stderr, "[update_floodfill #%d] targets=%d, floodfill_type=%d\n", 
            update_call_count, target_cells.size, menu_run_get_floodfill_type());
    for (uint8_t i = 0; i < target_cells.size && i < 8; i++) {
      fprintf(stderr, "  target[%d]=%d (x=%d,y=%d)\n", i, target_cells.stack[i], 
              target_cells.stack[i] % maze_get_columns(), 
              target_cells.stack[i] / maze_get_columns());
    }
    fflush(stderr);
  }
#endif
  if (straight_weights_count == 0 || diagonal_weights_count == 0) {
    uint16_t linear_speed = get_floodfill_linear_speed();
    uint16_t max_linear_speed = get_floodfill_max_linear_speed();
    uint16_t accel = get_floodfill_accel();
    straight_weights_count = floodfill_weights_cells_to_max_speed(180.0f, linear_speed, max_linear_speed, accel);
    floodfill_weights_table(180.0f, linear_speed, max_linear_speed, accel, straight_weights_count, straight_weights);
    diagonal_weights_count = floodfill_weights_cells_to_max_speed(127.3f, linear_speed, max_linear_speed, accel);
    floodfill_weights_table(127.3f, linear_speed, max_linear_speed, accel, diagonal_weights_count, diagonal_weights);
  }

  reset_floodfill_and_queue();

  for (uint8_t i = 0; i < target_cells.size; i++) {
    floodfill[target_cells.stack[i]] = 0;
    queue_push(target_cells.stack[i], TARGET, TARGET, 0);

#ifdef MMSIM_ENABLED
    API_setFloodFill(
        target_cells.stack[i] % maze_get_columns(),
        target_cells.stack[i] / maze_get_columns(),
        0);
    API_clearColor(
        target_cells.stack[i] % maze_get_columns(),
        target_cells.stack[i] / maze_get_columns());
#endif
  }

#ifdef MAZE_SIMULATOR
  uint32_t update_flood_iters = 0;
#endif
  while (cells_queue.head != cells_queue.tail) {
#ifdef MAZE_SIMULATOR
    update_flood_iters++;
    if (update_flood_iters > 100000) {
      fprintf(stderr, "update_floodfill: bucle infinito! head=%d tail=%d\n", cells_queue.head, cells_queue.tail);
      fflush(stderr);
      exit(1);
    }
#endif
    struct queue_cell queue_cell = queue_pop();
    uint8_t current_cell = queue_cell.cell;
    enum compass_direction direction = queue_cell.direction;
    enum compass_direction last_step = queue_cell.last_step;
    uint8_t count = queue_cell.count;

    float next_distance = 0;
    uint8_t next_cell = 0;
    uint8_t next_count = 0;
    enum compass_direction next_direction = 0;
    if (!wall_exists(current_cell, EAST_BIT)) {
      next_cell = current_cell + get_direction_value(EAST);
      next_direction = get_next_floodfill_direction(direction, last_step, EAST);
      next_count = get_next_floodfill_count(direction, next_direction, count);
      next_distance = get_next_floodfill_distance(floodfill[current_cell], direction, next_direction, next_count, count);
      if ((menu_run_get_floodfill_type() == FLOODFILL_TYPE_BASIC && floodfill[next_cell] > next_distance) ||
          (menu_run_get_floodfill_type() != FLOODFILL_TYPE_BASIC && floodfill[next_cell] >= next_distance)) {
        floodfill[next_cell] = next_distance;
        queue_push(next_cell, next_direction, EAST, next_count);

#ifdef MMSIM_ENABLED
        API_setFloodFill(
            next_cell % maze_get_columns(),
            next_cell / maze_get_columns(),
            next_distance);
        if (maze[next_cell] & VISITED_BIT) {
          API_setColor(
              next_cell % maze_get_columns(),
              next_cell / maze_get_columns(),
              'G');
        } else {
          API_clearColor(
              next_cell % maze_get_columns(),
              next_cell / maze_get_columns());
        }
#endif
      }
    }

    if (!wall_exists(current_cell, SOUTH_BIT)) {
      next_cell = current_cell + get_direction_value(SOUTH);
      next_direction = get_next_floodfill_direction(direction, last_step, SOUTH);
      next_count = get_next_floodfill_count(direction, next_direction, count);
      next_distance = get_next_floodfill_distance(floodfill[current_cell], direction, next_direction, next_count, count);
      if ((menu_run_get_floodfill_type() == FLOODFILL_TYPE_BASIC && floodfill[next_cell] > next_distance) ||
          (menu_run_get_floodfill_type() != FLOODFILL_TYPE_BASIC && floodfill[next_cell] >= next_distance)) {
        floodfill[next_cell] = next_distance;
        queue_push(next_cell, next_direction, SOUTH, next_count);

#ifdef MMSIM_ENABLED
        API_setFloodFill(
            next_cell % maze_get_columns(),
            next_cell / maze_get_columns(),
            next_distance);
        if (maze[next_cell] & VISITED_BIT) {
          API_setColor(
              next_cell % maze_get_columns(),
              next_cell / maze_get_columns(),
              'G');
        } else {
          API_clearColor(
              next_cell % maze_get_columns(),
              next_cell / maze_get_columns());
        }
#endif
      }
    }

    if (!wall_exists(current_cell, WEST_BIT)) {
      next_cell = current_cell + get_direction_value(WEST);
      next_direction = get_next_floodfill_direction(direction, last_step, WEST);
      next_count = get_next_floodfill_count(direction, next_direction, count);
      next_distance = get_next_floodfill_distance(floodfill[current_cell], direction, next_direction, next_count, count);
      if ((menu_run_get_floodfill_type() == FLOODFILL_TYPE_BASIC && floodfill[next_cell] > next_distance) ||
          (menu_run_get_floodfill_type() != FLOODFILL_TYPE_BASIC && floodfill[next_cell] >= next_distance)) {
        floodfill[next_cell] = next_distance;
        queue_push(next_cell, next_direction, WEST, next_count);

#ifdef MMSIM_ENABLED
        API_setFloodFill(
            next_cell % maze_get_columns(),
            next_cell / maze_get_columns(),
            next_distance);
        if (maze[next_cell] & VISITED_BIT) {
          API_setColor(
              next_cell % maze_get_columns(),
              next_cell / maze_get_columns(),
              'G');
        } else {
          API_clearColor(
              next_cell % maze_get_columns(),
              next_cell / maze_get_columns());
        }
#endif
      }
    }

    if (!wall_exists(current_cell, NORTH_BIT)) {
      next_cell = current_cell + get_direction_value(NORTH);
      next_direction = get_next_floodfill_direction(direction, last_step, NORTH);
      next_count = get_next_floodfill_count(direction, next_direction, count);
      next_distance = get_next_floodfill_distance(floodfill[current_cell], direction, next_direction, next_count, count);
      if ((menu_run_get_floodfill_type() == FLOODFILL_TYPE_BASIC && floodfill[next_cell] > next_distance) ||
          (menu_run_get_floodfill_type() != FLOODFILL_TYPE_BASIC && floodfill[next_cell] >= next_distance)) {
        floodfill[next_cell] = next_distance;
        queue_push(next_cell, next_direction, NORTH, next_count);

#ifdef MMSIM_ENABLED
        API_setFloodFill(
            next_cell % maze_get_columns(),
            next_cell / maze_get_columns(),
            next_distance);
        if (maze[next_cell] & VISITED_BIT) {
          API_setColor(
              next_cell % maze_get_columns(),
              next_cell / maze_get_columns(),
              'G');
        } else {
          API_clearColor(
              next_cell % maze_get_columns(),
              next_cell / maze_get_columns());
        }
#endif
      }
    }
  }

#ifdef MMSIM_ENABLED

  if (!is_race_started()) {
    current_position = 0;
    current_direction = initial_direction;
  }

  uint8_t _position = current_position;
  enum compass_direction _direction = current_direction;

  enum step_direction next_step = NONE;

  if (current_position == 0) {
    API_setColor(
        current_position % maze_get_columns(),
        current_position / maze_get_columns(),
        'B');
  }

  while (floodfill[current_position] > 0) {
    next_step = get_next_floodfill_virtual_step(get_current_stored_virtual_walls());
    if (next_step == NONE) {
      break;
    }
    update_position(next_step);
    // if (!(maze[current_position] & VISITED_BIT)) {
    API_setColor(
        current_position % maze_get_columns(),
        current_position / maze_get_columns(),
        target_cells.stack[0] == 0 ? 'R' : 'B');
    // }
  }

  current_position = _position;
  current_direction = _direction;
#endif

#ifdef MAZE_SIMULATOR
  // Marcar la ruta final igual que en MMS
  // Solo trazar cuando el objetivo NO es el inicio (evitar ciclo infinito)
  if (target_cells.stack[0] != 0) {
    // Guardar posición actual
    uint8_t _position_sim = current_position;
    enum compass_direction _direction_sim = current_direction;

    // Empezar desde el inicio
    current_position = 0;
    current_direction = initial_direction;

    enum step_direction next_step_sim = NONE;

    // Limpiar colores anteriores de ruta (solo ruta, no visitados)
    for (uint16_t i = 0; i < MAZE_CELLS; i++) {
      if (cell_colors[i] == 2) {
        cell_colors[i] = 0;
      }
    }

    // Marcar inicio como ruta
    cell_colors[0] = 2;

    while (floodfill[current_position] > 0) {
      next_step_sim = get_next_floodfill_virtual_step(get_current_stored_virtual_walls());
      if (next_step_sim == NONE) {
        break;
      }
      update_position(next_step_sim);
      cell_colors[current_position] = 2;  // Marcar como ruta final
    }

    // Restaurar posición
    current_position = _position_sim;
    current_direction = _direction_sim;
  }
#endif
}

#ifndef MMSIM_ENABLED
static void save_maze(void) {
  eeprom_set_data(DATA_INDEX_MAZE, maze, MAZE_CELLS);
  eeprom_save();
}

static void check_time_limit(void) {
  if ((time_limit > 0 && get_clock_ticks() - start_ms >= time_limit)) {
    set_target_linear_speed(0);
    set_ideal_angular_speed(0);
    while (get_ideal_linear_speed() != 0) {
      warning_status_led(50);
    }
    set_status_led(false);
    delay(500);
    set_race_started(false);
  }
}
#endif

static uint8_t find_standard_unknown_interesting_cell(void) {
  uint8_t cell = 0;

  uint8_t _position = current_position;
  enum compass_direction _direction = current_direction;

  enum step_direction next_step;

  current_position = 0;
  current_direction = initial_direction;

  // set_target(0);
  set_goal_as_target();
  update_floodfill();
  uint16_t max_iterations = maze_get_cells() * 4;  // Límite razonable
  uint16_t iterations = 0;
  while (floodfill[current_position] > 0) {
    iterations++;
    if (iterations > max_iterations) {
#ifdef MAZE_SIMULATOR
      fprintf(stderr, "[find_standard] bucle infinito! pos=%d, floodfill=%.2f\n", 
              current_position, floodfill[current_position]);
      fflush(stderr);
#endif
      break;
    }
    next_step = get_next_floodfill_virtual_step(get_current_stored_virtual_walls());
    if (next_step == NONE) {
      break;
    }
    update_position(next_step);
    if (!current_cell_is_visited() && !current_cell_is_goal()) {
      cell = current_position;
      break;
    }
  }

#ifdef MMSIM_ENABLED
  fprintf(stderr, "Interesting cell: %d - %d\n", cell % maze_get_columns() + 1, cell / maze_get_columns() + 1);
  fflush(stderr);
#endif

  current_position = _position;
  current_direction = _direction;

  // DEBUG DE EXPLORACIÓN
  // static char *labels[] = {
  //     "goal_x",
  //     "goal_y",
  //     "goal_direction",
  //     "target_x",
  //     "target_y",
  // };
  // macroarray_store(
  //     0,
  //     0b0,
  //     labels,
  //     5,
  //     maze_goal_position % maze_get_columns() + 1,
  //     maze_goal_position / maze_get_columns() + 1,
  //     maze_goal_direction,
  //     cell % maze_get_columns() + 1,
  //     cell / maze_get_columns() + 1);

  return cell;
}

static uint8_t find_closest_unknown_interesting_cell(void) {
  uint8_t cell = 0;

  bool looking_for_from_start = true;
  bool looking_for_from_position = false;

  uint8_t cell_from_start = 0;
  uint16_t cell_from_start_count_to_position = 0;

  uint8_t cell_from_position = 0;
  uint16_t cell_from_position_count_to_position = 0;

  uint8_t _position = current_position;
  enum compass_direction _direction = current_direction;

  enum step_direction next_step;

  current_position = 0;
  current_direction = initial_direction;

  // set_target(0);
  set_goal_as_target();
  update_floodfill();
  uint16_t max_iterations = maze_get_cells() * 4;  // Límite razonable
  uint16_t iterations = 0;
  while (floodfill[current_position] > 0) {
    iterations++;
    if (iterations > max_iterations) {
#ifdef MAZE_SIMULATOR
      fprintf(stderr, "[find_closest] bucle infinito! pos=%d, floodfill=%.2f\n", 
              current_position, floodfill[current_position]);
      fflush(stderr);
#endif
      break;
    }
    next_step = get_next_floodfill_virtual_step(get_current_stored_virtual_walls());
    if (next_step == NONE) {
      break;
    }
    update_position(next_step);
    if (!current_cell_is_visited() && !current_cell_is_goal()) {
      if (looking_for_from_start) {
        cell_from_start = current_position;
        cell_from_start_count_to_position = 0;
      }
      if (looking_for_from_position) {
        cell_from_position = current_position;
        looking_for_from_position = false;
      }
      // cell = current_position;
      // break;
      // continue;
    }
    if (looking_for_from_start) {
      cell_from_start_count_to_position++;
    }
    if (looking_for_from_position) {
      cell_from_position_count_to_position++;
    }
    if (current_position == _position) {
      looking_for_from_start = false;
      looking_for_from_position = true;
    }
  }

  if (cell_from_start != 0 && cell_from_position != 0) {
    if (cell_from_start_count_to_position <= cell_from_position_count_to_position) {
      cell = cell_from_start;
    } else {
      cell = cell_from_position;
    }
  } else if (cell_from_start != 0) {
    cell = cell_from_start;
  } else if (cell_from_position != 0) {
    cell = cell_from_position;
  }

#ifdef MMSIM_ENABLED
  fprintf(stderr, "Interesting cell: %d - %d\n", cell % maze_get_columns() + 1, cell / maze_get_columns() + 1);
  fflush(stderr);
#endif

  current_position = _position;
  current_direction = _direction;

  // DEBUG DE EXPLORACIÓN
  // static char *labels[] = {
  //     "goal_x",
  //     "goal_y",
  //     "goal_direction",
  //     "target_x",
  //     "target_y",
  // };
  // macroarray_store(
  //     0,
  //     0b0,
  //     labels,
  //     5,
  //     maze_goal_position % maze_get_columns() + 1,
  //     maze_goal_position / maze_get_columns() + 1,
  //     maze_goal_direction,
  //     cell % maze_get_columns() + 1,
  //     cell / maze_get_columns() + 1);

  return cell;
}

static uint8_t find_unknown_interesting_cell(void) {
  switch (menu_run_get_floodfill_type()) {
    case FLOODFILL_TYPE_TIMEv2:
      return find_closest_unknown_interesting_cell();
    default:
      return find_standard_unknown_interesting_cell();
  }
}

static bool floodfill_run(void) {
  uint8_t _current_position = current_position;

  uint16_t count_same_direction = 0;

  enum compass_direction next_direction = TARGET;
  do {
    float next_distance = floodfill[_current_position];
    if (!wall_exists(_current_position, NORTH_BIT) && floodfill[_current_position + get_direction_value(NORTH)] < next_distance) {
      next_distance = floodfill[_current_position + get_direction_value(NORTH)];
      next_direction = NORTH;
    }
    if (!wall_exists(_current_position, EAST_BIT) && floodfill[_current_position + get_direction_value(EAST)] < next_distance) {
      next_distance = floodfill[_current_position + get_direction_value(EAST)];
      next_direction = EAST;
    }
    if (!wall_exists(_current_position, SOUTH_BIT) && floodfill[_current_position + get_direction_value(SOUTH)] < next_distance) {
      next_distance = floodfill[_current_position + get_direction_value(SOUTH)];
      next_direction = SOUTH;
    }
    if (!wall_exists(_current_position, WEST_BIT) && floodfill[_current_position + get_direction_value(WEST)] < next_distance) {
      next_distance = floodfill[_current_position + get_direction_value(WEST)];
      next_direction = WEST;
    }

    if (next_direction == current_direction && is_visited(_current_position + get_direction_value(next_direction))) {
      count_same_direction++;
      _current_position += get_direction_value(next_direction);
    } else {
      break;
    }

  } while (next_direction == current_direction && _current_position != 0);

#ifdef MAZE_SIMULATOR
  if (count_same_direction > 0) {
    fprintf(stderr, "  -> floodfill_run: saltando %d celdas, de pos %d a %d\n", count_same_direction, current_position, _current_position);
    fflush(stderr);
  }
#endif

  int8_t next_turn_sign = 0;
  if (current_direction == EAST && next_direction == SOUTH) {
    next_turn_sign = 1;
  } else if (current_direction == EAST && next_direction == NORTH) {
    next_turn_sign = -1;
  } else if (current_direction == SOUTH && next_direction == WEST) {
    next_turn_sign = 1;
  } else if (current_direction == SOUTH && next_direction == EAST) {
    next_turn_sign = -1;
  } else if (current_direction == WEST && next_direction == NORTH) {
    next_turn_sign = 1;
  } else if (current_direction == WEST && next_direction == SOUTH) {
    next_turn_sign = -1;
  } else if (current_direction == NORTH && next_direction == EAST) {
    next_turn_sign = 1;
  } else if (current_direction == NORTH && next_direction == WEST) {
    next_turn_sign = -1;
  }

  if (count_same_direction > 0) {
    run_straight(CELL_DIMENSION * count_same_direction, 0, 0, count_same_direction, false, 3000, get_kinematics().linear_speed, next_turn_sign);
    current_position = _current_position;
    return true;
  }
  return false;
}

static void go_to_target(void) {
  struct walls walls;
  enum step_direction next_step_before_update_walls;
  enum step_direction next_step;
#ifdef MAZE_SIMULATOR
  uint32_t go_to_target_iterations = 0;  // No static - resetear en cada llamada
#endif

  update_floodfill();
  do {
#ifdef MAZE_SIMULATOR
    go_to_target_iterations++;
    if (go_to_target_iterations > 5000) {
      fprintf(stderr, "go_to_target: bucle infinito, pos=%d, floodfill=%.2f\n", current_position, floodfill[current_position]);
      fflush(stderr);
      set_race_started(false);
      return;
    }
#endif
    if (!is_race_started()) {
      return;
    }

    if (!current_cell_is_visited()) {
      next_step_before_update_walls = get_next_floodfill_step(get_current_stored_walls());
      walls = get_walls();
      update_walls(walls);
      update_floodfill();
      next_step = get_next_floodfill_step(walls);
      // Es el target aun interesante?
      if (target_cells.size == 1 && target_cells.stack[0] != 0 && maze_goal_position != 0 && next_step_before_update_walls != next_step) {
        uint8_t interesting_cell = find_unknown_interesting_cell();
        if (interesting_cell == 0) {
          return;
        }
        if (interesting_cell != target_cells.stack[0]) {
          set_target(interesting_cell);
          update_floodfill();
          next_step = get_next_floodfill_step(walls);
        }
      }
    } else {
      walls = get_current_stored_walls();
      next_step = get_next_floodfill_step(walls);
      
      // Durante retorno a casa: evitar celdas no visitadas
      // maze_goal_position != 0 significa que ya llegamos al centro al menos una vez
      // target_cells contiene solo el 0 (casa) cuando volvemos
      if (maze_goal_position != 0 && target_cells.size == 1 && target_cells.stack[0] == 0) {
        uint8_t next_pos = get_next_position(next_step);
        if (!is_visited(next_pos)) {
          // Celda no visitada durante retorno - marcarla como bloqueada virtualmente
          // Añadir pared virtual hacia esa dirección y recalcular
          enum compass_direction wall_dir = get_next_direction(next_step);
          uint8_t wall_bit = (wall_dir == NORTH) ? NORTH_BIT : 
                             (wall_dir == EAST) ? EAST_BIT : 
                             (wall_dir == SOUTH) ? SOUTH_BIT : WEST_BIT;
          maze[current_position] |= wall_bit;
          update_floodfill();
          walls = get_current_stored_walls();
          next_step = get_next_floodfill_step(walls);
        }
      }
    }

#ifndef SIM_MODE
    set_RGB_color_while(255, 255, 0, 33);
#endif

#ifdef MAZE_SIMULATOR
    {
      uint8_t cols = maze_get_columns();
      uint8_t cur_col = current_position % cols;
      uint8_t cur_row = current_position / cols;
      const char* step_str[] = {"FRONT", "LEFT", "RIGHT", "BACK"};
      uint8_t next_pos = get_next_position(next_step);
      bool accel = menu_run_get_accel_explore() != ACCEL_EXPLORE_DISABLED;
      bool next_visited = is_visited(next_pos);
      const char* dir_name = (current_direction == NORTH) ? "N" : 
                             (current_direction == EAST) ? "E" : 
                             (current_direction == SOUTH) ? "S" : 
                             (current_direction == WEST) ? "W" : "?";
      fprintf(stderr, "STEP %u: pos=%d [%d,%d] dir=%s walls=F%d L%d R%d -> next=%s nextpos=%d visited=%d accel=%d\n",
        go_to_target_iterations, current_position, cur_col, cur_row, dir_name,
        walls.front, walls.left, walls.right, step_str[next_step], next_pos, next_visited, accel);
      fflush(stderr);
    }
#endif

    if (menu_run_get_accel_explore() == ACCEL_EXPLORE_DISABLED || !(next_step != BACK && is_visited(get_next_position(next_step)) && floodfill_run())) {
      switch (next_step) {
        case FRONT:
          move(MOVE_FRONT);
          break;
        case LEFT:
          move(MOVE_LEFT);
          break;
        case RIGHT:
          move(MOVE_RIGHT);
          break;
        case BACK:
          if (walls.front) {
            move(MOVE_BACK_WALL);
          } else {
            move(MOVE_BACK);
          }
          break;
        default:
#ifdef SIM_MODE
          // Error en simulador: salir con error
          set_race_started(false);
          return;
#else
          while (true) {
            set_target_linear_speed(0);
            set_ideal_angular_speed(0);
            warning_status_led(50);
          }
#endif
          break;
      }
      update_position(next_step);
    }

  } while (floodfill[current_position] > 0);
  if (!current_cell_is_visited()) {
    walls = get_walls();
    update_walls(walls);
  }

  if (current_cell_is_goal()) {
    move(MOVE_FRONT);
    update_position(FRONT);
    if (!current_cell_is_visited()) {
      walls = get_walls();
      update_walls(walls);
    }
    move(MOVE_BACK_STOP);
    if (maze_goal_position == 0) {
      maze_goal_position = current_position;
      maze_goal_direction = current_direction;
    }
#ifndef MMSIM_ENABLED
    save_maze();
#endif
  }
}

static void build_run_sequence(void) {
  enum step_direction step;

  // set_initial_state();

  set_goals_from_maze();
  // set_goal_as_target();
  set_target(0);

  int16_t bak_maze[MAZE_CELLS];
  memcpy(bak_maze, maze, MAZE_CELLS * sizeof(int16_t));
  for (uint16_t i = 0; i < MAZE_CELLS; i++) {
    if (!(maze[i] & VISITED_BIT)) {
      current_position = i;
      set_wall(NORTH_BIT);
      set_wall(EAST_BIT);
      set_wall(SOUTH_BIT);
      set_wall(WEST_BIT);
    }
  }
  update_floodfill();
  memcpy(maze, bak_maze, MAZE_CELLS * sizeof(int16_t));

  float goal_value = MAZE_MAX_DISTANCE;
  for (uint8_t i = 0; i < goal_cells.size; i++) {
    if (floodfill[goal_cells.stack[i]] < goal_value) {
      goal_value = floodfill[goal_cells.stack[i]];
      current_position = goal_cells.stack[i];
    }
  }
  printf("Current position: %d - %d\n", (current_position % maze_get_columns()) + 1, (current_position / maze_get_rows()) + 1);
  float next_distance = goal_value;
  if (!wall_exists(current_position, NORTH_BIT) && floodfill[current_position + get_direction_value(NORTH)] < next_distance) {
    next_distance = floodfill[current_position + get_direction_value(NORTH)];
    printf("Next cell: %d - %d\n", ((current_position + get_direction_value(NORTH)) % 6) + 1, ((current_position + get_direction_value(NORTH)) / 6) + 1);
    current_direction = NORTH;
  }
  if (!wall_exists(current_position, EAST_BIT) && floodfill[current_position + get_direction_value(EAST)] < next_distance) {
    next_distance = floodfill[current_position + get_direction_value(EAST)];
    printf("Next cell: %d - %d\n", ((current_position + get_direction_value(EAST)) % 6) + 1, ((current_position + get_direction_value(EAST)) / 6) + 1);
    current_direction = EAST;
  }
  if (!wall_exists(current_position, SOUTH_BIT) && floodfill[current_position + get_direction_value(SOUTH)] < next_distance) {
    next_distance = floodfill[current_position + get_direction_value(SOUTH)];
    printf("Next cell: %d - %d\n", ((current_position + get_direction_value(SOUTH)) % 6) + 1, ((current_position + get_direction_value(SOUTH)) / 6) + 1);
    current_direction = SOUTH;
  }
  if (!wall_exists(current_position, WEST_BIT) && floodfill[current_position + get_direction_value(WEST)] < next_distance) {
    next_distance = floodfill[current_position + get_direction_value(WEST)];
    printf("Next cell: %d - %d\n", ((current_position + get_direction_value(WEST)) % 6) + 1, ((current_position + get_direction_value(WEST)) / 6) + 1);
    current_direction = WEST;
  }
  printf("Current direction: %d\n", current_direction);
  printf("Next distance: %f\n", next_distance);

  char reverse_run_sequence[MAZE_CELLS + 3];

  memset(reverse_run_sequence, 0, sizeof(reverse_run_sequence));
  memset(run_sequence, 0, sizeof(run_sequence));

  uint8_t i = 0;
  reverse_run_sequence[i++] = 'S';
  // if (goal_cells.size > 1) {
  //   reverse_run_sequence[i++] = 'F';
  // }
  while (floodfill[current_position] > 0) {
    step = get_next_floodfill_step(get_current_stored_walls());
    switch (step) {
      case FRONT:
        reverse_run_sequence[i++] = (current_position == 0 ? 'B' : 'F');
        break;
      case LEFT:
        reverse_run_sequence[i++] = 'L';
        break;
      case RIGHT:
        reverse_run_sequence[i++] = 'R';
        break;
      default:
        break;
    }
    update_position(step);
  }
  reverse_run_sequence[i++] = 'B';

  i = 0;
  for (int16_t j = strlen(reverse_run_sequence) - 1; j >= 0; j--) {
    if (reverse_run_sequence[j] == 'L') {
      run_sequence[i++] = 'R';
    } else if (reverse_run_sequence[j] == 'R') {
      run_sequence[i++] = 'L';
    } else {
      run_sequence[i++] = reverse_run_sequence[j];
    }
  }

  run_sequence[i] = '\0';

  printf("Run sequence: %s\n", run_sequence);
}

static void smooth_run_sequence(enum speed_strategy speed) {

  uint16_t index = 0;
  while (index < MAZE_CELLS + 3) {
    run_sequence_movements[index++] = MOVE_NONE;
  }
  index = 0;

  enum movement turn_movement;
  bool run_diagonal = false;
  char next_step_1;
  char next_step_2;
  switch (speed) {
    case SPEED_EXPLORE:
      for (uint8_t i = 0; i < strlen(run_sequence); i++) {
        switch (run_sequence[i]) {
          case 'B':
            run_sequence_movements[index++] = MOVE_START;
            break;
          case 'F':
            run_sequence_movements[index++] = MOVE_FRONT;
            break;
          case 'L':
            run_sequence_movements[index++] = MOVE_LEFT;
            break;
          case 'R':
            run_sequence_movements[index++] = MOVE_RIGHT;
            break;
          case 'S':
            run_sequence_movements[index++] = MOVE_HOME;
            break;
        }
      }
      break;

    case SPEED_NORMAL:
    case SPEED_MEDIUM:
    case SPEED_FAST:
    case SPEED_SUPER:
    case SPEED_HAKI:
      printf("Solve strategy: %d\n", menu_run_get_solve_strategy());
      switch (menu_run_get_solve_strategy()) {
        case SOLVE_DIAGONALS:
          for (uint8_t i = 0; i < strlen(run_sequence); i++) {
            switch (run_sequence[i]) {
              case 'B':
                run_sequence_movements[index++] = MOVE_START;
                break;
              case 'F':
                run_sequence_movements[index++] = MOVE_FRONT;
                break;
              case 'L':
                next_step_1 = run_sequence[i + 1];
                next_step_2 = run_sequence[i + 2];
                if (!run_diagonal) {
                  if (next_step_1 == 'F') {
                    run_sequence_movements[index++] = MOVE_LEFT_90;
                  } else if (next_step_1 == 'R') {
                    run_sequence_movements[index++] = MOVE_LEFT_TO_45;
                    run_diagonal = true;
                  } else if (next_step_1 == 'L' && next_step_2 == 'F') {
                    run_sequence_movements[index++] = MOVE_LEFT_180;
                    i++;
                  } else if (next_step_1 == 'L' && next_step_2 == 'R') {
                    run_sequence_movements[index++] = MOVE_LEFT_TO_135;
                    run_diagonal = true;
                    i++;
                  }
                } else {
                  if (next_step_1 == 'R') {
                    run_sequence_movements[index++] = MOVE_DIAGONAL;
                  } else if (next_step_1 == 'F') {
                    run_sequence_movements[index++] = MOVE_LEFT_FROM_45;
                    run_diagonal = false;
                  } else if (next_step_1 == 'L' && next_step_2 == 'R') {
                    run_sequence_movements[index++] = MOVE_LEFT_45_TO_45;
                    i++;
                  } else if (next_step_1 == 'L' && next_step_2 == 'F') {
                    run_sequence_movements[index++] = MOVE_LEFT_FROM_45_180;
                    i++;
                    run_diagonal = false;
                  }
                }
                break;
              case 'R':
                next_step_1 = run_sequence[i + 1];
                next_step_2 = run_sequence[i + 2];
                if (!run_diagonal) {
                  if (next_step_1 == 'F') {
                    run_sequence_movements[index++] = MOVE_RIGHT_90;
                  } else if (next_step_1 == 'L') {
                    run_sequence_movements[index++] = MOVE_RIGHT_TO_45;
                    run_diagonal = true;
                  } else if (next_step_1 == 'R' && next_step_2 == 'F') {
                    run_sequence_movements[index++] = MOVE_RIGHT_180;
                    i++;
                  } else if (next_step_1 == 'R' && next_step_2 == 'L') {
                    run_sequence_movements[index++] = MOVE_RIGHT_TO_135;
                    run_diagonal = true;
                    i++;
                  }
                } else {
                  if (next_step_1 == 'L') {
                    run_sequence_movements[index++] = MOVE_DIAGONAL;
                  } else if (next_step_1 == 'F') {
                    run_sequence_movements[index++] = MOVE_RIGHT_FROM_45;
                    run_diagonal = false;
                  } else if (next_step_1 == 'R' && next_step_2 == 'L') {
                    run_sequence_movements[index++] = MOVE_RIGHT_45_TO_45;
                    i++;
                  } else if (next_step_1 == 'R' && next_step_2 == 'F') {
                    run_sequence_movements[index++] = MOVE_RIGHT_FROM_45_180;
                    i++;
                    run_diagonal = false;
                  }
                }
                break;
              case 'S':
                run_sequence_movements[index++] = MOVE_HOME;
                break;
            }
          }
          break;
        case SOLVE_STANDARD:
        default:
          for (uint8_t i = 0; i < strlen(run_sequence); i++) {
            switch (run_sequence[i]) {
              case 'B':
                run_sequence_movements[index++] = MOVE_START;
                break;
              case 'F':
                run_sequence_movements[index++] = MOVE_FRONT;
                break;
              case 'L':
                if (i + 1 < (uint16_t)strlen(run_sequence) && run_sequence[i + 1] == 'L') {
                  turn_movement = MOVE_LEFT_180;
                  i++;
                } else {
                  turn_movement = MOVE_LEFT_90;
                }
                run_sequence_movements[index++] = turn_movement;
                break;
              case 'R':
                if (i + 1 < (uint16_t)strlen(run_sequence) && run_sequence[i + 1] == 'R') {
                  turn_movement = MOVE_RIGHT_180;
                  i++;
                } else {
                  turn_movement = MOVE_RIGHT_90;
                }
                run_sequence_movements[index++] = turn_movement;
                break;
              case 'S':
                run_sequence_movements[index++] = MOVE_HOME;
                break;
            }
          }
          break;
      }
      break;
  }

  uint16_t count_same_move = 0;
  for (uint16_t i = 0; i < index; i++) {
    count_same_move = 1;
    while (run_sequence_movements[i] == run_sequence_movements[i + 1]) {
      count_same_move++;
      i++;
    }
    if (count_same_move > 1) {
      printf("%dx", count_same_move);
    }
    printf("%s", get_movement_string(run_sequence_movements[i]));
    if (i + 1 == index) {
      printf("\n");
    } else {
      printf(" > ");
    }
  }
}

#ifndef MMSIM_ENABLED
static void floodfill_explore_finish(void) {
  set_target_linear_speed(0);
  set_ideal_angular_speed(0);
  set_target_fan_speed(0, 400);
  set_race_started(false);
  uint16_t ms = get_clock_ticks();
  while (get_clock_ticks() - ms < 1000) {
    warning_status_led(50);
    set_RGB_rainbow();
  }
  set_RGB_color(255, 255, 0);
  set_status_led(false);
  save_maze();
  set_RGB_color(0, 255, 0);
  delay(500);
}
#endif

static void loop_explore(void) {
  while (is_race_started()) {
    go_to_target();

    uint8_t interesting_cell = find_unknown_interesting_cell();
    if (current_position == 0) {
      // if (current_position == 0 || (interesting_cell == 0 /*  && current_cell_is_goal() */)) {
      if (!current_cell_is_goal()) {
        if (get_current_stored_walls().front) {
          move(MOVE_HOME);
        } else {
          move(MOVE_END);
        }
      }

#ifndef SIM_MODE
      floodfill_explore_finish();
      if (is_race_auto_run()) {
        set_race_started(true);
        floodfill_start_run();
        return;
      }
#else
      set_race_started(false);
#endif

      set_goal_as_target();
      update_floodfill();
      return;
    } else if (current_cell_is_goal() && get_ideal_linear_speed() == 0) {
      move(MOVE_START);
      update_position(BACK);
    }
    set_target(interesting_cell);

#ifndef SIM_MODE
    check_time_limit();
#endif
  }
}

static void loop_run(void) {
  move_run_sequence(run_sequence_movements);

#ifndef SIM_MODE
  set_target_linear_speed(0);
  set_ideal_angular_speed(0);
  set_target_fan_speed(0, 400);
  if (!is_motor_saturated()) {
    set_RGB_color_while(255, 0, 0, 33);
    uint16_t ms = get_clock_ticks();
    while (get_clock_ticks() - ms < 1000) {
      warning_status_led(50);
    }
    set_race_started(false);
  }
  set_status_led(false);
#else
  set_race_started(false);
#endif
}

#ifndef MMSIM_ENABLED
void floodfill_load_maze(void) {
  int16_t *data = eeprom_get_data();
  for (uint16_t i = DATA_INDEX_MAZE; i < (DATA_INDEX_MAZE + MAZE_CELLS); i++) {
    maze[i - DATA_INDEX_MAZE] = data[i];
  }
}
#endif

void floodfill_maze_print(void) {
  initialize_directions_values();
  configure_kinematics(menu_run_get_speed());
  build_run_sequence();
  smooth_run_sequence(menu_run_get_speed());
  // update_floodfill();
  for (int16_t r = maze_get_cells() - maze_get_columns(); r >= 0; r = r - maze_get_columns()) {
    // Borde superior del laberinto
    if (r == maze_get_cells() - maze_get_columns()) {
      printf("·");
      for (uint16_t i = maze_get_cells() - maze_get_columns(); i < maze_get_cells(); i++) {
        if (maze[i] & NORTH_BIT) {
          printf("═══════·");
        } else {
          printf("       ·");
        }
      }
    }
    printf("\n");

    // Paredes laterales del laberinto y VISITADO
    for (int16_t c = r; c < r + maze_get_columns(); c++) {
      if (maze[c] & WEST_BIT /* || c % maze_get_columns() == 0 */) {
        printf("║");
      } else {
        printf(" ");
      }
      // if (maze[c] & VISITED_BIT) {
      //   // printf(" V ");
      //   printf("%7.4f", floodfill[c]);
      // } else {
      //   // printf("   ");
      //   printf("%7.4f", floodfill[c]);
      // }

      if (floodfill[c] > 99) {
        printf("%7.2f", floodfill[c]);
      } else if (floodfill[c] > 9) {
        printf("%7.3f", floodfill[c]);
      } else {
        printf("%7.4f", floodfill[c]);
      }

      //   if ((c + 1 % maze_get_columns()) == 0) {
      //     printf("|");
      //   }
      if (maze[c] & EAST_BIT && ((c + 1) % maze_get_columns()) == 0) {
        printf("║");
      }
    }
    // printf("║\n");
    printf("\n");

    // Paredes inferiores del laberinto
    printf("·");
    for (int16_t c = r; c < r + maze_get_columns(); c++) {
      if (maze[c] & SOUTH_BIT /* || c / maze_get_columns() == 0 */) {
        printf("═══════·");
      } else {
        printf("       ·");
      }
    }
  }
  printf("\n");

  for (int16_t r = maze_get_cells() - maze_get_columns(); r >= 0; r = r - maze_get_columns()) {
    // Borde superior del laberinto
    if (r == maze_get_cells() - maze_get_columns()) {
      printf("·");
      for (uint16_t i = maze_get_cells() - maze_get_columns(); i < maze_get_cells(); i++) {
        if (maze[i] & NORTH_BIT) {
          printf("═══════·");
        } else {
          printf("       ·");
        }
      }
    }
    printf("\n");

    // Paredes laterales del laberinto y VISITADO
    for (int16_t c = r; c < r + maze_get_columns(); c++) {
      if (maze[c] & WEST_BIT /* || c % maze_get_columns() == 0 */) {
        printf("║");
      } else {
        printf(" ");
      }
#ifdef MAZE_SIMULATOR
      if (cell_colors[c] == 2) {
        printf("  ███  ");  // Ruta final (azul en MMS)
      } else if (maze[c] & VISITED_BIT) {
        printf("   V   ");
      } else {
        printf("       ");
      }
#else
      if (maze[c] & VISITED_BIT) {
        printf("   V   ");
        // printf("%5.2f", floodfill[c]);
      } else {
        printf("       ");
        // printf("%5.2f", floodfill[c]);
      }
#endif
      //   if ((c + 1 % maze_get_columns()) == 0) {
      //     printf("|");
      //   }
      if (maze[c] & EAST_BIT && ((c + 1) % maze_get_columns()) == 0) {
        printf("║");
      }
    }
    // printf("║\n");
    printf("\n");

    // Paredes inferiores del laberinto
    printf("·");
    for (int16_t c = r; c < r + maze_get_columns(); c++) {
      if (maze[c] & SOUTH_BIT /* || c / maze_get_columns() == 0 */) {
        printf("═══════·");
      } else {
        printf("       ·");
      }
    }
  }
  printf("\n");
}

void floodfill_set_time_limit(uint32_t ms) {
  time_limit = ms;
}

void floodfill_set_reset_maze_on_start_explore(bool reset) {
  reset_maze_on_start_explore = reset;
}

bool floodfill_is_reset_maze_on_start_explore(void) {
  return reset_maze_on_start_explore;
}

void floodfill_start_explore(void) {
  configure_kinematics(SPEED_EXPLORE);

#ifndef SIM_MODE
  clear_info_leds();
  set_RGB_color(0, 0, 0);
  delay(125);
  side_sensors_calibration(true);
  delay(125);
  set_target_fan_speed(get_kinematics().fan_speed, 400);
  start_ms = get_clock_ticks();
  delay(800);
#endif

  race_mode = false;
  maze_goal_position = 0;

#ifdef MAZE_SIMULATOR
  fprintf(stderr, "[DEBUG] Paso 1: initialize_directions_values()\n"); fflush(stderr);
#endif
  initialize_directions_values();
  if (reset_maze_on_start_explore) {
#ifdef MAZE_SIMULATOR
    fprintf(stderr, "[DEBUG] Paso 2: initialize_maze()\n"); fflush(stderr);
#endif
    initialize_maze();
    reset_maze_on_start_explore = false;
  }
#ifdef MAZE_SIMULATOR
  fprintf(stderr, "[DEBUG] Paso 3: set_initial_state()\n"); fflush(stderr);
#endif
  set_initial_state();

#ifdef MAZE_SIMULATOR
  fprintf(stderr, "[DEBUG] Paso 4: set_goals_from_maze()\n"); fflush(stderr);
#endif
  set_goals_from_maze();
#ifdef MAZE_SIMULATOR
  fprintf(stderr, "[DEBUG] Paso 5: set_goal_as_target()\n"); fflush(stderr);
#endif
  set_goal_as_target();

#ifdef MAZE_SIMULATOR
  fprintf(stderr, "[DEBUG] Paso 6: find_unknown_interesting_cell()\n"); fflush(stderr);
#endif
  if (find_unknown_interesting_cell() == 0) {
#ifndef SIM_MODE
    floodfill_explore_finish();
#else
    set_race_started(false);
#endif
    return;
  }

#ifdef MAZE_SIMULATOR
  fprintf(stderr, "[DEBUG] Paso 7: get_walls()\n"); fflush(stderr);
#endif
  struct walls walls = get_walls();
#ifdef MAZE_SIMULATOR
  fprintf(stderr, "[DEBUG] Paso 8: update_walls()\n"); fflush(stderr);
#endif
  update_walls(walls);
#ifdef MAZE_SIMULATOR
  fprintf(stderr, "[DEBUG] Paso 9: update_floodfill()\n"); fflush(stderr);
#endif
  update_floodfill();

#ifdef MAZE_SIMULATOR
  // DEBUG: Imprimir targets y valores iniciales de floodfill
  fprintf(stderr, "[DEBUG] ===== ESTADO INICIAL =====\n");
  fprintf(stderr, "[DEBUG] current_position: %d\n", current_position);
  fprintf(stderr, "[DEBUG] current_direction: %d\n", current_direction);
  fprintf(stderr, "[DEBUG] goal_cells.size: %d\n", goal_cells.size);
  for (uint8_t i = 0; i < goal_cells.size; i++) {
    fprintf(stderr, "[DEBUG]   goal[%d] = %d (floodfill=%.2f)\n", i, goal_cells.stack[i], floodfill[goal_cells.stack[i]]);
  }
  fprintf(stderr, "[DEBUG] target_cells.size: %d\n", target_cells.size);
  for (uint8_t i = 0; i < target_cells.size; i++) {
    fprintf(stderr, "[DEBUG]   target[%d] = %d (floodfill=%.2f)\n", i, target_cells.stack[i], floodfill[target_cells.stack[i]]);
  }
  fprintf(stderr, "[DEBUG] floodfill[0] (start) = %.2f\n", floodfill[0]);
  fprintf(stderr, "[DEBUG] floodfill_type = %d\n", menu_run_get_floodfill_type());
  fprintf(stderr, "[DEBUG] ==============================\n");
  fflush(stderr);
#endif

  move(MOVE_START);
  update_position(FRONT);

  // move_inplace_turn(MOVE_BACK);
  // delay(200);
  // set_race_started(false);
}

void floodfill_start_run(void) {
  configure_kinematics(menu_run_get_speed());
  race_mode = true;
#ifndef SIM_MODE
  clear_info_leds();
  set_RGB_color(0, 0, 0);
  delay(125);
  side_sensors_calibration(true);
  delay(125);
  set_target_fan_speed(get_kinematics().fan_speed, 1000);
  delay(1500);
#endif

  initialize_directions_values();
  build_run_sequence();
  smooth_run_sequence(menu_run_get_speed());
}

void floodfill_loop(void) {
  if (race_mode) {
    loop_run();
  } else {
    loop_explore();
  }
}

// ============== MAZE SIMULATOR ==============
#ifdef MAZE_SIMULATOR

void floodfill_simulator_set_maze_size(uint8_t rows, uint8_t cols) {
  sim_maze_rows = rows;
  sim_maze_cols = cols;
}

void floodfill_simulator_set_true_maze(int16_t *maze_array, uint16_t size) {
  for (uint16_t i = 0; i < size && i < MAZE_CELLS; i++) {
    true_maze[i] = maze_array[i];
  }
}

// Simula get_walls() leyendo del true_maze en lugar de sensores
// Pública para que maze_simulator.c pueda llamarla desde su stub get_walls()
struct walls simulator_get_walls(void) {
  struct walls walls;
  int16_t cell = true_maze[current_position];
  
  switch (current_direction) {
    case EAST:
      walls.left = (bool)(cell & NORTH_BIT);
      walls.front = (bool)(cell & EAST_BIT);
      walls.right = (bool)(cell & SOUTH_BIT);
      break;
    case SOUTH:
      walls.left = (bool)(cell & EAST_BIT);
      walls.front = (bool)(cell & SOUTH_BIT);
      walls.right = (bool)(cell & WEST_BIT);
      break;
    case WEST:
      walls.left = (bool)(cell & SOUTH_BIT);
      walls.front = (bool)(cell & WEST_BIT);
      walls.right = (bool)(cell & NORTH_BIT);
      break;
    case NORTH:
    default:
      walls.left = (bool)(cell & WEST_BIT);
      walls.front = (bool)(cell & NORTH_BIT);
      walls.right = (bool)(cell & EAST_BIT);
      break;
  }
  return walls;
}

// Funciones de utilidad para el simulador
uint16_t floodfill_count_visited(void) {
  uint16_t count = 0;
  uint16_t cells = sim_maze_rows * sim_maze_cols;
  for (uint16_t i = 0; i < cells; i++) {
    if (maze[i] & VISITED_BIT) {
      count++;
    }
  }
  return count;
}

int16_t floodfill_get_maze_cell(uint8_t pos) {
  return maze[pos];
}

#endif // MAZE_SIMULATOR