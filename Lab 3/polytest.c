
typedef struct polygonpoint
{
	char x,y;
	struct polygonpoint *next;
} POLYPOINT, *PPOLYPOINT;

void draw_polygon(PPOLYPOINT polygon);

void main(void)
{
	graphic_initalize();
	graphic_clear_screen();
	while(1)
	{
		// Resetting the values everytime it runs because sometimes it might change some values. 
		POLYPOINT pg8 = {20, 20, 0};
		POLYPOINT pg7 = {20, 55, &pg8};
		POLYPOINT pg6 = {70, 60, &pg7};
		POLYPOINT pg5 = {80, 35, &pg6};
		POLYPOINT pg4 = {100, 25, &pg5};
		POLYPOINT pg3 = {90, 10, &pg4};
		POLYPOINT pg2 = {40, 10, &pg3};
		POLYPOINT pg1 = {20, 20, &pg2};
		while (1)
		{
			draw_polygon(&pg1);
			delay_milli(2);
			//graphic_clear_screen();
		}
		
	}	

}

void draw_polygon(PPOLYPOINT polygon)
{
	POLYPOINT p0;
	p0.x = polygon->x;
	p0.y = polygon->y;
	PPOLYPOINT ptr = polygon->next;
	while (ptr != 0) 
	{
		POLYPOINT p1;
		p1.x = ptr->x;
		p1.y = ptr->y;
		LINE side = {{p0.x, p0.y}, {p1.x, p1.y}};
		draw_line(&side);
		p0.x = p1.x; p0.y = p1.y;
		ptr = ptr->next;
	}
}