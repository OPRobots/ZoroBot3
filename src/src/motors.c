#include <motors.h>

void set_motors_speed(float velI, float velD) {
  float ocI = 0;
  float ocD = 0;

  if (velI != 0) {
    if (velI > 0) {
      ocI += map(velI, 0, 100, 0, MOTORES_MAX_PWM);
      gpio_set(GPIOB, GPIO15);
      gpio_clear(GPIOB, GPIO14);
    } else {
      ocI -= map(abs(velI), 0, 100, 0, MOTORES_MAX_PWM);
      gpio_set(GPIOB, GPIO14);
      gpio_clear(GPIOB, GPIO15);
    }
  }

  if (velD != 0) {
    if (velD > 0) {
      ocD += map(velD, 0, 100, 0, MOTORES_MAX_PWM);
      gpio_set(GPIOB, GPIO12);
      gpio_clear(GPIOB, GPIO13);
    } else {
      ocD -= map(abs(velD), 0, 100, 0, MOTORES_MAX_PWM);
      gpio_set(GPIOB, GPIO13);
      gpio_clear(GPIOB, GPIO12);
    }
  }
  timer_set_oc_value(TIM8, TIM_OC4, (uint32_t)ocI);
  timer_set_oc_value(TIM8, TIM_OC3, (uint32_t)ocD);
}

void set_fan_speed(uint8_t vel) {
  uint32_t ocF = 0;
  if (vel != 0) {
    if (vel > 0) {
      ocF -= map(vel, 0, 100, 0, MOTORES_MAX_PWM);
    } else {
      ocF += map(abs(vel), 0, 100, 0, MOTORES_MAX_PWM);
    }
  }
  timer_set_oc_value(TIM8, TIM_OC2, ocF);
}
