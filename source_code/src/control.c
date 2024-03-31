#include <control.h>

static volatile bool competicionIniciada = false;

static volatile int32_t target_linear_speed = 0;
static volatile int32_t ideal_linear_speed = 0;
static volatile float ideal_angular_speed = 0.0;

static volatile float linear_error;
static volatile float last_linear_error;
static volatile float sum_linear_error;

static volatile float angular_error;
static volatile float last_angular_error;
static volatile float sum_angular_error;

static volatile float voltage_left;
static volatile float voltage_right;
static volatile int32_t pwm_left;
static volatile int32_t pwm_right;

static volatile uint16_t arr_debug_start = 0;
static volatile uint16_t arr_debug_end = 0;
static volatile int32_t arr_debug[CONTROL_DEBUG_LENGTH][CONTROL_DEBUG_SIZE];

/**
 * @brief Comprueba si el robot está en funcionamiento
 *
 * @return bool
 */

bool is_competicion_iniciada(void) {
  return competicionIniciada;
}

/**
 * @brief Establece el estado actual del robot
 *
 * @param state Estado actual del robot
 */

void set_competicion_iniciada(bool state) {
  competicionIniciada = state;
}

/**
 * @brief Convierte un valor de voltaje dado a su correspondiente PWM
 *
 * @param voltage
 * @return int32_t PWM a aplicar al motor
 */
static int32_t voltage_to_motor_pwm(float voltage) {
  return voltage / /* 8.0 */ get_battery_voltage() * MOTORES_MAX_PWM;
}

/**
 * @brief Actualiza la velocidad lineal ideal en función de la velocidad lineal objetivo y la aceleración
 *
 */
static void update_ideal_linear_speed(void) {
  if (ideal_linear_speed < target_linear_speed) {
    ideal_linear_speed += BASE_LINEAR_ACCEL / CONTROL_FREQUENCY_HZ;
    if (ideal_linear_speed > target_linear_speed) {
      ideal_linear_speed = target_linear_speed;
    }
  } else if (ideal_linear_speed > target_linear_speed) {
    ideal_linear_speed -= BASE_LINEAR_ACCEL / CONTROL_FREQUENCY_HZ;
    if (ideal_linear_speed < target_linear_speed) {
      ideal_linear_speed = target_linear_speed;
    }
  }
}

static float get_measured_linear_speed(void) {
  return (get_encoder_left_speed() + get_encoder_right_speed()) / 2.0f;
}

static float get_measured_angular_speed(void) {
  return -get_gyro_z_radps();
}

static void store_debug(void) {
  arr_debug[arr_debug_end][TARGET_LINEAR_SPEED] = target_linear_speed;
  arr_debug[arr_debug_end][IDEAL_LINEAR_SPEED] = ideal_linear_speed;
  arr_debug[arr_debug_end][MEASURED_LINEAR_SPEED] = get_measured_linear_speed() * 100.0;
  arr_debug[arr_debug_end][IDEAL_ANGULAR_SPEED] = ideal_angular_speed * 100;
  arr_debug[arr_debug_end][MEASURED_ANGULAR_SPEED] = get_measured_angular_speed() * 100.0;
  arr_debug[arr_debug_end][PWM_LEFT] = pwm_left;
  arr_debug[arr_debug_end][PWM_RIGHT] = pwm_right;
  arr_debug[arr_debug_end][BATTERY_LEVEL] = get_battery_voltage() * 100.0;

  if (arr_debug_end < arr_debug_start) {
    arr_debug_end++;
    arr_debug_start++;
  } else {
    arr_debug_end++;
  }
  if (arr_debug_start >= CONTROL_DEBUG_LENGTH) {
    arr_debug_start = 0;
  }
  if (arr_debug_end >= CONTROL_DEBUG_LENGTH) {
    arr_debug_end = 0;
    arr_debug_start = arr_debug_end + 1;
  }
}

void set_target_linear_speed(int32_t linear_speed) {
  target_linear_speed = linear_speed;
}

void set_ideal_angular_speed(float angular_speed) {
  ideal_angular_speed = angular_speed;
}

/**
 * @brief Función de control general del robot
 * · Gestiona velocidades, aceleraciones, correcciones, ...
 *
 */
void control_loop(void) {
  if (!competicionIniciada) {
    set_motors_speed(0, 0);
    set_fan_speed(0);
    return;
  }

  update_ideal_linear_speed();

  float linear_voltage = 0;
  float angular_voltage = 0;

  // TODO: corrección de sensores

  // TODO: corrección de gyro

  linear_error = ideal_linear_speed - get_measured_linear_speed();
  angular_error = ideal_angular_speed - get_measured_angular_speed();

  linear_voltage = KP_LINEAR * linear_error + KI_LINEAR * sum_linear_error + KD_LINEAR * (linear_error - last_linear_error);
  sum_linear_error += linear_error;
  last_linear_error = linear_error;
  // if (sum_linear_error > 1000) {
  //   sum_linear_error = 1000;
  // } else if (sum_linear_error < -1000) {
  //   sum_linear_error = -1000;
  // }

  angular_voltage = KP_ANGULAR * angular_error + KI_ANGULAR * sum_angular_error + KD_ANGULAR * (angular_error - last_angular_error);
  sum_angular_error += angular_error;
  last_angular_error = angular_error;

  voltage_left = linear_voltage + angular_voltage;
  voltage_right = linear_voltage - angular_voltage;
  pwm_left = voltage_to_motor_pwm(voltage_left);
  pwm_right = voltage_to_motor_pwm(voltage_right);
  set_motors_pwm(pwm_left, pwm_right);
  store_debug();
}

void control_debug(void) {
  if (arr_debug_start == arr_debug_end) {
    return;
  }
  uint16_t i = arr_debug_start;
  do {
    printf("%ld\t%ld\t%.2f\t%.2f\t%.2f\t%ld\t%ld\t%.2f\n",
           arr_debug[i][TARGET_LINEAR_SPEED],
           arr_debug[i][IDEAL_LINEAR_SPEED],
           arr_debug[i][MEASURED_LINEAR_SPEED] / 100.0,
           arr_debug[i][IDEAL_ANGULAR_SPEED] / 100.0,
           arr_debug[i][MEASURED_ANGULAR_SPEED] / 100.0,
           arr_debug[i][PWM_LEFT],
           arr_debug[i][PWM_RIGHT],
           arr_debug[i][BATTERY_LEVEL] / 100.0);
    i++;
    if (i >= CONTROL_DEBUG_LENGTH) {
      i = 0;
    }
  } while (i != arr_debug_end);
}