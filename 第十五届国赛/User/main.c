/*头文件声明区*/
#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <iic.h>
#include <LED.h>
#include <Uart.h>
#include <stdio.h>
#include <math.h>
#include <Ut.h>


/*全局变量声明区域*/
unsigned char Key_Val,Key_Down,Key_Old; //按键消抖专用变量
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};  //数码管段码数据存储数组
unsigned char Point[8]={0,0,0,0,0,0,0,0}; //数码管小数点数据存储数组
unsigned char Seg_Pos;//数码管位选

unsigned char ucLED[8]={0,0,0,0,0,0,0,0}; //LED使能数据存储数组

unsigned char Seg_Slow_Down;//数码管减速专用变量-200ms
unsigned char Key_Slow_Down;//按键减速专用变量-10ms

unsigned int Timer1000Ms;//1s
unsigned int Freq;//NE555

unsigned char Uart_Slow_Down;//200ms

unsigned char Uart_Recv[10];
unsigned char Uart_Recv_Index;

unsigned char Seg_Dis_Mode=0;//0-坐标 1-速度 2-参数

unsigned int Pos_Real[2]={0,0};//x,y
unsigned int Pos_Dest[2]={0,0};//目的地位置		X:0~999 Y:0~999

unsigned char Device_Status=0;//0-空闲 1-等待 2-运行

unsigned char Distance;//超声波测距

float v;//行进速度
float R=1.0;//1.0-2.0
char B_1=0;//-90~90
bit Canshu_Index;//0-R 1-B_1

bit show_flag;//0-第一次不显示freq频率

/*按键处理函数*/
void Key_Proc()
{
 if(Key_Slow_Down) return;
 Key_Slow_Down=1;							 //按键减速

 Key_Val=Key_Read();
 Key_Down=Key_Val&(Key_Val^Key_Old);
 Key_Old=Key_Val;						  //按键消抖


	switch(Key_Down)
	{
		
		case 5:
			if(Device_Status==0)		//处于空闲状态
				Pos_Real[0]=Pos_Real[1]=0;
		break;
			
		case 8:
			if(++Seg_Dis_Mode==3)
				Seg_Dis_Mode=0;
			Canshu_Index=0;
		break;
			
		case 9:
			if(Seg_Dis_Mode==2)		//参数界面
			{
				Canshu_Index^=1;
			}
		break;
			
		case 12:
			if(Seg_Dis_Mode==2)
			{
				if(Canshu_Index==0)	//R
				{
					R+=0.1;
					if(R>2.0)
						R=2.0;
				}
				else//B_1
				{
					B_1+=5;
					if(B_1==95)
						B_1=90;
				}
			}
		break;
			
		case 13:
			if(Seg_Dis_Mode==2)
			{
				if(Canshu_Index==0)	//R
				{
					R-=0.1;
					if(R<1.0)
						R=1.0;
				}
				else//B_1
				{
					B_1-=5;
					if(B_1==-95)
						B_1=-90;
				}
			}		
		break;
	}
	
}

