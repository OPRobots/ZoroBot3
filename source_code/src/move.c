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

static struct turn_params turns_explore[] = {
    [MOVE_LEFT] = {
        .start = 27,
        .end = 37,
        .linear_speed = 500,
        .angular_accel = 612.5,
        .max_angular_speed = 9.625,
        .t_accel = 16,
        .t_max = 150,
        .sign = -1,
    },
    [MOVE_RIGHT] = {
        .start = 27,
        .end = 37,
        .linear_speed = 500,
        .angular_accel = 612.5,
        .max_angular_speed = 9.625,
        .t_accel = 16,
        .t_max = 150,
        .sign = 1,
    },
    [MOVE_BACK] = {
        .start = 0,
        .end = 0,
        .linear_speed = 0,
        .angular_accel = 262.5,
        .max_angular_speed = 9.625,
        .t_accel = 37,
        .t_max = 291,
        .sign = 1,
    },
    [MOVE_BACK_WALL] = {
        .start = 0,
        .end = 0,
        .linear_speed = 0,
        .angular_accel = 612.5,
        .max_angular_speed = 9.625,
        .t_accel = 16,
        .t_max = 310,
        .sign = -1,
    },
    [MOVE_BACK_STOP] = {
        .start = 0,
        .end = 0,
        .linear_speed = 0,
        .angular_accel = 612.5,
        .max_angular_speed = 9.625,
        .t_accel = 16,
        .t_max = 310,
        .sign = -1,
    },
};

static struct turn_params turns_medium[] = {
    [MOVE_LEFT_90] = {
        .start = 0,
        .end = 0,
        .linear_speed = 650,
        .angular_accel = 612.5,
        .max_angular_speed = 7.49,
        .t_accel = 12,
        .t_max = 198,
        .sign = -1,
    },
    [MOVE_RIGHT_90] = {
        .start = 0,
        .end = 0,
        .linear_speed = 650,
        .angular_accel = 612.5,
        .max_angular_speed = 7.49,
        .t_accel = 12,
        .t_max = 198,
        .sign = 1,
    },
    [MOVE_LEFT_180] = {
        .start = 0,
        .end = 0,
        .linear_speed = 650,
        .angular_accel = 612.5,
        .max_angular_speed = 7.245,
        .t_accel = 12,
        .t_max = 423,
        .sign = -1,
    },
    [MOVE_RIGHT_180] = {
        .start = 0,
        .end = 0,
        .linear_speed = 650,
        .angular_accel = 612.5,
        .max_angular_speed = 7.245,
        .t_accel = 12,
        .t_max = 423,
        .sign = 1,
    },
    [MOVE_LEFT_TO_45] = {
        .start = -50,
        .end = 77.28,
        .linear_speed = 650,
        .angular_accel = 612.5,
        .max_angular_speed = 5.68,
        .t_accel = 9,
        .t_max = 129,
        .sign = -1,
    },
    [MOVE_RIGHT_TO_45] = {
        .start = -50,
        .end = 77.28,
        .linear_speed = 650,
        .angular_accel = 612.5,
        .max_angular_speed = 5.68,
        .t_accel = 9,
        .t_max = 129,
        .sign = 1,
    },
    [MOVE_LEFT_TO_135] = {
        .start = 0,
        .end = 74.56,
        .linear_speed = 650,
        .angular_accel = 612.5,
        .max_angular_speed = 8.94,
        .t_accel = 15,
        .t_max = 249,
        .sign = -1,
    },
    [MOVE_RIGHT_TO_135] = {
        .start = 0,
        .end = 74.56,
        .linear_speed = 650,
        .angular_accel = 612.5,
        .max_angular_speed = 8.94,
        .t_accel = 15,
        .t_max = 249,
        .sign = 1,
    },
    [MOVE_LEFT_45_TO_45] = {
        .start = 63.64,
        .end = 63.64,
        .linear_speed = 860,
        .angular_accel = 612.5,
        .max_angular_speed = 11.9,
        .t_accel = 19,
        .t_max = 113,
        .sign = -1,
    },
    [MOVE_RIGHT_45_TO_45] = {
        .start = 63.64,
        .end = 63.64,
        .linear_speed = 860,
        .angular_accel = 612.5,
        .max_angular_speed = 11.9,
        .t_accel = 19,
        .t_max = 113,
        .sign = 1,
    },
    [MOVE_LEFT_FROM_45] = {
        .start = 77.28,
        .end = -50,
        .linear_speed = 650,
        .angular_accel = 612.5,
        .max_angular_speed = 5.68,
        .t_accel = 9,
        .t_max = 129,
        .sign = -1,
    },
    [MOVE_RIGHT_FROM_45] = {
        .start = 77.28,
        .end = -50,
        .linear_speed = 650,
        .angular_accel = 612.5,
        .max_angular_speed = 5.68,
        .t_accel = 9,
        .t_max = 129,
        .sign = 1,
    },
    [MOVE_LEFT_FROM_45_180] = {
        .start = 74.56,
        .end = 0,
        .linear_speed = 650,
        .angular_accel = 612.5,
        .max_angular_speed = 8.94,
        .t_accel = 15,
        .t_max = 249,
        .sign = -1,
    },
    [MOVE_RIGHT_FROM_45_180] = {
        .start = 74.56,
        .end = 0,
        .linear_speed = 650,
        .angular_accel = 612.5,
        .max_angular_speed = 8.94,
        .t_accel = 15,
        .t_max = 249,
        .sign = 1,
    },
    [MOVE_BACK] = {
        .start = 0,
        .end = 0,
        .linear_speed = 0,
        .angular_accel = 262.5,
        .max_angular_speed = 9.625,
        .t_accel = 37,
        .t_max = 291,
        .sign = 1,
    },
    [MOVE_BACK_STOP] = {
        .start = 0,
        .end = 0,
        .linear_speed = 0,
        .angular_accel = 612.5,
        .max_angular_speed = 9.625,
        .t_accel = 16,
        .t_max = 310,
        .sign = -1,
    },
};

