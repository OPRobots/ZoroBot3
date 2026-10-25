#include <hardcode.h>

#include <battery.h>
#include <control.h>
#include <delay.h>
#include <leds.h>
#include <menu_run.h>
#include <motors.h>
#include <move.h>

static bool use_left_hand = true;
static bool hand_selected = false;
static bool hardcode_started = false;
static uint32_t start_ms = 0;
static uint32_t time_limit = 0;

void hardcode_reset(void) {
  hand_selected = false;
  hardcode_started = false;
}

bool hardcode_is_hand_selected(void) {
  return hand_selected;
}

void hardcode_toggle_hand(void) {
  if (use_left_hand) {
    hardcode_use_right_hand();
  } else {
    hardcode_use_left_hand();
  }
}

void hardcode_use_left_hand(void) {
  use_left_hand = true;
  set_info_led(INFO_LED_1, true);
  set_info_led(INFO_LED_2, true);
  set_info_led(INFO_LED_3, true);
  set_info_led(INFO_LED_4, true);
  set_info_led(INFO_LED_5, true);
  set_info_led(INFO_LED_A, false);
  set_info_led(INFO_LED_B, false);
  set_info_led(INFO_LED_C, false);
  set_info_led(INFO_LED_D, false);
  set_info_led(INFO_LED_E, false);
  delay(1500);
  hand_selected = true;
}

void hardcode_use_right_hand(void) {
  use_left_hand = false;
  set_info_led(INFO_LED_1, false);
  set_info_led(INFO_LED_2, false);
  set_info_led(INFO_LED_3, false);
  set_info_led(INFO_LED_4, false);
  set_info_led(INFO_LED_5, false);
  set_info_led(INFO_LED_A, true);
  set_info_led(INFO_LED_B, true);
  set_info_led(INFO_LED_C, true);
  set_info_led(INFO_LED_D, true);
  set_info_led(INFO_LED_E, true);
  delay(1500);
  hand_selected = true;
}

void hardcode_set_time_limit(uint32_t ms) {
  time_limit = ms;
}

void hardcode_start(void) {
  if (hardcode_started) {
    return;
  }
  hardcode_started = true;
  start_ms = get_clock_ticks();
  configure_kinematics(menu_run_get_speed());
  clear_info_leds();
  set_RGB_color(0, 0, 0);
  set_check_motors_saturated_enabled(false);
  if (is_battery_2s()) {
    set_target_fan_speed(get_kinematics().fan_speed_2s, 400);
  } else {
    set_target_fan_speed(get_kinematics().fan_speed_3s, 400);
  }
  delay(500);
}

void hardcode_loop(void) {
  if (!hardcode_started) {
    hardcode_start();
  }

  // if (time_limit > 0 && get_clock_ticks() - start_ms >= time_limit) {
  //   set_race_started(false);
  //   return;
  // }

  enum movement turn_right = MOVE_RIGHT;
  enum movement turn_left = MOVE_LEFT;
  enum movement turn_right_180 = MOVE_RIGHT_180_RC;
  enum movement turn_left_180 = MOVE_LEFT_180_RC;
  switch (menu_run_get_speed()) {
    case SPEED_NORMAL:
    case SPEED_MEDIUM:
    case SPEED_FAST:
    case SPEED_SUPER:
    case SPEED_HAKI:
      turn_right = MOVE_RIGHT_90_RC;
      turn_left = MOVE_LEFT_90_RC;
      turn_right_180 = MOVE_RIGHT_180_RC;
      turn_left_180 = MOVE_LEFT_180_RC;
      break;
    default:
      set_race_started(false);
      return;
  }

  if (use_left_hand) {
    run_hardcode_sector(45, turn_right); // long
    run_hardcode_sector(450, turn_left);
    run_hardcode_sector(90, turn_right); // long
    run_hardcode_sector(161, turn_left_180);

    run_hardcode_sector(207, turn_right);
    run_hardcode_sector(100, turn_left); // long
    run_hardcode_sector(279, turn_right_180);
    run_hardcode_sector(360, turn_left);
    run_hardcode_sector(104, turn_right); // long
    run_hardcode_sector(414, turn_left);
    run_hardcode_sector(150, MOVE_NONE); // long
  } else {
    run_hardcode_sector(45, turn_right); // long
    run_hardcode_sector(450, turn_left);
    run_hardcode_sector(90, turn_right); // long
    run_hardcode_sector(161, turn_left_180);

    run_hardcode_sector(207, turn_right_180);
    run_hardcode_sector(206, turn_left_180);
    run_hardcode_sector(361, turn_right);
    run_hardcode_sector(116, turn_left); // long
    run_hardcode_sector(419, turn_right);
    run_hardcode_sector(150, MOVE_NONE); // long
  }
  force_linear_speed(0);
  while (is_race_started()) {
    set_RGB_rainbow();
  }
}
