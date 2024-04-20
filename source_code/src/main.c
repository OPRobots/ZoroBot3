#include <battery.h>
#include <buttons.h>
#include <control.h>
#include <delay.h>
#include <encoders.h>
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
}

int main(void) {
  setup();
  show_battery_level();
  delay(1500);
  gyro_z_calibration();
  sensors_calibration();

  // uint32_t ticks = get_clock_ticks();
  // set_status_led(true);

  // set_target_linear_speed(0);
  // set_ideal_angular_speed(0);
  // set_front_sensors_correction(false);
  // set_side_sensors_close_correction(false);
  // set_side_sensors_far_correction(false);
  // move_arc_turn(MOVE_LEFT);
  // move_straight(100, 500, true);
  // move_straight(850, 1500, false);
  // move_straight(250, 500, true);
  // // move_straight(250, 500, false);
  // move_straight(250, 200, true);
  // move_straight(100, 100, false);
  // move_straight(45, 200, false);
  // move_straight(90, 200, false);
  // move_straight(95, 200, false);
  // // move_straight(100, 200, true);
  // move_inplace_turn(90.0, 10);
  // move_straight(180, 200, true);

  // set_competicion_iniciada(false);

  while (1) {

    if (!is_competicion_iniciada()) {
      bool start_sensor = false;
      while (!start_sensor) {
        set_RGB_rainbow();
        if (get_sensor_distance(SENSOR_FRONT_LEFT_WALL_ID) <= 130) {
          set_RGB_color(0, 50, 0);
          delay(2000);
          start_sensor = true;
          set_RGB_color(0, 0, 0);
        }
      }
      set_competicion_iniciada(true);
      set_front_sensors_correction(false);
      set_side_sensors_close_correction(true);
      set_side_sensors_far_correction(true);
      move_straight(70 + 46.36, 500, false);
    } else {
      // Loop de competición
      struct walls walls = get_walls();
      if (walls.front && walls.left && walls.right) {
        move_straight(100, 500, true);
        set_competicion_iniciada(false);
      } else if (!walls.right) {
        set_side_sensors_close_correction(false);
        set_side_sensors_far_correction(false);
        move_arc_turn(MOVE_RIGHT);
        set_side_sensors_close_correction(true);
        set_side_sensors_far_correction(true);
      } else if (!walls.left) {
        set_side_sensors_close_correction(false);
        set_side_sensors_far_correction(false);
        move_arc_turn(MOVE_LEFT);
        set_side_sensors_close_correction(true);
        set_side_sensors_far_correction(true);
      } else if (!walls.front) {
        set_side_sensors_close_correction(true);
        set_side_sensors_far_correction(true);
        move_straight(180, 500, false);
      } else {
        set_competicion_iniciada(false);
      }
    }

    // set_motors_pwm(512,512);
    // printf("%0x\n", mpu_who_am_i());
    // printf("%d \n", (int)get_gyro_z_degrees());
    // set_z_angle(0);
    // delay(100);

    // ZONA DEBUG TEMPORAL

    // LOG ERROR LATERAL
    // get_side_sensors_close_error();
    // printf("%4d - %4d\n", get_side_sensors_close_error(), get_side_sensors_far_error());
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
    warning_status_led(125);
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
    // printf("%ld (%ld)\t%ld (%ld) | %.4f %.4f %.4f \n", get_encoder_total_left_micrometers(), get_encoder_total_left_millimeters(), get_encoder_total_right_micrometers(), get_encoder_total_right_millimeters(), get_encoder_avg_speed(), get_encoder_angular_speed(), get_encoder_curernt_angle());
    // printf("%.2f\n", get_encoder_avg_speed());
    // delay(150);

    // SENSORES MOVIENDO EL ROBOT MANUALMENTE
    // static uint8_t count = 0;
    // if (get_encoder_average_micrometers()/10000 >= count || count == 0) {
    // printf("%4d\t", get_sensor_distance(SENSOR_FRONT_LEFT_WALL_ID));
    // printf("%4d", get_sensor_distance(SENSOR_FRONT_RIGHT_WALL_ID));
    // printf("%4d\t", get_sensor_distance(SENSOR_SIDE_LEFT_WALL_ID));
    // printf("%4d\t", get_sensor_distance(SENSOR_SIDE_RIGHT_WALL_ID));
    // printf("\n");
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