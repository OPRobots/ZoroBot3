#include <sensors.h>

static bool sensors_enabled = false;

static bool sensors_taking_values = false;
static uint32_t sensors_take_value_ms = 0;

static uint8_t emitter_status = 1;
static uint8_t sensor_index = SENSOR_FRONT_LEFT_WALL_ID;

static uint8_t aux_adc_channels[NUM_AUX_ADC_CHANNELS] = {
    ADC_CHANNEL4,
    ADC_CHANNEL5,
    ADC_CHANNEL6,
    ADC_CHANNEL7,
};
static volatile uint16_t aux_adc_raw[NUM_AUX_ADC_CHANNELS];

static uint8_t sensores[NUM_SENSORES] = {
    ADC_CHANNEL10, // DETECTA SENSOR_FRONT_LEFT_WALL_ID - NO CAMBIAR
    ADC_CHANNEL13, // DETECTA SENSOR_FRONT_RIGHT_WALL_ID - NO CAMBIAR
    ADC_CHANNEL12, // DETECTA SENSOR_SIDE_LEFT_WALL_ID - NO CAMBIAR
    ADC_CHANNEL11, // DETECTA SENSOR_SIDE_RIGHT_WALL_ID - NO CAMBIAR
};
static volatile uint16_t sensors_raw[NUM_SENSORES];
static volatile uint16_t sensors_off[NUM_SENSORES];
static volatile uint16_t sensors_on[NUM_SENSORES];

volatile uint16_t sensors_distance[NUM_SENSORES];
int16_t sensors_distance_offset[NUM_SENSORES] = {0, 0, 0, 0};

struct sensors_distance_calibration sensors_distance_calibrations[] = {{}, {}, {}, {}};

static int16_t sensors_raw_wall_detection_threshold[NUM_SENSORES] = {0, 0, 0, 0};
static int16_t sensors_middle_target_distance[NUM_SENSORES] = {0, 0, 0, 0};

#define WALL_DETECT_CONFIRM_COUNT 4
#define WALL_DETECT_RELEASE_COUNT 6

static int8_t wall_counter[NUM_SENSORES] = {0, 0, 0, 0};
static bool wall_present[NUM_SENSORES] = {false, false, false, false};

#define RAW_MEDIAN_SIZE 3
static uint16_t raw_median[NUM_SENSORES][RAW_MEDIAN_SIZE];
static uint8_t raw_median_index[NUM_SENSORES];

static uint16_t triplet_median(uint16_t a, uint16_t b, uint16_t c) {
  uint16_t t;
  if (a > b) {
    t = a;
    a = b;
    b = t;
  }
  if (b > c) {
    t = b;
    b = c;
    c = t;
  }
  if (a > b) {
    t = a;
    a = b;
    b = t;
  }
  return b;
}