static struct turn_params turns_fast[] = {
    [MOVE_LEFT_90] = {
        .start = 0,
        .end = 0,
        .linear_speed = 860,
        .angular_accel = 612.5,
        .max_angular_speed = 9.625,
        .t_accel = 16,
        .t_max = 148,
        .sign = -1,
    },
    [MOVE_RIGHT_90] = {
        .start = 0,
        .end = 0,
        .linear_speed = 860,
        .angular_accel = 612.5,
        .max_angular_speed = 9.625,
        .t_accel = 16,
        .t_max = 148,
        .sign = 1,
    },
    [MOVE_LEFT_180] = {
        .start = 0,
        .end = 0,
        .linear_speed = 860,
        .angular_accel = 612.5,
        .max_angular_speed = 9.625,
        .t_accel = 16,
        .t_max = 311,
        .sign = -1,
    },
    [MOVE_RIGHT_180] = {
        .start = 0,
        .end = 0,
        .linear_speed = 860,
        .angular_accel = 612.5,
        .max_angular_speed = 9.625,
        .t_accel = 16,
        .t_max = 311,
        .sign = 1,
    },
    [MOVE_LEFT_TO_45] = {
        .start = -50,
        .end = 77.28,
        .linear_speed = 860,
        .angular_accel = 612.5,
        .max_angular_speed = 7.91,
        .t_accel = 13,
        .t_max = 87,
        .sign = -1,
    },
    [MOVE_RIGHT_TO_45] = {
        .start = -50,
        .end = 77.28,
        .linear_speed = 860,
        .angular_accel = 612.5,
        .max_angular_speed = 7.91,
        .t_accel = 13,
        .t_max = 87,
        .sign = 1,
    },
    [MOVE_LEFT_TO_135] = {
        .start = 0,
        .end = 74.56,
        .linear_speed = 860,
        .angular_accel = 612.5,
        .max_angular_speed = 12.0225,
        .t_accel = 7,
        .t_max = 178,
        .sign = -1,
    },
    [MOVE_RIGHT_TO_135] = {
        .start = 0,
        .end = 74.56,
        .linear_speed = 860,
        .angular_accel = 612.5,
        .max_angular_speed = 12.0225,
        .t_accel = 20,
        .t_max = 177,
        .sign = 1,
    },
    [MOVE_LEFT_45_TO_45] = {
        .start = 63.64,
        .end = 63.64,
        .linear_speed = 860,
        .angular_accel = 612.5,
        .max_angular_speed = 16.625,
        .t_accel = 7,
        .t_max = 178,
        .sign = -1,
    },
    [MOVE_RIGHT_45_TO_45] = {
        .start = 63.64,
        .end = 63.64,
        .linear_speed = 860,
        .angular_accel = 612.5,
        .max_angular_speed = 16.625,
        .t_accel = 27,
        .t_max = 68,
        .sign = 1,
    },
    [MOVE_LEFT_FROM_45] = {
        .start = 77.28,
        .end = -50,
        .linear_speed = 860,
        .angular_accel = 612.5,
        .max_angular_speed = 7.91,
        .t_accel = 13,
        .t_max = 87,
        .sign = -1,
    },
    [MOVE_RIGHT_FROM_45] = {
        .start = 77.28,
        .end = -50,
        .linear_speed = 860,
        .angular_accel = 612.5,
        .max_angular_speed = 7.91,
        .t_accel = 13,
        .t_max = 87,
        .sign = 1,
    },
    [MOVE_LEFT_FROM_45_180] = {
        .start = 74.56,
        .end = 0,
        .linear_speed = 860,
        .angular_accel = 612.5,
        .max_angular_speed = 12.0225,
        .t_accel = 7,
        .t_max = 178,
        .sign = -1,
    },
    [MOVE_RIGHT_FROM_45_180] = {
        .start = 74.56,
        .end = 0,
        .linear_speed = 860,
        .angular_accel = 612.5,
        .max_angular_speed = 12.0225,
        .t_accel = 7,
        .t_max = 178,
        .sign = 1,
    },
    [MOVE_BACK] = {
        .start = 0,
        .end = 0,
        .linear_speed = 0,
        .angular_accel = 262.5,
        .max_angular_speed = 9.625,
        .t_accel = 37,
        .t_max = 291,
        .sign = 1,
    },
    [MOVE_BACK_STOP] = {
        .start = 0,
        .end = 0,
        .linear_speed = 0,
        .angular_accel = 612.5,
        .max_angular_speed = 9.625,
        .t_accel = 16,
        .t_max = 310,
        .sign = -1,
    },
};

