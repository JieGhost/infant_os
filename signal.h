#ifndef _SIGNAL_H
#define _SIGNAL_H

#ifndef ASM

volatile int alarm_sig_flag;
volatile int interrupt_sig_flag;

void* user_segfault_handler;
void* user_div_zero_handler;
void* user_alarm_handler;

void div_zero_default_handler(void);
void segfault_default_handler(void);

#endif

#endif

