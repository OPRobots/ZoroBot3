#include <motors.h>

void set_motors_speed(float velI, float velD) {
  float ocI = 0;
  float ocD = 0;

  if (velI != 0) {
    ocI = map(abs(velI), 0, 100, 0, MOTORES_MAX_PWM);
    if (velI > 0) {
      gpio_set(GPIOB, GPIO14);
      gpio_clear(GPIOB, GPIO15);
    } else {
      gpio_set(GPIOB, GPIO15);
      gpio_clear(GPIOB, GPIO14);
    }
  }

  if (velD != 0) {
    ocD = map(abs(velD), 0, 100, 0, MOTORES_MAX_PWM);
    if (velD > 0) {
      gpio_set(GPIOB, GPIO13);
      gpio_clear(GPIOB, GPIO12);
    } else {
      gpio_set(GPIOB, GPIO12);
      gpio_clear(GPIOB, GPIO13);
    }
  }
  printf("%ld - %ld\n", (uint32_t)ocI, (uint32_t)ocD);
  timer_set_oc_value(TIM8, TIM_OC4, (uint32_t)ocI);
  timer_set_oc_value(TIM8, TIM_OC3, (uint32_t)ocD);
}

void set_fan_speed(uint8_t vel) {
  uint32_t ocF = 0;
  if (vel != 0) {
    ocF = map(abs(vel), 0, 100, 0, MOTORES_MAX_PWM);
  }
  printf("%ld\n", ocF);
  timer_set_oc_value(TIM8, TIM_OC2, ocF);
}
