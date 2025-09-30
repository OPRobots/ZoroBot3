#include "control.h"

static volatile bool race_started = false;
static volatile uint32_t race_finish_ms = 0;
static volatile uint32_t sensor_front_left_start_ms = 0;
static volatile uint32_t sensor_front_right_start_ms = 0;

static volatile bool control_debug = false;

static volatile int32_t target_linear_speed = 0;
static volatile int32_t ideal_linear_speed = 0;
static volatile float ideal_angular_speed = 0.0;

static volatile int32_t target_fan_speed = 0;
static volatile float ideal_fan_speed = 0;
static volatile float fan_speed_accel = 0;

static volatile float linear_error;
static volatile float sum_linear_error;
static volatile float last_linear_error;

static volatile float angular_error;
static volatile float sum_angular_error;
static volatile float last_angular_error;

static volatile bool side_sensors_close_correction_enabled = false;
static volatile bool side_sensors_far_correction_enabled = false;
static volatile bool front_sensors_correction_enabled = false;
static volatile bool front_sensors_diagonal_correction_enabled = false;

static volatile float side_sensors_error;
static volatile float last_side_sensors_error;
static volatile float sum_side_sensors_error;

static volatile float front_sensors_error;
static volatile float sum_front_sensors_error;

static volatile float front_sensors_diagonal_error;
static volatile float last_front_sensors_diagonal_error;
static volatile float sum_front_sensors_diagonal_error;

static volatile float voltage_left;
static volatile float voltage_right;
static volatile int32_t pwm_left;
static volatile int32_t pwm_right;

/**
 * @brief Convierte un valor de voltaje dado a su correspondiente PWM
 *
 * @param voltage
 * @return int32_t PWM a aplicar al motor
 */
#ifndef MMSIM_ENABLED
static int32_t voltage_to_motor_pwm(float voltage) {
  return voltage / /* 8.0  */ get_battery_voltage() * MOTORES_MAX_PWM;
}
#endif

#ifndef MMSIM_ENABLED
static int32_t percentage_to_fan_pwm(float percentage) {
  return percentage > 0 ? (int32_t)constrain((get_battery_high_limit_voltage() / get_battery_voltage()) * percentage, percentage, 100.0f) : 0;
}
#endif

/**
 * @brief Actualiza la velocidad lineal ideal en función de la velocidad lineal objetivo y la aceleración
 *
 */
#ifndef MMSIM_ENABLED
static void update_ideal_linear_speed(void) {
  if (ideal_linear_speed < target_linear_speed) {
    int16_t accel = get_kinematics().linear_accel.accel_soft;
    if (get_kinematics().linear_accel.speed_hard == 0 || ideal_linear_speed < get_kinematics().linear_accel.speed_hard) {
      accel = get_kinematics().linear_accel.accel_hard;
    }
    ideal_linear_speed += accel / CONTROL_FREQUENCY_HZ;
    if (ideal_linear_speed > target_linear_speed) {
      ideal_linear_speed = target_linear_speed;
    }
  } else if (ideal_linear_speed > target_linear_speed) {
    ideal_linear_speed -= get_kinematics().linear_accel.break_accel / CONTROL_FREQUENCY_HZ;
    if (ideal_linear_speed < target_linear_speed) {
      ideal_linear_speed = target_linear_speed;
    }
  }
}

static void update_fan_speed(void) {
  if (ideal_fan_speed < target_fan_speed) {
    ideal_fan_speed += fan_speed_accel / CONTROL_FREQUENCY_HZ;
    if (ideal_fan_speed > target_fan_speed) {
      ideal_fan_speed = target_fan_speed;
    }
  } else if (ideal_fan_speed > target_fan_speed) {
    ideal_fan_speed += fan_speed_accel / CONTROL_FREQUENCY_HZ;
    if (ideal_fan_speed < target_fan_speed) {
      ideal_fan_speed = target_fan_speed;
    }
  }
}

static float get_measured_linear_speed(void) {
  return (get_encoder_left_speed() + get_encoder_right_speed()) / 2.0f;
}

static float get_measured_angular_speed(void) {
  return -lsm6dsr_get_gyro_z_radps();
}
#endif

/**
 * @brief Comprueba si el robot está en funcionamiento
 *
 * @return bool
 */

bool is_race_started(void) {
  return race_started;
}

/**
 * @brief Establece el estado actual del robot
 *
 * @param state Estado actual del robot
 */

