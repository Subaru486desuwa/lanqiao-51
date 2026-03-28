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

unsigned char Seg_Dis_Mode;	//0-电压 1-测距 2-参数

float Voltage;	//rb2读取的电压

bit Volt_Flag;	//超声波测距标志位 1-满足

unsigned char Distance;	//超声波测距

unsigned char Voltage_Dis[2]={45,5};//电压参数上下限 10x倍
unsigned char Voltage_Set[2]={45,5};//电压参数设置上下限 10x倍
bit Voltage_Set_Index;	//数组指针

unsigned char Timer_100Ms;//100ms
bit LED_Flag;

float Voltage_Output;	//DA输出电压

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
			Voltage_Set_Index=0;	//每次切换界面时，给设置数组指针归零
			
			if(Seg_Dis_Mode==0)	//参数界面回到电压界面
			{
				if(Voltage_Set[0]>=Voltage_Set[1])		//上限大于等于下限
				{
					Voltage_Dis[0]=Voltage_Set[0];
					Voltage_Dis[1]=Voltage_Set[1];
				}
				else
				{
					Voltage_Set[0]=Voltage_Dis[0];
					Voltage_Set[1]=Voltage_Dis[1];		//恢复到上一次显示的值
				}
			}
		break;
		
		case 5:
			if(Seg_Dis_Mode==2)
			{
				Voltage_Set_Index^=1;
			}
		break;
			
		case 6:
			if(Seg_Dis_Mode==2)
			{
				Voltage_Set[Voltage_Set_Index]+=5;
				if(Voltage_Set[Voltage_Set_Index]==55)
					Voltage_Set[Voltage_Set_Index]=5;
			}
		break;
			
		case 7:
			if(Seg_Dis_Mode==2)
			{
				Voltage_Set[Voltage_Set_Index]-=5;
				if(Voltage_Set[Voltage_Set_Index]==0)
					Voltage_Set[Voltage_Set_Index]=50;
			}
		break;
	}
}

/*数码管处理函数*/
void Seg_Proc()
{
  if(Seg_Slow_Down) return;
  Seg_Slow_Down=1;

	// IMPORTANT: ADC must be read TWICE due to STC15 hardware characteristic
	// First read: Triggers conversion, returns old value (discarded)
	// Second read: Returns new value from first trigger
	Ad_Read(0x43);
	Voltage=Ad_Read(0x43)/51.0;
	
	if((float)Voltage_Dis[0] /10 > Voltage && (float)Voltage_Dis[1]/10 <Voltage)	//满足超声波开启条件
	{
		Volt_Flag=1;
	}
	else
		Volt_Flag=0;
	
	if(Seg_Dis_Mode==0)
	{
			Seg_Buf[0]=11;//U
			Seg_Buf[3]=10;
			Seg_Buf[4]=10;
			Seg_Buf[5]=(unsigned char)Voltage;
			Seg_Buf[6]=(unsigned int)(Voltage*100)/10%10;
			Seg_Buf[7]=(unsigned int)(Voltage*100)%10;
			Point[3]=0;
			Point[5]=1;		
			Point[6]=0;
	}

	else if(Seg_Dis_Mode==1)	//超声波
	{
			Point[5]=0;
			Seg_Buf[0]=13;//L
			if(Volt_Flag==1)
			{
				unsigned char i=5;
				Distance=Ut_Wave_Data();
				Seg_Buf[5]=Distance/100%10;
				Seg_Buf[6]=Distance/10%10;
				Seg_Buf[7]=Distance%10;
				
				while(Seg_Buf[i]==0)		//不满足前两位，自动清除
				{
					Seg_Buf[i]=10;
					i++;
					if(i==7)	break;
				}
			}
			
			else//不满足超声波测量条件
			{
				Seg_Buf[5]=14;//A
				Seg_Buf[6]=14;
				Seg_Buf[7]=14;				
			}
	}
	else//参数界面
	{
		Seg_Buf[0]=12;
		Seg_Buf[3]=Voltage_Set[0]/10%10;
		Seg_Buf[4]=Voltage_Set[0]%10;
		Point[3]=1;
		Seg_Buf[5]=10;
		Seg_Buf[6]=Voltage_Set[1]/10%10;
		Seg_Buf[7]=Voltage_Set[1]%10;
		Point[6]=1;
	}

		

	
}

/*超声波距离信息处理函数*/
float Ut_Proc(unsigned char Distance)
{
	if(Distance<=20)
		return 1;
	else if(Distance>=80)
		return 5;
	else
	{
		return 4.0*(Distance-20)/60.0+1;
	}
}

/*LED处理函数*/
void LED_Proc()
{
	ucLED[0]=Seg_Dis_Mode==0;
	ucLED[1]=Seg_Dis_Mode==1;
	ucLED[2]=Seg_Dis_Mode==2;
	ucLED[7]=LED_Flag&&Volt_Flag;
	
	if(Volt_Flag==1)	//启动连续测量
		Voltage_Output=Ut_Proc(Distance);
	else
		Voltage_Output=0;
	
	if(Voltage_Output>=5)
		Da_Write(255);
	else if(Voltage_Output<=1&&Voltage_Output>0)
		Da_Write(51);
	else if(Voltage_Output==0)
		Da_Write(0);
	else
		Da_Write((unsigned char)(51*Voltage_Output));
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