static struct turn_params turns_haki[] = {
    [MOVE_LEFT_90] = {
        .start = 0,
        .end = 0,
        .linear_speed = 1200,
        .angular_accel = 700,
        .max_angular_speed = 15.435,
        .t_accel = 22,
        .t_max = 80,
        .sign = -1,
    },
    [MOVE_RIGHT_90] = {
        .start = 0,
        .end = 0,
        .linear_speed = 1200,
        .angular_accel = 700,
        .max_angular_speed = 15.435,
        .t_accel = 22,
        .t_max = 80,
        .sign = 1,
    },
    [MOVE_LEFT_180] = {
        .start = 0,
        .end = 0,
        .linear_speed = 1200,
        .angular_accel = 700,
        .max_angular_speed = 13.405,
        .t_accel = 19,
        .t_max = 216,
        .sign = -1,
    },
    [MOVE_RIGHT_180] = {
        .start = 0,
        .end = 0,
        .linear_speed = 1200,
        .angular_accel = 700,
        .max_angular_speed = 13.405,
        .t_accel = 19,
        .t_max = 216,
        .sign = 1,
    },
    [MOVE_LEFT_TO_45] = {
        .start = -50,
        .end = 77.28,
        .linear_speed = 1200,
        .angular_accel = 700,
        .max_angular_speed = 12.25,
        .t_accel = 18,
        .t_max = 46,
        .sign = -1,
    },
    [MOVE_RIGHT_TO_45] = {
        .start = -50,
        .end = 77.28,
        .linear_speed = 1200,
        .angular_accel = 700,
        .max_angular_speed = 12.25,
        .t_accel = 18,
        .t_max = 46,
        .sign = 1,
    },
    [MOVE_LEFT_TO_135] = {
        .start = 0,
        .end = 74.56,
        .linear_speed = 1200,
        .angular_accel = 700,
        .max_angular_speed = 17.675,
        .t_accel = 25,
        .t_max = 109,
        .sign = -1,
    },
    [MOVE_RIGHT_TO_135] = {
        .start = 0,
        .end = 74.56,
        .linear_speed = 1200,
        .angular_accel = 700,
        .max_angular_speed = 17.675,
        .t_accel = 25,
        .t_max = 109,
        .sign = 1,
    },
    [MOVE_LEFT_45_TO_45] = {
        .start = 63.64,
        .end = 63.64,
        .linear_speed = 1200,
        .angular_accel = 875,
        .max_angular_speed = 31.5,
        .t_accel = 36,
        .t_max = 14,
        .sign = -1,
    },
    [MOVE_RIGHT_45_TO_45] = {
        .start = 63.64,
        .end = 63.64,
        .linear_speed = 1200,
        .angular_accel = 875,
        .max_angular_speed = 31.5,
        .t_accel = 36,
        .t_max = 14,
        .sign = 1,
    },
    [MOVE_LEFT_FROM_45] = {
        .start = 77.28,
        .end = -50,
        .linear_speed = 1200,
        .angular_accel = 700,
        .max_angular_speed = 12.25,
        .t_accel = 18,
        .t_max = 46,
        .sign = -1,
    },
    [MOVE_RIGHT_FROM_45] = {
        .start = 77.28,
        .end = -50,
        .linear_speed = 1200,
        .angular_accel = 700,
        .max_angular_speed = 12.25,
        .t_accel = 18,
        .t_max = 46,
        .sign = 1,
    },
    [MOVE_LEFT_FROM_45_180] = {
        .start = 74.56,
        .end = 0,
        .linear_speed = 1200,
        .angular_accel = 700,
        .max_angular_speed = 17.675,
        .t_accel = 25,
        .t_max = 109,
        .sign = -1,
    },
    [MOVE_RIGHT_FROM_45_180] = {
        .start = 74.56,
        .end = 0,
        .linear_speed = 1200,
        .angular_accel = 700,
        .max_angular_speed = 17.675,
        .t_accel = 25,
        .t_max = 109,
        .sign = 1,
    },
    [MOVE_BACK] = {
        .start = 0,
        .end = 0,
        .linear_speed = 0,
        .angular_accel = 262.5,
        .max_angular_speed = 9.625,
        .t_accel = 37,
        .t_max = 291,
        .sign = 1,
    },
    [MOVE_BACK_STOP] = {
        .start = 0,
        .end = 0,
        .linear_speed = 0,
        .angular_accel = 612.5,
        .max_angular_speed = 9.625,
        .t_accel = 16,
        .t_max = 310,
        .sign = -1,
    },
};

