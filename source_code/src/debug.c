#include <debug.h>

bool debug_enabled = false;
bool debug_use_control = false;
uint32_t last_print_debug = 0;

bool debug_sensors_raw_enabled = false;

uint32_t last_keep_z_angle = 0;
uint32_t last_keep_front_distance = 0;

static void debug_macroarray(void) {
  macroarray_print();
  debug_enabled = false;
  menu_config_reset_values();
}

/**
 * @brief Imprime los valores de los sensores sin aplicar ninguna corrección
 *
 */
static void debug_sensors_raw(void) {
  if (get_clock_ticks() > last_print_debug + 50) {
    printf("SL: %4d - %4d = %4d ", get_sensor_raw(SENSOR_SIDE_LEFT_WALL_ID, 1), get_sensor_raw(SENSOR_SIDE_LEFT_WALL_ID, 0), get_sensor_raw_filter(SENSOR_SIDE_LEFT_WALL_ID));
    printf("FL: %4d - %4d = %4d ", get_sensor_raw(SENSOR_FRONT_LEFT_WALL_ID, 1), get_sensor_raw(SENSOR_FRONT_LEFT_WALL_ID, 0), get_sensor_raw_filter(SENSOR_FRONT_LEFT_WALL_ID));
    printf("FR: %4d - %4d = %4d ", get_sensor_raw(SENSOR_FRONT_RIGHT_WALL_ID, 1), get_sensor_raw(SENSOR_FRONT_RIGHT_WALL_ID, 0), get_sensor_raw_filter(SENSOR_FRONT_RIGHT_WALL_ID));
    printf("SR: %4d - %4d = %4d ", get_sensor_raw(SENSOR_SIDE_RIGHT_WALL_ID, 1), get_sensor_raw(SENSOR_SIDE_RIGHT_WALL_ID, 0), get_sensor_raw_filter(SENSOR_SIDE_RIGHT_WALL_ID));
    printf("\n");
    last_print_debug = get_clock_ticks();
  }
}

static void debug_sensors_distances(void) {
  if (get_clock_ticks() > last_print_debug + 50) {
    printf("SL: %4d ", get_sensor_distance(SENSOR_SIDE_LEFT_WALL_ID));
    printf("FL: %4d ", get_sensor_distance(SENSOR_FRONT_LEFT_WALL_ID));
    printf("FR: %4d ", get_sensor_distance(SENSOR_FRONT_RIGHT_WALL_ID));
    printf("SR: %4d ", get_sensor_distance(SENSOR_SIDE_RIGHT_WALL_ID));
    printf("LW: %d ", left_wall_detection() ? 1 : 0);
    printf("FW: %d ", front_wall_detection() ? 1 : 0);
    printf("RW: %d ", right_wall_detection() ? 1 : 0);
    printf("side_error: %4.2f ", get_side_sensors_error());
    printf("diagonal_error: %4d ", get_front_sensors_diagonal_error());
    printf("front_angle_error: %4d ", get_front_sensors_angle_error());
    printf("\n");
    last_print_debug = get_clock_ticks();
  }
}

static void debug_floodfill_maze(void) {
  floodfill_maze_print();
  debug_enabled = false;
  menu_config_reset_values();
}

static void check_debug_active(void) {
  if (get_menu_mode_btn()) {
    while (get_menu_mode_btn()) {
    }
    debug_enabled = !debug_enabled;
    if (!debug_enabled) {
      menu_config_reset_values();
      set_RGB_color(0, 0, 0);
    } else {
      set_RGB_color(0, 50, 0);
    }
  }
}

static void debug_encoders(void) {
  if (get_clock_ticks() > last_print_debug + 50) {
    printf("ENC_D: %4ld\tENC_I: %4ld\n", get_encoder_right_millimeters(), get_encoder_left_millimeters());
    last_print_debug = get_clock_ticks();
  }
}

static void debug_gyro(void) {
  if (get_clock_ticks() > last_print_debug + 50) {
    printf("raw: %.2f\tangle: %.2f\tangular_speed: %.2f\n", lsm6dsr_get_gyro_z_raw(), lsm6dsr_get_gyro_z_degrees(), lsm6dsr_get_gyro_z_radps());
    last_print_debug = get_clock_ticks();
  }
}

static void debug_motors_current(void) {
  if (get_clock_ticks() > last_print_debug + 50) {
    set_motors_enable(true);
    set_motors_speed(150, 150);
    // printf("BA: %4d CI: %4d CD: %4d BO: %4d\n", get_aux_raw(AUX_BATTERY_ID), get_aux_raw(AUX_CURRENT_LEFT_ID), get_aux_raw(AUX_CURRENT_RIGHT_ID), get_aux_raw(AUX_MENU_BTN_ID));
    printf("ENC_D: %4ld\tENC_I: %4ld\n", get_encoder_right_millimeters(), get_encoder_left_millimeters());
    last_print_debug = get_clock_ticks();
  }
}

