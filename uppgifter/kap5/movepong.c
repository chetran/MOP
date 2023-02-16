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
void init_app(void);
void delay_250ns(void);
void delay_micro(unsigned int ms);
void delay_milli(unsigned int ms);

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

// Lab 4
void my_irq_handler(void);

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
	0,0,
	1,1,
	draw_ballobject,
	clear_ballobject,
	move_ballobject,
	set_ballobject_speed,
};

void main(void)
{
	char c;
	POBJECT p = &ballobject;
	init_app();
	graphic_initalize();
	graphic_clear_screen();
	while(1)
	{
		p->move(p);
		delay_micro(100);
		c = keyb();
		switch(c)
		{
			case 6: p->set_speed(p, 3, 0); break;
			case 4: p->set_speed(p, -3, 0); break;
			case 5: p->set_speed(p, 0, 0); break;
			case 2: p->set_speed(p, 0, -3); break;
			case 8: p->set_speed(p, 0, 3); break;
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

// ------------------------------------------------------- Objects ------------------------------------------------------------------------------- //
void draw_ballobject(POBJECT o)
{
	int pixels = o->geo->numpoints;

	for (int i = 0; i < pixels; i++)
	{
		// (o->geo->px+i) gets the position of one of the pixels and then the period after gets x/y value of that point
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

// ------------------------------------------------------- Interrupt ------------------------------------------------------------------------------- //
void my_irq_handler(void)
{
	// tänd diodramp på port D 
	//*GPIO_D_MODER = 0x00005555;
	//*GPIO_D_ODR_LOW = 0xFF;
}


