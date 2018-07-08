#include "lib.h"
#include "videoterm.h"
#include "syscall.h"

/* 
  div_zero_default_handler
	Description: default divide zero handler, it simply kill the
				 current process.
	Input: none
	Output: none 
*/
void div_zero_default_handler()
{
	back_from_exception();		 // call helper function
	return;
}


/* 
  segfault_default_handler
	Description: default segfault handler, it simply kill the
				 current process.
	Input: none
	Output: none 
*/
void segfault_default_handler()
{
	back_from_exception();	     // call helper function
	return;
}


/*
int do_page_fault(void)
{	
	printf("page_fault\n");
	
	int cr2;
	asm("movl %%CR2, %0\n"
		:"=r"(cr2));
	printf("CR2 = %x\n", cr2);
	
	printf("dadas = %x\n",current->pid);
	printf("dasda = %x\n",current->regs.ret_addr);
sti();
	while(1); 
	back_from_exception();
	return 0;
}
*/

