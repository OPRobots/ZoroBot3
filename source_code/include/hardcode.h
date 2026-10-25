#ifndef HARDCODE_H
#define HARDCODE_H

#include <stdbool.h>
#include <stdint.h>

void hardcode_reset(void);
bool hardcode_is_hand_selected(void);

void hardcode_toggle_hand(void);
void hardcode_use_left_hand(void);
void hardcode_use_right_hand(void);
void hardcode_set_time_limit(uint32_t ms);

void hardcode_start(void);
void hardcode_loop(void);

#endif