void set_sensors_robot_calibration(uint16_t version) {
  switch (version) {
    case ZOROBOT3_A:
      sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].a = 2.792;
      sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].b = 0.323;
      sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].c = 55.097;

      sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].a = 2.606;
      sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].b = 0.304;
      sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].c = 27.843;

      sensors_distance_calibrations[SENSOR_SIDE_LEFT_WALL_ID].a = 2.240;
      sensors_distance_calibrations[SENSOR_SIDE_LEFT_WALL_ID].b = 0.284;
      sensors_distance_calibrations[SENSOR_SIDE_LEFT_WALL_ID].c = -4.468;

      sensors_distance_calibrations[SENSOR_SIDE_RIGHT_WALL_ID].a = 2.335;
      sensors_distance_calibrations[SENSOR_SIDE_RIGHT_WALL_ID].b = 0.300;
      sensors_distance_calibrations[SENSOR_SIDE_RIGHT_WALL_ID].c = 31.534;
      break;
    case ZOROBOT3_B:
      sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].a = 2.932;
      sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].b = 0.341;
      sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].c = 14.763;

      sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].a = 2.796;
      sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].b = 0.332;
      sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].c = 22.458;

      sensors_distance_calibrations[SENSOR_SIDE_LEFT_WALL_ID].a = 2.175;
      sensors_distance_calibrations[SENSOR_SIDE_LEFT_WALL_ID].b = 0.273;
      sensors_distance_calibrations[SENSOR_SIDE_LEFT_WALL_ID].c = 28.662;

      sensors_distance_calibrations[SENSOR_SIDE_RIGHT_WALL_ID].a = 2.384;
      sensors_distance_calibrations[SENSOR_SIDE_RIGHT_WALL_ID].b = 0.305;
      sensors_distance_calibrations[SENSOR_SIDE_RIGHT_WALL_ID].c = -8.160;
      break;
    case ZOROBOT3_C:
      sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].a = 2.717;
      sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].b = 0.321;
      sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].c = 34.784;

      sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].a = 2.992;
      sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].b = 0.355;
      sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].c = 37.060;

      sensors_distance_calibrations[SENSOR_SIDE_LEFT_WALL_ID].a = 1.900;
      sensors_distance_calibrations[SENSOR_SIDE_LEFT_WALL_ID].b = 0.247;
      sensors_distance_calibrations[SENSOR_SIDE_LEFT_WALL_ID].c = -0.844;

      sensors_distance_calibrations[SENSOR_SIDE_RIGHT_WALL_ID].a = 2.300;
      sensors_distance_calibrations[SENSOR_SIDE_RIGHT_WALL_ID].b = 0.295;
      sensors_distance_calibrations[SENSOR_SIDE_RIGHT_WALL_ID].c = 35.468;
      break;
  }
}

uint8_t *get_aux_adc_channels(void) {
  return aux_adc_channels;
}

uint8_t get_aux_adc_channels_num(void) {
  return NUM_AUX_ADC_CHANNELS;
}

volatile uint16_t *get_aux_adc_raw(void) {
  return aux_adc_raw;
}

uint16_t get_aux_raw(uint8_t pos) {
  return aux_adc_raw[pos];
}

/**
 * @brief Set an specific emitter ON.
 *
 * @param[in] emitter Emitter type.
 */
#ifndef MMSIM_ENABLED
static void set_emitter_on(uint8_t emitter) {
  switch (emitter) {
    case SENSOR_FRONT_LEFT_WALL_ID:
      gpio_set(GPIOA, GPIO0);
      break;
    case SENSOR_FRONT_RIGHT_WALL_ID:
      gpio_set(GPIOA, GPIO3);
      break;
    case SENSOR_SIDE_RIGHT_WALL_ID:
      gpio_set(GPIOA, GPIO1);
      break;
    case SENSOR_SIDE_LEFT_WALL_ID:
      gpio_set(GPIOA, GPIO2);
      break;
    default:
      break;
  }
}
#endif

/**
 * @brief Set an specific emitter OFF.
 *
 * @param[in] emitter Emitter type.
 */
#ifndef MMSIM_ENABLED
static void set_emitter_off(uint8_t emitter) {
  switch (emitter) {
    case SENSOR_FRONT_LEFT_WALL_ID:
      gpio_clear(GPIOA, GPIO0);
      break;
    case SENSOR_FRONT_RIGHT_WALL_ID:
      gpio_clear(GPIOA, GPIO3);
      break;
    case SENSOR_SIDE_RIGHT_WALL_ID:
      gpio_clear(GPIOA, GPIO1);
      break;
    case SENSOR_SIDE_LEFT_WALL_ID:
      gpio_clear(GPIOA, GPIO2);
      break;
    default:
      break;
  }
}
#endif

void set_sensors_enabled(bool enabled) {
  if (!sensors_enabled && enabled) {
    emitter_status = 1;
    sensor_index = SENSOR_FRONT_LEFT_WALL_ID;
    for (uint8_t i = 0; i < NUM_SENSORES; i++) {
      wall_counter[i] = 0;
      wall_present[i] = false;
    }
  }
  sensors_enabled = enabled;
}

bool get_sensors_enabled(void) {
  return sensors_enabled;
}

