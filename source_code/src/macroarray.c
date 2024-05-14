#include "macroarray.h"

static uint32_t macroarray_last_update_ms = 0;
static uint8_t macroarray_size = 0;
static uint16_t macroarray_float_bits = 0;
static uint16_t macroarray_start = 0;
static uint16_t macroarray_end = 0;
static int16_t macroarray[MACROARRAY_LENGTH];

void macroarray_store(uint8_t ms, uint16_t float_bits, uint8_t size, ...) {
  if (get_clock_ticks() - macroarray_last_update_ms < ms) {
    return;
  } else {
    macroarray_last_update_ms = get_clock_ticks();
  }

  if (macroarray_size != size) {
    macroarray_start = 0;
    macroarray_end = 0;
    macroarray_size = size;
    macroarray_float_bits = float_bits;
  }

  va_list valist;
  va_start(valist, size);
  for (uint8_t i = 0; i < size; i++) {
    macroarray[macroarray_end] = va_arg(valist, int);

    if (macroarray_end < macroarray_start) {
      macroarray_end++;
      macroarray_start++;
    } else {
      macroarray_end++;
    }
    if (macroarray_start >= MACROARRAY_LENGTH) {
      macroarray_start = 0;
    }
    if (macroarray_end >= MACROARRAY_LENGTH) {
      macroarray_end = 0;
      macroarray_start = 1;
    }
  }
  va_end(valist);
}

void macroarray_print(void) {
  if (macroarray_start == macroarray_end || macroarray_size == 0) {
    return;
  }
  uint16_t i = macroarray_start;
  uint8_t col = 1;
  do {
    if (macroarray_float_bits & (1 << (macroarray_size - col))) {
      printf(">log%d:%.2f",col, macroarray[i] / 100.0);
    } else {
      printf(">log%d:%d",col, macroarray[i]);
    }
    if (col == macroarray_size) {
      printf("\n");
      col = 1;
    } else {
      printf("\n"/* MACROARRAY_SEPARATOR */);
      col++;
    }

    i++;
    if (i >= MACROARRAY_LENGTH) {
      i = 0;
    }
  } while (i != macroarray_end);
}
