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
    for (int8_t sensor = 0; sensor < get_sensors_num(); sensor++) {
      printf("%d\t", get_sensor_raw(sensor, 1));
    }
    printf("\n");
    last_print_debug = get_clock_ticks();
  }
}

static void debug_sensors_distances(void) {
  if (get_clock_ticks() > last_print_debug + 50) {
    for (int8_t sensor = 0; sensor < get_sensors_num(); sensor++) {
      printf("%d\t", get_sensor_distance(sensor));
    }
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

static void debug_gyro_demo(void) {
  // if (get_clock_ticks() >= last_keep_z_angle + 1) {
  delay(1000);
  do {
    keep_z_angle();
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
      case DEBUG_GYRO_DEMO:
        debug_gyro_demo();
        break;
      case DEBUG_FAN_DEMO:
        debug_fan_demo();
        break;
    }
  } else {
    set_RGB_color(0, 0, 0);
  }
}