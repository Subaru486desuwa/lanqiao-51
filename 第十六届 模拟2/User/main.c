/*头文件声明区*/
#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <iic.h>
#include <LED.h>
#include <ds1302.h>

/*全局变量声明区域*/
unsigned char Key_Val,Key_Down,Key_Old; //按键消抖专用变量
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};  //数码管段码数据存储数组
unsigned char Point[8]={0,0,0,0,0,0,0,0}; //数码管小数点数据存储数组
unsigned char Seg_Pos;//数码管位选

unsigned char ucLED[8]={0,0,0,0,0,0,0,0}; //LED使能数据存储数组

unsigned char Seg_Slow_Down;//数码管减速专用变量-100ms
unsigned char Key_Slow_Down;//按键减速专用变量-10ms

unsigned char ucRtc[3]={0x23,0x59,0x50};//ds1302初始化数据存储数组

unsigned char Seg_Dis_Mode;	//0-时间 1-数据 2-历史
unsigned char Seg_Scene;	//1,2,3三个界面

float Voltage_rd1;	//光敏
float Voltage_rb2;	//分压

unsigned char ucRtc_Trigger[3][3]={{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00}};	//未触发前，数码管显示-
unsigned char Trigger_Index;	//数组存储指针
bit Trigger_Flag;	//1-触发
unsigned int Timer_3000Ms;//3s



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
			Seg_Scene=0;	//默认索引值为1
		break;
			
		case 5:
			if(Seg_Dis_Mode==2)	//处于历史查询界面
			{
				if(++Seg_Scene==3)
					Seg_Scene=0;
			}
		break;
			
		case 8:
			if(Seg_Dis_Mode==2)	//处于历史查询
				Trigger_Index=0;	
		break;
	}
	


}

