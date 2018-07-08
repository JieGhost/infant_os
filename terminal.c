#include "terminal.h"
#include "lib.h"
#include "videoterm.h"
#include "signal.h"

#define  L_SHIFT 0x2A
#define  R_SHIFT 0x36
#define CAPS_LOCK	0x3A

#define CTRL		0x1D

#define ALT		0x38

#define F1		0x3B
#define F2		0x3C
#define F3		0x3D
#define ENTERKEY 0x1C
#define BACKSPACE 0x0E

#define PAGE_UP		0x49
#define PAGE_DOWN	0x51

#define UP 0x48
#define DOWN 0x50
#define cmdhist_size 5

#define TAB 0x0F


char* reserved_cmd[7] = {"ls",
                         "grep",
                        "hello",
                        "sigtest",
                        "shell",
                        "fish",
                        "cat"
                        };

/////////////////////////////
static int scrll;

/* used for keyboard hanler */
static int capslock;
static int L_R_shift;
static int L_R_ctrl;
static int L_R_alt;
static uint8_t enter_pressed[3];

/* used for storing command and reading from terminal */
static char buff_read[cmdbuff_size];
static char* cmdbuff;
static int cmdbuffindex;
static int buffindex[3]={0,0,0};
static char commandbuff[3][cmdbuff_size];

/* used for implementing command history */
static char cmdhist[3][cmdhist_size][cmdbuff_size];
static int cmdhist_index[3]={0,0,0};
static int indexadjust;


static uint8_t scasc_unshifted[80] = {	0,0,'1','2','3','4','5','6','7','8',
										'9','0','-','=',0,0,0,0,0,0,
										0,0,0,0,0,0,'[',']',0,0,/*ctrl*/
										0,0,0,0,0,0,0,0,0,';',
										0x27,'`',0,0x5C,0,0,0,0,0,0,
										0,',','.','/',0,0,0,' ',0,0,
										0,0,0,0,0,0,0,0,0,0,
										0,0,0,0,0,0,0,0,0,0};
static uint8_t scasc_shifted[80] = {0,0,'!','@','#','$','%','^','&','*',
									'(',')','_','+',0,0,0,0,0,0,
									0,0,0,0,0,0,'{','}',0,0,/*ctrl*/
									0,0,0,0,0,0,0,0,0,':',
									'"','~',0,'|',0,0,0,0,0,0,
									0,'<','>','?',0,0,0,' ',0,0,
									0,0,0,0,0,0,0,0,0,0,
									0,0,0,0,0,0,0,0,0,0};

/* 
  do keyboard
	Description: receive scancode and 
				 call deal_keyboard to handle
	Input: none
	Output: none
*/
void do_keyboard(void)
{
	uint8_t c;
	
	outb(0x21,0x64);
	c = inb(0x60);
	
	deal_keyboard(c);
}

/* 
  read terminal
	Description: when enter key is pressed, copy string in buff_read to buf
				  add  \n and EOS in the end
	Input: target buf pointer, size of buf
	Output: number of bytes read
*/
int32_t read_terminal(uint32_t inode,uint32_t offset,uint8_t* buf,int32_t nbytes)
{
	int i;

	/* set enter flag of current process' terminal to 0 */
	enter_pressed[current->term] = 0;
	/* wait for enter to be pressed */
	while(!enter_pressed[current->term]);
	
	for(i=0;i<nbytes && buff_read[i]!='\0';i++)
		buf[i] = buff_read[i];
		beep(1046,0x20000);	
	buf[i] = '\n';
	buf[i+1] = '\0';
	
	/* reset the flag to 0 again */
	enter_pressed[current->term] = 0;
	
	return i+1;
}

