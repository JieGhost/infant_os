# int_handler.S - the linkage between the hardware interrupt and software int handler

#define ASM     1
#include "interrupt.h"
#include "sched.h"

.globl	handle_rtc
.globl 	handle_keyboard
.globl	handle_timer
.globl	handle_mouse


# ------------------------------------------------------------------
# function name : handle_rtc
# description : linkage of rtc interrupt handler 
# input : none
# output : none
# ------------------------------------------------------------------
handle_rtc:
	cli
	pushl $0				# push dummy	
	pushl $8				# push irq num
	jmp common_interrupt


# ------------------------------------------------------------------
# function name : handle_keyboard
# description : linkage of keyboard interrupt handler 
# input : none
# output : none
# ------------------------------------------------------------------
handle_keyboard:
	cli
	pushl $0				# push dummy	
	pushl $1				# push irq num
	jmp common_interrupt


# ------------------------------------------------------------------
# function name : handle_timer
# description : linkage of timer interrupt handler 
# input : none
# output : none
# ------------------------------------------------------------------
handle_timer:
	cli
	pushl $0				# push dummy
	pushl $0				# push irq num
	jmp common_interrupt


# ------------------------------------------------------------------
# function name : handle_mouse
# description : linkage of mouse interrupt handler 
# input : none
# output : none
# ------------------------------------------------------------------
handle_mouse:
	cli
	pushl $0				# push dummy 
	pushl $12				# push irq num
	jmp common_interrupt



# -------------------------------------------------------------
# function name : common_interrupt
# input : pcb[k]->mm.user_stack, entry_point, pcb[k]->parent
# output : none
# description : save registers, call do_IRQ to handle the INT
# -------------------------------------------------------------
common_interrupt:
					
	cld
	pushl %es				# save regs
	pushl %ds
	pushl %eax
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
	movl $43, %edx
	movl %edx, %ds
	movl %edx, %es

	movl %esp,%eax		    # used as pointer to the struct  
	movl %esp,curr_esp		# save the current kernel esp
	call do_IRQ
	
	popl %ebx				# restore regs after do_IRQ returns
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popl %eax
	popl %ds
	popl %es
	addl $8,%esp
	sti
	iret					

