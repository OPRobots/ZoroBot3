#include <basic_debug.h>

#define MODOS_DEBUG 3
uint8_t modo_debug = 0;

void debug_inicio(void) {
  if (get_menu_mode_btn()) {
    modo_debug = (modo_debug + 1) % MODOS_DEBUG;
    clear_info_leds();
    while (get_menu_mode_btn()) {
    }
  }
  if (modo_debug == 0) {
    for (int i = 0; i < 20; i++) {
      filtro_sensores();
      delay_us(1000 / 20);
    }
    set_info_led(0, sensor0());
    set_info_led(3, sensor1());
    set_info_led(4, sensor2());
    set_info_led(7, sensor3());
  }
  if (modo_debug == 1) {
    set_info_led(0, true);
    set_info_led(7, true);
    imprimir_sensores_raw();
  }
  if (modo_debug == 2) {
    set_info_led(0, true);
    set_info_led(7, true);
    set_info_led(1, true);
    set_info_led(6, true);
    imprimir_sensores_filtrados_analog();
  }
}

void imprimir_sensores_raw(void) {
  printf("%4d %4d %4d %4d", get_sensor_raw_filter(SENSOR_SIDE_LEFT_ID), get_sensor_raw_filter(SENSOR_FRONT_LEFT_ID), get_sensor_raw_filter(SENSOR_FRONT_RIGHT_ID), get_sensor_raw_filter(SENSOR_SIDE_RIGHT_ID));
  // delay(85);
}

void imprimir_sensores_filtrados(void) {
  for (int i = 0; i < 20; i++) {
    filtro_sensores();
    delay_us(1000 / 20);
  }
  printf("%d %d %d %d", sensor0(), sensor1(), sensor2(), sensor3());

  delay(85);
}
void imprimir_sensores_filtrados_analog(void) {
  for (int i = 0; i < 20; i++) {
    filtro_sensores();
    delay_us(1000 / 20);
  }
  printf("%4d %4d %4d %4d", sensor0_analog(), sensor1_analog(), sensor2_analog(), sensor3_analog());

  // delay(10);
}
