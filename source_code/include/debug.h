#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdint.h>

#include <buttons.h>
#include <config.h>
#include <constants.h>
#include <control.h>
#include <delay.h>
#include <encoders.h>
#include <floodfill.h>
#include <macroarray.h>
#include <motors.h>
#include <sensors.h>
#include <usart.h>

#define DEBUG_NONE 0
#define DEBUG_MACROARRAY 1
#define DEBUG_TYPE_SENSORS_RAW 2
#define DEBUG_TYPE_SENSORS_DISTANCES 3
#define DEBUG_FLOODFILL_MAZE 4
#define DEBUG_MOTORS_CURRENT 5
#define DEBUG_GYRO_DEMO 9
#define DEBUG_FAN_DEMO 10

bool is_debug_enabled(void);
void debug_from_config(uint8_t type);
void debug_from_main(uint8_t type);

#endif