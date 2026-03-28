/*头文件声明*/
#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <iic.h>
#include <LED.h>
#include <ds1302.h>
#include <math.h>

/*变量声明区*/
unsigned char Key_Val,Key_Down,Key_Old; //按键消抖专用参数
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};  //数码管段码缓冲数组
unsigned char Point[8]={0,0,0,0,0,0,0,0}; //小数点存储数组
unsigned char Seg_Pos;//数码管位码
unsigned char ucLED[8]={0,0,0,0,0,0,0,0}; //LED使能缓冲数组
unsigned int Seg_Slow_Down;//数码管减速专用80ms
unsigned char Key_Slow_Down;//按键减速专用10ms
unsigned char ucRtc[3]={0x23,0x59,0x55};//ds1302初始化数据存储数组
unsigned int Timer_1000Ms;	//1s
unsigned int Freq;	//测量到的频率	0-65535
unsigned int Freq_Final;	//校准过的频率
unsigned char Seg_Mode; //0-频率 1- 参数 2-时间 3-回显
unsigned int Freq_Set=2000;	//超限参数 1000-9000Hz
int Freq_Adjust=0;	//校准值 -900-900Hz
bit Can_Mode_Flag;	//参数界面切换标志位	0-超限 1-校准
bit Dis_Mode_Flag;	//回显界面切换标志位	0-频率 1-时间
bit Min_Flag;	//取反标志位 0-关闭 1-开启
float Voltage_Output;	//DA输出电压
unsigned char Timer_200Ms;	//0.2s
bit LED_Flag;	//0-关闭 1-开启
unsigned int Freq_Max;	//记录频率最大值
unsigned char ucRtc_Max[3]={0x00,0x00,0x00};//记录频率最大发生时的时间


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
  	++Seg_Mode;
	Can_Mode_Flag=0;	//归零
	Dis_Mode_Flag=0;
	if(Seg_Mode==4)	Seg_Mode=0;
  break;

  case 5:
  	if(Seg_Mode==1)		//参数设置切换
		Can_Mode_Flag^=1;

	if(Seg_Mode==3)	  //回显界面切换
		Dis_Mode_Flag^=1;	

  break;

  case 8:
  	if(Seg_Mode==1 &&Can_Mode_Flag==0)	//处于超限界面
	{
		Freq_Set+=1000;
		if(Freq_Set>9000)
			Freq_Set=9000;
	}
	
  	if(Seg_Mode==1 &&Can_Mode_Flag==1)	//处于校准界面
	{
		Freq_Adjust+=100;
		if(Freq_Adjust>900)
			Freq_Adjust=900;

		if(Freq_Adjust>=0)
			Min_Flag=0;
	}	
  break;

  case 9:
  	if(Seg_Mode==1 &&Can_Mode_Flag==0)	//处于超限界面
	{
	 	Freq_Set-=1000;
		if(Freq_Set<1000)
			Freq_Set=1000;
	}

	if(Seg_Mode==1 &&Can_Mode_Flag==1)
	{
	 Freq_Adjust-=100;

	 if(Freq_Adjust<0)
	 	Min_Flag=1;
	 else
	 	Min_Flag=0;

	 if(Freq_Adjust<-900)
	 	Freq_Adjust=-900;
	}
  break;
 }


}

