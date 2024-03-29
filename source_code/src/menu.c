#include <menu.h>

uint8_t modoConfig = 0;
#define MODE_NOTHING 0
#define MODE_SPEED 1
#define MODE_FANS 2
#define MODE_DEBUG 3

#define NUM_MODOS_RACE 3
#define NUM_MODOS_DEBUG 4

int8_t valorConfig[NUM_MODOS_DEBUG] = {0, 0, 0, 0};
#define NUM_VALORES 9

/**
 * @brief Indicar el tipo de menú que está actualmente activo mediante el led de estado
 * 
 */
static void handle_menu_mode() {
  switch (modoConfig) {
    case MODE_NOTHING:
      set_status_led(false);
      break;
    case MODE_SPEED:
      warning_status_led(50);
      break;
    case MODE_FANS:
      warning_status_led(200);
      break;
    case MODE_DEBUG:
      set_status_led(true);
      break;
  }
}

static void handle_menu_value() {
  switch (modoConfig) {
    case MODE_NOTHING:
      set_RGB_color(0, 0, 0);
      break;
    case MODE_SPEED:
      switch (valorConfig[modoConfig]) {
        case 0:
          set_RGB_color(0, 100, 100); // Cian
          break;
        case 1:
          set_RGB_color(0, 100, 0); // Verde ↓
          break;
        case 2:
          set_RGB_color(0, 255, 0); // Verde ↑
          break;
        case 3:
          set_RGB_color(100, 100, 0); // Amarillo ↓
          break;
        case 4:
          set_RGB_color(255, 225, 0); // Amarillo ↑
          break;
        case 5:
          set_RGB_color(100, 0, 0); // Rojo ↓
          break;
        case 6:
          set_RGB_color(255, 0, 0); // Rojo ↑
          break;
        case 7:
          set_RGB_color(100, 0, 100); // Haki ↓
          break;
        case 8:
          set_RGB_color(255, 0, 255); // Haki ↑
          break;
        case 9:
          set_RGB_color(255, 255, 255); // Party Mode
          break;
      }
      break;
    case MODE_FANS:
      switch (valorConfig[modoConfig]) {
        case 0:
          set_RGB_color(0, 10, 10);
          break;
        case 1:
          set_RGB_color(0, 10, 0);
          break;
        case 2:
          set_RGB_color(0, 255, 0);
          break;
        case 3:
          set_RGB_color(10, 10, 0);
          break;
        case 4:
          set_RGB_color(255, 225, 0);
          break;
        case 5:
          set_RGB_color(10, 0, 0);
          break;
        case 6:
          set_RGB_color(255, 0, 0);
          break;
        case 7:
          set_RGB_color(10, 0, 10);
          break;
        case 8:
          set_RGB_color(255, 0, 255);
          break;
        case 9:
          set_RGB_color(255, 255, 255);
          break;
      }
      break;
    case MODE_DEBUG:
      switch (valorConfig[modoConfig]) {
        case 0:
          set_RGB_color(0, 10, 10);
          break;
        case 1:
          set_RGB_color(0, 10, 0);
          break;
        case 2:
          set_RGB_color(0, 255, 0);
          break;
        case 3:
          set_RGB_color(10, 10, 0);
          break;
        case 4:
          set_RGB_color(255, 225, 0);
          break;
        case 5:
          set_RGB_color(10, 0, 0);
          break;
        case 6:
          set_RGB_color(255, 0, 0);
          break;
        case 7:
          set_RGB_color(10, 0, 10);
          break;
        case 8:
          set_RGB_color(255, 0, 255);
          break;
        case 9:
          set_RGB_color(255, 255, 255);
          break;
      }
        debug_from_config(valorConfig[modoConfig]);
      break;
  }
}

static uint8_t get_num_modos() {
  if (get_config_run() == CONFIG_RUN_RACE) {
    return NUM_MODOS_RACE; // NOTHING - VELOCIDAD - VENTILADORES
  } else {
    return NUM_MODOS_DEBUG; // NOTHING - VELOCIDAD - VENTILADORES - DEBUG
  }
}

void check_menu_button() {
  handle_menu_mode();
  if (in_debug_mode()) {
    handle_menu_value();
  }

  // Comprueba cambios del modo de configuración
  if (get_menu_mode_btn()) {
    modoConfig++;
    if (modoConfig >= get_num_modos()) {
      modoConfig = 0;
    }
    // Mejora detección de botón en pulsaciones repetidas
    while (get_menu_mode_btn()) {
      handle_menu_mode();
      handle_menu_value();
    };
    delay(50);
  }

  if (modoConfig == 0) {
    set_RGB_color(0, 0, 0);
    delay(50);
    handle_menu_value();
    return;
  }

  // Comprueba aumento de valor de configuración
  if (get_menu_up_btn()) {
    valorConfig[modoConfig]++;
    if (valorConfig[modoConfig] > NUM_VALORES) {
      valorConfig[modoConfig] = NUM_VALORES;
    }
    // Mejora detección de botón en pulsaciones repetidas
    while (get_menu_up_btn()) {
      handle_menu_value();
      handle_menu_mode();
    };
    if (!in_debug_mode()) {
      set_RGB_color(0, 0, 0);
    }
    delay(50);
  }

  // Comprueba descenso de valor de configuración
  if (get_menu_down_btn()) {
    valorConfig[modoConfig]--;
    if (valorConfig[modoConfig] < 0) {
      valorConfig[modoConfig] = 0;
    }
    // Mejora detección de botón en pulsaciones repetidas
    while (get_menu_down_btn()) {
      handle_menu_value();
      handle_menu_mode();
    };
    if (!in_debug_mode()) {
      set_RGB_color(0, 0, 0);
    }
    delay(50);
  }
}

bool in_debug_mode() {
  return modoConfig == MODE_DEBUG;
}
