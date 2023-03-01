/// main.c

#include "debug.h"
#include "startup.h"
#include <stdlib.h>

/*
	TIM6 is one of the simplest counter module MD407 have. It lacks I/O support and counts upwards, from 1 to 0xFFFF.
	That means "*TIM6_ARR = x" sets the boundary of the random number.
	The speed of the counter is based on the processors clock speed. 
*/

// In startup.c are functions defined that is built in for processor MD407.
// Timer 6
#define TIM6_CR1 ((volatile unsigned short *) 0x40001000)
#define TIM6_CNT ((volatile unsigned short *) 0x40001024)
#define TIM6_ARR ((volatile unsigned short *) 0x4000102C)
#define UDIS (1<<1)
#define CEN (1<<0)

#define PORT_D 0x40020C00
#define D_MODER ((volatile unsigned int *) PORT_D)
#define ODR_LOW ((volatile unsigned char *) (PORT_D+0x14))




// Functions
void init_app(void);
void timer6_init(void);


void main(void)
{
	char random = 0;
	init_app();
	//timer6_init();
	while (1)
	{
		random = (char) *TIM6_CNT;
		*ODR_LOW = random;
	}
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
	*TIM6_CR1 &= ~CEN; // Stops the counter module
	*TIM6_ARR = 0xFFFF; // If its set to a low number, the random number will always be the upper bound because it counts to fast.
	*TIM6_CR1 |= ( CEN | UDIS); // Activates the counter module and disables "update event"
}
