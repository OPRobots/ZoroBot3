#ifndef __BASIC_DEBUG_H
#define __BASIC_DEBUG_H

#include "sensors.h"
#include "leds.h"
#include "usart.h"
#include "buttons.h"

void debug_inicio();
void imprimir_sensores_raw();
void imprimir_sensores_filtrados();
void imprimir_sensores_filtrados_analog();

#endif