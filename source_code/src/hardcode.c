#include <hardcode.h>

static bool use_left_hand = true;
static uint32_t start_ms = 0;
static uint32_t time_limit = 0;

void hardcore_toggle_hand(void) {
  if (use_left_hand) {
    hardcode_use_right_hand();
  } else {
    hardcode_use_left_hand();
  }
}

void hardcode_use_left_hand(void) {
  use_left_hand = true;
  set_info_led(0, true);
  set_info_led(1, true);
  set_info_led(2, true);
  set_info_led(3, true);
  set_info_led(4, true);
  set_info_led(5, false);
  set_info_led(6, false);
  set_info_led(7, false);
  set_info_led(8, false);
  set_info_led(9, false);
  delay(1500);
}

void hardcode_use_right_hand(void) {
  use_left_hand = false;
  set_info_led(0, false);
  set_info_led(1, false);
  set_info_led(2, false);
  set_info_led(3, false);
  set_info_led(4, false);
  set_info_led(5, true);
  set_info_led(6, true);
  set_info_led(7, true);
  set_info_led(8, true);
  set_info_led(9, true);
  delay(1500);
}

void hardcode_set_time_limit(uint32_t ms) {
  time_limit = ms;
}

void hardcode_start(void) {
  start_ms = get_clock_ticks();
  configure_kinematics(menu_run_get_speed());
  clear_info_leds();
  set_RGB_color(0, 0, 0);
  set_target_fan_speed(get_kinematics().fan_speed, 400);
  delay(500);
  // move(MOVE_START);
}

void hardcode_loop(void) {
  if (time_limit > 0 && get_clock_ticks() - start_ms >= time_limit) {
    set_race_started(false);
    return;
  }

  enum movement TURN_RIGHT = MOVE_RIGHT;
  enum movement TURN_LEFT = MOVE_LEFT;
  enum movement TURN_RIGHT_180 = MOVE_RIGHT_180_RC;
  enum movement TURN_LEFT_180 = MOVE_LEFT_180_RC;
  switch (menu_run_get_speed()) {
    case SPEED_EXPLORE:
      TURN_RIGHT = MOVE_RIGHT;
      TURN_LEFT = MOVE_LEFT;
      break;
    case SPEED_NORMAL:
    case SPEED_MEDIUM:
    case SPEED_FAST:
    case SPEED_SUPER:
    case SPEED_HAKI:
      TURN_RIGHT = MOVE_RIGHT_90;
      TURN_LEFT = MOVE_LEFT_90;
      TURN_RIGHT_180 = MOVE_RIGHT_180_RC;
      TURN_LEFT_180 = MOVE_LEFT_180_RC;
      break;
  }

  if (use_left_hand) {
    run_hardcode_sector(10, TURN_RIGHT);
    run_hardcode_sector(400, TURN_LEFT);
    run_hardcode_sector(0, TURN_RIGHT);
    run_hardcode_sector(120, TURN_LEFT_180);
    run_hardcode_sector(200, TURN_RIGHT);
    run_hardcode_sector(0, TURN_LEFT);
    run_hardcode_sector(150, TURN_RIGHT_180);
    run_hardcode_sector(300, TURN_LEFT);
    run_hardcode_sector(0, TURN_RIGHT);
    run_hardcode_sector(300, TURN_LEFT);
    run_hardcode_sector(150, MOVE_NONE);
  } else {
    run_hardcode_sector(10, TURN_RIGHT);
    run_hardcode_sector(400, TURN_LEFT);
    run_hardcode_sector(0, TURN_RIGHT);
    run_hardcode_sector(120, TURN_LEFT_180);
    run_hardcode_sector(150, TURN_RIGHT_180);
    run_hardcode_sector(200, TURN_LEFT_180);
    run_hardcode_sector(280, TURN_RIGHT);
    run_hardcode_sector(0, TURN_LEFT);
    run_hardcode_sector(230, TURN_RIGHT);
    run_hardcode_sector(150, MOVE_NONE);
  }
  force_target_linear_speed(0);
  while (is_race_started()) {
    set_RGB_rainbow();
  }
}