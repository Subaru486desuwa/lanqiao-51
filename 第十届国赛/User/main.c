/*头文件声明*/
#include <STC15F2K60S2.H>
#include <stdio.h>
#include <LED.h>
#include <Seg.h>
#include <Key.h>
#include <Ut.h>
#include <Uart.h>
#include <onewire.h>
#include <iic.h>

/*全局变量声明*/
unsigned char Seg_Slow_Down;				//数码管减速专用200ms
unsigned char Key_Slow_Down;			//按键减速专用10ms
unsigned char Uart_Slow_Down;				//串口减速专用200ms

unsigned char Key_Val,Key_Down,Key_Up,Key_Old;				//按键消抖专用变量

unsigned char Seg_Pos;
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Point[8]={0,0,0,0,0,0,0,0};						//小数点
unsigned char ucLED[8]={0,0,0,0,0,0,0,0};			//LED

unsigned char Uart_Recv[10];				//串口接收数据数组
unsigned char Uart_Recv_Index;

float temperature;		//ds18b20;
unsigned char Ut_Distance;		//超声波测距

bit Seg_Mode;	//0-数据界面 1-参数界面
unsigned char Seg_Shuju_Mode;		//数据界面	0-温度 1-距离 2-变更次数
bit Seg_Canshu_Mode;//0-温度 1-距离

unsigned int Canshu_Count;		//参数变动次数

unsigned char temperature_Canshu_temp;		//温度参数 0-99
unsigned char Distance_Canshu_temp;	//距离参数	0-99
unsigned char temperature_Canshu=30;
unsigned char	Distance_Canshu=35;

bit S12_Timer_Flag;			//按键长短按判断标志位
bit S13_Timer_Flag;
unsigned int S12_Timer1000Ms;		//计时1s
unsigned int S13_Timer1000Ms;

bit Da_Flag=1;	//0-关闭DA 1-开启

