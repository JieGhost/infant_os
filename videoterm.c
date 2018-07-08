#include "videoterm.h"
#include "paging.h"
#include "lib.h"
#include "timer.h"


#define NUM_COLS_K 80
#define NUM_ROWS_K 24
#define ATTRIB_K 0x7

#define VIDEO_0	0xB9000
#define VIDEO_1	0xBA000
#define VIDEO_2	0xBB000

#define VBACK_0 0xBC000
#define VBACK_1 0xBD000
#define VBACK_2 0xBE000

#define VSAVE_0 0xBF000
#define VSAVE_1 0xBF000
#define VSAVE_2 0xBF000

static int screen_x_k;
static int screen_y_k;
static int s_x[3] = {0,0,0};
static int s_y[3] = {0,0,0};
static char* video_mem_k;
static char* videomem[3] = {(char*)VIDEO_0,(char*)VIDEO_1,(char*)VIDEO_2};

static char* video_back[3] = {(char*)VBACK_0,(char*)VBACK_1,(char*)VBACK_2};
static char* video_save[3] = {(char*)VSAVE_0,(char*)VSAVE_1,(char*)VSAVE_2};
static int back_line[3] = {0,0,0};

static uint8_t attr_term;
static uint8_t term_attrib[3] = {0x3,0x7,0xA};

/* 
  check_full_screen
	Description: check if screen is full of lines, 
				 used for scrolling
	Input: none
	Output: integer: 0/full screen, not 0/not full
*/
int check_full_screen(void)
{
	return NUM_ROWS_K - 1 - screen_y_k;
}

/* 
  save_to_save
	Description: copy current screen to backup memory
				 used for scrolling
	Input: none
	Output: none
*/
void save_to_save(void)
{
	clear_cursor(); //keeping mouse on the screen
	memcpy((uint8_t*)video_save[curr_term],(uint8_t*)video_mem_k,4096);
	put_cursor();
}

/* 
  back_to_video
	Description: restore the screen with content before
				 scroll happens
	Input: none
	Output: none
*/
void back_to_video(void)
{
	clear_cursor();
	memcpy((uint8_t*)video_mem_k,(uint8_t*)video_save[curr_term],4096);
	put_cursor();
}

