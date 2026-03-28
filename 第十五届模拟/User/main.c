/*头文件声明区*/
#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <iic.h>
#include <LED.h>
#include <Ut.h>

/*全局变量声明区域*/
unsigned char Key_Val,Key_Down,Key_Old; //按键消抖专用变量
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};  //数码管段码数据存储数组
unsigned char Point[8]={0,0,0,0,0,0,0,0}; //数码管小数点数据存储数组
unsigned char Seg_Pos;//数码管位选

unsigned char ucLED[8]={0,0,0,0,0,0,0,0}; //LED使能数据存储数组

unsigned int Seg_Slow_Down;//数码管减速专用变量-300ms
unsigned char Key_Slow_Down;//按键减速专用变量-10ms

unsigned char Seg_Dis_Mode;	//0-测距 1-参数 2-记录
bit Seg_Val_Flag;	//0-按键模式 1-旋钮模式

unsigned char Distance;//超声波测距

unsigned char Val_Up=60;//参数上限
unsigned char Val_Down=10;//参数下限

unsigned char Val_Up_Set=60;//参数上限
unsigned char Val_Down_Set=10;//参数下限

unsigned char Count_Num;	//报警次数
float Voltage;	//rb2电压

bit Trigger_Flag;	//0-关闭 1-激活

unsigned char Timer_100Ms;	//100ms
bit LED_Flag;	//LED激活标志位
bit Count_Flag;//计数标志位

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
			if(++Seg_Dis_Mode==3)
				Seg_Dis_Mode=0;
		break;
			
		case 5:
			if(Seg_Dis_Mode==1)
				Seg_Val_Flag^=1;
			
			if(Seg_Dis_Mode==2)
				Count_Num=0;
		break;
			
		case 9:
			if(Seg_Val_Flag==0 && Seg_Dis_Mode==1)	//按键模式
			{
				Val_Up+=10;
				if(Val_Up==100)
					Val_Up=50;
			}
			
			if(Seg_Val_Flag==1 && Seg_Val_Flag==1)
				Val_Up=Val_Up_Set;
			
		break;
			
		case 8:
			if(Seg_Val_Flag==0 && Seg_Dis_Mode==1)
			{
				Val_Down+=10;
				if(Val_Down==50)
					Val_Down=0;
			}
			
			if(Seg_Val_Flag==1 && Seg_Val_Flag==1)
				Val_Down=Val_Down_Set;			
		break;
	}
}

/*数码管处理函数*/
void Seg_Proc()
{
  if(Seg_Slow_Down) return;
  Seg_Slow_Down=1;
	
	Distance = Ut_Wave_Data();//获取超声波数据
	
	Ad_Read(0x43);
	Voltage =Ad_Read(0x43)/51.0;
	
	if(Seg_Val_Flag==1)	//旋钮模式
	{
		if((unsigned char)Voltage <1 )
		{
			Val_Up_Set=50;
			Val_Down_Set=0;
		}
		else if((unsigned char)Voltage >=1 && (unsigned char)Voltage<2)
		{
			Val_Up_Set=60;
			Val_Down_Set=10;
		}
		else if((unsigned char)Voltage >=2 && (unsigned char)Voltage<3)
		{
			Val_Up_Set=70;
			Val_Down_Set=20;		
		}
		else if((unsigned char)Voltage >=3 && (unsigned char)Voltage<4)
		{
			Val_Up_Set=80;
			Val_Down_Set=30;			
		}
		else
		{
			Val_Up_Set=90;
			Val_Down_Set=40;			
		}
	
	}

	
	
	if(Seg_Dis_Mode==0)	//测距界面
	{
		unsigned char i=5;
		
		Seg_Buf[0]= 11;	//A
		Seg_Buf[5]=Distance/100 %10;
		Seg_Buf[6]=Distance/10 %10;
		Seg_Buf[7]=Distance %10;
		
		while(Seg_Buf[i]==0)
		{
			Seg_Buf[i]=10;
			i++;
			if(i==7)	break;
		}
	}
	
	if(Seg_Dis_Mode==1)	//参数界面
	{
		Seg_Buf[0]= 12;	//C
		Seg_Buf[3]=Val_Down/10%10;
		Seg_Buf[4]=Val_Down%10;
		Seg_Buf[5]=13;
		Seg_Buf[6]=Val_Up/10%10;
		Seg_Buf[7]=Val_Up%10;
		if(Seg_Val_Flag==0)
			Seg_Buf[1]=1;
		else
			Seg_Buf[1]=2;
	}
	
	if(Seg_Dis_Mode==2)	//记录界面
	{
		unsigned char i=1;
		Seg_Buf[0]=14;//E
		for(i;i<7;i++)
			Seg_Buf[i]=10;
		if(Count_Num<=9)
			Seg_Buf[7]=Count_Num;
		else
			Seg_Buf[7]=13;
	}

}

/*LED处理函数*/
void LED_Proc()
{
	if(Trigger_Flag==0)
	{
		if(Distance<Val_Down || Distance>Val_Up)
		{
			Trigger_Flag=1;
			Count_Flag=1;
		}		
	}
	if(Distance>=Val_Down && Distance<=Val_Up)
		Trigger_Flag=0;


	if(Count_Flag==1)
	{
		Count_Num++;
		Count_Flag=0;	//记录一次后让他归零
	}

	ucLED[0]=Seg_Dis_Mode==0;
	ucLED[1]=Seg_Dis_Mode==1;
	ucLED[2]=Seg_Dis_Mode==2;
	ucLED[7]=Trigger_Flag==0 || (Trigger_Flag==1 && LED_Flag==1);
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

	if(++Timer_100Ms==100)
	{
		Timer_100Ms=0;
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
 Timer0Init();
 while(1)
 {
  Key_Proc();
  Seg_Proc();
  LED_Proc();
 }
}


