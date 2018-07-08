#include "lib.h"
#include "syscall.h"
#include "filesys.h"
#include "terminal.h"
#include "rtc.h"
#include "sched.h"
#include "signal.h"

static int num_task = 0;

static uint8_t args[50];

static task_struct* pcb[num_processes];
void fill_fd(int i, dentry_t* dentry);

int32_t skip_open(void);
int32_t skip_read(uint32_t inode,uint32_t offset,void* buf,uint32_t nbytes);
int32_t skip_write(const void* buf, int32_t nbytes);

/*
  init_pcb
	Description: initialize pcb array
				 set all pcb pointer to null
	input: none
	output: none
*/
void init_pcb(void)
{
	int i;
	current = NULL;

	for(i=0;i<num_process;i++)
		pcb[i] = NULL;
		
	pcb[0] = (task_struct*)0x800000 - 0x2000;
	pcb[0]->pid = 0;
	pcb[0]->flags = 1;
	pcb[0]->parent = NULL;
}

/*
  set_regs
	Description: set regs of process control block
	input: t: pointer to task_struct
	output: none
*/
void set_regs(task_struct* t)
{	
	uint32_t a;

	t->regs.ds = USER_DS;

	t->regs.ret_addr = t->ept;
	t->regs.cs = USER_CS;

	t->regs.esp = t->mm.user_stack;
	t->regs.ss = USER_DS;
	
	//set eflags
	asm volatile("          \n\
			pushfl			\n\
			popl	%0		\n\
			"
			: "=g" (a)
			:
			: "memory"
			);
	
	t->regs.eflags = a;
}



/*
  set_gdt_tss
	Description: set TSS values
	input: TSS pointer
	output: none
*/
void set_gdt_tss(seg_desc_t* ttt)
{
	ttt->seg_lim_15_00 = tss_size & 0x0000FFFF;
	ttt->base_15_00 = (uint32_t)(&tss) & 0x0000FFFF;
	ttt->base_23_16 = ((uint32_t)(&tss) & 0x00FF0000) >> 16;
	ttt->type = 0xB;
	ttt->sys = 0;
	ttt->dpl = 3;
	ttt->present = 1;
	ttt->seg_lim_19_16 = (tss_size & 0x000F0000) >> 16;
	ttt->avail = 1;
	ttt->reserved = 0;
	ttt->opsize = 0;
	ttt->granularity = 0;
	ttt->base_31_24 = ((uint32_t)(&tss) & 0xFF000000) >> 24;
}

/*
  back_from_exception
	Description: called when process trigger an
				 exception, halt current process
				 and return to parent process
	input: none
	output: none
*/
void back_from_exception(void)
{
	if(current->pid > 1)
	{
		pcb[current->pid] = NULL;
		current = current->parent;
		current->child->parent = NULL;
		current->child = NULL;
		
		num_task--;
		
		//set CR3 to shell
		switch_pd((uint32_t)current->pd);
		
		//set TSS
		set_gdt_tss(&tss_desc_ptr);
		tss.ss0 = KERNEL_DS;
		tss.esp0 = current->mm.kernel_stack;
		
		//return value to parent process
		from_exception(current);
	}
}


