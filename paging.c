#include "paging.h"

/* 
  initialize paging
	Description: set PDE PTE values of kernel paging and
				 all possible processes' paging
	Input: none
	Output: none
*/
void init_paging(void)
{
	int i,j,n;
	
	//initialize all the page directories and
	//page tables entries
	for(i = 0;i < page_dir_size;i++)
	{
		page_dir[i].present = 0;
		page_dir[i].read_write = 0;
		page_dir[i].user_supervisor = 0;
		page_dir[i].write_through = 0;
		page_dir[i].cache_disabled = 0;
		page_dir[i].accessed = 0;
		page_dir[i].reserved = 0;
		page_dir[i].page_size = 0;
		page_dir[i].global_page = 0;
		page_dir[i].avail = 0;
		page_dir[i].page_table_base = 0;
	}
	
	for(i = 0;i < page_table_size;i++)
	{
		page_tab[i].present = 0;
		page_tab[i].read_write = 0;
		page_tab[i].user_supervisor = 0;
		page_tab[i].write_through = 0;
		page_tab[i].cache_disabled = 0;
		page_tab[i].accessed = 0;
		page_tab[i].dirty = 0;
		page_tab[i].pat = 0;
		page_tab[i].global_page = 0;
		page_tab[i].avail = 0;
		page_tab[i].page_base = 0;
	}
	
	for(i=0;i<num_process;i++)
	{
		for(j=0;j<page_dir_size;j++)
		{
			page_dir_task[i][j].present = 0;
			page_dir_task[i][j].read_write = 0;
			page_dir_task[i][j].user_supervisor = 0;
			page_dir_task[i][j].write_through = 0;
			page_dir_task[i][j].cache_disabled = 0;
			page_dir_task[i][j].accessed = 0;
			page_dir_task[i][j].reserved = 0;
			page_dir_task[i][j].page_size = 0;
			page_dir_task[i][j].global_page = 0;
			page_dir_task[i][j].avail = 0;
			page_dir_task[i][j].page_table_base = 0;
		}
	}
	
	for(i=0;i<num_process;i++)
	{
		for(j=0;j<page_table_size;j++)
		{
			page_tab_task[i][j].present = 0;
			page_tab_task[i][j].read_write = 0;
			page_tab_task[i][j].user_supervisor = 0;
			page_tab_task[i][j].write_through = 0;
			page_tab_task[i][j].cache_disabled = 0;
			page_tab_task[i][j].accessed = 0;
			page_tab_task[i][j].dirty = 0;
			page_tab_task[i][j].pat = 0;
			page_tab_task[i][j].global_page = 0;
			page_tab_task[i][j].avail = 0;
			page_tab_task[i][j].page_base = 0;
		}
	}
	
	page_dir[0].present = 1;
	//initialize the firset page table in PDE[0]
	page_dir[0].page_table_base = ((uint32_t)page_tab >> 12) & 0x000FFFFF;
	
	//map 1st 4MB to first 4MB in physical mem space
	for(i = 0;i < page_table_size;i++)
		page_tab[i].page_base = i & 0x000FFFFF;
	
	//video mem present
	page_tab[0xB8].present = 1;
	
	page_tab[0xB9].present = 1; //first terminal
	page_tab[0xBA].present = 1; //second terminal
	page_tab[0xBB].present = 1; //third terminal
	
	//kernel page
	page_dir[1].present = 1;
	page_dir[1].page_size = 1;
	page_dir[1].global_page = 1;
	page_dir[1].page_table_base = 1 << 10;

///////////////////////////////////////////////////////////////////////////

	for(n=0;n<num_process;n++)
	{
		//first page table present
		page_dir_task[n][0].present = 1;
		page_dir_task[n][0].user_supervisor = 1;
		page_dir_task[n][0].read_write = 1;
		//initialize the firset page table
		page_dir_task[n][0].page_table_base = ((uint32_t)page_tab_task[n] >> 12) & 0x000FFFFF;
		
		//map 1st 4MB to first 4MB in physical mem space
		for(i = 0;i < page_table_size;i++)
			page_tab_task[n][i].page_base = i & 0x000FFFFF;
	
		//video mem present
		page_tab_task[n][0xB8].present = 1;
		page_tab_task[n][0xB8].user_supervisor = 1;
		page_tab_task[n][0xB8].read_write = 1;
		
		//terminal pages present
		page_tab_task[n][0xB9].present = 1;
		page_tab_task[n][0xB9].user_supervisor = 1;
		page_tab_task[n][0xB9].read_write = 1;
		
		page_tab_task[n][0xBA].present = 1;
		page_tab_task[n][0xBA].user_supervisor = 1;
		page_tab_task[n][0xBA].read_write = 1;
		
		page_tab_task[n][0xBB].present = 1;
		page_tab_task[n][0xBB].user_supervisor = 1;
		page_tab_task[n][0xBB].read_write = 1;
		
		/////////////////////////////////////////////////
		//terminal buffers, used for scroll up adn down
		page_tab_task[n][0xBC].present = 1;
		page_tab_task[n][0xBC].user_supervisor = 1;
		page_tab_task[n][0xBC].read_write = 1;
		
		page_tab_task[n][0xBD].present = 1;
		page_tab_task[n][0xBD].user_supervisor = 1;
		page_tab_task[n][0xBD].read_write = 1;
		
		page_tab_task[n][0xBE].present = 1;
		page_tab_task[n][0xBE].user_supervisor = 1;
		page_tab_task[n][0xBE].read_write = 1;
		////////////////////////////////////////////////////
		page_tab_task[n][0xBF].present = 1;
		page_tab_task[n][0xBF].user_supervisor = 1;
		page_tab_task[n][0xBF].read_write = 1;

		////////////////////////////////////////////////////
		//kernel page
		page_dir_task[n][1].present = 1;
		page_dir_task[n][1].page_size = 1;
		page_dir_task[n][1].global_page = 1;
		page_dir_task[n][1].page_table_base = 1 << 10;
		
		//user program page, virtual 128MB to physical 8MB, 16MB, ...
		page_dir_task[n][0x20].present = 1;
		page_dir_task[n][0x20].page_size = 1;
		page_dir_task[n][0x20].read_write = 1;
		page_dir_task[n][0x20].user_supervisor = 1;
		page_dir_task[n][0x20].page_table_base = ((n+2) << 10);
	}
	
}
