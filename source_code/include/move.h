#ifndef MOVE_H
#define MOVE_H

#include "control.h"

enum movement {
  MOVE_END,
  MOVE_START,
  MOVE_FRONT,
  MOVE_LEFT,
  MOVE_RIGHT,
  MOVE_LEFT_90,
  MOVE_RIGHT_90,
  MOVE_180,
  MOVE_180W,
};

struct turn_params {
  int16_t start;
  int16_t end;
  int16_t linear_speed;
  float angular_accel;
  float max_angular_speed;
  uint16_t t_accel;
  uint16_t t_max;
  int8_t sign;
};

void move_straight(int32_t distance, int32_t speed, bool stop);
void move_straight_until_front_distance(uint32_t distance, int32_t speed, bool stop);
void move_arc_turn(enum movement move);
void move_inplace_turn(float angle, float rads);

#endif