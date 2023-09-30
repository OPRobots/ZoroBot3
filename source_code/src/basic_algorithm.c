#include <basic_algorithm.h>

// Indica los millis a partir de los cuales realiza un cambio de pared de referencia (0 -> sin cambio)
#define REFERENCE_WALL_CHANGE_LENGTH 0
#define MILLIS_REFERENCE_WALL_CHANGE_1 5000
#define MILLIS_REFERENCE_WALL_CHANGE_2 MILLIS_REFERENCE_WALL_CHANGE_1 + 5000
#define MILLIS_REFERENCE_WALL_CHANGE_3 MILLIS_REFERENCE_WALL_CHANGE_2 + 5000
#define MILLIS_REFERENCE_WALL_CHANGE_4 MILLIS_REFERENCE_WALL_CHANGE_3 + 5000
#define MILLIS_REFERENCE_WALL_CHANGE_5 MILLIS_REFERENCE_WALL_CHANGE_4 + 5000

static uint32_t REFERENCE_WALL_CHANGE_MILLIS[] = {MILLIS_REFERENCE_WALL_CHANGE_1, MILLIS_REFERENCE_WALL_CHANGE_2, MILLIS_REFERENCE_WALL_CHANGE_3, MILLIS_REFERENCE_WALL_CHANGE_4, MILLIS_REFERENCE_WALL_CHANGE_5};
static bool REFERENCE_WALL_CHANGE_DONE[] = {false, false, false, false, false};

#define DERECHA 0
#define IZQUIERDA 1

#define DETECCION_FRONTAL 800
#define TIEMPO_FILTRO 20
#define DINAMICO true
#define MAX_ERROR_PID 150 // 0 anula la limitacion de error

int16_t objetivo_D = 0;
int16_t objetivo_I = 0;
int16_t error = 0;
int velBase = 50;

bool mano = false;

float p = 0;
float d = 0;
float kp = 0.5;
float ki = 0;
float kd = 0;
float kf = 1; // constante que determina cuanto afecta el sensor frontal para los giros dinamicos
int16_t sumError = 0;
int16_t ultError = 0;
int16_t correccion = 0;
bool frontal = false;
float millis_PID = 0;
float micros_filtro = 0;

bool started = false;
bool init = false;

uint32_t startedMillis = 0;

/**
 * @brief Comprueba cambio de mano por tiempo
 *
 */
static void check_reference_wall_change(uint32_t startedMillis, bool mano) {
  for (int i = 0; i < REFERENCE_WALL_CHANGE_LENGTH; i++) {
    if (!REFERENCE_WALL_CHANGE_DONE[i] && REFERENCE_WALL_CHANGE_MILLIS[i] != 0 && get_clock_ticks() > startedMillis + REFERENCE_WALL_CHANGE_MILLIS[i]) {
      mano = !mano;
      REFERENCE_WALL_CHANGE_DONE[i] = true;

      // TODO revisar esta parte, quizá no sea necesaria
      //  uint8_t old_side = SIDE_SENSOR_ID;
      //  SIDE_SENSOR_ID = OPOSITE_SENSOR_ID;
      //  OPOSITE_SENSOR_ID = old_side;
    }
  }
  if (mano == IZQUIERDA) {
    set_RGB_color(0, 0, 75);
  } else {
    set_RGB_color(0, 75, 0);
  }
}

/**
 * @brief Obtención de valores iniciales a partir de la mano seleccionada
 *
 */
static void basic_algorithm_start() {
  if (!started) {
    if (mano == IZQUIERDA) {
      // TODO: mostrar led de mano?
    } else {
      // TODO: mostrar led de mano?
    }

    for (uint8_t i = 0; i < TIEMPO_FILTRO; i++) {
      filtro_sensores();
      warning_status_led(50);
      delay(3000 / TIEMPO_FILTRO);
    }

    objetivo_I = sensor2_analog();
    objetivo_D = sensor1_analog();

    printf("%4d %4d\n",objetivo_I,objetivo_D);

    started = true;
    startedMillis = get_clock_ticks();
  }
}


