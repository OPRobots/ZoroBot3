#include <menu_run.h>

#define MODE_SPEED 0
#define MODE_RACE 1
#define MODE_MAZE_TYPE 2
#define MODE_SOLVE_STRATEGY 3
#define MODE_EXPLORE_ALGORITHM 4
uint8_t modeRun = MODE_SPEED;

#define MODE_SPEED_VALUES 6
#define MODE_RACE_VALUES 2
#define MODE_MAZE_TYPE_VALUES 2
#define MODE_EXPLORE_ALGORITHM_VALUES 3
#define MODE_SOLVE_STRATEGY_VALUES 2

int16_t valueRun[MENU_RUN_NUM_MODES] = {0, 0, 0, 0, 1};

uint32_t lastBlinkMs = 0;
bool blinkState = false;

static void handle_menu_run_values(void) {
  if (get_clock_ticks() - lastBlinkMs >= 125) {
    lastBlinkMs = get_clock_ticks();
    blinkState = !blinkState;
  }
  if (modeRun == MODE_SPEED) {
    set_RGB_color(0, 0, 0);

    if (valueRun[MODE_SPEED] == MODE_SPEED_VALUES - 1) {
      set_info_led(0, blinkState);
      set_info_led(1, blinkState);
      set_info_led(2, blinkState);
      set_info_led(3, blinkState);
      set_info_led(4, blinkState);
    } else {
      for (uint8_t i = 0; i < MODE_SPEED_VALUES - 1; i++) {
        if ((valueRun[MODE_SPEED] == i && blinkState)) {
          set_info_led(i, true);
        } else {
          set_info_led(i, false);
        }
      }
    }
  } else {
    if (valueRun[MODE_SPEED] == MODE_SPEED_VALUES - 1) {
      set_info_led(0, true);
      set_info_led(1, true);
      set_info_led(2, true);
      set_info_led(3, true);
      set_info_led(4, true);
    } else {
      for (uint8_t i = 0; i < MODE_SPEED_VALUES - 1; i++) {
        set_info_led(i, i == valueRun[MODE_SPEED]);
      }
    }
  }

  if (modeRun == MODE_RACE) {
    if (valueRun[modeRun] == 1) {
      set_RGB_color(50, 0, 0);
    } else {
      if (blinkState) {
        set_RGB_color(50, 0, 0);
      } else {
        set_RGB_color(0, 0, 0);
      }
    }
  }

  if (modeRun == MODE_MAZE_TYPE) {
    if (valueRun[modeRun] == 1) {
      set_RGB_color(0, 50, 0);
    } else {
      set_RGB_color(0, 0, 0);
    }
    set_info_led(7, blinkState);
  } else {
    set_info_led(7, valueRun[MODE_MAZE_TYPE] == 1);
  }

  if (modeRun == MODE_SOLVE_STRATEGY) {
    if (valueRun[modeRun] == 1) {
      set_RGB_color(0, 50, 0);
    } else {
      set_RGB_color(0, 0, 0);
    }
    set_info_led(8, blinkState);
  } else {
    set_info_led(8, valueRun[MODE_SOLVE_STRATEGY] == 1);
  }

  if (modeRun == MODE_EXPLORE_ALGORITHM) {
    switch (valueRun[modeRun]) {
      case EXPLORE_HANDWALL:
        set_RGB_color(0, 50, 0);
        break;
      case EXPLORE_FLOODFILL:
        set_RGB_color(0, 0, 50);
        break;
      case EXPLORE_TIME_TRIAL:
        set_RGB_color(50, 0, 50);
        break;
      default:
        set_RGB_color(0, 0, 0);
        break;
    }
    set_info_led(9, blinkState);
  } else {
    set_info_led(9, valueRun[MODE_EXPLORE_ALGORITHM] == 1);
  }
}

static void handle_menu_run_btn(void) {
  if (get_menu_up_btn()) {
    while (get_menu_up_btn()) {
      handle_menu_run_values();
    }
    menu_run_up();
  }

  if (get_menu_down_btn()) {
    while (get_menu_down_btn()) {
      handle_menu_run_values();
    }
    menu_run_down();
  }
}

bool menu_run_handler(void) {
  set_status_led(false);
  if (get_menu_mode_btn()) {
    uint32_t ms = get_clock_ticks();
    while (get_menu_mode_btn()) {
      if (get_clock_ticks() - ms >= 200) {
        warning_status_led(50);
      }
    }
    if (get_clock_ticks() - ms >= 200) {
      return true;
    } else {
      menu_run_mode_change();
    }
  }
  handle_menu_run_btn();
  handle_menu_run_values();
  return false;
}

void menu_run_reset(void) {
  modeRun = MODE_SPEED;
  valueRun[MODE_RACE] = 0;
}

void menu_run_load_values(void) {
  int16_t *data = eeprom_get_data();
  for (uint16_t i = DATA_INDEX_MENU_RUN; i < (DATA_INDEX_MENU_RUN + MENU_RUN_NUM_MODES); i++) {
    valueRun[i - DATA_INDEX_MENU_RUN] = data[i];
  }
  valueRun[MODE_RACE] = 0;
}

void menu_run_mode_change() {
  modeRun = (modeRun + 1) % MENU_RUN_NUM_MODES;
}

void menu_run_up() {
  uint8_t mode_values = 0;
  switch (modeRun) {
    case MODE_SPEED:
      mode_values = MODE_SPEED_VALUES;
      break;
    case MODE_MAZE_TYPE:
      mode_values = MODE_MAZE_TYPE_VALUES;
      break;
    case MODE_EXPLORE_ALGORITHM:
      mode_values = MODE_EXPLORE_ALGORITHM_VALUES;
      break;
    case MODE_RACE:
      mode_values = MODE_RACE_VALUES;
      break;
    case MODE_SOLVE_STRATEGY:
      mode_values = MODE_SOLVE_STRATEGY_VALUES;
      break;
  }
  valueRun[modeRun] = (valueRun[modeRun] + 1) % mode_values;
  if (modeRun == MODE_RACE && valueRun[modeRun] == 1) {
    set_RGB_color(50, 0, 0);
    eeprom_set_data(DATA_INDEX_MENU_RUN, valueRun, MENU_RUN_NUM_MODES);
    eeprom_save();
  }
}

void menu_run_down() {
  if (valueRun[modeRun] > 0) {
    valueRun[modeRun]--;
  }
}

bool menu_run_can_start(void) {
  return modeRun == MODE_RACE && valueRun[MODE_RACE] > 0;
}

int16_t *get_menu_run_values(void) {
  return valueRun;
}

enum speed_strategy menu_run_get_speed(void) {
  return valueRun[MODE_SPEED];
}

enum maze_type menu_run_get_maze_type(void) {
  return valueRun[MODE_MAZE_TYPE];
}

enum solve_strategy menu_run_get_solve_strategy(void) {
  return valueRun[MODE_SOLVE_STRATEGY];
}

enum explore_algorithm menu_run_get_explore_algorithm(void) {
  return valueRun[MODE_EXPLORE_ALGORITHM];
}