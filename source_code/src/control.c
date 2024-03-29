#include <control.h>

static volatile bool competicionIniciada = false;

static volatile int32_t target_linear_speed = 0;
static volatile int32_t ideal_linear_speed = 0;
static volatile float ideal_angular_speed = 0.0;

static volatile float linear_error;
static volatile float angular_error;
static volatile float last_linear_error;
static volatile float last_angular_error;

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
  return voltage / get_battery_voltage() * MOTORES_MAX_PWM;
}

/**
 * @brief Actualiza la velocidad lineal ideal en función de la velocidad lineal objetivo y la aceleración
 *
 */
static void update_ideal_linear_speed(void) {
  if (ideal_linear_speed < target_linear_speed) {
    ideal_linear_speed += BASE_LINEAR_ACCEL / 1000;
    if (ideal_linear_speed > target_linear_speed) {
      ideal_linear_speed = target_linear_speed;
    }
  } else if (ideal_linear_speed > target_linear_speed) {
    ideal_linear_speed -= BASE_LINEAR_ACCEL / 1000;
    if (ideal_linear_speed < target_linear_speed) {
      ideal_linear_speed = target_linear_speed;
    }
  }
}

static float get_measured_linear_speed(void) {
  return (get_encoder_left_speed() + get_encoder_right_speed()) / 2.;
}

static float get_measured_angular_speed(void) {
  return -get_gyro_z_radps();
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

  linear_voltage = KP_LINEAR * linear_error + KD_LINEAR * (linear_error - last_linear_error);
  // angular_voltage = KP_ANGULAR * angular_error + KD_ANGULAR * (angular_error - last_angular_error);

  voltage_left = linear_voltage + angular_voltage;
  voltage_right = linear_voltage - angular_voltage;
  // pwm_left = voltage_to_motor_pwm(voltage_left);
  // pwm_right = voltage_to_motor_pwm(voltage_right);
  // set_motors_pwm(pwm_left, pwm_right);
  if(voltage_left > 1000){
    voltage_left = 1000;
  }
  if(voltage_right > 1000){
    voltage_right = 1000;
  }
  // set_motors_speed(voltage_left, voltage_right);

}

void control_debug(void) {
  // printf("%ld\t%ld\t%.2f\t%.2f\t%.2f\n", target_linear_speed, ideal_linear_speed, get_measured_linear_speed(), ideal_angular_speed, get_measured_angular_speed());
  printf("%ld\t%ld\t%.2f\t%.2f\t%.2f\n", target_linear_speed, ideal_linear_speed, get_measured_linear_speed(), voltage_left, voltage_right);
}