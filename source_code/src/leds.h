#ifndef __LEDS_H
#define __LEDS_H

#include <config.h>
#include <delay.h>
#include <utils.h>
#include <usart.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>


void set_status_led(bool state);
void toggle_status_led();
void warning_status_led(uint16_t ms);
void set_RGB_color(uint32_t r, uint32_t g, uint32_t b);
void set_RGB_rainbow();
void set_leds_wave(int ms);
void set_leds_battery_level(float battery_level);
void all_leds_clear();
void set_info_led(uint8_t index, bool state);
void clear_info_leds();
void leds_configuracion(uint8_t patron_led);

#endif