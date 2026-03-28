#include <Ut.h>
#include <intrins.h>

sbit Tx = P1^0;
sbit Rx = P1^1;

void Delay12us()		//@12.000MHz
{
	unsigned char i;

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
	TMOD &=0x0f;//定时器1计时
	TH1=TL1=0;	//复位计数值
	Ut_Wave_Init();
	TR1=1;//开始计时
	while((Rx==1)&&(TF1==0));	//等待接收信号或定时器溢出	卡住程序
		TR1=0;//停止计时
	if(TF1==0)
	{
		time=TH1<<8|TL1;
		return(time*0.017);
	}
	else
	{
		TF1=0;//清除溢出标志位
		return 0;
	}
}