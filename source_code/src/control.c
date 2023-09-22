#include <control.h>

static bool competicionIniciada = false;


/**
 * @brief Comprueba si el robot está en funcionamiento
 * 
 * @return bool 
 */

bool is_competicion_iniciada() {
  return competicionIniciada;
}

/**
 * @brief Establece el estado actual del robot
 * 
 * @param state Estado actual del robot
 */

void set_competicion_iniciada(bool state) {
  competicionIniciada = state;
}