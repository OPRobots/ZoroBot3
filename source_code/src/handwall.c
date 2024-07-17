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
  move(MOVE_START);
}

void handwall_loop(void) {
  if (time_limit > 0 && get_clock_ticks() - start_ms >= time_limit) {
    set_race_started(false);
    return;
  }
  struct walls walls = get_walls();
  set_RGB_color_while(0, 0, 255, 20);
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
