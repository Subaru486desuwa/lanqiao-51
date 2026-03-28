/*头文件声明区*/
#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <LED.h>
#include <onewire.h>
#include <math.h>

/*全局变量声明区域*/
unsigned char Key_Val,Key_Down,Key_Old; //按键消抖专用变量
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};  //数码管段码数据存储数组
unsigned char Point[8]={0,0,0,0,0,0,0,0}; //数码管小数点数据存储数组
unsigned char Seg_Pos;//数码管位选

unsigned char ucLED[8]={0,0,0,0,0,0,0,0}; //LED使能数据存储数组

unsigned int Seg_Slow_Down;//数码管减速专用变量-300ms
unsigned char Key_Slow_Down;//按键减速专用变量-10ms

float temperature;
float Adjust_temperature;

unsigned char Seg_Dis_Mode;//0-温度 1-校准 2-参数
bit Trigger_Mode;//0-上触发 1-下触发

char Adjust; //-99 99
char Canshu=26;//-99 99

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
			Trigger_Mode^=1;
		break;
		
		case 8:
			if(Seg_Dis_Mode==1)
			{
				--Adjust;
				if(Adjust==-100)	Adjust=-99;
			}
			if(Seg_Dis_Mode==2)
			{
				--Canshu;
				if(Canshu==-100)	Canshu=-99;
			}
		break;
			
		case 9:
			if(Seg_Dis_Mode==1)
			{
				++Adjust;
				if(Adjust==100)	Adjust=99;
			}
			if(Seg_Dis_Mode==2)
			{
				++Canshu;
				if(Canshu==100)	Canshu=99;
			}
		break;
	}
}

/*数码管处理函数*/
void Seg_Proc()
{
  if(Seg_Slow_Down) return;
  Seg_Slow_Down=1;
	
	temperature=Read_Temperature();//读取温度
	Adjust_temperature=temperature+Adjust;//显示温度
	
	if(Seg_Dis_Mode==0)
	{
		Seg_Buf[0]=11;
		Seg_Buf[5]=(unsigned char)Adjust_temperature/10%10;
		Seg_Buf[6]=(unsigned char)Adjust_temperature%10;
		Seg_Buf[7]=(unsigned int)(Adjust_temperature*10)%10;
		Point[6]=1;
	}
	else if(Seg_Dis_Mode==1)
	{
		Seg_Buf[0]=12;
		Point[6]=0;
		Seg_Buf[6]=abs(Adjust)/10%10;
		Seg_Buf[7]=abs(Adjust)%10;
		if(Adjust<0)	Seg_Buf[5]=14;
		else	Seg_Buf[5]=10;
	}
	else				//参数界面
	{
		Seg_Buf[0]=13;
		Seg_Buf[6]=abs(Canshu)/10%10;
		Seg_Buf[7]=abs(Canshu)%10;
		if(Canshu<0)	Seg_Buf[5]=14;
		else	Seg_Buf[5]=10;
	}

}

/*LED处理函数*/
void LED_Proc()
{
	ucLED[0]=(Seg_Dis_Mode==0);
	ucLED[1]=(Seg_Dis_Mode==1);
	ucLED[2]=(Seg_Dis_Mode==2);
	ucLED[3]=(Trigger_Mode==0);
	ucLED[4]=(Trigger_Mode==1);
	if(Trigger_Mode==0)//上触发模式
	{
		ucLED[7]=(Adjust_temperature>Canshu);
	}
	if(Trigger_Mode==1)
	{
		ucLED[7]=(Adjust_temperature<Canshu);
	}
	

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

/*延时850ms*/
void Delay850ms()		//@12.000MHz
{
	unsigned char i, j, k;

	i = 39;
	j = 195;
	k = 2;
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
	Delay850ms();
  Timer0Init();
  while(1)
  {
   Key_Proc();
   Seg_Proc();
   LED_Proc();
  }
}