/*数码管处理函数*/
void Seg_Proc()
{
	if(Seg_Slow_Down) return;
	Seg_Slow_Down=1;
	
	Read_Rtc(ucRtc);	//获取实时时间
	
	Ad_Read(0x43);
	Ad_Read(0x41);
	Voltage_rd1=Ad_Read(0x43)/51.0;
	Voltage_rb2=Ad_Read(0x41)/51.0;
	

	if(Voltage_rd1<Voltage_rb2 && Timer_3000Ms>=3000)
	{
		unsigned char i=0;
		Timer_3000Ms=0;
		Trigger_Flag=1;
		for(i=0;i<3;i++)
			ucRtc_Trigger[Trigger_Index][i]=ucRtc[i];	//第一次数据-0行 第二次-1行
			Trigger_Index++;
			if(Trigger_Index==3)
				Trigger_Index=3;		//锁死在3

	}
	if(Timer_3000Ms>=3000&&Trigger_Flag==1)
		Trigger_Flag=0;
		
	if(Trigger_Flag==1)
	{
		Seg_Buf[0]=13;	//C
		Seg_Buf[1]=13;
		Seg_Buf[2]=ucRtc_Trigger[Trigger_Index-1][0]/16;
		Seg_Buf[3]=ucRtc_Trigger[Trigger_Index-1][0]%16;
		Seg_Buf[4]=ucRtc_Trigger[Trigger_Index-1][1]/16;
		Seg_Buf[5]=ucRtc_Trigger[Trigger_Index-1][1]%16;
		Seg_Buf[6]=ucRtc_Trigger[Trigger_Index-1][2]/16;
		Seg_Buf[7]=ucRtc_Trigger[Trigger_Index-1][2]%16;
		Point[1]=0;
		Point[5]=0;
		

	}
	else	//返回原状态
	{
	if(Seg_Dis_Mode==0)		//处于显示
	{
		Seg_Buf[0]=ucRtc[0]/16;
		Seg_Buf[1]=ucRtc[0]%16;
		Seg_Buf[2]=14;	//-
		Seg_Buf[3]=ucRtc[1]/16;
		Seg_Buf[4]=ucRtc[1]%16;
		Seg_Buf[5]=14;	//-
		Seg_Buf[6]=ucRtc[2]/16;
		Seg_Buf[7]=ucRtc[2]%16;
		Point[1]=0;
		Point[5]=0;
	}
	
	if(Seg_Dis_Mode==1)	//数据
	{
		Seg_Buf[0]=11;
		Seg_Buf[1]=(unsigned char)Voltage_rd1;
		Seg_Buf[2]=(unsigned char)(Voltage_rd1*100)/10 %10;
		Seg_Buf[3]=(unsigned char)(Voltage_rd1*100)/100 %10;
		Seg_Buf[4]=15;		//U
		Seg_Buf[5]=(unsigned char)Voltage_rb2;
		Seg_Buf[6]=(unsigned char)(Voltage_rb2*100)/10 %10;
		Seg_Buf[7]=(unsigned char)(Voltage_rb2*100)/100 %10; 
		Point[1]=1;
		Point[5]=1;
	}
	
	if(Seg_Dis_Mode==2)	//查询界面
	{
		Seg_Buf[0]=12;	//A
		Point[1]=0;
		Point[5]=0;
		if(Seg_Scene==0)		//A1
		{
			Seg_Buf[1]=1;
			if(Trigger_Index>=3)		//已存入三组时间
			{

				Seg_Buf[2]=ucRtc_Trigger[2][0]/16;
				Seg_Buf[3]=ucRtc_Trigger[2][0]%16;
				Seg_Buf[4]=ucRtc_Trigger[2][1]/16;
				Seg_Buf[5]=ucRtc_Trigger[2][1]%16;
				Seg_Buf[6]=ucRtc_Trigger[2][2]/16;
				Seg_Buf[7]=ucRtc_Trigger[2][2]%16;
			}
			else
			{
				unsigned char i;
				for(i=2;i<8;i++)
					Seg_Buf[i]=14;	//-
			}
		}
	}
		if(Seg_Scene==1)		//A2
		{
			Seg_Buf[1]=2;
			if(Trigger_Index>=2)		//已存入两组时间
			{

				Seg_Buf[2]=ucRtc_Trigger[1][0]/16;
				Seg_Buf[3]=ucRtc_Trigger[1][0]%16;
				Seg_Buf[4]=ucRtc_Trigger[1][1]/16;
				Seg_Buf[5]=ucRtc_Trigger[1][1]%16;
				Seg_Buf[6]=ucRtc_Trigger[1][2]/16;
				Seg_Buf[7]=ucRtc_Trigger[1][2]%16;
			}
			else
			{
				unsigned char i;
				for(i=2;i<8;i++)
					Seg_Buf[i]=14;	//-
			}
		}

		if(Seg_Scene==2)		//A3
		{
			Seg_Buf[1]=3;
			if(Trigger_Index>=1)		//已存入一组时间
			{
				Seg_Buf[2]=ucRtc_Trigger[0][0]/16;
				Seg_Buf[3]=ucRtc_Trigger[0][0]%16;
				Seg_Buf[4]=ucRtc_Trigger[0][1]/16;
				Seg_Buf[5]=ucRtc_Trigger[0][1]%16;
				Seg_Buf[6]=ucRtc_Trigger[0][2]/16;
				Seg_Buf[7]=ucRtc_Trigger[0][2]%16;
			}
			else
			{
				unsigned char i;
				for(i=2;i<8;i++)
					Seg_Buf[i]=14;	//-
			}
		}		
	}

	
}

void LED_Proc()
{
	ucLED[0]=(Seg_Dis_Mode==0 && Trigger_Flag==0)? 1:0;
	ucLED[1]=(Seg_Dis_Mode==1 && Trigger_Flag==0)? 1:0;
	ucLED[2]=(Seg_Dis_Mode==2 && Trigger_Flag==0)? 1:0;
	ucLED[7]=(Trigger_Flag==1)? 1:0;

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
	
	++Timer_3000Ms;
	

	if(++Key_Slow_Down==10)
		Key_Slow_Down=0;			//按键减速

	if(++Seg_Slow_Down==100)
		Seg_Slow_Down=0;			//数码管减速
	
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
 Set_Rtc(ucRtc);
 Timer0Init();
 while(1)
 {
  Key_Proc();
  Seg_Proc();
  LED_Proc();
 }
}


