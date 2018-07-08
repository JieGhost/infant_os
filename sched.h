#ifndef _SCHED_H
#define _SCHED_H

#include "syscall.h"
#include "types.h"
#include "paging.h"
#include "x86_desc.h"

#ifndef ASM

//structure of node in task list
typedef struct task_list_node task_node_t; 
struct task_list_node {
	task_node_t* next;
	task_struct* pcb_ptr;
};

// the first node in the list is sentinal head
task_node_t task0[num_processes+1];
task_node_t task1[num_processes+1];

//structure of run queue, contains pointers of two
//task lists
typedef struct run_queue {
	task_node_t* active_head;
	task_node_t* expired_head;
	task_node_t* active_tail;
	task_node_t* expired_tail;
} run_queue_t;

run_queue_t runqueue;

void init_schedule(void);
void schedule(void);
void add_to_list(task_struct* add_pcb_ptr);

void task_switch(void);

void task_switch_1(void);

uint32_t curr_esp;

#endif

#endif
