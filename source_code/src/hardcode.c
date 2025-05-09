#include <hardcode.h>

static bool use_left_hand = true;
static uint32_t start_ms = 0;
static uint32_t time_limit = 0;

static void hardcode_sector(uint16_t distance, enum movement turn) {
  if (distance > 0) {
    run_straight_hardcoded(distance, get_kinematics().linear_speed, turn != MOVE_NONE ? get_kinematics().turns[turn].linear_speed : 0, false, false);
  }
  if (turn != MOVE_NONE) {
    move_arc_turn(turn);
  }
}

void hardcode_use_left_hand(void) {
  use_left_hand = true;
}

void hardcode_use_right_hand(void) {
  use_left_hand = false;
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
  enum movement TURN_RIGHT_180 = MOVE_RIGHT_180;
  enum movement TURN_LEFT_180 = MOVE_LEFT_180;
  switch (menu_run_get_speed()) {
    case SPEED_EXPLORE:
    case SPEED_NORMAL:
      TURN_RIGHT = MOVE_RIGHT;
      TURN_LEFT = MOVE_LEFT;
      break;
    case SPEED_MEDIUM:
    case SPEED_FAST:
    case SPEED_HAKI:
      TURN_RIGHT = MOVE_RIGHT_90;
      TURN_LEFT = MOVE_LEFT_90;
      TURN_RIGHT_180 = MOVE_RIGHT_180;
      TURN_LEFT_180 = MOVE_LEFT_180;
      break;
  }

  if (use_left_hand) {
    hardcode_sector(50, MOVE_RIGHT);
    hardcode_sector(502, TURN_LEFT);
    hardcode_sector(35, TURN_RIGHT);
    hardcode_sector(130, TURN_LEFT_180);
    hardcode_sector(200, TURN_RIGHT);
    hardcode_sector(50, TURN_LEFT);
    hardcode_sector(260, TURN_RIGHT_180);
    hardcode_sector(350, TURN_LEFT);
    hardcode_sector(50, TURN_RIGHT);
    hardcode_sector(300, TURN_LEFT);
    hardcode_sector(300, MOVE_NONE);
  } else {
    hardcode_sector(50, MOVE_RIGHT);
    hardcode_sector(502, TURN_LEFT);
    hardcode_sector(30, TURN_RIGHT);
    hardcode_sector(130, TURN_LEFT_180);
    hardcode_sector(180, TURN_RIGHT_180);
    hardcode_sector(180, TURN_LEFT_180);
    hardcode_sector(280, TURN_RIGHT);
    hardcode_sector(110, TURN_LEFT);
    hardcode_sector(290, TURN_RIGHT);
    hardcode_sector(300, MOVE_NONE);
  }
  force_target_linear_speed(0);
  while (is_race_started()) {
    set_RGB_rainbow();
  }
}
