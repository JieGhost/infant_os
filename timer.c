#include "timer.h"

#define PITfreq 50

/*
  do_timer
	Description: timer handler, jusr call schedule function
	input: none
	output: none
*/
void do_timer(void)
{
	schedule();
}

/*
  init_timer
	Description: initialize timer, set the frequencyto 50 hz
	input: none
	output: none
*/
void init_timer(void)
{
	//PITfreq = value to divide it's input clock (1193180 Hz) by, to get our required frequency. 
   uint32_t divisor = 1193180 / PITfreq;

   //send the cmd byte to repeate mode
   outb(0x36,0x43);

   //send frequency divisor byte-wise, low then high.
   outb((uint8_t)(divisor & 0xFF),0x40);
   outb((uint8_t)( (divisor>>8) & 0xFF),0x40);
}

/*
  play_sound
	Description: initialize port 61 for PC speaker
	input: Frequency of the note to be played
	output: none
*/

void play_sound(int32_t nFrequence) 
{
 	int32_t Div;
 	int8_t tmp;
    //Set the PIT to the desired frequency
 	Div = 1193180 / nFrequence;
 	outb(0xb6, 0x43);
 	outb((int8_t) (Div), 0x42);
 	outb((int8_t) (Div >> 8), 0x42);
    //And play the sound using the PC speaker
 	tmp = inb(0x61);
	//printf("%x\n", tmp);
  	if (tmp != (tmp | 3)) 
	{
 		outb(tmp | 3, 0x61);
 	}
 }
 
 /*
  nosound
	Description: pause between continuous sounds
	input: none
	output: none
*/
void nosound()   // to pause the beep
{
 	int8_t tmp = (inb(0x61) & 0xFC);
  	outb(tmp, 0x61);
}
 
// beep function
//Description: Make a beep
//input:  nFrequence = sound frequency of a beep
//			duration = 1 represents apporiximatly 1/1000 of second
//output: none

 void beep(int32_t nFrequence, int32_t duration) 
 {
	if(sound_on == 0)
		return;

	play_sound(nFrequence);
	
	//while (alarm_sig_flag == 0);
	//alarm_sig_flag = 0;
	int i;
	 for(i=0;i<duration;i++);          //timer_wait(10);
 	 nosound();
 }
