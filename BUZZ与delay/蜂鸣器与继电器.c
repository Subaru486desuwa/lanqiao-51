#include "reg52.h"
sbit HC138_A=P2^5;
sbit HC138_B=P2^6;
sbit HC138_C=P2^7;

void InitHC138(unsigned char n)	 //函数切换138译码器的输出低电平端口
{
	switch(n)
	{
		case 4:
		HC138_A=0;
		HC138_B=0;
		HC138_C=1;
		break;
		case 5:
		HC138_A=1;
		HC138_B=0;
		HC138_C=1;
		break;
		case 6:
		HC138_A=0;
		HC138_B=1;
		HC138_C=1;
		break;
		case 7:
		HC138_A=1;
		HC138_B=1;
		HC138_C=1;
		break;
	}
}

void Delay(unsigned int t)
{
	while(t--);
	while(t--);
}


void LEDrunning()
{	unsigned char i;
 InitHC138(4);
 for(i=0;i<3;i++)
 {
 P0=0x00;
 Delay(60000);
 Delay(60000);
 P0=0xff;
 Delay(60000);
 Delay(60000);
 }
 for(i=1;i<8;i++)
 {
 	P0=0xff<<i;
	Delay(60000);
 	Delay(60000);
 }
 for(i=1;i<8;i++)
 {
 	P0=~(0xff<<i);
	Delay(60000);
 	Delay(60000);
 }
}





void main()
{
	while(1)
	{
		LEDrunning();
	}
}