bool is_sensors_taking_values(void) {
  return sensors_taking_values;
}

#ifndef MMSIM_ENABLED
void sensors_take_value(void) {
  if (sensors_take_value_ms == 0 || get_clock_ticks() - sensors_take_value_ms > 1000) {
    sensors_take_value_ms = get_clock_ticks();
  }
}
#endif

void get_sensors_raw(uint16_t *on, uint16_t *off) {
  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    on[i] = sensors_on[i];
    off[i] = sensors_off[i];
  }
}

uint8_t *get_sensors(void) {
  return sensores;
}

uint8_t get_sensors_num(void) {
  return NUM_SENSORES;
}

/**
 * @brief Máquina de estados de valores de sensores
 *
 */
void sm_emitter_adc(void) {
#ifndef MMSIM_ENABLED
  if (!sensors_enabled) {
    gpio_clear(GPIOA, GPIO0);
    gpio_clear(GPIOA, GPIO3);
    gpio_clear(GPIOA, GPIO1);
    gpio_clear(GPIOA, GPIO2);
    return;
  }

  switch (emitter_status) {
    case 1:
      sensors_off[sensor_index] = adc_read_injected(ADC2, (sensor_index + 1));
      set_emitter_on(sensor_index);
      emitter_status = 2;
      break;
    case 2:
      adc_start_conversion_injected(ADC2);
      emitter_status = 3;
      break;
    case 3:
      sensors_on[sensor_index] = adc_read_injected(ADC2, (sensor_index + 1));
      set_emitter_off(sensor_index);
      emitter_status = 4;
      break;
    case 4:
      adc_start_conversion_injected(ADC2);
      emitter_status = 1;
      if (sensor_index == (NUM_SENSORES - 1))
        sensor_index = 0;
      else
        sensor_index++;
      break;
    default:
      break;
  }
#endif
}

uint16_t get_sensor_raw(uint8_t pos, bool on) {
  if (pos < NUM_SENSORES) {
    return on ? sensors_on[pos] : sensors_off[pos];
  } else {
    return 0;
  }
}

