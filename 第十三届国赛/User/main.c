/*头文件声明*/
#include <STC15F2K60S2.H>
#include <LED.h>
#include <Seg.h>
#include <Key.h>
#include <iic.h>
#include <Ut.h>

/*全局变量声明*/
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Seg_Pos;
unsigned char Point[8]={0,0,0,0,0,0,0,0};

unsigned char ucLED[8]={0,0,0,0,0,0,0,0};

unsigned char Key_Val,Key_Down,Key_Up,Key_Old;

unsigned char Seg_Slow_Down;
unsigned char Key_Slow_Down;

unsigned int Timer1000Ms;//1s
unsigned int Freq;

unsigned char Seg_Dis_Mode;//0-频率 1-湿度 2-测距 3-参数
bit Seg_Freq_Mode;//0-Hz 1-kHz

float Humi;//湿度0-100
float Voltage;//RB2获取电压

unsigned char Distance;//超声波距离
bit Ut_Dis_Mode;//0-cm 1-m

unsigned char Canshu_Mode;//0-频率 1-湿度 2-距离
float Freq_Canshu=9.0;	// 1.0-12.0kHz
unsigned char Humi_Canshu=40;//10-60
float Distance_Canshu=0.6;	//0.1-1.2m

bit Timer_Flag;	//0-关闭 1-开始计时
unsigned int Key_Timer1000Ms;

unsigned char Relay_Count;
float Voltage_Output;

bit Relay_Flag;	//1-由高到低

bit LED_Flag;
unsigned char Timer100Ms;//0.1s

unsigned char Timer_Count;