/**
 * @brief Bucle principal de código una vez iniciada la competición
 *
 */
void basic_algorithm_loop() {
  if (!started) {
    basic_algorithm_start(); //Obtencion de valores al inicio
  }

  if (started) {
    filtro_sensores();

    if (get_clock_ticks() - millis_PID >= 1) {

      check_reference_wall_change(startedMillis, mano);

      // TODO: comprobar sensor frontal en función de la mano elegida?
      if (sensor0_analog() > DETECCION_FRONTAL && sensor3_analog() > DETECCION_FRONTAL) {
        frontal = true;
      } else {
        frontal = false;
      }

      if (mano == IZQUIERDA) {
        if (sensor2_analog() > 0) {
          error = objetivo_I - sensor2_analog();
        } else {
          error = MAX_ERROR_PID;
        }

        if (frontal && !DINAMICO) {

          set_motors_speed(300, -300);
          delay(150);
          set_motors_speed(0, 0);
          delay(50);
          return;
        }
      }
      if (mano == DERECHA) {
        if (sensor1_analog() > 0) {
          error = objetivo_D - sensor1_analog();
        } else {
          error = MAX_ERROR_PID;
        }

        if (frontal && !DINAMICO) {
          
          set_motors_speed(-300, 300);
          delay(150);
          set_motors_speed(0, 0);
          delay(50);
          return;
        }
      }

      // Serial.println(error);
      // if (MAX_ERROR_PID != 0) {
      //   error = constrain(error, -MAX_ERROR_PID, MAX_ERROR_PID);
      // }

      if (DINAMICO && frontal) { // Añadir lectura delantera al error para tener giros dinamicos
        error -= ((sensor0_analog() + sensor3_analog())/2 - DETECCION_FRONTAL) * kf;
      }

      p = kp * error;
      d = kd * (error - ultError);
      ultError = error;
      correccion = p + d;
      if (mano == DERECHA) {
        correccion = -correccion;
      }
      set_motors_speed(velBase - correccion, velBase + correccion);

      millis_PID = get_clock_ticks();
    }
  }
}

void start_from_front_sensor() {
  if (!is_competicion_iniciada() && get_sensor_raw_filter(SENSOR_SIDE_LEFT_ID) >= DETECCION_FRONTAL) {
    while (get_sensor_raw_filter(SENSOR_SIDE_LEFT_ID) >= DETECCION_FRONTAL) {
      warning_status_led(125);
    }

    set_competicion_iniciada(true);
  }
}

void start_from_ms() {
  if (!is_competicion_iniciada()) {
    uint32_t millis_target = get_clock_ticks();
    while (get_clock_ticks() < millis_target + 4000) {
      if (mano == IZQUIERDA) {
        gpio_toggle(GPIOC, GPIO4);
        gpio_toggle(GPIOA, GPIO5 | GPIO6 | GPIO7);
      } else {
        gpio_toggle(GPIOC, GPIO5);
        gpio_toggle(GPIOB, GPIO0 | GPIO1 | GPIO2);
      }
      delay(250);
    }

    set_competicion_iniciada(true);
  }
}

void basic_algorithm_config() {
  while (!init) {
    if (get_menu_up_btn()) {
      mano = DERECHA;
    } else if (get_menu_down_btn()) {
      mano = IZQUIERDA;
    }

    // PA5, PA6, PA7, PC4, PC5, PB0, PB1, PB2
    gpio_clear(GPIOA, GPIO5 | GPIO6 | GPIO7);
    gpio_clear(GPIOB, GPIO0 | GPIO1 | GPIO2);
    gpio_clear(GPIOC, GPIO4 | GPIO5);

    if (mano == IZQUIERDA) {
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
    }
  }

  gpio_clear(GPIOA, GPIO5 | GPIO6 | GPIO7);
  gpio_clear(GPIOB, GPIO0 | GPIO1 | GPIO2);
  gpio_clear(GPIOC, GPIO4 | GPIO5);
}
