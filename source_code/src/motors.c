#include <motors.h>

static bool motors_saturated = false;
static uint32_t motors_saturated_ms = 0;
static uint16_t left_motor_saturation_count = 0;
static uint16_t right_motor_saturation_count = 0;

static void check_motors_saturated(void) {
  if (left_motor_saturation_count > MAX_MOTOR_SATURATION_COUNT || right_motor_saturation_count > MAX_MOTOR_SATURATION_COUNT) {
    if (!motors_saturated) {
      motors_saturated_ms = get_clock_ticks();
    }
    motors_saturated = true;
  } else {
    motors_saturated = false;
  }
}

void set_motors_speed(float velI, float velD) {
  float ocI = 0;
  float ocD = 0;

  if (velI != 0) {
    ocI = map(abs(velI), 0, 1000, 0, MOTORES_MAX_PWM);
    if (velI > 0) {
      gpio_set(GPIOB, GPIO14);
      gpio_clear(GPIOB, GPIO15);
    } else {
      gpio_set(GPIOB, GPIO15);
      gpio_clear(GPIOB, GPIO14);
    }
  }

  if (velD != 0) {
    ocD = map(abs(velD), 0, 1000, 0, MOTORES_MAX_PWM);
    if (velD > 0) {
      gpio_set(GPIOB, GPIO13);
      gpio_clear(GPIOB, GPIO12);
    } else {
      gpio_set(GPIOB, GPIO12);
      gpio_clear(GPIOB, GPIO13);
    }
  }
  // printf("%ld - %ld\n", (uint32_t)ocI, (uint32_t)ocD);
  timer_set_oc_value(TIM8, TIM_OC4, (uint32_t)ocI);
  timer_set_oc_value(TIM8, TIM_OC3, (uint32_t)ocD);
}

void set_motors_brake(void) {
  gpio_clear(GPIOB, GPIO12);
  gpio_clear(GPIOB, GPIO13);
  gpio_clear(GPIOB, GPIO14);
  gpio_clear(GPIOB, GPIO15);
  timer_set_oc_value(TIM8, TIM_OC4, 0);
  timer_set_oc_value(TIM8, TIM_OC3, 0);
}

void set_motors_pwm(int32_t pwm_left, int32_t pwm_right) {
  if (pwm_left > MOTORES_MAX_PWM) {
    pwm_left = MOTORES_MAX_PWM;
    left_motor_saturation_count++;
  } else if (pwm_left < -MOTORES_MAX_PWM) {
    pwm_left = -MOTORES_MAX_PWM;
    left_motor_saturation_count++;
  } else {
    left_motor_saturation_count = 0;
  }
  if (pwm_left >= 0) {
    gpio_set(GPIOB, GPIO14);
    gpio_clear(GPIOB, GPIO15);
  } else {
    gpio_set(GPIOB, GPIO15);
    gpio_clear(GPIOB, GPIO14);
  }
  if (pwm_right > MOTORES_MAX_PWM) {
    pwm_right = MOTORES_MAX_PWM;
    right_motor_saturation_count++;
  } else if (pwm_right < -MOTORES_MAX_PWM) {
    pwm_right = -MOTORES_MAX_PWM;
    right_motor_saturation_count++;
  } else {
    right_motor_saturation_count = 0;
  }
  if (pwm_right >= 0) {
    gpio_set(GPIOB, GPIO13);
    gpio_clear(GPIOB, GPIO12);
  } else {
    gpio_set(GPIOB, GPIO12);
    gpio_clear(GPIOB, GPIO13);
  }
  check_motors_saturated();
  // printf("%d - %d\n", abs(pwm_left), abs(pwm_right));
  timer_set_oc_value(TIM8, TIM_OC4, abs(pwm_left));
  timer_set_oc_value(TIM8, TIM_OC3, abs(pwm_right));
}

void set_fan_speed(uint8_t vel) {
  uint32_t ocF = 0;
  if (vel != 0) {
    ocF = map(abs(vel), 0, 100, 0, MOTORES_MAX_PWM);
  }
  // printf("%ld\n", ocF);
  timer_set_oc_value(TIM8, TIM_OC2, ocF);
}

void clear_motors_saturated(void) {
  motors_saturated = false;
  motors_saturated_ms = 0;
  left_motor_saturation_count = 0;
  right_motor_saturation_count = 0;
}

bool is_motor_saturated(void) {
  return motors_saturated;
}

uint32_t get_motors_saturated_ms(void) {
  return motors_saturated_ms;
}
