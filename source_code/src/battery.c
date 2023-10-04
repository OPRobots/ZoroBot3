#include "battery.h"

float get_battery_voltage(void) {
  uint16_t battery_bits;

  adc_start_conversion_regular(ADC2);
  while (!adc_eoc(ADC2))
    ;
  battery_bits = adc_read_regular(ADC2);
  return battery_bits * ADC_LSB * VOLT_DIV_FACTOR;
}

void show_battery_level() {
  uint16_t ticksInicio = get_clock_ticks();
  while(get_clock_ticks() < ticksInicio + 750){
	  set_leds_battery_level(get_battery_voltage());
  }
  all_leds_clear();
}
