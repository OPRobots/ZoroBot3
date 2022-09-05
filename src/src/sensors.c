#include <sensors.h>

#define NUM_SENSORES 4

static uint8_t sensores[NUM_SENSORES] = {
    ADC_CHANNEL10, // Frontal Izquierdo
    ADC_CHANNEL13, // Frontal Derecho
    ADC_CHANNEL11, // Pared Derecha
    ADC_CHANNEL12, // Pared Izquierda
};

static volatile uint16_t sensores_raw[NUM_SENSORES];

static volatile uint16_t sensors_off[NUM_SENSORES];
static volatile uint16_t sensors_on[NUM_SENSORES];

uint8_t *get_sensors() {
  return sensores;
}

uint8_t get_sensors_num() {
  return NUM_SENSORES;
}

volatile uint16_t *get_sensors_raw() {
  return sensores_raw;
}

uint16_t get_sensor_raw(uint8_t pos, bool on) {
  if (pos < NUM_SENSORES) {
    return on ? sensors_on[pos] : sensors_off[pos];
  } else {
    return 0;
  }
}

/**
 * @brief Set an specific emitter ON.
 *
 * @param[in] emitter Emitter type.
 */
static void set_emitter_on(uint8_t emitter) {
  switch (emitter) {
    case SENSOR_SIDE_LEFT_ID:
      gpio_set(GPIOA, GPIO0);
      break;
    case SENSOR_SIDE_RIGHT_ID:
      gpio_set(GPIOA, GPIO3);
      break;
    case SENSOR_FRONT_LEFT_ID:
      gpio_set(GPIOA, GPIO1);
      break;
    case SENSOR_FRONT_RIGHT_ID:
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
    case SENSOR_SIDE_LEFT_ID:
      gpio_clear(GPIOA, GPIO0);
      break;
    case SENSOR_SIDE_RIGHT_ID:
      gpio_clear(GPIOA, GPIO3);
      break;
    case SENSOR_FRONT_LEFT_ID:
      gpio_clear(GPIOA, GPIO1);
      break;
    case SENSOR_FRONT_RIGHT_ID:
      gpio_clear(GPIOA, GPIO2);
      break;
    default:
      break;
  }
}

void sm_emitter_adc(void) {
  static uint8_t emitter_status = 1;
  static uint8_t sensor_index = SENSOR_SIDE_LEFT_ID;

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
