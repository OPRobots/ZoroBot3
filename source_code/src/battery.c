#include "battery.h"

static volatile float battery_voltage = 0;

void update_battery_voltage(void) {
  adc_start_conversion_regular(ADC2);
  while (!adc_eoc(ADC2))
    ;
  float voltage = adc_read_regular(ADC2) * ADC_LSB * VOLT_DIV_FACTOR;
  if (battery_voltage == 0) {
    battery_voltage = voltage;
  } else {
    battery_voltage = BATTERY_VOLTAGE_LOW_PASS_FILTER_ALPHA * voltage + (1 - BATTERY_VOLTAGE_LOW_PASS_FILTER_ALPHA) * battery_voltage;
  }
}

float get_battery_voltage(void) {
  return battery_voltage;
}

void show_battery_level(void) {
  uint32_t ticksInicio = get_clock_ticks();
  while (get_clock_ticks() < ticksInicio + 750) {
    set_leds_battery_level(get_battery_voltage());
  }
  all_leds_clear();
}