uint16_t get_sensor_raw_filter(uint8_t pos) {
  if (pos < NUM_SENSORES) {
    if (sensors_on[pos] > sensors_off[pos]) {
      return sensors_on[pos] - sensors_off[pos];
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}

bool use_raw_sensors(void) {
#ifdef USE_RAW_SENSORS
  return true;
#else
  return false;
#endif
}

void front_sensors_calibration(void) {
#ifndef MMSIM_ENABLED
  int32_t left_raw_temp = 0;
  int32_t left_temp = 0;

  int32_t right_raw_temp = 0;
  int32_t right_temp = 0;

  set_sensors_enabled(true);
  sensors_distance_offset[SENSOR_FRONT_LEFT_WALL_ID] = 0;
  sensors_distance_offset[SENSOR_FRONT_RIGHT_WALL_ID] = 0;
  delay(5);

  for (int i = 0; i < SENSOR_FRONT_CALIBRATION_READINGS; i++) {
    left_raw_temp += get_sensor_raw_filter(SENSOR_FRONT_LEFT_WALL_ID);
    right_raw_temp += get_sensor_raw_filter(SENSOR_FRONT_RIGHT_WALL_ID);

    left_temp += sensors_distance[SENSOR_FRONT_LEFT_WALL_ID];
    right_temp += sensors_distance[SENSOR_FRONT_RIGHT_WALL_ID];
    set_leds_wave(35);
    delay(5);
  }
  set_info_leds();
  sensors_raw_wall_detection_threshold[SENSOR_FRONT_LEFT_WALL_ID] = (uint16_t)(left_raw_temp / SENSOR_FRONT_CALIBRATION_READINGS / (SENSOR_RAW_THRESHOLD_DISTANCE_FACTOR * SENSOR_RAW_THRESHOLD_DISTANCE_FACTOR));
  sensors_raw_wall_detection_threshold[SENSOR_FRONT_RIGHT_WALL_ID] = (uint16_t)(right_raw_temp / SENSOR_FRONT_CALIBRATION_READINGS / (SENSOR_RAW_THRESHOLD_DISTANCE_FACTOR * SENSOR_RAW_THRESHOLD_DISTANCE_FACTOR));

  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    printf("Sensor %d raw threshold: %d\n", i, sensors_raw_wall_detection_threshold[i]);
  }

  sensors_distance_offset[SENSOR_FRONT_LEFT_WALL_ID] = (int16_t)((CELL_DIMENSION - WALL_WIDTH / 2) - (left_temp / SENSOR_FRONT_CALIBRATION_READINGS));
  sensors_distance_offset[SENSOR_FRONT_RIGHT_WALL_ID] = (int16_t)((CELL_DIMENSION - WALL_WIDTH / 2) - (right_temp / SENSOR_FRONT_CALIBRATION_READINGS));

  set_sensors_enabled(false);
  delay(500);
  clear_info_leds();
  eeprom_set_data(DATA_INDEX_SENSORS_OFFSETS, sensors_distance_offset, NUM_SENSORES);
  eeprom_set_data(DATA_INDEX_SENSORS_RAW_THRESHOLDS, sensors_raw_wall_detection_threshold, NUM_SENSORES);
#endif
}

void front_sensors_middle_calibration(void) {
#ifndef MMSIM_ENABLED
  int32_t left_raw_temp = 0;
  int32_t left_temp = 0;

  int32_t right_raw_temp = 0;
  int32_t right_temp = 0;

  set_sensors_enabled(true);
  delay(5);

  for (int i = 0; i < SENSOR_FRONT_CALIBRATION_READINGS; i++) {
    left_raw_temp += get_sensor_raw_filter(SENSOR_FRONT_LEFT_WALL_ID);
    right_raw_temp += get_sensor_raw_filter(SENSOR_FRONT_RIGHT_WALL_ID);

    left_temp += sensors_distance[SENSOR_FRONT_LEFT_WALL_ID];
    right_temp += sensors_distance[SENSOR_FRONT_RIGHT_WALL_ID];
    set_leds_wave(35);
    delay(5);
  }
  set_info_leds();
  sensors_middle_target_distance[SENSOR_FRONT_LEFT_WALL_ID] = (uint16_t)(left_temp / SENSOR_FRONT_CALIBRATION_READINGS);
  sensors_middle_target_distance[SENSOR_FRONT_RIGHT_WALL_ID] = (uint16_t)(right_temp / SENSOR_FRONT_CALIBRATION_READINGS);

  sensors_middle_target_distance[SENSOR_FRONT_LEFT_WALL_ID + 2] = (uint16_t)(left_raw_temp / SENSOR_FRONT_CALIBRATION_READINGS);
  sensors_middle_target_distance[SENSOR_FRONT_RIGHT_WALL_ID + 2] = (uint16_t)(right_raw_temp / SENSOR_FRONT_CALIBRATION_READINGS);

  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    if (i <= 1) {
      printf("Sensor %d middle target_distance: %d\n", i, sensors_middle_target_distance[i]);
    } else {
      printf("Sensor %d middle target_raw: %d\n", (i - 2), sensors_middle_target_distance[i]);
    }
  }

  set_sensors_enabled(false);
  delay(500);
  clear_info_leds();
  eeprom_set_data(DATA_INDEX_SENSORS_MIDDLE_TARGET_DISTANCE, sensors_middle_target_distance, NUM_SENSORES);
#endif
}

