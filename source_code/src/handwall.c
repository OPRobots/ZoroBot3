#include <handwall.h>

static bool priorize_front = true;
static bool use_left_hand = true;

void handwall_set_priorize_front(bool priorize) {
  priorize_front = priorize;
}

void handwall_use_left_hand(void) {
  use_left_hand = true;
}

void handwall_use_right_hand(void) {
  use_left_hand = false;
}

void handwall_start(void) {
  set_front_sensors_correction(false);
  set_side_sensors_close_correction(true);
  set_side_sensors_far_correction(true);
  move_straight(70 + 46.36, 500, false);
}

void handwall_loop(void) {
  struct walls walls = get_walls();
  if (!walls.front && priorize_front) {
    set_side_sensors_close_correction(true);
    set_side_sensors_far_correction(true);
    move_straight(180, 500, false);
  } else if ((use_left_hand && !walls.left) || (!use_left_hand && walls.right && !walls.left)) {
    set_side_sensors_close_correction(false);
    set_side_sensors_far_correction(false);
    move_arc_turn(MOVE_LEFT);
    set_side_sensors_close_correction(true);
    set_side_sensors_far_correction(true);
  } else if ((!use_left_hand && !walls.right) || (use_left_hand && !walls.right)) {
    set_side_sensors_close_correction(false);
    set_side_sensors_far_correction(false);
    move_arc_turn(MOVE_RIGHT);
    set_side_sensors_close_correction(true);
    set_side_sensors_far_correction(true);
  } else if (!walls.front && !priorize_front) {
    set_side_sensors_close_correction(true);
    set_side_sensors_far_correction(true);
    move_straight(180, 500, false);
  } else if (walls.front && walls.left && walls.right) {
    // TODO: 180
    set_competicion_iniciada(false);
  } else {
    set_target_linear_speed(0);
    set_ideal_angular_speed(0);
    warning_status_led(50);
  }
}
