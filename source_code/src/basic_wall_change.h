#ifndef __BASIC_WALL_CHANGE_H
#define __BASIC_WALL_CHANGE_H

#include "basic_algorithm.h"
#include "leds.h"
#include "usart.h"
#include "buttons.h"

bool check_reference_wall_change(uint32_t startedMillis, bool mano);
bool configuracion(void);


#endif