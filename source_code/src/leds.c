#include <leds.h>

static int lastTicksRainbow = 0;
static uint32_t rainbowRGB[3] = {LEDS_MAX_PWM, 0, 0};
static int rainbowColorDesc = 0;
static int rainbowColorAsc = 1;

static int lastTicksWarning = 0;

static int lastTicksWave = 0;
static int currentIndexWave = 0;

static int lastTicksWarningBateria = 0;

void set_status_led(bool state) {
  if (state) {
    gpio_set(GPIOA, GPIO12);
  } else {
    gpio_clear(GPIOA, GPIO12);
  }
}

void toggle_status_led() {
  gpio_toggle(GPIOA, GPIO12);
}

void warning_status_led(uint16_t ms) {
  if (get_clock_ticks() > lastTicksWarning + ms) {
    toggle_status_led();
    lastTicksWarning = get_clock_ticks();
  }
}

void set_RGB_color(uint32_t r, uint32_t g, uint32_t b) {
  timer_set_oc_value(TIM1, TIM_OC2, r);
  timer_set_oc_value(TIM1, TIM_OC3, g);
  timer_set_oc_value(TIM1, TIM_OC4, b);
}

void set_RGB_rainbow() {
  if (get_clock_ticks() > lastTicksRainbow + 10) {
    lastTicksRainbow = get_clock_ticks();
    rainbowRGB[rainbowColorDesc] -= 20;
    rainbowRGB[rainbowColorAsc] += 20;
    set_RGB_color(rainbowRGB[0], rainbowRGB[1], rainbowRGB[2]);
    if (rainbowRGB[rainbowColorDesc] <= 0 || rainbowRGB[rainbowColorAsc] >= LEDS_MAX_PWM) {
      rainbowRGB[rainbowColorDesc] = 0;
      rainbowRGB[rainbowColorAsc] = LEDS_MAX_PWM;
      set_RGB_color(rainbowRGB[0], rainbowRGB[1], rainbowRGB[2]);
      rainbowColorDesc++;
      if (rainbowColorDesc > 2) {
        rainbowColorDesc = 0;
      }
      rainbowColorAsc = rainbowColorDesc == 2 ? 0 : rainbowColorDesc + 1;
    }
  }
}

void set_leds_wave(int ms) {
  if (get_clock_ticks() > lastTicksWave + ms) {
    gpio_clear(GPIOA, GPIO5 | GPIO6 | GPIO7);
    gpio_clear(GPIOC, GPIO4 | GPIO5);
    gpio_clear(GPIOB, GPIO0 | GPIO1 | GPIO2);

    switch (currentIndexWave) {
      case 0:
        gpio_set(GPIOC, GPIO4);
        gpio_set(GPIOC, GPIO5);
        break;
      case 1:
        gpio_set(GPIOA, GPIO7);
        gpio_set(GPIOB, GPIO0);
        break;
      case 2:
        gpio_set(GPIOB, GPIO1);
        gpio_set(GPIOA, GPIO6);
        break;
      case 3:
        gpio_set(GPIOA, GPIO5);
        gpio_set(GPIOB, GPIO2);
        break;
    }

    currentIndexWave++;
    if (currentIndexWave >= 4) {
      currentIndexWave = 0;
    }
    lastTicksWave = get_clock_ticks();
  }
}

void set_leds_battery_level(float battery_level) {
  float percent_battery_level = map(battery_level, BATTERY_LOW_LIMIT_VOLTAGE, BATTERY_HIGH_LIMIT_VOLTAGE, 0.0f, 100.0f);
  if (percent_battery_level <= 10) {

    gpio_clear(GPIOA, GPIO5 | GPIO6 | GPIO7);
    gpio_clear(GPIOB, GPIO0 | GPIO1 | GPIO2);
    if (get_clock_ticks() > lastTicksWarningBateria + 50) {

      gpio_toggle(GPIOC, GPIO4 | GPIO5);

      lastTicksWarningBateria = get_clock_ticks();
    }

  } else if (percent_battery_level <= 25) {

    gpio_clear(GPIOA, GPIO5 | GPIO6 | GPIO7);
    gpio_clear(GPIOB, GPIO0 | GPIO1 | GPIO2);

    gpio_set(GPIOC, GPIO4 | GPIO5);

  } else if (percent_battery_level <= 50) {
    gpio_clear(GPIOA, GPIO5 | GPIO6);
    gpio_clear(GPIOB, GPIO1 | GPIO2);

    gpio_set(GPIOC, GPIO4 | GPIO5);
    gpio_set(GPIOA, GPIO7);
    gpio_set(GPIOB, GPIO0);

  } else if (percent_battery_level <= 75) {

    gpio_clear(GPIOA, GPIO5);
    gpio_clear(GPIOB, GPIO2);

    gpio_set(GPIOC, GPIO4 | GPIO5);
    gpio_set(GPIOA, GPIO7 | GPIO6);
    gpio_set(GPIOB, GPIO0 | GPIO1);

  } else if (percent_battery_level <= 90) {

    gpio_set(GPIOA, GPIO5 | GPIO6 | GPIO7);
    gpio_set(GPIOC, GPIO4 | GPIO5);
    gpio_set(GPIOB, GPIO0 | GPIO1 | GPIO2);
  } else {
    if (get_clock_ticks() > lastTicksWarningBateria + 50) {

      gpio_toggle(GPIOA, GPIO5 | GPIO6 | GPIO7);
      gpio_toggle(GPIOC, GPIO4 | GPIO5);
      gpio_toggle(GPIOB, GPIO0 | GPIO1 | GPIO2);

      lastTicksWarningBateria = get_clock_ticks();
    }
  }
}