void set_race_started(bool state) {
  race_started = state;

#ifndef MMSIM_ENABLED
  reset_control_all();
  if (!state) {
    menu_reset();
    race_finish_ms = get_clock_ticks();
  }
#endif
}

void set_control_debug(bool state) {
  control_debug = state;
}

#ifndef MMSIM_ENABLED
int8_t check_start_run(void) {
  if (get_sensor_distance(SENSOR_FRONT_LEFT_WALL_ID) <= SENSOR_FRONT_DETECTION_START) {
    if (sensor_front_left_start_ms == 0 && sensor_front_right_start_ms == 0) {
      sensor_front_left_start_ms = get_clock_ticks();
    }
  } else {
    sensor_front_left_start_ms = 0;
  }
  if (get_sensor_distance(SENSOR_FRONT_RIGHT_WALL_ID) <= SENSOR_FRONT_DETECTION_START) {
    if (sensor_front_left_start_ms == 0 && sensor_front_right_start_ms == 0) {
      sensor_front_right_start_ms = get_clock_ticks();
    }
  } else {
    sensor_front_right_start_ms = 0;
  }

  if (sensor_front_left_start_ms >= SENSOR_START_MIN_MS || sensor_front_right_start_ms >= SENSOR_START_MIN_MS) {
    set_RGB_color(0, 50, 0);
    delay(1000);
    set_RGB_color(0, 0, 0);
    set_race_started(true);
    uint8_t sensor = sensor_front_left_start_ms >= SENSOR_START_MIN_MS ? SENSOR_FRONT_LEFT_WALL_ID : SENSOR_FRONT_RIGHT_WALL_ID;
    sensor_front_left_start_ms = 0;
    sensor_front_right_start_ms = 0;
    menu_run_reset();
    return sensor;
  }
  return -1;
}
#endif

void set_side_sensors_close_correction(bool enabled) {
  side_sensors_close_correction_enabled = enabled;
}

void set_side_sensors_far_correction(bool enabled) {
  side_sensors_far_correction_enabled = enabled;
}

void set_front_sensors_correction(bool enabled) {
  front_sensors_correction_enabled = enabled;
}

bool is_front_sensors_correction_enabled(void) {
  return front_sensors_correction_enabled;
}

void set_front_sensors_diagonal_correction(bool enabled) {
  front_sensors_diagonal_correction_enabled = enabled;
}

void disable_sensors_correction(void) {
  set_front_sensors_correction(false);
  set_front_sensors_diagonal_correction(false);
  set_side_sensors_close_correction(false);
  set_side_sensors_far_correction(false);
}

void reset_control_errors(void) {
  sum_side_sensors_error = 0;
  last_side_sensors_error = 0;
  sum_front_sensors_error = 0;
  sum_front_sensors_diagonal_error = 0;
  linear_error = 0;
  angular_error = 0;
  sum_angular_error = 0;
  sum_linear_error = 0;
  last_linear_error = 0;
  last_angular_error = 0;
}

void reset_control_speed(void) {
  target_linear_speed = 0;
  ideal_linear_speed = 0;
  ideal_angular_speed = 0.0;
  voltage_left = 0;
  voltage_right = 0;
  pwm_left = 0;
  pwm_right = 0;
}

void reset_control_all(void) {
  reset_control_errors();
  reset_control_speed();
#ifndef MMSIM_ENABLED
  reset_motors_saturated();
  reset_encoder_avg();
#endif
}

void set_target_linear_speed(int32_t linear_speed) {
  target_linear_speed = linear_speed;
}

int32_t get_ideal_linear_speed(void) {
  return ideal_linear_speed;
}

void set_ideal_angular_speed(float angular_speed) {
  ideal_angular_speed = angular_speed;
}

float get_ideal_angular_speed(void) {
  return ideal_angular_speed;
}

#ifndef MMSIM_ENABLED
void set_target_fan_speed(int32_t fan_speed, int32_t ms) {
  target_fan_speed = fan_speed; // percentage_to_fan_pwm(fan_speed);
  fan_speed_accel = (fan_speed - ideal_fan_speed) * CONTROL_FREQUENCY_HZ / ms;
}
#endif

/**
 * @brief Función de control general del robot
 * · Gestiona velocidades, aceleraciones, correcciones, ...
 *
 */

