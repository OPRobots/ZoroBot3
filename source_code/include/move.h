#ifndef MOVE_H
#define MOVE_H

#include "control.h"
#include "menu.h"
#include "menu_run.h"
#include <string.h>

enum movement {
  MOVE_NONE,
  MOVE_HOME,
  MOVE_START,
  MOVE_END,
  MOVE_FRONT,
  MOVE_LEFT,
  MOVE_RIGHT,
  MOVE_LEFT_90,
  MOVE_RIGHT_90,
  MOVE_LEFT_180,
  MOVE_RIGHT_180,
  MOVE_LEFT_TO_45,
  MOVE_RIGHT_TO_45,
  MOVE_LEFT_TO_135,
  MOVE_RIGHT_TO_135,
  MOVE_LEFT_45_TO_45,
  MOVE_RIGHT_45_TO_45,
  MOVE_LEFT_FROM_45,
  MOVE_RIGHT_FROM_45,
  MOVE_LEFT_FROM_45_180,
  MOVE_RIGHT_FROM_45_180,
  MOVE_BACK,
  MOVE_BACK_WALL,
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

struct kinematics {
  int16_t linear_speed;
  int16_t linear_accel;
  struct turn_params *turns;
};

char *get_movement_string(enum movement movement);

enum speed_strategy;
void configure_kinematics(enum speed_strategy speed);
struct kinematics get_kinematics(void);

void set_starting_position(void);
int32_t get_current_cell_travelled_distance(void);

void move_straight(int32_t distance, int32_t speed, bool check_wall_loss, bool stop);
void move_straight_until_front_distance(uint32_t distance, int32_t speed, bool stop);
void move_arc_turn(enum movement move);
void move_inplace_turn(enum movement movement);
void move_inplace_angle(float angle, float rads);

void run_straight(int32_t distance, uint16_t cells, bool has_begin, int32_t speed, int32_t final_speed);

void move(enum movement movement);
void move_run_sequence(char *sequence, enum movement *sequence_movements);

#endif