#include <floodfill.h>

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
    case FLOODFILL_TYPE_TIME: {
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

  while (cells_queue.head != cells_queue.tail) {
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

static uint8_t find_unknown_interesting_cell(void) {
  uint8_t cell = 0;

  uint8_t _position = current_position;
  enum compass_direction _direction = current_direction;

  enum step_direction next_step;

  // if (maze_goal_position != 0) {
  //   current_position = maze_goal_position;
  //   current_direction = maze_goal_direction;
  // } else {
  //   current_position = goal_cells.stack[0];
  //   current_direction = NORTH;
  // }
  current_position = 0;
  current_direction = initial_direction;

  // set_target(0);
  set_goal_as_target();
  update_floodfill();
  while (floodfill[current_position] > 0) {
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
  // #ifdef MMSIM_ENABLED
  // API_turnLeft();
  // API_turnLeft();
  // API_turnLeft();
  // API_turnLeft();
  // #endif

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

  } while (next_direction == current_direction);

  if (count_same_direction > 0) {
    run_straight(CELL_DIMENSION * count_same_direction, 0, 0, count_same_direction, false, 2000, get_kinematics().linear_speed);
    current_position = _current_position;
    return true;
  }
  return false;
}

static void go_to_target(void) {
  struct walls walls;
  enum step_direction next_step_before_update_walls;
  enum step_direction next_step;

  update_floodfill();
  do {
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
    }

#ifndef MMSIM_ENABLED
    set_RGB_color_while(255, 255, 0, 33);
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
          while (true) {
#ifndef MMSIM_ENABLED
            set_target_linear_speed(0);
            set_ideal_angular_speed(0);
            warning_status_led(50);
#endif
          }
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

  for (uint16_t i = 0; i < strlen(run_sequence); i++) {
    run_sequence[i] = '\0';
  }

  uint8_t i = 0;
  char reverse_run_sequence[MAZE_CELLS + 3];
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

static void floodfill_explore_finish(void) {
  set_target_linear_speed(0);
  set_ideal_angular_speed(0);
  set_target_fan_speed(0, 400);
  uint16_t ms = get_clock_ticks();
  while (get_clock_ticks() - ms < 2000) {
    warning_status_led(50);
    set_RGB_rainbow();
  }
  set_RGB_color(255, 255, 0);
  set_status_led(false);
  set_race_started(false);
  save_maze();
  set_RGB_color(0, 255, 0);
  delay(500);
}

static void loop_explore(void) {
  while (is_race_started()) {
    go_to_target();

    uint8_t interesting_cell = find_unknown_interesting_cell();
    if (current_position == 0 || (interesting_cell == 0 /*  && current_cell_is_goal() */)) {
      if (!current_cell_is_goal()) {
        if (get_current_stored_walls().front) {
          move(MOVE_HOME);
        } else {
          move(MOVE_END);
        }
      }

#ifndef MMSIM_ENABLED
      floodfill_explore_finish();
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

#ifndef MMSIM_ENABLED
    check_time_limit();
#endif
  }
}

static void loop_run(void) {
  move_run_sequence(run_sequence_movements);

#ifndef MMSIM_ENABLED
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
      if (maze[c] & VISITED_BIT) {
        printf("   V   ");
        // printf("%5.2f", floodfill[c]);
      } else {
        printf("       ");
        // printf("%5.2f", floodfill[c]);
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

  race_mode = false;
  maze_goal_position = 0;

#ifndef MMSIM_ENABLED
  clear_info_leds();
  set_RGB_color(0, 0, 0);
  set_target_fan_speed(get_kinematics().fan_speed, 400);
  start_ms = get_clock_ticks();
  delay(500);
#endif

  initialize_directions_values();
  if (reset_maze_on_start_explore) {
    initialize_maze();
    reset_maze_on_start_explore = false;
  }
  set_initial_state();

  set_goals_from_maze();
  set_goal_as_target();

  if (find_unknown_interesting_cell() == 0) {
#ifndef MMSIM_ENABLED
    floodfill_explore_finish();
#else
    set_race_started(false);
#endif
    return;
  }

  struct walls walls = get_walls();
  update_walls(walls);
  update_floodfill();

  move(MOVE_START);
  update_position(FRONT);
}

void floodfill_start_run(void) {
  configure_kinematics(menu_run_get_speed());
  race_mode = true;
#ifndef MMSIM_ENABLED
  clear_info_leds();
  set_RGB_color(0, 0, 0);
  set_target_fan_speed(get_kinematics().fan_speed, 400);
  delay(500);
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