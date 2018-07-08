/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */


/*
  i8259_init
	Description: initialize the 8259 PIC
	Input: none
	Output: none
*/
void
i8259_init(void)
{
	master_mask = 0xff;
	slave_mask = 0xff;

	outb(master_mask, MASTER_8259_PORT + 1);
	outb(slave_mask, SLAVE_8259_PORT + 1);

	/* init master interrupt controller */
	outb(ICW1, MASTER_8259_PORT); /* Start init sequence */
	outb(ICW2_MASTER, MASTER_8259_PORT + 1); /* Vector base */
	outb(ICW3_MASTER, MASTER_8259_PORT + 1); /* edge tiggered, Cascade (slave) on IRQ2 */
	outb(ICW4, MASTER_8259_PORT + 1); /* Select 8086 mode */

	/* init slave interrupt controller */
	outb(ICW1, SLAVE_8259_PORT); /* Start init sequence */
	outb(ICW2_SLAVE, SLAVE_8259_PORT + 1); /* Vector base */
	outb(ICW3_SLAVE, SLAVE_8259_PORT + 1); /* edge triggered, Cascade (slave) on IRQ2 */
	outb(ICW4, SLAVE_8259_PORT + 1); /* Select 8086 mode */

}


/*
  enable_irq
	Description: enable (unmask) the specified IRQ
	Input: irq_num: IRQ number
	Output: none
*/
void
enable_irq(uint32_t irq_num)
{	
	if(irq_num & 8)
	{
		slave_mask  &= ~(1 << (irq_num - 8));
		outb(slave_mask, SLAVE_8259_PORT + 1);
	}
	else
	{	
		master_mask &= ~(1 << irq_num);
		outb(master_mask, MASTER_8259_PORT + 1);
	}
}


/*
  disable_irq
	Description: disable (mask) the specified IRQ
	Input: irq_num: IRQ number
	Output: none
*/
void
disable_irq(uint32_t irq_num)
{
	if(irq_num & 8)
	{
		slave_mask	|= (1 << (irq_num - 8));
		outb(slave_mask, SLAVE_8259_PORT + 1);
	}
	else
	{
		master_mask	|= (1 << irq_num);
		outb(master_mask, MASTER_8259_PORT + 1);
	}
}


/*
  send_eoi
	Description: send end-of-interrupt signal for the specified IRQ
	Input: irq_num: IRQ number
	Output: none
*/
void
send_eoi(uint32_t irq_num)
{
	if(irq_num & 8)
		outb(EOI | (irq_num - 8), SLAVE_8259_PORT);
	else
		outb(EOI | irq_num, MASTER_8259_PORT);
}

