void draw_rectangle(PRECT r);

void main(void)
{
	graphic_initalize();
	graphic_clear_screen();
	while(1)
	{
		// Resetting the values everytime it runs because sometimes it might change some values. 
		RECT rectangles[] = {
			{10, 10, 20, 10},
			{25, 25, 10, 20},
			{40, 30, 70, 20},
			{60, 35, 10, 10},
			{70, 10, 5, 5},
		};
		for (int i = 0; i < sizeof(rectangles)/sizeof(RECT); i++)
		{
			draw_rectangle(&rectangles[i]);
			delay_milli(1);
		}
		graphic_clear_screen();
	}	

}

void draw_rectangle(PRECT r)
{
	POINT start;
	POINT end;
	LINE side;
	start.x = r->p.x; start.y = r->p.y; end.x = r->p.x + r->x; end.y = r->p.y; side.p0 = start; side.p1 = end; draw_line(&side);
	start.x = r->p.x + r->x; start.y = r->p.y; end.x = r->p.x + r->x; end.y = r->p.y + r->y; side.p0 = start; side.p1 = end; draw_line(&side);
	start.x = r->p.x + r->x; start.y = r->p.y + r->y; end.x = r->p.x; end.y = r->p.y + r->y; side.p0 = start; side.p1 = end; draw_line(&side);
	start.x = r->p.x; start.y = r->p.y + r->y; end.x = r->p.x; end.y = r->p.y; side.p0 = start; side.p1 = end; draw_line(&side);
}