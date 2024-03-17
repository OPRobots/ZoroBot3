#ifndef __BASIC_DEBUG_H
#define __BASIC_DEBUG_H

#include "sensors.h"
#include "leds.h"
#include "usart.h"
#include "buttons.h"

void debug_inicio(void);
void imprimir_sensores_raw(void);
void imprimir_sensores_filtrados(void);
void imprimir_sensores_filtrados_analog(void);

#endif