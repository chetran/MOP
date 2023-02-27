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
#define GPIO_D 0x40020C00
#define GPIO_ODR_HIGH ((volatile unsigned char *)   (GPIO_D+0x15))
#define GPIO_ODR_LOW ((volatile unsigned char *)   (GPIO_D+0x14))
#define GPIO_IDR_HIGH ((volatile unsigned char *)   (GPIO_D+0x11))
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

void init_app(void);
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

unsigned char keyb(void);
int ReadColumn( void );
void ActivateRow( unsigned int row );

#define MAX_POINTS 30

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

typedef struct
{
	int numpoints;
	int sizex;
	int sizey;
	POINT px[MAX_POINTS];
} GEOMETRY, *PGEOMETRY;

typedef struct tObj
{
	PGEOMETRY geo;
	int dirx, diry;
	int posx, posy;
	void (* draw) (struct tObj *);
	void (* clear) (struct tObj *);
	void (* move) (struct tObj *);
	void (* set_speed) (struct tObj *, int, int);
} OBJECT, *POBJECT;

int draw_line(PLINE l);
void swap(unsigned char *a, unsigned char *b);
void draw_rectangle(PRECT r);
void draw_polygon(PPOLYPOINT polygon);
void draw_ballobject(POBJECT o);
void clear_ballobject(POBJECT o);
void move_ballobject(POBJECT o);
void set_ballobject_speed(POBJECT o, int speedx, int speedy);
void move_paddle(POBJECT p);
void bounce(POBJECT paddle, POBJECT ball);
int gameover(POBJECT b);
void resetgame(POBJECT paddle, POBJECT ball);

// Lab 4
void my_irq_handler(void);

// PONG BALL 
GEOMETRY ball_geometry = 
{
	12,
	4,4,
	{
		{0,1},{0,2},{1,0},{1,1},{1,2},{1,3},{2,0},{2,1},{2,2},{2,3},{3,1},{3,2}
	}
};

static OBJECT ballobject =
{
	&ball_geometry,
	7,1,
	1,1,
	draw_ballobject,
	clear_ballobject,
	move_ballobject,
	set_ballobject_speed,
};

// paddle
GEOMETRY paddle = 
{
	27,
	5,9,
	{
		{0,0},{1,0},{2,0},{3,0},{4,0},{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{4,7},{4,8},{3,8},{2,8},{1,8},{0,8},{0,7},{0,6},{0,5},{0,4},{0,3},{0,2},{0,1},{2,3},{2,4},{2,5}
	}
};

static OBJECT paddle_object = 
{
	&paddle,
	0,0,
	115,25,
	draw_ballobject,
	clear_ballobject,
	move_paddle,
	set_ballobject_speed,
};

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
		POLYPOINT pg2 = {40, 10, &pg3};
		POLYPOINT pg1 = {20, 20, &pg2};
		while (1)
		{
			draw_polygon(&pg1);
			delay_milli(10);
			graphic_clear_screen();
		}
		
	}	

}

