# exec.S - memory operation & instruction used for the context switch between 
#		 - different processes. 

#define ASM		1
#include "syscall.h"
#include "paging.h"
#include "signal.h"

.globl user_exec
.globl switch_pd
.globl back_to_parent
.globl from_exception
.globl shell_ss
.globl shell_bp


shell_ss:
	.long 0
shell_bp:
	.long 0
	


# --------------------------------------------------------------------
# function name : switch_pd
# input : pd pointer
# output : none
# description : set the CR3 to points to next process's pd
# ----------------------------------------------------------
switch_pd:

	pushl	%ebp				
	movl	%esp,%ebp
	pushl	%ebx				 # callee saved regs
	pushl	%esi
	
	movl	8(%ebp),%esi		 # get pd pointer
	
	movl	%CR3,%ebx
	andl	$0x00000FFF,%ebx		
	orl		%esi,%ebx
	movl	%ebx,%CR3			 # update CR3
	
	popl	%esi
	popl	%ebx
	leave
	ret



# --------------------------------------------------------------------
# function name : user_exec
# input : pcb[k]->mm.user_stack, entry_point, pcb[k]->parent
# output : none
# description : execute the 2nd process using iret. Never return back
# --------------------------------------------------------------------
user_exec:

	movl	12(%esp),%esi       
	movl	(%esi),%edi			 
	cmpl	$0,%edi				  # check if parent is the 0th process
	je		now_do				  # if true, go now_do
	movl	%esp,4(%esi)		  # save esp in pcb
	movl	%ebp,8(%esi)		  # save ebp in pcb

now_do:
	movl	4(%esp),%esi		  # prepare stack for iret
	movl	8(%esp),%edi
	
	pushfl
	popl	%ecx				  # push eflags
	orl		$0x4000,%ecx

	pushl	$0x002B				  # push segment selector
	pushl	%esi
	pushl	%ecx
	pushl	$0x0023
	pushl	%edi
	movw	$0x002B,%si
	movw	%si,%ds
	movw	$0x0030,%si
	
	iret						   # iret to the new program



# ----------------------------------------------------------------------------
# function name : back_to_parent
# input : current, int status
# output : none
# description :	return to the parent program if calls halt or program finishes
# ----------------------------------------------------------------------------
back_to_parent:
	movl	4(%esp),%esi			#restore the parent esp and eip
	movb	8(%esp),%bl	
	movzbl	%bl,%eax
	movl	4(%esi),%esp
	movl	8(%esi),%ebp
	
	movl $0, user_div_zero_handler	# clear all flags before return
	movl $0, user_segfault_handler
	movl $0, user_alarm_handler

	ret								# return to the parent program



# ---------------------------------------------------------------------------
# function name : from_exception
# input : task_struct* current 
# output : 256 
# description : return from a exception to parent process
# ---------------------------------------------------------------------------
from_exception:	
	movl	4(%esp),%eax
	movl	4(%eax),%esp		    #restore the parent esp and eip
	movl	8(%eax),%ebp
	
	movl	$256,%eax
	movl $0, user_div_zero_handler	# clear all flags before return
	movl $0, user_segfault_handler
	movl $0, user_alarm_handler

	ret
