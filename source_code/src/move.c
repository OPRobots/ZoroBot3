#include "move.h"

struct turn_params turns[] = {
    [MOVE_LEFT] = {32, 35, 500, 612.5, 9.625, 16, 148, -1},
    [MOVE_RIGHT] = {32, 35, 500, 612.5, 9.625, 16, 148, 1},
    [MOVE_180] = {0, 0, 0, 262.5, 9.625, 37, 291, 1},
    [MOVE_180W] = {0, 0, 0, 612.5, 9.625, 16, 310, -1},
};

static int32_t current_cell_start_mm = 0;
static int32_t current_cell_absolute_start_mm = 0;

static int32_t calc_straight_stop_distance(int32_t speed) {
  return (speed * speed) / (2 * BASE_LINEAR_ACCEL);
}

static void enter_next_cell(void) {
  current_cell_start_mm = -SENSING_POINT_DISTANCE;
  current_cell_absolute_start_mm = get_encoder_avg_millimeters();
  // if (front_wall_detection()) {
  //   int16_t distance = get_front_wall_distance() - ((CELL_DIMENSION - WALL_WIDTH / 2) + SENSING_POINT_DISTANCE);
  //   if(abs(distance)>5){
  //     current_cell_start_mm -= distance;
  //   }
  // }
}

/**
 * @brief Movimiento frontal relativo a la celda actual; avanza a la siguiente celda
 *
 */
static void move_front(void) {
  set_front_sensors_correction(false);
  set_side_sensors_close_correction(true);
  set_side_sensors_far_correction(false);
  move_straight(CELL_DIMENSION - SENSING_POINT_DISTANCE - current_cell_start_mm, 500, false);
  enter_next_cell();
}

static void move_side(enum movement movement) {
  set_front_sensors_correction(false);
  set_side_sensors_close_correction(true);
  set_side_sensors_far_correction(false);
  move_straight(turns[movement].start - current_cell_start_mm, 500, false);

  disable_sensors_correction();
  move_arc_turn(movement);

  set_front_sensors_correction(false);
  set_side_sensors_close_correction(true);
  set_side_sensors_far_correction(false);
  move_straight(turns[movement].end - SENSING_POINT_DISTANCE, 500, false);
  enter_next_cell();
}

static void move_back(enum movement movement) {
  set_front_sensors_correction(false);
  set_side_sensors_close_correction(false);
  set_side_sensors_far_correction(false);

  move_straight((CELL_DIMENSION / 2) - calc_straight_stop_distance(300) - current_cell_start_mm, 300, false);
  // move_straight_until_front_distance(CELL_DIMENSION / 2, 300, true);

  // delay(1500);

  disable_sensors_correction();
  // int8_t sign = (int8_t)(rand() % 2) * 2 - 1;
  // move_inplace_angle(180 * sign, 10);
  move_inplace_turn(movement);

  set_side_sensors_close_correction(true);


  if (movement == MOVE_180W) {
    move_straight((CELL_DIMENSION - WALL_WIDTH) / 2 - ROBOT_BACK_LENGTH, -100, true);
    set_starting_position();
  } else {
    move_straight(((CELL_DIMENSION - WALL_WIDTH) / 2 - ROBOT_BACK_LENGTH) / 2, -100, true);
    current_cell_start_mm = ((CELL_DIMENSION - WALL_WIDTH) / 2 - ROBOT_BACK_LENGTH) / 2;
  }

  
  // delay(1500);
  // set_competicion_iniciada(false);
  // return;

  set_front_sensors_correction(false);
  set_side_sensors_close_correction(true);
  set_side_sensors_far_correction(false);
  move_straight(CELL_DIMENSION - SENSING_POINT_DISTANCE - current_cell_start_mm, 500, false);
  enter_next_cell();
}

void set_starting_position(void) {
  current_cell_start_mm = ROBOT_BACK_LENGTH + WALL_WIDTH / 2;
}

int32_t get_current_cell_travelled_distance(void){
  return get_encoder_avg_millimeters() - current_cell_absolute_start_mm;
}

/**
 * @brief Movimiento frontal paramétrico
 *
 * @param distance
 * @param speed
 * @param stop
 */
void move_straight(int32_t distance, int32_t speed, bool stop) {
  int32_t current_distance = get_encoder_avg_micrometers();
  int32_t stop_distance = 0;
  set_ideal_angular_speed(0.0);
  set_target_linear_speed(speed);
  if (speed >= 0) {
    while (is_competicion_iniciada() && get_encoder_avg_micrometers() <= current_distance + (distance - stop_distance) * MICROMETERS_PER_MILLIMETER) {
      if (stop) {
        stop_distance = calc_straight_stop_distance(get_ideal_linear_speed());
      }
    }
  } else {
    while (is_competicion_iniciada() && get_encoder_avg_micrometers() >= current_distance - (distance - stop_distance) * MICROMETERS_PER_MILLIMETER) {
      if (stop) {
        stop_distance = calc_straight_stop_distance(get_ideal_linear_speed());
      }
    }
  }
  if (stop) {
    set_target_linear_speed(0);
    set_ideal_angular_speed(0.0);
    while (is_competicion_iniciada() &&  get_ideal_linear_speed() != 0) {
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
  while (is_competicion_iniciada() && (get_sensor_distance(SENSOR_FRONT_LEFT_WALL_ID) + get_sensor_distance(SENSOR_FRONT_RIGHT_WALL_ID)) / 2 > (distance + stop_distance)) {
    if (stop) {
      stop_distance = calc_straight_stop_distance(get_ideal_linear_speed());
    }
    set_RGB_color(255, 0, 0);
  }
  set_RGB_color(0, 0, 0);
  if (stop) {
    set_target_linear_speed(0);
    while (is_competicion_iniciada() && get_encoder_avg_speed() != 0) {
    }
  }
}

/**
 * @brief Movimiento de giro paramétrico
 *
 * @param turn_type
 */
void move_arc_turn(enum movement turn_type) {
  struct turn_params turn = turns[turn_type];

  uint32_t ms_start = get_clock_ticks();
  uint32_t ms_current = ms_start;
  float angular_speed = 0;
  while (true) {
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
  set_ideal_angular_speed(0);
}

void move_inplace_turn(enum movement movement) {
  struct turn_params turn = turns[movement];
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
    while (is_competicion_iniciada() && get_gyro_z_degrees() <= target_angle) {
    }
  } else {
    set_ideal_angular_speed(-rads);
    while (is_competicion_iniciada() && get_gyro_z_degrees() >= target_angle) {
    }
  }
  set_ideal_angular_speed(0.0);
}

void move(enum movement movement) {
  switch (movement) {
    case MOVE_START:
      set_starting_position();
      move_front();
      break;
    case MOVE_FRONT:
      move_front();
      break;
    case MOVE_LEFT:
    case MOVE_RIGHT:
      move_side(movement);
      break;
    case MOVE_180:
    case MOVE_180W:
      move_back(movement);
      break;
    default:
      break;
  }
}