/* 
  write terminal
	Description: write the content in buf to the terminal
	Input: source buf pointer, size of buf
	Output: number of bytes written
*/
int32_t write_terminal(char* buf,int32_t nbytes)
{	
	int i,j,t;
	int f = 0;
	//uint8_t temp;
	dentry_t dtt;
	char ff[30];
	/* check if the buf content is an executable file name,
	   if so, write with different color, for extra credit */
	for(j=0;j<30;j++)
	{
		if(buf[j]=='\n')
			break;
		ff[j] = buf[j];
	}
	ff[j] = '\0';
	if(read_dentry_by_name((uint8_t*)ff,&dtt)==0)
	{
		if(check_executable(dtt.inode)>0)
			f = 1;
	}
	
	/* writing process, call different functions based on whether
	   current process' terminal and current terminal are match */
	cli();
	
	t = current->term;
	if(t==curr_term)
	{
		if(f==0)
		{	
			for(i=0;i<nbytes /*&& buf[i]!='\0'*/;i++)
				putc_keyb(buf[i]);
		}
		else
			i = put_exe(buf,nbytes); //special function for executable file name
	}
	else
	{
		i = put_t(buf,nbytes,t);
	}

	sti();
	return i;
}
/////////////////////////////////////////////////



/* 
  inikeyboard
	Description: set flags to 0
				 clear buff and cmd buff index
	Input: none
	Output: none
*/
void inikeyboard()
{
	scrll = 0;
	capslock = 0;
	L_R_shift = 0;
	L_R_ctrl = 0;
	L_R_alt = 0;

	cmdbuff = commandbuff[0];
	curr_term = 0;

	clear_cmdbuff();
	cmdbuffindex = 0;

	clear_keyb();

}


/* 
  clear_cmdbuff
	Description: helper function used for clearing command buff
	Input: none
	Output: none
*/
void clear_cmdbuff()
{
	int i;
	for(i=0; i< (unsigned int) cmdbuff_size; i++)
		cmdbuff[i] = 0;
	return;
}


