#ifndef MOVE_H
#define MOVE_H

#include "control.h"
#include "menu_run.h"
#ifndef MMSIM_ENABLED
#include "menu.h"
#include "motors.h"
#include <string.h>
#else
#include "constants.h"
#include "mmsim_api.h"
#include "sensors.h"
#include <stdbool.h>
#include <stdint.h>

#endif

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
  MOVE_DIAGONAL,
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
  MOVE_BACK_STOP,
};

struct inplace_params {
  int16_t start;
  int16_t end;
  int16_t linear_speed;
  float angular_accel;
  float max_angular_speed;
  uint16_t t_accel;
  uint16_t t_max;
  int8_t sign;
};
struct turn_params {
  float start;
  float end;
  int16_t linear_speed;
  float max_angular_speed;
  float transition;
  float arc;
  int8_t sign;
};

struct linear_accel_params {
  int16_t break_accel;
  int16_t accel_hard;
  int16_t speed_hard;
  int16_t accel_soft;
};

struct kinematics {
  int16_t linear_speed;
  struct linear_accel_params linear_accel;
  int16_t fan_speed;
  struct turn_params *turns;
};

bool get_cell_change_toggle_state(void);
bool get_wall_lost_toggle_state(void);

char *get_movement_string(enum movement movement);

enum speed_strategy;
void configure_kinematics(enum speed_strategy speed);
struct kinematics get_kinematics(void);
uint16_t get_floodfill_linear_speed(void);
uint16_t get_floodfill_max_linear_speed(void);
uint16_t get_floodfill_accel(void);

void set_starting_position(void);
int32_t get_current_cell_travelled_distance(void);

void move_straight(int32_t distance, int32_t speed, bool check_wall_loss, bool stop);
void move_straight_until_front_distance(uint32_t distance, int32_t speed, bool stop);
void move_arc_turn(struct turn_params turn);
void move_inplace_turn(enum movement movement);
void move_inplace_angle(float angle, float rads);

void run_straight(float distance, float start_offset, float end_offset, uint16_t cells, bool has_begin, int32_t speed, int32_t final_speed, int8_t next_turn_sign);
void run_side(enum movement movement, struct turn_params turn, struct turn_params next_turn);
void run_diagonal(float distance, float end_offset, uint16_t cells, int32_t speed, int32_t final_speed);

void move(enum movement movement);
void move_run_sequence(enum movement *sequence_movements);

#endif