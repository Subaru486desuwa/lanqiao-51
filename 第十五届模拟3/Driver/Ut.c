#include <Ut.h>

sbit Tx=P1^0;
sbit Rx=P1^1;

/*初始化方波信号*/
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

/*读取距离*/
unsigned char Ut_Wave_Data()
{
	unsigned int time;
	CMOD=0x00;
	CH=CL=0;
	CR=1;//开始计时
	Ut_Wave_Init();
	while((Rx==1)&&(CF==0));
	if(CF==0)
	{
		time=CH<<8|CL;
		return (time*0.017);
	}
	else//溢出
	{
		CH=CL=0;
		CF=0;
		return 0;
	}
}