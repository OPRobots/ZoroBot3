#include "move.h"

static char *movement_string[] = {
    "MOVE_NONE",
    "MOVE_HOME",
    "MOVE_START",
    "MOVE_END",
    "MOVE_FRONT",
    "MOVE_LEFT",
    "MOVE_RIGHT",
    "MOVE_LEFT_90",
    "MOVE_RIGHT_90",
    "MOVE_LEFT_180",
    "MOVE_RIGHT_180",
    "MOVE_DIAGONAL",
    "MOVE_LEFT_TO_45",
    "MOVE_RIGHT_TO_45",
    "MOVE_LEFT_TO_135",
    "MOVE_RIGHT_TO_135",
    "MOVE_LEFT_45_TO_45",
    "MOVE_RIGHT_45_TO_45",
    "MOVE_LEFT_FROM_45",
    "MOVE_RIGHT_FROM_45",
    "MOVE_LEFT_FROM_45_180",
    "MOVE_RIGHT_FROM_45_180",
    "MOVE_BACK",
    "MOVE_BACK_WALL",
};

static struct turn_params turns_params[] = {
    [MOVE_LEFT] = {
        .start = 27,
        .end = 37,
        .radius = 55,
        .transition = 5,
        .angle = 90,
        .sign = -1,
    },
    [MOVE_RIGHT] = {
        .start = 27,
        .end = 37,
        .radius = 55,
        .transition = 5,
        .angle = 90,
        .sign = 1,
    },
    [MOVE_BACK] = {
        .start = 0,
        .end = 0,
        .radius = ROBOT_WIDTH / 2,
        .transition = 5,
        .angle = 180,
        .sign = 1,
    },
    [MOVE_BACK_WALL] = {
        .start = 0,
        .end = 0,
        .radius = ROBOT_WIDTH / 2,
        .transition = 5,
        .angle = 180,
        .sign = -1,
    },
    [MOVE_LEFT_90] = {
        .start = 0,
        .end = 0,
        .radius = 90,
        .transition = 5,
        .angle = 90,
        .sign = -1,
    },
    [MOVE_RIGHT_90] = {
        .start = 0,
        .end = 0,
        .radius = 90,
        .transition = 5,
        .angle = 90,
        .sign = 1,
    },
    [MOVE_LEFT_180] = {
        .start = 0,
        .end = 0,
        .radius = 90,
        .transition = 5,
        .angle = 180,
        .sign = -1,
    },
    [MOVE_RIGHT_180] = {
        .start = 0,
        .end = 0,
        .radius = 90,
        .transition = 5,
        .angle = 180,
        .sign = 1,
    },

    [MOVE_LEFT_TO_45] = {
        .start = -50,
        .end = 77.28,
        .radius = 120.71,
        .transition = 5,
        .angle = 45,
        .sign = -1,
    },
    [MOVE_RIGHT_TO_45] = {
        .start = -50,
        .end = 77.28,
        .radius = 120.71,
        .transition = 5,
        .angle = 45,
        .sign = 1,
    },
    [MOVE_LEFT_TO_135] = {
        .start = 0,
        .end = 74.56,
        .radius = 74.56,
        .transition = 5,
        .angle = 135,
        .sign = -1,
    },
    [MOVE_RIGHT_TO_135] = {
        .start = 0,
        .end = 74.56,
        .radius = 74.56,
        .transition = 5,
        .angle = 135,
        .sign = 1,
    },
    [MOVE_LEFT_45_TO_45] = {
        .start = 63.64,
        .end = 63.64,
        .radius = 63.64,
        .transition = 5,
        .angle = 90,
        .sign = -1,
    },
    [MOVE_RIGHT_45_TO_45] = {
        .start = 63.64,
        .end = 63.64,
        .radius = 63.64,
        .transition = 5,
        .angle = 90,
        .sign = 1,
    },
    [MOVE_LEFT_FROM_45] = {
        .start = 77.28,
        .end = -50,
        .radius = 120.71,
        .transition = 5,
        .angle = 45,
        .sign = -1,
    },
    [MOVE_RIGHT_FROM_45] = {
        .start = 77.28,
        .end = -50,
        .radius = 120.71,
        .transition = 5,
        .angle = 45,
        .sign = 1,
    },
    [MOVE_LEFT_FROM_45_180] = {
        .start = 74.56,
        .end = 0,
        .radius = 74.56,
        .transition = 5,
        .angle = 135,
        .sign = -1,
    },
    [MOVE_RIGHT_FROM_45_180] = {
        .start = 74.56,
        .end = 0,
        .radius = 74.56,
        .transition = 5,
        .angle = 135,
        .sign = 1,
    },
};

