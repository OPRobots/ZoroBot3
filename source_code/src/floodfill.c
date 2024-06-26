#include <floodfill.h>

static bool use_left_hand = true;
static uint32_t start_ms = 0;
static uint32_t time_limit = 0;

static uint8_t maze[MAZE_CELLS];

static enum compass_direction initial_direction = NORTH;

static uint8_t current_position = 0;
static enum compass_direction current_direction = NORTH;

static void set_initial_state(void) {
  current_position = 0;
  current_direction = initial_direction;
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

static void build_wall(uint8_t position, uint8_t wall_bit) {
  maze[position] |= wall_bit;
  switch (wall_bit) {
    case EAST_BIT:
      if (position % MAZE_COLUMNS == MAZE_COLUMNS - 1) {
        break;
      }
      maze[position + EAST] |= WEST_BIT;
      break;
    case SOUTH_BIT:
      if (position / MAZE_COLUMNS == 0) {
        break;
      }
      maze[position + SOUTH] |= NORTH_BIT;
      break;
    case WEST_BIT:
      if (position % MAZE_COLUMNS == 0) {
        break;
      }
      maze[position + WEST] |= EAST_BIT;
      break;
    case NORTH_BIT:
      if (position / MAZE_COLUMNS == MAZE_ROWS - 1) {
        break;
      }
      maze[position + NORTH] |= SOUTH_BIT;
      break;
  }
}

static bool set_wall(uint8_t wall_bit) {
  if (!wall_exists(current_position, wall_bit)) {
    build_wall(current_position, wall_bit);
    return true;
  } else {
    return false;
  }
}

static void set_visited(void) {
  maze[current_position] |= VISITED_BIT;
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

void floodfill_use_left_hand(void) {
  use_left_hand = true;
}

void floodfill_use_right_hand(void) {
  use_left_hand = false;
}

void floodfill_set_time_limit(uint32_t ms) {
  time_limit = ms;
}

void floodfill_maze_print(void) {
  for (int16_t r = MAZE_CELLS - MAZE_COLUMNS; r >= 0; r = r - MAZE_COLUMNS) {
    // Borde superior del laberinto
    if (r == MAZE_CELLS - MAZE_COLUMNS) {
      printf("·");
      for (uint8_t i = 0; i < MAZE_COLUMNS; i++) {
        printf("═══·");
      }
    }
    printf("\n");

    // Paredes laterales del laberinto y VISITADO
    for (int16_t c = r; c < r + MAZE_COLUMNS; c++) {
      if (maze[c] & WEST_BIT || c % MAZE_COLUMNS == 0) {
        printf("║");
      } else {
        printf(" ");
      }
      if (maze[c] & VISITED_BIT) {
        printf(" V ");
      } else {
        printf("   ");
      }
    //   if ((c + 1 % MAZE_COLUMNS) == 0) {
    //     printf("|");
    //   }
    }
    printf("║\n");

    // Paredes inferiores del laberinto
    printf("·");
    for (int16_t c = r; c < r + MAZE_COLUMNS; c++) {
      if (maze[c] & SOUTH_BIT || c / MAZE_COLUMNS == 0) {
        printf("═══·");
      } else {
        printf("   ·");
      }
    }
  }
  printf("\n");
}

void floodfill_start(void) {
  for (uint16_t i = 0; i < MAZE_CELLS; i++) {
    maze[i] = 0;
  }
  set_initial_state();

  struct walls walls = get_walls();
  update_walls(walls);

  start_ms = get_clock_ticks();
  move(MOVE_START);

  update_position(FRONT);
}

void floodfill_loop(void) {
  if (time_limit > 0 && get_clock_ticks() - start_ms >= time_limit) {
    set_competicion_iniciada(false);
    return;
  }
  struct walls walls = get_walls();
  update_walls(walls);
  // TODO: update_floodfill
  set_RGB_color_while(0, 0, 255, 20);
  if ((use_left_hand && !walls.left) || (!use_left_hand && walls.right && !walls.left)) {
    move(MOVE_LEFT);
    update_position(LEFT);
  } else if ((!use_left_hand && !walls.right) || (use_left_hand && walls.left && !walls.right)) {
    move(MOVE_RIGHT);
    update_position(RIGHT);
  } else if (!walls.front) {
    move(MOVE_FRONT);
    update_position(FRONT);
  } else if (walls.front && walls.left && walls.right) {
    move(MOVE_180W);
    update_position(BACK);
  } else {
    set_target_linear_speed(0);
    set_ideal_angular_speed(0);
    warning_status_led(50);
  }
}