void init_app(void)
{
	// Need to intials the outport 
	*((unsigned long *) 0x40023830) = 0x18;
	
	// Interrupt
	*SYSCFG_EXTICR1 &= 0xF0FF;
	*SYSCFG_EXTICR1 |= 0x0400; // Vill sätta trdje byten med e register hence 4 == 0100

	*EXTI_IMR |= (1 << 2); // aktivera abrottslinan 
	
	*EXTI_RTSR |=  (1 << 2); // välj avbrott vid postivit eller negatic flan k i exti rtsr/exti ftsr 

	*((void (**)(void))(0x2001C0000 + 0x60)) = &my_irq_handler; // sätt avbrottshanterare i vektortabllen.

	*NVIC_ISER0 |= (1 << 8);

}

void my_irq_handler(void)
{
	// tänd diodramp på port D 
	*GPIO_D_MODER = 0x00005555;
	*GPIO_D_ODR_LOW = 0xFF;
}
