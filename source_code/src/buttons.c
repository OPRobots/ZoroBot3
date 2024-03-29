#include <buttons.h>
#include <delay.h>

/**
 * @brief Obtiene el estado del botón de Menú Arriba
 * 
 * @return bool
 */
bool get_menu_up_btn() {
  bool state1 = (bool)gpio_get(GPIOC, GPIO14);
  delay(50);
  bool state2 = (bool)gpio_get(GPIOC, GPIO14);
  return state1 && state2;
}

/**
 * @brief Obtiene el estado del botón de Menú Abajo
 * 
 * @return bool
 */
bool get_menu_down_btn() {
  bool state1 = (bool)gpio_get(GPIOC, GPIO15);
  delay(50);
  bool state2 = (bool)gpio_get(GPIOC, GPIO15);
  return state1 && state2;
}

/**
 * @brief Obtiene el estado del botón de Menú Modo
 * 
 * @return bool
 */
bool get_menu_mode_btn() {
  bool state1 = (bool)gpio_get(GPIOC, GPIO13);
  delay(50);
  bool state2 = (bool)gpio_get(GPIOC, GPIO13);
  return state1 && state2;
}

/**
 * 
 * @brief Obtiene el estado del switch 1
 * 
 * @return bool
 */
bool get_swtich_1() {
  return (bool)gpio_get(GPIOB, GPIO8);
}

/**
 * 
 * @brief Obtiene el estado del switch 2
 * 
 * @return bool
 */
bool get_swtich_2() {
  return (bool)gpio_get(GPIOB, GPIO9);
}

/**
 * @deprecated
 * @brief Obtiene el número decimal a partir del binario de los Switches
 * 
 * @return uint8_t decimal
 */
uint8_t get_switch_decimal() {
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

/**
 * @brief Comprueba el estado del modulo de inicio-parada
 * 
 */
void check_start_stop_module() {
  if ((bool)gpio_get(GPIOB, GPIO8)) {
    set_status_led(true);
    set_competicion_iniciada(true);
  } else {
    set_status_led(false);
    set_competicion_iniciada(false);
  }
}