static struct kinematics kinematics_settings[] = {
    [SPEED_EXPLORE] = {
        .linear_speed = 500,
        .linear_accel = 3000,
        .turns_linear_speed = 500,
        .turns_params = turns_params,
    },
    [SPEED_NORMAL] = {
        .linear_speed = 900,
        .linear_accel = 3000,
        .turns_linear_speed = 500,
        .turns_params = turns_params,
    },
    [SPEED_FAST] = {
        .linear_speed = 2500,
        .linear_accel = 3000,
        .turns_linear_speed = 860,
        .turns_params = turns_params,
    },
    [SPEED_DIAGONALS] = {
        .linear_speed = 3500,
        .linear_accel = 3000,
        .turns_linear_speed = 1200,
        .turns_params = turns_params,
    },
};

static struct kinematics kinematics;

static int32_t current_cell_start_mm = 0;
static bool current_cell_wall_lost = false;
static int32_t current_cell_absolute_start_mm = 0;

static int32_t calc_straight_to_speed_distance(int32_t from_speed, int32_t to_speed) {
  return abs((to_speed * to_speed - from_speed * from_speed) / (2 * kinematics.linear_accel));
}

static void enter_next_cell(void) {
  current_cell_start_mm = -SENSING_POINT_DISTANCE;
  current_cell_absolute_start_mm = get_encoder_avg_millimeters();
  current_cell_wall_lost = false;
  set_RGB_color_while(0, 0, 255, 150);
  // if (front_wall_detection()) {
  //   int16_t distance = get_front_wall_distance() - ((CELL_DIMENSION - WALL_WIDTH / 2) + SENSING_POINT_DISTANCE);
  //   if(abs(distance)>5){
  //     current_cell_start_mm -= distance;
  //   }
  // }
}

static bool check_wall_loss_correction(struct walls initial_walls) {
  if (current_cell_wall_lost) {
    return false;
  }
  struct walls current_walls = get_walls();
  bool wall_lost = false;
  if (initial_walls.left && !current_walls.left) {
    wall_lost = true;
  } else if (initial_walls.right && !current_walls.right) {
    wall_lost = true;
  } /* else {
    if (last_check_walls_loss.left) {
      count_check_walls_left++;
      if (count_check_walls_left > 10 && !current_walls.left) {
        wall_lost = true;
        count_check_walls_left = 0;
      }
    } else {
      count_check_walls_left = 0;
    }
    if (last_check_walls_loss.right) {
      count_check_walls_right++;
      if (count_check_walls_right > 10 && !current_walls.right) {
        wall_lost = true;
        count_check_walls_right = 0;
      }
    } else {
      count_check_walls_right = 0;
    }
  }
  last_check_walls_loss = current_walls; */
  if (wall_lost) {
    current_cell_wall_lost = true;
  }
  return wall_lost;
}

static void move_home(void) {
  set_front_sensors_correction(true);
  set_front_sensors_diagonal_correction(false);
  set_side_sensors_close_correction(true);
  set_side_sensors_far_correction(false);

  move_straight_until_front_distance(CELL_DIMENSION / 2, 300, true);

  disable_sensors_correction();
  move_inplace_turn(MOVE_BACK);
  reset_control_errors();
  move_straight((CELL_DIMENSION - WALL_WIDTH) / 2 - ROBOT_BACK_LENGTH, -100, false, true);
  set_starting_position();
}

