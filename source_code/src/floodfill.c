#include <floodfill.h>

static uint32_t start_ms = 0;
static uint32_t time_limit = 0;

static uint8_t floodfill[MAZE_CELLS];
static uint16_t maze[MAZE_CELLS];

static enum compass_direction initial_direction = NORTH;

static uint8_t current_position = 0;
static enum compass_direction current_direction = NORTH;

static struct cells_queue cells_queue;

static struct cells_stack target_cells;
static struct cells_stack goal_cells;

static bool race_mode = false;
static char run_sequence[MAZE_CELLS + 3];

static void initialize_maze(void) {
  for (uint16_t i = 0; i < MAZE_CELLS; i++) {
    maze[i] = 0;
  }

  for (uint16_t i = 0; i < MAZE_ROWS; i++) {
    maze[(MAZE_COLUMNS - 1) + i * (MAZE_COLUMNS)] |= EAST_BIT;
    maze[i * MAZE_COLUMNS] |= WEST_BIT;
  }

  for (uint16_t i = 0; i < MAZE_COLUMNS; i++) {
    maze[i] |= SOUTH_BIT;
    maze[(MAZE_ROWS - 1) * MAZE_COLUMNS + i] |= NORTH_BIT;
  }
}

static void set_initial_state(void) {
  current_position = 0;
  current_direction = initial_direction;
}

