#include <basic_algorithm.h>

bool init = false;
bool started = false;
static uint32_t startedMillis = 0;

static uint32_t REFERENCE_WALL_CHANGE_MILLIS[] = {MILLIS_REFERENCE_WALL_CHANGE_1, MILLIS_REFERENCE_WALL_CHANGE_2, MILLIS_REFERENCE_WALL_CHANGE_3};
static bool REFERENCE_WALL_CHANGE_DONE[] = {false, false, false};

bool mano_izquierda = true;
uint8_t SIDE_SENSOR_ID = 0;
uint8_t OPOSITE_SENSOR_ID = 0;
uint8_t FRONT_SENSOR_ID = 0;
uint16_t SENSOR_TARGET = 0;
uint16_t OPOSITE_SENSOR_TARGET = 0;

float velBase = 10;

float correccion_velocidad = 0;
float error_anterior = 0;
int32_t micrometers_without_wall = 0;

uint8_t uTurnChecks = 0;

static void start_basic_algorithm() {
  if (!started) {
    if (mano_izquierda) {
      SIDE_SENSOR_ID = SENSOR_FRONT_RIGHT_ID;
      OPOSITE_SENSOR_ID = SENSOR_FRONT_LEFT_ID;
      FRONT_SENSOR_ID = SENSOR_SIDE_RIGHT_ID;
    } else {
      SIDE_SENSOR_ID = SENSOR_FRONT_LEFT_ID;
      OPOSITE_SENSOR_ID = SENSOR_FRONT_RIGHT_ID;
      FRONT_SENSOR_ID = SENSOR_SIDE_LEFT_ID;
    }

    SENSOR_TARGET = get_sensor_raw_filter(SIDE_SENSOR_ID);
    OPOSITE_SENSOR_TARGET = get_sensor_raw_filter(OPOSITE_SENSOR_ID);
    started = true;
  }
}

static void check_reference_wall_change() {
  for (int i = 0; i < REFERENCE_WALL_CHANGE_LENGTH; i++) {
    if (!REFERENCE_WALL_CHANGE_DONE[i] && REFERENCE_WALL_CHANGE_MILLIS[i] != 0 && get_clock_ticks() > startedMillis + REFERENCE_WALL_CHANGE_MILLIS[i]) {
      mano_izquierda = !mano_izquierda;
      uint8_t old_side = SIDE_SENSOR_ID;
      SIDE_SENSOR_ID = OPOSITE_SENSOR_ID;
      OPOSITE_SENSOR_ID = old_side;
      REFERENCE_WALL_CHANGE_DONE[i] = true;
    }
  }
  if (mano_izquierda) {
    set_RGB_color(0, 0, 75);
  } else {
    set_RGB_color(0, 75, 0);
  }
}

static void check_u_turn() {
  if (get_sensor_raw_filter(FRONT_SENSOR_ID) >= FRONT_SENSOR_THRESHOLD && get_sensor_raw_filter(OPOSITE_SENSOR_ID) >= OPOSITE_SENSOR_TARGET) {
    // if (mano_izquierda) {
    //   set_z_angle(get_gyro_z_degrees() + 87);
    // } else {
    //   set_z_angle(get_gyro_z_degrees() - 87);
    // }
    // delay(100);
    set_motors_speed(0, 0);
    delay(250);

    if (mano_izquierda) {
      set_gyro_z_degrees(0);
      set_z_angle(get_gyro_z_degrees() + 85);
      // self_rotate(180);
    } else {
      set_gyro_z_degrees(0);
      set_z_angle(get_gyro_z_degrees() - 85);
      // self_rotate(-180);
    }
    // delay(100);
    // set_motors_speed(-velBase, -velBase);
    // delay(125);
    set_motors_speed(0, 0);
    delay(100);
  }
}