void side_sensors_calibration(bool keep_sensors_on) {
#ifndef MMSIM_ENABLED
  int32_t left_raw_temp = 0;
  int32_t left_temp = 0;

  int32_t right_raw_temp = 0;
  int32_t right_temp = 0;

  set_sensors_enabled(true);
  sensors_distance_offset[SENSOR_SIDE_LEFT_WALL_ID] = 0;
  sensors_distance_offset[SENSOR_SIDE_RIGHT_WALL_ID] = 0;
  delay(5);

  for (int i = 0; i < SENSOR_SIDE_CALIBRATION_READINGS; i++) {
    left_raw_temp += get_sensor_raw_filter(SENSOR_SIDE_LEFT_WALL_ID);
    right_raw_temp += get_sensor_raw_filter(SENSOR_SIDE_RIGHT_WALL_ID);
    left_temp += sensors_distance[SENSOR_SIDE_LEFT_WALL_ID];
    right_temp += sensors_distance[SENSOR_SIDE_RIGHT_WALL_ID];
    set_leds_wave(35);
    delay(5);
  }
  set_info_leds();
  sensors_raw_wall_detection_threshold[SENSOR_SIDE_LEFT_WALL_ID] = (uint16_t)(left_raw_temp / SENSOR_SIDE_CALIBRATION_READINGS / (SENSOR_RAW_THRESHOLD_DISTANCE_FACTOR * SENSOR_RAW_THRESHOLD_DISTANCE_FACTOR));
  sensors_raw_wall_detection_threshold[SENSOR_SIDE_RIGHT_WALL_ID] = (uint16_t)(right_raw_temp / SENSOR_SIDE_CALIBRATION_READINGS / (SENSOR_RAW_THRESHOLD_DISTANCE_FACTOR * SENSOR_RAW_THRESHOLD_DISTANCE_FACTOR));

  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    printf("Sensor %d raw threshold: %d\n", i, sensors_raw_wall_detection_threshold[i]);
  }

  sensors_distance_offset[SENSOR_SIDE_LEFT_WALL_ID] = (int16_t)(MIDDLE_MAZE_DISTANCE - (left_temp / SENSOR_SIDE_CALIBRATION_READINGS));
  sensors_distance_offset[SENSOR_SIDE_RIGHT_WALL_ID] = (int16_t)(MIDDLE_MAZE_DISTANCE - (right_temp / SENSOR_SIDE_CALIBRATION_READINGS));

  if (!keep_sensors_on) {
    set_sensors_enabled(false);
  }
  delay(500);
  clear_info_leds();
  eeprom_set_data(DATA_INDEX_SENSORS_OFFSETS, sensors_distance_offset, NUM_SENSORES);
  eeprom_set_data(DATA_INDEX_SENSORS_RAW_THRESHOLDS, sensors_raw_wall_detection_threshold, NUM_SENSORES);
#endif
}

#ifndef MMSIM_ENABLED
void all_sensors_take_values(uint8_t sensor) {
  sensors_taking_values = true;
  set_sensors_enabled(true);
  delay(100);
  do {
    if (get_clock_ticks() - sensors_take_value_ms < 1000) {

      uint32_t sum_raw_filter = 0;
      for (uint8_t i = 0; i < 5; i++) {
        uint16_t s = get_sensor_raw_filter(sensor);
        sum_raw_filter += s;
        // printf("r: %d\n", s);
        delay(50);
      }
      printf("S: %ld\n", sum_raw_filter / 5);
      delay(1000);
    }
  } while (true);
}
#endif

void sensors_load_eeprom(void) {
#ifndef MMSIM_ENABLED
  int16_t *data = eeprom_get_data();
  for (uint16_t i = DATA_INDEX_SENSORS_OFFSETS; i < (DATA_INDEX_SENSORS_OFFSETS + NUM_SENSORES); i++) {
    sensors_distance_offset[i - DATA_INDEX_SENSORS_OFFSETS] = data[i];
    printf("Sensor %d offset: %d\n", i - DATA_INDEX_SENSORS_OFFSETS, sensors_distance_offset[i - DATA_INDEX_SENSORS_OFFSETS]);
  }
  for (uint16_t i = DATA_INDEX_SENSORS_RAW_THRESHOLDS; i < (DATA_INDEX_SENSORS_RAW_THRESHOLDS + NUM_SENSORES); i++) {
    sensors_raw_wall_detection_threshold[i - DATA_INDEX_SENSORS_RAW_THRESHOLDS] = data[i];
    printf("Sensor %d raw threshold: %d\n", i - DATA_INDEX_SENSORS_RAW_THRESHOLDS, sensors_raw_wall_detection_threshold[i - DATA_INDEX_SENSORS_RAW_THRESHOLDS]);
  }
  for (uint16_t i = DATA_INDEX_SENSORS_MIDDLE_TARGET_DISTANCE; i < (DATA_INDEX_SENSORS_MIDDLE_TARGET_DISTANCE + NUM_SENSORES); i++) {
    sensors_middle_target_distance[i - DATA_INDEX_SENSORS_MIDDLE_TARGET_DISTANCE] = data[i];
    printf("Sensor %d raw middle position: %d\n", i - DATA_INDEX_SENSORS_MIDDLE_TARGET_DISTANCE, sensors_middle_target_distance[i - DATA_INDEX_SENSORS_MIDDLE_TARGET_DISTANCE]);
  }
#endif
}

