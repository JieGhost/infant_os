#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"
#include "syscall.h"
#include "filesys.h"
#include "signal.h"
#include "timer.h"
#include "videoterm.h" 

#define cmdbuff_size  300

int curr_term; //current terminal

void inikeyboard();
void do_keyboard(void);
void deal_keyboard(uint8_t scancode);
char scan_ascii(char inscan);
void clear_cmdbuff();

int32_t read_terminal(uint32_t inode,uint32_t offset,uint8_t* buf,int32_t nbytes);
int32_t write_terminal(char* buf,int32_t nbytes);

void getcmd_hist(int move);
void addto_histcmd (void);

int tab_comp(char* usercmd, int cmdlength,char** reservelist, int reservelength);
int cmpr_cmd(char* usercmd, int cmdlength, char* reserve);

void term_change(void);
void mouse_cmd(char* buf);

#endif
