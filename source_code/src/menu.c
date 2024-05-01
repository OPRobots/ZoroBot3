#include <menu.h>

uint8_t modoConfig = 0;
#define MODE_CALIBRATION 0
#define MODE_DEBUG 1

#define NUM_MODOS_RACE 1
#define NUM_MODOS_DEBUG 2

int8_t valorConfig[NUM_MODOS_DEBUG] = {0, 0};
#define NUM_VALORES_CALIBRATION 5
#define NUM_VALORES 10

/**
 * @brief Indicar el tipo de menú que está actualmente activo mediante el led de estado
 *
 */
static void handle_menu_mode(void) {
  switch (modoConfig) {
    case MODE_CALIBRATION:
      set_status_led(false);
      break;
    case MODE_DEBUG:
      set_status_led(true);
      break;
  }
}

static void handle_menu_value(void) {
  switch (modoConfig) {
    case MODE_CALIBRATION:
      set_RGB_color(0, 0, 0);
      switch (valorConfig[modoConfig]) {
        case CALIBRATE_NONE:
          clear_info_leds();
          break;
        case CALIBRATE_GYRO_Z:
          set_leds_wave(120);
          break;
        case CALIBRATE_SIDE_SENSORS_OFFSET:
          set_leds_side_sensors(120);
          break;
        case CALIBRATE_FRONT_SENSORS:
          set_leds_front_sensors(120);
          break;
        case CALIBRATE_STORE_EEPROM:
          set_leds_blink(250);
          break;
      }
      calibrate_from_config(valorConfig[modoConfig]);
      break;
    case MODE_DEBUG:
      clear_info_leds();
      switch (valorConfig[modoConfig]) {
        case DEBUG_NONE:
          set_RGB_color(0, 0, 0);
          break;
        case DEBUG_TYPE_SENSORS_RAW:
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

static uint8_t get_num_modos(void) {
  if (get_config_run() == CONFIG_RUN_RACE) {
    return NUM_MODOS_RACE; // CALIBRATION
  } else {
    return NUM_MODOS_DEBUG; // CALIBRATION - DEBUG
  }
}

bool check_menu_button(void) {
  handle_menu_mode();
  handle_menu_value();

  // Comprueba cambios del modo de configuración
  if (!((modoConfig == MODE_CALIBRATION || modoConfig == MODE_DEBUG) && valorConfig[modoConfig] != 0)) {
    if (get_menu_mode_btn()) {
      modoConfig++;
      if (modoConfig >= get_num_modos()) {
        modoConfig = 0;
      }
      while (get_menu_mode_btn()) {
        handle_menu_mode();
        handle_menu_value();
      };
      delay(50);
    }
  }

  // Comprueba aumento de valor de configuración
  if (get_menu_up_btn()) {
    valorConfig[modoConfig]++;
    switch (modoConfig) {
      case MODE_CALIBRATION:
        if (valorConfig[modoConfig] >= NUM_VALORES_CALIBRATION) {
          valorConfig[modoConfig] = 0;
        }
        break;
      default:
        if (valorConfig[modoConfig] >= NUM_VALORES) {
          valorConfig[modoConfig] = 0;
        }
        break;
    }
    clear_info_leds();
    while (get_menu_up_btn()) {
      handle_menu_value();
      handle_menu_mode();
    };
    delay(50);
  }

  // Comprueba descenso de valor de configuración
  if (get_menu_down_btn()) {
    if (valorConfig[modoConfig] > 0) {
      valorConfig[modoConfig]--;
    }

    clear_info_leds();
    while (get_menu_down_btn()) {
      handle_menu_value();
      handle_menu_mode();
    };
    delay(50);
  }

  return valorConfig[modoConfig] != 0;
}

bool in_debug_mode(void) {
  return modoConfig == MODE_DEBUG;
}
