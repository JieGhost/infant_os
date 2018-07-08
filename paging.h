#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"

#define page_dir_size		1024
#define page_table_size		1024
#define num_process			8
#define space_dir_task		num_process * page_dir_size
#define space_tab_task		num_process * page_table_size

#ifndef ASM

//page directory entry
struct page_directory{
	uint32_t present : 1;
	uint32_t read_write : 1;
	uint32_t user_supervisor : 1;
	uint32_t write_through : 1;
	uint32_t cache_disabled : 1;
	uint32_t accessed : 1;
	uint32_t reserved : 1;
	uint32_t page_size : 1;
	uint32_t global_page : 1;
	uint32_t avail : 3;
	uint32_t page_table_base : 20;
}__attribute__((packed));
typedef struct page_directory page_dir_entry;

//page table entry
struct page_table{
	uint32_t present : 1;
	uint32_t read_write : 1;
	uint32_t user_supervisor : 1;
	uint32_t write_through : 1;
	uint32_t cache_disabled : 1;
	uint32_t accessed : 1;
	uint32_t dirty : 1;
	uint32_t pat : 1;
	uint32_t global_page : 1;
	uint32_t avail : 3;
	uint32_t page_base : 20;
}__attribute__((packed)); 
typedef struct page_table page_table_entry;

//declared in page.S
extern page_dir_entry page_dir[page_dir_size];
extern page_table_entry page_tab[page_table_size];
extern page_dir_entry page_dir_task[num_process][page_dir_size];
extern page_table_entry page_tab_task[num_process][page_table_size];

void init_paging(void);

#endif


#endif
