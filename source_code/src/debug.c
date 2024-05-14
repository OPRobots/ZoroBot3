#include <debug.h>

bool debug_enabled = false;
uint32_t last_print_debug = 0;

static void debug_macroarray(void) {
  macroarray_print();
  debug_enabled = false;
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

static void check_debug_active(void) {
  if (get_menu_mode_btn()) {
    while (get_menu_mode_btn()) {
    }
    debug_enabled = !debug_enabled;
  }
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
      case DEBUG_TYPE_SENSORS_RAW:
        debug_sensors_raw();
        break;
    }
  }
}