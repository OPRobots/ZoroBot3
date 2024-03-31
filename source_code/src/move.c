#include "move.h"

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
//   while(get_gyro_z_radps() != 0){

//   }
}