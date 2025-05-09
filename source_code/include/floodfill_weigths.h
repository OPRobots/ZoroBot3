#ifndef FLOODFILL_WEIGHTS_H
#define FLOODFILL_WEIGHTS_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

struct cell_weigth {
  uint16_t speed;
  float time;
  float total_time;
  float penalty;
};

uint16_t floodfill_weights_cells_to_max_speed(float distance, uint16_t init_speed, uint16_t max_speed, uint16_t accel);
void floodfill_weights_table(float distance, uint16_t init_speed, uint16_t max_speed, uint16_t accel, uint16_t cells_to_max_speed, struct cell_weigth *weights_out);

#endif