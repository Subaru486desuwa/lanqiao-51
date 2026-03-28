/*头文件声明区*/
#include <STC15F2K60S2.H>
#include <onewire.h>
#include <Seg.h>
#include <Key.h>
#include <LED.h>

/*变量声明区*/
unsigned char Key_Slow_Down;
unsigned char Seg_Slow_Down;
unsigned char Key_Val,Key_Down,Key_Old;
unsigned char Seg_Pos;
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Point[8]={0,0,0,0,0,0,0,0};
unsigned char ucLED[8]={0,0,0,0,0,0,0,0};

unsigned int Timer1000Ms;
unsigned int Freq; //0-65535
unsigned int Cycle;

float temperature;

bit Seg_Dis_Mode;//0-频率 1-温度
bit Signal_Dis_Mode;//0-频率 1-周期
bit Temper_Dis_Mode;//0-有小数点 1-无

bit Trigger_Flag;//L3
bit LED_Flag;
unsigned int LED_Timer;//1000ms计时

unsigned char Timer100Ms;

bit Trigger;//L8
bit L8_Flag;

/*信息处理函数*/
void Seg_Proc()
{
	if(Seg_Slow_Down)	return;
		Seg_Slow_Down=1;
	
	temperature=Read_Temperature();
	
	if(Seg_Dis_Mode==0)
	{
		unsigned char i=3;
		
		if(Signal_Dis_Mode==0)
		{
			Seg_Buf[0]=11;
			Seg_Buf[3]=Freq/10000%10;
			Seg_Buf[4]=Freq/1000%10;
			Seg_Buf[5]=Freq/100%10;
			Seg_Buf[6]=Freq/10%10;
			Seg_Buf[7]=Freq%10;		
			Point[6]=0;			
		}
		else
		{
			Cycle=1000000/Freq;
			Seg_Buf[0]=11;
			Seg_Buf[3]=Cycle/10000%10;
			Seg_Buf[4]=Cycle/1000%10;
			Seg_Buf[5]=Cycle/100%10;
			Seg_Buf[6]=Cycle/10%10;
			Seg_Buf[7]=Cycle%10;
			Point[6]=0;	
		}
		
		while(Seg_Buf[i]==0)	//高位熄灭
		{
			Seg_Buf[i]=10;
			i++;
			if(i==7) break;
		}
	}
	
	else	//温度
	{
		if(Temper_Dis_Mode==0)
		{
			Seg_Buf[0]=12;
			Seg_Buf[3]=10;
			Seg_Buf[4]=10;
			Seg_Buf[5]=(unsigned char)temperature/10%10;
			Seg_Buf[6]=(unsigned char)temperature%10;
			Point[6]=1;	
			Seg_Buf[7]=(unsigned int)(temperature*10)%10;			
		}
		else
		{
			Seg_Buf[0]=12;
			Seg_Buf[3]=10;
			Seg_Buf[4]=10;
			Seg_Buf[5]=10;
			Point[6]=0;
			Seg_Buf[6]=(unsigned char)temperature/10%10;
			Seg_Buf[7]=(unsigned char)temperature%10;
		}
	}
	
	if(temperature>30)
	{
		Trigger=1;
	}
	else	Trigger=0;
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
			Seg_Dis_Mode^=1;
		break;
		
		case 5:
			if(Seg_Dis_Mode==0)
			{
				Signal_Dis_Mode^=1;
				Trigger_Flag=1;
			}
			if(Seg_Dis_Mode==1)
			{
				Temper_Dis_Mode^=1;
			}
		break;
			
		case 9:
			Seg_Dis_Mode=0;
			Signal_Dis_Mode=0;
			Temper_Dis_Mode=0;
		break;
	}
}

/*LED处理函数*/
void LED_Proc()
{
	ucLED[0]=(Seg_Dis_Mode==0);
	ucLED[1]=(Seg_Dis_Mode==1);
	ucLED[2]=(LED_Flag==1);
	ucLED[7]=L8_Flag&&Trigger;
}

/*定时器0初始化函数*/
void Timer0Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD &= 0xF0;		//设置定时器模式
	TMOD |=0X05;
	TL0 = 0x18;		//设置定时初值
	TH0 = 0xFC;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
}


/*定时器1初始化*/
void Timer1Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0xBF;		//定时器时钟12T模式
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0x18;		//设置定时初值
	TH1 = 0xFC;		//设置定时初值
	TF1 = 0;		//清除TF1标志
	TR1 = 1;		//定时器1开始计时
	ET1=1;
	EA=1;
}


/*定时器0终端服务函数*/
void Timer1Server() interrupt 3
{
	if(++Timer1000Ms==1000)
	{
		Timer1000Ms=0;
		Freq=TH0<<8|TL0;
		TH0=TL0=0;
	}
	if(++Key_Slow_Down==10)
		Key_Slow_Down=0;
	
	if(++Seg_Slow_Down==200)
		Seg_Slow_Down=0;
	
	if(Trigger_Flag==1)
	{
		++LED_Timer;
		LED_Flag=1;
	}
	
	if(LED_Timer==1000)
	{
		LED_Timer=0;
		LED_Flag=0;
		Trigger_Flag=0;
	}
	
	if(Trigger==1)
	{
		Timer100Ms++;
		if(Timer100Ms==100)
		{
			Timer100Ms=0;
			L8_Flag^=1;
		}
	}
	
	if(++Seg_Pos==8)
		Seg_Pos=0;
	Seg_Dis(Seg_Pos,Seg_Buf[Seg_Pos],Point[Seg_Pos]);
	LED_Dis(Seg_Pos,ucLED[Seg_Pos]);
}

/*delay850ms*/
void Delay850ms()		//@12.000MHz
{
	unsigned char i, j, k;

	i = 39;
	j = 195;
	k = 2;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

/*main函数*/
void main()
{
	Read_Temperature();
	Delay850ms();
	Timer0Init();
	Timer1Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		LED_Proc();
	}
}