#ifndef MMSIM_ENABLED
void control_loop(void) {
  // gpio_set(GPIOB, GPIO13);
  // delay_us(100);
  // gpio_clear(GPIOB, GPIO13);
  // return;
  if (is_debug_enabled()) {
    return;
  }
  if ((is_motor_pwm_saturated() || is_motor_angle_saturated()) && is_race_started()) {
    set_motors_speed(0, 0);
    set_fan_speed(0);
    if (get_clock_ticks() - get_motors_saturated_ms() < 3000) {
      blink_RGB_color(is_motor_pwm_saturated() ? 255 : 0, 0, is_motor_angle_saturated() ? 255 : 0, 50);
    } else {
      set_RGB_color(0, 0, 0);
      set_race_started(false);
    }
    return;
  }
  if (!is_race_started()) {
    if (race_finish_ms > 0 && get_clock_ticks() - race_finish_ms <= 3000) {
      set_motors_brake();
    } else {
      set_motors_speed(0, 0);
      set_motors_enable(false);
    }
    set_fan_speed(0);
    return;
  } else {
    set_motors_enable(true);
  }

  update_ideal_linear_speed();
  update_fan_speed();
  set_fan_speed(ideal_fan_speed);

  float linear_voltage = 0;
  float angular_voltage = 0;

  last_linear_error = linear_error;
  linear_error = ideal_linear_speed - get_measured_linear_speed();
  sum_linear_error += linear_error;

  last_angular_error = angular_error;
  angular_error = ideal_angular_speed - get_measured_angular_speed();
  sum_angular_error += angular_error;

  // side_sensors_error = 0;
  // if (side_sensors_close_correction_enabled) {
  //   side_sensors_error += get_side_sensors_close_error();
  //   sum_side_sensors_error += side_sensors_error;
  // }
  // if (side_sensors_far_correction_enabled) {
  //   side_sensors_error += get_side_sensors_far_error();
  //   sum_side_sensors_error += side_sensors_error;
  // }

  side_sensors_error = 0;
  if (side_sensors_close_correction_enabled) {
    side_sensors_error += get_side_sensors_error();
    sum_side_sensors_error += side_sensors_error;
  }

  if (!side_sensors_close_correction_enabled && !side_sensors_far_correction_enabled) {
    sum_side_sensors_error = 0;
    last_side_sensors_error = 0;
  }

  front_sensors_error = 0;
  if (front_sensors_correction_enabled) {
    front_sensors_error = get_front_sensors_angle_error();
    sum_front_sensors_error += front_sensors_error;
  }

  front_sensors_diagonal_error = 0;
  if (front_sensors_diagonal_correction_enabled) {
    front_sensors_diagonal_error = get_front_sensors_diagonal_error();
    sum_front_sensors_diagonal_error += front_sensors_diagonal_error;
    // if (front_sensors_diagonal_error != 0) {
    //   set_RGB_color(255, 0, 0);
    // } else {
    //   set_RGB_color(0, 255, 0);
    // }
  } else {
    front_sensors_diagonal_error = 0;
    sum_front_sensors_diagonal_error = 0;
    last_front_sensors_diagonal_error = 0;
  }

  linear_voltage = KP_LINEAR * linear_error + KI_LINEAR * sum_linear_error + KD_LINEAR * (linear_error - last_linear_error);

  angular_voltage =
      KP_ANGULAR * angular_error + KI_ANGULAR * sum_angular_error + KD_ANGULAR * (angular_error - last_angular_error) +
      KP_SIDE_SENSORS * side_sensors_error + KI_SIDE_SENSORS * sum_side_sensors_error + KD_SIDE_SENSORS * (side_sensors_error - last_side_sensors_error) +
      KP_FRONT_SENSORS * front_sensors_error + KI_FRONT_SENSORS * sum_front_sensors_error +
      KP_FRONT_DIAGONAL_SENSORS * front_sensors_diagonal_error + KI_FRONT_DIAGONAL_SENSORS * sum_front_sensors_diagonal_error + KD_FRONT_DIAGONAL_SENSORS * (front_sensors_diagonal_error - last_front_sensors_diagonal_error);

  last_side_sensors_error = side_sensors_error;
  last_front_sensors_diagonal_error = front_sensors_diagonal_error;

  // if (get_ideal_linear_speed() > 0) {
  //   angular_voltage *= get_ideal_linear_speed() / 500.0f; // TODO: definear 500 as explore speed
  // }

  voltage_left = linear_voltage + angular_voltage;
  voltage_right = linear_voltage - angular_voltage;
  pwm_left = voltage_to_motor_pwm(voltage_left);
  pwm_right = voltage_to_motor_pwm(voltage_right);
  set_motors_pwm(pwm_left, pwm_right);

  if (ideal_linear_speed != 0 || ideal_angular_speed != 0) {
    static char *labels[] = {
        "target_linear_speed",
        "ideal_linear_speed",
        "measured_linear_speed",
        "measured_left_speed",
        "measured_right_speed",
        "ideal_angular_speed",
        "measured_angular_speed",
        "pwm_left",
        "pwm_right",
        // "encoder_avg_millimeters",
        "side_sensors_error",
        "battery_voltage"};
    macroarray_store(
        5,
        0b00000110011,
        labels,
        11,
        (int16_t)target_linear_speed,
        (int16_t)ideal_linear_speed,
        (int16_t)(get_measured_linear_speed()),
        (int16_t)(get_encoder_left_speed()),
        (int16_t)(get_encoder_right_speed()),
        (int16_t)(ideal_angular_speed * 100),
        (int16_t)(get_measured_angular_speed() * 100),
        (int16_t)pwm_left,
        (int16_t)pwm_right,
        // (int16_t)get_encoder_avg_millimeters(),
        (int16_t)(side_sensors_error * 100),
        (int16_t)(get_battery_voltage() * 100));

    // static char *labels[] = {
    //     "target_linear_speed",
    //     "ideal_linear_speed",
    //     "measured_linear_speed",
    //     "ideal_angular_speed",
    //     "measured_angular_speed",
    //     "front_left_distance",
    //     "front_right_distance",
    //     "diagonal_error",
    //     "encoder_avg_millimeters",
    //     "wall_lost_toggle_state",
    //     "cell_change_toggle_state"};
    // macroarray_store(
    //     2,
    //     0b00011000000,
    //     labels,
    //     11,
    //     (int16_t)target_linear_speed,
    //     (int16_t)ideal_linear_speed,
    //     (int16_t)(get_measured_linear_speed()),
    //     (int16_t)(ideal_angular_speed * 100.0),
    //     (int16_t)(get_measured_angular_speed() * 100),
    //     (int16_t)get_sensor_distance(SENSOR_FRONT_LEFT_WALL_ID),
    //     (int16_t)get_sensor_distance(SENSOR_FRONT_RIGHT_WALL_ID),
    //     (int16_t)front_sensors_diagonal_error,
    //     (int16_t)get_encoder_avg_millimeters(),
    //     (int16_t)get_wall_lost_toggle_state() ? 1 : 0,
    //     (int16_t)get_cell_change_toggle_state() ? 1 : 0
    //     );

    // static char *labels[] = {
    //     "sl",
    //     "fl",
    //     "fr",
    //     "sr",
    // };
    // macroarray_store(
    //     0,
    //     0b0,
    //     labels,
    //     4,
    //     (int16_t)get_sensor_distance(SENSOR_SIDE_LEFT_WALL_ID),
    //     (int16_t)get_sensor_distance(SENSOR_FRONT_LEFT_WALL_ID),
    //     (int16_t)get_sensor_distance(SENSOR_FRONT_RIGHT_WALL_ID),
    //     (int16_t)get_sensor_distance(SENSOR_SIDE_RIGHT_WALL_ID));
  }
}
#endif

#ifndef MMSIM_ENABLED
void keep_z_angle(void) {
  float linear_voltage = 0;
  float angular_voltage = 0;
  last_linear_error = linear_error;
  linear_error += ideal_linear_speed - get_measured_linear_speed();

  last_angular_error = angular_error;
  angular_error += ideal_angular_speed - get_measured_angular_speed();

  angular_voltage = KP_ANGULAR * angular_error + KD_ANGULAR * (angular_error - last_angular_error);
  linear_voltage = KP_LINEAR * linear_error + KD_LINEAR * (linear_error - last_linear_error);
  voltage_left = linear_voltage + angular_voltage;
  voltage_right = linear_voltage - angular_voltage;
  pwm_left = voltage_to_motor_pwm(voltage_left);
  pwm_right = voltage_to_motor_pwm(voltage_right);
  gpio_set(GPIOB, GPIO15);
  set_motors_pwm(pwm_left, pwm_right);
}
#endif