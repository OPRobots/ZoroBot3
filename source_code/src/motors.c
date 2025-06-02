#include <motors.h>

static bool motors_pwm_saturated = false;
static bool motors_angle_saturated = false;
static uint32_t motors_saturated_ms = 0;
static uint16_t left_motor_saturation_count = 0;
static uint16_t right_motor_saturation_count = 0;
static uint16_t angular_speed_saturation_count = 0;

static bool check_motors_saturated_enabled = true;

static void check_motors_saturated(void) {
  if (check_motors_saturated_enabled) {
    if (left_motor_saturation_count > MAX_MOTOR_SATURATION_COUNT || right_motor_saturation_count > MAX_MOTOR_SATURATION_COUNT) {
      if (!motors_pwm_saturated) {
        motors_saturated_ms = get_clock_ticks();
      }
      motors_pwm_saturated = true;
    } else {
      motors_pwm_saturated = false;
    }

    if (angular_speed_saturation_count > MAX_MOTOR_ANGULAR_SATURATION_COUNT) {
      if (!motors_angle_saturated) {
        motors_saturated_ms = get_clock_ticks();
      }
      motors_angle_saturated = true;
    } else {
      motors_angle_saturated = false;
    }
  } else {
    motors_pwm_saturated = false;
    motors_angle_saturated = false;
  }
}

void set_check_motors_saturated_enabled(bool enabled) {
  check_motors_saturated_enabled = enabled;
}

void set_motors_enable(bool enabled) {
  if (enabled) {
    gpio_set(GPIOB, GPIO15);
  } else {
    gpio_clear(GPIOB, GPIO15);
  }
}

void set_motors_speed(float velI, float velD) {
  float ocI = 0;
  float ocD = 0;

  ocI = map(abs(velI), 0, 1000, 0, MOTORES_MAX_PWM);
  if (velI > 0) {
    timer_set_oc_value(TIM8, TIM_OC4, MOTORES_MAX_PWM - (uint32_t)ocI);
    timer_set_oc_value(TIM8, TIM_OC3, MOTORES_MAX_PWM);
  } else {
    timer_set_oc_value(TIM8, TIM_OC3, MOTORES_MAX_PWM - (uint32_t)ocI);
    timer_set_oc_value(TIM8, TIM_OC4, MOTORES_MAX_PWM);
  }

  ocD = map(abs(velD), 0, 1000, 0, MOTORES_MAX_PWM);
  if (velD > 0) {
    timer_set_oc_value(TIM8, TIM_OC2, MOTORES_MAX_PWM - (uint32_t)ocD);
    timer_set_oc_value(TIM8, TIM_OC1, MOTORES_MAX_PWM);
  } else {
    timer_set_oc_value(TIM8, TIM_OC1, MOTORES_MAX_PWM - (uint32_t)ocD);
    timer_set_oc_value(TIM8, TIM_OC2, MOTORES_MAX_PWM);
  }
  // printf("%ld - %ld\n", (uint32_t)ocI, (uint32_t)ocD);
}

void set_motors_brake(void) {
  timer_set_oc_value(TIM8, TIM_OC1, MOTORES_MAX_PWM);
  timer_set_oc_value(TIM8, TIM_OC2, MOTORES_MAX_PWM);
  timer_set_oc_value(TIM8, TIM_OC3, MOTORES_MAX_PWM);
  timer_set_oc_value(TIM8, TIM_OC4, MOTORES_MAX_PWM);
}

void set_motors_pwm(int32_t pwm_left, int32_t pwm_right) {
  if (pwm_left > MOTORES_MAX_PWM) {
    pwm_left = MOTORES_MAX_PWM;
  } else if (pwm_left < -MOTORES_MAX_PWM) {
    pwm_left = -MOTORES_MAX_PWM;
  }

  if (abs(pwm_left) >= MOTORES_SATURATION_PWM) {
    left_motor_saturation_count++;
  } else {
    left_motor_saturation_count = 0;
  }

  if (pwm_left >= 0) {
    timer_set_oc_value(TIM8, TIM_OC4, MOTORES_MAX_PWM - abs(pwm_left));
    timer_set_oc_value(TIM8, TIM_OC3, MOTORES_MAX_PWM);
  } else {
    timer_set_oc_value(TIM8, TIM_OC3, MOTORES_MAX_PWM - abs(pwm_left));
    timer_set_oc_value(TIM8, TIM_OC4, MOTORES_MAX_PWM);
  }

  if (pwm_right > MOTORES_MAX_PWM) {
    pwm_right = MOTORES_MAX_PWM;
  } else if (pwm_right < -MOTORES_MAX_PWM) {
    pwm_right = -MOTORES_MAX_PWM;
  }

  if (abs(pwm_right) >= MOTORES_SATURATION_PWM) {
    right_motor_saturation_count++;
  } else {
    right_motor_saturation_count = 0;
  }

  if (pwm_right >= 0) {
    timer_set_oc_value(TIM8, TIM_OC2, MOTORES_MAX_PWM - abs(pwm_right));
    timer_set_oc_value(TIM8, TIM_OC1, MOTORES_MAX_PWM);
  } else {
    timer_set_oc_value(TIM8, TIM_OC1, MOTORES_MAX_PWM - abs(pwm_right));
    timer_set_oc_value(TIM8, TIM_OC2, MOTORES_MAX_PWM);
  }

  if (abs(get_encoder_angular_speed()) >= MOTORES_SATURATION_ANGULAR_SPEED) {
    angular_speed_saturation_count++;
  } else {
    angular_speed_saturation_count = 0;
  }

  check_motors_saturated();
  // printf("%d - %d\n", abs(pwm_left), abs(pwm_right));
}

void set_fan_speed(uint8_t vel) {
  uint32_t ocF = 0;
  if (vel != 0) {
    ocF = map(abs(vel), 0, 100, 0, LEDS_MAX_PWM);
  }
  // printf("%ld\n", ocF);
  timer_set_oc_value(TIM1, TIM_OC1, ocF);
}

void reset_motors_saturated(void) {
  motors_pwm_saturated = false;
  motors_angle_saturated = false;
  motors_saturated_ms = 0;
  left_motor_saturation_count = 0;
  right_motor_saturation_count = 0;
  angular_speed_saturation_count = 0;
  check_motors_saturated_enabled = true;
}

bool is_motor_saturated(void) {
  return motors_pwm_saturated || motors_angle_saturated;
}

bool is_motor_pwm_saturated(void) {
  return motors_pwm_saturated;
}

bool is_motor_angle_saturated(void) {
  return motors_angle_saturated;
}

uint32_t get_motors_saturated_ms(void) {
  return motors_saturated_ms;
}
