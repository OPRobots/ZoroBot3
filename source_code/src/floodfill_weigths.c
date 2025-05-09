#include "floodfill_weigths.h"

static float second_degree_equation(float a, float b, float c) {
  double delta = b * b - 4 * a * c;
  if (delta < 0) {
    return -1;
  }
  return (float)((-b + sqrt(delta)) / (2 * a));
}

static float time_penalty(uint16_t speed, uint16_t init_speed, uint16_t accel) {
  if (init_speed < speed) {
    return (speed - init_speed) / (float)accel;
  } else {
    return 0.0f;
  }
}

static float time_taken_distance(float distance, uint16_t speed, uint16_t max_speed, uint16_t accel) {
  if (speed < max_speed) {
    float time_to_max_speed = (max_speed - speed) / (float)accel;
    float time_to_distance = second_degree_equation(0.5f * accel, speed, -distance);
    if (time_to_distance < time_to_max_speed) {
      return time_to_distance;
    } else {
      float left_distance = distance - (speed * time_to_max_speed) + (0.5f * accel * time_to_max_speed * time_to_max_speed);
      return time_to_max_speed + (left_distance / max_speed);
    }
  } else {
    return distance / (float)speed;
  }
  return 0;
}

static uint16_t speed_after_time(float time, uint16_t speed, uint16_t max_speed, uint16_t accel) {
  if (speed < max_speed) {
    float speed_at_time = speed + (accel * time);
    return speed_at_time > max_speed ? max_speed : (uint16_t)speed_at_time;
  } else {
    return max_speed;
  }
}

uint16_t floodfill_weights_cells_to_max_speed(float distance, uint16_t init_speed, uint16_t max_speed, uint16_t accel) {
  float time_to_max_speed = (max_speed - init_speed) / (float)accel;
  float distance_to_max_speed = (init_speed * time_to_max_speed) + (0.5f * accel * time_to_max_speed * time_to_max_speed);
  return (uint16_t)round((distance_to_max_speed / distance)) + 2 + 1;
}

void floodfill_weights_table(float distance, uint16_t init_speed, uint16_t max_speed, uint16_t accel, uint16_t cells_to_max_speed, struct cell_weigth *weights_out) {
  uint16_t speed = init_speed;
  float time = distance / speed;
  float penalty = 0.0f;
  float total_time = time;
  weights_out[0].speed = speed;
  weights_out[0].time = time;
  weights_out[0].total_time = total_time;
  weights_out[0].penalty = penalty;
  for (uint16_t i = 1; i < cells_to_max_speed; i++) {
    time = time_taken_distance(distance, speed, max_speed, accel);
    total_time += time;
    speed = speed_after_time(time, speed, max_speed, accel);
    penalty = time_penalty(speed, init_speed, accel);
    weights_out[i].speed = speed;
    weights_out[i].time = time;
    weights_out[i].total_time = total_time;
    weights_out[i].penalty = penalty;
  }

  // for (uint16_t i = 0; i < cells_to_max_speed; i++) {
  //   printf("CELL %d: \n", i + 1);
  //   printf("  Speed: %d\n", weights_out[i].speed);
  //   printf("  Time: %.4f\n", weights_out[i].time);
  //   printf("  Total Time: %.4f\n", weights_out[i].total_time);
  //   printf("  Penalty: %.4f\n", weights_out[i].penalty);
  //   printf("\n");
  // }
}