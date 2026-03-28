/*头文件声明*/
#include <STC15F2K60S2.H>
#include <LED.h>
#include <Seg.h>
#include <Key.h>
#include <ds1302.h>
#include <iic.h>
#include <Ut.h>

/*变量声明区*/
unsigned char Key_Val,Key_Down,Key_Old;
unsigned char Key_Slow_Down;
unsigned char Seg_Slow_Down;
unsigned char Seg_Pos;
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Point[8]={0,0,0,0,0,0,0,0};
unsigned char ucLED[8]={0,0,0,0,0,0,0,0};

unsigned char ucRtc[3]={0x20,0x20,0x01};

bit Seg_Mode;	//0-数据显示 1-参数设置
unsigned char Seg_Dis_Mode;	//0-时间 1-距离数据 2-数据记录

bit Ut_Mode;//0-触发 1-定时

unsigned char Distance;//超声波测距

unsigned char Record_Mode;	//0-max 1-min 2-aver

bit Setting;	//0-时间 1-距离参数

unsigned char Time[5]={2,3,5,7,9};
unsigned char Time_Index;

unsigned char Distance_Setting[]={10,20,30,40,50,60,70,80};//距离参数 10-80
unsigned char Distance_Setting_Index=1;	//默认值20

unsigned char Time_Rtc;

unsigned int Timer1000Ms;//1s

unsigned int Count;//测量此数
unsigned int Sum_Distance;

unsigned char Distance_Max;
unsigned char Distance_Min;

float Average;

float Voltage;//光敏电阻电压
bit Voltage_Flag;//读取光敏电阻变化标志位

