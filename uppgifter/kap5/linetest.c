typedef struct 
{
	unsigned char x,y;
} POINT, *PPOINT;

typedef struct
{
	POINT p0,p1;
} LINE, *PLINE;

int draw_line(PLINE l);
void swap(unsigned char *a, unsigned char *b);

void main(void)
{
	graphic_initalize();
	graphic_clear_screen();
	while(1)
	{
		// Resetting the values everytime it runs because sometimes it might change some values. 
		LINE lines[] = {
		{40, 10, 100, 10},
		{40, 10, 100, 20},
		{40, 10, 100, 30},
		{40, 10, 100, 40},
		{40, 10, 100, 50},
		{40, 10, 100, 60},
		{40, 10, 90, 60},
		{40, 10, 80, 60},
		{40, 10, 70, 60},
		{40, 10, 60, 60},
		{40, 10, 50, 60},
		{40, 10, 40, 60},
		};
		for (int i = 0; i < sizeof(lines)/sizeof(LINE); i++)
		{
			draw_line(&lines[i]);
			delay_milli(1);
		}
		graphic_clear_screen();
	}	

}

void swap(unsigned char *a, unsigned char *b)
{
	unsigned char temp = *a;
	*a = *b;
	*b = temp;
}

int draw_line(PLINE l)
{
	// This just checks so that there are no points outside of the display which is 128x64 
	if ((l->p0.x < 1 && l->p0.x > 64) | (l->p1.x < 1 && l->p1.x > 64) | (l->p0.y < 1 && l->p0.y > 128) | (l->p1.y < 1 && l->p1.y > 128))
		return 0;
	
	// Bresenhams Algorithm - honestly no idea how it works 
	char steep;
	if (abs(l->p0.y - l->p1.y) > abs(l->p0.x - l->p1.x))
		steep = 1;
	else
		steep = 0;
	if (steep)
	{
		swap(&l->p0.x, &l->p0.y);
		swap(&l->p1.x, &l->p1.y);
	}
	if (l->p0.x > l->p1.x)
	{
		swap(&l->p0.x, &l->p1.x);
		swap(&l->p0.y, &l->p1.y);
	}
	char deltax = l->p1.x - l->p0.x;
	char deltay = abs(l->p1.y - l->p0.y);
	char error = 0;
	char y = l->p0.y;
	char ystep;
	if (l->p0.y < l->p1.y)
		ystep = 1;
	else
		ystep = -1;
	for (int x = l->p0.x; x <= l->p1.x; x++)
	{
		if (steep)
			graphic_pixel_set(y, x);
		else
			graphic_pixel_set(x, y);
		error = error + deltay;
		if (2 * error >= deltax)
		{
			y = y +ystep;
			error = error - deltax;
		}
	}

	// Returns 1 if it succeded
	return 1;
}


