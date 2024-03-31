#ifndef MOVE_H
#define MOVE_H

#include "control.h"

// void move();
void move_straight(int32_t distance, int32_t speed, bool stop);
void move_inplace_turn(float angle, float rads);

#endif