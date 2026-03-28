#include <Ut.h>

sbit Tx=P1^0;
sbit Rx=P1^1;

void Delay12us()		//@12.000MHz
{
	unsigned char i;
	i = 38;
	while (--i);
}

void Ut_Wave_Init()
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

unsigned char Ut_Wave_Data()
{
	unsigned int time;
	
	CMOD=0X00;
	CH=CL=0;
	Ut_Wave_Init();
	CR=1;
	while((CF==0)&&(Rx==1));
	CR=0;
	if(CF==0)	//有效
	{
		time=CH<<8|CL;
		CH=CL=0;
		return time*0.017;
	}
	else
	{
		CH=CL=0;
		CF=0;
		return 0;
	}
}