/**
 * @brief Movimiento frontal relativo a la celda actual; avanza a la siguiente celda
 *
 */
static void move_front(void) {
  set_front_sensors_correction(false);
  set_front_sensors_diagonal_correction(false);
  struct walls initial_walls = get_walls();
  if (initial_walls.left || initial_walls.right) {
    set_side_sensors_close_correction(true);
    set_side_sensors_far_correction(true);
  } else {
    set_side_sensors_close_correction(false);
    set_side_sensors_far_correction(false);
  }
  move_straight(CELL_DIMENSION - SENSING_POINT_DISTANCE - current_cell_start_mm, kinematics.linear_speed, true, false);
  set_RGB_color_while(255, 0, 0, 150);
  enter_next_cell();
}

static void move_side(enum movement movement) {
  set_front_sensors_correction(false);
  set_front_sensors_diagonal_correction(false);
  set_side_sensors_close_correction(true);
  set_side_sensors_far_correction(true);

  int32_t end_distance_offset = 0;
  struct walls walls = get_walls();
  if (kinematics.turns_params[movement].sign > 0) {
    if (walls.left) {
      end_distance_offset = MIDDLE_MAZE_DISTANCE - get_sensor_distance(SENSOR_SIDE_LEFT_WALL_ID);
    }
  } else {
    if (walls.right) {
      end_distance_offset = MIDDLE_MAZE_DISTANCE - get_sensor_distance(SENSOR_SIDE_RIGHT_WALL_ID);
    }
  }

  int32_t start_distance_offset = 0;
  if (walls.front) {
    start_distance_offset = get_front_wall_distance() - (CELL_DIMENSION - (WALL_WIDTH / 2));
  }

  if (kinematics.turns_params[movement].start > 0) {
    if (abs(start_distance_offset) > kinematics.turns_params[movement].start / 2) {
      start_distance_offset = start_distance_offset > 0 ? kinematics.turns_params[movement].start / 2 : -kinematics.turns_params[movement].start / 2;
    }
    move_straight(kinematics.turns_params[movement].start - current_cell_start_mm + start_distance_offset, kinematics.turns_linear_speed, false, false);
  }

  disable_sensors_correction();
  // reset_control_errors(); //! Esto se había puesto por un problema en la acumulación de error según aumenta el número de giros realizados
  move_arc_turn(movement);

  set_front_sensors_correction(false);
  set_front_sensors_diagonal_correction(false);
  set_side_sensors_close_correction(true);
  set_side_sensors_far_correction(true);

  if (kinematics.turns_params[movement].end > 0) {
    if (abs(end_distance_offset) > kinematics.turns_params[movement].end / 2) {
      end_distance_offset = end_distance_offset > 0 ? kinematics.turns_params[movement].end / 2 : -kinematics.turns_params[movement].end / 2;
    }
    move_straight(kinematics.turns_params[movement].end + end_distance_offset, kinematics.turns_linear_speed, false, false);
  }
  enter_next_cell();
}

static void move_back(enum movement movement) {
  set_front_sensors_correction(true);
  set_front_sensors_diagonal_correction(false);
  set_side_sensors_close_correction(true);
  set_side_sensors_far_correction(true);

  if (movement == MOVE_BACK_WALL) {
    move_straight_until_front_distance(CELL_DIMENSION / 2, 300, true);
  } else {
    move_straight((CELL_DIMENSION / 2) - calc_straight_to_speed_distance(300, 0) - current_cell_start_mm, 300, false, false);
  }

  disable_sensors_correction();
  move_inplace_turn(movement);

  // TODO: intentar corregir con las paredes marcha atras??

  if (movement == MOVE_BACK_WALL) {
    move_straight((CELL_DIMENSION - WALL_WIDTH) / 2 - ROBOT_BACK_LENGTH, -100, false, true);
    set_starting_position();
  } else {
    move_straight(((CELL_DIMENSION - WALL_WIDTH) / 2 - ROBOT_BACK_LENGTH) / 2, -100, false, true);
    current_cell_start_mm = ((CELL_DIMENSION - WALL_WIDTH) / 2 - ROBOT_BACK_LENGTH) / 2;
  }
  reset_control_errors(); //? Aquí sí reseteamos al estar en una "posición inicial estática"

  set_front_sensors_correction(false);
  set_front_sensors_diagonal_correction(false);
  set_side_sensors_close_correction(true);
  set_side_sensors_far_correction(true);
  move_straight(CELL_DIMENSION - SENSING_POINT_DISTANCE - current_cell_start_mm, kinematics.linear_speed, true, false);
  enter_next_cell();
}

