#ifndef _MOUSE_H
#define _MOUSE_H

#include "types.h"

void put_cursor(void);
void clear_cursor(void);

void init_mouse(void);
void do_mouse(void);

int mouse_wait(int a);
void mouse_read(void);
void mouse_write(uint8_t c);

#endif
