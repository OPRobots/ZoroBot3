#include "macroarray.h"

static uint32_t macroarray_last_update_ms = 0;
static uint8_t macroarray_size = 0;
static uint16_t macroarray_float_bits = 0;
static uint16_t macroarray_start = 0;
static uint16_t macroarray_end = 0;
static int16_t macroarray[MACROARRAY_LENGTH];
static char *macroarray_labels[20];

void macroarray_store(uint8_t ms, uint16_t float_bits, char **labels, uint8_t size, ...) {
  if (get_clock_ticks() - macroarray_last_update_ms < ms || size > 20) {
    return;
  } else {
    macroarray_last_update_ms = get_clock_ticks();
  }

  if (macroarray_size != size) {
    macroarray_start = 0;
    macroarray_end = 0;
    macroarray_size = size;
    macroarray_float_bits = float_bits;
    for (uint8_t i = 0; i < size; i++) {
      macroarray_labels[i] = labels[i];
    }
  }

  if (macroarray_end + size > MACROARRAY_LENGTH) {
    macroarray_start = macroarray_end - size;
    macroarray_end = 0;
  }

  va_list valist;
  va_start(valist, size);
  for (uint8_t i = 0; i < size; i++) {
    macroarray[macroarray_end++] = va_arg(valist, int);
  }
  va_end(valist);
}

void macroarray_print(void) {
  if (macroarray_size == 0) {
    return;
  }
  uint16_t i = macroarray_start;
  uint8_t col = 1;
  do {
    if (macroarray_float_bits & (1 << (macroarray_size - col))) {
      printf(">%s:%.2f", macroarray_labels[col - 1], macroarray[i] / 100.0);
    } else {
      printf(">%s:%d", macroarray_labels[col - 1], macroarray[i]);
    }
    printf("\n");
    if (col == macroarray_size) {
      col = 1;
      i++;
      if (i + macroarray_size > MACROARRAY_LENGTH) {
        i = 0;
      }
    } else {
      col++;
      i++;
    }
  } while (i != macroarray_end);
}