/* 
  scrolled
	Description: called when shift+pageup/down pressed
				 implement back scroll
	Input: int a:indicate scroll direction(1/up,0/downn)
	Output: none
*/
void scrolled(int a)
{
	int i,j;
	
	clear_cursor();
	
	j = (NUM_ROWS_K-1)*NUM_COLS_K;
	
	//check scroll direction
	if(a==1)  //scrolled_up
	{	//if scroll limit already reached, just return, same for a=0
		if(scrolled_count==NUM_ROWS_K || scrolled_count==back_line[curr_term])
		{
			put_cursor();
			return;
		}
		//update scroll count	
		scrolled_count ++;
	}
	else if(a==0)  //scrolled_down
	{
		if(scrolled_count==0)
		{
			put_cursor();
			return;
		}
			
		scrolled_count --;
	}
	else
	{
		put_cursor();
		return;
	}

	//scroll count=0 means no scroll, just restore the screen
	if(scrolled_count==0)
	{
		memcpy((uint8_t*)video_mem_k,(uint8_t*)video_save[curr_term],4096);
		put_cursor();
		return;
	}
	
	//if scroll up,move current screen down by one line and copy one line from screen buffer
	//if scroll down,move current screen up by one line,and copy one line from second screen buffer
	if(a==1)		
	{	
		for(i=NUM_ROWS_K*NUM_COLS_K-1;i>=NUM_COLS_K;i--)
		{
			*(uint8_t *)(video_mem_k + (i << 1)) = *(uint8_t *)(video_mem_k + ((i-NUM_COLS_K) << 1));
			*(uint8_t *)(video_mem_k + (i << 1) + 1) = *(uint8_t *)(video_mem_k + ((i-NUM_COLS_K) << 1)+1);
		}
		for(i=0;i<NUM_COLS_K;i++)
		{
			*(uint8_t *)(video_mem_k + (i << 1)) = 
			*(uint8_t *)(video_back[curr_term] + 
						 ((i + NUM_COLS_K * ((back_line[curr_term] - scrolled_count) % NUM_ROWS_K)) << 1));
			*(uint8_t *)(video_mem_k + (i << 1) + 1) = 
			*(uint8_t *)(video_back[curr_term] + 
						 ((i + NUM_COLS_K * ((back_line[curr_term] - scrolled_count) % NUM_ROWS_K)) << 1) + 1);
		}
	}
	else if(a==0)
	{
		for(i=0;i<(NUM_ROWS_K-1)*NUM_COLS_K;i++)
		{
			*(uint8_t *)(video_mem_k + (i << 1)) = *(uint8_t *)(video_mem_k + ((i+NUM_COLS_K) << 1));
			*(uint8_t *)(video_mem_k + (i << 1) + 1) = *(uint8_t *)(video_mem_k + ((i+NUM_COLS_K) << 1)+1);
		}
		for(i=0;i<NUM_COLS_K;i++)
		{
			*(uint8_t *)(video_mem_k + ((i+j) << 1)) = 
			*(uint8_t *)(video_save[curr_term] + 
						 ((i + NUM_COLS_K * ((NUM_ROWS_K - 1 - scrolled_count) % NUM_ROWS_K)) << 1));
			*(uint8_t *)(video_mem_k + ((i+j) << 1) + 1) = 
			*(uint8_t *)(video_save[curr_term] + 
						 ((i + NUM_COLS_K * ((NUM_ROWS_K - 1 - scrolled_count) % NUM_ROWS_K)) << 1) + 1);
		}
	}
	else
	{
		put_cursor();
		return;
	}
	
	put_cursor();
}

/* 
  put_exe
	Description: helper function used for displaying executable file
				 on the screen(different color)
	Input: buf: cource string buffer,
		   nbytes: length of string
	Output: int:bytes written
*/
int put_exe(char* buf,int32_t nbytes)
{
	int i;
	uint8_t temp;
	temp = attr_term;
	attr_term = 0x53;
	for(i=0;i<nbytes;i++)
		putc_keyb(buf[i]);
	//restore current attribute	
	attr_term = temp;
	
	return i;
}

/* 
  put_t
	Description: helper function used for writing to currently non
				 active terminal
	Input: buf: cource string buffer,
		   nbytes: length of string
		   t: terminal index
	Output: int:bytes written
*/
int put_t(char* buf,int32_t nbytes,int t)
{
	int i;
	int temp_x,temp_y;
	char* temp_vidmem;
	uint8_t temp_attr;
	
	//save current screen info
	temp_x = screen_x_k;
	temp_y = screen_y_k;
	temp_vidmem = video_mem_k;
	temp_attr = attr_term;
	//load with dest terminal screen info
	screen_x_k = s_x[t];
	screen_y_k = s_y[t];
	video_mem_k = videomem[t];
	attr_term = term_attrib[t];
	
	for(i=0;i<nbytes;i++)
		putc_keyb(buf[i]);
	//update dest screen info	
	s_x[t] = screen_x_k;
	s_y[t] = screen_y_k;
	//restore current screen info
	screen_x_k = temp_x;
	screen_y_k = temp_y;
	video_mem_k = temp_vidmem;
	attr_term = temp_attr;
	
	return i;
}

/* 
  flush_tlb
	Description: flush tlb after page table changed
	Input: none
	Output: none
*/
void flush_tlb(void)
{
	asm volatile("                  \n\
			movl    %%CR3,%%ecx		\n\
			movl	%%ecx,%%CR3		\n\
			"
			:
			:
			: "ecx"
			);
}