static void add_goal(uint8_t x, uint8_t y) {
  goal_cells.stack[goal_cells.size++] = (x - 1) + (y - 1) * MAZE_COLUMNS;
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

static uint8_t get_next_position(enum step_direction step) {
  return current_position + get_next_direction(step);
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
        if (current_position % MAZE_COLUMNS == MAZE_COLUMNS - 1) {
          break;
        }
        maze[current_position + EAST] |= WEST_BIT;
        break;
      case SOUTH_BIT:
        if (current_position / MAZE_COLUMNS == 0) {
          break;
        }
        maze[current_position + SOUTH] |= NORTH_BIT;
        break;
      case WEST_BIT:
        if (current_position % MAZE_COLUMNS == 0) {
          break;
        }
        maze[current_position + WEST] |= EAST_BIT;
        break;
      case NORTH_BIT:
        if (current_position / MAZE_COLUMNS == MAZE_ROWS - 1) {
          break;
        }
        maze[current_position + NORTH] |= SOUTH_BIT;
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

static void set_visited(void) {
  maze[current_position] |= VISITED_BIT;
}

static bool current_cell_is_visited(void) {
  return maze[current_position] & VISITED_BIT;
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
  }
  if (cell_walls[1]) {
    set_wall(SOUTH_BIT);
  }
  if (cell_walls[2]) {
    set_wall(WEST_BIT);
  }
  if (cell_walls[3]) {
    set_wall(NORTH_BIT);
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

static void queue_push(uint8_t position) {
  cells_queue.queue[cells_queue.head++] = position;
}

static uint8_t queue_pop(void) {
  return cells_queue.queue[cells_queue.tail++];
}

static void update_floodfill(void) {
  reset_floodfill_and_queue();

  for (uint8_t i = 0; i < target_cells.size; i++) {
    floodfill[target_cells.stack[i]] = 0;
    queue_push(target_cells.stack[i]);
  }

  while (cells_queue.head != cells_queue.tail) {
    uint8_t current_cell = queue_pop();
    uint8_t next_distance = floodfill[current_cell] + 1;
    if (!wall_exists(current_cell, EAST_BIT) && floodfill[current_cell + EAST] > next_distance) {
      floodfill[current_cell + EAST] = next_distance;
      queue_push(current_cell + EAST);
    }
    if (!wall_exists(current_cell, SOUTH_BIT) && floodfill[current_cell + SOUTH] > next_distance) {
      floodfill[current_cell + SOUTH] = next_distance;
      queue_push(current_cell + SOUTH);
    }
    if (!wall_exists(current_cell, WEST_BIT) && floodfill[current_cell + WEST] > next_distance) {
      floodfill[current_cell + WEST] = next_distance;
      queue_push(current_cell + WEST);
    }
    if (!wall_exists(current_cell, NORTH_BIT) && floodfill[current_cell + NORTH] > next_distance) {
      floodfill[current_cell + NORTH] = next_distance;
      queue_push(current_cell + NORTH);
    }
  }
}

static enum step_direction get_next_floodfill_step(struct walls walls) {
  if (!walls.front && floodfill[get_next_position(FRONT)] < floodfill[current_position]) {
    return FRONT;
  }
  if (!walls.left && floodfill[get_next_position(LEFT)] < floodfill[current_position]) {
    return LEFT;
  }
  if (!walls.right && floodfill[get_next_position(RIGHT)] < floodfill[current_position]) {
    return RIGHT;
  }
  return BACK;
}

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

static uint8_t find_unknown_interesting_cell(void) {
  uint8_t cell = 0;

  uint8_t _position = current_position;
  enum compass_direction _direction = current_direction;

  enum step_direction next_step;

  set_initial_state();
  set_goal_as_target();
  update_floodfill();
  while (floodfill[current_position] > 0) {
    next_step = get_next_floodfill_step(get_current_stored_walls());
    update_position(next_step);
    if (!current_cell_is_visited()) {
      cell = current_position;
      break;
    }
  }

  current_position = _position;
  current_direction = _direction;
  return cell;
}

static void go_to_target(void) {
  struct walls walls;
  enum step_direction next_step;

  update_floodfill();
  do {
    if (!current_cell_is_visited()) {
      walls = get_walls();
      update_walls(walls);
      update_floodfill();
    } else {
      walls = get_current_stored_walls();
    }

    next_step = get_next_floodfill_step(walls);
    update_position(next_step);
    set_RGB_color_while(0, 0, 255, 20);
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
          move(MOVE_180W);
        } else {
          move(MOVE_180);
        }
        break;
      default:
        while (true) {
          set_target_linear_speed(0);
          set_ideal_angular_speed(0);
          warning_status_led(50);
        }
        break;
    }
  } while (floodfill[current_position] > 0);
  walls = get_walls();
  update_walls(walls);
}

static void build_run_sequence(void) {
  uint8_t i = 0;
  enum step_direction step;

  set_initial_state();
  add_goal(6, 4);
  add_goal(6, 5);
  add_goal(7, 4);
  add_goal(7, 5);
  set_goal_as_target();
  update_floodfill();

  while (floodfill[current_position] > 0) {
    step = get_next_floodfill_step(get_current_stored_walls());
    switch (step) {
      case FRONT:
        run_sequence[i++] = (current_position == 0 ? 'B' : 'F');
        break;
      case LEFT:
        run_sequence[i++] = 'L';
        break;
      case RIGHT:
        run_sequence[i++] = 'R';
        break;
      default:
        break;
    }
    update_position(step);
  }
  // while (true) {
  //   update_position(FRONT);
  //   if (floodfill[current_position] != 0) {
  //     break;
  //   }
  //   run_sequence[i++] = 'F';
  // }
  if (goal_cells.size > 1) {
    run_sequence[i++] = 'F';
  }
  run_sequence[i++] = 'S';
  run_sequence[i] = '\0';

  printf("Run sequence: %s\n", run_sequence);
}

static void loop_explore(void) {
  while (is_race_started()) {
    go_to_target();

    if (current_position == 0) {
      move(MOVE_HOME);
      set_target_linear_speed(0);
      set_ideal_angular_speed(0);
      set_RGB_color_while(255, 0, 0, 20);
      uint16_t ms = get_clock_ticks();
      while (get_clock_ticks() - ms < 2000) {
        warning_status_led(50);
      }
      set_status_led(false);
      set_race_started(false);
      set_goal_as_target();
      update_floodfill();
      save_maze();
      return;
    }
    set_target(find_unknown_interesting_cell());

    check_time_limit();
  }
}

static void loop_run(void) {
  build_run_sequence();
  move_run_sequence(run_sequence);

  set_target_linear_speed(0);
  set_ideal_angular_speed(0);
  set_RGB_color_while(255, 0, 0, 20);
  uint16_t ms = get_clock_ticks();
  while (get_clock_ticks() - ms < 1000) {
    warning_status_led(50);
  }
  set_status_led(false);
  set_race_started(false);
}

void floodfill_load_maze(void) {
  uint16_t *data = eeprom_get_data();
  for (uint16_t i = DATA_INDEX_MAZE; i < (DATA_INDEX_MAZE + MAZE_CELLS); i++) {
    maze[i - DATA_INDEX_MAZE] = data[i];
  }
}

void floodfill_maze_print(void) {
  build_run_sequence();
  for (int16_t r = MAZE_CELLS - MAZE_COLUMNS; r >= 0; r = r - MAZE_COLUMNS) {
    // Borde superior del laberinto
    if (r == MAZE_CELLS - MAZE_COLUMNS) {
      printf("·");
      for (uint16_t i = MAZE_CELLS - MAZE_COLUMNS; i < MAZE_CELLS; i++) {
        if (maze[i] & NORTH_BIT) {
          printf("═══·");
        } else {
          printf("   ·");
        }
      }
    }
    printf("\n");

    // Paredes laterales del laberinto y VISITADO
    for (int16_t c = r; c < r + MAZE_COLUMNS; c++) {
      if (maze[c] & WEST_BIT /* || c % MAZE_COLUMNS == 0 */) {
        printf("║");
      } else {
        printf(" ");
      }
      if (maze[c] & VISITED_BIT) {
        // printf(" V ");
        printf("%3d", floodfill[c]);
      } else {
        // printf("   ");
        printf("%3d", floodfill[c]);
      }
      //   if ((c + 1 % MAZE_COLUMNS) == 0) {
      //     printf("|");
      //   }
      if (maze[c] & EAST_BIT && ((c + 1) % MAZE_COLUMNS) == 0) {
        printf("║");
      }
    }
    // printf("║\n");
    printf("\n");

    // Paredes inferiores del laberinto
    printf("·");
    for (int16_t c = r; c < r + MAZE_COLUMNS; c++) {
      if (maze[c] & SOUTH_BIT /* || c / MAZE_COLUMNS == 0 */) {
        printf("═══·");
      } else {
        printf("   ·");
      }
    }
  }
  printf("\n");

  for (int16_t r = MAZE_CELLS - MAZE_COLUMNS; r >= 0; r = r - MAZE_COLUMNS) {
    // Borde superior del laberinto
    if (r == MAZE_CELLS - MAZE_COLUMNS) {
      printf("·");
      for (uint16_t i = MAZE_CELLS - MAZE_COLUMNS; i < MAZE_CELLS; i++) {
        if (maze[i] & NORTH_BIT) {
          printf("═══·");
        } else {
          printf("   ·");
        }
      }
    }
    printf("\n");

    // Paredes laterales del laberinto y VISITADO
    for (int16_t c = r; c < r + MAZE_COLUMNS; c++) {
      if (maze[c] & WEST_BIT /* || c % MAZE_COLUMNS == 0 */) {
        printf("║");
      } else {
        printf(" ");
      }
      if (maze[c] & VISITED_BIT) {
        printf(" V ");
        // printf("%3d", floodfill[c]);
      } else {
        printf("   ");
        // printf("%3d", floodfill[c]);
      }
      //   if ((c + 1 % MAZE_COLUMNS) == 0) {
      //     printf("|");
      //   }
      if (maze[c] & EAST_BIT && ((c + 1) % MAZE_COLUMNS) == 0) {
        printf("║");
      }
    }
    // printf("║\n");
    printf("\n");

    // Paredes inferiores del laberinto
    printf("·");
    for (int16_t c = r; c < r + MAZE_COLUMNS; c++) {
      if (maze[c] & SOUTH_BIT /* || c / MAZE_COLUMNS == 0 */) {
        printf("═══·");
      } else {
        printf("   ·");
      }
    }
  }
  printf("\n");
}

void floodfill_set_time_limit(uint32_t ms) {
  time_limit = ms;
}

void floodfill_start_explore(void) {
  race_mode = false;
  initialize_maze();
  set_initial_state();

  add_goal(6, 4);
  add_goal(6, 5);
  add_goal(7, 4);
  add_goal(7, 5);
  set_goal_as_target();

  struct walls walls = get_walls();
  update_walls(walls);
  update_floodfill();

  start_ms = get_clock_ticks();

  move(MOVE_START);
  update_position(FRONT);
}

void floodfill_start_run(void) {
  race_mode = true;
  // TODO: race_mode things
}

void floodfill_loop(void) {
  if (race_mode) {
    loop_run();
  } else {
    loop_explore();
  }
}