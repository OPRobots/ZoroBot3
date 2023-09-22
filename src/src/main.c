#include <basic_algorithm.h>
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
#include <walls.h>
#include <mpu.h>


void sys_tick_handler(void) {
  clock_tick();
  update_distance_readings();
  update_encoder_readings();
  update_gyro_readings();
}

int main(void) {
  setup();
  gyro_z_calibration();
  mpu_set_updating(true);
  show_battery_level();

  basic_algorithm_init();
  while (1) {

    // check_start_stop_module();
    check_start_front_sensor();

    // printf("%d\n", get_sensor_raw_filter(SENSOR_FRONT_RIGHT_ID));

    if (!is_competicion_iniciada()) {
      check_menu_button();
      set_motors_speed(0, 0);
      // set_fan_speed(0);
    } else {
      // printf("pato\n");
      basic_algorithm_loop();
      delay_us(500);
    }

    // ZONA DEBUG TEMPORAL

    // Lectura de encoders
    // printf("%ld (%ld)\t%ld (%ld)\n", get_encoder_left_total_ticks(), get_encoder_left_micrometers(), get_encoder_right_total_ticks(), get_encoder_right_micrometers());

    // printf("%.3f  (%d)\t%.3f  (%d)\t%.3f  (%d)\t%.3f  (%d)\t\n", get_sensor_log(0), get_sensor_raw_filter(0),  get_sensor_log(1), get_sensor_raw_filter(1),  get_sensor_log(2), get_sensor_raw_filter(2),  get_sensor_log(3), get_sensor_raw_filter(3));

    //printf("%d \t", get_sensor_raw_filter(SENSOR_SIDE_LEFT_ID)); //1
    //printf("%d \t", get_sensor_raw_filter(SENSOR_SIDE_RIGHT_ID)); //4
    //printf("%d \t", get_sensor_raw_filter(SENSOR_FRONT_LEFT_ID)); //2
    //printf("%d \t", get_sensor_raw_filter(SENSOR_FRONT_RIGHT_ID)); //3
    //printf("\n");

    // printf("%d \t%d \t\n", get_sensor_raw_filter(0), get_sensor_raw_filter(1)/* , get_sensor_raw_filter(2), get_sensor_raw_filter(3) */);
    // printf("%.2f\n", get_battery_voltage());

    // uint16_t on[NUM_SENSORES], off[NUM_SENSORES];

    // get_sensors_raw(on, off);
    //   printf("%.3f - %.3f - %.3f - %.3f\n",sensors_raw_log(on[0], off[0]),sensors_raw_log(on[1], off[1]),sensors_raw_log(on[2], off[2]),sensors_raw_log(on[3], off[3]));
    //delay(50);
  }
  return 0;
}