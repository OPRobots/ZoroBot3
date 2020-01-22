#include <delay.h>
#include <encoders.h>
#include <leds.h>
#include <mpu.h>
#include <setup.h>
#include <usart.h>

void sys_tick_handler(void) {
  clock_tick();
  update_encoder_readings();
}

int main(void) {
  setup();
  gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);
  printf("Todo listo\n");
  // gpio_clear(GPIOA, GPIO12);
      gpio_set(GPIOA, GPIO1);
  while (1) {
    // warning_status_led(250);

    // gpio_toggle(GPIOA, GPIO1);	/* LED on/off */
    // for (int i = 0; i < 1000000; i++) {	/* Wait a bit. */
    // 	__asm__("nop");
    // }
    printf("%d\n", get_gyro_z_raw());
    if (get_gyro_z_raw() != 0) {

      gpio_clear(GPIOA, GPIO1);
    } else {

      // gpio_set(GPIOA, GPIO1);
    }
    // delay(200);
  }
}