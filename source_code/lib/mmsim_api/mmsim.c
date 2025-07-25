#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef MMSIM_ENABLED
#include "control.h"
#include "floodfill.h"

#include "mmsim_api.h"

int main(void) {
  API_log("Running...");

  set_race_started(true);
  floodfill_start_explore();
  while (is_race_started()) {
    floodfill_loop();
  }

  API_log("Finished!");
}
#endif