#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef MMSIM_ENABLED
#include "control.h"
#include "floodfill.h"
#include "menu_run.h"

#include "mmsim_api.h"

int MMSIM_EXPLORE_TYPE = EXPLORE_SIMPLE;
int MMSIM_FLOODFILL_TYPE = FLOODFILL_TYPE_BASIC;

int main(int argc, char *argv[]) {

  for (int i = 1; i < argc; i++) {
    if (strncmp(argv[i], "-floodfill-type=", 16) == 0 || strncmp(argv[i], "--floodfill-type=", 17) == 0) {
      const char *val = strchr(argv[i], '=') + 1;
      if (val) {
        switch ((int)strtol(val, NULL, 10)) {
          case FLOODFILL_TYPE_BASIC:
            MMSIM_FLOODFILL_TYPE = FLOODFILL_TYPE_BASIC;
            break;
          case FLOODFILL_TYPE_DIAGONAL:
            MMSIM_FLOODFILL_TYPE = FLOODFILL_TYPE_DIAGONAL;
            break;
          case FLOODFILL_TYPE_TIME:
            MMSIM_FLOODFILL_TYPE = FLOODFILL_TYPE_TIME;
            break;
          case FLOODFILL_TYPE_TIMEv2:
            MMSIM_FLOODFILL_TYPE = FLOODFILL_TYPE_TIMEv2;
            break;
          default:
            fprintf(stderr, "Invalid floodfill type: %s\n", val);
            fflush(stderr);
            exit(EXIT_FAILURE);
            break;
        }
      }
    } else if (strncmp(argv[i], "-explore-type=", 14) == 0 || strncmp(argv[i], "--explore-type=", 15) == 0) {
      const char *val = strchr(argv[i], '=') + 1;
      if (val) {
        switch ((int)strtol(val, NULL, 10)) {
          case EXPLORE_SIMPLE:
            MMSIM_EXPLORE_TYPE = EXPLORE_SIMPLE;
            break;
          case EXPLORE_HOME:
            MMSIM_EXPLORE_TYPE = EXPLORE_HOME;
            break;
          case EXPLORE_COMPLETE:
            MMSIM_EXPLORE_TYPE = EXPLORE_COMPLETE;
            break;
          default:
            fprintf(stderr, "Invalid explore type: %s\n", val);
            fflush(stderr);
            exit(EXIT_FAILURE);
            break;
        }
      }
    }
  }

  API_log("Running...");

  set_race_started(true);
  floodfill_start_explore();
  while (is_race_started()) {
    floodfill_loop();
  }

  API_log("Finished!");
}
#endif