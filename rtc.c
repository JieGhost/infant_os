#include "rtc.h"

#define MAX_FREQ  32768  // max rtc fequency

static volatile int rtc_count;
static int rtc_10_sec;

/* 
  init_rtc
	Description: initialize rtc, set frequency
	Input: none
	Output: none
*/
void init_rtc(void)
{
	outb(0x0A,0x70);
	outb(0xAF,0x71); // initialize to 2Hz
	outb(0x0B,0x70);
	uint8_t prev_B = inb(0x71);
	//set bit 6, open periodic int
	outb(0x0B,0x70);
	outb(0x40|prev_B,0x71);

	rtc_count = 0;			// clear rtc_count
	rtc_10_sec = 0;
	alarm_sig_flag = 0;		// reset alarm_sig_flag
}


/* 
  do_rtc
	Description: rtc interrupt handler
	Input: none
	Output: none
*/
void do_rtc(void)
{
	outb(0x0C,0x70);
	inb(0x71);

	rtc_count ++;		// every time rtc int occurs, increment count
	outb(0x0A,0x70);
	// frequency = 32768 / (2^freq_div_power)
	int freq_div_power = inb(0x71) & 0x0F;	// get freq divident 
	int freq_div = 1;
	int i;
	// get frequency
	for(i=0; i!=freq_div_power-1; i++)
		freq_div *= 2;						// 

	// add freq to count 10 s
	rtc_10_sec += freq_div;
	if(rtc_10_sec >= 327680) 
	{
		rtc_10_sec = 0;
		alarm_sig_flag = 1;
		return;
	}
	
	return;
}


/* 
  op_open_rtc
	Description: set RTC status reg, initialize frequency,
				 receive intr
	Input: none
	Output: none
*/
void op_open_rtc(void)
{
	cli();

	outb(0x0A,0x70);
	outb(0xAF,0x71); // initialize to 2kHz

	sti();
}

/* 
  op_read_rtc
	Descriptiong: when rtc flag == 1, 
	Input: none
	Output: return 0
*/
int op_read_rtc()
{
	//flag_rtc = 1;
	int prev_count = rtc_count;
	while(rtc_count == prev_count);

	return 0;
}


/* 
  op_write_rtc
	Description: when rtc flag == 1, print ... 
	Input: frequency to set
	Output: none
*/
void op_write_rtc(uint32_t freq)
{
	//convert the frequency para to a binary rate
	uint16_t rtc_div = MAX_FREQ / freq ;
	uint8_t rate = 0;
	while(rtc_div!=0)
	{
		rtc_div /= 2;
		rate ++ ;
	}

	rate = rate & 0x0f;

	cli();
	// select reg A
	outb(0x0A,0x70);
	uint8_t prev_A = inb(0x71);
	outb(0x0A,0x70);
	// write to regA to change the freq divident
	outb((prev_A & 0xF0)|rate, 0x71); // set the frequency
	sti();
}


/* 
  open_rtc
	Description: call op_open_rtc to open rtc
	Input: none
	Output: 0 
*/
int32_t open_rtc(void)
{
    op_open_rtc();
    return 0;
}



/* 
  read_rtc
	Description: call op_read_rtc to open rtc
	Input: uint32_t inode,uint32_t offset, void* buf, int32_t nbytes
	Output: 0 
*/
int32_t read_rtc(uint32_t inode,uint32_t offset, void* buf, int32_t nbytes)
{
    op_read_rtc();
    return 0;
}


/* 
  write_rtc
	Description: check if the new freq is valid. if Valid, call op_write_rtc
	Input:  void*, int32_t
	Output: 4 on success, -1 on failure  
*/
int32_t write_rtc(void* buf, int32_t nbytes)
{
    int freq = *(int*) buf;				// get the new frequency
    if(freq > 1024 || freq <=0)			// check if within valid range
        return -1;
    if ( (freq & (freq-1)) != 0 )
        return -1;

    op_write_rtc(freq);
        return 4;						// return # of bytes write
}