bool left_wall_detection(void) {
  return wall_present[SENSOR_SIDE_LEFT_WALL_ID];
}

bool right_wall_detection(void) {
  return wall_present[SENSOR_SIDE_RIGHT_WALL_ID];
}

bool front_wall_detection(void) {
  return wall_present[SENSOR_FRONT_LEFT_WALL_ID] || wall_present[SENSOR_FRONT_RIGHT_WALL_ID];
}

/**
 * @brief Las magias de los sensores vienen dadas por un script de python que calcula los valores ABC de la ecuación que los caracteriza.
 * TODO: A mayores, se aplica un offset al valor calculado de las distancias, calibrado mediante menú.
 */
void update_sensors_magics(void) {
  if (!sensors_enabled) {
    return;
  }
  for (uint8_t sensor = 0; sensor < NUM_SENSORES; sensor++) {
    if (sensors_on[sensor] > sensors_off[sensor]) {
      sensors_raw[sensor] = sensors_on[sensor] - sensors_off[sensor];

      if (raw_median[sensor][0] == 0) {
        raw_median[sensor][0] = sensors_raw[sensor];
        raw_median[sensor][1] = sensors_raw[sensor];
        raw_median[sensor][2] = sensors_raw[sensor];
      }
      raw_median[sensor][raw_median_index[sensor]] = sensors_raw[sensor];
      raw_median_index[sensor] = (raw_median_index[sensor] + 1) % RAW_MEDIAN_SIZE;

      uint16_t raw_filtered = triplet_median(raw_median[sensor][0], raw_median[sensor][1], raw_median[sensor][2]);

      int16_t ln_index = (raw_filtered + sensors_distance_calibrations[sensor].c) / 4;
      if (ln_index < 0) {
        ln_index = 0;
      }
      float ln = get_ln_value(ln_index);
      int16_t new_sensor_distance = (int16_t)((sensors_distance_calibrations[sensor].a / ln - sensors_distance_calibrations[sensor].b) * 1000.0f);
      if (new_sensor_distance < 0) {
        new_sensor_distance = 0;
      }

      switch (sensor) {
        case SENSOR_FRONT_LEFT_WALL_ID:
        case SENSOR_FRONT_RIGHT_WALL_ID:
          new_sensor_distance += ROBOT_FRONT_LENGTH;
          break;
        case SENSOR_SIDE_LEFT_WALL_ID:
        case SENSOR_SIDE_RIGHT_WALL_ID:
          new_sensor_distance += ROBOT_MIDDLE_WIDTH;
          break;
      }
      new_sensor_distance += sensors_distance_offset[sensor];

      float delta = new_sensor_distance - sensors_distance[sensor];
      if (delta < 0.0f) {
        delta = -delta;
      }
      float alpha = (delta > 10.0f) ? 0.6f : 0.2f;
      sensors_distance[sensor] = (uint16_t)(alpha * new_sensor_distance + (1.0f - alpha) * sensors_distance[sensor]);
    }

    bool detected = false;
    if (use_raw_sensors()) {
      detected = get_sensor_raw_filter(sensor) > (uint16_t)sensors_raw_wall_detection_threshold[sensor];
    } else {
      switch (sensor) {
        case SENSOR_FRONT_LEFT_WALL_ID:
        case SENSOR_FRONT_RIGHT_WALL_ID:
          detected = sensors_distance[sensor] < (uint16_t)SENSOR_FRONT_DETECTION;
          break;
        default:
          detected = sensors_distance[sensor] < SENSOR_SIDE_DETECTION;
          break;
      }
    }

    if (detected) {
      if (wall_counter[sensor] < WALL_DETECT_RELEASE_COUNT) {
        wall_counter[sensor]++;
      }
    } else if (wall_counter[sensor] > 0) {
      wall_counter[sensor]--;
    }

    if (!wall_present[sensor]) {
      if (wall_counter[sensor] >= WALL_DETECT_CONFIRM_COUNT) {
        wall_present[sensor] = true;
      }
    } else if (wall_counter[sensor] <= 0) {
      wall_present[sensor] = false;
    }
  }
}

