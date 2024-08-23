#include <floodfill.h>

static uint32_t start_ms = 0;
static uint32_t time_limit = 0;

static uint8_t floodfill[MAZE_CELLS];
static int16_t maze[MAZE_CELLS];

static enum compass_direction initial_direction = NORTH;

static uint8_t current_position = 0;
static enum compass_direction current_direction = NORTH;

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
  }

  for (uint16_t i = 0; i < maze_get_columns(); i++) {
    maze[i] |= SOUTH_BIT;
    maze[(maze_get_rows() - 1) * maze_get_columns() + i] |= NORTH_BIT;
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
    if (!wall_exists(current_cell, EAST_BIT) && floodfill[current_cell + get_direction_value(EAST)] > next_distance) {
      floodfill[current_cell + get_direction_value(EAST)] = next_distance;
      queue_push(current_cell + get_direction_value(EAST));
    }
    if (!wall_exists(current_cell, SOUTH_BIT) && floodfill[current_cell + get_direction_value(SOUTH)] > next_distance) {
      floodfill[current_cell + get_direction_value(SOUTH)] = next_distance;
      queue_push(current_cell + get_direction_value(SOUTH));
    }
    if (!wall_exists(current_cell, WEST_BIT) && floodfill[current_cell + get_direction_value(WEST)] > next_distance) {
      floodfill[current_cell + get_direction_value(WEST)] = next_distance;
      queue_push(current_cell + get_direction_value(WEST));
    }
    if (!wall_exists(current_cell, NORTH_BIT) && floodfill[current_cell + get_direction_value(NORTH)] > next_distance) {
      floodfill[current_cell + get_direction_value(NORTH)] = next_distance;
      queue_push(current_cell + get_direction_value(NORTH));
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
          move(MOVE_BACK_WALL);
        } else {
          move(MOVE_BACK);
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
  enum step_direction step;

  set_initial_state();

  set_goals_from_maze();
  set_goal_as_target();
  update_floodfill();

  for (uint16_t i = 0; i < strlen(run_sequence); i++) {
    run_sequence[i] = '\0';
  }

  uint8_t i = 0;
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

    case SPEED_FAST:
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
    case SPEED_NORMAL:
    case SPEED_DIAGONALS:
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
  move_run_sequence(run_sequence, run_sequence_movements);
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
  int16_t *data = eeprom_get_data();
  for (uint16_t i = DATA_INDEX_MAZE; i < (DATA_INDEX_MAZE + MAZE_CELLS); i++) {
    maze[i - DATA_INDEX_MAZE] = data[i];
  }
}

void floodfill_maze_print(void) {
  initialize_directions_values();
  build_run_sequence();
  smooth_run_sequence(menu_run_get_speed());
  for (int16_t r = maze_get_cells() - maze_get_columns(); r >= 0; r = r - maze_get_columns()) {
    // Borde superior del laberinto
    if (r == maze_get_cells() - maze_get_columns()) {
      printf("·");
      for (uint16_t i = maze_get_cells() - maze_get_columns(); i < maze_get_cells(); i++) {
        if (maze[i] & NORTH_BIT) {
          printf("═══·");
        } else {
          printf("   ·");
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
        // printf(" V ");
        printf("%3d", floodfill[c]);
      } else {
        // printf("   ");
        printf("%3d", floodfill[c]);
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
        printf("═══·");
      } else {
        printf("   ·");
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
          printf("═══·");
        } else {
          printf("   ·");
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
        printf(" V ");
        // printf("%3d", floodfill[c]);
      } else {
        printf("   ");
        // printf("%3d", floodfill[c]);
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
  clear_info_leds();
  set_RGB_color(0, 0, 0);
  set_fan_speed(50);
  delay(500);

  initialize_directions_values();
  initialize_maze();
  set_initial_state();

  set_goals_from_maze();
  set_goal_as_target();

  struct walls walls = get_walls();
  update_walls(walls);
  update_floodfill();

  start_ms = get_clock_ticks();

  configure_kinematics(SPEED_EXPLORE);
  move(MOVE_START);
  update_position(FRONT);
}

void floodfill_start_run(void) {
  race_mode = true;
  clear_info_leds();
  set_RGB_color(0, 0, 0);
  set_fan_speed(50);
  delay(500);

  initialize_directions_values();
  configure_kinematics(menu_run_get_speed());
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