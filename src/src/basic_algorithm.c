#include <basic_algorithm.h>

bool init = false;
bool started = false;

bool mano_izquierda = false;

static void start_basic_algorithm() {
  if (!started) {

    started = true;
  }
}

void basic_algorithm_init() {
  while (!init) {
    if (get_menu_up_btn()) {
      mano_izquierda = false;
    } else if (get_menu_down_btn()) {
      mano_izquierda = true;
    }

    // PA5, PA6, PA7, PC4, PC5, PB0, PB1, PB2
    gpio_clear(GPIOA, GPIO5 | GPIO6 | GPIO7);
    gpio_clear(GPIOB, GPIO0 | GPIO1 | GPIO2);
    gpio_clear(GPIOC, GPIO4 | GPIO5);

    if (mano_izquierda) {
      gpio_set(GPIOC, GPIO4);
      gpio_set(GPIOA, GPIO5 | GPIO6 | GPIO7);
    } else {
      gpio_set(GPIOC, GPIO5);
      gpio_set(GPIOB, GPIO0 | GPIO1 | GPIO2);
    }

    if (get_menu_mode_btn() == 1) {
      init = true;
    }
  }

  gpio_clear(GPIOA, GPIO5 | GPIO6 | GPIO7);
  gpio_clear(GPIOB, GPIO0 | GPIO1 | GPIO2);
  gpio_clear(GPIOC, GPIO4 | GPIO5);
}

void basic_algorithm_loop() {
  start_basic_algorithm();
}