static struct kinematics kinematics_settings[] = {
    [SPEED_EXPLORE] = {
        .linear_speed = 500,
        .linear_accel = 3000,
        .fan_speed = 50,
        .turns = turns_explore,
    },
    [SPEED_NORMAL] = {
        .linear_speed = 1500,
        .linear_accel = 3000,
        .fan_speed = 75,
        .turns = turns_explore,
    },
    [SPEED_MEDIUM] = {
        .linear_speed = 2500,
        .linear_accel = 5000,
        .fan_speed = 85,
        .turns = turns_medium,
    },
    [SPEED_FAST] = {
        .linear_speed = 3000,
        .linear_accel = 7000,
        .fan_speed = 85,
        .turns = turns_fast,
    },
    [SPEED_HAKI] = {
        .linear_speed = 4000,
        .linear_accel = 15000,
        .fan_speed = 85,
        .turns = turns_haki,
    }};

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

  int32_t end_distance_offset = 0;
  int32_t start_distance_offset = 0;
  bool enable_end_distance_offset = true;
  bool enable_start_distance_offset = true;

  switch (movement) {
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
      enable_start_distance_offset = false;
      enable_end_distance_offset = false;
      set_side_sensors_close_correction(false);
      set_side_sensors_far_correction(false);
      break;
    default:
      set_side_sensors_close_correction(true);
      set_side_sensors_far_correction(true);
      break;
  }

  struct walls walls = get_walls();
  if (enable_end_distance_offset) {
    if (kinematics.turns[movement].sign > 0) {
      if (walls.left) {
        end_distance_offset = MIDDLE_MAZE_DISTANCE - get_sensor_distance(SENSOR_SIDE_LEFT_WALL_ID);
      }
    } else {
      if (walls.right) {
        end_distance_offset = MIDDLE_MAZE_DISTANCE - get_sensor_distance(SENSOR_SIDE_RIGHT_WALL_ID);
      }
    }
  }

  if (enable_start_distance_offset) {
    if (walls.front) {
      start_distance_offset = get_front_wall_distance() - (CELL_DIMENSION - (WALL_WIDTH / 2));
    }
  }

  if (kinematics.turns[movement].start > 0) {
    if (abs(start_distance_offset) > kinematics.turns[movement].start / 2) {
      start_distance_offset = start_distance_offset > 0 ? kinematics.turns[movement].start / 2 : -kinematics.turns[movement].start / 2;
    }
    move_straight(kinematics.turns[movement].start - current_cell_start_mm + start_distance_offset, kinematics.turns[movement].linear_speed, false, false);
  }

  disable_sensors_correction();
  // reset_control_errors(); //! Esto se había puesto por un problema en la acumulación de error según aumenta el número de giros realizados
  move_arc_turn(movement);

  set_front_sensors_correction(false);
  set_side_sensors_close_correction(true);
  set_side_sensors_far_correction(true);

  switch (movement) {
    case MOVE_LEFT_TO_45:
    case MOVE_RIGHT_TO_45:
    case MOVE_LEFT_TO_135:
    case MOVE_RIGHT_TO_135:
    case MOVE_LEFT_45_TO_45:
    case MOVE_RIGHT_45_TO_45:
      set_front_sensors_diagonal_correction(true);
      break;
    default:
      set_front_sensors_diagonal_correction(false);
      break;
  }

  if (kinematics.turns[movement].end > 0) {
    if (abs(end_distance_offset) > kinematics.turns[movement].end / 2) {
      end_distance_offset = end_distance_offset > 0 ? kinematics.turns[movement].end / 2 : -kinematics.turns[movement].end / 2;
    }
    move_straight(kinematics.turns[movement].end + end_distance_offset, kinematics.turns[movement].linear_speed, false, false);
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

  set_side_sensors_close_correction(false);
  switch (movement) {
    case MOVE_BACK_WALL:
    case MOVE_BACK_STOP:
      move_straight((CELL_DIMENSION - WALL_WIDTH) / 2 - ROBOT_BACK_LENGTH, -100, false, true);
      set_starting_position();
      break;
    case MOVE_BACK:
      move_straight(((CELL_DIMENSION - WALL_WIDTH) / 2 - ROBOT_BACK_LENGTH) / 2, -100, false, true);
      current_cell_start_mm = ((CELL_DIMENSION - WALL_WIDTH) / 2 - ROBOT_BACK_LENGTH) / 2;
      break;
    default:
      break;
  }
  reset_control_errors(); //? Aquí sí reseteamos al estar en una "posición inicial estática"

  if (movement != MOVE_BACK_STOP) {
    set_front_sensors_correction(false);
    set_front_sensors_diagonal_correction(false);
    set_side_sensors_close_correction(true);
    set_side_sensors_far_correction(true);
    move_straight(CELL_DIMENSION - SENSING_POINT_DISTANCE - current_cell_start_mm, kinematics.linear_speed, true, false);
    enter_next_cell();
  } else {
    set_side_sensors_close_correction(true);
    set_side_sensors_far_correction(true);
  }
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

void run_straight_hardcoded(int32_t distance, int32_t speed, int32_t final_speed, bool check_wall_loss, bool stop) {
  set_front_sensors_correction(false);
  set_front_sensors_diagonal_correction(false);
  set_side_sensors_close_correction(false);
  set_side_sensors_far_correction(false);

  int32_t current_distance = get_encoder_avg_micrometers();
  int32_t slow_distance = 0;
  int32_t stop_distance = 0;
  struct walls initial_walls = get_walls();

  // macroarray_store(
  //     1,
  //     0b0,
  //     8,
  //     (int16_t)get_encoder_avg_micrometers(),
  //     (int16_t)current_distance,
  //     (int16_t)distance,
  //     (int16_t)stop_distance,
  //     (int16_t)slow_distance,
  //     (int16_t)distance,
  //     (int16_t)speed,
  //     (int16_t)final_speed,
  //     (int16_t)check_wall_loss_correction,
  //     (int16_t)stop);

  set_ideal_angular_speed(0.0);
  set_target_linear_speed(speed);
  if (speed >= 0) {
    while (is_race_started() && get_encoder_avg_micrometers() <= current_distance + (distance - stop_distance - slow_distance) * MICROMETERS_PER_MILLIMETER) {
      // if (check_wall_loss && check_wall_loss_correction(initial_walls)) {
      //   current_distance = get_encoder_avg_micrometers();
      //   distance = 73;
      //   set_RGB_color_while(0, 255, 0, 150);
      // }
      // macroarray_store(
      //     1,
      //     0b0,
      //     8,
      //     (int16_t)get_encoder_avg_micrometers(),
      //     (int16_t)current_distance,
      //     (int16_t)distance,
      //     (int16_t)stop_distance,
      //     (int16_t)slow_distance,
      //     (int16_t)distance,
      //     (int16_t)speed,
      //     (int16_t)final_speed,
      //     (int16_t)check_wall_loss_correction,
      //     (int16_t)stop);

      if (stop) {
        stop_distance = calc_straight_to_speed_distance(get_ideal_linear_speed(), 0);
      }

      if (final_speed != speed && !stop) {
        slow_distance = calc_straight_to_speed_distance(get_ideal_linear_speed(), final_speed);
      }
    }
    // if (stop) {
    //   set_target_linear_speed(0);
    //   set_ideal_angular_speed(0.0);
    //   while (is_race_started() && get_ideal_linear_speed() != 0) {
    //   }
    // }

    set_target_linear_speed(final_speed);
    while (is_race_started() && get_encoder_avg_micrometers() <= current_distance + distance * MICROMETERS_PER_MILLIMETER) {
    }
  }
}

void run_diagonal(int32_t distance, int32_t speed, int32_t final_speed) {
  set_front_sensors_correction(false);
  set_front_sensors_diagonal_correction(true);
  set_side_sensors_close_correction(false);
  set_side_sensors_far_correction(false);

  int32_t current_distance = get_encoder_avg_micrometers();
  int32_t slow_distance = 0;
  // int32_t remaining_distance = 0;

  set_ideal_angular_speed(0.0);
  set_target_linear_speed(speed);
  while (is_race_started() && get_encoder_avg_micrometers() <= current_distance + (distance - slow_distance) * MICROMETERS_PER_MILLIMETER) {
    // remaining_distance = distance * MICROMETERS_PER_MILLIMETER - (get_encoder_avg_micrometers() - current_distance);
    // if (remaining_distance < CELL_DIAGONAL * MICROMETERS_PER_MILLIMETER * 0.5) {
    //   set_front_sensors_diagonal_correction(false);
    //   set_RGB_color_while(50, 0, 50, 150);
    // }

    if (final_speed != speed) {
      slow_distance = calc_straight_to_speed_distance(get_ideal_linear_speed(), final_speed);
    }
  }
  set_target_linear_speed(final_speed);
  while (is_race_started() && get_encoder_avg_micrometers() <= current_distance + distance * MICROMETERS_PER_MILLIMETER) {
    // remaining_distance = distance * MICROMETERS_PER_MILLIMETER - (get_encoder_avg_micrometers() - current_distance);
    // if (remaining_distance < CELL_DIAGONAL * 0.5) {
    //   set_front_sensors_diagonal_correction(false);
    //   set_RGB_color_while(50, 0, 50, 150);
    // }
  }
}

/**
 * @brief Movimiento de giro paramétrico
 *
 * @param turn_type
 */
void move_arc_turn(enum movement turn_type) {
  struct turn_params turn = kinematics.turns[turn_type];

  uint32_t ms_start = get_clock_ticks();
  uint32_t ms_current = ms_start;
  float angular_speed = 0;
  while (true) {
    set_RGB_color(255, 0, 0);
    ms_current = get_clock_ticks();
    if (ms_current - ms_start <= turn.t_accel) {
      angular_speed = turn.angular_accel * (ms_current - ms_start) / 1000;
    } else if (ms_current - ms_start <= (turn.t_accel + turn.t_max)) {
      angular_speed = turn.max_angular_speed;
    } else if (ms_current - ms_start <= (uint32_t)(turn.t_accel + turn.t_max + turn.t_accel)) {
      angular_speed = turn.max_angular_speed - (turn.angular_accel * (ms_current - ms_start - turn.t_accel - turn.t_max) / 1000);
    } else {
      break;
    }
    set_ideal_angular_speed(angular_speed * turn.sign);
  }
  set_RGB_color(0, 0, 0);
  set_ideal_angular_speed(0);
}

void move_inplace_turn(enum movement movement) {
  struct turn_params turn = kinematics.turns[movement];
  set_target_linear_speed(turn.linear_speed);

  uint32_t ms_start = get_clock_ticks();
  uint32_t ms_current = ms_start;
  float angular_speed = 0;
  int8_t sign = turn.sign == 0 ? ((int8_t)(rand() % 2) * 2 - 1) : turn.sign;
  while (true) {
    ms_current = get_clock_ticks();
    if (ms_current - ms_start <= turn.t_accel) {
      angular_speed = turn.angular_accel * (ms_current - ms_start) / 1000;
    } else if (ms_current - ms_start <= (turn.t_accel + turn.t_max)) {
      angular_speed = turn.max_angular_speed;
    } else if (ms_current - ms_start <= (uint32_t)(turn.t_accel + turn.t_max + turn.t_accel)) {
      angular_speed = turn.max_angular_speed - (turn.angular_accel * (ms_current - ms_start - turn.t_accel - turn.t_max) / 1000);
    } else {
      set_ideal_angular_speed(0);
      break;
    }
    set_ideal_angular_speed(angular_speed * sign);
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
  set_check_motors_saturated_enabled(true);
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
    case MOVE_BACK_STOP:
      set_check_motors_saturated_enabled(false);
      move_back(movement);
      set_check_motors_saturated_enabled(true);
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
          if (kinematics.turns[sequence_movements[i]].start < 0) {
            end_offset = kinematics.turns[sequence_movements[i]].start;
          }
          if (running_diagonal) {
            run_diagonal(distance, kinematics.linear_speed, kinematics.turns[sequence_movements[i]].linear_speed);
          } else {
            run_straight(distance, end_offset, straight_cells, straight_has_begin, kinematics.linear_speed, kinematics.turns[sequence_movements[i]].linear_speed);
          }
          if (kinematics.turns[sequence_movements[i]].end < 0) {
            distance = kinematics.turns[sequence_movements[i]].end;
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