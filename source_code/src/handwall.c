#include <handwall.h>

static bool use_left_hand = true;

void handwall_use_left_hand(void) {
  use_left_hand = true;
}

void handwall_use_right_hand(void) {
  use_left_hand = false;
}

void handwall_start(void) {
  configure_kinematics(menu_run_get_speed());
  clear_info_leds();
  set_RGB_color(0, 0, 0);
  if (is_battery_2s()) {
    set_target_fan_speed(get_kinematics().fan_speed_2s, 400);
  } else {
    set_target_fan_speed(get_kinematics().fan_speed_3s, 400);
  }
  delay(500);
  move(MOVE_START);
}

void handwall_loop(void) {
  struct walls walls = get_walls();
  set_RGB_color_while(255, 255, 0, 20);
  if ((use_left_hand && !walls.left) || (!use_left_hand && walls.right && !walls.left)) {
    move(MOVE_LEFT);
  } else if ((!use_left_hand && !walls.right) || (use_left_hand && walls.left && !walls.right)) {
    move(MOVE_RIGHT);
  } else if (!walls.front) {
    move(MOVE_FRONT);
  } else if (walls.front && walls.left && walls.right) {
    move(MOVE_BACK_WALL);
  } else {
    set_target_linear_speed(0);
    set_ideal_angular_speed(0);
    warning_status_led(50);
  }
}