/*信息处理函数*/
void Seg_Proc()
{
	if(Seg_Slow_Down)	return;
		Seg_Slow_Down=1;
	
	/*获取数据*/
	temperature=Rd_temperature();			//温度
	Ut_Distance=Ut_Wave_Data();			//超声波
	
	/*DA转换*/
	if(Da_Flag)
	{
		if(Ut_Distance>Distance_Canshu)
			Da_Write(4*51);
		else
			Da_Write(2*51);
	}
	else
		Da_Write(51*4/10);
	
	if(Seg_Mode==0)			//处于数据界面
	{
		Seg_Buf[3]=10;
		if(Seg_Shuju_Mode==0)	//温度
		{
			Seg_Buf[0]=11;		//C
			Seg_Buf[4]=(unsigned char)temperature/10%10;
			Seg_Buf[5]=(unsigned char)temperature%10;
			Seg_Buf[6]=(unsigned int)(temperature*10)%10;
			Seg_Buf[7]=(unsigned int)(temperature*100)%10;
			Point[5]=1;
		}
		else if(Seg_Shuju_Mode==1)			//距离界面
		{
			Seg_Buf[0]=12;		//L
			Seg_Buf[4]=Seg_Buf[5]=10;
			Seg_Buf[6]=Ut_Distance/10%10;
			Seg_Buf[7]=Ut_Distance%10;
			Point[5]=0;
		}
		else			//变更次数界面
		{
			Seg_Buf[0]=13;	//N
			Seg_Buf[3]=(Canshu_Count/10000%10==0)?10:Canshu_Count/10000%10;
			Seg_Buf[4]=(Canshu_Count/1000%10==0 && Seg_Buf[3]==10)?10:Canshu_Count/1000%10;
			Seg_Buf[5]=(Canshu_Count/100%10==0 && Seg_Buf[4]==10)?10:Canshu_Count/100%10;
			Seg_Buf[6]=(Canshu_Count/10%10==0 && Seg_Buf[5]==10)?10:Canshu_Count/10%10;
			Seg_Buf[7]=Canshu_Count%10;
		}
	}
	
	else					//处于参数界面
	{
		Seg_Buf[0]=14;		//P
		Seg_Buf[3]=(unsigned char)Seg_Canshu_Mode+1;
		Seg_Buf[4]=Seg_Buf[5]=10;
		Point[5]=0;
		if(Seg_Canshu_Mode==0)		//温度参数
		{
			Seg_Buf[6]=temperature_Canshu_temp/10%10;
			Seg_Buf[7]=temperature_Canshu_temp%10;
		}
		else			//距离参数
		{
			Seg_Buf[6]=Distance_Canshu_temp/10%10;
			Seg_Buf[7]=Distance_Canshu_temp%10;
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
		case 16:
			if(Seg_Mode==1 &&Seg_Canshu_Mode==0)		//处于参数温度界面
			{
				temperature_Canshu_temp-=2;
				if(temperature_Canshu_temp==254)
				{
					temperature_Canshu_temp=0;
				}
			}
			else if(Seg_Mode==1 &&Seg_Canshu_Mode==1)		//处于距离参数界面
			{
				Distance_Canshu_temp-=5;
				if(Distance_Canshu_temp==251)
				{
					Distance_Canshu_temp=0;
				}
			}
		break;
			
		case 17:
			if(Seg_Mode==1 &&Seg_Canshu_Mode==0)		//处于参数温度界面
			{
				temperature_Canshu_temp+=2;
				if(temperature_Canshu_temp==100)
				{
					temperature_Canshu_temp=98;
				}
			}
			else if(Seg_Mode==1 &&Seg_Canshu_Mode==1)		//处于距离参数界面
			{
				Distance_Canshu_temp+=5;
				if(Distance_Canshu_temp==100)
				{
					Distance_Canshu_temp=95;
				}
			}
		break;
	}
	
	if(Key_Down==12)
		S12_Timer_Flag=1;
	

	if(S12_Timer1000Ms<1000)		//短按
	{
		if(Key_Up==12)
		{
				if(Seg_Mode==0)
				{
					if(++Seg_Shuju_Mode==3)
						Seg_Shuju_Mode=0;
				}
				if(Seg_Mode==1)
				{
					Seg_Canshu_Mode^=1;
				}
				S12_Timer1000Ms=S12_Timer_Flag=0;
		}

	}
	else
	{
		if(Key_Up==12)
		{
			S12_Timer_Flag=S12_Timer1000Ms=0;
			Canshu_Count=0;
			EEPROM_Write(&Canshu_Count,0,2);

		}
	}

	if(Key_Down==13)
		S13_Timer_Flag=1;
	
	if(S13_Timer1000Ms<1000)
	{
		if(Key_Up==13)
		{
			S13_Timer_Flag=S13_Timer1000Ms=0;
			Seg_Mode^=1;
			Seg_Canshu_Mode=0;
			Seg_Shuju_Mode=0;
			if(Seg_Mode==1)		//数据界面切换到参数界面
			{
				temperature_Canshu_temp=temperature_Canshu;
				Distance_Canshu_temp=Distance_Canshu;
			}
			
			if(Seg_Mode==0)			//参数切换到数据
			{
				if((temperature_Canshu!=temperature_Canshu_temp)||(Distance_Canshu!=Distance_Canshu_temp))		//参数发生变动
				{
					++Canshu_Count;
					EEPROM_Write(&Canshu_Count,0,2);			//存入unsigned int只需存两次
				}
				temperature_Canshu=temperature_Canshu_temp;
				Distance_Canshu=Distance_Canshu_temp;
			}
		}
	}
	else
	{
		if(Key_Up==13)
		{
			S13_Timer_Flag=S13_Timer1000Ms=0;
			Da_Flag^=1;
		}
	}
	
}


/*串口处理函数*/
void Uart_Proc()
{
	if(Uart_Slow_Down)	return;
		Uart_Slow_Down=1;
	
	if(Uart_Recv_Index==0)	return;
	
	if(Uart_Recv[0]=='S'&&Uart_Recv[1]=='T'&&Uart_Recv[2]=='\r'&&Uart_Recv[3]=='\n')
		printf("$%bu,%.2f\r\n",Ut_Distance,temperature);
	
	else if(Uart_Recv[0]=='P'&&Uart_Recv[1]=='A'&&Uart_Recv[2]=='R'&&Uart_Recv[3]=='A'&&Uart_Recv[4]=='\r'&&Uart_Recv[5]=='\n')
		printf("%bu,%bu\r\n",Distance_Canshu,temperature_Canshu);
	
	else
		printf("ERROR\r\n");
	
	Uart_Recv_Index=0;
}



/*LED处理函数*/
void LED_Proc()
{
	ucLED[0]=temperature>=temperature_Canshu;
	ucLED[1]=Ut_Distance<Distance_Canshu;
	ucLED[2]=Da_Flag;
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
	if(++Seg_Slow_Down==200)
	{
		Seg_Slow_Down=0;
	}
	
	if(++Key_Slow_Down==10)
	{
		Key_Slow_Down=0;
	}
	
	if(++Uart_Slow_Down==200)
	{
		Uart_Slow_Down=0;
	}
	
	if(S12_Timer_Flag)
	{
		if(++S12_Timer1000Ms>1002)
			S12_Timer1000Ms=1002;
	}

	if(S13_Timer_Flag)
	{
		if(++S13_Timer1000Ms>1002)
			S13_Timer1000Ms=1002;
	}
	
	if(++Seg_Pos==8)
	{
		Seg_Pos=0;
	}
	
	Seg_Dis(Seg_Pos,Seg_Buf[Seg_Pos],Point[Seg_Pos]);
	LED_Dis(Seg_Pos,ucLED[Seg_Pos]);
}

/*串口中断函数*/
void Uart1Server() interrupt 4
{
	if(RI==1&&Uart_Recv_Index<10)
	{
		Uart_Recv[Uart_Recv_Index]=SBUF;
		RI=0;
		Uart_Recv_Index++;
	}
	
	if(Uart_Recv_Index>8)
		Uart_Recv_Index=0;
}

/*延时750ms*/
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


/*main*/
void main()
{
	EEPROM_Read(&Canshu_Count,0,2);
	Rd_temperature();
	Delay750ms();
	UartInit();
	Timer0Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		LED_Proc();
		Uart_Proc();
	}
}