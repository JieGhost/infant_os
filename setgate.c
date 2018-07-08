#include "setgate.h"

/*
  set_intr_gate 
	Description: set idt[n] to intr gate and attach handler at
				 addr to idt[n]
	Input: handler function at addr, index number n
	Output: none
*/
void set_intr_gate(int n, void* addr)
{
	idt[n].seg_selector = 0x10;
	idt[n].reserved4	= 0;
	idt[n].reserved3	= 0;
	idt[n].reserved2	= 1;
	idt[n].reserved1	= 1;
	idt[n].size			= 1;
	idt[n].reserved0	= 0;
	idt[n].dpl			= 0;
	idt[n].present		= 1;
	SET_IDT_ENTRY(idt[n], addr);
}


/*
  set_system_gate 
	Description: set idt[n] to system gate and attach handler at
				 addr to idt[n]
	Input: handler function at addr, index number n
	Output: none
*/
void set_system_gate(int n, void* addr)
{
	idt[n].seg_selector = 0x10;
	idt[n].reserved4	= 0;
	idt[n].reserved3	= 1;
	idt[n].reserved2	= 1;
	idt[n].reserved1	= 1;
	idt[n].size			= 1;
	idt[n].reserved0	= 0;
	idt[n].dpl			= 3;
	idt[n].present		= 1;
	SET_IDT_ENTRY(idt[n], addr);
}


/*
  set_system_intr_gate 
	Description: set idt[n] to system_intr gate and attach handler at
				 addr to idt[n]
	Input: handler function at addr, index number n
	Output: none
*/
void set_system_intr_gate(int n, void* addr)
{
	idt[n].seg_selector = 0x10;
	idt[n].reserved4	= 0;
	idt[n].reserved3	= 0;
	idt[n].reserved2	= 1;
	idt[n].reserved1	= 1;
	idt[n].size			= 1;
	idt[n].reserved0	= 0;
	idt[n].dpl			= 3;
	idt[n].present		= 1;
	SET_IDT_ENTRY(idt[n], addr);
}


/*
  set_trap_gate 
	Description: set idt[n] to trap gate and attach handler at
				 addr to idt[n]
	Input: handler function at addr, index number n
	Output: none
*/
void set_trap_gate(int n, void* addr)
{
	idt[n].seg_selector = 0x10;
	idt[n].reserved4	= 0;
	idt[n].reserved3	= 1;
	idt[n].reserved2	= 1;
	idt[n].reserved1	= 1;
	idt[n].size			= 1;
	idt[n].reserved0	= 0;
	idt[n].dpl			= 0;
	idt[n].present		= 1;
	SET_IDT_ENTRY(idt[n], addr);
}


/*
  set_task_gate 
	Description: set idt[n] to task gate and attach handler at
				 addr to idt[n]
	Input: handler function at addr, index number n
	Output: none
*/
void set_task_gate(int n, void* addr)
{
	idt[n].seg_selector = 0x10;
	idt[n].reserved4	= 0;
	idt[n].reserved3	= 1;
	idt[n].reserved2	= 0;
	idt[n].reserved1	= 1;
	idt[n].size			= 0;
	idt[n].reserved0	= 0;
	idt[n].dpl			= 3;
	idt[n].present		= 1;
	SET_IDT_ENTRY(idt[n], addr);
}
