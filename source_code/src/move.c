#include "move.h"

struct turn_params turns[] = {
    [MOVE_LEFT] = {55, 15, 500, 612.5, 9.625, 16, 148, -1},
    [MOVE_RIGHT] = {55, 15, 500, 612.5, 9.625, 16, 148, 1},
};

void move_straight(int32_t distance, int32_t speed, bool stop) {

  int32_t current_distance = get_encoder_total_average_micrometers();
  set_ideal_angular_speed(0.0);
  set_target_linear_speed(speed);
  if (speed >= 0) {
    while (get_encoder_total_average_micrometers() <= current_distance + distance * MICROMETERS_PER_MILLIMETER) {
      //   printf("%4ld %4ld %4ld\n", current_distance, get_encoder_total_average_millimeters(), current_distance + distance* MICROMETERS_PER_MILLIMETER);
    }
  } else {
    while (get_encoder_total_average_micrometers() >= current_distance - distance * MICROMETERS_PER_MILLIMETER) {
      //   printf("%4ld %4ld %4ld\n", current_distance, get_encoder_total_average_millimeters(), current_distance - distance* MICROMETERS_PER_MILLIMETER);
    }
  }

  if (stop) {
    set_target_linear_speed(0);
    set_ideal_angular_speed(0.0);
    while (get_encoder_avg_speed() != 0) {
      // printf("stop\n");
    }
  }
}

void move_arc_turn(enum movement turn_type) {
  struct turn_params turn = turns[turn_type];

  move_straight(turn.start, turn.linear_speed, false);
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
  move_straight(turn.end, turn.linear_speed, false);
}

void move_inplace_turn(float angle, float rads) {
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
    while (get_gyro_z_degrees() <= target_angle) {
    }
  } else {
    set_ideal_angular_speed(-rads);
    while (get_gyro_z_degrees() >= target_angle) {
    }
  }
  set_ideal_angular_speed(0.0);
}