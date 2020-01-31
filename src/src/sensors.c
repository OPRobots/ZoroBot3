#include <sensors.h>

#define NUM_SENSORES 4

static uint8_t sensores[NUM_SENSORES] = {ADC_CHANNEL10, ADC_CHANNEL11, ADC_CHANNEL12, ADC_CHANNEL13};

static volatile uint16_t sensores_raw[NUM_SENSORES];

uint8_t *get_sensors() {
  return sensores;
}

uint8_t get_sensors_num() {
  return NUM_SENSORES;
}

volatile uint16_t *get_sensors_raw() {
  return sensores_raw;
}

uint16_t get_sensor_raw(uint8_t pos) {
  if (pos < NUM_SENSORES) {
    return sensores_raw[pos];
  } else {
    return 0;
  }
}
