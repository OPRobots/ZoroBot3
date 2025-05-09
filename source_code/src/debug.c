#include <debug.h>

bool debug_enabled = false;
uint32_t last_print_debug = 0;

uint32_t last_keep_z_angle = 0;

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
    }
  }
}

static void debug_motors_current(void) {
  if (get_clock_ticks() > last_print_debug + 50) {
    set_motors_enable(true);
    set_motors_speed(150, 150);
    printf("BA: %4d CI: %4d CD: %4d BO: %4d\n", get_aux_raw(AUX_BATTERY_ID), get_aux_raw(AUX_CURRENT_LEFT_ID), get_aux_raw(AUX_CURRENT_RIGHT_ID), get_aux_raw(AUX_MENU_BTN_ID));
    last_print_debug = get_clock_ticks();
  }
}

static void debug_gyro_demo(void) {
  // if (get_clock_ticks() >= last_keep_z_angle + 1) {
  lsm6dsr_set_gyro_z_degrees(0);
  delay(1000);
  do {
    lsm6dsr_keep_z_angle();
    check_debug_active();
  } while (debug_enabled);
  // last_keep_z_angle = get_clock_ticks();
  // }
}

static void debug_fan_demo(void) {
  delay(1000);
  do {
    set_fan_speed(75);
    check_debug_active();
  } while (debug_enabled);
  set_fan_speed(0);
}

bool is_debug_enabled(void) {
  return debug_enabled;
}

void debug_from_config(uint8_t type) {
  if (type != DEBUG_NONE) {
    check_debug_active();
  } else {
    debug_enabled = false;
  }
  if (debug_enabled) {
    set_sensors_enabled(true);
    set_RGB_color(0, 50, 0);
    switch (type) {
      case DEBUG_MACROARRAY:
        debug_macroarray();
        break;
      case DEBUG_TYPE_SENSORS_RAW:
        debug_sensors_raw();
        break;
      case DEBUG_TYPE_SENSORS_DISTANCES:
        debug_sensors_distances();
        break;
      case DEBUG_FLOODFILL_MAZE:
        debug_floodfill_maze();
        break;
      case DEBUG_MOTORS_CURRENT:
        debug_motors_current();
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