#ifndef _VIDEOTERM_H
#define _VIDEOTERM_H

#include "types.h"
#include "mouse.h"
#include "terminal.h"


int scrolled_count;
int sound_on;

void move_video_up(void);
void scrolled(int a);

void save_to_save(void);
void back_to_video(void);

int check_full_screen(void);
void clear_keyb(void);
void putc_keyb(uint8_t c);
int32_t puts_keyb(int8_t *s);
void backspace_keyb(void);

void init_term(void);
void switch_term(int a,int b);

void flush_tlb(void);
void splash_screen(void);

int put_t(char* buf,int32_t nbytes,int t);
int put_exe(char* buf,int32_t nbytes);


#endif
