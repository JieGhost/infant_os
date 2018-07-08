#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "types.h"
#include "x86_desc.h"
#include "paging.h"

#ifndef ASM

#define num_processes		8

//structure of all registers information
typedef struct regs_info{
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t ebp;
	uint32_t eax;
	uint32_t ds;
	uint32_t es;
	uint32_t IRQ_NUM;
	uint32_t ret_addr;
	uint32_t cs;
	uint32_t eflags;
	uint32_t esp;
	uint32_t ss;
} regs_info_t;

//structure of file operation table
typedef struct file_op_table
{
    int32_t (*op_open)(void);
	int32_t (*op_read)(uint32_t,uint32_t,void*,int32_t);
	int32_t (*op_write)(const void*,int32_t);
} file_ops_t;

//structure of file descriptors
typedef struct file_desc {
	file_ops_t f_op;		//op table for this file
    uint32_t   inode;       //inode of this file
    uint32_t   f_pos;		// current file offset
    uint32_t   f_flag;		//special flag mark file as in_use
	uint32_t   f_count;  
} file_desc_t;

//structure of memory map
typedef struct mm_struct_t
{
	uint32_t user_stack;
	uint32_t kernel_stack;
}mm_struct;

//structure of process control block
typedef struct process_control_block task_struct;
struct process_control_block
{
	uint32_t pid;
	uint32_t parent_esp;
	uint32_t parent_ebp;
	uint32_t flags;
	mm_struct mm;
	file_desc_t fd_array[8];
	//signal_struct signal;
	page_dir_entry* pd;
	task_struct* parent;
	task_struct* child;
	uint32_t ept;
	int term;
	regs_info_t regs;
	uint32_t switch_esp;
};

task_struct* current;
//task_struct* pcb[num_processes];

void back_from_exception(void);
extern int32_t from_exception(task_struct* t);

extern void system_call(void);

void init_pcb(void);

void set_regs(task_struct* t);

void set_gdt_tss(seg_desc_t* ttt);

extern int32_t user_exec(uint32_t us,uint32_t entry_point,task_struct* t);
extern void back_to_parent(task_struct* t,uint8_t status);
extern void switch_pd(uint32_t pd);

void set_stdio(task_struct* a);

int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler);
int32_t sigreturn(void);



#endif

#endif
