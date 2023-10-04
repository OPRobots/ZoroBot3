#include <basic_algorithm.h>
#include <basic_wall_change.h>


#define DERECHA 0
#define IZQUIERDA 1

#define DETECCION_FRONTAL 500
#define TIEMPO_FILTRO 20
#define DINAMICO true
#define MAX_ERROR_PID 70

int16_t objetivo_D = 0;
int16_t objetivo_I = 0;
int16_t error = 0;
int velBase = 70;

bool mano = false;

float aux_max_error_PID = 0; // 0 ira recto, infinito anulara la utilidad de la variable
float p = 0;
float d = 0;
float kp = 0.3;
float ki = 1; // Ajusta el aumento de error mientras no se detecte pared (No estoy seguro de que esto funcione)
float kd = 10;
float kf = 1.3; // Kfrontal constante que determina cuanto afecta el sensor frontal para los giros dinamicos
float kw = 0.33; // Kwall constante que determina cuando se considera que se perdio la pared de 0 a 1 siendo 1 pegado al robot, 0 lejos del roboot
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

    printf("%4d %4d\n", objetivo_I, objetivo_D);

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
    basic_algorithm_start(); // Obtencion de valores al inicio
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
        if (sensor2_analog() > objetivo_D*kw) {
          error = objetivo_I - sensor2_analog();
          aux_max_error_PID = MAX_ERROR_PID;
        }  else {
          error = aux_max_error_PID;
          aux_max_error_PID = aux_max_error_PID + ki;
        }

        if (frontal && !DINAMICO) {

          set_motors_speed(60, -60);
          delay(150);
          set_motors_speed(0, 0);
          delay(50);
          return;
        }
      }
      if (mano == DERECHA) {
        if (sensor1_analog() > objetivo_D*kw) {
          error = objetivo_D - sensor1_analog();
          aux_max_error_PID = MAX_ERROR_PID;
        } else {
          error = aux_max_error_PID;
          aux_max_error_PID = aux_max_error_PID + ki;
        }

        if (frontal && !DINAMICO) {

          set_motors_speed(-60, 60);
          delay(150);
          set_motors_speed(0, 0);
          delay(50);
          return;
        }
      }

      // Serial.println(error);
      // if (max_error_PID != 0) {
      //   error = constrain(error, -max_error_PID, max_error_PID);
      // }

      if (DINAMICO && frontal) { // Añadir lectura delantera al error para tener giros dinamicos
        error -= ((sensor0_analog() + sensor3_analog()) / 2 - DETECCION_FRONTAL) * kf;
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


/**
 * @brief Usar el sensor 0 para inicio sin tocar el robot
 */

void start_from_front_sensor() {
  if (!is_competicion_iniciada() && get_sensor_raw_filter(SENSOR_SIDE_LEFT_ID) >= DETECCION_FRONTAL) {
    while (get_sensor_raw_filter(SENSOR_SIDE_LEFT_ID) >= DETECCION_FRONTAL) {
      warning_status_led(125);
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