/* 
  init_term
	Description: initialize terminal,and set page table so virtual address
				 of 1st terminal mapped to video memory
	Input: none
	Output: none
*/
void init_term(void)
{
	int i,j;
	char* c = "     Terminal ";
	page_tab[0xB9].page_base = 0xB8 & 0x000FFFFF;
	
	for(i=0;i<num_process;i++)
		page_tab_task[i][0xB9].page_base = 0xB8 & 0x000FFFFF;
	//virtual address of 1st terminal	
	video_mem_k = videomem[0];
	//color of first terminal
	attr_term = term_attrib[0];
	
	scrolled_count = 0;
	//print terminal number in status bar at the bottom of the screen
	for(j=0;j<3;j++)
	{
		for(i=0;i<14;i++)
		{
			*(uint8_t*)((VIDEO_0 + 0x1000 * j) + ((i + NUM_COLS_K * NUM_ROWS_K) << 1)) = c[i];
			*(uint8_t*)((VIDEO_0 + 0x1000 * j) + ((i + NUM_COLS_K * NUM_ROWS_K) << 1) + 1) = 0x62;
		}
		
		*(uint8_t*)((VIDEO_0 + 0x1000 * j) + ((i + NUM_COLS_K * NUM_ROWS_K) << 1)) = (0x31 + j);
		*(uint8_t*)((VIDEO_0 + 0x1000 * j) + ((i + NUM_COLS_K * NUM_ROWS_K) << 1) + 1) = 0x62;
		i++;
		
		while(i < NUM_COLS_K)
		{
			*(uint8_t*)((VIDEO_0 + 0x1000 * j) + ((i + NUM_COLS_K * NUM_ROWS_K) << 1)) = ' ';
			*(uint8_t*)((VIDEO_0 + 0x1000 * j) + ((i + NUM_COLS_K * NUM_ROWS_K) << 1) + 1) = 0x62;
			i++;
		}
	}
}

/* 
  switch_term
	Description: implement the switch of different terminals
				 for each switch, first set virtual address of
				 orig term mapped to its own physical memory,
				 then copy the current screen content to its physical
				 memory, then copy the content of dest term's own
				 physical memory to video meory and finally set virtual
				 address of dest term mapped to video memory
	Input: a: dest terminal
		   b: original terminal
	Output: none
*/
void switch_term(int a,int b)	//from b to a
{
	int i;

	clear_cursor();
	//save screen info of old term
	s_x[b] = screen_x_k;
	s_y[b] = screen_y_k;
	//load screen info of new term
	screen_x_k = s_x[a];
	screen_y_k = s_y[a];
	video_mem_k = videomem[a];
	
	attr_term = term_attrib[a];

	page_tab[0xB9+b].page_base = (0xB9+b) & 0x000FFFFF;	
	for(i=0;i<num_process;i++)
		page_tab_task[i][0xB9+b].page_base = (0xB9+b) & 0x000FFFFF;
	flush_tlb();

	//copy from vid mem to orig term mem
	memcpy((uint8_t*)(0xB9000+0x1000*b),(uint8_t*)0xB8000,4096);
	//copy from dest term mem to vid mem
	memcpy((uint8_t*)0xB8000,(uint8_t*)(0xB9000+0x1000*a),4096);
	
	page_tab[0xB9+a].page_base = 0xB8 & 0x000FFFFF;	
	for(i=0;i<num_process;i++)
		page_tab_task[i][0xB9+a].page_base = 0xB8 & 0x000FFFFF;
	flush_tlb();
	
	//////////////////////
	put_cursor();
}


/* 
  clear_keyb
	Description: clear screen and reset the screen coordinates
	Input: none
	Output: none
*/
void
clear_keyb(void)
{
    int32_t i;	
	clear_cursor();
	
    for(i=0; i<NUM_ROWS_K*NUM_COLS_K; i++) {
        *(uint8_t *)(video_mem_k + (i << 1)) = ' ';
        *(uint8_t *)(video_mem_k + (i << 1) + 1) = attr_term;//ATTRIB_K;
    }
	
	screen_x_k = 0;
	screen_y_k = 0;

	put_cursor();
	
}

