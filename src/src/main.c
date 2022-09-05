#include <battery.h>
#include <delay.h>
#include <motors.h>
#include <encoders.h>
#include <leds.h>
#include <mpu.h>
#include <sensors.h>
#include <setup.h>
#include <usart.h>
#include <control.h>
#include <buttons.h>
#include <menu.h>

void sys_tick_handler(void) {
  clock_tick();
  update_encoder_readings();
  update_gyro_readings();
}

int main(void) {
  setup();
  gyro_z_calibration();
  mpu_set_updating(true);
  show_battery_level();
  while (1) {

    if(!is_competicion_iniciada()){
      check_menu_button();


    }else{
      // TODO: cosas de competi iniciada
    }
 
    // printf("%d\t%d\t%d\t%d\t\n", get_sensor_raw(0, 1),  get_sensor_raw(1, 1),  get_sensor_raw(2, 1),  get_sensor_raw(3, 1));
    delay(50);
  }
  return 0;
}