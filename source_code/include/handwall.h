#ifndef HANDWALL_C
#define HANDWALL_C


#include <stdio.h>

#include <sensors.h>
#include <move.h>

void handwall_set_priorize_front(bool priorize);
void handwall_use_left_hand(void);
void handwall_use_right_hand(void);

void handwall_start(void);
void handwall_loop(void);

#endif