char *get_movement_string(enum movement movement) {
  return movement_string[movement];
}

void configure_kinematics(enum speed_strategy speed) {
  kinematics = kinematics_settings[speed];
}

struct kinematics get_kinematics(void) {
  return kinematics;
}

void set_starting_position(void) {
  current_cell_start_mm = ROBOT_BACK_LENGTH + WALL_WIDTH / 2;
}

int32_t get_current_cell_travelled_distance(void) {
  return get_encoder_avg_millimeters() - current_cell_absolute_start_mm;
}

/**
 * @brief Movimiento frontal paramétrico
 *
 * @param distance
 * @param speed
 * @param stop
 */
void move_straight(int32_t distance, int32_t speed, bool check_wall_loss, bool stop) {
  int32_t current_distance = get_encoder_avg_micrometers();
  int32_t stop_distance = 0;
  struct walls initial_walls = get_walls();
  set_ideal_angular_speed(0.0);
  set_target_linear_speed(speed);
  if (speed >= 0) {
    while (is_race_started() && get_encoder_avg_micrometers() <= current_distance + (distance - stop_distance) * MICROMETERS_PER_MILLIMETER) {
      if (check_wall_loss && check_wall_loss_correction(initial_walls) /* && distance_left < 90 */) { // TODO: resetear distancia solo cuando el error es pequeño.
        // int32_t left_distance = (distance * MICROMETERS_PER_MILLIMETER) - (get_encoder_avg_micrometers() - current_distance);
        // while (true) {
        //   set_race_started(false);
        //   set_target_linear_speed(0);
        //   printf("%ld\n", left_distance);
        // }
        current_distance = get_encoder_avg_micrometers();
        distance = 73;
        set_RGB_color_while(0, 255, 0, 150);
      }

      if (stop) {
        stop_distance = calc_straight_to_speed_distance(get_ideal_linear_speed(), 0);
      }
    }
  } else {
    while (is_race_started() && get_encoder_avg_micrometers() >= current_distance - (distance + stop_distance) * MICROMETERS_PER_MILLIMETER) {
      if (stop) {
        stop_distance = calc_straight_to_speed_distance(get_ideal_linear_speed(), 0);
      }
    }
  }
  if (stop) {
    set_target_linear_speed(0);
    set_ideal_angular_speed(0.0);
    while (is_race_started() && get_ideal_linear_speed() != 0) {
    }
  }
}

/**
 * @brief Movimiento frontal paramétrico hasta una distancia determinada de la pared frontal
 *
 * @param distance
 * @param speed
 * @param stop
 */
void move_straight_until_front_distance(uint32_t distance, int32_t speed, bool stop) {
  int32_t stop_distance = 0;
  set_ideal_angular_speed(0.0);
  set_target_linear_speed(speed);
  while (is_race_started() && (get_sensor_distance(SENSOR_FRONT_LEFT_WALL_ID) + get_sensor_distance(SENSOR_FRONT_RIGHT_WALL_ID)) / 2 > (distance + stop_distance)) {
    if (stop) {
      stop_distance = calc_straight_to_speed_distance(get_ideal_linear_speed(), 0);
    }
    set_RGB_color(255, 0, 0);
  }
  set_RGB_color(0, 0, 0);
  if (stop) {
    set_target_linear_speed(0);
    while (is_race_started() && get_ideal_linear_speed() != 0) {
    }
  }
}

