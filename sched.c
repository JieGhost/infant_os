#include "sched.h"
#include "i8259.h"

/*
  init_schedule
	Description: initialize schedule related data structures
	Input: none
	Output: none
*/
void init_schedule()
{
	runqueue.active_head = task0;  // initialize the active_list to be the 0th array
	runqueue.expired_head = task1;  // initialize the expired_list to be the 1th array
	runqueue.active_tail = task0;
	runqueue.expired_tail = task1;

	int k;
	// link together the linked list
	for (k=0; k<num_processes; k++)
	{
		task0[k].next = &task0[k+1];
		task0[k].pcb_ptr = NULL;
		task1[k].next = &task1[k+1];
		task1[k].pcb_ptr = NULL;
	}
	task0[num_processes].next = NULL; // tail points to NULL
	task0[num_processes].pcb_ptr = NULL;	
	task1[num_processes].next = NULL; // tail points to NULL
	task1[num_processes].pcb_ptr = NULL;	
}
		
/*
  init_schedule
	Description: implement round robin scheduling
				 it uses two lists, one active, one expired
				 every time it put currently running task to
				 expired list and takes one pcb from active list and
				 schedule it to be running until active list is empty,
				 at which time it switches two lists
	Input: none
	Output: none
*/
void schedule()
{
	if (runqueue.active_head == runqueue.active_tail && runqueue.expired_head == runqueue.expired_tail) // no task is running
		return;

	if (current != NULL) //&& current->flags!= 0)
	{
		// save prev task esp reg
		current->switch_esp = curr_esp;

		runqueue.expired_tail->pcb_ptr = current;
		runqueue.expired_tail =runqueue.expired_tail->next;
	}

	// check if the active list is empty
	if(runqueue.active_head == runqueue.active_tail)
	{
		// is empty, switch active/expire lists
		task_node_t* temp;
		temp = runqueue.active_head;
		runqueue.active_head = runqueue.expired_head;
		runqueue.expired_head = temp;

		temp = runqueue.active_tail;
		runqueue.active_tail = runqueue.expired_tail;
		runqueue.expired_tail = temp;
	}

	current = runqueue.active_head->pcb_ptr;

	// update active list
	task_node_t* temp_node = runqueue.active_head;
	runqueue.active_head = runqueue.active_head->next;
	temp_node->next = runqueue.active_tail->next;
	runqueue.active_tail->next = temp_node;
	temp_node->pcb_ptr = NULL;

	if(current->flags!=0)
		task_switch_1();
	else{
		current->flags=1;
		task_switch();
	}
	return;
}

/*
  task_switch_1
	Description: switch between different tasks
				 used by schedule function
	Input: none
	Output: none
*/
void task_switch_1(void)
{	
	//get access to stack of next running task
	curr_esp = current->switch_esp;

	//switch page directory and set TSS value
	switch_pd((uint32_t)(current->pd));
	set_gdt_tss(&tss_desc_ptr);
	tss.ss0 = KERNEL_DS;
	tss.esp0 = current->mm.kernel_stack;
	
	//enable timer interrupt since we won't go back to handler
	enable_irq(0);
	//pop register values and iret to task
	asm volatile("         		\n\
			movl %0,%%esp		\n\
			popl %%ebx			\n\
			popl %%ecx			\n\
			popl %%edx			\n\
			popl %%esi			\n\
			popl %%edi			\n\
			popl %%ebp			\n\
			popl %%eax			\n\
			popl %%ds			\n\
			popl %%es			\n\
			addl $8,%%esp		\n\
			sti					\n\
			iret				\n\
			"
			:
			: "g" (curr_esp)
			: "memory","cc"
			);
}

/*
  task_switch
	Description: used for initializing three shells
				 of three terminals
	Input: none
	Output: none
*/
void task_switch(void)
{
	//switch to processes' page directories and set TSS values 
	switch_pd((uint32_t)(current->pd));
	set_gdt_tss(&tss_desc_ptr);
	tss.ss0 = KERNEL_DS;
	tss.esp0 = current->mm.kernel_stack;
	
	enable_irq(0);
	//switch to user from kernel
	asm volatile("         			\n\
			pushl	$0x002B			\n\
			pushl	%0				\n\
			pushl	%1				\n\
			pushl	$0x0023			\n\
			pushl	%2				\n\
			movw	$0x002B,%%si	\n\
			movw	%%si,%%ds		\n\
			sti						\n\
			iret					\n\
			"
			:
			: "g" (current->regs.esp),"g"(current->regs.eflags),"g"(current->regs.ret_addr)
			: "memory","cc","si"
			);
}

/*
  add_to_list
	Description: switch between different tasks
				 used by schedule function
	Input: add_pcb_ptr: pcb ptr of task needed to be added to list
	Output: none
*/
void add_to_list(task_struct* add_pcb_ptr)
{
	if (runqueue.active_tail == NULL)
		return;
	
	runqueue.active_tail->pcb_ptr = add_pcb_ptr;
	runqueue.active_tail = runqueue.active_tail->next;
}
