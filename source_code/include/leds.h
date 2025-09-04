#ifndef __LEDS_H
#define __LEDS_H

#include <config.h>
#include <constants.h>
#include <delay.h>
#include <usart.h>
#include <utils.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

enum info_led {
  INFO_LED_1 = 0,
  INFO_LED_2 = 1,
  INFO_LED_3 = 2,
  INFO_LED_4 = 3,
  INFO_LED_5 = 4,
  INFO_LED_A = 5,
  INFO_LED_B = 6,
  INFO_LED_C = 7,
  INFO_LED_D = 8,
  INFO_LED_E = 9,
};

void set_status_led(bool state);
void toggle_status_led(void);
void warning_status_led(uint32_t ms);
bool is_status_led_on(void);
void set_RGB_color(uint32_t r, uint32_t g, uint32_t b);
void set_RGB_rainbow(void);
void set_RGB_color_while(uint32_t r, uint32_t g, uint32_t b, uint32_t ms);
void blink_RGB_color(uint32_t r, uint32_t g, uint32_t b, uint32_t ms);
void check_leds_while(void);
void set_leds_wave(int ms);
void set_leds_side_sensors(int ms);
void set_leds_front_sensors(int ms);
void set_leds_blink(int ms);
void set_leds_battery_level(float battery_level);
void all_leds_clear(void);
void set_info_led(uint8_t index, bool state);
void set_info_leds(void);
void show_robot_version(uint16_t version);
void clear_info_leds(void);

#endif