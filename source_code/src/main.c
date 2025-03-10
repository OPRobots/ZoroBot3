#include <battery.h>
#include <buttons.h>
#include <control.h>
#include <delay.h>
#include <encoders.h>
#include <floodfill.h>
#include <handwall.h>
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
  // update_sensors_magics();
  update_battery_voltage();
  check_leds_while();
  check_buttons();
  lsm6dsr_update();
}

int main(void) {
  setup();
  // eeprom_load();

  while (1) {
    // warning_status_led(125);
    set_leds_wave(125);
    // LEDS MENU
    // for (uint8_t i = 0; i < 10; i++) {
    //   clear_info_leds();
    //   set_info_led(i, true);
    //   printf("LED %i\n", i);
    //   delay(250);
    // }

    // LED RGB
    // set_RGB_color(20, 0, 0);
    // delay(250);
    // set_RGB_color(0, 20, 0);
    // delay(250);
    // set_RGB_color(0, 0, 20);
    // delay(250);
    // set_RGB_color(LEDS_MAX_PWM, LEDS_MAX_PWM, LEDS_MAX_PWM);
    // set_info_leds();
    // set_status_led(true);
    // set_RGB_rainbow();

    // MPU
    // printf("MPU: 0x%02X\t", lsm6dsr_who_am_i());
    // printf("MPU: 0x%02X\t", lsm6dsr_read_register(WHO_AM_I_ADDR));
    // printf("Z(raw): %6d Z(radps): %6.4f Z(dps): %6.4f Z(deg): %6.4f\t", lsm6dsr_get_gyro_z_raw(), lsm6dsr_get_gyro_z_radps(), lsm6dsr_get_gyro_z_dps(), lsm6dsr_get_gyro_z_degrees());
    // printf("Z(raw): %6d\t", lsm6dsr_get_gyro_z_raw());
    // delay(500);

    // VENTILADOR
    //  set_fan_speed(50);

    // MOTORES
    // gpio_set(GPIOB, GPIO15);
    // set_motors_speed(20, 20);

    // gpio_clear(GPIOA, GPIO0); // EMITTER OFF
    // gpio_clear(GPIOA, GPIO3); // EMITTER OFF
    // gpio_clear(GPIOA, GPIO1); // EMITTER OFF
    // gpio_clear(GPIOA, GPIO2); // EMITTER OFF

    // SENSORES
    // printf("S1: %4d S2: %4d S3: %4d S4: %4d\t", get_sensor_raw(SENSOR_FRONT_LEFT_WALL_ID, true), get_sensor_raw(SENSOR_FRONT_RIGHT_WALL_ID, true), get_sensor_raw(SENSOR_SIDE_LEFT_WALL_ID, true), get_sensor_raw(SENSOR_SIDE_RIGHT_WALL_ID, true));

    // AUX ANALÓGICOS
    // printf("BA: %4d CI: %4d CD: %4d BO: %4d\t", get_aux_raw(AUX_BATTERY_ID), get_aux_raw(AUX_CURRENT_LEFT_ID), get_aux_raw(AUX_CURRENT_RIGHT_ID), get_aux_raw(AUX_MENU_BTN_ID));
    // printf("BA: %.2f\n", get_battery_voltage());

    // ENCODERS
    printf("L: %ld R: %ld\t", get_encoder_left_millimeters(), get_encoder_right_millimeters());

    // printf("PATO\n");
    // gpio_toggle(GPIOB, GPIO13);

    printf("\n");
    delay(50);
  }

  show_battery_level();

  while (1) {
    if (!is_race_started()) {
      menu_handler();
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
        default:
          set_race_started(false);
          break;
      }
    }

    // ZONA DEBUG TEMPORAL
    // int16_t distance = get_front_wall_distance() - ((CELL_DIMENSION - WALL_WIDTH / 2) + SENSING_POINT_DISTANCE);
    // printf("%d\n", abs(distance)<=5?0:distance);
    // delay(100);

    // LOG ERROR LATERAL
    // get_side_sensors_close_error();
    // printf("%4d - %4d\n", get_side_sensors_close_error(true), get_side_sensors_far_error(true));
    // delay(100);

    // LOG MPU DEG
    // printf("%.4f\n", lsm6dsr_get_gyro_z_degrees());
    // delay(100);

    // MOVIMIENTO RECTO

    // VELOCIDAD LINEAL
    // if (get_clock_ticks() - ticks <= 100) {
    //   set_target_linear_speed(100);
    //   // printf("%d\n", 100);
    // } else if (get_clock_ticks() - ticks <= 400) {
    //   set_target_linear_speed(200);
    //   // printf("%d\n", 200);
    // } else if (get_clock_ticks() - ticks <= 700) {
    //   set_target_linear_speed(0);
    //   // printf("%d\n", 0);
    // } else {
    // set_race_started(false);
    // set_status_led(false);
    // warning_status_led(125);
    // }

    // VELOCIDAD LINEAL Y ANGULAR
    // if (get_clock_ticks() - ticks <= 500) {
    //   set_target_linear_speed(500);
    //   set_ideal_angular_speed(0);
    //   // printf("%d\n", 200);
    // } else {
    //   set_race_started(false);
    //   set_status_led(false);
    // }
    // // if (is_race_started()) {
    // //   control_debug();
    // // }
    // // // set_motors_speed(80,80);
    // // delay(1);
    // if (get_menu_mode_btn()) {
    //   while (get_menu_mode_btn())
    //     ;
    //   macroarray_print();
    // }

    // BATERÍA
    //  printf("%.2f\n", get_battery_voltage());

    // ENCODERS
    // printf("%ld (%ld)\t%ld (%ld) | %.4f %.4f %.4f \n", get_encoder_total_left_micrometers(), get_encoder_left_millimeters(), get_encoder_total_right_micrometers(), get_encoder_right_millimeters(), get_encoder_avg_speed(), get_encoder_angular_speed(), get_encoder_curernt_angle());
    // printf("%.2f\n", get_encoder_avg_speed());
    // printf("%ld - %ld\n", get_encoder_left_ticks(),get_encoder_right_ticks());
    // delay(150);

    // 321194,5 -> 3000000
    // 1 -> x
    // SENSORES MOVIENDO EL ROBOT MANUALMENTE
    // static uint8_t count = 0;
    // if (get_encoder_average_micrometers()/10000 >= count || count == 0) {
    // printf("%4d\t", get_sensor_distance(SENSOR_FRONT_LEFT_WALL_ID));
    // // printf("(%4d)\t", get_sensor_linearized(SENSOR_FRONT_LEFT_WALL_ID));
    // printf("%4d\t", get_sensor_distance(SENSOR_FRONT_RIGHT_WALL_ID));
    // // printf("(%4d)\t", get_sensor_linearized(SENSOR_FRONT_RIGHT_WALL_ID));
    // // // printf("%4d\t", (get_sensor_distance(SENSOR_FRONT_LEFT_WALL_ID)+get_sensor_distance(SENSOR_FRONT_RIGHT_WALL_ID))/2);
    // // printf("%4d\t", get_sensor_distance(SENSOR_SIDE_LEFT_WALL_ID));
    // // printf("%4d\t", get_sensor_distance(SENSOR_SIDE_RIGHT_WALL_ID));
    // printf("\n");
    // delay(100);
    // count++;
    // }

    // SENSORES PULSADO EL BOTÓN DE MENÚ
    // if (get_menu_mode_btn()) {
    //   while (get_menu_mode_btn())
    //     ;
    // printf("%4d\t", get_sensor_distance(SENSOR_FRONT_LEFT_WALL_ID)-get_sensor_distance(SENSOR_FRONT_RIGHT_WALL_ID));
    // if (!is_race_started()) {
    //   printf("%4d\t", get_sensor_distance(SENSOR_FRONT_LEFT_WALL_ID));
    //   printf("%4d\t", get_sensor_distance(SENSOR_FRONT_RIGHT_WALL_ID));
    //   printf("%4d\t", get_sensor_distance(SENSOR_SIDE_LEFT_WALL_ID));
    //   printf("%4d\t", get_sensor_distance(SENSOR_SIDE_RIGHT_WALL_ID));
    //   printf("\n");
    //   delay(100);
    // }
    // }

    // printf("%.3f  (%d)\t%.3f  (%d)\t%.3f  (%d)\t%.3f  (%d)\t\n", get_sensor_log(0), get_sensor_raw_filter(0),  get_sensor_log(1), get_sensor_raw_filter(1),  get_sensor_log(2), get_sensor_raw_filter(2),  get_sensor_log(3), get_sensor_raw_filter(3));

    // delay(100);

    // printf("%d \t%d \t\n", get_sensor_raw_filter(0), get_sensor_raw_filter(1)/* , get_sensor_raw_filter(2), get_sensor_raw_filter(3) */);
    // printf("%.2f\n", get_battery_voltage());

    // uint16_t on[NUM_SENSORES], off[NUM_SENSORES];

    // get_sensors_raw(on, off);
    //   printf("%.3f - %.3f - %.3f - %.3f\n",sensors_raw_log(on[0], off[0]),sensors_raw_log(on[1], off[1]),sensors_raw_log(on[2], off[2]),sensors_raw_log(on[3], off[3]));
    // delay(50);
  }

  return 0;
}