/* 
  deal_keyboard
	Description: process input scancode,
				 core part of keyboard handler
	Input: scancode
	Output: none
*/
void deal_keyboard(uint8_t scancode)
{
	int a;
	uint8_t charprint;

	/* check pageup and pagedown for scroll screen, for extra credit */
	if(scancode==PAGE_UP)
	{
		if(L_R_shift==1)
		{
			if(check_full_screen()==0)
			{
				if(scrll==0)
					save_to_save();
				scrolled(1);
				scrll = 1;
			}
		}
		return;
	}
	if(scancode==PAGE_DOWN)
	{
		if(L_R_shift==1)
		{
			if(check_full_screen()==0)
			{
				if(scrll==0)
					save_to_save();
				scrolled(0);
				scrll = 1;
			}
		}
		return;
	}
	
	/* check press and release of ctrl,alt,shift */
	if(scancode==CTRL)
	{
		L_R_ctrl = 1;
		return;
	}
	
	if(scancode==ALT)
	{
		L_R_alt = 1;
		return;
	}
	
	if(scancode==0x9D)
	{
		L_R_ctrl = 0;
		return;
	}
	
	if(scancode==ALT+0x80)
	{
		L_R_alt = 0;
		return;
	}
	
	//check if shift is pressed
	if(scancode==0x2A || scancode==0x36)
	{
		L_R_shift = 1;
		return;
	}
	//check if shift is released
	if(scancode==0xAA || scancode==0xB6)
	{
		L_R_shift = 0;
		return;
	}
		
	//decode the scan code to ascii code for further process
	charprint = scan_ascii(scancode);
	
	//if it's a release, bit 7 = 1
	if (scancode & 0x80)
		return;
	
	//check if currently in scroll situation
	if(scrll==1)
	{
		scrll = 0;
		scrolled_count = 0;
		back_to_video();
	}
	
	//set enter_press flag of current terminal
	if(scancode==0x1C)
	{
		enter_pressed[curr_term] = 1;
		//return;
	}
	
	//used for command history, for extra credit
	if(scancode==UP)
	{
		getcmd_hist(UP);
		return;
	}
	if(scancode==DOWN)
	{
		getcmd_hist(DOWN);
		return;
	}
	
	//CAPS_LOCK pressed
	if((scancode & 0xFF) == CAPS_LOCK)
	{
		capslock ^= 0x01;
		return;
	}
	
	if(scancode== TAB)
    {
        if(tab_comp(cmdbuff, cmdbuffindex,(char**)reserved_cmd,7) == 1)
        {
            enter_pressed[curr_term] = 1;
            return;
        }
    }
	
	//used for switching terminals
	if(scancode==F1 || scancode==F2 || scancode==F3)
	{
		if(L_R_alt == 0)
			return;
			
		a = scancode - F1;
		
		cmdbuff = commandbuff[a];
		buffindex[curr_term] = cmdbuffindex;
		cmdbuffindex = buffindex[a];
		
		switch_term(a,curr_term);
		curr_term = a;

		return;
	}

	//not a key release

	//if back space is pressed
	if ((scancode & 0xFF) == BACKSPACE && (cmdbuffindex > 0))
	{	
		// remove the last char on screen
		
		
		backspace_keyb();

		//delete the last char entery in cmd buffer
		cmdbuff[cmdbuffindex-1] = 0;			
		cmdbuffindex --;
		return;
	}

	//if enter key is pressed
	if ((scancode & 0xFF) == ENTERKEY )
	{	
		// no cmd entered
		if(cmdbuffindex==0)
		{
			buff_read[0] = '\0';
			putc_keyb('\n');
//			puts_keyb("\nEnter Command>: ");
			return;
		}


		cmdbuff[cmdbuffindex] = '\0';
		addto_histcmd ();
		//execute the "clear" cmd entered
		if(strncmp(cmdbuff,"clear",5)==0)
			clear_keyb();

		else if (strncmp(cmdbuff,"soundoff",8)==0)
			sound_on = 0;

		else if (strncmp(cmdbuff,"soundon",7)==0)
			sound_on = 1;

		//when cmd not recognized, print \n, printing prompt illegal cmd msg
		else
		{	
			putc_keyb('\n');
//			puts_keyb(cmdbuff);
//			puts_keyb(": command not found");
//			
//			// change to a new prompt line
//			puts_keyb("\nEnter Command>: ");
		}
		
		strcpy(buff_read,cmdbuff);
		
		//clear the cmd buffer
		clear_cmdbuff();
		cmdbuffindex = 0;
		return;
	}
	//print char
	if(cmdbuffindex < cmdbuff_size)
	{
		//check if it's character that needs to be printed (scancode is NOT a symbol)
		if (charprint!=0)
		{
			//if reached end of row, go to new line
			//if  ((cmdbuffindex -64) % 80 == 0) 
			//	putc_keyb('\n');
			if(L_R_ctrl==1 && charprint=='l')
			{
				enter_pressed[curr_term] = 1;
				clear_keyb();
				clear_cmdbuff();
				cmdbuffindex = 0;
				
				buff_read[0] = '\0';
				return;
			}
			if (L_R_ctrl == 1 && charprint=='c')    //ctrl-c is pressed
			{
				interrupt_sig_flag = 1;
				enter_pressed[curr_term] = 1;
				return;
			}

		// cap char when necessary
			if(capslock!=0)
			{
				if(L_R_shift==0)
					charprint -= 0x20;
			}
			else
			{
				if(L_R_shift!=0)
					charprint -= 0x20;
			}
			
			// print char on terminal and save it into cmdbuff
			putc_keyb(charprint);
			cmdbuff[cmdbuffindex] = charprint;
			cmdbuffindex ++;
		}

		// scancode is a symbol
		else
		{
			if(L_R_shift==0)
				charprint = scasc_unshifted[scancode];
			else
				charprint = scasc_shifted[scancode];
				
			if(charprint!=0)
			{
				putc_keyb(charprint);
				cmdbuff[cmdbuffindex] = charprint;
				cmdbuffindex ++;
			}
		}
	}
}

