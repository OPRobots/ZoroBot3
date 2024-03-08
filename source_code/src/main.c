#include <basic_algorithm.h>
#include <basic_debug.h>
#include <basic_wall_change.h>
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

bool debug = false;
uint16_t debug_objetivo_I = 0;
uint16_t debug_objetivo_D = 0;

void sys_tick_handler(void) {
  clock_tick();
  // update_distance_readings();
  // update_encoder_readings();
  update_gyro_readings();
  set_z_angle(0);
}

int main(void) {
  setup();
  delay(1500);
  gyro_z_calibration();
  mpu_set_updating(true);

  while (1) {
    // printf("%0x\n", mpu_who_am_i());
    // printf("%d | ", (int)get_gyro_z_degrees());
    // set_z_angle(0);
    // delay(100);
  }

  // mpu_set_updating(true);
  if (get_menu_mode_btn()) {
    while (get_menu_mode_btn()) {
    }
    debug = true;
    set_status_led(true);
  }

  show_battery_level();
  while (!configuracion()) {
  }

  if (!debug) {
    basic_algorithm_config();
  }
  while (1) {
    if (debug) {
      debug_inicio();
      if (debug_objetivo_D == 0 || debug_objetivo_I == 0) {
        debug_objetivo_I = sensor3_analog();
        debug_objetivo_D = sensor1_analog();
      }
      printf("\t\t\t\t%4d\t%4d\n", debug_objetivo_I - sensor3_analog(), debug_objetivo_D - sensor1_analog());
      delay(10);
      continue;
    }

    if (!is_competicion_iniciada()) {
      start_from_front_sensor();
      // start_from_ms();
      set_motors_speed(0, 0);
    } else {
      basic_algorithm_loop();
    }

    // ZONA DEBUG TEMPORAL

    // Lectura de encoders
    // printf("%ld (%ld)\t%ld (%ld)\n", get_encoder_left_total_ticks(), get_encoder_left_micrometers(), get_encoder_right_total_ticks(), get_encoder_right_micrometers());

    // printf("%.3f  (%d)\t%.3f  (%d)\t%.3f  (%d)\t%.3f  (%d)\t\n", get_sensor_log(0), get_sensor_raw_filter(0),  get_sensor_log(1), get_sensor_raw_filter(1),  get_sensor_log(2), get_sensor_raw_filter(2),  get_sensor_log(3), get_sensor_raw_filter(3));

    // printf("%d \t", get_sensor_raw_filter(SENSOR_SIDE_LEFT_ID)); //1
    // printf("%d \t", get_sensor_raw_filter(SENSOR_SIDE_RIGHT_ID)); //4
    // printf("%d \t", get_sensor_raw_filter(SENSOR_FRONT_LEFT_ID)); //2
    // printf("%d \t", get_sensor_raw_filter(SENSOR_FRONT_RIGHT_ID)); //3
    // printf("\n");

    // printf("%d \t%d \t\n", get_sensor_raw_filter(0), get_sensor_raw_filter(1)/* , get_sensor_raw_filter(2), get_sensor_raw_filter(3) */);
    // printf("%.2f\n", get_battery_voltage());

    // uint16_t on[NUM_SENSORES], off[NUM_SENSORES];

    // get_sensors_raw(on, off);
    //   printf("%.3f - %.3f - %.3f - %.3f\n",sensors_raw_log(on[0], off[0]),sensors_raw_log(on[1], off[1]),sensors_raw_log(on[2], off[2]),sensors_raw_log(on[3], off[3]));
    // delay(50);
  }
  return 0;
}