#include <buttons.h>
#include <delay.h>

uint32_t btn_menu_up_ms = 0;
uint32_t btn_menu_down_ms = 0;
uint32_t btn_menu_mode_ms = 0;

void check_buttons(void) {
  if ((bool)gpio_get(GPIOC, GPIO14)) {
    if (btn_menu_up_ms == 0) {
      btn_menu_up_ms = get_clock_ticks();
    }
  } else {
    btn_menu_up_ms = 0;
  }
  if ((bool)gpio_get(GPIOC, GPIO15)) {
    if (btn_menu_down_ms == 0) {
      btn_menu_down_ms = get_clock_ticks();
    }
  } else {
    btn_menu_down_ms = 0;
  }
  if ((bool)gpio_get(GPIOC, GPIO13)) {
    if (btn_menu_mode_ms == 0) {
      btn_menu_mode_ms = get_clock_ticks();
    }
  } else {
    btn_menu_mode_ms = 0;
  }
}

/**
 * @brief Obtiene el estado del botón de Menú Arriba
 *
 * @return bool
 */
bool get_menu_up_btn(void) {
  return btn_menu_up_ms > 0 && get_clock_ticks() - btn_menu_up_ms > 50;
}

/**
 * @brief Obtiene el estado del botón de Menú Abajo
 *
 * @return bool
 */
bool get_menu_down_btn(void) {
  return btn_menu_down_ms > 0 && get_clock_ticks() - btn_menu_down_ms > 50;
}

/**
 * @brief Obtiene el estado del botón de Menú Modo
 *
 * @return bool
 */
bool get_menu_mode_btn(void) {
  return btn_menu_mode_ms > 0 && get_clock_ticks() - btn_menu_mode_ms > 50;
}

/**
 *
 * @brief Obtiene el estado del switch 1
 *
 * @return bool
 */
bool get_swtich_1(void) {
  return (bool)gpio_get(GPIOB, GPIO8);
}

/**
 *
 * @brief Obtiene el estado del switch 2
 *
 * @return bool
 */
bool get_swtich_2(void) {
  return (bool)gpio_get(GPIOB, GPIO9);
}

/**
 * @deprecated
 * @brief Obtiene el número decimal a partir del binario de los Switches
 *
 * @return uint8_t decimal
 */
uint8_t get_switch_decimal(void) {
  uint8_t binario = 0;
  uint8_t decimal = 0;

  if (get_swtich_1()) {
    binario += 10;
  }
  if (get_swtich_2()) {
    binario += 1;
  }

  uint8_t i = 0;
  uint8_t resto;
  while (binario != 0) {
    resto = binario % 10;
    binario /= 10;
    decimal += resto * pow(2, i);
    ++i;
  }
  return decimal;
}