#include <dragrace.h>

void dragrace_start(void) {
  configure_kinematics(menu_run_get_speed());
  clear_info_leds();
  set_RGB_color(0, 0, 0);
  if (is_battery_2s()) {
    set_target_fan_speed(get_kinematics().fan_speed_2s, 2000);
  } else {
    set_target_fan_speed(get_kinematics().fan_speed_3s, 2000);
  }
  set_side_sensors_correction(true);
  set_front_sensors_diagonal_correction(false);
  set_front_sensors_distance_correction(false);
  set_front_sensors_angle_correction(false);
  set_check_motors_saturated_enabled(false);
  delay(3000);
  set_target_linear_speed(get_kinematics().linear_speed);
  while(is_race_started() && !is_motor_pwm_saturated()){

  }
  set_race_started(false);
}