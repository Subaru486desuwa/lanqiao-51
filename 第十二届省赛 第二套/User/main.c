/*头文件声明区*/
#include <STC15F2K60S2.H>
#include <LED.h>
#include <Seg.h>
#include <Key.h>
#include <iic.h>

/*变量声明区*/
unsigned char Seg_Slow_Down;
unsigned char Key_Slow_Down;

unsigned char Key_Val,Key_Down,Key_Up,Key_Old;

unsigned char Seg_Pos;
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Point[8]={0,0,0,0,0,0,0,0};

unsigned char ucLED[8]={0,0,0,0,0,0,0,0};

unsigned int Timer_1000Ms;
unsigned int Freq;
unsigned int Freq_Save;

unsigned int Cycle;//周期

unsigned char Seg_Dis_Mode;//0-频率 1-周期 2-电压

bit Volt_Mode;//0-光敏 1-rb2

float Voltage_lux;
float Voltage_rb2;
float Voltage_rb2_save;

// FIXED BUG: Removed duplicate definition (Timer_1000Ms already defined on line 20)
bit Count_Flag;//按下按键计时标志位

bit LED_Flag=1;//0-熄灭 1-开启

/*信息处理函数*/
void Seg_Proc()
{
	if(Seg_Slow_Down)	return;
		Seg_Slow_Down=1;
	
	Cycle=1000000/Freq;

	// IMPORTANT: ADC must be read TWICE due to STC15 hardware characteristic
	// First read: Triggers conversion, returns old cached value (discarded)
	// Second read: Returns the new converted value
	Ad_Read(0x41);
	Voltage_lux=Ad_Read(0x41)/51.0;

	Ad_Read(0x43);
	Voltage_rb2=Ad_Read(0x43)/51.0;
	
	if(Seg_Dis_Mode==0)//频率界面
	{
		unsigned char i=3;
		
		Seg_Buf[0]=11;//F
		Seg_Buf[1]=10;
		Seg_Buf[2]=10;
		Seg_Buf[3]=Freq/10000%10;
		Seg_Buf[4]=Freq/1000%10;
		Seg_Buf[5]=Freq/100%10;
		Seg_Buf[6]=Freq/10%10;
		Seg_Buf[7]=Freq%10;
		Point[5]=0;
		
		while(Seg_Buf[i]==0)
		{
			Seg_Buf[i]=10;
			i++;
			if(i==7)	break;
		}
	}
	
	else if(Seg_Dis_Mode==1)//周期界面
	{
		unsigned char i=3;
		
		Seg_Buf[0]=12;//N
		Seg_Buf[3]=Cycle/10000%10;
		Seg_Buf[4]=Cycle/1000%10;
		Seg_Buf[5]=Cycle/100%10;
		Seg_Buf[6]=Cycle/10%10;
		Seg_Buf[7]=Cycle%10;	

		while(Seg_Buf[i]==0)
		{
			Seg_Buf[i]=10;
			i++;
			if(i==7)	break;	
		}
	}
		
	else
	{
		Seg_Buf[0]=13;//U
		Seg_Buf[1]=14;
		Seg_Buf[3]=10;
		Seg_Buf[4]=10;
		if(Volt_Mode==0)
		{
			Seg_Buf[2]=1;	//光敏
			Seg_Buf[5]=(unsigned char)Voltage_lux;
			Seg_Buf[6]=(unsigned int)(Voltage_lux*100)/10%10;
			Seg_Buf[7]=(unsigned int)(Voltage_lux*100)%10;
			Point[5]=1;
		}
		else
		{
			Seg_Buf[2]=3;
			Seg_Buf[5]=(unsigned char)Voltage_rb2;
			Seg_Buf[6]=(unsigned int)(Voltage_rb2*100)/10%10;
			Seg_Buf[7]=(unsigned int)(Voltage_rb2*100)%10;
			Point[5]=1;
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
	Key_Up= ~Key_Val &(Key_Val^Key_Old);
	Key_Old=Key_Val;
	
	switch(Key_Down)
	{
		case 4:
			if(++Seg_Dis_Mode==3)
				Seg_Dis_Mode=0;
			Volt_Mode=0;
		break;
			
		case 5:
			if(Seg_Dis_Mode==2)
				Volt_Mode^=1;
		break;
			
		case 6:
			Voltage_rb2_save=Voltage_rb2;
		break;
	}
	
	if(Key_Down==7)
	{
		Count_Flag=1;
	}
	
	if(Key_Up==7)
	{
		if(Timer_1000Ms<1000)
		{
			Freq_Save=Freq;
			Count_Flag=Timer_1000Ms=0;
		}
		else
		{
			LED_Flag^=1;
			Count_Flag=Timer_1000Ms=0;
		}
	}
	


	
	
	
}
/*LED处理函数*/
void LED_Proc()
{
	if(LED_Flag==1)
	{
		ucLED[0]=Voltage_rb2>Voltage_rb2_save;
		ucLED[1]=Freq>Freq_Save;
		ucLED[2]=Seg_Dis_Mode==0;
		ucLED[3]=Seg_Dis_Mode==1;
		ucLED[4]=Seg_Dis_Mode==2;		
	}
	else
	{
		unsigned char i;
		for(i=0;i<8;i++)
			ucLED[i]=0;
	}

}
/*NE555定时器0初始化函数*/
void Timer0Init(void)		//@12.000MHz
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
	
	if(++Key_Slow_Down==200)
		Key_Slow_Down=0;	
	
	if(++Timer_1000Ms==1000)
	{
		TR0=0;
		Timer_1000Ms=0;
		Freq=TH0<<8|TL0;
		TH0 = TL0 = 0;
		TR0=1;
	}
	
	if(Count_Flag==1)
	{
		if(++Timer_1000Ms==1101)
			Timer_1000Ms=1100;//超过1s后，卡在1.1s即可
	}
	
	if(++Seg_Pos==8)
		Seg_Pos=0;
	
	Seg_Dis(Seg_Pos,Seg_Buf[Seg_Pos],Point[Seg_Pos]);
	LED_Dis(Seg_Pos,ucLED[Seg_Pos]);
}
/*Main函数*/
void main()
{
	Timer0Init();	
	Timer1Init();	
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		LED_Proc();
	}
}