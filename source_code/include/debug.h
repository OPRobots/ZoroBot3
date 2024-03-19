#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdint.h>

#include <buttons.h>
#include <config.h>
#include <control.h>
#include <delay.h>
#include <motors.h>
#include <sensors.h>
#include <encoders.h>
#include <usart.h>

#define DEBUG_TYPE_SENSORS_RAW 0

void debug_from_config(uint8_t type);

#endif