void update_side_sensors_leds(void) {
#ifndef MMSIM_ENABLED
  int16_t side_error_leds = get_side_sensors_error();
  if (abs(side_error_leds) < 2) {
    clear_info_leds();
  } else if (side_error_leds >= 20) {
    set_info_led(0, true);
    set_info_led(1, false);
    set_info_led(2, false);
    set_info_led(3, false);
    set_info_led(4, false);
    set_info_led(5, false);
    set_info_led(6, false);
    set_info_led(7, false);
  } else if (side_error_leds >= 10) {
    set_info_led(0, false);
    set_info_led(1, true);
    set_info_led(2, false);
    set_info_led(3, false);
    set_info_led(4, false);
    set_info_led(5, false);
    set_info_led(6, false);
    set_info_led(7, false);
  } else if (side_error_leds >= 5) {
    set_info_led(0, false);
    set_info_led(1, false);
    set_info_led(2, true);
    set_info_led(3, false);
    set_info_led(4, false);
    set_info_led(5, false);
    set_info_led(6, false);
    set_info_led(7, false);
  } else if (side_error_leds >= 0) {
    set_info_led(0, false);
    set_info_led(1, false);
    set_info_led(2, false);
    set_info_led(3, true);
    set_info_led(4, false);
    set_info_led(5, false);
    set_info_led(6, false);
    set_info_led(7, false);
  } else if (side_error_leds <= -20) {
    set_info_led(0, false);
    set_info_led(1, false);
    set_info_led(2, false);
    set_info_led(3, false);
    set_info_led(4, false);
    set_info_led(5, false);
    set_info_led(6, false);
    set_info_led(7, true);
  } else if (side_error_leds <= -10) {
    set_info_led(0, false);
    set_info_led(1, false);
    set_info_led(2, false);
    set_info_led(3, false);
    set_info_led(4, false);
    set_info_led(5, false);
    set_info_led(6, true);
    set_info_led(7, false);
  } else if (side_error_leds <= -5) {
    set_info_led(0, false);
    set_info_led(1, false);
    set_info_led(2, false);
    set_info_led(3, false);
    set_info_led(4, false);
    set_info_led(5, true);
    set_info_led(6, false);
    set_info_led(7, false);
  } else if (side_error_leds <= 0) {
    set_info_led(0, false);
    set_info_led(1, false);
    set_info_led(2, false);
    set_info_led(3, false);
    set_info_led(4, true);
    set_info_led(5, false);
    set_info_led(6, false);
    set_info_led(7, false);
  }
#endif
}

uint16_t get_sensor_distance(uint8_t pos) {
  return sensors_distance[pos];
}

uint16_t get_front_wall_middle_target_distance(void) {
  if (use_raw_sensors()) {
    return (sensors_middle_target_distance[SENSOR_FRONT_LEFT_WALL_ID + 2] + sensors_middle_target_distance[SENSOR_FRONT_RIGHT_WALL_ID + 2]) / 2;
  } else {
    return get_front_wall_middle_target_distance_mm();
  }
}