/* 
  puts_keyb
	Description: print the string to the terminal
	Input: string
	Output: none
*/
int32_t
puts_keyb(int8_t* s)
{
	register int32_t index = 0;
	while(s[index] != '\0') {
		putc_keyb(s[index]);
		index++;
	}

	return index;
}

/* 
  putc_keyb
	Description: the char to the terminal
	Input: char
	Output: none
*/
void
putc_keyb(uint8_t c)
{
	clear_cursor();

	if(screen_y_k >= NUM_ROWS_K)
	{	
		move_video_up();
		screen_y_k = NUM_ROWS_K - 1;
	}

	//regular char
    if(c == '\n' || c == '\r') {
        screen_y_k++;
        screen_x_k=0;

	//new line
    } 
	else 
	{
        *(uint8_t *)(video_mem_k + ((NUM_COLS_K*screen_y_k + screen_x_k) << 1)) = c;
        *(uint8_t *)(video_mem_k + ((NUM_COLS_K*screen_y_k + screen_x_k) << 1) + 1) = attr_term;//ATTRIB_K;
        screen_x_k++;

        screen_y_k = (screen_y_k + (screen_x_k / NUM_COLS_K)); //% NUM_ROWS_K;
		screen_x_k %= NUM_COLS_K;
    }
	
	put_cursor();
}

/* 
  move_video_up
	Description: called when screen is full of lines
				 move screen up by one line and 
				 save the top line to terminal buffer
	Input: none
	Output: none
*/
void move_video_up()
{
	int i,j;
	j = (NUM_ROWS_K-1)*NUM_COLS_K;
	//save top line
	for(i=0;i<NUM_COLS_K;i++)
	{
		*(uint8_t *)(video_back[curr_term] + ((i + NUM_COLS_K * (back_line[curr_term] % NUM_ROWS_K)) << 1)) 
		= *(uint8_t *)(video_mem_k + (i << 1));
        *(uint8_t *)(video_back[curr_term] + ((i + NUM_COLS_K * (back_line[curr_term] % NUM_ROWS_K)) << 1) + 1) 
		= *(uint8_t *)(video_mem_k + (i << 1) + 1);
	}
	back_line[curr_term] ++;
	//move up by one linw
	for(i=0;i<(NUM_ROWS_K-1)*NUM_COLS_K;i++)
	{
		*(uint8_t *)(video_mem_k + (i << 1)) = *(uint8_t *)(video_mem_k + ((i+NUM_COLS_K) << 1));
        *(uint8_t *)(video_mem_k + (i << 1) + 1) = *(uint8_t *)(video_mem_k + ((i+NUM_COLS_K) << 1)+1);
	}
	//set new bottom line
	for(i=0;i<NUM_COLS_K;i++)
	{
		*(uint8_t *)(video_mem_k + ((i+j) << 1)) = ' ';
        *(uint8_t *)(video_mem_k + ((i+j) << 1) + 1) = term_attrib[curr_term];//ATTRIB_K;
	}
}


/////////////////////////////////////////////

/* 
  backspace_keyb
	Description: back space: clear one char on terminal 
	Input: none
	Output: none
*/
void backspace_keyb()
{
	//different lines
	if(screen_x_k == 0)
	{
		screen_y_k --;
		screen_x_k = NUM_COLS_K - 1;
		putc_keyb(' ');
		screen_y_k --;
		screen_x_k = NUM_COLS_K - 1;
	}
	//same line
	else
	{
		screen_x_k --;
		putc_keyb(' ');
		screen_x_k --;
	}
}