void all_leds_clear() {
  set_RGB_color(0, 0, 0);
  set_status_led(false);

  gpio_clear(GPIOA, GPIO5 | GPIO6 | GPIO7);
  gpio_clear(GPIOC, GPIO4 | GPIO5);
  gpio_clear(GPIOB, GPIO0 | GPIO1 | GPIO2);
}

void set_info_led(uint8_t index, bool state) {
  switch (index) {
    case 0:
      if (state) {
        gpio_set(GPIOA, GPIO5);
      } else {
        gpio_clear(GPIOA, GPIO5);
      }
      break;
    case 1:
      if (state) {
        gpio_set(GPIOB, GPIO1);
      } else {
        gpio_clear(GPIOB, GPIO1);
      }
      break;
    case 2:
      if (state) {
        gpio_set(GPIOA, GPIO7);
      } else {
        gpio_clear(GPIOA, GPIO7);
      }
      break;
    case 3:
      if (state) {
        gpio_set(GPIOC, GPIO4);
      } else {
        gpio_clear(GPIOC, GPIO4);
      }
      break;
    case 4:
      if (state) {
        gpio_set(GPIOC, GPIO5);
      } else {
        gpio_clear(GPIOC, GPIO5);
      }
      break;
    case 5:
      if (state) {
        gpio_set(GPIOB, GPIO0);
      } else {
        gpio_clear(GPIOB, GPIO0);
      }
      break;
    case 6:
      if (state) {
        gpio_set(GPIOA, GPIO6);
      } else {
        gpio_clear(GPIOA, GPIO6);
      }
      break;
    case 7:
      if (state) {
        gpio_set(GPIOB, GPIO2);
      } else {
        gpio_clear(GPIOB, GPIO2);
      }
      break;
  }
}

void clear_info_leds() {

  gpio_clear(GPIOA, GPIO5 | GPIO6 | GPIO7);
  gpio_clear(GPIOC, GPIO4 | GPIO5);
  gpio_clear(GPIOB, GPIO0 | GPIO1 | GPIO2);
}


void leds_configuracion(uint8_t patron_led) {
  switch (patron_led) {
    case 0:
      set_info_led(0, false);
      set_info_led(1, false);
      set_info_led(2, false);
      set_info_led(3, false);
      set_info_led(4, false);
      set_info_led(5, false);
      set_info_led(6, false);
      set_info_led(7, false);
      set_RGB_color(0, 0, 0);
      break;
    case 1:
      set_info_led(0, true);
      set_info_led(1, false);
      set_info_led(2, false);
      set_info_led(3, false);
      set_info_led(4, false);
      set_info_led(5, false);
      set_info_led(6, false);
      set_info_led(7, true);
      set_RGB_color(0, 0, 0);
      break;
    case 2:
      set_info_led(0, true);
      set_info_led(1, true);
      set_info_led(2, false);
      set_info_led(3, false);
      set_info_led(4, false);
      set_info_led(5, false);
      set_info_led(6, true);
      set_info_led(7, true);
      set_RGB_color(0, 0, 0);
      break;
    case 3:
      set_info_led(0, true);
      set_info_led(1, true);
      set_info_led(2, true);
      set_info_led(3, false);
      set_info_led(4, false);
      set_info_led(5, true);
      set_info_led(6, true);
      set_info_led(7, true);
      set_RGB_color(0, 0, 0);
      break;
    case 4:
      set_info_led(0, true);
      set_info_led(1, true);
      set_info_led(2, true);
      set_info_led(3, true);
      set_info_led(4, true);
      set_info_led(5, true);
      set_info_led(6, true);
      set_info_led(7, true);
      set_RGB_color(0, 0, 0);
      break;
    case 5:
      set_info_led(0, false);
      set_info_led(1, false);
      set_info_led(2, false);
      set_info_led(3, false);
      set_info_led(4, false);
      set_info_led(5, false);
      set_info_led(6, false);
      set_info_led(7, false);
      set_RGB_color(0, 75, 0);
      break;
    case 6:
      set_info_led(0, true);
      set_info_led(1, false);
      set_info_led(2, false);
      set_info_led(3, false);
      set_info_led(4, false);
      set_info_led(5, false);
      set_info_led(6, false);
      set_info_led(7, true);
      set_RGB_color(0, 75, 0);
      break;
    case 7:
      set_info_led(0, true);
      set_info_led(1, true);
      set_info_led(2, false);
      set_info_led(3, false);
      set_info_led(4, false);
      set_info_led(5, false);
      set_info_led(6, true);
      set_info_led(7, true);
      set_RGB_color(0, 75, 0);
      break;
    case 8:
      set_info_led(0, true);
      set_info_led(1, true);
      set_info_led(2, true);
      set_info_led(3, false);
      set_info_led(4, false);
      set_info_led(5, true);
      set_info_led(6, true);
      set_info_led(7, true);
      set_RGB_color(0, 75, 0);
      break;
    case 9:
      set_info_led(0, true);
      set_info_led(1, true);
      set_info_led(2, true);
      set_info_led(3, true);
      set_info_led(4, true);
      set_info_led(5, true);
      set_info_led(6, true);
      set_info_led(7, true);
      set_RGB_color(0, 75, 0);
      break;

    default:
      break;
  }
}