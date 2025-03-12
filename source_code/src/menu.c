#include <menu.h>

#define MENU_RUN 0
#define MENU_CONFIG 1
#define MENU_TYPES 2

static uint8_t current_menu = MENU_RUN;

static bool handle_current_menu(void) {
  switch (current_menu) {
    case MENU_RUN:
      return menu_run_handler();
    case MENU_CONFIG:
      return menu_config_handler();
  }
  return false;
}

/**
 * @brief Controla que menú está activo [MENU_RUN, MENU_CONFIG]
 *
 */
void menu_handler(void) {
  bool menu_mode_btn_long_pressed = handle_current_menu();
  if (menu_mode_btn_long_pressed) {
    menu_config_reset_mode();
    current_menu = (current_menu + 1) % MENU_TYPES;
  }
}

void menu_reset(void) {
  current_menu = MENU_RUN;
  menu_run_reset();
}

void menu_rc5_mode_change(void) {
  switch (current_menu) {
    case MENU_RUN:
      menu_run_mode_change();
      break;
  }
}

void menu_rc5_up(void) {
  switch (current_menu) {
    case MENU_RUN:
      menu_run_up();
      break;
  }
}

void menu_rc5_down(void) {
  switch (current_menu) {
    case MENU_RUN:
      menu_run_down();
      break;
  }
}