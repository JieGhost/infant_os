# exception.S - linkage between exception and handler

#define ASM 1

#include "signal.h" 
#include "exception.h"

.globl divide_error
.globl debug
.globl nmi
.globl int3
.globl overflow
.globl bounds
.globl invalid_op
.globl device_not_available
.globl doublefault_fn
.globl coprocessor_segment_overrun
.globl invalid_TSS
.globl segment_not_present
.globl stack_segment
.globl general_protection
.globl page_fault
.globl coprocessor_error
.globl alignment_check
.globl machine_check
.globl simd_coprocessor_error


# ------------------------------------------------------------------
# the following linkages is call when the corresponding exception 
# occurs, it will push a dummy(if necessary), and the exception #
# onto the stack and then call the signal handler.
# ------------------------------------------------------------------


divide_error:
	cli
	pushl	$0
	pushl	$0
	jmp		handle_div_zero
	
debug:
	cli
	pushl	$0
	pushl	$1
	jmp		handle_segfault
	
nmi:
	cli
	pushl	$0
	pushl	$2
	jmp		handle_segfault
	
int3:
	cli			
	pushl	$0
	pushl	$3
	jmp		handle_segfault

overflow:
	cli
	pushl	$0
	pushl   $4
	jmp		handle_segfault
	
bounds:
	cli
	pushl	$0
	pushl	$5
	jmp		handle_segfault
	
invalid_op:
	cli
	pushl	$0
	pushl	$6
	jmp		handle_segfault
	
device_not_available:
	cli
	pushl	$0
	pushl	$7
	jmp		handle_segfault
	
doublefault_fn:
	cli
	# pushl	$0
	pushl	$8
	jmp		handle_segfault
	
coprocessor_segment_overrun:
	cli
	pushl	$0
	pushl	$9
	jmp		handle_segfault
	
invalid_TSS:
	cli
	# pushl	$0
	pushl	$10
	jmp		handle_segfault
	
segment_not_present:
	cli
	# pushl	$0
	pushl	$11
	jmp		handle_segfault
	
stack_segment:
	cli
	# pushl	$0
	pushl	$12
	jmp		handle_segfault
	
general_protection:
	cli
	# pushl	$0
	pushl   $13
	jmp		handle_segfault
	
page_fault:
	cli
	# pushl	$0
	pushl	$14
	jmp		handle_segfault
	
coprocessor_error:
	cli
	pushl	$0
	pushl	$15
	jmp		handle_segfault
	
alignment_check:
	cli
	pushl	$0
	pushl	$16
	jmp		handle_segfault
	
machine_check:
	cli
	pushl	$0
	pushl	$17
	jmp		handle_segfault
	
simd_coprocessor_error:
	cli
	pushl	$0
	pushl	$18
	jmp		handle_segfault
	
