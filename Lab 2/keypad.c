/*
 *    startup.c
 *
 */
#define GPIO_D 0x40020C00
#define GPIO_ODR_HIGH ((volatile unsigned char *)   (GPIO_D+0x15))
#define GPIO_ODR_LOW ((volatile unsigned char *)   (GPIO_D+0x14))
#define GPIO_IDR_HIGH ((volatile unsigned char *)   (GPIO_D+0x11))

void ActivateRow( unsigned int row )
{

    /* Aktivera angiven rad hos tangentbordet, eller

    * deaktivera samtliga */
    switch( row )
    {
    case 1: *GPIO_ODR_HIGH = 0x10; break;
    case 2: *GPIO_ODR_HIGH = 0x20; break;
    case 3: *GPIO_ODR_HIGH = 0x40; break;
    case 4: *GPIO_ODR_HIGH = 0x80; break;
    case 0: *GPIO_ODR_HIGH = 0x00; break;

    }

}

int ReadColumn( void )
{

    /* Om någon tangent (i aktiverad rad)

    * är nedtryckt, returnera dess kolumnnummer,

    * annars, returnera 0 */
    unsigned char c;
    c = *GPIO_IDR_HIGH;
    if ( c & 0x8 )
        return 4;
    if ( c & 0x4 )
        return 3;
    if ( c & 0x2 )
        return 2;
    if ( c & 0x1 )
        return 1;

    return 0;
}

unsigned char keyb(void)
{

    unsigned char key[]={1,2,3,0xA,4,5,6,0xB,7,8,9,0xC,0xE,0,0xF,0xD};

    int row, col;
    for(row=1; row <=4 ; row++ )
    {
        ActivateRow( row );
        if( (col = ReadColumn () ) )
        {
            ActivateRow( 0 );
            return key [4*(row-1)+(col-1) ];
        }
    }
    ActivateRow( 0 );
    return  0xFF;
}

void out7seg(unsigned char c)
{
    unsigned char character[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
    if (c >= 0 && c <= 15)
    {
        *GPIO_ODR_LOW = character[c];
    }
}

void app_init ( void )
{
    *((volatile unsigned long *) 0x40023830) = 0x18;
    *((volatile unsigned int *)0x40020C08) = 0x55555555; // MEDIUM SPEED
    * ( (volatile unsigned int *) 0x40020C00) &= 0x00000000; // MODER CONFIG
    * ( (volatile unsigned int *) 0x40020C00) |= 0x55005555; // MODER CONFIG
    * ( (volatile unsigned short *) 0x40020C04) &= 0x0000; // TYPER CONFIG
    * ( (volatile unsigned int *) 0x40020C0C) &= 0x00000000; // PUPDR CONFIG
    * ( (volatile unsigned int *) 0x40020C0C) |= 0x0000AAAA; // PUPDR CONFIG

}

void main(void)
{
    app_init();
    while(1){
    out7seg(keyb());
    }
}