uint16_t get_front_wall_middle_target_distance_mm(void) {
  return (sensors_middle_target_distance[SENSOR_FRONT_LEFT_WALL_ID] + sensors_middle_target_distance[SENSOR_FRONT_RIGHT_WALL_ID]) / 2;
}

uint16_t get_front_wall_distance(void) {
  if (use_raw_sensors()) {
    return (get_sensor_raw_filter(SENSOR_FRONT_LEFT_WALL_ID) + get_sensor_raw_filter(SENSOR_FRONT_RIGHT_WALL_ID)) / 2;
  } else {
    return get_front_wall_distance_mm();
  }
}

uint16_t get_front_wall_distance_mm(void) {
  return (sensors_distance[SENSOR_FRONT_LEFT_WALL_ID] + sensors_distance[SENSOR_FRONT_RIGHT_WALL_ID]) / 2;
}

struct walls get_walls(void) {
  struct walls walls;

#ifdef MMSIM_ENABLED
  walls.front = API_wallFront();
  walls.left = API_wallLeft();
  walls.right = API_wallRight();
  return walls;
#endif

  walls.front = front_wall_detection();
  walls.left = left_wall_detection();
  walls.right = right_wall_detection();
  return walls;
}

float get_side_sensors_error(void) {
  int16_t left_error = sensors_distance[SENSOR_SIDE_LEFT_WALL_ID] - MIDDLE_MAZE_DISTANCE;
  int16_t right_error = sensors_distance[SENSOR_SIDE_RIGHT_WALL_ID] - MIDDLE_MAZE_DISTANCE;

  if (sensors_distance[SENSOR_SIDE_LEFT_WALL_ID] < 90 && sensors_distance[SENSOR_SIDE_RIGHT_WALL_ID] < 90) {
    return right_error - left_error;
  } else if (sensors_distance[SENSOR_SIDE_LEFT_WALL_ID] < 90) {
    return -2 * left_error;
  } else if (sensors_distance[SENSOR_SIDE_RIGHT_WALL_ID] < 90) {
    return 2 * right_error;
  } else if (left_error > 100 && right_error < 40) {
    return 2 * right_error;
  } else if (right_error > 100 && left_error < 40) {
    return -2 * left_error;
  }
  return 0;
}

int16_t get_diagonal_sensors_error(void) {
  bool left_correction = sensors_distance[SENSOR_FRONT_LEFT_WALL_ID] < sensors_distance[SENSOR_FRONT_RIGHT_WALL_ID];

  if (left_correction && sensors_distance[SENSOR_FRONT_LEFT_WALL_ID] < SENSOR_DIAGONAL_REFERENCE_DISTANCE) {
    return -(sensors_distance[SENSOR_FRONT_LEFT_WALL_ID] - SENSOR_DIAGONAL_REFERENCE_DISTANCE);
  } else if (!left_correction && sensors_distance[SENSOR_FRONT_RIGHT_WALL_ID] < SENSOR_DIAGONAL_REFERENCE_DISTANCE) {
    return sensors_distance[SENSOR_FRONT_RIGHT_WALL_ID] - SENSOR_DIAGONAL_REFERENCE_DISTANCE;
  }
  return 0;
}

int16_t get_front_sensors_angle_error(void) {
  if (!front_wall_detection()) {
    return 0;
  }
  return sensors_distance[SENSOR_FRONT_LEFT_WALL_ID] - sensors_distance[SENSOR_FRONT_RIGHT_WALL_ID];
}

int16_t get_front_sensors_diagonal_error(void) {
  int16_t left_error = sensors_distance[SENSOR_FRONT_LEFT_WALL_ID] - SENSOR_DIAGONAL_REFERENCE_DISTANCE;
  int16_t right_error = sensors_distance[SENSOR_FRONT_RIGHT_WALL_ID] - SENSOR_DIAGONAL_REFERENCE_DISTANCE;
  // printf("\t\t%4d - %4d\n", left_error, right_error);

  if (right_error < 0) {
    return right_error;
  }
  if (left_error < 0) {
    return -left_error;
  }

  return 0;
}
