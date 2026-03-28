/*头文件声明区*/
#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <LED.h>
#include <Uart.h>
#include <Ut.h>
#include <stdio.h>

/*全局变量声明区域*/
unsigned char Key_Val,Key_Down,Key_Old; //按键消抖专用变量
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};  //数码管段码数据存储数组
unsigned char Point[8]={0,0,0,0,0,0,0,0}; //数码管小数点数据存储数组
unsigned char Seg_Pos;//数码管位选

unsigned char ucLED[8]={0,0,0,0,0,0,0,0}; //LED使能数据存储数组

unsigned int Seg_Slow_Down;//数码管减速专用变量-300ms
unsigned char Key_Slow_Down;//按键减速专用变量-10ms
unsigned char Uart_Slow_Down;//串口减速专用变量-200ms

unsigned char Uart_Recv_Index;	//串口接收数组指针
unsigned char Uart_Recv[10];		//串口接收数据数组 默认长度10

bit Seg_Dis_Mode;	//0-距离显示 1-参数设置

unsigned char Distance;	//测量出的距离参数
int Distance_Set=30;	//距离设置参数

unsigned char Timer_200Ms;//200ms
bit LED_Flag;


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
		case 4:
			Seg_Dis_Mode^=1;
		break;
		
		case 8:
			if(Seg_Dis_Mode==0)
			{
				Distance_Set=(int)Distance;
			}
		break;
		
		case 12:
			if(Seg_Dis_Mode==1)
			{
				Distance_Set+=10;
				if(Distance_Set>999)
					Distance_Set=999;
			}			
		break;
			
		case 16:
			if(Seg_Dis_Mode==1)
			{
				Distance_Set-=10;
				if(Distance_Set < 0)
					Distance_Set=0;
				
			}
		break;
			
		case 9:
			printf("Distance:%dcm\r\n",(unsigned int)Distance);	
		break;
	}


}

/*数码管处理函数*/
void Seg_Proc()
{
	unsigned char i=5;
	
	if(Seg_Slow_Down) return;
  Seg_Slow_Down=1;
	
	Distance = Ut_Wave_Data();

	if(Seg_Dis_Mode==0)	//距离显示界面
	{
		Seg_Buf[0]=11;//U
		Seg_Buf[1]=1;
		Seg_Buf[5]=Distance/100%10;
		Seg_Buf[6]=Distance/10%10;
		Seg_Buf[7]=Distance%10;			

	}
	
	else	//处于参数界面
	{
		Seg_Buf[1]=2;
		Seg_Buf[5]=Distance_Set/100%10;
		Seg_Buf[6]=Distance_Set/10%10;
		Seg_Buf[7]=Distance_Set%10;		
	}
	
	while(Seg_Buf[i] == 0)
	{
		Seg_Buf[i]=10;
		i++;
		if(i==7)
			break;
	}
}

/*LED处理函数*/
void LED_Proc()
{
	ucLED[0]=	Seg_Dis_Mode==0;
	ucLED[1]=	Seg_Dis_Mode==1;
	ucLED[2]=(Distance>Distance_Set) && LED_Flag==1;
}

/*串口处理函数*/
void Uart_Proc()
{
	if(Uart_Slow_Down) return;
	Uart_Slow_Down=1;
	

	Uart_Recv_Index=0;
}


/*串口1中断服务函数*/
void Uart1Server() interrupt 4
{
	if(RI== 1 && Uart_Recv_Index<10)	//串口接收数据 RI=1-读到数据	限制
	{
		Uart_Recv[Uart_Recv_Index] = SBUF;
		Uart_Recv_Index++; 		//处理函数中，用完的接收指针需要归零
		RI=0;
	}
	
	if(Uart_Recv_Index>6)	//根据题目要求修改
			Uart_Recv_Index=0;
}

/*定时器0中断初始化函数*/
void Timer0Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TL0 = 0x18;				//设置定时初始值
	TH0 = 0xFC;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
	ET0=1;
	EA=1;
}



/*定时器0中断服务函数*/
void Timer0Server() interrupt 1
{
	TL0 = 0x18;				//设置定时初始值
	TH0 = 0xFC;				//设置定时初始值
	

	if(++Key_Slow_Down==10)
		Key_Slow_Down=0;			//按键减速

	if(++Seg_Slow_Down==300)
		Seg_Slow_Down=0;			//数码管减速
	
	if(++Uart_Recv_Index==200)
		Uart_Recv_Index=0;			//串口减速
	
	if(++Timer_200Ms==200)
	{
		Timer_200Ms=0;
		LED_Flag^=1;
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
 while(1)
 {
  Key_Proc();
  Seg_Proc();
  LED_Proc();
	Uart_Proc();
 }
}


