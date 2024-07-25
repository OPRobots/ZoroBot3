#include <menu_run.h>

#define MODE_SPEED 0
#define MODE_RACE 1
#define MODE_MAZE_TYPE 2
#define MODE_STRATEGY 3
uint8_t modeRun = MODE_SPEED;

#define MODE_SPEED_VALUES 4
#define MODE_RACE_VALUES 2
#define MODE_MAZE_TYPE_VALUES 2
#define MODE_STRATEGY_VALUES 2

uint16_t valueRun[MENU_RUN_NUM_MODES] = {0, 0, 0, 1};

uint32_t lastBlinkMs = 0;
bool blinkState = false;

static void handle_menu_run_values(void) {
  if (get_clock_ticks() - lastBlinkMs >= 125) {
    lastBlinkMs = get_clock_ticks();
    blinkState = !blinkState;
  }
  if (modeRun == MODE_SPEED) {
    set_RGB_color(0, 0, 0);
    for (uint8_t i = 0; i < 4; i++) {
      if (i < valueRun[MODE_SPEED] || (valueRun[MODE_SPEED] == i && blinkState)) {
        set_info_led(i, true);
      } else {
        set_info_led(i, false);
      }
    }
  } else {
    for (uint8_t i = 0; i < 4; i++) {
      set_info_led(i, i <= valueRun[MODE_SPEED]);
    }
  }

  if (modeRun == MODE_RACE) {
    if (valueRun[modeRun] == 1) {
      set_RGB_color(50, 0, 50);
    } else {
      set_RGB_color(50, 0, 0);
    }
    set_info_led(4, blinkState);
  } else {
    set_info_led(4, false);
  }

  if (modeRun == MODE_MAZE_TYPE) {
    if (valueRun[modeRun] == 1) {
      set_RGB_color(0, 50, 0);
    } else {
      set_RGB_color(0, 0, 0);
    }
    set_info_led(5, blinkState);
  } else {
    set_info_led(5, valueRun[MODE_MAZE_TYPE] == 1);
  }

  if (modeRun == MODE_STRATEGY) {
    if (valueRun[modeRun] == 1) {
      set_RGB_color(0, 50, 0);
    } else {
      set_RGB_color(0, 0, 0);
    }
    set_info_led(7, blinkState);
  } else {
    set_info_led(7, valueRun[MODE_STRATEGY] == 1);
  }
}

static void handle_menu_run_btn(void) {
  if (get_menu_up_btn()) {
    while (get_menu_up_btn()) {
      handle_menu_run_values();
    }
    uint8_t mode_values = 0;
    switch (modeRun) {
      case MODE_SPEED:
        mode_values = MODE_SPEED_VALUES;
        break;
      case MODE_MAZE_TYPE:
        mode_values = MODE_MAZE_TYPE_VALUES;
        break;
      case MODE_STRATEGY:
        mode_values = MODE_STRATEGY_VALUES;
        break;
      case MODE_RACE:
        mode_values = MODE_RACE_VALUES;
        break;
    }
    valueRun[modeRun] = (valueRun[modeRun] + 1) % mode_values;
  }

  if (get_menu_down_btn()) {
    while (get_menu_down_btn()) {
      handle_menu_run_values();
    }
    if (valueRun[modeRun] > 0) {
      valueRun[modeRun]--;
    }
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
      modeRun = (modeRun + 1) % MENU_RUN_NUM_MODES;
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
  uint16_t *data = eeprom_get_data();
  for (uint16_t i = DATA_INDEX_MENU_RUN; i < (DATA_INDEX_MENU_RUN + MENU_RUN_NUM_MODES); i++) {
    valueRun[i - DATA_INDEX_MENU_RUN] = data[i];
    printf("valueRun[%d] = %d\n", i - DATA_INDEX_MENU_RUN, valueRun[i - DATA_INDEX_MENU_RUN]);
  }
}

bool menu_run_can_start(void) {
  return modeRun == MODE_RACE && valueRun[MODE_RACE] > 0;
}

uint16_t *get_menu_run_values(void) {
  return valueRun;
}

enum speed_strategy menu_run_get_speed(void) {
  return valueRun[MODE_SPEED];
}

enum maze_type menu_run_get_maze_type(void) {
  return valueRun[MODE_MAZE_TYPE];
}

uint8_t menu_run_get_strategy(void) {
  return valueRun[MODE_STRATEGY];
}