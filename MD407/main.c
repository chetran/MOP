/// main.c

#include "debug.h"
#include "startup.h"
#include <stdlib.h>

// In startup.c are functions defined that is built in for processor MD407.

// From delay.c
//#define SIMULATOR // Need to comment to deativate 
#define STK_CTRL (volatile unsigned int *) 0xE000E010 
#define STK_LOAD (volatile unsigned int *) 0xE000E014 
#define STK_VAL (volatile unsigned int *) 0xE000E018 
// For asciidisplay.c
#define PORT_BASE 0x40021000 // Port E 
#define portModer ((volatile unsigned int *)  PORT_BASE)
#define portOtyper ((volatile unsigned short *) (PORT_BASE+0x4))
#define portOspeedr ((volatile unsigned int *) (PORT_BASE+0x8))
#define portPupdr ((volatile unsigned int *) (PORT_BASE+0xC))
#define portIdrLow ((volatile unsigned char *) (PORT_BASE+0x10))
#define portIdrHigh ((volatile unsigned char *) (PORT_BASE+0x11))
#define portOdrLow ((volatile unsigned char *) (PORT_BASE+0x14))	
#define portOdrHigh ((volatile unsigned char *) (PORT_BASE+0x15))
#define B_E 0x40
#define B_SELECT 4
#define B_RW 2
#define B_RS 1

// Functions from asciidisplay.c
void delay_250ns(void);
void delay_micro(unsigned int ms);
void delay_milli(unsigned int ms);
void ascii_ctrl_bit_set(char x);
void ascii_ctrl_bit_clear(char x);
void init_app(void);
void ascii_write_cmd(unsigned char command);
void ascii_write_data(unsigned char data);
unsigned char ascii_read_status(void);
unsigned char ascii_read_data(void);
void ascii_write_controller(unsigned char byte);
unsigned char ascii_read_controller(void);
void ascii_command(unsigned char command);
void ascii_init(void);
void ascii_write_char(unsigned char c);
void ascii_gotoxy(int x, int y);

// LCD MODULE
typedef struct 
{
	unsigned char x,y;
} POINT, *PPOINT;

typedef struct
{
	POINT p0,p1;
} LINE, *PLINE;

typedef struct
{
	POINT p;
	unsigned char x,y;
} RECT, *PRECT;

typedef struct polygonpoint
{
	char x,y;
	struct polygonpoint *next;
} POLYPOINT, *PPOLYPOINT;

int draw_line(PLINE l);
void swap(unsigned char *a, unsigned char *b);
void draw_rectangle(PRECT r);
void draw_polygon(PPOLYPOINT polygon);

void main(void)
{
	graphic_initalize();
	graphic_clear_screen();
	while(1)
	{
		// Resetting the values everytime it runs because sometimes it might change some values. 
		POLYPOINT pg8 = {20, 20, 0};
		POLYPOINT pg7 = {20, 55, &pg8};
		POLYPOINT pg6 = {70, 60, &pg7};
		POLYPOINT pg5 = {80, 35, &pg6};
		POLYPOINT pg4 = {100, 25, &pg5};
		POLYPOINT pg3 = {90, 10, &pg4};
		POLYPOINT pg2 = {40, 90, &pg3};
		POLYPOINT pg1 = {20, 20, &pg2};
		while (1)
		{
			draw_polygon(&pg1);
			delay_milli(2);
			//graphic_clear_screen();
		}
		
	}	

}

void init_app(void)
{
	// Need to intials the outport 
	*((unsigned long *) 0x40023830) = 0x18;

	// Port E för usage of LCD
	*portModer = 0x55555555;

}

// ------------------------------------------------------- DELAYS ------------------------------------------------------------------------------- //
void delay_250ns(void)
{
	// page 98
	*STK_CTRL = 0; // Resets SysTick
	*STK_LOAD = (168 / 4) - 1; // Minus 1 comes from how to processor counts 
	*STK_VAL = 0; // Resets counter register 
	*STK_CTRL = 5; // Starts the count down 
	while(*STK_CTRL & 0x00010000){} // while the countflag is 1 
	*STK_CTRL = 0; // resets Systick 
}

void delay_micro(unsigned int ms)
{
	// When SIMULATOR is defined for some reason it gets stuck here. 	
	#ifdef SIMULATOR
		ms = ms / 1000;
		ms++;
	#endif
	// 4 * 250 ns = 1 µs
	for (int i = 0; i < ms; i++)
	{
		delay_250ns();
		delay_250ns();
		delay_250ns();
		delay_250ns();
	}

}

void delay_milli(unsigned int ms)
{
	#ifdef SIMULATOR
		ms = ms / 1000;
		ms++;
	#endif
	// 1000 µs = 1 ms 
	
	delay_micro(ms * 1000);
}

// ------------------------------------------------------- ASCII DISPLAY ------------------------------------------------------------------------------- //
// B_SELECT is needed here to activate the ascii display
void ascii_ctrl_bit_set(char x)
{
	char c;
	c = *portOdrLow;
	*portOdrLow = B_SELECT | x | c;
}

void ascii_ctrl_bit_clear(char x)
{
	char c;
	c = *portOdrLow;
	c = c & ~x;
	*portOdrLow = B_SELECT | c;
}

// The commented time is the required time for MD407 to perform a certain task. More time doens't affect anything only less time. 
void ascii_write_controller(unsigned char byte)
{
	// These delays are need for the processor to execute the respective functions.
	delay_250ns(); // 40ns
	ascii_ctrl_bit_set(B_E);
	*portOdrHigh = byte;
	delay_250ns(); //230ns
	ascii_ctrl_bit_clear(B_E);
	delay_250ns(); // 10ns 
}

