#include <timetrial.h>

#define NUM_LAPS 6

void timetrial_start(void) {
  configure_kinematics(menu_run_get_speed());
  clear_info_leds();
  set_RGB_color(0, 0, 0);
  set_target_fan_speed(get_kinematics().fan_speed, 400);
  delay(500);
}

void timetrial_loop(void) {
  run_straight(CELL_DIMENSION * (maze_get_rows() - 1) - (ROBOT_BACK_LENGTH + WALL_WIDTH / 2.0f),
               ROBOT_BACK_LENGTH + WALL_WIDTH / 2.0f,
               get_kinematics().turns[MOVE_RIGHT_90].start,
               maze_get_rows() - 1,
               true,
               get_kinematics().linear_speed,
               get_kinematics().turns[MOVE_RIGHT_90].linear_speed);

  run_side(MOVE_RIGHT_90, get_kinematics().turns[MOVE_RIGHT_90], get_kinematics().turns[MOVE_RIGHT_90]);

  for (int i = 0; i < NUM_LAPS * 4 - 1; i++) {
    run_straight(CELL_DIMENSION * (maze_get_columns() - 2),
                 get_kinematics().turns[MOVE_RIGHT_90].end,
                 get_kinematics().turns[MOVE_RIGHT_90].start,
                 maze_get_columns() - 2,
                 false,
                 get_kinematics().linear_speed,
                 get_kinematics().turns[MOVE_RIGHT_90].linear_speed);

    run_side(MOVE_RIGHT_90,
             get_kinematics().turns[MOVE_RIGHT_90],
             get_kinematics().turns[MOVE_RIGHT_90]);
  }

  move(MOVE_BACK_STOP);
  set_race_started(false);
}