static void check_u_turn_multiple() {
  if (get_sensor_raw_filter(FRONT_SENSOR_ID) >= FRONT_SENSOR_THRESHOLD && get_sensor_raw_filter(OPOSITE_SENSOR_ID) >= OPOSITE_SENSOR_TARGET) {
    uTurnChecks++;
    if (uTurnChecks >= MIN_U_TURN_CECKES) {
      if (mano_izquierda) {
        set_z_angle(get_gyro_z_degrees() + 87);
      } else {
        set_z_angle(get_gyro_z_degrees() - 87);
      }
      delay(100);
    }
  } else {
    uTurnChecks = 0;
  }
}

static float calc_pid_correction(uint16_t side_sensor_value, uint16_t front_sensor_value) {
  float p = 0;
  float i = 0;
  float d = 0;
  int32_t error = SENSOR_TARGET - side_sensor_value;
  if (front_sensor_value >= FRONT_SENSOR_THRESHOLD) {
    error += (FRONT_SENSOR_THRESHOLD - front_sensor_value) * 1.33f;
  }

  p = BASIC_ALGORITHM_KP * error;
  d = BASIC_ALGORITHM_KD * (error - error_anterior);
  error_anterior = error;

  return p + i + d;
}

void check_start_front_sensor() {
  if (!is_competicion_iniciada() && get_sensor_raw_filter(FRONT_SENSOR_ID) >= FRONT_SENSOR_THRESHOLD) {
    while (get_sensor_raw_filter(FRONT_SENSOR_ID) >= FRONT_SENSOR_THRESHOLD) {
      warning_status_led(125);
    }
    delay(1000);
    set_competicion_iniciada(true);
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

    if (get_menu_mode_btn()) {
      init = true;
      while (get_menu_mode_btn()) {
        delay(100);
      }
      uint32_t millis_target = get_clock_ticks();
      while (get_clock_ticks() < millis_target + 4000) {
        if (mano_izquierda) {
          gpio_toggle(GPIOC, GPIO4);
          gpio_toggle(GPIOA, GPIO5 | GPIO6 | GPIO7);
        } else {
          gpio_toggle(GPIOC, GPIO5);
          gpio_toggle(GPIOB, GPIO0 | GPIO1 | GPIO2);
        }
        delay(250);
      }
      start_basic_algorithm();
    }
  }

  gpio_clear(GPIOA, GPIO5 | GPIO6 | GPIO7);
  gpio_clear(GPIOB, GPIO0 | GPIO1 | GPIO2);
  gpio_clear(GPIOC, GPIO4 | GPIO5);
}

uint8_t lostWallChecks = 0;
void basic_algorithm_loop() {
  if (startedMillis == 0) {
    startedMillis = get_clock_ticks();
    set_status_led(false);
  }
  check_reference_wall_change();
  // start_basic_algorithm(); // Se llama desde la función de init, después de confirmar la mano dominante

  //check_u_turn();

  uint16_t side_sensor_value = get_sensor_raw_filter(SIDE_SENSOR_ID);

  if (side_sensor_value < (SENSOR_TARGET / 2)) {
    lostWallChecks++;
  } else {
    lostWallChecks = 0;
  }

  if (lostWallChecks > 5) {
    set_status_led(true);
    delay(100);
  } else {
    set_status_led(false);
  }

  correccion_velocidad = calc_pid_correction(side_sensor_value, get_sensor_raw_filter(FRONT_SENSOR_ID));
  if (mano_izquierda) {
    correccion_velocidad = -correccion_velocidad;
  }

  float velI = velBase + correccion_velocidad;
  float velD = velBase - correccion_velocidad;

  if (velD < -100) {
    velD = -100;
  } else if (velD > 100) {
    velD = 100;
  }
  if (velI < -100) {
    velI = -100;
  } else if (velI > 100) {
    velI = 100;
  }
  // printf("%.2f - %.2f | %.2f - %d\n", velI, velD, correccion_velocidad, get_sensor_raw_filter(SIDE_SENSOR_ID));
  set_motors_speed(velI, velD);
}