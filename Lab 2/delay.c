/// main.c

#include "debug.h"
#include "startup.h"

//#define SIMULATOR // Need to comment to deativate 
#define GPIO_E 0x40021000
#define GPIO_E_MODER ((volatile unsigned int *)  GPIO_E)
#define Bargraph *((volatile unsigned int *) (GPIO_E+0x14))
#define STK_CTRL (volatile unsigned int *) 0xE000E010 
#define STK_LOAD (volatile unsigned int *) 0xE000E014 
#define STK_VAL (volatile unsigned int *) 0xE000E018 

void delay_ns(void)
{
	// page 98
	*STK_CTRL = 0; // Resets SysTick
	*STK_LOAD = 168 / 4 - 1; // Minus 1 comes from how to processor counts 
	*STK_VAL = 0; // Resets counter register 
	*STK_CTRL = 5; // Starts the count down 
	while(*STK_CTRL & 0x00010000){} // while the countflag is 1 
	*STK_CTRL = 0; // resets Systick 
}

void delay_micro(unsigned int ms)
{	
	#ifdef SIMULATOR
		ms = ms / 1000;
		ms++;
	#endif

	// Mathematically 4 of these are 1 mili sec
	for (int i = 0; i < ms; i++)
	{
		delay_ns();
		delay_ns();
		delay_ns();
		delay_ns();
	}

}

void init_app(void)
{
	// Need to intials the outport 
	*((unsigned long *) 0x40023830) = 0x18;
	*GPIO_E_MODER &= 0xFFFF0000;
	*GPIO_E_MODER |= 0x00005555;
}

void main(void)
{
	init_app();
	while(1)
	{
		Bargraph = 0;
		delay_micro(500);
		Bargraph = 0xFF;
		delay_micro(500);
	}
}
