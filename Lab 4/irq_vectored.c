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
#define PORT_D 0x40020C00
#define PORT_E 0x40021000
#define D_MODER ((volatile unsigned int *) PORT_D)
#define ODR_LOW ((volatile unsigned char *) (PORT_D+0x14))
#define ODR_HIGH ((volatile unsigned char *) (PORT_D+0x15))
#define E_ODR_LOW ((volatile unsigned char *) (PORT_E+0x14))
#define E_IDR_LOW ((volatile unsigned char *) (PORT_E+0x10))

// IRF_VECTOR 
#define SYSCFG_BASE ((volatile unsigned int *) 0x40013800)
#define SYSCFG_EXTICR1 ((volatile unsigned int *) 0x40013808)
#define EXTI_IMR ((unsigned int *) 0x40013C00)
#define EXTI_RTSR ((unsigned int *) 0x40013C08)
#define EXTI_FTSR ((unsigned int *) 0x40013C0C)
#define EXTI_PR ((volatile unsigned int *) 0x40013C14)
#define EXTI3_IRQVEC ((void (**)(void)) 0x2001C064)
#define EXTI2_IRQVEC ((void (**)(void)) 0x2001C060)
#define EXTI1_IRQVEC ((void (**)(void)) 0x2001C05C)
#define EXTI0_IRQVEC ((void (**)(void)) 0x2001C058)
#define NVIC_ISER0 ((volatile unsigned int *) 0xE000E100)  
#define NVIC_EXTI3_IRQ_BPOS (1<<9)
#define NVIC_EXTI2_IRQ_BPOS (1<<8)
#define NVIC_EXTI1_IRQ_BPOS (1<<7)
#define NVIC_EXTI0_IRQ_BPOS (1<<6)
#define EXTI3_IRQ_BPOS (1<<3)
#define EXTI2_IRQ_BPOS (1<<2)
#define EXTI1_IRQ_BPOS (1<<1)
#define EXTI0_IRQ_BPOS (1<<0)

// Functions
void init_app(void);
void exit2_handler(void);
void exit1_handler(void);
void exit0_handler(void);

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

	// Connect PE3 to EXTI 0 to 3
	*((unsigned int *) SYSCFG_EXTICR1) = 0x0444;

	*EXTI_IMR |= EXTI2_IRQ_BPOS | EXTI1_IRQ_BPOS | EXTI0_IRQ_BPOS; // Activates EXTIS
	*EXTI_RTSR |= EXTI2_IRQ_BPOS | EXTI1_IRQ_BPOS | EXTI0_IRQ_BPOS;; // Activates for rising flag
	*EXTI_FTSR &= ~EXTI2_IRQ_BPOS | ~EXTI1_IRQ_BPOS | ~EXTI0_IRQ_BPOS;; // Masks negative flag

	*EXTI2_IRQVEC = exit2_handler; // sets up the interrupt handler
	*EXTI1_IRQVEC = exit1_handler; 
	*EXTI0_IRQVEC = exit0_handler; 

	*NVIC_ISER0 |= NVIC_EXTI2_IRQ_BPOS | NVIC_EXTI1_IRQ_BPOS | NVIC_EXTI0_IRQ_BPOS;
	
}

void exit2_handler(void)
{
	if (*EXTI_PR & EXTI2_IRQ_BPOS)
	{
		*EXTI_PR |= EXTI2_IRQ_BPOS;
		if (*ODR_LOW)
			count = 0;
		else
			count = 0xFF;
	}
}
void exit1_handler(void)
{
	if (*EXTI_PR & EXTI1_IRQ_BPOS)
	{
		count = 0;
		*EXTI_PR |= EXTI1_IRQ_BPOS;
	}

}
void exit0_handler(void)
{
	if (*EXTI_PR & EXTI0_IRQ_BPOS)
	{
		*EXTI_PR |= EXTI0_IRQ_BPOS;
		count++;
	}
}