/*
  halt
	Description: terminates a process, returning the specified value to its parent process
	input: return value to shell: status 
	output: -1 if not halting the 2nd user program
*/
int32_t halt(uint8_t status)
{
	if(num_task > 3)
	{
		//num_task--;
		pcb[current->pid] = NULL;
		current = current->parent;
		current->child->parent = NULL;
		current->child = NULL;
		
		num_task--;
		
		//set CR3 to shell
		switch_pd((uint32_t)current->pd);
		
		//set TSS
		set_gdt_tss(&tss_desc_ptr);
		tss.ss0 = KERNEL_DS;
		tss.esp0 = current->mm.kernel_stack;
		
		//return value to parent process
		back_to_parent(current,status);
		return 0;
	}
	else
	{	sti();
		while(1);
		return -1;
	}

}
/*
  execute sys call
	Description: attempts to load and execute a new program, 
				 handing off the processor to the new program 
				 until it terminates
	input: command string
	output: not exist, not an executable command file -1
			program dies by an exception 256
			program executes a halt system call 0 to 255
*/
int32_t execute(const uint8_t* command)
{
	//char* fn;
	uint8_t fn[50];
	int k;
	int i=0;
	int j=0;
	dentry_t dt;
	uint8_t* dest;
	uint32_t entry_point;
	
	//destination to copy the file
	dest = (uint8_t*)0x08048000;
	
	//clear the file name and arg buffer
	for(i=0;i<50;i++)
	{
		fn[i] = '\0';
		args[i] = '\0';
	}
	i = 0;
	
	//copy the file name in command buffer into the file name buffer
	while((command[i]!=' ') && (command[i]!='\n') && (command[i]!='\0'))
	{
		fn[i] = command[i];
		i++;
	}
	//add EOS
	fn[i] = '\0';
	
	i++;
	
	//if there are cmd after file name, copy the cmd into args buffer
	while(command[j+i]!='\n' && command[j+i]!='\0')
	{
		args[j] = command[j+i];
		j++;
	}
	args[j] = '\0';
	
	//check if the filename is valid
	if(strncmp((int8_t*)fn,"clear",5)==0)
		return 0;

	if(strncmp((int8_t*)fn,"soundoff",8)==0)
		return 0;

	if(strncmp((int8_t*)fn,"soundon",7)==0)
		return 0;

	if(read_dentry_by_name(fn,&dt)==-1)
		return -1;
	
	//check if the file is executable then retrun the entry point of the program
	entry_point = check_executable(dt.inode);
	
	if(entry_point==0)
		return -1;
		
	///////////////////////////////////////////
	
	if(num_task < num_processes - 1)
	{	
		num_task++;
		//find available PCB
		for(k=1;k<num_processes;k++)
		{
			if(pcb[k]==NULL)
				break;
		}
		
		//assign pcb address
		pcb[k] = (task_struct*)(0x800000 - 0x2000 * (k + 1));
		
		//initialize PCB struct
		pcb[k]->pid = k;
		pcb[k]->mm.user_stack = 0x400000 + 0x08000000 - 0x4;
		pcb[k]->mm.kernel_stack = 0x800000 - 0x2000 * k - 0x4;

		//initialize stdio for the 1st PCB (shell)
		set_stdio(pcb[k]);
		
		pcb[k]->pd = page_dir_task[pcb[k]->pid - 1];
		
		//non shell process
		if(num_task > 3)
		{
			pcb[k]->flags = 1;
			current->child = pcb[k];
			pcb[k]->parent = current;
			pcb[k]->child = NULL;
			pcb[k]->ept = entry_point;
			pcb[k]->term = current->term;
		
			current = pcb[k];
		}
		else
		{
			pcb[k]->flags = 0;
			pcb[k]->parent = pcb[0];
			pcb[k]->child = NULL;
			pcb[k]->ept = entry_point;
			
			pcb[k]->term = num_task - 1;
		}
		
		set_regs(pcb[k]);
		
		//ser CR3
		switch_pd((uint32_t)(pcb[k]->pd));
		//destination to copy the file
		copy_executable(dt.inode,dest);

		//set TSS
		set_gdt_tss(&tss_desc_ptr);
		tss.ss0 = KERNEL_DS;
		tss.esp0 = pcb[k]->mm.kernel_stack;
		
		if(num_task<=3)
		{	
			add_to_list(pcb[k]);
			return 0;
		}
		
		//return halt return value
		return user_exec(pcb[k]->mm.user_stack,entry_point,pcb[k]->parent);
	}
	
	return -1;
}

/*
  set_stdio
	Description: initialize stdio fd entry with 
				 read_terminal and write_terminal
	input: PCB pointer a
	output: none
*/
void set_stdio(task_struct* a)
{
	int i;
	//stdin
	a->fd_array[0].f_op.op_open = (void*)skip_open;
	a->fd_array[0].f_op.op_read = (void*)read_terminal;
	a->fd_array[0].f_op.op_write = (void*)skip_write;
	//stdout
	a->fd_array[1].f_op.op_open = (void*)skip_open;
	a->fd_array[1].f_op.op_read = (void*)skip_read;
	a->fd_array[1].f_op.op_write = (void*)write_terminal;
	//set all following present flags to 0
	for(i=2;i<8;i++)
		a->fd_array[i].f_flag = 0;
}

/*
  read
	Description: reads data from the keyboard, a file, device (RTC), or directory
	input: fd, dest buf, bytes to read nbytes
	output: bytes read, or -1 on failure
*/
int32_t read(int32_t fd, void* buf, int32_t nbytes)
{	
	int i;
	i = current->fd_array[fd].f_op.op_read(current->fd_array[fd].inode,current->fd_array[fd].f_pos,buf,nbytes);
	if(i<0)
		return -1;
	else
	{	
		current->fd_array[fd].f_pos += i;

		if(current->fd_array[fd].f_op.op_read==(void*)read_dir)
		{
			if(i==0)
				return 0;
			else
				return strlen(buf);
		}
		else
			return i;
	}
}		

/*
  write
	Description: writes data to the terminal or to a device
	input:  fd, dest buf, bytes to write nbytes
	output: bytes wrote, or -1 on failure
*/
int32_t write(int32_t fd, const void* buf, int32_t nbytes)
{
	return current->fd_array[fd].f_op.op_write(buf,nbytes);
}