/*数码管处理函数*/
void Seg_Proc()
{
 if(Seg_Slow_Down) return;
 Seg_Slow_Down=1;
	
	/*数据获取*/
	if(show_flag)
	{
		v=3.14*R*Freq/100.0+B_1;		
	}
	Distance=Ut_Wave_Data();

	/*显示*/
	if(Seg_Dis_Mode==0)	//坐标界面
	{
		Seg_Buf[0]=11;			//L
		Point[2]=0;
		if(Device_Status==1||Device_Status==2)		//0-空闲 1-等待 2-运行
		{
			
			Seg_Buf[1]=(Pos_Dest[0]/100%10==0)?10:Pos_Dest[0]/100%10;
			Seg_Buf[2]=(Pos_Dest[0]/10%10==0 &&Seg_Buf[1]==10)?10:Pos_Dest[0]/10%10;
			Seg_Buf[3]=Pos_Dest[0]%10;
			Seg_Buf[4]=12;			//-
			Seg_Buf[5]=(Pos_Dest[1]/100%10==0)?10:Pos_Dest[1]/100%10;
			Seg_Buf[6]=(Pos_Dest[1]/10%10==0 &&Seg_Buf[5]==10)?10:Pos_Dest[1]/10%10;
			Seg_Buf[7]=Pos_Dest[1]%10;
		}
		
		
		else				//处于空闲状态
		{
			
			Seg_Buf[1]=Pos_Real[0]/100%10==0 ? 10:Pos_Real[0]/100%10;
			Seg_Buf[2]=(Pos_Real[0]/10%10==0 &&	Seg_Buf[1]==10)? 10:Pos_Real[0]/10%10;
			Seg_Buf[3]=Pos_Real[0]%10;
			Seg_Buf[4]=12;			//-
			Seg_Buf[5]=Pos_Real[1]/100%10==0 ? 10:Pos_Real[1]/100%10;
			Seg_Buf[6]=(Pos_Real[1]/10%10==0 && Seg_Buf[5]==10)? 10: Pos_Real[1]/10%10;
			Seg_Buf[7]=Pos_Real[1]%10;		
			
			
			
		}
	}
	
	else if(Seg_Dis_Mode==1)		//处于速度界面
	{
		Seg_Buf[0]=13;			//E
		if(Device_Status==2)		//运行状态
		{
		
			Seg_Buf[1]=1;
			Seg_Buf[2]=10;
			Seg_Buf[3]=((unsigned int)v/1000%10==0)?10:(unsigned int)v/1000%10;
			Seg_Buf[4]=((unsigned int)v/100%10==0 && Seg_Buf[3]==10)? 10: (unsigned int)v/100%10;
			Seg_Buf[5]=((unsigned int)v/10%10 ==0 && Seg_Buf[4]==10)? 10: (unsigned int)v/10%10;
			Seg_Buf[6]=((unsigned int)v%10 ==0 && Seg_Buf[5]==10) ? 10: (unsigned int)v%10;
			Seg_Buf[7]=(unsigned int)(v*10)%10;
			Point[6]=1;
			
		}

		else if(Device_Status==0)	//空闲
		{
			unsigned char i;
			Seg_Buf[1]=2;
			Point[6]=0;
			for(i=3;i<8;i++)
			{
				Seg_Buf[i]=12;//-
			}
		}
		
		else		//等待
		{
			
			Seg_Buf[3]=Distance/10000%10==0 ? 10:Distance/10000%10;
			Seg_Buf[4]=(Distance/1000%10==0 && Seg_Buf[3]==10)? 10:Distance/1000%10;
			Seg_Buf[5]=(Distance/100%10==0 && Seg_Buf[4]==10)? 10:Distance/100%10;
			Seg_Buf[6]=(Distance/10%10==0 && Seg_Buf[5]==10)? 10:Distance/10%10;
			Seg_Buf[7]=Distance%10;
			
		}
	}
	
	else	//参数界面
	{
		Seg_Buf[0]=14;//P
		Seg_Buf[1]=10;
		Seg_Buf[2]=(unsigned char)R;
		Seg_Buf[3]=(unsigned char)(R*10)%10;
		Point[2]=1;
		Seg_Buf[4]=10;
		if(abs(B_1)<10)
		{
			Seg_Buf[5]=10;
			Seg_Buf[7]=abs(B_1);
			if(B_1<0)	Seg_Buf[6]=12;
			else	Seg_Buf[6]=10;
		}
		else if(abs(B_1)>=10)
		{
			Seg_Buf[6]=abs(B_1)/10%10;
			Seg_Buf[7]=abs(B_1)%10;
			if(B_1<0)	Seg_Buf[5]=12;
			else	Seg_Buf[5]=10;
		}
	}
}


/*LED处理*/
void LED_Proc()
{


}

/*串口处理*/
void Uart_Proc()
{
	if(Uart_Slow_Down)	return;
		Uart_Slow_Down=1;
	
	if(Uart_Recv_Index<10)
	{
		if(Uart_Recv_Index==1 && Uart_Recv[0]=='?')
		{
			if(Device_Status==0)//0-空闲 1-等待 2-运行
				printf("Idle\r\n");
			else if(Device_Status==1)
				printf("Wait\r\n");
			else
				printf("Busy\r\n");
		}
	}
	
	Uart_Recv_Index=0;
}


/*定时器0中断初始化函数*/
void Timer0Init(void)
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD|=0X05;
	TL0 = 0x00;				//设置定时初始值
	TH0 = 0x00;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
}

void Uart1Server() interrupt 4
{
	if(RI==1 && Uart_Recv_Index<10)
	{
		Uart_Recv[Uart_Recv_Index]=SBUF;
		Uart_Recv_Index++;
		RI=0;
	}
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

	if(++Key_Slow_Down==10)
		Key_Slow_Down=0;			//按键减速

	if(++Seg_Slow_Down==200)
		Seg_Slow_Down=0;			//数码管减速
	
	if(++Uart_Slow_Down==200)
		Uart_Slow_Down=0;	//串口减速
	
	if(++Timer1000Ms==1000)
	{
		TR0=0;
		Timer1000Ms=0;
		Freq=TH0<<8|TL0;
		TH0=TL0=0;
		show_flag=1;
		TR0=1;
	}
	
	if(++Seg_Pos==8)
	{
	 Seg_Pos=0;				   //数码管位选
	}
	Seg_Dis(Seg_Pos,Seg_Buf[Seg_Pos],Point[Seg_Pos]);
	LED_Dis(Seg_Pos,ucLED[Seg_Pos]); 
}

/*main*/
void main()
{
	UartInit();
  Timer0Init();
	Timer1Init();
  while(1)
  { 
   Key_Proc();
   Seg_Proc();
   LED_Proc();
	 Uart_Proc();
  }
}


