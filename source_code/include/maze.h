#ifndef MAZE_H
#define MAZE_H

#ifndef MMSIM_ENABLED
#ifndef MAZE_SIMULATOR
#include "menu_run.h"
#endif
#endif

#include "floodfill.h"

uint16_t maze_get_rows(void);
uint16_t maze_get_columns(void);
uint16_t maze_get_cells(void);
struct cells_stack *maze_get_goals(void);

#ifdef MAZE_SIMULATOR
void maze_simulator_set_size(uint16_t rows, uint16_t cols);
#endif

#endif