/* 
  getcmd_hist
	Description: display command history on the screen
	Input: move: indicate older or newer command to display
	Output: none
*/
void getcmd_hist(int move)
{
	int indexcmd;
	
	//adjust index
	if (move == UP)
		indexadjust += 1;	
	else
		indexadjust -= 1;
	
	if(indexadjust > cmdhist_size)
		indexadjust = cmdhist_size;
	else if(indexadjust < 0)
		indexadjust = 0;
		
	if(indexadjust > cmdhist_index[curr_term])
		indexadjust = cmdhist_index[curr_term];

	// if there is a hist cmd
	indexcmd = cmdhist_index[curr_term]-indexadjust;

	//clear typed on screen ( garbage on cmd line)
	while (cmdbuffindex >0)
	{
		backspace_keyb();
		//delete the last char entery in cmd buffer
		cmdbuffindex --;
		cmdbuff[cmdbuffindex] = 0;	
	}
	
	if(indexadjust==0)
		return;
	
	//move the hist cmd to cmd buff
	strcpy((char*)cmdbuff, (char*)cmdhist[curr_term][indexcmd % cmdhist_size]);
	//print current cmd in cmd buff to screen
	for(cmdbuffindex=0; cmdbuff[cmdbuffindex] != 0; cmdbuffindex++)
		putc_keyb(cmdbuff[cmdbuffindex]);

	return;
}

/* 
  addto_histcmd
	Description: after enter key is typed, command will be
				 added to the command history buffer
	Input: none
	Output: none
*/
void addto_histcmd (void)
{	
	indexadjust = 0;
	
	if(strlen(cmdbuff)<1)
		return;

	cmdhist_index[curr_term] ++;

	strcpy((char*)cmdhist[curr_term][(cmdhist_index[curr_term] - 1) % cmdhist_size],(char*)cmdbuff);

	return;
}


//input: user entered cmd buffer and length, list of reserved cmd
// -1 no cmd entered; 0 only one cmd matched; 1 more than one cmd matched
int tab_comp(char* usercmd, int cmdlength,char** reservelist, int reservelength)
{
    //no cmd entered
    if (cmdlength == 0)
        return -1;

    int matchedIndex, i;
    int samecmd_num[8] = {-1};
    int foundcmd = 0;

    //traverse the list of reserved cmd
    for (i=0;i<reservelength;i++)
    {
        int tempIndex = cmpr_cmd(usercmd, cmdlength, (char*)reservelist[i]);
        if ( tempIndex > -1)
        {
            matchedIndex = tempIndex;
            samecmd_num[foundcmd] = i;        //matched cmd index in the reservelist
            foundcmd++;                        //number of matched cmd
        }
    }

    if (foundcmd == 0)
        return -1;

    //more than 1 cmd found, clear cmdbuff and list all possibilities
    if (foundcmd > 1)
    {
        putc_keyb('\n');
        for (i=0;i<foundcmd;i++)
        {
            puts_keyb((char*)reservelist[samecmd_num[i]]);
            putc_keyb('\n');
        }
   
        // clear cmd buff and print new prompt
        buff_read[0] = '\0';
        //clear the cmd buffer
        clear_cmdbuff();
        cmdbuffindex = 0;
        return 1;
    }
   
    //matched one cmd
    if (foundcmd == 1)
    {
        matchedIndex++;
        while (reservelist[samecmd_num[0]][matchedIndex] != 0)
        {
            putc_keyb(reservelist[samecmd_num[0]][matchedIndex]);
            cmdbuff[cmdbuffindex] = (char)((reservelist[samecmd_num[0]])[matchedIndex]);
            cmdbuffindex++;
            matchedIndex++;
        }
        return 0;
    }
    return -1;

}


//input: user entered cmd buffer and length, one cmd to compare
// -1 not matched ; cmprflag index matched
int cmpr_cmd(char* usercmd, int cmdlength, char* reserve)
{
    int i;
    int cmprflag = 0;
    for (i=0; i<cmdlength; i++)
    {
        //user cmd same as reserved cmd
        if (usercmd[i] == reserve[i])
            cmprflag=i;
        else
            return -1;
    }
    return cmprflag;
}

void term_change(void)
{
	int a;

	a = (curr_term + 1) % 3;
		
	cmdbuff = commandbuff[a];
	buffindex[curr_term] = cmdbuffindex;
	cmdbuffindex = buffindex[a];
	
	switch_term(a,curr_term);
	curr_term = a;
}

void mouse_cmd(char* buf)
{
	strcpy(buff_read,buf);
	enter_pressed[curr_term] = 1;
	putc_keyb('\n');
}


