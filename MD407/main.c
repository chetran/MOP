/// main.c

#include "debug.h"
#include "startup.h"
#include <stdbool.h>

// Timer 6
#define STK_CTRL (volatile unsigned int *) 0xE000E010 
#define STK_LOAD (volatile unsigned int *) 0xE000E014 
#define STK_VAL (volatile unsigned int *) 0xE000E018 
#define GPIO_D 0x40020C00
#define GPIO_MODER ((volatile unsigned int *)   (GPIO_D))
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

#define TIM6_CR1 ((volatile unsigned short *) 0x40001000)
#define TIM6_CNT ((volatile unsigned short *) 0x40001024)
#define TIM6_ARR ((volatile unsigned short *) 0x4000102C)
#define UDIS (1<<1)
#define CEN (1<<0)

#define MAX_POINTS 50
#define X_SIZE 128
#define Y_SIZE 64

// ------------------------------------------------------- STRUCTS ------------------------------------------------------------------------------- //
typedef struct 
{
	unsigned char x,y;
} POINT, *PPOINT;

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
	int posx, posy;
	void (* draw) (struct tObj *);
	void (* clear) (struct tObj *);
	void (* move) (struct tObj *);
	void (* set_speed) (struct tObj *, int, int);
} OBJECT, *POBJECT;

typedef struct {
    int x, y;
} apple_t;

typedef enum {
    UP, RIGHT, DOWN, LEFT
} dir_t;

// The snake
typedef struct {
    OBJECT body_part[X_SIZE * Y_SIZE];
    int length;
    dir_t dir;
} snake_t;

// ------------------------------------------------------- FUNCTIONS ------------------------------------------------------------------------------- //

void init_app(void)
{
	// Starts the clocks for D and E port
	*((unsigned long *) 0x40023830) = 0x18;
	// Starts the clock for SYSCFG */
	* ((unsigned long *)0x40023844) |= 0x4000; 	
	// Relocates the vector table
	* ((unsigned long *)0xE000ED08) = 0x2001C000;

	// Initialize port D for display usage
	*GPIO_MODER = 0x55555555;	

}

void timer6_init(void)
{
	*TIM6_CR1 &= ~CEN; // Stops the counter module
	*TIM6_ARR = 0xFFFF; // If its set to a low number, the random number will always be the upper bound because it counts to fast.
	*TIM6_CR1 |= ( CEN | UDIS); // Activates the counter module and disables "update event"
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

// ------------------------------------------------------- KEYPAD ------------------------------------------------------------------------------- //

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

// ------------------------------------------------------- SNAKE FUNCTIONS ------------------------------------------------------------------------------- //

void apple_new(apple_t *apple, snake_t *snake) {
    // Draw random positions until position not hitting snake found
    bool ready = false;
    int x_try, y_try;
    while (!ready)
    {
        x_try = 1 + (char) *TIM6_CNT % (X_SIZE); // (char) *TIM6_CNT gets a random number 
        y_try = 1 + (char) *TIM6_CNT % (Y_SIZE);
        ready = true;
        for (int i=0; i<snake->length && ready; ++i)
        {
            if (x_try == snake->body_part[0].posx && y_try == snake->body_part[0].posy)
                ready = false;
        }
    }
    apple->x = x_try;
    apple->y = y_try;
}

void snake_turn(snake_t *snake) {
	char input = keyb(); // fetch the key value to input
	switch (input)
	{
	case 2:
		snake->dir = UP;
		break;
	case 4:
		snake->dir = LEFT;
		break;
	case 8:
		snake->dir = DOWN;
		break;
	case 6:
		snake->dir = RIGHT;
		break;
	default:
		break;
	}

}

void snake_move(snake_t *snake) 
{
    for (int i = snake->length - 0; i > 0; i--)
    {
        snake->body_part[i].posx = snake->body_part[i - 1].posx;
        snake->body_part[i].posy = snake->body_part[i - 1].posy;
        
    }
    switch (snake->dir)
        {
        case UP:
            snake->body_part[0].posy--;
            break;
        case DOWN:
            snake->body_part[0].posy++;
            break;
        case LEFT:
            snake->body_part[0].posx--;
            break;
        default:
            snake->body_part[0].posx++;
            break;
        }
    
}

bool snake_eat_apple(apple_t *apple, snake_t *snake) {
    if (snake->body_part[0].posx == apple->x && snake->body_part[0].posy == apple->y)
    {
        // Can't increase the length of the snake.
        apple_new(apple, snake);
        snake->length++;
        int new = snake->length - 1;
        
        switch (snake->dir)
        {
        case UP:
            snake->body_part[new].posx = snake->body_part[new - 1].posx;
            snake->body_part[new].posy = snake->body_part[new - 1].posx + 1;
        case DOWN:
            snake->body_part[new].posx = snake->body_part[new - 1].posx;
            snake->body_part[new].posy = snake->body_part[new - 1].posx - 1;
        case LEFT:
            snake->body_part[new].posx = snake->body_part[new - 1].posx - 1;
            snake->body_part[new].posy = snake->body_part[new - 1].posx;
            break;
        default:
            snake->body_part[new].posx = snake->body_part[new - 1].posx + 1;
            snake->body_part[new].posx = snake->body_part[new - 1].posx;
            break;
        }

        return true;
    }
    return false;
}

bool snake_hit_self(snake_t *snake) {
    
    if (snake->length > 2)
    {
        for (int i = 1; i < snake->length; i++)
        {
            if (snake->body_part[0].posx == snake->body_part[i].posx && snake->body_part[0].posy == snake->body_part[i].posy)
            {
                return true;
            }
        }
        
    }
    return false;
}

bool snake_hit_wall(snake_t *snake) 
{
    for (int i = 0; i < X_SIZE; i++)
    {
        if (snake->body_part[0].posx == i && (snake->body_part[0].posy == 0 || snake->body_part[0].posy == Y_SIZE))
        {
            return true;
        }
    }
    for (int j = 0; j < Y_SIZE; j++)
    {
        if (snake->body_part[0].posy == j && (snake->body_part[0].posx == 0 || snake->body_part[0].posx == X_SIZE)) {
            return true;
        }
    }
    return false;
}

// ------------------------------------------------------- GAME ------------------------------------------------------------------------------- //

void main(void) 
{
	bool snake_dead = false;
	snake_t snake;
	init_app();

	apple_t apple;
    apple_new(&apple, &snake);

	while (!snake_dead) 
	{
        snake_turn(&snake);
        snake_move(&snake);
		// need some kinda of draw function 
    
        if (snake_eat_apple(&apple, &snake)) 
		{

        }
        snake_dead = snake_hit_wall(&snake) || snake_hit_self(&snake);
        if (snake_dead) 
		{
            
        }
        
    }
	
	


}
