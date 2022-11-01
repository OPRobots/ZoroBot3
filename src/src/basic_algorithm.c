#include <basic_algorithm.h>

bool init = false;
bool started = false;

bool mano_izquierda = true;
uint8_t SIDE_SENSOR_ID = 0;
uint8_t FRONT_SENSOR_ID = 0;
uint16_t SENSOR_TARGET = 0;

float velBase = 10;

float correccion_velocidad = 0;
float error_anterior = 0;

static void start_basic_algorithm() {
  if (!started) {
    if (mano_izquierda) {
      SIDE_SENSOR_ID = SENSOR_FRONT_RIGHT_ID;
      FRONT_SENSOR_ID = SENSOR_SIDE_LEFT_ID;
    } else {
      SIDE_SENSOR_ID = SENSOR_FRONT_LEFT_ID;
      FRONT_SENSOR_ID = SENSOR_SIDE_RIGHT_ID;
    }

    SENSOR_TARGET = get_sensor_raw_filter(SIDE_SENSOR_ID);
    started = true;
  }
}

static float calc_pid_correction(uint16_t side_sensor_value, uint16_t front_sensor_value) {
  float p = 0;
  float i = 0;
  float d = 0;
  int32_t error = SENSOR_TARGET - side_sensor_value;
  if (front_sensor_value >= FRONT_SENSOR_THRESHOLD) {
    error += (FRONT_SENSOR_THRESHOLD - front_sensor_value) / 2.0f;
  }

  p = BASIC_ALGORITHM_KP * error;
  d = BASIC_ALGORITHM_KD * (error - error_anterior);
  error_anterior = error;

  return p + i + d;
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

  correccion_velocidad = calc_pid_correction(get_sensor_raw_filter(SIDE_SENSOR_ID), get_sensor_raw_filter(FRONT_SENSOR_ID));

  float velI = velBase + correccion_velocidad;
  float velD = velBase - correccion_velocidad;

  if (velD < MIN_SPEED_PERCENT) {
    velD = MIN_SPEED_PERCENT;
  } else if (velD > 100) {
    velD = 100;
  }
  if (velI < MIN_SPEED_PERCENT) {
    velI = MIN_SPEED_PERCENT;
  } else if (velI > 100) {
    velI = 100;
  }
  // printf("%.2f - %.2f | %.2f - %d\n", velD, velI, correccion_velocidad, get_sensor_raw_filter(SIDE_SENSOR_ID));
  set_motors_speed(velD, velI);
}
