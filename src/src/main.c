#include <delay.h>
#include <encoders.h>
#include <leds.h>
#include <mpu.h>
#include <setup.h>
#include <usart.h>

void sys_tick_handler(void) {
  clock_tick();
  update_encoder_readings();
  update_gyro_readings();
}

int main(void) {
  setup();
  gyro_z_calibration();
  mpu_set_updating(true);

  while (1) {
    // printf("%d - %d\n", mpu_who_am_i(), mpu_read_gyro_z_raw());
    printf("%.4f\n", get_gyro_z_degrees());
    delay_us(50000);
  }
  return 0;
}