# page S file

#define ASM 1
#include "paging.h"

.globl paging_enable
.globl page_dir
.globl page_tab
.globl page_dir_task
.globl page_tab_task
# .globl page_tab_vidmap


.align 4096

# set page_dir to the next mem addr with addr that is multi of 4096 (4kB)
# then fill in 1024 long
page_dir:
	.rept page_dir_size
	.long 0
	.endr
	
.align 4096

# set page_tab to the next mem addr with addr that is multi of 4096 (4kB)
# then fill in 1024 long
page_tab:
	.rept page_table_size
	.long 0
	.endr

# set page_dir_task to the next mem addr with addr that is multi of 4096 (4kB)
# then fill in space_dir_task long
.align 4096
page_dir_task:
	.rept space_dir_task
	.long 0
	.endr

# set page_tab_task to the next mem addr with addr that is multi of 4096 (4kB)
# then fill in space_tab_task long
.align 4096
page_tab_task:
	.rept space_tab_task
	.long 0
	.endr



########################################### 
# paging_enable
#	Description: set CR0,CR3,CR4 registers
#				 to enable paging in our OS
#	Input: none
#	Output: none
###########################################
.align 4
paging_enable:
	pushl	%ebp
	movl	%esp,%ebp

	pushl	%ebx
	
	# put page dir pointer to CR3 PCD flag
	movl	%CR3,%ebx
	andl	$0x00000FFF,%ebx
	orl		$page_dir,%ebx
	movl	%ebx,%CR3
	
	# PSE flag
	movl	%CR4,%ebx
	orl		$0x00000010,%ebx
	movl	%ebx,%CR4
	
	# PG flag memory paging is enabled
	# PE Protected Environment flag
	movl	%CR0,%ebx
	orl		$0x80000001,%ebx
	movl	%ebx,%CR0
	
	popl	%ebx
	
	leave
	ret