void ascii_write_cmd(unsigned char command)
{
	ascii_ctrl_bit_clear(B_RS);
	ascii_ctrl_bit_clear(B_RW);
	ascii_write_controller(command);
}

void ascii_write_data(unsigned char data)
{
	ascii_ctrl_bit_set(B_RS);
	ascii_ctrl_bit_clear(B_RW);
	ascii_write_controller(data);
}

unsigned char ascii_read_controller(void)
{
	ascii_ctrl_bit_set(B_E);
	delay_250ns();
	delay_250ns(); // 360ns
	unsigned char rv = *portIdrHigh;
	ascii_ctrl_bit_clear(B_E);
	return rv;
}

unsigned char ascii_read_status(void)
{
	*portModer = 0x00005555; // Set bit15-8 as input 
	ascii_ctrl_bit_clear(B_RS);
	ascii_ctrl_bit_set(B_RW);
	unsigned char rv = ascii_read_controller();
	*portModer = 0x55555555; // Set bit15-8 as output
	return rv;
}

unsigned char ascii_read_data(void)
{
	*portModer = 0x00005555; // Set bit15-8 as input 
	ascii_ctrl_bit_set(B_RS);
	ascii_ctrl_bit_set(B_RW);
	unsigned char rv = ascii_read_controller();
	*portModer = 0x55555555; // Set bit15-8 as output 
	return rv;
}

void ascii_command(unsigned char command)
{
	while( (ascii_read_status() & 0x80) == 0x80) // Wait for the display to be ready for instructions
	{}
	delay_micro(8);
	ascii_write_cmd(command);
	delay_micro(45);
}

void ascii_init(void)
{
	ascii_command(0x38); // 2 rows, 5x8 
	ascii_command(0x0E); // Activate display, activate cursor and set it as constant 
	ascii_command(0x01); // Clear Display
	ascii_command(0x06); // Increment, No shift
}

void ascii_write_char(unsigned char c)
{
	while( (ascii_read_status() & 0x80) == 0x80) // Wait for the display to be ready for instructions
	{}
	delay_micro(8);
	ascii_write_data(c);
	delay_micro(45);
}

void ascii_gotoxy(int x, int y)
{
	unsigned char adress = x - 1;
	if ( y == 2 )
	{
		adress = adress + 0x40;
	}
	ascii_write_cmd(0x80 | adress);
}

// ------------------------------------------------------- LCD MODULE ------------------------------------------------------------------------------- //
void swap(unsigned char *a, unsigned char *b)
{
	unsigned char temp = *a;
	*a = *b;
	*b = temp;
}

int draw_line(PLINE l)
{
	// This just checks so that there are no points outside of the display which is 128x64 
	if ((l->p0.x < 1 && l->p0.x > 64) | (l->p1.x < 1 && l->p1.x > 64) | (l->p0.y < 1 && l->p0.y > 128) | (l->p1.y < 1 && l->p1.y > 128))
		return 0;
	
	// Bresenhams Algorithm - honestly no idea how it works 
	char steep;
	if (abs(l->p0.y - l->p1.y) > abs(l->p0.x - l->p1.x))
		steep = 1;
	else
		steep = 0;
	if (steep)
	{
		swap(&l->p0.x, &l->p0.y);
		swap(&l->p1.x, &l->p1.y);
	}
	if (l->p0.x > l->p1.x)
	{
		swap(&l->p0.x, &l->p1.x);
		swap(&l->p0.y, &l->p1.y);
	}
	char deltax = l->p1.x - l->p0.x;
	char deltay = abs(l->p1.y - l->p0.y);
	char error = 0;
	char y = l->p0.y;
	char ystep;
	if (l->p0.y < l->p1.y)
		ystep = 1;
	else
		ystep = -1;
	for (int x = l->p0.x; x <= l->p1.x; x++)
	{
		if (steep)
			graphic_pixel_set(y, x);
		else
			graphic_pixel_set(x, y);
		error = error + deltay;
		if (2 * error >= deltax)
		{
			y = y +ystep;
			error = error - deltax;
		}
	}

	// Returns 1 if it succeded
	return 1;
}

void draw_rectangle(PRECT r)
{
	POINT start;
	POINT end;
	LINE side;
	// Honestly would never write code like this but in the book it looked like this. 
	start.x = r->p.x; start.y = r->p.y; end.x = r->p.x + r->x; end.y = r->p.y; side.p0 = start; side.p1 = end; draw_line(&side);
	start.x = r->p.x + r->x; start.y = r->p.y; end.x = r->p.x + r->x; end.y = r->p.y + r->y; side.p0 = start; side.p1 = end; draw_line(&side);
	start.x = r->p.x + r->x; start.y = r->p.y + r->y; end.x = r->p.x; end.y = r->p.y + r->y; side.p0 = start; side.p1 = end; draw_line(&side);
	start.x = r->p.x; start.y = r->p.y + r->y; end.x = r->p.x; end.y = r->p.y; side.p0 = start; side.p1 = end; draw_line(&side);
}

void draw_polygon(PPOLYPOINT polygon)
{
	POLYPOINT p0;
	p0.x = polygon->x;
	p0.y = polygon->y;
	PPOLYPOINT ptr = polygon->next;
	while (ptr != 0) 
	{
		POLYPOINT p1;
		p1.x = ptr->x;
		p1.y = ptr->y;
		LINE side = {{p0.x, p0.y}, {p1.x, p1.y}};
		draw_line(&side);
		p0.x = p1.x; p0.y = p1.y;
		ptr = ptr->next;
	}
}


