#include <calibrations.h>

bool calibration_enabled = false;

static void check_calibration_active(void) {
  if (get_menu_mode_btn()) {
    while (get_menu_mode_btn()) {
    }
    calibration_enabled = !calibration_enabled;
  }
}

void calibrate_from_config(uint8_t type) {
  if (type != CALIBRATE_NONE) {
    check_calibration_active();
  } else {
    calibration_enabled = false;
  }
  if (calibration_enabled) {
    clear_info_leds();
    set_RGB_color(0, 125, 0);
    delay(2000);
    switch (type) {
      case CALIBRATE_GYRO_Z:
        lsm6dsr_gyro_z_calibration();
        break;
      case CALIBRATE_SIDE_SENSORS_OFFSET:
        side_sensors_calibration();
        break;
      case CALIBRATE_FRONT_SENSORS:
        front_sensors_calibration();
        break;
      case CALIBRATE_STORE_EEPROM:
        eeprom_save();
        break;
    }
    set_RGB_color(0, 0, 0);
    calibration_enabled = false;
    menu_config_reset_values();
  }
}

void calibrate_manual_distances(void) {
  if (get_debug_btn()) {
    delay(500);
    printf("L: %4d FL: %4d FR: %4d R: %4d\n", get_sensor_raw_filter(SENSOR_SIDE_LEFT_WALL_ID), get_sensor_raw_filter(SENSOR_FRONT_LEFT_WALL_ID), get_sensor_raw_filter(SENSOR_FRONT_RIGHT_WALL_ID), get_sensor_raw_filter(SENSOR_SIDE_RIGHT_WALL_ID));
    set_debug_btn(false);
  }
}