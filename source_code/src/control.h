#ifndef __CONTROL_H
#define __CONTROL_H

#include <config.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <math.h>

bool is_competicion_iniciada();
void set_competicion_iniciada(bool state);

#endif