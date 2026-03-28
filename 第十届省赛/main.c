/*头文件声明*/
#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <iic.h>
#include <LED.h>


/*变量声明区*/
unsigned char Key_Val,Key_Down,Key_Old; //按键消抖专用参数
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};  //数码管段码缓冲数组
unsigned char Point[8]={0,0,0,0,0,0,0,0}; //小数点存储数组
unsigned char Seg_Pos;//数码管位码

unsigned char ucLED[8]={0,0,0,0,0,0,0,0}; //LED使能缓冲数组

unsigned int Seg_Slow_Down;//数码管减速专用300ms
unsigned char Key_Slow_Down;//按键减速专用10ms

unsigned int Timer_1000Ms;	//1s
unsigned int Freq;	//NE555频率 0-65535
float Voltage;	//AD读取rb2的电压

unsigned char Seg_Dis_Mode;	//0-频率显示页面 1-电压显示页面
unsigned char Output_Mode;		//0-固定2V 1-随rb2输出变化
unsigned char LED_Flag=1;	//LED控制标志位 0-关闭 1-开启
unsigned char Seg_Flag=1;	//数码管标志位  0-关闭 1-开启


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
	break;

	case 5:
		Output_Mode^=1;
	break;

	case 6:
		LED_Flag^=1;
	break;

	case 7:
		Seg_Flag^=1;
	break;
 }


}

/*数码管处理*/
void Seg_Proc()
{
 if(Seg_Slow_Down) return;
 Seg_Slow_Down=1;

 Voltage=Ad_Read(0x43)/51.0;		//读取rb2的电压



 if(Output_Mode==0)	Da_Write(102);	//固定输出2V
 if(Output_Mode==1)	Da_Write(Voltage);	//跟随rb2


 if(Seg_Flag==0)		//数码管关闭
 {
  unsigned char i;
  for(i=0;i<8;i++)	Seg_Buf[i]=10;
  Point[5]=0;
 }
 if(Seg_Flag==1)		//数码管显示
 {
  if(Seg_Dis_Mode==0)		//处于频率显示页面
 {
	unsigned char i=3; 	

  	Seg_Buf[0]=11;	//F
	Seg_Buf[3]=Freq/10000 %10;
	Seg_Buf[4]=Freq/1000 %10;
	Seg_Buf[5]=Freq/100 %10;
	Seg_Buf[6]=Freq/10 %10;
	Seg_Buf[7]=Freq%10;
	Point[5]=0;
	
	while(Seg_Buf[i] == 0)
	{
		Seg_Buf[i]=10;
		i++;
		if(i==8)	break;
	} 	
 }

 if(Seg_Dis_Mode==1)		//电压显示界面
 {
  	Seg_Buf[0]=12;
	Seg_Buf[3]=10;
	Seg_Buf[4]=10;
	Seg_Buf[5]=(unsigned char)Voltage;
	Seg_Buf[6]=(unsigned char)(Voltage*100)/10 %10;
	Seg_Buf[7]=(unsigned char)(Voltage*100)/100 %10;
	Point[5]=1;
 }
 }

}
void LED_Proc()
{
 if(LED_Flag==0)
 {
  unsigned char i;
  for(i=0;i<8;i++)
  {
   ucLED[i]=0;
  }
 }
 if(LED_Flag==1)
 {
  if(Seg_Dis_Mode==0)	//频率
  {
   ucLED[0]=0;
   ucLED[1]=1;
  }

  if(Seg_Dis_Mode==1)	//电压
  {
   ucLED[0]=1;
   ucLED[1]=0;
  }

  if(Voltage <1.5 ||(Voltage>=2.5 && Voltage<3.5))	ucLED[2]=0;
  if((Voltage >=1.5 && Voltage<2.5)||Voltage>=3.5)	ucLED[2]=1;		//L3功能

  if(Freq<1000 || (Freq>=5000 && Freq<10000)) ucLED[3]=0;
  if((Freq>=1000 && Freq<5000)|| Freq>10000) ucLED[3]=1;		//L4功能

  if(Output_Mode==1) ucLED[4]=1;
  if(Output_Mode==0) ucLED[4]=0;		//L5


 }



}

/*频率测量定时器配置*/
void Timer0Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |=0x05;	
	TL0 = 0;				//设置定时初始值
	TH0 = 0;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
}


/*定时器初始化函数*/
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
void Timer1Server() interrupt 3		//中断序列号改为3
{
	++Key_Slow_Down;
	if(Key_Slow_Down==10) Key_Slow_Down=0;			//按键减速
	
	++Seg_Slow_Down;
	if(Seg_Slow_Down==300) Seg_Slow_Down=0;			//数码管减速

	if(++Timer_1000Ms==1000)
	{
	 	Timer_1000Ms=0;
		Freq = TH0 <<8 |TL0;		//高八位左移
		TH0=0;
		TL0=0;	  //手动复位		
	}
	
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
 Timer1Init();
 Timer0Init();
 while(1)
 {
  Key_Proc();
  Seg_Proc();
  LED_Proc();
 }
}


