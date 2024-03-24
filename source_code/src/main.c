#include <battery.h>
#include <buttons.h>
#include <control.h>
#include <delay.h>
#include <encoders.h>
#include <leds.h>
#include <menu.h>
#include <motors.h>
#include <mpu.h>
#include <sensors.h>
#include <setup.h>
#include <usart.h>

void sys_tick_handler(void) {
  clock_tick();
  update_encoder_readings();
  update_sensors_magics();
  update_gyro_readings();
}

int main(void) {
  setup();
  show_battery_level();
  delay(1500);
  gyro_z_calibration();
  
  while (1) {
    // printf("%0x\n", mpu_who_am_i());
    // printf("%d \n", (int)get_gyro_z_degrees());
    // set_z_angle(0);
    // delay(100);

    // ZONA DEBUG TEMPORAL

    // Lectura de encoders
    // printf("%ld (%ld)\t%ld (%ld)\n", get_encoder_left_total_ticks(), get_encoder_left_micrometers()/10000, get_encoder_right_total_ticks(), get_encoder_right_micrometers()/10000);

    // SENSORES FRONTALES MOVIENDO EL ROBOT MANUALMENTE
    // static uint8_t count = 0;
    // if (get_encoder_average_micrometers()/10000 >= count || count == 0) {
      printf("%d (%d)\t", get_sensor_distance(SENSOR_FRONT_LEFT_WALL_ID), get_sensor_linearized(SENSOR_FRONT_LEFT_WALL_ID));
      printf("%d (%d)\t", get_sensor_distance(SENSOR_FRONT_RIGHT_WALL_ID), get_sensor_linearized(SENSOR_FRONT_RIGHT_WALL_ID));
      // printf("%4d ", get_sensor_filtered(SENSOR_SIDE_RIGHT_WALL_ID));
      // printf("%4d ", get_sensor_filtered(SENSOR_SIDE_LEFT_WALL_ID));
      printf("\n");
      // count++;
    // }

    // if (get_menu_mode_btn()) {
    //   while (get_menu_mode_btn())
    //     ;
    //   // printf("%4d\t", get_sensor_linearized(SENSOR_SIDE_LEFT_WALL_ID));
    //   printf("%4d\t", get_sensor_linearized(SENSOR_SIDE_RIGHT_WALL_ID));
    //   printf("\n");
      // delay(1000);
    // }

    // printf("%.3f  (%d)\t%.3f  (%d)\t%.3f  (%d)\t%.3f  (%d)\t\n", get_sensor_log(0), get_sensor_raw_filter(0),  get_sensor_log(1), get_sensor_raw_filter(1),  get_sensor_log(2), get_sensor_raw_filter(2),  get_sensor_log(3), get_sensor_raw_filter(3));

    delay(100);

    // printf("%d \t%d \t\n", get_sensor_raw_filter(0), get_sensor_raw_filter(1)/* , get_sensor_raw_filter(2), get_sensor_raw_filter(3) */);
    // printf("%.2f\n", get_battery_voltage());

    // uint16_t on[NUM_SENSORES], off[NUM_SENSORES];

    // get_sensors_raw(on, off);
    //   printf("%.3f - %.3f - %.3f - %.3f\n",sensors_raw_log(on[0], off[0]),sensors_raw_log(on[1], off[1]),sensors_raw_log(on[2], off[2]),sensors_raw_log(on[3], off[3]));
    // delay(50);
  }

  return 0;
}