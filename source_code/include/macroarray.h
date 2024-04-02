#ifndef MACROARRAY_H
#define MACROARRAY_H

#include <stdio.h>
#include <stdarg.h>

#include "delay.h"

#define MACROARRAY_LENGTH 30000
#define MACROARRAY_SEPARATOR "\t"

void macroarray_store(uint8_t ms, uint16_t float_bits, uint8_t size, ...);
void macroarray_print(void);

#endif