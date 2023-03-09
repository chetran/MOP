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
// For asciidisplay
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

// For random num
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
	int x,y;
} POINT, *PPOINT;

typedef struct tObj
{
	int posx, posy;
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

// ------------------------------------------------------- VARIABLES ------------------------------------------------------------------------------- //
int count_eaten_apples;
char firstD;
char lastD;
POINT snake_design[] = {{2,2},{1,2},{0,2},{-1,2},{-2,2},{-2,1},{-2,0},{-2,-1},{-2,-2},{-1,-2},{0,-2},{1,-2},{2,-2},{2,-1},{2,0},{2,1}};
POINT apple_design[] = {{-1,2},{0,2},{1,2},{-2,1},{-1,1},{0,1},{1,1},{2,1},{-2,0},{-1,0},{0,0},{1,0},{2,0},{-2,-1},{-1,-1},{0,-1},{1,-1},{2,-1},{-1,-2},{0,-2},{1,-2}};
POINT s_start[] = {{27,26},{28,26},{29,26},{30,26},{25,27},{26,27},{27,27},{28,27},{29,27},{30,27},{31,27},{32,27},{23,28},{24,28},{25,28},{26,28},{31,28},{32,28},{33,28},{34,28},{23,29},{24,29},{25,29},{32,29},{33,29},{34,29},{22,30},{23,30},{24,30},{25,30},{23,31},{24,31},{25,31},{23,32},{24,32},{25,32},{26,32},{24,33},{25,33},{26,33},{27,34},{28,34},{29,34},{30,34},{31,34},{32,34},{33,34},{31,35},{32,35},{33,35},{34,35},{32,35},{33,35},{34,35},{32,36},{33,36},{34,36},{35,36},{23,37},{24,37},{25,37},{32,37},{33,37},{34,37},{23,38},{24,38},{25,38},{26,38},{31,38},{32,38},{33,38},{34,38},{25,39},{26,39},{27,39},{28,39},{29,39},{30,39},{31,39},{32,39},{27,40},{28,40},{29,40},{30,40},{40,26},{40,27},{40,28},{40,29},{40,30},{40,31},{40,32},{40,33},{40,34},{40,35},{40,36},{40,37},{40,38},{40,39},{40,40},{40,41},{41,26},{41,27},{41,28},{41,29},{41,30},{41,31},{41,32},{41,33},{41,34},{41,35},{41,36},{41,37},{41,38},{41,39},{41,40},{41,41},{42,26},{42,27},{42,28},{42,29},{42,30},{42,31},{42,32},{42,33},{42,34},{42,35},{42,36},{42,37},{42,38},{42,39},{42,40},{42,41},{43,26},{43,27},{43,28},{44,28},{44,29},{44,30},{45,30},{45,31},{45,30},{45,31},{45,32},{46,32},{46,33},{46,34},{47,34},{47,35},{47,36},{48,36},{48,37},{48,38},{49,38},{49,39},{49,40},{50,39},{50,40},{50,41},{51,26},{51,27},{51,28},{51,29},{51,30},{52,31},{51,32},{51,33},{51,34},{51,35},{51,36},{51,37},{51,38},{51,39},{51,40},{51,41},{52,26},{52,27},{52,28},{52,29},{52,30},{52,31},{52,32},{52,33},{52,34},{52,35},{52,36},{52,37},{52,38},{52,39},{52,40},{52,41},{53,26},{53,27},{53,28},{53,29},{53,30},{53,31},{53,32},{53,33},{53,34},{53,35},{53,36},{53,37},{53,38},{53,39},{53,40},{53,41},{59,32},{59,33},{59,34},{59,35},{59,36},{59,37},{59,38},{59,39},{59,40},{59,41},{60,31},{60,32},{60,33},{60,34},{60,35},{60,36},{60,37},{60,38},{60,39},{60,40},{60,41},{61,30},{61,31},{61,32},{61,33},{61,34},{61,35},{61,36},{61,37},{61,38},{61,39},{61,40},{61,41},{62,29},{62,30},{62,31},{62,36},{62,37},{62,38},{63,28},{63,29},{63,30},{63,36},{63,37},{63,38},{64,27},{64,28},{64,29},{64,36},{64,37},{64,38},{65,26},{65,27},{65,28},{65,36},{65,37},{65,38},{66,26},{66,27},{66,28},{66,36},{66,37},{66,38},{67,27},{67,28},{67,29},{67,36},{67,37},{67,38},{68,28},{68,29},{68,30},{68,36},{68,37},{68,38},{69,29},{69,30},{69,31},{69,36},{69,37},{69,38},{70,30},{70,31},{70,32},{70,33},{70,34},{70,35},{70,36},{70,37},{70,38},{70,39},{70,40},{70,41},{71,31},{71,32},{71,33},{71,34},{71,35},{71,36},{71,37},{71,38},{71,39},{71,40},{71,41},{72,32},{72,33},{72,34},{72,35},{72,36},{72,37},{72,38},{72,39},{72,40},{72,41},{78,26},{78,27},{78,28},{78,29},{78,30},{78,31},{78,32},{78,33},{78,34},{78,35},{78,36},{78,37},{78,38},{78,39},{78,40},{78,41},{79,26},{79,27},{79,28},{79,29},{79,30},{79,31},{79,32},{79,33},{79,34},{79,35},{79,36},{79,37},{79,38},{79,39},{79,40},{79,41},{80,26},{80,27},{80,28},{80,29},{80,30},{80,31},{80,32},{80,33},{80,34},{80,35},{80,36},{80,37},{80,38},{80,39},{80,40},{80,41},{81,26},{81,27},{81,28},{81,29},{81,30},{81,31},{81,32},{81,33},{81,34},{81,35},{81,36},{81,37},{81,38},{81,39},{81,40},{81,41},{82,33},{82,34},{83,33},{83,34},{84,32},{84,33},{84,34},{84,35},{85,31},{85,32},{85,33},{85,34},{85,35},{85,36},{86,30},{86,31},{86,32},{86,35},{86,36},{86,37},{87,29},{87,30},{87,31},{87,36},{87,37},{87,38},{88,28},{88,29},{88,30},{88,37},{88,38},{88,39},{89,27},{89,28},{89,29},{89,38},{89,39},{89,40},{90,26},{90,27},{90,28},{90,39},{90,40},{90,41},{91,26},{91,27},{91,40},{91,41},{97,26},{97,27},{97,28},{97,29},{97,30},{97,31},{97,32},{97,33},{97,34},{97,35},{97,36},{97,37},{97,38},{97,39},{97,40},{97,41},{98,26},{98,27},{98,28},{98,29},{98,30},{98,31},{98,32},{98,33},{98,34},{98,35},{98,36},{98,37},{98,38},{98,39},{98,40},{98,41},{99,26},{99,27},{99,28},{99,29},{99,30},{99,31},{99,32},{99,33},{99,34},{99,35},{99,36},{99,37},{99,38},{99,39},{99,40},{99,41},{100,26},{100,27},{100,28},{100,32},{100,33},{100,34},{100,35},{100,39},{100,40},{100,41},{101,26},{101,27},{101,28},{101,32},{101,33},{101,34},{101,35},{101,39},{101,40},{101,41},{102,26},{102,27},{102,28},{102,32},{102,33},{102,34},{102,35},{102,39},{102,40},{102,41},{103,26},{103,27},{103,28},{103,32},{103,33},{103,34},{103,35},{103,39},{103,40},{103,41},{104,26},{104,27},{104,28},{104,32},{104,33},{104,34},{104,35},{104,39},{104,40},{104,41},{105,26},{105,27},{105,28},{105,32},{105,33},{105,34},{105,35},{105,39},{105,40},{105,41},{106,26},{106,27},{106,28},{106,39},{106,40},{106,41},{107,26},{107,27},{107,28},{107,39},{107,40},{107,41},{108,26},{108,27},{108,28},{108,39},{108,40},{108,41},{109,26},{109,27},{109,28},{109,39},{109,40},{109,41},{110,26},{110,27},{110,28},{110,39},{110,40},{110,41}};
// "n" shifted 10 pixels away from "s"
POINT n_start[] = {{40,26},{40,27},{40,28},{40,29},{40,30},{40,31},{40,32},{40,33},{40,34},{40,35},{40,36},{40,37},{40,38},{40,39},{40,40},{40,41},{41,26},{41,27},{41,28},{41,29},{41,30},{41,31},{41,32},{41,33},{41,34},{41,35},{41,36},{41,37},{41,38},{41,39},{41,40},{41,41},{42,26},{42,27},{42,28},{42,29},{42,30},{42,31},{42,32},{42,33},{42,34},{42,35},{42,36},{42,37},{42,38},{42,39},{42,40},{42,41},{43,26},{43,27},{43,28},{44,28},{44,29},{44,30},{45,30},{45,31},{45,30},{45,31},{45,32},{46,32},{46,33},{46,34},{47,34},{47,35},{47,36},{48,36},{48,37},{48,38},{49,38},{49,39},{49,40},{50,39},{50,40},{50,41},{51,26},{51,27},{51,28},{51,29},{51,30},{52,31},{51,32},{51,33},{51,34},{51,35},{51,36},{51,37},{51,38},{51,39},{51,40},{51,41},{52,26},{52,27},{52,28},{52,29},{52,30},{52,31},{52,32},{52,33},{52,34},{52,35},{52,36},{52,37},{52,38},{52,39},{52,40},{52,41},{53,26},{53,27},{53,28},{53,29},{53,30},{53,31},{53,32},{53,33},{53,34},{53,35},{53,36},{53,37},{53,38},{53,39},{53,40},{53,41}};


// ------------------------------------------------------- FUNCTIONS ------------------------------------------------------------------------------- //
void timer6_init(void)
{
	*TIM6_CR1 &= ~CEN; // Stops the counter module
	*TIM6_ARR = 0xFFFF; // If its set to a low number, the random number will always be the upper bound because it counts to fast.
	*TIM6_CR1 |= ( CEN | UDIS); // Activates the counter module and disables "update event"
}

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

	*((volatile unsigned int *)0x40020C08) = 0x55555555; // MEDIUM SPEED
    * ( (volatile unsigned int *) 0x40020C00) &= 0x00000000; // MODER CONFIG
    * ( (volatile unsigned int *) 0x40020C00) |= 0x55005555; // MODER CONFIG
    * ( (volatile unsigned short *) 0x40020C04) &= 0x0000; // TYPER CONFIG
    * ( (volatile unsigned int *) 0x40020C0C) &= 0x00000000; // PUPDR CONFIG
    * ( (volatile unsigned int *) 0x40020C0C) |= 0x0000AAAA; // PUPDR CONFIG	

	timer6_init();

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

// ------------------------------------------------------- ASCII FUNCTIONS ------------------------------------------------------------------------------- //

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

// ------------------------------------------------------- SNAKE FUNCTIONS ------------------------------------------------------------------------------- //
void draw_clear_snake(snake_t *snake, int clear)
{
	
	for (int i = 0; i < snake->length; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			if (clear)
				graphic_pixel_clear(snake->body_part[i].posx + snake_design[j].x, snake->body_part[i].posy + snake_design[j].y);
			else
				graphic_pixel_set(snake->body_part[i].posx + snake_design[j].x, snake->body_part[i].posy + snake_design[j].y);
		}	
	}
}

