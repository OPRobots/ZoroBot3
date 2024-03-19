#include <debug.h>

bool debug_enabled = false;
uint32_t last_print_debug = 0;

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

static void check_debug_active() {
  debug_enabled = true;
}

void debug_from_config(uint8_t type) {
  check_debug_active();
  if (debug_enabled) {
    switch (type) {
      case DEBUG_TYPE_SENSORS_RAW:
        debug_sensors_raw();
        break;
    }
  }
}