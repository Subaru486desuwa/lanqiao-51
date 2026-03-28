#include "reg52.h"
void LEDrunning()
{
	P2=((P2&0x1f)|0x80);
	P0=0xff;
}
void Delay(unsigned int t)
{
	while(t--);
	while(t--);
}


 
void main()
{
	while(1)
	{
			LEDrunning();
			Delay(60000);
	}
}