/* 
  scan_ascii
	Description: convert scan code (a letter) into ascii
	Input: scan code
	Output: ascii
*/
char scan_ascii(char inscan)
{	
	char c;
	inscan &= 0xFF;
	switch(inscan)
	{
		case 30:
			c = 'a'; 
			beep(130,0x200000);	
			break;
		case 48:
			c = 'b';	
			beep(138,0x200000);	
			break;
		case 46:
			c = 'c';
			beep(146,0x200000);	
			break;
		case 32:
			c = 'd'; 
			beep(155,0x200000);	
			break;
		case 18:
			c = 'e'; 	
			beep(330,0x200000);	
			break;
		case 33:
			c = 'f';
			beep(174,0x200000);	
			break;
		case 34:
			c = 'g'; 
			beep(185,0x200000);	
			break;
		case 35:
			c = 'h'; 
			beep(196,0x200000);	
			break;
		case 23:
			c = 'i'; 
			beep(523,0x200000);	
			break;
		case 36:
			c = 'j'; 
			beep(220,0x200000);	
			break;
		case 37:
			c = 'k'; 
			beep(233,0x200000);	
			break;
		case 38:
			c = 'l'; 
			beep(246,0x200000);	
			break;
		case 50:
			c = 'm'; 
			beep(261,0x200000);	
			break;
		case 49:
			c = 'n'; 
			beep(277,0x200000);	
			break;
		case 24:
			c = 'o'; 
			beep(587,0x200000);	
			break;
		case 25:
			c = 'p'; 
			beep(660,0x200000);	
			break;
		case 16:
			c = 'q'; 
			beep(261,0x200000);	
			break;
		case 19:
			c = 'r'; 
			beep(350,0x200000);	
			break;
		case 31:
			c = 's'; 
			beep(369,0x200000);	
			break;
		case 20:
			c = 't'; 
			beep(392,0x200000);	
			break;
		case 22:
			c = 'u'; 
			beep(494,0x200000);	
			break;
		case 47:
			c = 'v'; 
			beep(440,0x200000);	
			break;
		case 17:
			c = 'w'; 
			beep(294,0x200000);	
			break;
		case 45:
			c = 'x'; 
			beep(493,0x200000);	
			break;
		case 21:
			c = 'y'; 
			beep(440,0x200000);	
			break;
		case 44:
			c = 'z'; 
			beep(554,0x200000);	
			break;
		default:
			c = 0; 
			beep(587,0x200000);	
			break;
	}
	
	return c;
}


			
/*		
               |10  16  Q      |20  32  D      |30  48  B      |40  64  F6
01   1  ESC    |11  17  W      |21  33  F      |31  49  N      |41  65  F7
02   2  1      |12  18  E      |22  34  G      |32  50  M      |42  66  F8
03   3  2      |13  19  R      |23  35  H      |33  51  ,      |43  67  F9
04   4  3      |14  20  T      |24  36  J      |34  52  .      |44  68  F10
05   5  4      |15  21  Y      |25  37  K      |35  53  /      |45  69  Num
06   6  5      |16  22  U      |26  38  L      |36  54  R Shift|46  70  Scroll
07   7  6      |17  23  I      |27  39  ;      |37  55  PrtSc  |47  71  Home
08   8  7      |18  24  O      |28  40  '      |38  56  Alt    |48  72  Up
09   9  8      |19  25  P      |29  41  `      |39  57  Space  |49  73  PgUp
0A  10  9      |1A  26  [      |2A  42  L Shift|3A  58  Caps   |4A  74  -
0B  11  0      |1B  27  ]      |2B  43  \      |3B  59  F1     |4B  75  Left
0C  12  -      |1C  28    	   |2C  44  Z      |3C  60  F2     |4C  76  Center
0D  13  =      |1D  29  CTRL   |2D  45  X      |3D  61  F3     |4D  77  Right
0E  14  bs     |1E  30  A      |2E  46  C      |3E  62  F4     |4E  78  +
0F  15  Tab    |1F  31  S      |2F  47  V      |3F  63  F5     |4F  79  End
---------------+---------------+---------------+---------------+---------------
50  80  Down   |               |               |               |
51  81  PgDn   |               |               |               |
52  82  Ins    |               |               |               |
53  83  Del    |               |               |               |
---------------+---------------+---------------+---------------+---------------
*/















