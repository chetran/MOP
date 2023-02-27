/// main.c

#include "debug.h"
#include "startup.h"
#include <stdlib.h>

/*
The program works by setting the amount of micro seconds delay. 
For everytime the processor counts to one micro second an interrupt happoend and a handler will be called.
This goes on for all the micro seconds set. 
Deping on the if this is run on the hardware or simulation you may need to define "SIMULATION"
*/

// In startup.c are functions defined that is built in for processor MD407.
//#define SIMULATOR
#define PORT_D 0x40020C00
#define D_MODER ((volatile unsigned int *) PORT_D)
#define ODR_LOW ((volatile unsigned char *) (PORT_D+0x14))
#define ODR_HIGH ((volatile unsigned char *) (PORT_D+0x15))
#define SCB_VTOR ((volatile unsigned long *)0xE000ED08)

#define STK_CTRL (volatile unsigned int *) 0xE000E010 
#define STK_LOAD (volatile unsigned int *) 0xE000E014 
#define STK_VAL (volatile unsigned int *) 0xE000E018 


// Settings
#ifdef SIMULATOR
#define DELAY_COUNT 100
#else
#define DELAY_COUNT 10000
#endif

// Functions
void init_app_(void);
void systick_irq_handler();
void delay_micro(void);
void delay(unsigned int count);

// Global variables 
int systick_flag = 0;
int delay_count; 

void main(void)
{
	init_app_();
	*ODR_HIGH = 0;
	*ODR_LOW = 0;
	// Delay is in micro seconds.
	delay(DELAY_COUNT);
	*ODR_LOW = 0xFF; // NOT PART OF ASSIGNMENT
	*ODR_HIGH = 0x01;
	while(1)
	{
		if (systick_flag)
			break;
		// Just to make sure its not a blocking delay by having D ports ODR HIGH do something
		// NOT PART OF ASSIGNMENT
		else
		{
			unsigned char c = *ODR_HIGH << 1;
			if (c == 128)
				c = 1;
			*ODR_HIGH = c;
			
		}
	}
	*ODR_HIGH = 0xFF; // NOT PART OF ASSIGNMENT
	*ODR_LOW = 0;



}

void init_app_(void)
{
	// Need to intials the outport 
	*((unsigned long *) 0x40023830) = 0x18;

	// Initialize port D for display usage
	*D_MODER = 0x55555555;

	// Relocates the vector table
	*SCB_VTOR = 0x2001C000;

	// Now that its relocated we can go to this offset to set our Systick handler so whenever systick goes off this function is called
	*((void (**)(void)) 0x2001C03C) = systick_irq_handler;
}

void delay_micro(void)
{
	systick_flag = 0;
	*STK_CTRL = 0; // Resets SysTick
	*STK_LOAD = 168 - 1; // Minus 1 comes from how to processor counts 
	*STK_VAL = 0; // Resets counter register 
	*STK_CTRL = 7; // 7 equals 111 in binary, the first activates the clock, second one activates an interrrupt when its done counting, third not sure
}

void delay(unsigned int count)
{
	delay_count = count;
	delay_micro(); // Starts the first count down. 
}

void systick_irq_handler(void)
{
	*STK_CTRL = 0; // Resets clock 
	delay_count--;
	if (delay_count)
	{
		delay_micro();
	}
	else
	{
		systick_flag = 1; 
	}
}