void init_app(void)
{
	// Need to intials the outport 
	*((unsigned long *) 0x40023830) = 0x18;

    *((volatile unsigned int *)0x40020C08) = 0x55555555; // MEDIUM SPEED
    * ( (volatile unsigned int *) 0x40020C00) &= 0x00000000; // MODER CONFIG
    * ( (volatile unsigned int *) 0x40020C00) |= 0x55005555; // MODER CONFIG
    * ( (volatile unsigned short *) 0x40020C04) &= 0x0000; // TYPER CONFIG
    * ( (volatile unsigned int *) 0x40020C0C) &= 0x00000000; // PUPDR CONFIG
    * ( (volatile unsigned int *) 0x40020C0C) |= 0x0000AAAA; // PUPDR CONFIG

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

// ------------------------------------------------------- keypad ------------------------------------------------------------------------------- //

void ActivateRow( unsigned int row )
{

    /* Aktivera angiven rad hos tangentbordet, eller

    * deaktivera samtliga */
    switch( row )
    {
    case 1: *GPIO_ODR_HIGH = 0x10; break;
    case 2: *GPIO_ODR_HIGH = 0x20; break;
    case 3: *GPIO_ODR_HIGH = 0x40; break;
    case 4: *GPIO_ODR_HIGH = 0x80; break;
    case 0: *GPIO_ODR_HIGH = 0x00; break;

    }

}

int ReadColumn( void )
{

    /* Om någon tangent (i aktiverad rad)

    * är nedtryckt, returnera dess kolumnnummer,

    * annars, returnera 0 */
    unsigned char c;
    c = *GPIO_IDR_HIGH;
    if ( c & 0x8 )
        return 4;
    if ( c & 0x4 )
        return 3;
    if ( c & 0x2 )
        return 2;
    if ( c & 0x1 )
        return 1;

    return 0;
}

unsigned char keyb(void)
{

    unsigned char key[]={1,2,3,0xA,4,5,6,0xB,7,8,9,0xC,0xE,0,0xF,0xD};

    int row, col;
    for(row=1; row <=4 ; row++ )
    {
        ActivateRow( row );
        if( (col = ReadColumn () ) )
        {
            ActivateRow( 0 );
            return key [4*(row-1)+(col-1) ];
        }
    }
    ActivateRow( 0 );
    return  0xFF;
}

// ------------------------------------------------------- Ascii display ------------------------------------------------------------------------------- //
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
	int steep;
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
	int deltax = l->p1.x - l->p0.x;
	int deltay = abs(l->p1.y - l->p0.y);
	int error = 0;
	int y = l->p0.y;
	int ystep;
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

// ------------------------------------------------------- Objects ------------------------------------------------------------------------------- //
void draw_ballobject(POBJECT o)
{
	int pixels = o->geo->numpoints;

	for (int i = 0; i < pixels; i++)
	{
		// (o->geo->px+i) gets the position of one of the pixels and then the period after gets x/y value of that point
		int testx = o->posx + (o->geo->px+i)->x;
		int testy = o->posy + (o->geo->px+i)->y;
		graphic_pixel_set(o->posx + (o->geo->px+i)->x, o->posy + (o->geo->px+i)->y);
	}

}

void clear_ballobject(POBJECT o)
{
	int pixels = o->geo->numpoints;
	for (int i = 0; i < pixels; i++)
	{
		graphic_pixel_clear(o->posx + (o->geo->px+i)->x, o->posy + (o->geo->px+i)->y);
	}

}

void move_ballobject(POBJECT o)
{
	clear_ballobject(o);
	int newx = o->dirx + o->posx;
	int newy = o->diry + o->posy;
	if (newx < 1) // touches left side
	{
		// if its towards the left side x dir is negative which means we need to set it as positive 
		o->dirx = abs(o->dirx);
		newx = o->dirx + o->posx;
	}	
	if (newx > 128) // touches right side
	{
		// if it touches right side x dir is positive and we need to set it negative. 
		o->dirx = -(o->dirx);
		newx = o->dirx + o->posx;
	}
	if (newy < 1) // touches top side
	{
		o->diry = abs(o->diry);
		newy = o->diry + o->posy;
	}
	if (newy > 64) // touches bottom side
	{
		o->diry = -(o->diry);
		newy = o->diry + o->posy;
	}
	o->posx = newx;
	o->posy = newy;
	draw_ballobject(o);
}

void set_ballobject_speed(POBJECT o, int speedx, int speedy)
{
	o->dirx = speedx;
	o->diry = speedy;
}

void move_paddle(POBJECT p)
{
	clear_ballobject(p);
	int newy = p->posy + p->diry;
	if (newy > -1 && newy < 60)
		p->posy = newy;
	draw_ballobject(p);
}

void bounce(POBJECT paddle, POBJECT ball)
{
	int ballx = ball->posx + ball->dirx;
	int bally = ball->posy;
	int paddlex = paddle->posx;
	int paddley = paddle->posy;
	int dir = -ball->dirx;
	if (ballx >= paddlex && bally >= paddley && bally <= (paddley + 8))
	{
		ball->set_speed(ball, dir, ball->diry);
	}
}

int gameover(POBJECT b)
{
	if (b->posx >= 127)
	{
		char *s;
		char test1[] = "Game Over! ";

		init_app();
		ascii_init();
		ascii_gotoxy(1, 1);
		s = test1;
		while (*s)
		{
			ascii_write_char(*s++);
		}
		return 1;
	}
	return 0;
}

void resetgame(POBJECT paddle, POBJECT ball)
{
	graphic_clear_screen();
	ball->posx = 1;
	ball->posy = 1;
	paddle->posx = 115;
	paddle->posy = 25;
	paddle->draw(paddle);
	ball->draw(ball);
}

// ------------------------------------------------------- Interrupt ------------------------------------------------------------------------------- //
void my_irq_handler(void)
{
	// tänd diodramp på port D 
	//*GPIO_D_MODER = 0x00005555;
	//*GPIO_D_ODR_LOW = 0xFF;
}


