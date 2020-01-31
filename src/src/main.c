#include <battery.h>
#include <delay.h>
#include <encoders.h>
#include <leds.h>
#include <mpu.h>
#include <setup.h>
#include <usart.h>
#include <sensors.h>


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
gpio_set(GPIOA, GPIO1);
    gpio_clear(GPIOA, GPIO0 | GPIO2 | GPIO3);
  while (1) {
    // printf("%d - %d\n", mpu_who_am_i(), mpu_read_gyro_z_raw());
    // printf("%.4f\n", get_gyro_z_degrees());
    // delay_us(50000);
    // set_leds_wave(200);
    // set_RGB_rainbow();

    // gpio_set(GPIOA, GPIO0 | GPIO1 | GPIO2 | GPIO3);
    // delay(2000);

    // gpio_clear(GPIOA, GPIO0 | GPIO1 | GPIO2 | GPIO3);
    // delay(2000);

    /* gpio_set(GPIOA, GPIO0);
    gpio_clear(GPIOA, GPIO1 | GPIO2 | GPIO3);
    delay(1000);
    gpio_set(GPIOA, GPIO1);
    gpio_clear(GPIOA, GPIO0 | GPIO2 | GPIO3);
    delay(1000);
    gpio_set(GPIOA, GPIO2);
    gpio_clear(GPIOA, GPIO0 | GPIO1 | GPIO3);
    delay(1000);
    gpio_set(GPIOA, GPIO3);
    gpio_clear(GPIOA, GPIO0 | GPIO1 | GPIO2);
    delay(1000); */

    // printf("V= %.2fv\n", get_battery_voltage());
    // delay(250);

    printf("%d\n", get_sensor_raw(1)/* ,  get_sensor_raw(1),  get_sensor_raw(2),  get_sensor_raw(3) */);
    delay(50);
    // set_RGB_rainbow();
  }
  return 0;
}