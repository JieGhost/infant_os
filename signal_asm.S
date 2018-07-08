# signal_asm.S - assmbly instructions that call to handle different signals

#define ASM		1
#include "signal.h"

.globl   kill_process
.globl   handle_alarm
.globl   handle_div_zero
.globl   handle_segfault

user_esp:
	.long 0

kernel_esp: 
	.long 0



# ------------------------------------------------------------------
# function name : kill_process
# input : none
# output : none
# description : called when the interrupt signal is raised
#				kill the current process
# ------------------------------------------------------------------
kill_process:
	
	movl $0, interrupt_sig_flag 
	movl $0, alarm_sig_flag
	addl $4, %esp
	pushl $1						# calling the halt syscall
	jmp continue_syscall			



# ------------------------------------------------------------------
# function name :handle_alarm
# input : none
# output : none
# description : when sig_alarm is raised, set up user stack, and iret
#				to user_alarm_handler if loaded.
# ------------------------------------------------------------------
handle_alarm : 

	cld

	popl %eax
	movl $0, alarm_sig_flag    #clear the alarm

	movl 20(%esp), %eax        # get user stack esp 
	movl %eax, user_esp
	movl %esp, kernel_esp	   # save current kernel esp 
	movl user_esp, %esp	       # go to user stack to set up frame

	pushl $0x80cd0000		   # assembly linkage to sigreturn syscall
	pushl $0x000ab858		   # popl %eax, movl $10,%eax, int 0x80

	movl kernel_esp, %eax	   # copy h/w context onto user stack
	pushl 24(%eax)
	pushl 20(%eax)
	pushl 16(%eax)
	pushl 12(%eax)
	pushl 8(%eax)
	pushl 4(%eax)
	pushl (%eax)

	pushl %es
	pushl %ds
	pushl %eax
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx

	movl $0x2B, %edx		    # set ds , es to user
	movl %edx, %ds
	movl %edx, %es
	
	pushl $2   					# push signum
	movl %esp, %eax				# push user stack
	addl $68, %eax				
	pushl %eax					# push ret addr (to the assem linkage)
	movl %esp, user_esp			
	
	movl kernel_esp, %esp		# back to kernel stack
	addl $8, %esp				
	movl user_alarm_handler, %eax	# set up stack for iret
	movl %eax, (%esp)				# ret addr is user_alarm_handler
	movl user_esp, %eax
	movl %eax, 12(%esp)				
	sti
	iret						 # iret to user alarm handler



# ------------------------------------------------------------------
# function name :handle_div_zero
# input : none
# output : none
# description : when divide zero exception, set up user stack, and iret
#				to user_div_zero_handler if loaded.
# ------------------------------------------------------------------
handle_div_zero :

	cld

	movl user_div_zero_handler, %eax
	cmp $0, %eax
	je div_zero_default_handler

	movl 20(%esp), %eax        # get user stack esp 
	movl %eax, user_esp
	movl %esp, kernel_esp	   # save current kernel esp 
	movl user_esp, %esp	       # go to user stack to set up frame

	# assembly linkage to sigreturn syscall
	# popl %eax, movl $10,%eax, int 0x80
	pushl $0x80cd0000
	pushl $0x000ab858

	# copy h/w context
	movl kernel_esp, %eax
	pushl 24(%eax)
	pushl 20(%eax)
	pushl 16(%eax)
	pushl 12(%eax)
	pushl 8(%eax)
	pushl 4(%eax)
	pushl (%eax)

	pushl %es
	pushl %ds
	pushl %eax
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
	

	movl $0x2B, %edx				# set ds , es to user
	movl %edx, %ds
	movl %edx, %es
	
	pushl $0   # signum				# push signum
	movl %esp, %eax					# push user stack
	addl $68, %eax
	pushl %eax						# push ret addr (to the assem linkage)
	movl %esp, user_esp

	movl kernel_esp, %esp			
	addl $8, %esp
	movl user_div_zero_handler, %eax
	movl %eax, (%esp)
	movl user_esp, %eax
	movl %eax, 12(%esp)
	sti
	iret								# iret to user div zero handler



	
# ------------------------------------------------------------------
# function name :handle_segfault
# input : none
# output : none
# description : when segfault exception, set up user stack, and iret
#				to user_segfault_handler if loaded.
# ------------------------------------------------------------------
handle_segfault:

	cld

	movl user_segfault_handler, %eax
	cmp  $0, %eax
	je  segfault_default_handler

	movl 20(%esp), %eax        # get user stack esp 
	movl %eax, user_esp
	movl %esp, kernel_esp	   # save current kernel esp 
	movl user_esp, %esp	       # go to user stack to set up frame

	# assembly linkage to sigreturn syscall
	# popl %eax, movl $10,%eax, int 0x80
	pushl $0x80cd0000
	pushl $0x000ab858

	# copy h/w context
	movl kernel_esp, %eax
	pushl 24(%eax)
	pushl 20(%eax)
	pushl 16(%eax)
	pushl 12(%eax)
	pushl 8(%eax)
	pushl 4(%eax)
	pushl (%eax)

	pushl %es
	pushl %ds
	pushl %eax
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx

	movl $0x2B, %edx			# set ds , es to user
	movl %edx, %ds
	movl %edx, %es
	
	pushl $1  					 # push signum
	movl %esp, %eax				 # push user stack
	addl $68, %eax
	pushl %eax					 # push ret addr (to the assem linkage)
	movl %esp, user_esp

	movl kernel_esp, %esp
	addl $8, %esp
	movl user_segfault_handler, %eax
	movl %eax, (%esp)
	movl user_esp, %eax
	movl %eax, 12(%esp)
	sti	
	iret						 # iret to user segfault handler

