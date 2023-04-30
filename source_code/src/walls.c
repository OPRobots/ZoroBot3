#include <walls.h>

#define SIDE_WALL_DETECTION (CELL_DIMENSION * 0.90)
#define FRONT_WALL_DETECTION (CELL_DIMENSION * 1.5)
#define SIDE_CALIBRATION_READINGS 20
#define SENSOR_CALIBRATION_MS 4

static volatile float distance[NUM_SENSORES];
static volatile float calibration_factor[NUM_SENSORES];
const float sensors_calibration_a[NUM_SENSORES] = {
    SENSOR_SIDE_LEFT_A, SENSOR_SIDE_RIGHT_A, SENSOR_FRONT_LEFT_A,
    SENSOR_FRONT_RIGHT_A};
const float sensors_calibration_b[NUM_SENSORES] = {
    SENSOR_SIDE_LEFT_B, SENSOR_SIDE_RIGHT_B, SENSOR_FRONT_LEFT_B,
    SENSOR_FRONT_RIGHT_B};

struct walls_around {
  bool front : 1;
  bool left : 1;
  bool right : 1;
};

void update_distance_readings() {
  uint16_t on[NUM_SENSORES], off[NUM_SENSORES];

  get_sensors_raw(on, off);

  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    distance[i] = (sensors_calibration_a[i] / sensors_raw_log(on[i], off[i]) - sensors_calibration_b[i]);

    
    if ((i == SENSOR_SIDE_LEFT_ID) || (i == SENSOR_SIDE_RIGHT_ID)) {
      distance[i] -= calibration_factor[i];
    }
  }
}

/**
 * @brief Get distance value from front left sensor.
 */
float get_front_left_distance(void) {
  return distance[SENSOR_FRONT_LEFT_ID];
}

/**
 * @brief Get distance value from front right sensor.
 */
float get_front_right_distance(void) {
  return distance[SENSOR_FRONT_RIGHT_ID];
}

/**
 * @brief Get distance value from side left sensor.
 */
float get_side_left_distance(void) {
  return distance[SENSOR_SIDE_LEFT_ID];
}

/**
 * @brief Get distance value from side right sensor.
 */
float get_side_right_distance(void) {
  return distance[SENSOR_SIDE_RIGHT_ID];
}

/**
 * @brief Calculate and return the side sensors error when object is too close.
 *
 * Taking into account that the walls are parallel to the robot, this function
 * returns the distance that the robot is moved from the center of the
 * corridor.
 */
float get_side_sensors_close_error(void) {
  float left_error = distance[SENSOR_SIDE_LEFT_ID] - MIDDLE_MAZE_DISTANCE;
  float right_error = distance[SENSOR_SIDE_RIGHT_ID] - MIDDLE_MAZE_DISTANCE;

  if ((left_error > 0) && (right_error < 0)) {
    return right_error;
  }
  if ((right_error > 0) && (left_error < 0)) {
    return -left_error;
  }
  return 0;
}

/**
 * @brief Calculate and return the side sensors error when object is too far.
 *
 * This is useful when the robot is too far away from a lateral wall on one
 * side but there is no wall on the other side.
 */
float get_side_sensors_far_error(void) {
  float left_error = distance[SENSOR_SIDE_LEFT_ID] - MIDDLE_MAZE_DISTANCE;
  float right_error = distance[SENSOR_SIDE_RIGHT_ID] - MIDDLE_MAZE_DISTANCE;

  if ((left_error > 0.1) && (right_error < 0.04))
    return right_error;
  if ((right_error > 0.1) && (left_error < 0.04))
    return -left_error;
  return 0;
}

/**
 * @brief Calculate and return the front sensors error.
 *
 * Taking into account that robot is approaching to a perpendicular wall, this
 * function returns the difference between the front sensors distances.
 *
 * If there is no front wall detected, it returns 0.
 */
float get_front_sensors_error(void) {
  if (!front_wall_detection()) {
    return 0;
  }
  return distance[SENSOR_FRONT_LEFT_ID] - distance[SENSOR_FRONT_RIGHT_ID];
}

/**
 * @brief Return the front wall distance, in meters.
 */
float get_front_wall_distance(void) {
  return (distance[SENSOR_FRONT_LEFT_ID] +
          distance[SENSOR_FRONT_RIGHT_ID]) /
         2.;
}

/**
 * @brief Detect the existance or absence of the left wall.
 */
bool left_wall_detection(void) {
  return (distance[SENSOR_SIDE_LEFT_ID] < SIDE_WALL_DETECTION);
}

/**
 * @brief Detect the existance or absence of the right wall.
 */
bool right_wall_detection(void) {
  return (distance[SENSOR_SIDE_RIGHT_ID] < SIDE_WALL_DETECTION);
}

/**
 * @brief Detect the existance or absence of the front wall.
 */
bool front_wall_detection(void) {
  return ((distance[SENSOR_FRONT_LEFT_ID] < FRONT_WALL_DETECTION) && (distance[SENSOR_FRONT_RIGHT_ID] < FRONT_WALL_DETECTION));
}

/**
 * @brief Return left, front and right walls detection readings.
 */
struct walls_around read_walls(void) {
  struct walls_around walls_readings;

  walls_readings.left = left_wall_detection();
  walls_readings.front = front_wall_detection();
  walls_readings.right = right_wall_detection();
  return walls_readings;
}

/**
 * @brief Calibration for side sensors.
 */
void side_sensors_calibration(void) {
  float right_temp = 0;
  float left_temp = 0;

  for (int i = 0; i < SIDE_CALIBRATION_READINGS; i++) {
    left_temp += distance[SENSOR_SIDE_LEFT_ID];
    right_temp += distance[SENSOR_SIDE_RIGHT_ID];
    delay(SENSOR_CALIBRATION_MS);
  }
  calibration_factor[SENSOR_SIDE_LEFT_ID] += (left_temp / SIDE_CALIBRATION_READINGS) - MIDDLE_MAZE_DISTANCE;
  calibration_factor[SENSOR_SIDE_RIGHT_ID] += (right_temp / SIDE_CALIBRATION_READINGS) - MIDDLE_MAZE_DISTANCE;
}
