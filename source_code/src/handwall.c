#include <handwall.h>

static bool use_left_hand = true;
static uint32_t start_ms = 0;
static uint32_t time_limit = 0;

void handwall_use_left_hand(void) {
  use_left_hand = true;
}

void handwall_use_right_hand(void) {
  use_left_hand = false;
}

void handwall_set_time_limit(uint32_t ms) {
  time_limit = ms;
}

void handwall_start(void) {
  start_ms = get_clock_ticks();
  set_front_sensors_correction(false);
  set_side_sensors_close_correction(true);
  set_side_sensors_far_correction(true);
  move_straight(CELL_DIMENSION - ROBOT_BACK_LENGTH - SENSING_POINT_DISTANCE - (WALL_WIDTH / 2), 500, false);
}

void handwall_loop(void) {
  if (get_clock_ticks() - start_ms >= time_limit) {
    set_competicion_iniciada(false);
    return;
  }
  struct walls walls = get_walls();
  set_RGB_color_while(0, 0, 255, 100);
  if ((use_left_hand && !walls.left) || (!use_left_hand && walls.right && !walls.left)) {
    set_side_sensors_close_correction(false);
    set_side_sensors_far_correction(false);
    move_arc_turn(MOVE_LEFT);
    set_side_sensors_close_correction(true);
    set_side_sensors_far_correction(true);
  } else if ((!use_left_hand && !walls.right) || (use_left_hand && walls.left && !walls.right)) {
    set_side_sensors_close_correction(false);
    set_side_sensors_far_correction(false);
    move_arc_turn(MOVE_RIGHT);
    set_side_sensors_close_correction(true);
    set_side_sensors_far_correction(true);
  } else if (!walls.front) {
    set_side_sensors_close_correction(true);
    set_side_sensors_far_correction(true);
    move_straight(CELL_DIMENSION, 500, false);
  } else if (walls.front && walls.left && walls.right) {
    set_side_sensors_close_correction(true);
    set_side_sensors_far_correction(false);
    move_straight(MIDDLE_MAZE_DISTANCE + SENSING_POINT_DISTANCE, 350, false);
    // set_front_sensors_correction(true);
    move_straight_until_front_distance(MIDDLE_MAZE_DISTANCE, 250, true);

    // set_front_sensors_correction(false);
    set_side_sensors_close_correction(false);
    // TODO: cambiar a giro con aceleracion angular y comprobación mediante encoders
    move_inplace_turn(180, 10);

    move_straight(ROBOT_BACK_LENGTH, -100, true);

    set_side_sensors_close_correction(true);
    set_side_sensors_far_correction(true);
    move_straight(CELL_DIMENSION - ROBOT_BACK_LENGTH - SENSING_POINT_DISTANCE - (WALL_WIDTH / 2), 500, false);
  } else {
    set_target_linear_speed(0);
    set_ideal_angular_speed(0);
    warning_status_led(50);
  }
}