void
putc_keyb_color(uint8_t c, int color)
{
    clear_cursor();

    if(screen_y_k >= NUM_ROWS_K)
    {   
        move_video_up();
        screen_y_k = NUM_ROWS_K - 1;
    }

    //regular char
    if(c == '\n' || c == '\r') {
        screen_y_k++;
        screen_x_k=0;

    //new line
    }
    else
    {
        *(uint8_t *)(video_mem_k + ((NUM_COLS_K*screen_y_k + screen_x_k) << 1)) = c;
        *(uint8_t *)(video_mem_k + ((NUM_COLS_K*screen_y_k + screen_x_k) << 1) + 1) = color;//ATTRIB_K;
        screen_x_k++;

        screen_y_k = (screen_y_k + (screen_x_k / NUM_COLS_K)); //% NUM_ROWS_K;
        screen_x_k %= NUM_COLS_K;
    }
   
    put_cursor();
}

int32_t
puts_keyb_color(int8_t* s, int color)
{
    register int32_t index = 0;
    while(s[index] != '\0') {
        putc_keyb_color(s[index], color);
        index++;
    }

    return index;
}

void print_splash(int color)
{
    clear_keyb();
    puts_keyb_color(".........D..DDDDDDDDDDDDDDD..D..........\n",color);
    puts_keyb_color("......OD.DDDDDDDDDDDDDDDDDDDDD.DO.......\n",color);
    puts_keyb_color(".....D.DDDDDDDDDDDDDDDDDDDDDDDDD.D......\n",color);
    puts_keyb_color("...,..DDDDDDDDDDDDDDDDDDDDDDDDDDD..,....\n",color);
    puts_keyb_color("..D.DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD.D...\n",color);
    puts_keyb_color("...ODDDDDDDDDD...............DDDDDDO....\n",color);
    puts_keyb_color(".D.DDDDDDDDD.................DDDDDDD.D..\n",color);
    puts_keyb_color("D.DDDDDDDDD..DD..=DDDD..DDDDDDDDDDDDD.D.\n",color);
    puts_keyb_color("D.DDDDDDDD.DDDD..DDDDD..DDDDDDDDDDDDD.D.\n",color);
    puts_keyb_color(".DDDDDDDDDDDDDD..DDDDD..DDDDDDDDDDDDDD..\n",color);
    puts_keyb_color(".DDDDDDDDDDDDDD..DDDDD..DDDDDDDDDDDDDD.Z\n",color);
    puts_keyb_color(".DDDDDDDDDDDDDD..DDDDN..DDDDDDDDDDDDDD.Z\n",color);
    puts_keyb_color(".DDDDDDDDDDDDDO..DDDD7..DDDDDDDDDDDDDD..\n",color);
    puts_keyb_color("D.DDDDDDDDDDDD...DDDDO..DDDDDDDDDDDDD.D.\n",color);
    puts_keyb_color("D.DDDDDDDDDDD...DDDDDD..DDDD.DDDDDDDD.D.\n",color);
    puts_keyb_color(".D.DDDDDDDDD....DDDDDD......DDDDDDDD.D..\n",color);
    puts_keyb_color("...ODDDDDDDD...DDDDDDDD....DDDDDDDDO....\n",color);
    puts_keyb_color("..D.DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD.D...\n",color);
    puts_keyb_color("..D.DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD.D...\n",color);
    puts_keyb_color("...,..DDDDDDDDDDDDDDDDDDDDDDDDDDD..,....\n",color);
    puts_keyb_color("......OD  TEAM PI   LINUX OS  D.DO......\n",color);
    puts_keyb_color(".....   PRESS ENTER TO CONTINUE   ......\n",color);
    puts_keyb_color("........................................\n",color);
    return;
}


