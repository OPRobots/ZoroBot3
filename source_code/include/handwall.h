#ifndef HANDWALL_C
#define HANDWALL_C

#include <stdio.h>

#include <move.h>
#include <sensors.h>


void handwall_use_left_hand(void);
void handwall_use_right_hand(void);
void handwall_set_time_limit(uint32_t ms);

void handwall_start(void);
void handwall_loop(void);

#endif