#include "buttons.h"

static uint32_t btn_analog = 0;

static uint32_t btn_menu_up_ms = 0;
static uint32_t btn_menu_down_ms = 0;
static uint32_t btn_menu_mode_ms = 0;

void check_buttons(void) {
  btn_analog = get_aux_raw(AUX_MENU_BTN_ID);

  if (btn_analog >= 1000 && btn_analog <= 2000) {
    if (btn_menu_up_ms == 0) {
      btn_menu_up_ms = get_clock_ticks();
    }
  } else {
    btn_menu_up_ms = 0;
  }
  if (btn_analog >= 2000 && btn_analog <= 3000) {
    if (btn_menu_down_ms == 0) {
      btn_menu_down_ms = get_clock_ticks();
    }
  } else {
    btn_menu_down_ms = 0;
  }
  if (btn_analog >= 3000 && btn_analog <= 4000) {
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