void draw_clear_apple(apple_t *apple, int clear)
{
	for (int i = 0; i < 21; i++)
	{
		if (clear)
			graphic_pixel_clear(apple->x + apple_design[i].x, apple->y+ apple_design[i].y);
		else
			graphic_pixel_set(apple->x + apple_design[i].x, apple->y+ apple_design[i].y);
	}
}


void apple_new(apple_t *apple, snake_t *snake) {
    // Draw random positions until position not hitting snake found
    bool ready = false;
    int x_try, y_try;
    while (!ready)
    {
        x_try = 20 + (char) *TIM6_CNT % (X_SIZE / 2); // (char) *TIM6_CNT gets a random number 
        y_try = 7 + (char) *TIM6_CNT % (Y_SIZE / 2);
        ready = true;
        for (int i=0; i<snake->length && ready; ++i)
        {
            if ((x_try >= snake->body_part[i].posx - 2  && x_try <= snake->body_part[i].posx + 2) && (y_try >= snake->body_part[i].posy - 2 && y_try <= snake->body_part[i].posy + 2))
                ready = false;
        }
    }
    apple->x = x_try;
    apple->y = y_try;
}

void snake_turn(snake_t *snake) 
{
	draw_clear_snake(snake, 1);

	char input = keyb(); // fetch the key value to input
	if (input == 2 && snake->dir != DOWN)
	{
		snake->dir = UP;
	}
	else if (input == 4 && snake->dir != RIGHT)
	{
		snake->dir = LEFT;
	}
	else if (input == 8 && snake->dir != UP)
	{
		snake->dir = DOWN;
	}
	else if (input == 6 && snake->dir != LEFT)
	{
		snake->dir = RIGHT;
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
            snake->body_part[0].posy -= 5;
            break;
        case DOWN:
            snake->body_part[0].posy += 5;
            break;
        case LEFT:
            snake->body_part[0].posx -= 5;
            break;
        default:
            snake->body_part[0].posx += 5;
            break;
        }
    
}