void run_straight(int32_t distance, int32_t end_offset, uint16_t cells, bool has_begin, int32_t speed, int32_t final_speed) {
  set_front_sensors_correction(false);
  set_front_sensors_diagonal_correction(false);
  set_side_sensors_close_correction(true);
  set_side_sensors_far_correction(true);

  int32_t current_distance = get_encoder_avg_micrometers();
  int32_t slow_distance = 0;

  uint16_t current_cell = 0;
  int32_t current_cell_distance_left;
  if (has_begin) {
    current_cell_distance_left = CELL_DIMENSION - (ROBOT_BACK_LENGTH + WALL_WIDTH / 2);
  } else {
    current_cell_distance_left = CELL_DIMENSION;
  }
  bool wall_lost = false;

  struct walls cell_walls = get_walls();
  struct walls current_walls;
  set_ideal_angular_speed(0.0);
  set_target_linear_speed(speed);
  distance += end_offset;
  while (is_race_started() && get_encoder_avg_micrometers() <= current_distance + (distance - slow_distance) * MICROMETERS_PER_MILLIMETER) {
    current_walls = get_walls();

    if (!wall_lost && ((cell_walls.left && !current_walls.left) || (cell_walls.right && !current_walls.right))) {
      current_distance = get_encoder_avg_micrometers();
      distance = 73 + CELL_DIMENSION * (cells - current_cell - 1) + end_offset;
      current_cell_distance_left = 73;
      set_RGB_color_while(0, 255, 0, 150);
      wall_lost = true;
    }
    if (get_encoder_avg_micrometers() - current_distance >= (current_cell_distance_left * MICROMETERS_PER_MILLIMETER) && cells > current_cell + 1) {
      current_distance = get_encoder_avg_micrometers();
      distance = CELL_DIMENSION * (cells - current_cell - 1) + end_offset;
      current_cell++;
      current_cell_distance_left = CELL_DIMENSION;
      cell_walls = current_walls;
      set_RGB_color_while(0, 0, 255, 150);
      wall_lost = false;
    }

    if (final_speed != speed) {
      slow_distance = calc_straight_to_speed_distance(get_ideal_linear_speed(), final_speed);
    }
  }
  set_target_linear_speed(final_speed);
  while (is_race_started() && get_encoder_avg_micrometers() <= current_distance + distance * MICROMETERS_PER_MILLIMETER) {
  }
}

void run_diagonal(int32_t distance, int32_t speed, int32_t final_speed) {
  set_front_sensors_correction(false);
  set_front_sensors_diagonal_correction(true);
  set_side_sensors_close_correction(false);
  set_side_sensors_far_correction(false);

  int32_t current_distance = get_encoder_avg_micrometers();
  int32_t slow_distance = 0;
  int32_t remaining_distance = 0;

  set_ideal_angular_speed(0.0);
  set_target_linear_speed(speed);
  while (is_race_started() && get_encoder_avg_micrometers() <= current_distance + (distance - slow_distance) * MICROMETERS_PER_MILLIMETER) {
    remaining_distance = distance * MICROMETERS_PER_MILLIMETER - (get_encoder_avg_micrometers() - current_distance);
    if (remaining_distance < CELL_DIAGONAL * MICROMETERS_PER_MILLIMETER * 1) {
      set_front_sensors_diagonal_correction(false);
      set_RGB_color_while(50, 0, 50, 150);
    }

    if (final_speed != speed) {
      slow_distance = calc_straight_to_speed_distance(get_ideal_linear_speed(), final_speed);
    }
  }
  set_target_linear_speed(final_speed);
  while (is_race_started() && get_encoder_avg_micrometers() <= current_distance + distance * MICROMETERS_PER_MILLIMETER) {
    remaining_distance = distance * MICROMETERS_PER_MILLIMETER - (get_encoder_avg_micrometers() - current_distance);
    if (remaining_distance < CELL_DIAGONAL * 1) {
      set_front_sensors_diagonal_correction(false);
      set_RGB_color_while(50, 0, 50, 150);
    }
  }
}

/**
 * @brief Movimiento de giro paramétrico
 *
 * @param turn_type
 */