/*信息处理函数*/
void Seg_Proc()
{
	if(Seg_Slow_Down)	return;
		Seg_Slow_Down=1;
	
	/*数据获取区*/
	Voltage=Ad_Read(0x43)/51.0;
	Humi=20*Voltage;//获取湿度
	
	Distance=Ut_Wave_Data();//获取超声波测距距离
	
	/*数码管显示区域*/
	if(Seg_Dis_Mode==0)
	{
		Seg_Buf[0]=11;//F
		Seg_Buf[1]=10;
		if(Seg_Freq_Mode==0)	//Hz
		{
			unsigned char i=3;

			Seg_Buf[3]=(Freq/10000%10	==0)?	10:Freq/10000%10;
			Seg_Buf[4]=(Freq/1000%10==0 && Seg_Buf[3]==10)?	10:Freq/1000%10;
			Seg_Buf[5]=(Freq/100%10==0 && Seg_Buf[4]==10)?	10:Freq/100%10;
			Seg_Buf[6]=(Freq/10%10==0&& Seg_Buf[5]==10)?	10 :Freq/10%10;
			Seg_Buf[7]=Freq%10;
			Point[6]=0;
			
			
			
		}
		else	//kHz
		{
			unsigned char i=3;
			
			Seg_Buf[3]=Seg_Buf[4]=10;
			Point[6]=1;
			if(Freq>=10000)
			{
				Seg_Buf[5]=Freq/100/100%10;	//64500 64.5kHz	645
				Seg_Buf[6]=Freq/100/10%10;
				Seg_Buf[7]=Freq/100%10;
			}
			else if(Freq<10000 && Freq>=1000)
			{
				Seg_Buf[5]=10;
				Seg_Buf[6]=Freq/100/10%10;	//6400	6.4
				Seg_Buf[7]=Freq/100%10;
			}
			else
			{
				Seg_Buf[5]=10;//999	0.9
				Seg_Buf[6]=0;
				Seg_Buf[7]=Freq/100%10;
			}
			
			while(Seg_Buf[i]==0)
			{
				Seg_Buf[i]=10;
				i++;
				if(i==6)	break;
			}
		}
	}
	
	else if(Seg_Dis_Mode==1)	//湿度
	{
		Seg_Buf[0]=12;//H
		Seg_Buf[1]=Seg_Buf[2]=Seg_Buf[3]=Seg_Buf[4]=Seg_Buf[5]=10;
		Seg_Buf[6]=(unsigned char)Humi/10%10;
		Seg_Buf[7]=(unsigned char)Humi%10;
	}
	
	else if(Seg_Dis_Mode==2)	//测距
	{
		Seg_Buf[0]=13;//A
		if(Ut_Dis_Mode==0)	//cm
		{
			unsigned char i=5;
			Seg_Buf[5]=Distance/100%10;
			Seg_Buf[6]=Distance/10%10;
			Seg_Buf[7]=Distance%10;
			Point[5]=0;
			
			while(Seg_Buf[i]==0)//高位熄灭
			{
				Seg_Buf[i]=10;
				++i;
				if(i==7)	break;
			}
		}
		else
		{
			Seg_Buf[5]=Distance/100%10;	//111cm 1.11m
			Seg_Buf[6]=Distance/10%10;
			Seg_Buf[7]=Distance%10;
			Point[5]=1;
		}
		
	}
	
	else		//参数界面
	{
		Seg_Buf[0]=14;//P
		if(Canshu_Mode==0)	//频率
		{
			Seg_Buf[1]=1;
			Point[6]=1;
			Seg_Buf[5]=(unsigned char)Freq_Canshu/10%10;
			Seg_Buf[6]=(unsigned char)Freq_Canshu%10;
			Seg_Buf[7]=(unsigned char)(Freq_Canshu*10)%10; 
			if(Seg_Buf[5]==0)	Seg_Buf[5]=10;
		}
		
		else if(Canshu_Mode==1)	//湿度
		{
			Seg_Buf[1]=2;
			Seg_Buf[5]=10;
			Point[6]=0;
			Seg_Buf[6]=Humi_Canshu/10%10;
			Seg_Buf[7]=Humi_Canshu%10;
		}
		else
		{
			Seg_Buf[1]=3;
			Point[6]=1;
			Seg_Buf[6]=(unsigned char)Distance_Canshu;
			Seg_Buf[7]=(unsigned char)(Distance_Canshu*10)%10;
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
	Key_Up=~Key_Val&(Key_Val^Key_Old);
	Key_Old=Key_Val;
	
	switch(Key_Down)
	{
		case 4:
			if(++Seg_Dis_Mode==4)
				Seg_Dis_Mode=0;
		break;
			
		case 5:
			if(Seg_Dis_Mode==3)	//处于参数界面
			{
				if(++Canshu_Mode==3)
					Canshu_Mode=0;
			}
		break;
			
		case 6:	//加
			if((Seg_Dis_Mode==3))	
			{
				if(Canshu_Mode==0)	//频率参数界面
				{
					Freq_Canshu+=0.5;
					if(Freq_Canshu==12.5)	
						Freq_Canshu=1.0;					
				}
				else if(Canshu_Mode==1)		//湿度界面
				{
					Humi_Canshu+=10;
					if(Humi_Canshu==70)
						Humi_Canshu=10;
				}
				else	//距离界面
				{
					Distance_Canshu+=0.1;
					if(Distance_Canshu >= 1.3)
						Distance_Canshu=0.1;
				}
			}
			
			if(Seg_Dis_Mode==2)
				Ut_Dis_Mode^=1;
		break;
		
		case 7:
			if((Seg_Dis_Mode==3))	
			{
				if(Canshu_Mode==0)	//频率参数界面
				{
					Freq_Canshu-=0.5;
					if(Freq_Canshu==0.5)
						Freq_Canshu=12.0;					
				}
				else if(Canshu_Mode==1)		//湿度界面
				{
					Humi_Canshu-=10;
					if(Humi_Canshu==0)
						Humi_Canshu=60;
				}
				else	//距离界面
				{
					Distance_Canshu-=0.1;
					if(Distance_Canshu<=0.1)
						Distance_Canshu=1.2;
				}	
			}
				
			if(Seg_Dis_Mode==0)//频率界面
				Seg_Freq_Mode^=1;
		break;
	}
			
	if(Key_Down==7	&& Seg_Dis_Mode==1)	//湿度界面按下S7
	{
		Timer_Flag=1;
	}
	
	if(Key_Timer1000Ms>1000)
	{
		if(Key_Up==7)	//长按
		{
			Key_Timer1000Ms=Timer_Flag=0;
			Relay_Count=0;	//清除继电器吸合次数	
			EEPROM_Write(&Relay_Count,0,1);
		}

	}
}

/*LED函数*/
void LED_Proc()
{
	/*DAC转换*/
	if(Humi>=80)
		Voltage_Output=5*51;
	else if(Humi<=Humi_Canshu)
		Voltage_Output=1*51;
	else
		Voltage_Output=(4*(80.0-Humi_Canshu)*(Humi-Humi_Canshu)+1)*51;
	
	Da_Write((unsigned char)Voltage_Output);	//DAC转换
	
	/*继电器*/
	if(Distance<(unsigned char)(Distance_Canshu*100))
	{
		Relay(0);
		Relay_Flag=1;
	}
	else if(Relay_Flag==1)		//**两个判断条件，得到距离从小于变到大于 
	{
		Relay(1);		
		Relay_Flag=0;
		Relay_Count++;
		EEPROM_Write(&Relay_Count,0,1);
	}
	
	ucLED[0]=(Seg_Dis_Mode==0)&&LED_Flag;
	ucLED[1]=(Seg_Dis_Mode==1)&&LED_Flag;
	ucLED[2]=(Seg_Dis_Mode==2)&&LED_Flag;
	ucLED[3]=Freq>(unsigned int)(Freq_Canshu*1000);
	ucLED[4]=(unsigned char)Humi>Humi_Canshu;
	ucLED[5]=Distance>(unsigned char)(Distance_Canshu*100);
}

/*定时器0初始化函数*/
void Timer0Init(void)	
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
	
	if(++Seg_Pos==8)
		Seg_Pos=0;
	
	if(++Timer1000Ms==1000)
	{
		TR0=0;
		Timer1000Ms=0;
		Freq=TH0<<8|TL0;	//NE555频率
		TH0=TL0=0;
		TR0=1;
	}
	
	if(Timer_Flag==1)
	{
		++Key_Timer1000Ms;
		if(Key_Timer1000Ms>1002)
			Key_Timer1000Ms=1002;//卡住
	}
	
	if(++Timer100Ms==100)
	{
		Timer100Ms=0;
		LED_Flag^=1;
	}
	
	Seg_Dis(Seg_Pos,Seg_Buf[Seg_Pos],Point[Seg_Pos]);
	LED_Dis(Seg_Pos,ucLED[Seg_Pos]);
	
}

/*定时器2初始化函数*/
void Timer2Init(void)		//100微秒@12.000MHz
{
	AUXR &= 0xFB;		//定时器时钟12T模式
	T2L = 0x9C;		//设置定时初值
	T2H = 0xFF;		//设置定时初值
	AUXR |= 0x10;		//定时器2开始计时
	IE2 |= 0x04;
	EA=1;
}

/*定时器2初始化函数*/
void Timer2Server() interrupt 12
{
	if(++Timer_Count==10)
		Timer_Count=0;				//0-9	100us*10=1ms
	
	if(Freq>Freq_Canshu)
	{
		if(Timer_Count<8)
			Motor(1);
		else
			Motor(0);
	}
	else
	{
		if(Timer_Count<2)
			Motor(1);
		else
			Motor(0);
	}
}


/*main*/
void main()
{
	EEPROM_Read(&Relay_Count,0,1);
	Timer0Init();	
	Timer1Init();
	Timer2Init();
	while(1)
	{
		Key_Proc();
		LED_Proc();
		Seg_Proc();
	}
}