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

void handle_robot_version(void) {
  enum ROBOT_VERSION robot_version = ZOROBOT3_UNKNOWN;
  if (UID_WORD0 == 0x00310016 && UID_WORD1 == 0x3038470E && UID_WORD2 == 0x39323137) {
    robot_version = ZOROBOT3_B;
  } else if (UID_WORD0 == 0x0039001C && UID_WORD1 == 0x3235510F && UID_WORD2 == 0x32313338) {
    robot_version = ZOROBOT3_C;
  } else if (UID_WORD0 != 0 && UID_WORD1 != 0 && UID_WORD2 != 0) {
    robot_version = ZOROBOT3_A;
  }

  printf("UID: %08X %08X %08X\n", (unsigned int)UID_WORD0, (unsigned int)UID_WORD1, (unsigned int)UID_WORD2);

  set_battery_volt_div_factor(robot_version);
  set_sensors_robot_calibration(robot_version);
  show_robot_version(robot_version);
}

/**
 * @brief Establece todas las configuraciones (CARRERA, TODO: añadir más) en funcion de los Switches
 *
 */
void set_all_configs(void) {

  // set_config_run(CONFIG_RUN_RACE);
  set_config_run(CONFIG_RUN_DEBUG);
}

/**
 * @brief Obtiene la configuración de Carrera
 *
 * @return uint16_t CONFIG_RUN_RACE | CONFIG_RUN_DEBUG
 */
uint16_t get_config_run(void) {
  return _run_mode;
}