void move_arc_turn(enum movement turn_type) {
  struct turn_params turn = kinematics.turns_params[turn_type];

  int32_t start;
  int32_t current;
  float travelled;
  float angular_speed;
  float max_angular_speed;
  float factor;
  float arc;

  start = get_encoder_avg_micrometers();
  arc = 2 * PI * turn.radius * (turn.angle / 360.0);
  max_angular_speed = turn.sign * (kinematics.turns_linear_speed / turn.radius);
  while (is_race_started()) {
    set_RGB_color(255, 0, 0);
    current = get_encoder_avg_micrometers();
    travelled = (float)((current - start) / MICROMETERS_PER_MILLIMETER);
    if (travelled >= arc) {
      break;
    }
    angular_speed = max_angular_speed;
    if (travelled < turn.transition) {
      factor = travelled / turn.transition;
      angular_speed *= sin(factor * PI / 2);
    } else if (travelled >= arc - turn.transition) {
      factor = (travelled - (arc - (2 * turn.transition))) / turn.transition;
      angular_speed *= sin(factor * PI / 2);
    }
    set_ideal_angular_speed(angular_speed);
  }
  set_ideal_angular_speed(0);
  set_RGB_color(0, 0, 0);
  set_ideal_angular_speed(0);
}

void move_inplace_turn(enum movement movement) {
  struct turn_params turn = kinematics.turns_params[movement];
  set_target_linear_speed(0);

  int32_t start;
  int32_t current;
  float time;
  float angular_speed;
  float max_angular_speed;
  float angular_accel;
  float factor;
  float arc;
  float transition;
  float duration;
  float transition_angle;

  angular_accel = 612.5;
  max_angular_speed = (300 / turn.radius);
  duration = max_angular_speed / angular_accel * PI;
  transition_angle = duration * max_angular_speed / PI;
  arc = (PI - 2 * transition_angle) / max_angular_speed;
  transition = duration / 2;
  max_angular_speed *= turn.sign;

  set_target_linear_speed(0);

  start = get_clock_ticks();
  while (is_race_started()) {
    current = get_clock_ticks();
    time = (float)(current - start) / SYSTICK_FREQUENCY_HZ;
    if (time >= 2 * transition + arc) {
      break;
    }
    angular_speed = max_angular_speed;
    if (time < transition) {
      factor = time / transition;
      angular_speed *= sin(factor * PI / 2);
    } else if (time >= transition + arc) {
      factor = (time - arc) / transition;
      angular_speed *= sin(factor * PI / 2);
    }
    set_ideal_angular_speed(angular_speed);
  }
  set_ideal_angular_speed(0);
}

/**
 * @brief Movimiento de giro en el sitio paramétrico
 *
 * @param angle
 * @param rads
 */
void move_inplace_angle(float angle, float rads) {
  set_gyro_z_degrees(0);
  float current_angle = get_gyro_z_degrees();
  float target_angle = current_angle + angle;
  if (target_angle > 360.0) {
    target_angle = 360.0 - target_angle;
  } else if (target_angle < -360) {
    target_angle = 360.0 + target_angle;
  }
  set_target_linear_speed(0.0);
  if (angle >= 0) {
    set_ideal_angular_speed(rads);
    while (is_race_started() && get_gyro_z_degrees() <= target_angle) {
    }
  } else {
    set_ideal_angular_speed(-rads);
    while (is_race_started() && get_gyro_z_degrees() >= target_angle) {
    }
  }
  set_ideal_angular_speed(0.0);
}

