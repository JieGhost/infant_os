#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include "types.h"

#ifndef ASM

//structure of stack after interrupt happens
struct pt_regs
{
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t ebp;
	uint32_t eax;
	uint32_t ds;
	uint32_t es;
	uint32_t IRQ_NUM;
	uint32_t ret_addr;
	uint32_t cs;
	uint32_t eflags;
	uint32_t esp;
	uint32_t ss;
};

extern void handle_timer(void);
extern void handle_keyboard(void);
extern void handle_rtc(void);
extern void handle_mouse(void);

#endif

#endif
