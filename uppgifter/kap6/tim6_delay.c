/// main.c

#include "debug.h"
#include "startup.h"
#include <stdlib.h>

/*
	TIM6 is one of the simplest counter module MD407 have. It lacks I/O support and counts upwards, from 1 to 0xFFFF.
	That means "*TIM6_ARR = x" sets the boundary of the random number.
	The speed of the counter is based on the processors clock speed. 

	Interrupts from periferal circuits is controlled by the NVIC module. It handles 81 different interrupts. 
	In the vector table the vector number and its offset is listed. 

	The bit that needs to be set is calculated like this:
		x = vector_number
		bit = vector_number - (32 * x)
	This bit is then set on the second register of the module. 

*/

// In startup.c are functions defined that is built in for processor MD407.
// Timer 6
#define TIM6_CR1 ((volatile unsigned short *) 0x40001000)
#define TIM6_CNT ((volatile unsigned short *) 0x40001024)
#define TIM6_ARR ((volatile unsigned short *) 0x4000102C)
#define TIM6_SR ((volatile unsigned short *) 0x40001010)
#define TIM6_PSC ((volatile unsigned short *) 0x40001028)
#define TIM6_DIER ((volatile unsigned short *) 0x4000100C)
#define UDIS (1<<1)
#define CEN (1<<0)
#define UIF (1<<0)
#define UIE (1<<0)
#define OPM (1<<3)

#define PORT_D 0x40020C00
#define D_MODER ((volatile unsigned int *) PORT_D)
#define ODR_LOW ((volatile unsigned char *) (PORT_D+0x14))
#define ODR_HIGH ((volatile unsigned char *) (PORT_D+0x15))

#define NVIC_TIM6_ISER ((volatile unsigned int *) 0xE000E104) // second register of NIVC module 
#define NVIC_TIM6_IRQ_BPOS (1<<22) // bit set to activate the timer. 
#define TIM6_IRQVEC ((void (**)(void)) 0x2001C118)

// Functions
void init_app(void);
void timer6_interrupt_handler(void);
void delay(unsigned int count);
void timer6_init(void);
void timer6_interrupt(void);


// Variables 
static volatile int flag;
static volatile int delay_count;
volatile int ticks;
volatile int seconds;

void main(void)
{
	init_app();
	*ODR_LOW = 0;
	*ODR_HIGH = 0;
	delay(1000);
	*ODR_HIGH = 0x01;
	*ODR_LOW = 0xFF;
	while(1)
	{
		if (flag)
			break;
		//insert non blocking test
		else
		{
			unsigned char c = *ODR_HIGH << 1;
			if (c == 128)
				c = 1;
			*ODR_HIGH = c;
			
		}

	}
	*ODR_LOW = 0;
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
	*D_MODER = 0x55555555;	
	
}

void timer6_init(void)
{
	ticks = 0;
	seconds = 0;
	*TIM6_CR1 &= ~CEN;
	*TIM6_IRQVEC = timer6_interrupt;
	*NVIC_TIM6_ISER |= NVIC_TIM6_IRQ_BPOS;

	// Sets time base to 0.1 s 
	*TIM6_PSC = 839;
	*TIM6_ARR = 9999;
	*TIM6_DIER |= UIE;
	*TIM6_CR1 |= CEN;
}

void timer6_interrupt_handler(void)
{
	*TIM6_SR &= ~UIF; //
	delay_count--;
	if (delay_count > 0)
		return;
	*TIM6_CR1 &= ~CEN; // Deactivates TIM&
	flag = 1;
}

void delay(unsigned int count)
{
	if (count == 0)
		return;
	delay_count = count;
	flag = 0;
	*TIM6_CR1 &= ~CEN;

	// Sets the interrrupt handler 
	*TIM6_IRQVEC = timer6_interrupt_handler;

	// Makes it possible for interrupts
	*NVIC_TIM6_ISER |= NVIC_TIM6_IRQ_BPOS;

	// Sets time base to 1 ms
	*TIM6_PSC = 83;
	*TIM6_ARR = 999;

	// Enables interrupts
	*TIM6_DIER |= UIE;

	// Activates the timer 
	*TIM6_CR1 |= CEN; // in the book it wants it be ( CEN | OPM), but then it will only count down once. 

}

void timer6_interrupt(void)
{
	*TIM6_SR &= ~UIF;
	ticks++;
	if (ticks > 9)
	{
		ticks = 0;
		seconds++;
	}
}
