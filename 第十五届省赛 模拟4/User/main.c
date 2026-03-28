/*头文件声明区*/
#include <STC15F2K60S2.H>
#include <Seg.h>
#include <Key.h>
#include <LED.h>
#include <onewire.h>
#include <iic.h>

/*变量声明区*/
unsigned char Seg_Slow_Down;//数码管减速专用变量
unsigned char Key_Slow_Down;//按键减速专用变量

unsigned char Seg_Pos;
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};	//数码管数字存储数组
unsigned char	Point[8]={0,0,0,0,0,0,0,0};//数码管小数点存储数组

unsigned char ucLED[8]={0,0,0,0,0,0,0,0};//LED灯使能存储数组

unsigned char Key_Val,Key_Down,Key_Old;//按键消抖专用变量

unsigned int Freq;	//NE555频率	0-65535
unsigned int Timer_1000Ms;//1s

unsigned char Seg_Dis_Mode=0;	//0-信号 1-温度 2-参数

float temperature;

unsigned char Temperature_Set=25;//温度参数

bit LED_Flag;
bit Enable_Flag;
unsigned char Timer_100Ms;//0.1s

float Voltage_Output;//输出电压

/*信息处理函数*/
void Seg_Proc()
{

	if(Seg_Slow_Down)	return;
		Seg_Slow_Down=1;
	
	temperature = Read_Temperature();
	
	if(Seg_Dis_Mode==0)	//信号界面
	{
		unsigned char i=3;
		Seg_Buf[0]=11;	//P
		Seg_Buf[3]=Freq/10000%10;
		Seg_Buf[4]=Freq/1000%10;
		Seg_Buf[5]=Freq/100%10;
		Seg_Buf[6]=Freq/10%10;
		Seg_Buf[7]=Freq%10;
		
	
		while(Seg_Buf[i]==0)
		{
			Seg_Buf[i]=10;
			i++;
			if(i==7)	break;
		}
	}
	else if(Seg_Dis_Mode==1)	//温度界面
	{
		unsigned char i=5;
		
		Seg_Buf[0]=12;//C
		Seg_Buf[3]=10;
		Seg_Buf[4]=10;
		Seg_Buf[5]=(unsigned char)temperature/10 %10;
		Seg_Buf[6]=(unsigned char)temperature%10;
		Seg_Buf[7]=(unsigned int)(temperature*10)%10;\
		Point[6]=1;
		
		while(Seg_Buf[i]==0)
				{
					Seg_Buf[i]=10;
					i++;
					if(i==6)	break;
				}
	}
	else
	{
		Seg_Buf[0]=13;//U
		Seg_Buf[5]=10;
		Seg_Buf[6]=Temperature_Set/10%10;
		Seg_Buf[7]=Temperature_Set%10;
		Point[6]=0;
	}

}

/*按键处理函数*/
void Key_Proc()
{
	if(Key_Slow_Down)	return;
		Key_Slow_Down=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Val^Key_Old);	//按键消抖
	Key_Old=Key_Val;
	
	switch(Key_Down)
	{
		case 4:
			if(++Seg_Dis_Mode==3)
				Seg_Dis_Mode=0;
		break;
			
		case 9:
			if(Seg_Dis_Mode==2)	//处于参数模式
			{
				Temperature_Set++;
				if(Temperature_Set==36)
					Temperature_Set=20;
			}
		break;
			
		case 8:
			if(Seg_Dis_Mode==2)
			{
				Temperature_Set--;
				if(Temperature_Set==19)
					Temperature_Set=35;
			}
		break;
	}
}

/*DA输出电压处理函数*/
float Voltage(unsigned int Freq)
{
	if(Freq>=2000)
		return 4.5;
	else if(Freq<=200)
		return 0.5;
	else
	{
		return (Freq-200)*4/1800.0+0.5;
	}
}

/*LED处理函数*/
void LED_Proc()
{
	Buzz(0);
	
	Voltage_Output = Voltage(Freq);
	ucLED[0]=Seg_Dis_Mode==0;
	ucLED[1]=Seg_Dis_Mode==1;
	ucLED[7]=LED_Flag&&(temperature>=30);
	
	Enable_Flag=(temperature>=Temperature_Set)?1:0;
	Relay(Enable_Flag);
	
	Da_Write((unsigned char)(51 * Voltage_Output) );
}

/*NE555中断初始化函数*/
void Timer0Init(void)		//12.000MHz
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD &= 0xF0;		//设置定时器模式
	TMOD|=0X05;
	TL0 = 0x00;		//设置定时初值
	TH0 = 0x00;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
}


/*定时器1初始化函数*/
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


/*定时器1中断服务函数*/
void Timer1Server() interrupt 3
{
	if(++Seg_Slow_Down==200)
		Seg_Slow_Down=0;
	
	if(++Key_Slow_Down==10)
		Key_Slow_Down=0;
	
	if(++Timer_1000Ms==1000)
	{
		Timer_1000Ms=0;
		Freq=TH0<<8|TL0;
		TH0=TL0=0;
	}
	
	if(++Timer_100Ms==100)
	{
		TR0=0;
		Timer_100Ms=0;
		LED_Flag^=1;
		TR0=1;
	}
	
	if(++Seg_Pos==8)
		Seg_Pos=0;
	
	Seg_Dis(Seg_Pos,Seg_Buf[Seg_Pos],Point[Seg_Pos]);
	LED_Dis(Seg_Pos,ucLED[Seg_Pos]);
		
		
}

void Delay750ms()		//@12.000MHz
{
	unsigned char i, j, k;

	i = 35;
	j = 51;
	k = 182;
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
	Delay750ms();
	Timer0Init();
	Timer1Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		LED_Proc();
	}
}