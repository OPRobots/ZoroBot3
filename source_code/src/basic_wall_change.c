#include <basic_wall_change.h>

// Indica los millis a partir de los cuales realiza un cambio de pared de referencia (0 -> sin cambio)
#define REFERENCE_WALL_CHANGE_LENGTH 0
#define MILLIS_REFERENCE_WALL_CHANGE_1 5000
#define MILLIS_REFERENCE_WALL_CHANGE_2 MILLIS_REFERENCE_WALL_CHANGE_1 + 5000
#define MILLIS_REFERENCE_WALL_CHANGE_3 MILLIS_REFERENCE_WALL_CHANGE_2 + 5000
#define MILLIS_REFERENCE_WALL_CHANGE_4 MILLIS_REFERENCE_WALL_CHANGE_3 + 5000
#define MILLIS_REFERENCE_WALL_CHANGE_5 MILLIS_REFERENCE_WALL_CHANGE_4 + 5000

static uint32_t REFERENCE_WALL_CHANGE_MILLIS[] = {MILLIS_REFERENCE_WALL_CHANGE_1, MILLIS_REFERENCE_WALL_CHANGE_2, MILLIS_REFERENCE_WALL_CHANGE_3, MILLIS_REFERENCE_WALL_CHANGE_4, MILLIS_REFERENCE_WALL_CHANGE_5};
static bool REFERENCE_WALL_CHANGE_DONE[] = {false, false, false, false, false};


/**
 * @brief Comprueba cambio de mano por tiempo
 *
 */
 void check_reference_wall_change(uint32_t startedMillis, bool mano) {
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
  if (mano == true) { //Izquierda
    set_RGB_color(0, 0, 75);
  } else {
    set_RGB_color(0, 75, 0);
  }
}