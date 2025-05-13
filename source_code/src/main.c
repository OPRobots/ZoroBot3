#include <battery.h>
#include <buttons.h>
#include <control.h>
#include <delay.h>
#include <encoders.h>
#include <floodfill.h>
#include <handwall.h>
#include <timetrial.h>
#include <leds.h>
#include <lsm6dsr.h>
#include <macroarray.h>
#include <menu.h>
#include <motors.h>
#include <move.h>
// #include <mpu6500.h>
#include <rc5.h>
#include <sensors.h>
#include <setup.h>
#include <usart.h>

void sys_tick_handler(void) {
  clock_tick();
  update_encoder_readings();
  update_sensors_magics();
  update_battery_voltage();
  check_leds_while();
  check_buttons();
  lsm6dsr_update();
}

int main(void) {
  setup();
  eeprom_load();
  show_battery_level();

  printf("BA: %4d CI: %4d CD: %4d BO: %4d\n", get_aux_raw(AUX_BATTERY_ID), get_aux_raw(AUX_CURRENT_LEFT_ID), get_aux_raw(AUX_CURRENT_RIGHT_ID), get_aux_raw(AUX_MENU_BTN_ID));

  while (1) {
    if (!is_race_started()) {
      menu_handler();
      if (!get_sensors_enabled() && menu_run_can_start()) {
        set_sensors_enabled(menu_run_can_start());
        delay(200);
      } else {
        set_sensors_enabled(menu_run_can_start());
      }
      if (menu_run_can_start()) {
        int8_t sensor_started = check_start_run();
        if (is_race_started()) {
          switch (menu_run_get_explore_algorithm()) {
            case EXPLORE_HANDWALL:
              switch (sensor_started) {
                case SENSOR_FRONT_LEFT_WALL_ID:
                  handwall_use_left_hand();
                  handwall_start();
                  break;
                case SENSOR_FRONT_RIGHT_WALL_ID:
                  handwall_use_right_hand();
                  handwall_start();
                  break;
              }
              break;
            case EXPLORE_FLOODFILL:
              switch (sensor_started) {
                case SENSOR_FRONT_LEFT_WALL_ID:
                  floodfill_start_run();
                  break;
                case SENSOR_FRONT_RIGHT_WALL_ID:
                  floodfill_start_explore();
                  break;
              }
              break;
            case EXPLORE_TIME_TRIAL:
              timetrial_start();
              break;
            default:
              set_race_started(false);
              break;
          }
        }
      }
    } else {
      switch (menu_run_get_explore_algorithm()) {
        case EXPLORE_HANDWALL:
          handwall_loop();
          break;
        case EXPLORE_FLOODFILL:
          floodfill_loop();
          break;
        case EXPLORE_TIME_TRIAL:
          timetrial_loop();
          break;
        default:
          set_race_started(false);
          break;
      }
    }
  }

  // ZONA DEBUG TEMPORAL

  while (1) {
    // LED WARNING
    // warning_status_led(125);

    // LEDS MENU
    // set_leds_wave(125);

    // LED RGB
    // set_RGB_color(20, 0, 0);
    // delay(250);
    // set_RGB_color(0, 20, 0);
    // delay(250);
    // set_RGB_color(0, 0, 20);
    // delay(250);
    // set_RGB_color(LEDS_MAX_PWM, LEDS_MAX_PWM, LEDS_MAX_PWM);
    // set_RGB_rainbow();

    // MPU
    // printf("MPU: 0x%02X\t", lsm6dsr_who_am_i());
    // printf("Z(raw): %6d Z(radps): %6.4f Z(dps): %6.4f Z(deg): %6.4f\t", lsm6dsr_get_gyro_z_raw(), lsm6dsr_get_gyro_z_radps(), lsm6dsr_get_gyro_z_dps(), lsm6dsr_get_gyro_z_degrees());
    // printf("Z(raw): %6d\t", lsm6dsr_get_gyro_z_raw());
    // delay(500);

    // VENTILADOR
    //  set_fan_speed(50);

    // MOTORES
    // gpio_set(GPIOB, GPIO15);
    // set_motors_speed(20, 20);

    // SENSORES
    // printf("S1: %4d S2: %4d S3: %4d S4: %4d\t", get_sensor_raw(SENSOR_FRONT_LEFT_WALL_ID, true), get_sensor_raw(SENSOR_FRONT_RIGHT_WALL_ID, true), get_sensor_raw(SENSOR_SIDE_LEFT_WALL_ID, true), get_sensor_raw(SENSOR_SIDE_RIGHT_WALL_ID, true));

    // AUX ANALÓGICOS
    // printf("BA: %4d CI: %4d CD: %4d BO: %4d\t", get_aux_raw(AUX_BATTERY_ID), get_aux_raw(AUX_CURRENT_LEFT_ID), get_aux_raw(AUX_CURRENT_RIGHT_ID), get_aux_raw(AUX_MENU_BTN_ID));
    // printf("BA: %.2f\n", get_battery_voltage());

    // ENCODERS
    // printf("L: %ld R: %ld\t", get_encoder_left_millimeters(), get_encoder_right_millimeters());

    // printf("\n");
    // delay(50);
  }

  return 0;
}