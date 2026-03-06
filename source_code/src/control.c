#include "control.h"

static volatile bool race_started = false;
static volatile bool race_auto_run = false;
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

static volatile int16_t ideal_front_distance = 0;

static volatile bool linear_error_correction_enabled = true;

static volatile float linear_error;
static volatile float sum_linear_error;
static volatile float last_linear_error;

static volatile bool angular_error_correction_enabled = true;

static volatile float angular_error;
static volatile float sum_angular_error;
static volatile float last_angular_error;

static volatile bool side_sensors_correction_enabled = false;
static volatile bool front_sensors_angle_correction_enabled = false;
static volatile bool front_sensors_distance_correction_enabled = false;
static volatile bool front_sensors_diagonal_correction_enabled = false;

static volatile float side_sensors_error;
static volatile float last_side_sensors_error;
static volatile float sum_side_sensors_error;

static volatile float front_sensors_angle_error;
static volatile float last_front_sensors_angle_error;
static volatile float sum_front_sensors_angle_error;

static volatile float front_sensors_distance_error;
static volatile float last_front_sensors_distance_error;
static volatile float sum_front_sensors_distance_error;

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
static int32_t normalize_fan_percentage(float percentage) {
  return percentage > 0 ? (int32_t)constrain(get_battery_high_limit_voltage() * percentage / get_battery_voltage(), percentage, 100.0f) : 0;
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

bool is_race_auto_run(void) {
  return race_auto_run;
}

void set_race_auto_run(bool state) {
  race_auto_run = state;
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
    uint8_t sensor = sensor_front_left_start_ms >= SENSOR_START_MIN_MS ? SENSOR_FRONT_LEFT_WALL_ID : SENSOR_FRONT_RIGHT_WALL_ID;
    sensor_front_left_start_ms = 0;
    sensor_front_right_start_ms = 0;
    set_RGB_color(0, 50, 0);

    set_race_auto_run(false);
    uint32_t starting_ms = get_clock_ticks();
    while (get_clock_ticks() - starting_ms < 2000) {
      warning_status_led(50);

      if (sensor == SENSOR_FRONT_RIGHT_WALL_ID) {
        if (get_sensor_distance(SENSOR_FRONT_LEFT_WALL_ID) <= SENSOR_FRONT_DETECTION_START) {
          if (sensor_front_left_start_ms == 0 && sensor_front_right_start_ms == 0) {
            sensor_front_left_start_ms = get_clock_ticks();
          }
        } else {
          sensor_front_left_start_ms = 0;
        }

        if (sensor_front_left_start_ms >= SENSOR_START_MIN_MS) {
          set_RGB_color(50, 0, 50);
          set_race_auto_run(true);
        }
      }
    }
    set_RGB_color(0, 0, 0);
    set_status_led(false);

    set_race_started(true);
    menu_run_reset();
    return sensor;
  }
  return -1;
}
#endif

void set_linear_error_correction(bool enabled) {
  linear_error_correction_enabled = enabled;
}

void set_angular_error_correction(bool enabled) {
  angular_error_correction_enabled = enabled;
}

void set_side_sensors_correction(bool enabled) {
  side_sensors_correction_enabled = enabled;
}

void set_front_sensors_angle_correction(bool enabled) {
  front_sensors_angle_correction_enabled = enabled;
}

bool is_front_sensors_angle_correction_enabled(void) {
  return front_sensors_angle_correction_enabled;
}

void set_front_sensors_distance_correction(bool enabled) {
  front_sensors_distance_correction_enabled = enabled;
  if (!enabled) {
    ideal_front_distance = 0;
  }
}

void set_front_sensors_diagonal_correction(bool enabled) {
  front_sensors_diagonal_correction_enabled = enabled;
}

void disable_sensors_correction(void) {
  set_front_sensors_angle_correction(false);
  set_front_sensors_diagonal_correction(false);
  set_side_sensors_correction(false);
}

