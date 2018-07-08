//reference: SANik, osdev.org

#include "mouse.h"
#include "lib.h"
#include "terminal.h"

#define NUM_COLS_M 80
#define NUM_ROWS_M 25
#define ATTRIB_M 0x07

#define M_CUR 0x2A

#define MOUSE_VID 0xB8000

static int mouse_x;
static int mouse_y;
static char* mouse_video = (char*)MOUSE_VID;

static char char_restore = ' ';
static uint8_t char_attrib = ATTRIB_M;

static int mouse_cycle = 0;

static int cnt = 0;

static int8_t s[3];

static char m_cmd[50];

static int exe_flag[3] = {0,0,0};

/* 
  clear_cursor
	Description: restore the character that has been covered
				 by cursor, and clear cursor out of screen
				 used together with put_cursor
	Input: none
	Output: none
*/
void clear_cursor(void)
{
	*(uint8_t *)(mouse_video + ((NUM_COLS_M*mouse_y + mouse_x) << 1)) = char_restore;
	*(uint8_t *)(mouse_video + ((NUM_COLS_M*mouse_y + mouse_x) << 1) + 1) = char_attrib;
}

/* 
  clear_cursor
	Description: save the character that is at the same position 
				 as cursor, and replace it with cursor character
				 used together with clear_cursor
	Input: none
	Output: none
*/
void put_cursor(void)
{
	char_restore = *(uint8_t *)(mouse_video + ((NUM_COLS_M*mouse_y + mouse_x) << 1));
	char_attrib = *(uint8_t *)(mouse_video + ((NUM_COLS_M*mouse_y + mouse_x) << 1) + 1);

	*(uint8_t *)(mouse_video + ((NUM_COLS_M*mouse_y + mouse_x) << 1)) = M_CUR;
	*(uint8_t *)(mouse_video + ((NUM_COLS_M*mouse_y + mouse_x) << 1) + 1) = ATTRIB_M;
}

/* 
  init_mouse
	Description: enable mouse port, enable mouse to
				 generate interrupt
	Input: none
	Output: none
*/
void init_mouse(void)
{	char c;
	
	//Enable the auxiliary mouse device
	mouse_wait(0);
	outb(0xA8,0x64);
	
	//enable interrupts
	mouse_wait(0);
	outb(0x20,0x64);
	
	mouse_wait(1);
	c = (inb(0x60) | 0x02) & 0xDF;
	
	mouse_wait(0);
	outb(0x60,0x64);
	
	mouse_wait(0);
	outb(c,0x60);
	
	//set mouse setting to default
	mouse_write(0xF6);
	mouse_read();
	
	mouse_write(0xF4);
	mouse_read();
	
	mouse_write(0xF3);
	mouse_read();
	mouse_write(100);
	
	inb(0x60);
	
	//initialize mouse position
	mouse_x = NUM_COLS_M / 2;
	mouse_y = NUM_ROWS_M / 2;
	
	*(uint8_t *)(mouse_video + ((NUM_COLS_M*mouse_y + mouse_x) << 1)) = M_CUR;
	*(uint8_t *)(mouse_video + ((NUM_COLS_M*mouse_y + mouse_x) << 1) + 1) = ATTRIB_M;
}

