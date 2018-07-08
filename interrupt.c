#include "i8259.h"
#include "interrupt.h"
#include "lib.h"
#include "terminal.h"
#include "rtc.h"
#include "timer.h"
#include "mouse.h"


/*
  do_IRQ 
	Description: common interrupt handler
				 it first disables the interrupt of
				 corresponding irq_num and sends eoi,
				 then calls specific handler and
				 finally enable the interrupt again
	Input: regs: pointer of the structure of current stack
	Output: int: 0,always
*/
__attribute__((regparm(3))) unsigned int do_IRQ (struct pt_regs *regs)
{
	uint32_t irq = regs ->IRQ_NUM; //get irq number

	if(irq & 8)	//slave PIC
	{
		disable_irq(irq);	
		send_eoi(2);
		send_eoi(irq);
	}
	else //master PIC
	{
		disable_irq(irq);
		send_eoi(irq);
	}
	
	if(irq==0)
		do_timer();
	else if(irq==1)
		do_keyboard();
	else if(irq==8)
		do_rtc();
	else if(irq==12)
		do_mouse();
	
	enable_irq(irq); //re-enable interrupt

	return 0;

}
