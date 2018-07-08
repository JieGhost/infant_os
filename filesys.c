#include "filesys.h"
#include "lib.h"

static filesys_t filesys;

/* 
  get_stat_filesys
	Description: look for filesys and save information
				 of filesys for further usage
	Input: none
	Output: int: -1 no filesys
				 0, filesys exists
*/
int get_stat_filesys()
{
	if(fs_base==NULL)
		return -1;
	
	filesys.num_entry = *((int*)fs_base);
	filesys.num_inode = *((int*)(fs_base+4));
	filesys.num_datablock = *((int*)(fs_base+8));
	
	return 0;
}

/* 
  read_dentry_by_name
	Description: take an string as input and go through filesys
				 to look for file with that name, if found, set
				 info of that file into dentry_t structure
	Input: fname: file name we want to search
		   dentry: pointer of dentry_t structure, file info will be
				   saved to it
	Output: int: -1 no file with input name
				 0, file found
*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
	int i;
	if(*fname=='\0')
		return -1;
	
	for(i=1;i<64;i++)
	{
		if(!strncmp((int8_t*)fname, (int8_t*)fs_base+64*i, 32))
		{
			strncpy(dentry->filename,(int8_t*)fname,32);
			dentry->filetype = *((int*)(fs_base+64*i+32));
			dentry->inode = *((int*)(fs_base+64*i+36));
			
			return 0;
		}
	}
	
	return -1;
}

/* 
  read_dentry_by_index
	Description: take an index as input and go through filesys
				 to look for file with that index, if found, set
				 info of that file into dentry_t structure
	Input: fname: file index we want to search
		   dentry: pointer of dentry_t structure, file info will be
				   saved to it
	Output: int: -1 no file with input index
				 0, file found
*/
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
	int i;
	for(i=1;i<64;i++)
	{
		if(i-1==index)
		{
			strncpy(dentry->filename,fs_base+64*i,32);
			dentry->filetype = *((int*)(fs_base+64*i+32));
			dentry->inode = *((int*)(fs_base+64*i+36));
			
			return 0;
		}
	}
	
	return -1;
}

/*
  read dir
	Description: only the filename should be provided (as much as fits, or all 32 bytes), 
				 and subsequent reads should read from successive directory entries until 
				 the last is reached, at which point read should repeatedly return 0.
	input: inode,offset: for format, not used here
		   buf: dest buffer
		   nbytes: num bytes to copy
	output: 0: no more entries to read
			1: one entry read
*/
int32_t read_dir (uint32_t inode,uint32_t offset, uint8_t* buf, int32_t nbytes)
{
	int i;
    //finished reading all dir entries
    if (filesys.num_entry <= offset)
        return 0;

	if(nbytes<32)
		i = nbytes;
	else
		i = 32;
		
    strncpy((int8_t*)buf,(int8_t*)fs_base+64*(offset+1),32);
	return 1;
    
}


/*
  read_data
	Description: read the file of inode, starting at position offset,
				 and save to destination buf
	Input: inode: denote file to read from
		   offset: start position in the file
		   buf: destination buffer
		   length: number of bytes to read
	Output: int: -1 on failure
			else number of bytes read
*/
int32_t read_data(uint32_t inode,uint32_t offset,uint8_t* buf,uint32_t length)
{
	char* inode_base;
	int num_entry,num_inode,num_datablock;
	int file_length;
	char* filedata_pt;
	int i,db_cnt,db_index;
	
	num_entry = filesys.num_entry;
	num_inode = filesys.num_inode;
	num_datablock = filesys.num_datablock;
	//check if parameters are valid
	inode_base = fs_base + (inode+1)*(4096);
	file_length = *((int*)(inode_base));
	
	if(inode >= num_inode)
		return -1;
	
	db_cnt = offset / 4096 + 1;	//datablock count
	db_index = (*(int*)(inode_base+4*db_cnt));	//get index of first datablock
	
	if(db_index>=num_datablock)
		return -1;
		
	filedata_pt = fs_base+(1+num_inode+db_index)*(4096);
	for(i = offset;i<offset+length && i<file_length;i++)
	{
		if(i % 0x1000 ==0 && i!=offset)	//check if finish reading one datablock
		{	
			db_cnt ++;
			db_index = (*(int*)(inode_base+4*db_cnt));
			
			if(db_index>=num_datablock)
				return -1;
				
			filedata_pt = fs_base+(1+num_inode+db_index)*(4096);
		}
		buf[i-offset] = *(filedata_pt+i%0x1000);
	}

	return i - offset;
}

/*
  check_executable
	Description: read the file of inode, starting at position offset,
				 and save to destination buf
	Input: inode: denote inode of file to be checked
	Output: 0 if not executable
			else entry point is returned
*/
uint32_t check_executable(uint32_t inode)
{
	char* inode_base;
	int num_entry,num_inode,num_datablock;
	char* filedata_pt;
	int db_index;
	
	//get file info from boot block
	num_entry = filesys.num_entry;
	num_inode = filesys.num_inode;
	num_datablock = filesys.num_datablock;
	
	//get index node addr
	inode_base = fs_base + (inode+1)*(4096);
	
	//check if inode valid
	if(inode >= num_inode)
		return 0;
	
	//get 0th data block index
	db_index = (*(int*)(inode_base+4));
	//get 0th data block addr
	filedata_pt = fs_base+(1+num_inode+db_index)*(4096);
	//check if file is executable via megic num
	if(*(uint32_t*)(filedata_pt) == 0x464C457F)
		return *(uint32_t*)(filedata_pt+24);	//return entry point
	//not executable 
	return 0;

}

/*
  copy_executable
	Description: copy content of an executable file
				 to memory location starting at dest
				 need call check_executable first
	Input: inode: inode num of file to be copied
		   dest: destination memory location
	Output: 0 if success
			-1 if failure
*/
int32_t copy_executable(uint32_t inode,uint8_t* dest)
{
	char* inode_base;
	int num_entry,num_inode,num_datablock;
	int file_length;
	char* filedata_pt;
	int i,db_cnt,db_index;
	
	i = 0;
	db_cnt = 1;
	
	num_entry = filesys.num_entry;
	num_inode = filesys.num_inode;
	num_datablock = filesys.num_datablock;
	
	inode_base = fs_base + (inode+1)*(4096); //get inode location
	file_length = *((int*)(inode_base));	//get file length
	
	if(inode >= num_inode)
		return -1;
	
	db_index = (*(int*)(inode_base+4));

	filedata_pt = fs_base+(1+num_inode+db_index)*(4096);
	//copy datablock by datablock
	//end when file_length exceded
	while(i<file_length)
	{	
		db_index = (*(int*)(inode_base+4*db_cnt));
			
		if(db_index>=num_datablock)
			return -1;
				
		filedata_pt = fs_base+(1+num_inode+db_index)*(4096);
			
		memcpy(dest,filedata_pt,4096);
		db_cnt++;
		dest+=4096;
		i+=4096;
	}
	
	return 0;
}
