#include <basic_wall_change.h>


// Indica los millis a partir de los cuales realiza un cambio de pared de referencia (0 -> sin cambio)
#define REFERENCE_WALL_CHANGE_LENGTH 2 // numero de cambios

int REFERENCE_WALL_CHANGE_MILLIS[5] = {0, 0, 0, 0, 0};

bool REFERENCE_WALL_CHANGE_DONE[] = {false, false, false, false, false};

int indice_cambios = 0;
int indice_check_cambio = 0;
bool tiempo_seg = true;
bool tiempo_mseg = false;
int patron_led = 0;
bool configuracion_lista = false;
long ultimo_cambio_millis = 0;

/**
 * @brief Comprueba cambio de mano por tiempo
 *
 */

bool check_reference_wall_change(uint32_t startedMillis, bool mano) {
  if (ultimo_cambio_millis == 0) {
    ultimo_cambio_millis = startedMillis;
  }
  if (!REFERENCE_WALL_CHANGE_DONE[indice_check_cambio] && REFERENCE_WALL_CHANGE_MILLIS[indice_check_cambio] != 0 && get_clock_ticks() > ultimo_cambio_millis + REFERENCE_WALL_CHANGE_MILLIS[indice_check_cambio]) {
    ultimo_cambio_millis = get_clock_ticks();
    mano = !mano;
    REFERENCE_WALL_CHANGE_DONE[indice_check_cambio] = true;
    indice_check_cambio++;
  }

  if (mano) {
    // set_RGB_color(75, 0, 0);
    //set_info_led(0, false);
    //set_info_led(7, true);
  } else {
    // set_RGB_color(0, 75, 0);
    //set_info_led(0, true);
    //set_info_led(7, false);
  }
  return mano;
}

bool configuracion() {

  if (get_menu_up_btn()) {
    if (tiempo_seg) {
      REFERENCE_WALL_CHANGE_MILLIS[indice_cambios] += 1000;
    } else if (tiempo_mseg) {
      REFERENCE_WALL_CHANGE_MILLIS[indice_cambios] += 100;
    }
    while (get_menu_up_btn()) {
    }
    patron_led = (patron_led + 1) % 10;
    leds_configuracion(patron_led);
  } else if (get_menu_down_btn()) {
    if (tiempo_seg) {
      REFERENCE_WALL_CHANGE_MILLIS[indice_cambios] -= 1000;
    } else if (tiempo_mseg) {
      REFERENCE_WALL_CHANGE_MILLIS[indice_cambios] -= 100;
    }
    if (REFERENCE_WALL_CHANGE_MILLIS[indice_cambios] < 0) {
      REFERENCE_WALL_CHANGE_MILLIS[indice_cambios] = 0;
    }
    while (get_menu_down_btn()) {
    }
    // patron_led = (patron_led - 1) % 10;
    patron_led--;
    if (patron_led < 0) {
      patron_led = 9;
    }
    leds_configuracion(patron_led);
  }
  if (get_menu_mode_btn()) {
    if (tiempo_seg) {
      tiempo_seg = false;
      tiempo_mseg = true;
      patron_led = 0;
      leds_configuracion(patron_led);
    } else if (tiempo_mseg) {
      if (indice_cambios >= (REFERENCE_WALL_CHANGE_LENGTH - 1)) {
        configuracion_lista = true;
      } else {
        indice_cambios++;
        tiempo_seg = true;
        tiempo_mseg = false;

        patron_led = 0;
        leds_configuracion(patron_led);
      }
    }
    while (get_menu_mode_btn()) {
    }
  }
  if (configuracion_lista) {
    for (int i = 0; i < REFERENCE_WALL_CHANGE_LENGTH; i++) {

      leds_configuracion(1);
      delay(50);
      leds_configuracion(2);
      delay(50);
      leds_configuracion(3);
      delay(50);
      leds_configuracion(4);
      delay(50);
      leds_configuracion(3);
      delay(50);
      leds_configuracion(2);
      delay(50);
      leds_configuracion(1);
      delay(50);
      leds_configuracion(0);
      printf("%d\n", REFERENCE_WALL_CHANGE_MILLIS[i]);
    }
  }
  return configuracion_lista;
}
