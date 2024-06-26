#include <battery.h>
#include <buttons.h>
#include <control.h>
#include <delay.h>
#include <encoders.h>
#include <floodfill.h>
#include <handwall.h>
#include <leds.h>
#include <macroarray.h>
#include <menu.h>
#include <motors.h>
#include <move.h>
#include <mpu.h>
#include <sensors.h>
#include <setup.h>
#include <usart.h>

void sys_tick_handler(void) {
  clock_tick();
  update_encoder_readings();
  update_sensors_magics();
  update_gyro_readings();
  update_battery_voltage();
  check_leds_while();
  check_buttons();
}

int main(void) {
  setup();
  show_battery_level();
  eeprom_load();
  delay(1500);

  // floodfill_debug_update_walls(0, true, true, true, false);
  // floodfill_debug_update_walls(7, false, false, true, true);
  // floodfill_debug_update_walls(8, false, true, false, true);

  while (1) {
    if (!is_competicion_iniciada()) {
      if (!check_menu_button()) {
        switch (check_iniciar_competicion()) {
          case SENSOR_FRONT_LEFT_WALL_ID:
            floodfill_use_left_hand();
            break;
          case SENSOR_FRONT_RIGHT_WALL_ID:
            floodfill_use_right_hand();
            break;
        }

        if (is_competicion_iniciada()) {
          // set_front_sensors_correction(true);
          floodfill_set_time_limit(30000);
          // handwall_set_time_limit(30000);
          floodfill_start();
          // move(MOVE_START);
          // move(MOVE_FRONT);
          // move(MOVE_FRONT);
          // move(MOVE_FRONT);
          // move(MOVE_LEFT);
          // move(MOVE_180W);
          // move_straight(CELL_DIMENSION/2, 300, false, true);
          // delay(50);
          // set_side_sensors_close_correction(true);
          // set_side_sensors_far_correction(true);
          // move_straight(60, 500, false, true);
          // move(MOVE_LEFT);

          // delay(100);
          // move_inplace_turn(MOVE_180);
          // set_side_sensors_close_correction(true);
          // set_side_sensors_far_correction(true);
          // move_straight(4*CELL_DIMENSION - ROBOT_FRONT_LENGTH - ROBOT_BACK_LENGTH, 500, true);
          // delay(100);
          // move_inplace_turn(MOVE_180);
          // delay(500);
          // set_competicion_iniciada(false);
        } else {
          // update_side_sensors_leds();
          // set_motors_brake();
          // printf("%.4f\n", get_gyro_z_radps());
        }
      }
    } else {
      // Loop de competición
      floodfill_loop();
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
    // printf("%.4f\n", get_gyro_z_degrees());
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
    // set_competicion_iniciada(false);
    // set_status_led(false);
    // warning_status_led(125);
    // }

    // VELOCIDAD LINEAL Y ANGULAR
    // if (get_clock_ticks() - ticks <= 500) {
    //   set_target_linear_speed(500);
    //   set_ideal_angular_speed(0);
    //   // printf("%d\n", 200);
    // } else {
    //   set_competicion_iniciada(false);
    //   set_status_led(false);
    // }
    // // if (is_competicion_iniciada()) {
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
    // printf("%4d\t", get_sensor_distance(SENSOR_FRONT_LEFT_WALL_ID));
    // printf("%4d\t", get_sensor_distance(SENSOR_FRONT_RIGHT_WALL_ID));
    // printf("%4d\t", get_sensor_distance(SENSOR_SIDE_LEFT_WALL_ID));
    // printf("%4d\t", get_sensor_distance(SENSOR_SIDE_RIGHT_WALL_ID));
    // printf("\n");
    // delay(100);
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