void reset_control_errors(void) {
  sum_side_sensors_error = 0;
  last_side_sensors_error = 0;
  last_front_sensors_angle_error = 0;
  sum_front_sensors_angle_error = 0;
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

void reset_control_fan_speed(void) {
  target_fan_speed = 0;
  ideal_fan_speed = 0;
  fan_speed_accel = 0;
}

void reset_control_all(void) {
  reset_control_errors();
  reset_control_speed();
  reset_control_fan_speed();
#ifndef MMSIM_ENABLED
  reset_motors_saturated();
  reset_encoder_avg();
#endif
}

void set_target_linear_speed(int32_t linear_speed) {
  target_linear_speed = linear_speed;
}

void force_linear_speed(int32_t linear_speed) {
  target_linear_speed = linear_speed;
  ideal_linear_speed = linear_speed;
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
  if (ms > 0) {
    target_fan_speed = normalize_fan_percentage(fan_speed);
    fan_speed_accel = (target_fan_speed - ideal_fan_speed) * CONTROL_FREQUENCY_HZ / ms;
  } else {
    target_fan_speed = normalize_fan_percentage(fan_speed);
    ideal_fan_speed = target_fan_speed;
    fan_speed_accel = 0;
  }
}
#endif

void set_ideal_front_distance(int16_t distance) {
  ideal_front_distance = distance;
}

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
  if (is_debug_enabled() && !is_debug_use_control()) {
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

  if (linear_error_correction_enabled) {
    last_linear_error = linear_error;
    linear_error = ideal_linear_speed - get_measured_linear_speed();
    sum_linear_error += linear_error;
  } else {
    linear_error = 0;
    sum_linear_error = 0;
    last_linear_error = 0;
  }

  if (angular_error_correction_enabled) {
    last_angular_error = angular_error;
    angular_error = ideal_angular_speed - get_measured_angular_speed();
    sum_angular_error += angular_error;
  } else {
    angular_error = 0;
    sum_angular_error = 0;
    last_angular_error = 0;
  }

  if (side_sensors_correction_enabled) {
    side_sensors_error = get_side_sensors_error();
    sum_side_sensors_error += side_sensors_error;
  } else {
    side_sensors_error = 0;
    sum_side_sensors_error = 0;
    last_side_sensors_error = 0;
  }

  if (front_sensors_angle_correction_enabled) {
    front_sensors_angle_error = get_front_sensors_angle_error();
    sum_front_sensors_angle_error += front_sensors_angle_error;
  } else {
    front_sensors_angle_error = 0;
    sum_front_sensors_angle_error = 0;
    last_front_sensors_angle_error = 0;
  }

  if (front_sensors_distance_correction_enabled && ideal_front_distance > 0 && get_front_wall_distance() < CELL_DIMENSION) {
    front_sensors_distance_error = get_front_wall_distance() - ideal_front_distance;
    // if (abs(front_sensors_distance_error) <= 2) {
    //   front_sensors_distance_error = 0;
    // }
    // printf("front distance: %4d front error: %.4f\n", get_front_wall_distance(), front_sensors_distance_error);
    sum_front_sensors_distance_error += front_sensors_distance_error;

    static char *labels[] = {
        "front_error",
        "pwm_left",
        "pwm_right",
    };
    macroarray_store(
        1,
        0b0,
        labels,
        3,
        (int16_t)front_sensors_distance_error,
        (int16_t)pwm_left,
        (int16_t)pwm_right);
  } else {
    front_sensors_distance_error = 0;
    sum_front_sensors_distance_error = 0;
    last_front_sensors_distance_error = 0;
  }

  if (front_sensors_diagonal_correction_enabled) {
    front_sensors_diagonal_error = get_front_sensors_diagonal_error();
    sum_front_sensors_diagonal_error += front_sensors_diagonal_error;
  } else {
    front_sensors_diagonal_error = 0;
    sum_front_sensors_diagonal_error = 0;
    last_front_sensors_diagonal_error = 0;
  }

  linear_voltage =
      get_kinematics().kpi[KPI_LINEAR].kp * linear_error +
      get_kinematics().kpi[KPI_LINEAR].ki * sum_linear_error +
      get_kinematics().kpi[KPI_LINEAR].kd * (linear_error - last_linear_error) +

      get_kinematics().kpi[KPI_FRONT_DISTANCE_SENSORS].kp * front_sensors_distance_error +
      get_kinematics().kpi[KPI_FRONT_DISTANCE_SENSORS].ki * sum_front_sensors_distance_error +
      get_kinematics().kpi[KPI_FRONT_DISTANCE_SENSORS].kd * (front_sensors_distance_error - last_front_sensors_distance_error);

  angular_voltage =
      get_kinematics().kpi[KPI_ANGULAR].kp * angular_error +
      get_kinematics().kpi[KPI_ANGULAR].ki * sum_angular_error +
      get_kinematics().kpi[KPI_ANGULAR].kd * (angular_error - last_angular_error) +

      get_kinematics().kpi[KPI_SIDE_SENSORS].kp * side_sensors_error +
      get_kinematics().kpi[KPI_SIDE_SENSORS].ki * sum_side_sensors_error +
      get_kinematics().kpi[KPI_SIDE_SENSORS].kd * (side_sensors_error - last_side_sensors_error) +

      get_kinematics().kpi[KPI_FRONT_ANGLE_SENSORS].kp * front_sensors_angle_error +
      get_kinematics().kpi[KPI_FRONT_ANGLE_SENSORS].ki * sum_front_sensors_angle_error +
      get_kinematics().kpi[KPI_FRONT_ANGLE_SENSORS].kd * (front_sensors_angle_error - last_front_sensors_angle_error) +

      get_kinematics().kpi[KPI_FRONT_DIAGONAL_SENSORS].kp * front_sensors_diagonal_error +
      get_kinematics().kpi[KPI_FRONT_DIAGONAL_SENSORS].ki * sum_front_sensors_diagonal_error +
      get_kinematics().kpi[KPI_FRONT_DIAGONAL_SENSORS].kd * (front_sensors_diagonal_error - last_front_sensors_diagonal_error);

  last_side_sensors_error = side_sensors_error;
  last_front_sensors_angle_error = front_sensors_angle_error;
  last_front_sensors_distance_error = front_sensors_distance_error;
  last_front_sensors_diagonal_error = front_sensors_diagonal_error;

  voltage_left = linear_voltage + angular_voltage;
  voltage_right = linear_voltage - angular_voltage;
  pwm_left = voltage_to_motor_pwm(voltage_left);
  pwm_right = voltage_to_motor_pwm(voltage_right);
  set_motors_pwm(pwm_left, pwm_right);

  if (ideal_linear_speed != 0 || ideal_angular_speed != 0) {
    // static char *labels[] = {
    //     "target_linear_speed",
    //     "ideal_linear_speed",
    //     "measured_linear_speed",
    //     // "measured_left_speed",
    //     // "measured_right_speed",
    //     "ideal_angular_speed",
    //     "measured_angular_speed",
    //     "raw_angular_speed",
    //     "pwm_left",
    //     "pwm_right",
    //     // "encoder_avg_millimeters",
    //     // "side_sensors_error",
    //     // "angular_voltage",
    //     "battery_voltage"};
    // macroarray_store(
    //     1,
    //     0b000111001,
    //     labels,
    //     9,
    //     (int16_t)target_linear_speed,
    //     (int16_t)ideal_linear_speed,
    //     (int16_t)(get_measured_linear_speed()),
    //     // (int16_t)(get_encoder_left_speed()),
    //     // (int16_t)(get_encoder_right_speed()),
    //     (int16_t)(ideal_angular_speed * 100),
    //     (int16_t)(get_measured_angular_speed() * 100),
    //     (int16_t)(lsm6dsr_get_gyro_z_raw() * 100),
    //     (int16_t)pwm_left,
    //     (int16_t)pwm_right,
    //     // (int16_t)get_encoder_avg_millimeters(),
    //     // (int16_t)(side_sensors_error * 100),
    //     // (int16_t)(angular_voltage * 100),
    //     (int16_t)(get_battery_voltage() * 100));

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
  linear_error = ideal_linear_speed - get_measured_linear_speed();
  sum_linear_error += linear_error;

  last_angular_error = angular_error;
  angular_error = ideal_angular_speed - get_measured_angular_speed();
  sum_angular_error += angular_error;

  angular_voltage =
      get_kinematics().kpi[KPI_ANGULAR].kp * angular_error +
      get_kinematics().kpi[KPI_ANGULAR].ki * sum_angular_error +
      get_kinematics().kpi[KPI_ANGULAR].kd * (angular_error - last_angular_error);

  linear_voltage =
      get_kinematics().kpi[KPI_LINEAR].kp * linear_error +
      get_kinematics().kpi[KPI_LINEAR].ki * sum_linear_error +
      get_kinematics().kpi[KPI_LINEAR].kd * (linear_error - last_linear_error);

  voltage_left = linear_voltage + angular_voltage;
  voltage_right = linear_voltage - angular_voltage;
  pwm_left = voltage_to_motor_pwm(voltage_left);
  pwm_right = voltage_to_motor_pwm(voltage_right);
  gpio_set(GPIOB, GPIO15);
  set_motors_pwm(pwm_left, pwm_right);
  // printf("R: %4ld L: %4ld\n", pwm_right, pwm_left);
}
#endif