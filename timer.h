#ifndef _TIMER_H
#define _TIMER_H

#include "sched.h"
#include "lib.h"
#include "types.h"
#include "terminal.h"

void do_timer(void);
void init_timer(void);
void play_sound(int32_t nFrequence);
void nosound();
void beep(int32_t nFrequence, int32_t duration);
#endif