/* 
  do_mouse
	Description: mouse interrupt handler,
				 mouse will generate 3 consecutive
				 interrupts every time mouse is moved
				 or clicked,process these packets to
				 show the correct move of mouse
	Input: none
	Output: none
*/
void do_mouse(void)
{
	int i;
	char c;
	dentry_t dt;
	char ss[15] = {'E','x','e','c','u','t','e','?',' ','Y','e','s',' ','N','o'};
	//Mouse functions
	cnt++; //further use, hasn't been implemented yet
	//save 3 packets
	switch(mouse_cycle)
	{
		case 0:
			s[0]=inb(0x60);
			mouse_cycle++;
			break;
		case 1:
			s[1]=inb(0x60);
			mouse_cycle++;
			break;
		case 2:
			s[2]=inb(0x60);
			mouse_cycle=0;
			break;
	}
	
//	if(cnt == 9)
//	{
//		//printf("mouse interrupt, %x, %x, %x\n", s[0],s[1],s[2]);
//		cnt = 0;
//	}

	//when all 3 packets received, start processing
	if(mouse_cycle==0)
	{
		clear_cursor();
		if(s[1] & 0x10)	//check move in x direction
			mouse_x += s[2] / 3;
		else
			mouse_x += s[2] / 3;
	
		if(s[1] & 0x20)	//check move in y direction
			mouse_y -= s[0] / 3;
		else
			mouse_y -= s[0] / 3;
			
		//keep mouse in the range of screen
		if(mouse_x < 0)
			mouse_x = 0;
		if(mouse_x > 79)
			mouse_x = 79;
		if(mouse_y < 0)
			mouse_y = 0;
		if(mouse_y > 24)
		mouse_y = 24;
		
		//put_cursor();
		
		if(s[1] & 0x01)
		{
			if(mouse_y==NUM_ROWS_M - 1 && mouse_x < 40)
				term_change();
			else
			{
				if(exe_flag[curr_term]==0){	
					i = 0;
					c = *(char *)(mouse_video + ((NUM_COLS_M*mouse_y + i) << 1));
					while(c!='\n' && c!=' ')
					{
						if(c==0x2A)
							c = char_restore;
						m_cmd[i] = c;
						i++;
						c = *(char *)(mouse_video + ((NUM_COLS_M*mouse_y + i) << 1));
					}
					m_cmd[i] = '\0';
					//printf("%s\n",m_cmd);
					read_dentry_by_name(m_cmd,&dt);
					if(check_executable(dt.inode) > 0)
					{
						exe_flag[curr_term] = 1;//mouse_cmd(m_cmd);
						for(i=0;i<15;i++)
						{
							*(char *)(mouse_video + ((NUM_COLS_M*24 + 40 + i) << 1)) = ss[i];
							*(char *)(mouse_video + ((NUM_COLS_M*24 + 40 + i) << 1) + 1) = 0x67;
						}
					}
				}
				else{
					if((mouse_x==49 || mouse_x==50 || mouse_x==51)&& mouse_y==NUM_ROWS_M-1)
					{
						mouse_cmd(m_cmd);
						exe_flag[curr_term] = 0;
					}
					else if((mouse_x==53 || mouse_x==54)&& mouse_y==NUM_ROWS_M-1)
						exe_flag[curr_term] = 0;
						
					for(i=0;i<15;i++)
					{
						*(char *)(mouse_video + ((NUM_COLS_M*24 + 40 + i) << 1)) = ' ';
						*(char *)(mouse_video + ((NUM_COLS_M*24 + 40 + i) << 1) + 1) = 0x62;
					}
				}
			}
		}
		put_cursor();
	}
}

/* 
  mouse_read
	Description: read the acknowledge code sent by mouse,
				 which means it has got the command
	Input: none
	Output: none
*/
void mouse_read(void)
{	char c;
	mouse_wait(1);
	c = inb(0x60);
}

/* 
  mouse_write
	Description: send command to mouse to make
				 adjustment to mouse
	Input: c: the content we wish to send to mouse
	Output: none
*/
void mouse_write(uint8_t c)
{
	mouse_wait(0);
	outb(0xD4,0x64);	//tell mouse a command will be sent
	mouse_wait(0);
	outb(c,0x60);
}

/* 
  mouse_wait
	Description: each time we want to write or read from mouse
				 we have to wait for mouse to get ready, this
				 function implement this, and returns when mouse
				 is ready to take next step
	Input: a: indicate wait mode, 0/write to 0x64
			  1/read from port
	Output: none
*/
int mouse_wait(int a)
{
	char aa;
	aa = inb(0x64);
	if(a==0)
	{
		while(aa & 0x02)
			aa = inb(0x64);
	}
	else
	{
		while(!(aa & 0x01))
			aa = inb(0x64);
	}	
	return 0;
}