static void debug_timetrial_demo(void) {
  delay(1000);
  configure_kinematics(menu_run_get_speed());
  clear_info_leds();
  set_RGB_color(0, 0, 0);
  debug_enabled = false;
  set_race_started(true);
  set_target_fan_speed(get_kinematics().fan_speed, 1000);
  delay(1500);

  run_straight(CELL_DIMENSION * 2 - (ROBOT_BACK_LENGTH + WALL_WIDTH / 2.0f),
               ROBOT_BACK_LENGTH + WALL_WIDTH / 2.0f,
               get_kinematics().turns[MOVE_RIGHT_90].start,
               2,
               true,
               get_kinematics().linear_speed,
               get_kinematics().turns[MOVE_RIGHT_90].linear_speed,
               get_kinematics().turns[MOVE_RIGHT_90].sign);

  run_side(MOVE_RIGHT_90, get_kinematics().turns[MOVE_RIGHT_90], get_kinematics().turns[MOVE_RIGHT_90]);

  for (int i = 0; i < 3; i++) {
    run_straight(CELL_DIMENSION + get_kinematics().turns[MOVE_RIGHT_90].end,
                 get_kinematics().turns[MOVE_RIGHT_90].end,
                 get_kinematics().turns[MOVE_RIGHT_90].start,
                 1,
                 false,
                 get_kinematics().linear_speed,
                 get_kinematics().turns[MOVE_RIGHT_90].linear_speed,
                 get_kinematics().turns[MOVE_RIGHT_90].sign);

    run_side(MOVE_RIGHT_90,
             get_kinematics().turns[MOVE_RIGHT_90],
             get_kinematics().turns[MOVE_RIGHT_90]);
  }

  move(MOVE_BACK_STOP);
  set_race_started(false);
  menu_config_reset_values();
}

static void debug_keep_front_distance_demo(void) {
  // if (get_clock_ticks() >= last_keep_front_distance + 50) {
  //   if (get_front_wall_distance() < CELL_DIMENSION) {
  //     printf("front distance: %4d front error: %.4f\n", get_front_wall_distance(), get_front_wall_distance() - MIDDLE_MAZE_DISTANCE);
  //   }
  //   last_keep_front_distance = get_clock_ticks();
  // }
  set_RGB_color(0, 50, 0);
  reset_control_all();
  debug_use_control = true;
  configure_kinematics(SPEED_EXPLORE);
  delay(1000);
  set_race_started(true);
  set_sensors_enabled(true);
  set_RGB_color(0, 50, 0);
  set_target_fan_speed(get_kinematics().fan_speed, 1000);
  delay(1200);
  do {
    if (get_clock_ticks() >= last_keep_front_distance + 1) {
      if (get_front_wall_distance() < CELL_DIMENSION) {
        // printf("front distance: %4d front error: %.4f\n", get_front_wall_distance(), get_front_wall_distance() - MIDDLE_MAZE_DISTANCE);
        set_RGB_color(0, 0, 50);
        set_linear_error_correction(false);
        set_angular_error_correction(false);
        set_front_sensors_angle_correction(true);
        set_front_sensors_distance_correction(true);
        set_ideal_front_distance(MIDDLE_MAZE_DISTANCE);
      } else {
        set_RGB_color(50, 0, 0);
        set_front_sensors_angle_correction(false);
        set_front_sensors_distance_correction(false);
        set_linear_error_correction(true);
        set_angular_error_correction(true);
      }
      last_keep_front_distance = get_clock_ticks();
    }
    check_debug_active();
  } while (debug_enabled && is_race_started() && !is_motor_saturated());

  debug_enabled = false;
  debug_use_control = false;
  set_fan_speed(0);
  set_race_started(false);
  reset_control_all();
  menu_config_reset_values();
}

static void debug_gyro_demo(void) {
  reset_control_all();
  delay(1000);
  set_fan_speed(35);
  do {
    if (get_clock_ticks() >= last_keep_z_angle + 1) {
      keep_z_angle();
      last_keep_z_angle = get_clock_ticks();
    }
    check_debug_active();
  } while (debug_enabled);
  set_fan_speed(0);
  reset_control_all();
}

static void debug_fan_demo(void) {
  reset_control_all();
  delay(1000);
  set_fan_speed(35);
  do {
    check_debug_active();
  } while (debug_enabled);
  set_fan_speed(0);
  reset_control_all();
}

bool is_debug_enabled(void) {
  return debug_enabled;
}

bool is_debug_use_control(void) {
  return debug_use_control;
}

void set_debug_enabled(bool enabled) {
  debug_enabled = enabled;
}

void debug_from_config(uint8_t type) {
  if (type != DEBUG_NONE) {
    check_debug_active();
  } else {
    debug_enabled = false;
  }
  if (debug_enabled) {
    switch (type) {
      case DEBUG_MACROARRAY:
        debug_macroarray();
        break;
      case DEBUG_TYPE_SENSORS:
        if (get_menu_up_btn()) {
          while (get_menu_up_btn()) {
          }
          debug_sensors_raw_enabled = !debug_sensors_raw_enabled;
        }
        if (debug_sensors_raw_enabled) {
          debug_sensors_raw();
        } else {
          debug_sensors_distances();
        }
        break;
      case DEBUG_FLOODFILL_MAZE:
        debug_floodfill_maze();
        break;
      case DEBUG_ENCODERS:
        debug_encoders();
        break;
      case DEBUG_GYRO:
        debug_gyro();
        break;
      case DEBUG_MOTORS_CURRENT:
        debug_motors_current();
        break;
      case DEBUG_TIMETRIAL:
        debug_timetrial_demo();
        break;
      case DEBUG_KEEP_FRONT_DISTANCE:
        debug_keep_front_distance_demo();
        break;
      case DEBUG_GYRO_DEMO:
        debug_gyro_demo();
        break;
      case DEBUG_FAN_DEMO:
        debug_fan_demo();
        break;
      default:
        debug_enabled = false;
        break;
    }
  } else {
    set_RGB_color(0, 0, 0);
  }
}

void debug_from_main(uint8_t type) {
  debug_enabled = true;
  set_sensors_enabled(true);
  debug_from_config(type);
}