/*
  open
	Description: provides access to the file system and initialize the device 
	input: filename to open
	output: If the named file does not exist or no descriptors are free: -1. 
		if success: 0
*/
int32_t open(const uint8_t* filename)
{
	int i;
	dentry_t dt;
	if(read_dentry_by_name(filename,&dt)==-1)
		return -1;
	
	for(i=2;i<8;i++)
	{
		if(current->fd_array[i].f_flag==0)
		{
			fill_fd(i,&dt);
			current->fd_array[i].f_op.op_open();
			return i;
		}
	}
	return -1;
}

/*
  fill_fd
	help function to initialize a fd entry
	input: file type i, dentry number
	output: none
*/
void fill_fd(int i, dentry_t* dentry)
{
	switch (dentry->filetype) {
		case 0:	// real-time clock
			current->fd_array[i].f_op.op_open = (void*)open_rtc;
			current->fd_array[i].f_op.op_read = (void*)read_rtc;
			current->fd_array[i].f_op.op_write = (void*)write_rtc;
			break;
		case 1:	// directory
			current->fd_array[i].f_op.op_open = (void*)skip_open;
			current->fd_array[i].f_op.op_read = (void*)read_dir;
			current->fd_array[i].f_op.op_write = (void*)skip_write;
			break;
		case 2:	// data file
			current->fd_array[i].f_op.op_open = (void*)skip_open;
			current->fd_array[i].f_op.op_read = (void*)read_data;
			current->fd_array[i].f_op.op_write = (void*)skip_write;
			break;
	}
	
	//set inode num, read file position =0, present flag = 1
	current->fd_array[i].inode = dentry->inode;
	current->fd_array[i].f_pos = 0;
	current->fd_array[i].f_flag = 1;
	
	return;
}

//===============================================================================
/*
  close
	Description: reads the program’s command line arguments into a user-level buffer
	input: int32_t fd
	output: Trying to close an invalid descriptor -1. if success: 0
*/
int32_t close(int32_t fd)
{

	if((fd < 2) || (fd > 7) || ((current->fd_array[fd].f_flag & 0x01) == 0))
        return -1;
	else
    {	
		//set present flag to 0
		current->fd_array[fd].f_flag = 0;
		return 0;
	}
}


/*
  getargs
	Description: reads the program’s command line 
				 arguments into a user-level buffer
	input: dest buf, bytes to get nbytes
	output: If do not fit in the buffer,  return -1. if success: 0
*/
int32_t getargs(uint8_t* buf, int32_t nbytes)
{
	int i;
	for(i=0;i<nbytes;i++)
	{	
		//traverse each char until NULL
		if(args[i]!='\0')
			buf[i] = args[i];
		else
		{	
			buf[i] = '\0';
			return 0;
		}
	}
	
	return -1;
}



/*
  vidmap
	Description: maps the text-mode video memory into 
				 user space at a pre-set virtual address
	input: uint8_t** screen_start
	output: If the location is invalid: -1. if success: 0
*/
int32_t vidmap(uint8_t** screen_start)
{
	//check whether the address falls within the address range covered by the single user-level page
	if ((int)screen_start < 0x8000000 || (int)screen_start >= 0x8400000 )
		return -1;

	*screen_start = (uint8_t*)(0xB9000 + 0x1000 * current->term);
	return 0;
}



/*
  set_handler
	Description: system call to load the user handler
	input: signum ,user handler addr
	output : 0 on success, -1 on failure
*/
int32_t set_handler(int32_t signum, void* handler)
{
	if (signum > 3 || signum < 0 || handler == NULL)
		return -1;

	if (signum == 0)
		user_div_zero_handler = handler;

	if (signum == 1)
		user_segfault_handler = handler;

	//else if (signum == 2)
		//user_interrupt_handler = handler;

	if (signum == 2)
	{
		user_alarm_handler = handler;
		alarm_sig_flag = 0;
	}
	return 0;

}

//===============================================================================

/*
  skip_open
	Description: when no action is needed for this sys call, return 0
	input: none
	output: 0
*/
int32_t skip_open()
{
	return 0;
}
/*
  skip_read
	Description: when no action is needed for this sys call, return 0
	input: uint32_t inode,uint32_t offset,void* buf,uint32_t nbytes
	output: 0
*/
int32_t skip_read(uint32_t inode,uint32_t offset,void* buf,uint32_t nbytes)
{
	return -1;
}

/*
  skip_write
	Description: when no action is needed for this sys call, return 0
	input: const void* buf, int32_t nbytes
	output: 0
*/
int32_t skip_write(const void* buf, int32_t nbytes)
{
	return -1;
}
