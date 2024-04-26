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

static volatile bool side_sensors_close_correction_enabled = false;
static volatile bool side_sensors_far_correction_enabled = false;
static volatile bool front_sensors_correction_enabled = true;

static volatile float side_sensors_error;
static volatile float last_side_sensors_error;
static volatile float sum_side_sensors_error;

static volatile float front_sensors_error;
static volatile float sum_front_sensors_error;
static volatile float last_front_sensors_error;

static volatile float voltage_left;
static volatile float voltage_right;
static volatile int32_t pwm_left;
static volatile int32_t pwm_right;

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
  return voltage / /* 8.0  */ get_battery_voltage() * MOTORES_MAX_PWM;
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

void set_side_sensors_close_correction(bool enabled) {
  side_sensors_close_correction_enabled = enabled;
}

void set_side_sensors_far_correction(bool enabled) {
  side_sensors_far_correction_enabled = enabled;
}

void set_front_sensors_correction(bool enabled) {
  front_sensors_correction_enabled = enabled;
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
  sum_linear_error += linear_error;
  last_linear_error = linear_error;
  // if (sum_linear_error > 1000) {
  //   sum_linear_error = 1000;
  // } else if (sum_linear_error < -1000) {
  //   sum_linear_error = -1000;
  // }

  angular_error = ideal_angular_speed - get_measured_angular_speed();
  sum_angular_error += angular_error;
  last_angular_error = angular_error;

  side_sensors_error = 0;
  if (side_sensors_close_correction_enabled) {
    side_sensors_error += get_side_sensors_close_error();
  }
  if (side_sensors_far_correction_enabled) {
    side_sensors_error += get_side_sensors_far_error();
  }
  if (side_sensors_close_correction_enabled || side_sensors_far_correction_enabled) {
    sum_side_sensors_error += side_sensors_error;
    last_side_sensors_error = side_sensors_error;
  } else {
    side_sensors_error = 0;
    sum_side_sensors_error = 0;
    last_side_sensors_error = 0;
  }
  if (front_sensors_correction_enabled) {
    front_sensors_error = get_front_sensors_error();
    sum_front_sensors_error += front_sensors_error;
    last_front_sensors_error = front_sensors_error;
  } else {
    front_sensors_error = 0;
    sum_front_sensors_error = 0;
    last_front_sensors_error = 0;
  }

  linear_voltage =
      KP_LINEAR * linear_error + KI_LINEAR * sum_linear_error + KD_LINEAR * (linear_error - last_linear_error);
  angular_voltage =
      KP_ANGULAR * angular_error + KI_ANGULAR * sum_angular_error + KD_ANGULAR * (angular_error - last_angular_error) +
      KP_SIDE_SENSORS * side_sensors_error + KI_SIDE_SENSORS * sum_side_sensors_error + KD_SIDE_SENSORS * (side_sensors_error - last_side_sensors_error) +
      KP_FRONT_SENSORS * front_sensors_error + KI_FRONT_SENSORS * sum_front_sensors_error + KD_FRONT_SENSORS * (front_sensors_error - last_front_sensors_error);

  voltage_left = linear_voltage + angular_voltage;
  voltage_right = linear_voltage - angular_voltage;
  pwm_left = voltage_to_motor_pwm(voltage_left);
  pwm_right = voltage_to_motor_pwm(voltage_right);
  set_motors_pwm(pwm_left, pwm_right);
  // store_debug();
  macroarray_store(
      0,
      0b00011001,
      8,
      target_linear_speed,
      ideal_linear_speed,
      (int16_t)(get_measured_linear_speed()),
      (int16_t)(ideal_angular_speed * 100.0),
      (int16_t)(get_measured_angular_speed() * 100.0),
      pwm_left,
      pwm_right,
      (int16_t)(get_battery_voltage() * 100.0));
}