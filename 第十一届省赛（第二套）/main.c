/*头文件声明*/
#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <iic.h>
#include <LED.h>
#include <onewire.h>

/*变量声明区*/
unsigned char Key_Val,Key_Down,Key_Old; //按键消抖专用参数
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};  //数码管段码缓冲数组
unsigned char Point[8]={0,0,0,0,0,0,0,0}; //小数点存储数组
unsigned char Seg_Pos;//数码管位码

unsigned char ucLED[8]={0,0,0,0,0,0,0,0}; //LED使能缓冲数组

unsigned int Seg_Slow_Down;//数码管减速专用300ms
unsigned char Key_Slow_Down;//按键减速专用10ms

unsigned char Seg_Dis_Mode; //0-数据显示界面 1-参数设置界面

float temperature;	//接收DS18B20读取温度

unsigned char T[2]={30,20};	//参数设置有效值数组	默认为30 20
unsigned char Set_Temperature[2]={30,20};	//温度参数设置数组 TMAX TMIN
unsigned char Set_Temperature_Index=1;	//数组参数指针	默认指向温度下限

unsigned char LED_Flag;	//检测参数设置操作是否合理

/*按键处理*/
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
		if(Seg_Dis_Mode==1)	Set_Temperature_Index=1;	//确保每次从显示到
		if(Seg_Dis_Mode==0)
		{
			if(Set_Temperature[0] < Set_Temperature[1])
			{
				Set_Temperature[0] = T[0];	//回复之前的有效值
				Set_Temperature[1] = T[1];
				LED_Flag=1;		
			}
			else
			{
				T[0] = Set_Temperature[0];
				T[1] = Set_Temperature[1];	//覆盖原有有效数组数据
				LED_Flag=0;	
			}
		}	
	break;

	case 5:
		if(Seg_Dis_Mode==1)
		{
			Set_Temperature_Index^=1;	
		}
	break;

	case 6:
		if(Seg_Dis_Mode==1)
		{
			++Set_Temperature[Set_Temperature_Index];
			if(Set_Temperature[Set_Temperature_Index]==100)	Set_Temperature[Set_Temperature_Index]=99;
		}
		
	break;

	case 7:
		if(Seg_Dis_Mode==1)
		{
			--Set_Temperature[Set_Temperature_Index];
			if(Set_Temperature[Set_Temperature_Index]==255)	Set_Temperature[Set_Temperature_Index]=0;
		}

	break;

 }


}

/*数码管处理*/
void Seg_Proc()
{
 if(Seg_Slow_Down) return;
 Seg_Slow_Down=1;

 temperature =Read_t();	//读取温度

 if(Seg_Dis_Mode==0)
 {
  	Seg_Buf[0]=11;	//C
	Seg_Buf[3]=10;
	Seg_Buf[4]=10;
	Seg_Buf[6]=(unsigned char)temperature /10 %10;
	Seg_Buf[7]=(unsigned char)temperature%10;
 }

 if(Seg_Dis_Mode==1)
 {
  	Seg_Buf[0]=12;	//P
	Seg_Buf[3]=Set_Temperature[0]/10 %10;
	Seg_Buf[4]=Set_Temperature[0]%10;

	Seg_Buf[6]=Set_Temperature[1]/10 %10;
	Seg_Buf[7]=Set_Temperature[1]%10;
 }

}

void LED_Proc()
{
	if((unsigned char)temperature > Set_Temperature[0])	//T>Tmax
	{
		Da_Write(204);	//DA输出4.0V
		ucLED[0]=1;	//L1点亮
	}
	else	ucLED[0]=0;	//L1熄灭

	if((unsigned char)temperature >=Set_Temperature[1] && (unsigned char)temperature <=Set_Temperature[0])	//T<Tmax T>Tmin
	{
		Da_Write(153);	//DA输出3.0V
		ucLED[1]=1;	//L2
	}
	else
	{
		ucLED[1]=0;	//L2
	}

	if((unsigned char)temperature < Set_Temperature[1])
	{
		Da_Write(102);	//DA输出2.0V
		ucLED[2]=1;	//L3
	}
	else
		ucLED[2]=0;	//L3

	if(LED_Flag)	//判断参数设置操作是否正确
		ucLED[3]=1;	//操作错误，L4点亮
	else
		ucLED[3]=0;




}


/*定时器0初始化函数*/
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


/*定时器0服务*/
void Timer0Server() interrupt 1
{
	TL0 = 0x18;				//设置定时初始值
	TH0 = 0xFC;				//设置定时初始值
	
	++Key_Slow_Down;
	if(Key_Slow_Down==10) Key_Slow_Down=0;			//按键减速
	
	++Seg_Slow_Down;
	if(Seg_Slow_Down==300) Seg_Slow_Down=0;			//数码管减速
	
	if(++Seg_Pos==8)
	{
	 Seg_Pos=0;				   //数码管显示
	}
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


