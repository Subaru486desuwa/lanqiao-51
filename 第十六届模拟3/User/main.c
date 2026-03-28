/*头文件声明区*/
#include <STC15F2K60S2.H>
#include <LED.h>
#include <Seg.h>
#include <Key.h>
#include <iic.h>

/*变量声明区*/
unsigned char Key_Val,Key_Down,Key_Old;
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Seg_Pos;
unsigned char Point[8]={0,0,0,0,0,0,0,0};
unsigned char ucLED[8]={0,0,0,0,0,0,0,0};
unsigned char Key_Slow_Down;
unsigned char Seg_Slow_Down;

unsigned char Seg_Dis_Mode;//0-湿度 1-参数 2-时间

float Voltage;
float Humi;//湿度

unsigned char Humi_Default=50;//湿度参数

unsigned char Time=3;

bit Relay_Status;//继电器状态 0-关闭 1-开启

bit Relay_Flag;//1-开启
bit Relay_Trigger;

unsigned int Trigger; //1000-10000ms

/*湿度处理函数*/
float Humi_Proc(float Voltage)
{
	if(Voltage>1 && Voltage<4)
	{
		return 80*(Voltage-1.0)/3.0+10;		
	}
	else if(Voltage<=1)
		return 10;
	else
		return 90;
}

/*信息处理函数*/
void Seg_Proc()
{
	if(Seg_Slow_Down)	return;
		Seg_Slow_Down=1;
	
	
	
	if(Seg_Dis_Mode==0)
	{
		Voltage=Ad_Read(0x43)/51.0;
		Humi=Humi_Proc(Voltage);
		Seg_Buf[0]=11;
		Seg_Buf[6]=(unsigned char)Humi/10%10;
		Seg_Buf[7]=(unsigned char)Humi%10;
	}
	
	else if(Seg_Dis_Mode==1)
	{
		Seg_Buf[0]=12;
		Seg_Buf[6]=Humi_Default/10%10;
		Seg_Buf[7]=Humi_Default%10;
	}
	
	else
	{
		Seg_Buf[0]=13;
		Seg_Buf[6]=Time/10%10;
		Seg_Buf[7]=Time%10;
	}
	
	if(Seg_Buf[6]==0)	Seg_Buf[6]=10;	//高位熄灭
}

/*按键处理函数*/
void Key_Proc()
{
	if(Key_Slow_Down)	return;
		Key_Slow_Down=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Val^Key_Old);
	Key_Old=Key_Val;
	
	switch(Key_Down)
	{
		case 4:
			if(++Seg_Dis_Mode==3)
				Seg_Dis_Mode=0;
		break;
			
		case 5:
			Relay_Status^=1;
			Relay_Trigger=0;
		break;
		
		case 8:
			Relay_Trigger=0;		//继电器吸合刷新
			if(Seg_Dis_Mode==1)
			{
				Humi_Default-=5;
				if(Humi_Default==25)
					Humi_Default=90;
			}
			else if(Seg_Dis_Mode==2)
			{
				if(--Time==0)
					Time=10;
			}
		break;
			
		case 9:
			Relay_Trigger=0;
				if(Seg_Dis_Mode==1)
			{
				Humi_Default+=5;
				if(Humi_Default==95)
					Humi_Default=30;
			}
			else if(Seg_Dis_Mode==2)
			{
				if(++Time==11)
					Time=1;
			}
		break;
	}
}

/*LED处理函数*/
void LED_Proc()
{
	ucLED[0]=(Seg_Dis_Mode==0);
	ucLED[1]=(Seg_Dis_Mode==1);
	ucLED[2]=(Seg_Dis_Mode==2);
	ucLED[7]=Relay_Status;
	
	if(Relay_Status==1)
	{
		if(Humi<Humi_Default &&	Relay_Trigger==0)
		{
			Relay_Flag=1;	//判断是否需要吸合 1-吸合过了 0-吸合
		}

	}
	else if(Relay_Status==0)
	{
			Relay_Flag=0;
	}
	
	Relay(Relay_Flag);
}

/*定时器0初始化函数*/
void Timer0Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0x18;		//设置定时初值
	TH0 = 0xFC;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0=1;
	EA=1;
}

/*定时器0中断服务函数*/
void Timer0Server() interrupt 1
{
	if(++Key_Slow_Down==10)
		Key_Slow_Down=0;
	
	if(++Seg_Slow_Down==100)
		Seg_Slow_Down=0;
	
	if(Relay_Flag==1)
	{
		++Trigger;
		if(Trigger==Time*1000)
		{
			Trigger=0;
			Relay_Flag=0;
			Relay_Trigger=1;		//不需要二次吸合
		}
	}
	
	if(++Seg_Pos==8)
		Seg_Pos=0;
	
	Seg_Dis(Seg_Pos,Seg_Buf[Seg_Pos],Point[Seg_Pos]);
	LED_Dis(Seg_Pos,ucLED[Seg_Pos]);
}

/*main*/
void main()
{
	Timer0Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		LED_Proc();
	}
}