/*数码管处理*/
void Seg_Proc()
{
 if(Seg_Slow_Down) return;
 Seg_Slow_Down=1;

 Read_Rtc(ucRtc);		//获取实时时间
 Freq_Final=Freq+Freq_Adjust;	//获取校准的频率

 if(Freq_Max<=Freq_Final &&Freq_Final<40000)
 {
 	unsigned char i;
  	Freq_Max=Freq_Final;
	for(i=0;i<3;i++)
	{
	 ucRtc_Max[i]=ucRtc[i];
	}
 }


 if(Seg_Mode==0)	//处于频率界面
 {
 	if(Freq_Final>40000)	//最大值约为25000+9000	超过极有可能为负数
	{
	  Seg_Buf[0]=11;	//F
	  Seg_Buf[1]=10;
	  Seg_Buf[2]=10;
	  Seg_Buf[3]=10;
	  Seg_Buf[4]=10;
	  Seg_Buf[5]=10;
	  Seg_Buf[6]=16;
	  Seg_Buf[7]=16;	//L

	}
	else
	{
	  unsigned char i=3;
	  Seg_Buf[0]=11;	//F
	  Seg_Buf[1]=10;
	  Seg_Buf[2]=10;
	  Seg_Buf[3]=Freq_Final/10000 %10;
	  Seg_Buf[4]=Freq_Final/1000 %10;
	  Seg_Buf[5]=Freq_Final/100 %10;
	  Seg_Buf[6]=Freq_Final/10 %10;
	  Seg_Buf[7]=Freq_Final %10;
	
	  while(Seg_Buf[i]==0)
	  {
	   Seg_Buf[i]=10;
	   i++;
	   if(i==7)	break;
	  }
	}

 }

 if(Seg_Mode==1)	//参数界面
 {
  if(Can_Mode_Flag==0)
  {
	  Seg_Buf[0]=12;//P
	  Seg_Buf[1]=1;
	  Seg_Buf[3]=10;
	  Seg_Buf[4]=Freq_Set/1000 %10;
	  Seg_Buf[5]=Freq_Set/100 %10;
	  Seg_Buf[6]=Freq_Set/10 %10;
	  Seg_Buf[7]=Freq_Set %10;
  }
  else
  {
	  Seg_Buf[0]=12;//P
	  Seg_Buf[1]=2;
	  Seg_Buf[3]=10;

	  Seg_Buf[5]=abs(Freq_Adjust)/100%10;
	  Seg_Buf[6]=abs(Freq_Adjust)/10%10;
	  Seg_Buf[7]=abs(Freq_Adjust)%10;
	  if(Min_Flag==1)
	 	 Seg_Buf[4]=15;	//-
	  else
	  	Seg_Buf[4]=10;

  }

 }

 if(Seg_Mode==2)	//时间界面
 {
  Seg_Buf[0]=ucRtc[0]/16;
  Seg_Buf[1]=ucRtc[0]%16;
  Seg_Buf[2]=15;	//-
  Seg_Buf[3]=ucRtc[1]/16;
  Seg_Buf[4]=ucRtc[1]%16;
  Seg_Buf[5]=15;	//-
  Seg_Buf[6]=ucRtc[2]/16;
  Seg_Buf[7]=ucRtc[2]%16;
 }

 if(Seg_Mode==3)		//回显界面
 {
  if(Dis_Mode_Flag==0)	//频率界面
  {
  	  unsigned char i=3;
	  Seg_Buf[0]=13;//H
	  Seg_Buf[1]=11;//F
	  Seg_Buf[2]=10;
	  Seg_Buf[3]=Freq_Max/10000 %10;
	  Seg_Buf[4]=Freq_Max/1000 %10;
	  Seg_Buf[5]=Freq_Max/100 %10;
	  Seg_Buf[6]=Freq_Max/10 %10;
	  Seg_Buf[7]=Freq_Max %10;
	
	  while(Seg_Buf[i]==0)
	  {
	   Seg_Buf[i]=10;
	   i++;
	   if(i==7)	break;
	  }
  }
  else	//时间界面
  {
   		Seg_Buf[0]=13;	//H
		Seg_Buf[1]=14;	//A
		Seg_Buf[2]=ucRtc_Max[0]/16;
		Seg_Buf[3]=ucRtc_Max[0]%16;
		Seg_Buf[4]=ucRtc_Max[1]/16;					 //有问题
		Seg_Buf[5]=ucRtc_Max[1]%16;
		Seg_Buf[6]=ucRtc_Max[2]/16;
		Seg_Buf[7]=ucRtc_Max[2]%16;
  }

 }

}

void LED_Proc()
{


	ucLED[0] = Seg_Mode==0&&LED_Flag==1;	//处于频率界面


	if(Freq_Final>40000)		//显示错误LL
	{
		ucLED[1]=1;
		Da_Write(0);
	}
	else
	{
		ucLED[1]=0 ||(ucLED[1]=Freq_Final>Freq_Set&& LED_Flag==1);	//大于超限参数;
		Voltage_Output=4/(Freq_Set-500.0) *(Freq_Final -500.0)+1;
		Da_Write((unsigned char)(Voltage_Output*51));
	}


}

/*定时器0初始化函数*/
void Timer0Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |=0x05;
	TL0 = 0x00;				//设置定时初始值
	TH0 = 0x00;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
}


/*定时器1初始化函数*/
void Timer1Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0xBF;			//定时器时钟12T模式
	TMOD &= 0x0F;			//设置定时器模式
	TL1 = 0x18;				//设置定时初始值
	TH1 = 0xFC;				//设置定时初始值
	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
	ET1=1;
	EA=1;
}
	


/*定时器1中断服务函数*/
void Timer1Server() interrupt 3
{
	if(++Timer_1000Ms==1000)
	{
		Timer_1000Ms=0;
		Freq=TH0<<8|TL0;
		TH0=TL0=0;
	}
	++Key_Slow_Down;
	if(Key_Slow_Down==10) Key_Slow_Down=0;			//按键减速
	
	++Seg_Slow_Down;
	if(Seg_Slow_Down==80) Seg_Slow_Down=0;			//数码管减速
	
	if(++Seg_Pos==8)
	{
	 Seg_Pos=0;				   //数码管显示
	}

	if(++Timer_200Ms==200)
	{
	 Timer_200Ms=0;
	 LED_Flag^=1;
	}
	Seg_Dis(Seg_Pos,Seg_Buf[Seg_Pos],Point[Seg_Pos]);
	LED_Dis(Seg_Pos,ucLED[Seg_Pos]); 
}

/*main*/
void main()
{
 Set_Rtc(ucRtc);
 Timer1Init();
 Timer0Init();
 while(1)
 {
  Key_Proc();
  Seg_Proc();
  LED_Proc();
 }
}


