#include <sensors.h>

static uint8_t sensores[NUM_SENSORES] = {
    ADC_CHANNEL10, // DETECTA SENSOR_FRONT_LEFT_WALL_ID - NO CAMBIAR
    ADC_CHANNEL13, // DETECTA SENSOR_FRONT_RIGHT_WALL_ID - NO CAMBIAR
    ADC_CHANNEL12, // DETECTA SENSOR_SIDE_LEFT_WALL_ID - NO CAMBIAR
    ADC_CHANNEL11, // DETECTA SENSOR_SIDE_RIGHT_WALL_ID - NO CAMBIAR
};

static volatile uint16_t sensores_raw[NUM_SENSORES];

static volatile uint16_t sensors_off[NUM_SENSORES];
static volatile uint16_t sensors_on[NUM_SENSORES];
uint16_t sensors_filtered[NUM_SENSORES];

/**
 * @brief Set an specific emitter ON.
 *
 * @param[in] emitter Emitter type.
 */
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

/**
 * @brief Set an specific emitter OFF.
 *
 * @param[in] emitter Emitter type.
 */
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
  static uint8_t emitter_status = 1;
  static uint8_t sensor_index = SENSOR_FRONT_LEFT_WALL_ID;

  switch (emitter_status) {
    case 1:
      sensors_off[sensor_index] = adc_read_injected(ADC1, (sensor_index + 1));
      set_emitter_on(sensor_index);
      emitter_status = 2;
      break;
    case 2:
      adc_start_conversion_injected(ADC1);
      emitter_status = 3;
      break;
    case 3:
      sensors_on[sensor_index] = adc_read_injected(ADC1, (sensor_index + 1));
      set_emitter_off(sensor_index);
      emitter_status = 4;
      break;
    case 4:
      adc_start_conversion_injected(ADC1);
      emitter_status = 1;
      if (sensor_index == (NUM_SENSORES - 1))
        sensor_index = 0;
      else
        sensor_index++;
      break;
    default:
      break;
  }
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

void update_sensors_low_pass_filter(void) {
  for (uint8_t sensor = 0; sensor < NUM_SENSORES; sensor++) {
    if (sensors_on[sensor] > sensors_off[sensor]) {
      int16_t sensor_value = (sensors_on[sensor] - sensors_off[sensor])
      if (sensor_value < 0) {
        sensor_value = 0;
      }
      sensors_filtered[sensor] = SENSOR_LOW_PASS_FILTER_ALPHA * sensor_value + (1 - SENSOR_LOW_PASS_FILTER_ALPHA) * sensors_filtered[sensor];
    }
  }
}

uint16_t get_sensor_filtered(uint8_t pos) {
  return sensors_filtered[pos];
}