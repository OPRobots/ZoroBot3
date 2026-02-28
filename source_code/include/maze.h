#ifndef MAZE_H
#define MAZE_H

#ifndef MMSIM_ENABLED
#include "menu_run.h"
#endif

#include "floodfill.h"

uint16_t maze_get_rows(void);
uint16_t maze_get_columns(void);
uint16_t maze_get_cells(void);
struct cells_stack *maze_get_goals(void);

#endif