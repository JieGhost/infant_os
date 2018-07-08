# systemcall.S - system call service routine

#define ASM     1
#include "syscall.h"


.globl	system_call
.globl  sigreturn
.globl  continue_syscall


# ------------------------------------------------------------------
# function name : system_call
# input : none
# output : none
# description : system_call service routine
# ------------------------------------------------------------------
system_call:
	cli
	pushl   $0							# push dummy
	pushl	%eax						# push syscall num

	pushl %eax							# save eax

	movl interrupt_sig_flag, %eax		# check if interrupt sig is raised
	cmp $0, %eax
	jne  check_curr_term

	movl alarm_sig_flag, %eax			# check if alarm sig is raised
	cmp $0, %eax
	je continue_syscall

	movl user_alarm_handler, %eax		# check if user handler is loaded
	cmp  $0, %eax
	jne  handle_alarm

continue_syscall:
	popl %eax
	sti
	cld
	pushl	%es							# save registers
	pushl	%ds
	pushl	%eax
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	movl	$43,%edx					
	movl	%edx,%ds
	movl 	%edx,%es

	call 	*sys_call_table(,%eax,4)	# use jmp table to jmp to corresponding system call
	movl	%eax,24(%esp)				# save return value of the sys call

	popl	%ebx						# restore regs
	popl	%ecx
	popl	%edx
	popl	%esi
	popl	%edi
	popl	%ebp
	popl	%eax
	popl	%ds
	popl	%es

	addl $8,%esp
	iret								# iret to user program



# ------------------------------------------------------------------
# function name : check_curr_term
# input : none
# output : none
# description : check if killing the correct process 
#				(the one runung in current terminal).
# ------------------------------------------------------------------
check_curr_term:
	
	movl curr_term, %eax
	movl current, %ebx
	movl 264(%ebx), %ebx				# get the current terminal num from the pcb
	cmp %eax, %ebx						# if is the correct process
	je kill_process						# go kill_process
	jmp continue_syscall				# else, resume normal service routine




# ---------------------------------------------------------------------
# function name : sigreturn
# input : signum	
# output : 0 on success, -1 on fail
# description : the sigreturn system call (sysnum #10).
#				return from the signal handler back to the user program
# ---------------------------------------------------------------------
sigreturn:

	addl $4, %esp
	movl %esp, %eax
	addl $56, %eax				# get user esp
	movl (%eax), %ebx			# ebx <- user esp

	movl $15, %edx

# a loop to get the top 16 elements(h/w context) on user stack
# and copy the contents on the kernel stack
copy_hw:						
	cmp  $-1, %edx
	je	 done_ret
	movl (%ebx,%edx,4), %eax	
	movl %eax, (%esp,%edx,4)	
	addl $-1, %edx
	jmp copy_hw

done_ret: 
	movl	$43,%edx
	movl	%edx,%ds
	movl 	%edx,%es
	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popl %eax
	popl %ds
	popl %es
	addl $8, %esp
	iret						 # iret to the user program



# ----------------------------------------------------------------------
# sys_call_table - the jump table that jmps to the sys call
#				 - based on the sig num
# ----------------------------------------------------------------------
sys_call_table:
	.long	0
	.long	halt
	.long	execute
	.long	read
	.long	write
	.long	open
	.long	close
	.long	getargs
	.long	vidmap
	.long	set_handler
	.long	sigreturn



