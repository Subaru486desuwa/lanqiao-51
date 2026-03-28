/*头文件声明*/
#include <STC15F2K60S2.H>
#include <LED.h>
#include <Seg.h>
#include <Key.h>
#include <iic.h>

/*全局变量声明*/
unsigned char Seg_Slow_Down;
unsigned char Key_Slow_Down;

unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};
unsigned char Seg_Pos;
unsigned char Point[8]={0,0,0,0,0,0,0,0};
unsigned char ucLED[8]={0,0,0,0,0,0,0,0};

unsigned char Key_Val,Key_Down,Key_Up,Key_Old;

bit Seg_Dis_Mode;//0-显示 1-回显

unsigned char Count;//0-255
unsigned char Count_EEPROM[1]={0};//0-255

unsigned int Timer2000Ms;	//2S
bit Timer_Trigger;

unsigned char Timer200Ms;
bit LED_Enable;
bit LED_Flag;

/*信息处理函数*/
void Seg_Proc()
{
	unsigned char i=5;	//高位熄灭
	
	/*信息获取*/
	if(Count%5==0 &&Count!=0)
		Da_Write(204);//4V
	else
		Da_Write(51);
	
	if(Seg_Slow_Down)	return;
	Seg_Slow_Down=1;
	
	if(Seg_Dis_Mode==0)
	{
		Seg_Buf[0]=11;
		Seg_Buf[5]=Count/100%10;
		Seg_Buf[6]=Count/10%10;
		Seg_Buf[7]=Count%10;
	}
	else
	{
		Seg_Buf[0]=12;
		Seg_Buf[5]=Count_EEPROM[0]/100%10;
		Seg_Buf[6]=Count_EEPROM[0]/10%10;
		Seg_Buf[7]=Count_EEPROM[0]%10;
	}

	while(Seg_Buf[i]==0)
	{
		Seg_Buf[i]=10;
		i++;
		if(i==7)	break;
	}
	
}
/*按键处理函数*/
void Key_Proc()
{
	if(Key_Slow_Down)	return;
	Key_Slow_Down=1;
	
	Key_Val=Key_Read();
	Key_Down=Key_Val&(Key_Val^Key_Old);
	// FIXED BUG: Key_Up detection was identical to Key_Down
	Key_Up=(~Key_Val)&(Key_Val^Key_Old);  // Correct: detects key release
	Key_Old=Key_Val;
	
	switch(Key_Down)
	{
		case 4:
			if(Seg_Dis_Mode==0)
			{
				Count++;
				if(Count==0)
					Count=255;
			}
		break;
			
		case 5:
			if(Seg_Dis_Mode==0)
			{
				Count--;
				if(Count==255)
					Count=0;
			}		
		break;
			
		case 9:
			Seg_Dis_Mode^=1;
		break;
	}
	
	if(Seg_Dis_Mode == 0)
	{
		if(Key_Down==8)
		{
			Timer_Trigger=1;
		}		
		
	}
	
	if(Seg_Dis_Mode==0)
	{
		if(Timer2000Ms<2000)	//短按
		{
			if(Key_Up == 8)
			{
				Count_EEPROM[0]=Count;
				EEPROM_Write(Count_EEPROM,0,1);
				Timer_Trigger=Timer2000Ms=0;				
			}
		}
		else
		{
			if(Key_Up == 8)
			{
				Count=0;
				Timer_Trigger=Timer2000Ms=0;
			}
		}		
	}


}

/*LED处理函数*/
void LED_Proc()
{
	if(Count>Count_EEPROM[0])
		LED_Flag=1;
	else
		LED_Flag=0;
	
	ucLED[0]=Seg_Dis_Mode==0;
	ucLED[1]=LED_Enable&&LED_Flag;
	
	
}
/*定时器0初始化函数*/
void Timer0Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0x18;		//设置定时初值
	TH0 = 0xFC;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0=1;
	EA=1;
}

/*定时器0中断服务函数*/
void Timer0Server() interrupt 1
{
	if(++Key_Slow_Down==10)
		Key_Slow_Down=0;
	
	if(++Seg_Slow_Down==100)
		Seg_Slow_Down=0;
	
	if(Timer_Trigger==1)
	{
		if(++Timer2000Ms==2002)
		{
			Timer2000Ms=2001;
		}
	}
	
	if(++Timer200Ms==200)
	{
		Timer200Ms=0;
		LED_Enable^=1;
	}

	
	if(++Seg_Pos==8)
		Seg_Pos=0;
	
	Seg_Dis(Seg_Pos,Seg_Buf[Seg_Pos],Point[Seg_Pos]);
	LED_Dis(Seg_Pos,ucLED[Seg_Pos]);
}
/*main函数*/
void main()
{
	EEPROM_Read(Count_EEPROM,0,1);
	Timer0Init();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		LED_Proc();
	}
}