bool snake_eat_apple(apple_t *apple, snake_t *snake) {
    if ((apple->x >= snake->body_part[0].posx - 3  && apple->x <= snake->body_part[0].posx + 3) && (apple->y >= snake->body_part[0].posy - 3 && apple->y <= snake->body_part[0].posy + 3))
    {
        // Can't increase the length of the snake.
		draw_clear_apple(apple, 1);
        apple_new(apple, snake);
        snake->length++;
        int new = snake->length - 1;

		draw_clear_apple(apple, 0);
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
    if (snake->body_part[0].posx + 2 >= X_SIZE || snake->body_part[0].posx + 2 <= 0 || snake->body_part[0].posy + 2 <= 5 || snake->body_part[0].posy + 2 >= Y_SIZE)
		return true;
    return false;
}

void init_snake(snake_t *snake)
{
	snake->length = 2;
	snake->body_part[0].posx = 70;
	snake->body_part[0].posy = 27;
	snake->body_part[1].posx = 65;
	snake->body_part[1].posy = 27;
	snake->dir = RIGHT;
}

void printEatenApples(){
    ascii_gotoxy(14,1);
    count_eaten_apples++;

    
    // fungerar bara upp till 99 sen börjar det om från 0
    if (count_eaten_apples > 9) 
	{
        firstD++;
		count_eaten_apples = 0;
    }

    lastD = '0' + count_eaten_apples;
    ascii_write_char('0' + firstD);
    ascii_write_char(lastD);
}

void draw_game(snake_t *snake, apple_t *apple)
{
	draw_clear_snake(snake, 0);
	draw_clear_apple(apple, 0);
}

void print_text(char text[])
{
	ascii_command(0x01); 
	char *s = text;
	while (*s)
	{
		ascii_write_char(*s++);
	}
	ascii_gotoxy(1, 2);
}

void write_logo(void)
{
	graphic_clear_screen();
	for (int i = 0; i < 592; i++)
	{
		graphic_pixel_set(s_start[i].x, s_start[i].y);
	}
	delay_micro(300);
}

void new_game(void)
{
	count_eaten_apples = 0;
	firstD = 0;
	
	bool snake_dead = false;
	snake_t snake;

	graphic_initalize();
	graphic_clear_screen();
	ascii_init();
	print_text("Eaten apples: ");

	init_snake(&snake);
	apple_t apple;
    apple_new(&apple, &snake);
	draw_game(&snake, &apple);
	
	while (!snake_dead) 
	{
		
        snake_turn(&snake);
        snake_move(&snake);
		draw_game(&snake, &apple);		
		 if (snake_eat_apple(&apple, &snake)) 
		{
			printEatenApples();
        }
        snake_dead = snake_hit_wall(&snake) || snake_hit_self(&snake);    
		delay_micro(150); 
    }
	print_text("Game Over!");
}

int stop_game(void)
{
	print_text("Play game? (1/0)");
	while (1)
	{
		char input = keyb();
		if (input == 1 || input == 0)
			return input;
	}
	delay_micro(500);
}
// ------------------------------------------------------- GAME ------------------------------------------------------------------------------- //

void main(void) 
{
	init_app();
	write_logo();
	while (stop_game())
	{
		new_game();
	}
	print_text("See you next time!");
	
}
