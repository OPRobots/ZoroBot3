#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdint.h>

#include <buttons.h>
#include <config.h>
#include <control.h>
#include <delay.h>
#include <encoders.h>
#include <macroarray.h>
#include <motors.h>
#include <sensors.h>
#include <usart.h>


#define DEBUG_NONE 0
#define DEBUG_MACROARRAY 1
#define DEBUG_TYPE_SENSORS_RAW 2

void debug_from_config(uint8_t type);

#endif