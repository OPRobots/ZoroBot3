/**
    Basado en https://clearwater.com.au/code/rc5
 */
#include <rc5.h>

enum RC5_TIMMINGS {
  MIN_SHORT = 444,
  MAX_SHORT = 1333,
  MIN_LONG = 1334,
  MAX_LONG = 2222
};

enum RC5_EVENT {
  EVENT_SHORTSPACE = 0,
  EVENT_SHORTPULSE = 2,
  EVENT_LONGSPACE = 4,
  EVENT_LONGPULSE = 6
};

enum RC5_STATE {
  STATE_START1 = 0,
  STATE_MID1 = 1,
  STATE_MID0 = 2,
  STATE_START0 = 3
};

enum RC5_MANAGE {
  S2_MASK = 0x1000,
  S2_SHIFT = 12,
  TOGGLE_MASK = 0x0800,
  TOGGLE_SHIFT = 11,
  ADDRESS_MASK = 0x7C0,
  ADDRESS_SHIFT = 6,
  COMMAND_MASK = 0x003F,
  COMMAND_SHIFT = 0
};

enum RC5_ADDRESS {
  ADDRESS_PROG = 0x0B,
  ADDRESS_COMP = 0x07,
  ADDRESS_MENU = 0x1B,
};

enum RC5_CUSTOM_CMD {
  CUSTOM_CMD_MENU = 0x0D,
  CUSTOM_CMD_MENU_UP = 0x0E,
  CUSTOM_CMD_MENU_DOWN = 0x0C,
};

static int16_t rc5_stored_data[RC5_DATA_LENGTH];
enum RC5_DATA_INDEX {
  DATA_STOP = 0,
  DATA_START = 1,
  DATA_MENU = 2,
  DATA_MENU_UP = 3,
  DATA_MENU_DOWN = 4,
};

static const uint8_t trans[] = {0x01, 0x91, 0x9B, 0xFB};

static uint8_t state = STATE_MID1;
static uint16_t cmd = 1;
static uint8_t bits = 1;
static uint32_t last_us = 0;

static void rc5_manage_command(uint16_t message) {
  // unsigned char toggle = (message & TOGGLE_MASK) >> TOGGLE_SHIFT;
  unsigned char address = (message & ADDRESS_MASK) >> ADDRESS_SHIFT;
  unsigned char command = (message & COMMAND_MASK) >> COMMAND_SHIFT;
  // printf("RC5: %d %d\n", address, command);
  switch (address) {
    case ADDRESS_PROG:
      rc5_stored_data[DATA_STOP] = command;
      rc5_stored_data[DATA_START] = command + DATA_START;
      rc5_stored_data[DATA_MENU] = command;
      rc5_stored_data[DATA_MENU_UP] = command + 1;
      rc5_stored_data[DATA_MENU_DOWN] = command + 2;
      eeprom_set_data(DATA_INDEX_RC5, rc5_stored_data, RC5_DATA_LENGTH);
      eeprom_save();
      break;
    case ADDRESS_COMP:
      if (command == rc5_stored_data[DATA_START]) {
        if(menu_run_can_start()){
        // set_competicion_iniciando(true);
        }else{
          set_debug_btn(true);
        }
      } else if (command == rc5_stored_data[DATA_STOP]) {
        set_race_started(false);
      }
      break;
    case ADDRESS_MENU:
      if (command == CUSTOM_CMD_MENU || command == rc5_stored_data[DATA_MENU]) {
        menu_rc5_mode_change();
      } else if (command == CUSTOM_CMD_MENU_UP || command == rc5_stored_data[DATA_MENU_UP]) {
        menu_rc5_up();
      } else if (command == CUSTOM_CMD_MENU_DOWN || command == rc5_stored_data[DATA_MENU_DOWN]) {
        menu_rc5_down();
      }
      break;
  }
}

static void reset(void) {
  state = STATE_MID1;
  cmd = 1;
  bits = 1;
  last_us = get_us_counter();
}

static void rc5_decode_event(enum RC5_EVENT event) {

  unsigned char newState = (trans[state] >> event) & 0x3;
  if (newState == state) {
    reset();
  } else {
    state = newState;
    if (newState == STATE_MID0) {
      cmd = (cmd << 1);
      bits++;
    } else if (newState == STATE_MID1) {
      cmd = (cmd << 1) + 1;
      bits++;
    }
  }
}

static void rc5_decode_pulse(enum RC5_TRIGGER trigger, uint32_t elapsed) {

  if (elapsed >= MIN_SHORT && elapsed <= MAX_SHORT) {
    rc5_decode_event(trigger == RC5_TRIGGER_FALLING ? EVENT_SHORTSPACE : EVENT_SHORTPULSE);
  } else if (elapsed >= MIN_LONG && elapsed <= MAX_LONG) {
    rc5_decode_event(trigger == RC5_TRIGGER_FALLING ? EVENT_LONGSPACE : EVENT_LONGPULSE);
  } else {
    reset();
  }
}

void rc5_load_eeprom(void) {
  int16_t *eeprom_data = eeprom_get_data();
  for (uint16_t i = DATA_INDEX_RC5; i < (DATA_INDEX_RC5 + RC5_DATA_LENGTH); i++) {
    rc5_stored_data[i - DATA_INDEX_RC5] = eeprom_data[i];
  }
}

void rc5_register(enum RC5_TRIGGER trigger) {
  uint32_t us = get_us_counter();
  rc5_decode_pulse(trigger, us - last_us);
  last_us = us;

  if (bits == 14) {
    rc5_manage_command(cmd);
    cmd = 0;
    bits = 0;
  }
}