/*
  splash_screen
    Description: set the starting scene of our operating system
    Input: none
    Output: none
*/
void splash_screen(void)
{
    puts_keyb(".........D..DDDDDDDDDDDDDDD..D..........\n");
    puts_keyb("......OD.DDDDDDDDDDDDDDDDDDDDD.DO.......\n");
    puts_keyb(".....D.DDDDDDDDDDDDDDDDDDDDDDDDD.D......\n");
    puts_keyb("...,..DDDDDDDDDDDDDDDDDDDDDDDDDDD..,....\n");
    puts_keyb("..D.DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD.D...\n");
    puts_keyb("...ODDDDDDDDDD...............DDDDDDO....\n");
    puts_keyb(".D.DDDDDDDDD.................DDDDDDD.D..\n");
    puts_keyb("D.DDDDDDDDD..DD..=DDDD..DDDDDDDDDDDDD.D.\n");
    puts_keyb("D.DDDDDDDD.DDDD..DDDDD..DDDDDDDDDDDDD.D.\n");
    puts_keyb(".DDDDDDDDDDDDDD..DDDDD..DDDDDDDDDDDDDD..\n");
    puts_keyb(".DDDDDDDDDDDDDD..DDDDD..DDDDDDDDDDDDDD.Z\n");
    puts_keyb(".DDDDDDDDDDDDDD..DDDDN..DDDDDDDDDDDDDD.Z\n");
    puts_keyb(".DDDDDDDDDDDDDO..DDDD7..DDDDDDDDDDDDDD..\n");
    puts_keyb("D.DDDDDDDDDDDD...DDDDO..DDDDDDDDDDDDD.D.\n");
    puts_keyb("D.DDDDDDDDDDD...DDDDDD..DDDD.DDDDDDDD.D.\n");
    puts_keyb(".D.DDDDDDDDD....DDDDDD......DDDDDDDD.D..\n");
    puts_keyb("...ODDDDDDDD...DDDDDDDD....DDDDDDDDO....\n");
    puts_keyb("..D.DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD.D...\n");
    puts_keyb("..D.DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD.D...\n");
    puts_keyb("...,..DDDDDDDDDDDDDDDDDDDDDDDDDDD..,....\n");
    puts_keyb("......OD  TEAM PI   LINUX OS  D.DO......\n");
    puts_keyb(".....   PRESS ENTER TO CONTINUE   ......\n");
    puts_keyb("........................................\n");

   
    // STAR WAR THEME  -  BY JOHN WILLIAMS
    beep(293, 0x900000);  //D4
    print_splash(1);
    beep(293, 0x900000);  //D4
    print_splash(2);
    beep(293, 0x900000);  //D4
    print_splash(3);
    beep(392, 0x1400000);  //F4
    print_splash(4);

    beep(587, 0x900000);   //D5
    print_splash(5);
    beep(523, 0x900000);   //C5
    print_splash(6);
    beep(494, 0x900000);   //B4
    print_splash(7);
    beep(440, 0x900000);    //A4
    print_splash(8);
    beep(784, 0x2400000);    //F5
    print_splash(9);
    beep(587, 0x1400000);    //D5
    print_splash(0xA);

    beep(587, 0x900000);   //D5
    print_splash(0xB);
    beep(523, 0x900000);   //C5
    print_splash(0xC);
    beep(494, 0x900000);   //B4
    print_splash(0xD);
    beep(440, 0x900000);    //A4
    print_splash(1);
    beep(784, 0x2400000);    //F5
    print_splash(2);
    beep(587, 0x1400000);    //D5
    print_splash(3);

    beep(523, 0xA00000);   //C5
    print_splash(4);
    beep(494, 0xA00000);   //B4
    print_splash(5);
    beep(523, 0xA00000);   //C5
    print_splash(1);
    beep(440, 0x2400000);    //A4
    print_splash(6);

    int delay = 0;
    uint8_t c;

    //wait for enter key pressed
    while (delay<0x23000)
    {
        outb(0x21,0x64);
        c = inb(0x60);
        if (c == 0x1C)
            break;
        delay++;
    }
    clear_keyb();
    return;
}




