#include <config.h>

uint16_t _run_mode;

/**
 * @brief Establece la configuración de Carrera
 * 
 * @param run_mode CONFIG_RUN_RACE | CONFIG_RUN_DEBUG
 */
static void set_config_run(uint16_t run_mode) {
  _run_mode = run_mode;
}

/**
 * @brief Establece todas las configuraciones (CARRERA, TODO: añadir más) en funcion de los Switches
 * 
 */
void set_all_configs(void) {
  if (get_swtich_1()) {
    set_config_run(CONFIG_RUN_RACE);
  } else {
    set_config_run(CONFIG_RUN_DEBUG);
  }
}

/**
 * @brief Obtiene la configuración de Carrera
 * 
 * @return uint16_t CONFIG_RUN_RACE | CONFIG_RUN_DEBUG
 */
uint16_t get_config_run(void) {
  return _run_mode;
}