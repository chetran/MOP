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
#define PORT_E 0x40021000
#define D_MODER ((volatile unsigned int *) PORT_D)
#define ODR_LOW ((volatile unsigned char *) (PORT_D+0x14))
#define ODR_HIGH ((volatile unsigned char *) (PORT_D+0x15))
#define E_ODR_LOW ((volatile unsigned char *) (PORT_E+0x14))
#define E_IDR_LOW ((volatile unsigned char *) (PORT_E+0x10))

#define NVIC_TIM6_ISER1 ((volatile unsigned int *) 0xE000E100)  
#define NVIC_TIM6_ISER2 ((volatile unsigned int *) 0xE000E104) // second register of NIVC module 
#define NVIC_TIM6_IRQ_BPOS (1<<22) // bit set to activate the timer. 
#define TIM6_IRQVEC ((void (**)(void)) 0x2001C118)

#define EXTI3_IRQVEC ((void (**)(void)) 0x2001C064)
#define SYSCFG_EXTICR1 0x40013808
#define EXTI_SWIER ((volatile unsigned int *) 0x40013C10)
#define EXTI_PR ((volatile unsigned int *) 0x40013C14)


// Functions
void init_app(void);
void exti3_handler(void);

// Variables 
volatile int count = 0;

void main(void)
{
	init_app();
	while(1)
	{
		*ODR_LOW = count;
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

	// Connect PE3 to EXTI3
	*((unsigned int *) SYSCFG_EXTICR1) &= ~0xF000;
	*((unsigned int *) SYSCFG_EXTICR1) |= 0x4000; // Configures PE3 to EXTI3

	*((unsigned int *) 0x40013C00) |= 8; // Activates EXTI3
	*((unsigned int *) 0x40013C08) |= 8; // Activates for rising flag
	*((unsigned int *) 0x40013C0C) &= ~8; // Masks negative flag

	*EXTI3_IRQVEC = exti3_handler; // sets up the interrupt handler

	*NVIC_TIM6_ISER1 |= (1<<9);
	
}

void exti3_handler(void)
{
	if (*EXTI_PR & 8)
	{
		*EXTI_PR |= 8;
		if (*E_IDR_LOW & 1)
		{
			*E_ODR_LOW |= (1<<4);
			count++;
		}
		if (*E_IDR_LOW & 2)
		{
			*E_ODR_LOW |= (1<<5);
			count = 0;
		}
		if (*E_IDR_LOW & 4)
		{
			*E_ODR_LOW |= (1<<6);
			if (*ODR_LOW)
				count = 0;
			else
				count = 0xFF;
		}
		
	}
}
