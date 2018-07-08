#ifndef _RTC_H
#define _RTC_H

#include "lib.h"
#include "types.h"
#include "videoterm.h"
#include "signal.h"


void init_rtc(void);

void do_rtc(void);
void op_open_rtc(void);
int op_read_rtc(void);
void op_write_rtc(uint32_t freq);

int32_t open_rtc(void);
int32_t read_rtc(uint32_t inode,uint32_t offset, void* buf, int32_t nbytes);
int32_t write_rtc(void* buf, int32_t nbytes);


#endif
