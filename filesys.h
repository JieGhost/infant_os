#ifndef _FILESYS_H
#define _FILESYS_H

#include "types.h"

//structure contains file system status
struct filesys_st
{
    int num_entry;
    int num_inode;
    int num_datablock;
};
typedef struct filesys_st filesys_t;

//structure contains info of directory entry
struct dir_entry
{
    char filename[32];
    uint32_t filetype;
    uint32_t inode;
	uint8_t reserved[24];
};
typedef struct dir_entry dentry_t;

//base address of file system
char* fs_base; 

int get_stat_filesys(void);

int32_t read_dir(uint32_t inode,uint32_t offset, uint8_t* buf, int32_t nbytes);

int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode,uint32_t offset,uint8_t* buf,uint32_t length);

uint32_t check_executable(uint32_t inode);
int32_t copy_executable(uint32_t inode,uint8_t* dest);


#endif
