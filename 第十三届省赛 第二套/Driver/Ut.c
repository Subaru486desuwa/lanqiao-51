#include <Ut.h>
#include <intrins.h>

sbit Tx = P1^0;
sbit Rx = P1^1;

void Delay12us()		//@12.000MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 38;
	while (--i);
}



void Ut_Wave_Init()	//超声波初始化函数 产生八个40khz方波信号
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
	unsigned int time;//时间存储变量
	CMOD=0X00;
	CH=CL=0;	//复位计数值
	Ut_Wave_Init();
	CR=1;//开始计时
	while((Rx==1)&&(CF==0));	//等待接收信号或定时器溢出	卡住程序
		CR=0;//停止计时
	if(CF==0)
	{
		time=CH<<8|CL;
		return(time*0.017);
	}
	else
	{
		CF=0;//清除溢出标志位
		return 0;
	}
}