#include <Ut.h>

sbit Tx=P1^0;
sbit Rx=P1^1;

void Delay12us()		//@12.000MHz
{
	unsigned char i;

	i = 38;
	while (--i);
}

void Ut_init()
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
		Tx=1;
		Delay12us();
		Tx=0;
		Delay12us();
	}
}

unsigned char Ut_Wave_Data(void)
{
	unsigned int time;
	CMOD=0X00;
	CH=CL=0;
	CR=1;//开始计时
	Ut_init();
	while((Rx==1)&&(CF==0));
	if(CF==0)
	{
		time=CH<8|CL;
		return time*0.017;
	}
	else
	{
		CH=CL=0;
		CF=0;
		return 0;
	}
}