/*信息处理函数*/
void Seg_Proc()
{
	if(Seg_Slow_Down)	return;
		Seg_Slow_Down=1;
	
	/*信息获取*/
	Read_Rtc(ucRtc);
	
	Voltage=Ad_Read(0x41)/51.0;
	
	Time_Rtc=ucRtc[2]/16*10+ucRtc[2]%16;	//时钟模块的秒

	if(Timer1000Ms==0)	//1s判断一次
	{
		if((Time_Rtc%Time[Time_Index]==0)	&& Ut_Mode==1)	//定时模式下，能被整除	测量一次
		{
			Distance=Ut_Wave_Data();
			if(Distance>Distance_Max)
				Distance_Max=Distance;
			if(Distance<Distance_Min)
				Distance_Min=Distance;
			Sum_Distance+=Distance;
			Count++;
		}
		
		if(Ut_Mode==0&& Voltage>=3)
		{
			Voltage_Flag=1;
		}
		
		if(Voltage_Flag==1 && Voltage<1)
		{
				Voltage_Flag=0;
				Distance=Ut_Wave_Data();
				Sum_Distance+=Distance;
				Count++;
				if(Distance>Distance_Max)
					Distance_Max=Distance;
				if(Distance<Distance_Min)
					Distance_Min=Distance;
		}
	}
	
	if(Count!=0)
		Average=Sum_Distance/Count;	//平均
	
	/*数码管显示*/
	if(Seg_Mode==0)
	{
		Point[6]=0;
		if(Seg_Dis_Mode==0)
		{
			Seg_Buf[0]=ucRtc[0]/16;
			Seg_Buf[1]=ucRtc[0]%16;
			Seg_Buf[2]=11;
			Seg_Buf[3]=ucRtc[1]/16;
			Seg_Buf[4]=ucRtc[1]%16;
			Seg_Buf[5]=11;
			Seg_Buf[6]=ucRtc[2]/16;
			Seg_Buf[7]=ucRtc[2]%16;	
		}
		
		else if(Seg_Dis_Mode==1)	//处于测距
		{
			unsigned char i=5;
			
			Seg_Buf[0]=14;//L
			if(Ut_Mode==0)	Seg_Buf[1]=15;//C
			else Seg_Buf[1]=16;//F
			
			Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=10;
			
			Seg_Buf[5]=Distance/100%10;
			Seg_Buf[6]=Distance/10%10;
			Seg_Buf[7]=Distance%10;
			while(Seg_Buf[i]==0)		//高位熄灭
			{
				Seg_Buf[i]=10;
				i++;
				if(i==7)	break;
			}
		}
		
		else //记录
		{
			unsigned char i=4;
			while(Seg_Buf[i]==0)
			{
				Seg_Buf[i]=10;
				i++;
				if(i==7)	break;
			}
			
			Seg_Buf[0]=17;//H
			if(Record_Mode==0)
			{			
				Seg_Buf[1]=12;	//max
				Seg_Buf[5]=10;
				Seg_Buf[6]=Distance_Max/10%10;
				Seg_Buf[7]=Distance_Max%10;
				Point[6]=0;
			}
			else if(Record_Mode==2)
			{		
				Seg_Buf[1]=11;	//aver
				Seg_Buf[5]=(unsigned char)Average/10%10;
				Seg_Buf[6]=(unsigned char)Average%10;
				Seg_Buf[7]=(unsigned int)(Average*10)%10;
				Point[6]=1;
			}
			else if(Record_Mode==1)
			{			
				Seg_Buf[1]=13;	//min
				Seg_Buf[5]=10;
				Seg_Buf[6]=Distance_Min/10%10;
				Seg_Buf[7]=Distance_Min%10;
				Point[6]=0;
			}

		}
	}
	
	else	//参数设置
	{
		Seg_Buf[0]=18;//P
		Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=10;
		Point[6]=0;

		if(Setting==0)
		{	
			Seg_Buf[1]=1;
			Seg_Buf[6]=Time[Time_Index]/10%10;
			Seg_Buf[7]=Time[Time_Index]%10;
		}
		else
		{	
			Seg_Buf[1]=2;
			Seg_Buf[6]=Distance_Setting[Distance_Setting_Index]/10%10;
			Seg_Buf[7]=Distance_Setting[Distance_Setting_Index]%10;
		}

	}
	
	
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
			Seg_Mode^=1;
			Setting=0;	//切换界面默认采集时间设置
			Seg_Dis_Mode=0;//切换界面默认为时间显示
		break;
		
		case 5:
			if(Seg_Mode==0)
			{
				if(++Seg_Dis_Mode==3)
					Seg_Dis_Mode=0;
			}
			
			if(Seg_Dis_Mode==2)
				Record_Mode=0;	//进入记录界面，默认为最大值
				
			else
			{
				Setting^=1;
			}
		break;
			
		case 8:
			if(Seg_Mode==0 && Seg_Dis_Mode==1)	//处于测距
				Ut_Mode^=1;
			
			if(Seg_Mode==0 && Seg_Dis_Mode==2)		//处于记录
			{
				++Record_Mode;
				if(Record_Mode==3)	Record_Mode=0;
			}
				
		break;
			
		case 9:
			if(Seg_Mode==1 &&Setting==0)	//时间参数设置
			{
				++Time_Index;
				if(Time_Index==5)	Time_Index=0;
				
			}
			
			if(Seg_Mode==1 &&Setting==1)
			{
				if(++Distance_Setting_Index==8)
					Distance_Setting_Index=0;
			}
		break;
	}
}

/*LED处理函数*/
void LED_Proc()
{
	
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
	if(++Seg_Slow_Down==100)	
		Seg_Slow_Down=0;
	
	if(++Key_Slow_Down==100)	
		Key_Slow_Down=0;
	
	if(++Timer1000Ms==1000)
		Timer1000Ms=0;
	
	if(++Seg_Pos==8)
		Seg_Pos=0;
	
	Seg_Dis(Seg_Pos,Seg_Buf[Seg_Pos],Point[Seg_Pos]);
	LED_Dis(Seg_Pos,ucLED[Seg_Pos]);
}
/*main*/
void main()
{
	Set_Rtc(ucRtc);
	Timer0Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		LED_Proc();
	}
}