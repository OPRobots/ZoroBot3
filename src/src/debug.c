#include <debug.h>

bool debug_enabled = false;
uint32_t last_print_debug = 0;

/**
 * @brief Imprime los valores de los sensores sin aplicar ninguna corrección
 * 
 */
static void debug_sensors_raw() {
  if (get_clock_ticks() > last_print_debug + 50) {
    for (int8_t sensor = 0; sensor < get_sensors_num(); sensor++) {
      printf("%d\t", get_sensor_raw(sensor, 1));
    }
    printf("\n");
    last_print_debug = get_clock_ticks();
  }
}


static void check_debug_active(){
    debug_enabled = true;
}

void debug_from_config(uint8_t type) {
  check_debug_active();
  if (debug_enabled) {
    switch (type) {
      case DEBUG_TYPE_SENSORS_RAW:
        debug_sensors_raw();
        break;
      case DEBUG_TYPE_SENSORS_CALIBRATED:
        // debug_sensors_calibrated();
        break;
      case DEBUG_TYPE_LINE_POSITION:
        // debug_line_position();
        break;
      case DEBUG_TYPE_MOTORS:
        // debug_motors();
        break;
      case DEBUG_TYPE_ENCODERS:
        // debug_encoders();
        break;
      case DEBUG_TYPE_DIGITAL_IO:
        // debug_digital_io();
        break;
      case DEBUG_TYPE_CORRECCION_POSICION:
        // debug_posicion_correccion();
        break;
      case DEBUG_TYPE_LEDS_PARTY:
        // debug_all_leds();
        break;
      case DEBUG_TYPE_FANS_DEMO:
        // debug_fans();
        break;
    }
  } else {
    switch (type) {
      case DEBUG_TYPE_MOTORS:
        // set_motors_speed(0, 0);
        break;
      case DEBUG_TYPE_LEDS_PARTY:
        // all_leds_clear();
        break;
      case DEBUG_TYPE_FANS_DEMO:
        // set_fan_speed(0);
        break;
    }
  }
}