void move(enum movement movement) {
  switch (movement) {
    case MOVE_HOME:
      move_home();
      break;
    case MOVE_START:
      set_starting_position();
      move_front();
      break;
    case MOVE_FRONT:
      move_front();
      break;
    case MOVE_LEFT:
    case MOVE_RIGHT:
    case MOVE_LEFT_90:
    case MOVE_RIGHT_90:
    case MOVE_LEFT_180:
    case MOVE_RIGHT_180:
    case MOVE_LEFT_TO_45:
    case MOVE_RIGHT_TO_45:
    case MOVE_LEFT_TO_135:
    case MOVE_RIGHT_TO_135:
    case MOVE_LEFT_45_TO_45:
    case MOVE_RIGHT_45_TO_45:
    case MOVE_LEFT_FROM_45:
    case MOVE_RIGHT_FROM_45:
    case MOVE_LEFT_FROM_45_180:
    case MOVE_RIGHT_FROM_45_180:
      move_side(movement);
      break;
    case MOVE_BACK:
    case MOVE_BACK_WALL:
      move_back(movement);
      break;
    default:
      break;
  }
}

void move_run_sequence(char *sequence, enum movement *sequence_movements) {
  float distance = 0;
  float end_offset = 0;
  bool running_diagonal = false;
  bool straight_has_begin = true;
  uint16_t straight_cells = 0;

  for (uint16_t i = 0; i < (MAZE_CELLS + 3); i++) {
    switch (sequence_movements[i]) {
      case MOVE_START:
        distance += CELL_DIMENSION - (ROBOT_BACK_LENGTH + WALL_WIDTH / 2);
        straight_cells++;
        straight_has_begin = true;
        break;
      case MOVE_FRONT:
        distance += CELL_DIMENSION;
        straight_cells++;
        break;
      case MOVE_DIAGONAL:
        running_diagonal = true;
        distance += CELL_DIAGONAL;
        straight_cells++;
        break;
      case MOVE_HOME:
        if (distance > 0) {
          if (running_diagonal) {
            run_diagonal(distance, kinematics.linear_speed, 500);
          } else {
            run_straight(distance, 0, straight_cells, straight_has_begin, kinematics.linear_speed, 500);
          }
          distance = 0;
          end_offset = 0;
          straight_cells = 0;
          straight_has_begin = false;
          running_diagonal = false;
        }
        move(MOVE_HOME);
        break;
      case MOVE_LEFT:
      case MOVE_RIGHT:
      case MOVE_LEFT_90:
      case MOVE_RIGHT_90:
      case MOVE_LEFT_180:
      case MOVE_RIGHT_180:
      case MOVE_LEFT_TO_45:
      case MOVE_RIGHT_TO_45:
      case MOVE_LEFT_TO_135:
      case MOVE_RIGHT_TO_135:
      case MOVE_LEFT_45_TO_45:
      case MOVE_RIGHT_45_TO_45:
      case MOVE_LEFT_FROM_45:
      case MOVE_RIGHT_FROM_45:
      case MOVE_LEFT_FROM_45_180:
      case MOVE_RIGHT_FROM_45_180:
        if (distance > 0) {
          if (kinematics.turns_params[sequence_movements[i]].start < 0) {
            end_offset = kinematics.turns_params[sequence_movements[i]].start;
          }
          if (running_diagonal) {
            run_diagonal(distance, kinematics.linear_speed, kinematics.turns_linear_speed);
          } else {
            run_straight(distance, end_offset, straight_cells, straight_has_begin, kinematics.linear_speed, kinematics.turns_linear_speed);
          }
          if (kinematics.turns_params[sequence_movements[i]].end < 0) {
            distance = kinematics.turns_params[sequence_movements[i]].end;
          } else {
            distance = 0;
          }
          end_offset = 0;
          straight_cells = 0;
          straight_has_begin = false;
          running_diagonal = false;
        }
        move(sequence_movements[i]);
        break;
      default:
        i = (MAZE_CELLS + 3);
        break;
    }
  }
  delay(1000);
  configure_kinematics(SPEED_EXPLORE);
  for (int16_t i = strlen(sequence) - 1; i >= 0; i--) {
    switch (sequence[i]) {
      case 'S':
        move(MOVE_START);
        break;
      case 'F':
        move(MOVE_FRONT);
        break;
      case 'R':
        move(MOVE_LEFT);
        break;
      case 'L':
        move(MOVE_RIGHT);
        break;
      case 'B':
        move(MOVE_HOME);
        break;
    }
  }
}