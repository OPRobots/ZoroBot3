#include "encoders.h"

/* Physical variables for calibration */
static volatile float micrometers_per_tick = MICROMETERS_PER_TICK;
static volatile float wheels_separation = WHEELS_SEPARATION;

/* Difference between the current count and the latest count */
static volatile int32_t left_diff_ticks;
static volatile int32_t right_diff_ticks;

/* Total number of counts */
static volatile int32_t left_total_ticks;
static volatile int32_t right_total_ticks;

/* Total travelled distance, in micrometers */
static volatile int32_t left_micrometers;
static volatile int32_t right_micrometers;
static volatile float avg_micrometers;

static volatile int32_t left_millimeters;
static volatile int32_t right_millimeters;
static volatile float avg_millimeters;

/* Speed, in meters per second */
static volatile float left_speed;
static volatile float right_speed;

/* Angular speed, in radians per second */
static volatile float angular_speed;

/* Current angle, in rad*/
static volatile float current_angle = 0;

float get_micrometers_per_tick(void) {
  return micrometers_per_tick;
}

void set_micrometers_per_tick(float value) {
  micrometers_per_tick = value;
}

float get_wheels_separation(void) {
  return wheels_separation;
}

void set_wheels_separation(float value) {
  wheels_separation = value;
}

/**
 * @brief Read left motor encoder counter.
 */
static uint16_t read_encoder_left(void) {
  return (uint16_t)timer_get_counter(TIM4);
}

/**
 * @brief Read right motor encoder counter.
 */
static uint16_t read_encoder_right(void) {
  return (uint16_t)timer_get_counter(TIM3);
}

/**
 * @brief Read left motor encoder travelled distance in micrometers.
 */
int32_t get_encoder_total_left_micrometers(void) {
  return left_micrometers;
}

/**
 * @brief Read right motor encoder travelled distance in micrometers.
 */
int32_t get_encoder_total_right_micrometers(void) {
  return right_micrometers;
}

/**
 * @brief Read the average travelled distance in micrometers.
 */
int32_t get_encoder_total_average_micrometers(void) {
  return (left_micrometers + right_micrometers) / 2;
}

/**
 * @brief Read left motor encoder travelled distance in millimeters.
 */
int32_t get_encoder_total_left_millimeters(void) {
  return left_millimeters;
}

/**
 * @brief Read right motor encoder travelled distance in millimeters.
 */
int32_t get_encoder_total_right_millimeters(void) {
  return right_millimeters;
}

/**
 * @brief Read the average travelled distance in millimeters.
 */
int32_t get_encoder_total_average_millimeters(void) {
  return (left_millimeters + right_millimeters) / 2;
}

/**
 * @brief Read left motor speed in millimeters per second.
 */
float get_encoder_left_speed(void) {
  return left_speed;
}

/**
 * @brief Read right motor speed in millimeters per second.
 */
float get_encoder_right_speed(void) {
  return right_speed;
}

/**
 * @brief Read the average motor speed in millimeters per second.
 */
float get_encoder_avg_speed(void) {
  return (left_speed + right_speed) / 2.0f;
}


/**
 * @brief Read the average distance in micrometers.
 */
float get_encoder_avg_micrometers(void) {
  return avg_micrometers;
}

/**
 * @brief Read the average distance in millimeters.
 */
float get_encoder_avg_millimeters(void) {
  return avg_millimeters;
}

/**
 * @brief Read the current angle in rad.
 */
float get_encoder_curernt_angle(void) {
  return current_angle;
}

/**
 * @brief Read angular speed in radians per second.
 */
float get_encoder_angular_speed(void) {
  return angular_speed;
}

/**
 * @brief Return the most likely counter difference.
 *
 * When reading an increasing or decreasing counter caution must be taken to:
 *
 * - Account for counter overflow.
 * - Account for counter direction (increasing or decreasing).
 *
 * This function assumes the most likely situation is to read the counter fast
 * enough to ensure that the direction is defined by the minimal difference
 * between the two reads (i.e.: readings are much faster than counter
 * overflows).
 *
 * @param[in] now Counter reading now.
 * @param[in] before Counter reading before.
 */
int32_t max_likelihood_counter_diff(uint16_t now, uint16_t before) {
  uint16_t forward_diff;
  uint16_t backward_diff;

  forward_diff = (uint16_t)(now - before);
  backward_diff = (uint16_t)(before - now);
  if (forward_diff > backward_diff)
    return -(int32_t)backward_diff;
  return (int32_t)forward_diff;
}

/**
 * @brief Update encoder readings.
 *
 * - Read raw encoder counters.
 * - Update the count differences (with respect to latest reading).
 * - Calculate distance travelled.
 * - Calculate speed.
 */
void update_encoder_readings(void) {
  static uint16_t last_left_ticks;
  static uint16_t last_right_ticks;

  uint16_t left_ticks;
  uint16_t right_ticks;

  left_ticks = read_encoder_left();
  right_ticks = read_encoder_right();
  left_diff_ticks = -max_likelihood_counter_diff(left_ticks, last_left_ticks);
  right_diff_ticks = max_likelihood_counter_diff(right_ticks, last_right_ticks);
  left_total_ticks += left_diff_ticks;
  right_total_ticks += right_diff_ticks;

  left_micrometers = (int32_t)(left_total_ticks * micrometers_per_tick);
  right_micrometers = (int32_t)(right_total_ticks * micrometers_per_tick);
  left_millimeters = left_micrometers / MICROMETERS_PER_MILLIMETER;
  right_millimeters = right_micrometers / MICROMETERS_PER_MILLIMETER;

  left_speed = left_diff_ticks * (micrometers_per_tick / MICROMETERS_PER_MILLIMETER) * SYSTICK_FREQUENCY_HZ;
  right_speed = right_diff_ticks * (micrometers_per_tick / MICROMETERS_PER_MILLIMETER) * SYSTICK_FREQUENCY_HZ;

  angular_speed = (left_speed - right_speed) / wheels_separation;

  current_angle += angular_speed / SYSTICK_FREQUENCY_HZ;

  avg_micrometers = (left_speed / SYSTICK_FREQUENCY_HZ + right_speed / SYSTICK_FREQUENCY_HZ) / 2.0f;

  avg_millimeters = avg_micrometers / MICROMETERS_PER_MILLIMETER;

  last_left_ticks = left_ticks;
  last_right_ticks = right_ticks;
}
