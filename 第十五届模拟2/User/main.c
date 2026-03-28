/*头文件声明区*/
#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <iic.h>
#include <LED.h>
#include <onewire.h>

/*全局变量声明区域*/
unsigned char Key_Val,Key_Down,Key_Old; //按键消抖专用变量
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};  //数码管段码数据存储数组
unsigned char Point[8]={0,0,0,0,0,0,0,0}; //数码管小数点数据存储数组
unsigned char Seg_Pos;//数码管位选

unsigned char ucLED[8]={0,0,0,0,0,0,0,0}; //LED使能数据存储数组

unsigned int Seg_Slow_Down;//数码管减速专用变量-300ms
unsigned char Key_Slow_Down;//按键减速专用变量-10ms

float temperature;	//温度
unsigned char Lux;	//光照等级

bit Seg_Dis_Mode;	//模式界面切换
bit Seg_Mode;	//0-模式界面 1-输出界面

float Voltage_Output;	//处理参数，转换为DAC输出电压

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
			if(Seg_Dis_Mode==0)	//处于模式界面
			{
				Seg_Mode^=1;
			}
		break;
			
		case 5:
			Seg_Dis_Mode^=1;
		break;
	}

}

/*温度参数处理函数*/
float Temperature_Proc(float T)
{
	if((unsigned char)T<10)
		return 1;
	else if((unsigned char)T>40)
		return 5;
	else
	{
		return (T-10)*4/30+1;
	}
}

/*光敏参数处理函数*/
float Lux_Proc(unsigned char Lux)
{
	if(Lux<10)
		return 1;
	else if(Lux>240)
		return 5;
	else
	{
		return (float)(Lux-10)*4/230+1;
	}
}


/*数码管处理函数*/
void Seg_Proc()
{
  if(Seg_Slow_Down) return;
  Seg_Slow_Down=1;

	temperature=Read_Temperature();	//读取温度
	Lux=Ad_Read(0x41);	//读取光敏电阻数值
	
	if(Seg_Mode==0)	//模式1
	{
		Voltage_Output=Temperature_Proc(temperature);
	}
	else
	{
		Voltage_Output=Lux_Proc(Lux);
	}
	
	if(Seg_Dis_Mode==0)	//模式界面
	{
		if(Seg_Mode==0)	//温度
		{
			Seg_Buf[0]=1;
			Seg_Buf[5]=(unsigned char)temperature/10 %10;
			Seg_Buf[6]=(unsigned char)temperature %10;
			Seg_Buf[7]=(unsigned int)(temperature*10) %10;
			Point[6]=1;
		}
		if(Seg_Mode==1)	//光照
		{
			unsigned char i=5;
			Seg_Buf[0]=2;
			Seg_Buf[5]=Lux/100%10;
			Seg_Buf[6]=Lux/10%10;
			Seg_Buf[7]=Lux%10;
			Point[6]=0;

			while(Seg_Buf[i]==0)
			{
				Seg_Buf[i]=10;
				i++;
				if(i==7)	break;
			}
		}
	}
	else	//输出界面
	{
		Seg_Buf[0]=11; //U
		Seg_Buf[5]=10;
		Seg_Buf[6]=(unsigned char)Voltage_Output;
		Seg_Buf[7]=(unsigned int)(Voltage_Output*10)%10;
		Point[6]=1;
	}
	
}



/*LED处理函数*/
void LED_Proc()
{
	Da_Write((unsigned char)(51*Voltage_Output));		//DAC输出转换
	ucLED[0]=Seg_Mode==0;
	ucLED[1]=Seg_Mode==1;
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
	
	if(++Seg_Pos==8)
	{
	 Seg_Pos=0;				   //数码管位选
	}
	Seg_Dis(Seg_Pos,Seg_Buf[Seg_Pos],Point[Seg_Pos]);
	LED_Dis(Seg_Pos,ucLED[Seg_Pos]); 
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
	Read_Temperature();
	Delay750ms();
  Timer0Init();
  while(1)
  {
   Key_Proc();
   Seg_Proc();
   LED_Proc();
  }
}


