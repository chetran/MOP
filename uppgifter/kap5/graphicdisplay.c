void main(void)
{
	int i;
	graphic_initalize();
	graphic_clear_screen();
	for (i = 1; i <= 128; i++)
		graphic_pixel_set(i, 10);
	for (i = 1; i <= 64; i++)
		graphic_pixel_set(10, i);
	delay_milli(1);
	for (i = 1; i <= 128; i++)
		graphic_pixel_clear(i ,10);
	for (i = 1; i <= 64; i++)
		graphic_pixel_clear(10, i);
}