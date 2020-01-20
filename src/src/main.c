#include <delay.h>
#include <encoders.h>
#include <setup.h>
#include <leds.h>


void sys_tick_handler(void) {
  clock_tick();
  update_encoder_readings();
}

int main(void) {
  setup();
// gpio_clear(GPIOA, GPIO12);
  while (1) {
    // warning_status_led(250);
    // gpio_toggle(GPIOA, GPIO12);	/* LED on/off */
		// for (int i = 0; i < 1000000; i++) {	/* Wait a bit. */
		// 	__asm__("nop");
		// }
  }
}