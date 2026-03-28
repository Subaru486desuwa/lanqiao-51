#include "STC15F2K60S2.H"
#include "intrins.h"
//ÑÓÊ±200ms
void delay()		//@11.0592MHz
{
    unsigned char i, j, k;

    _nop_();
    _nop_();
    i = 9;
    j = 104;
    k = 139;
    do
    {
        do
        {
            while (--k);
        }
        while (--j);
    }
    while (--i);
}

 void Delay200ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 9;
	j = 104;
	k = 139;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}


void delay_ms(unsigned char delay)//ÑÓÊ±º¯Êý¡ª¡ª¡ª¡ªms
{
  unsigned char i,j;
  while(delay--)
    for(i=100;i>0;i--)
      for(j=100;j>0;j--);
 
}