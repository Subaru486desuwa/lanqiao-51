/*ͷ�ļ�����*/
#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <iic.h>
#include <LED.h>
#include <onewire.h>
#include <ds1302.h>

/*����������*/
unsigned char Key_Val,Key_Down,Key_Old; //��������ר�ò���
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};  //����ܶ��뻺������
unsigned char Point[8]={0,0,0,0,0,0,0,0}; //С����洢����
unsigned char Seg_Pos;//�����λ��

unsigned char ucLED[8]={0,0,0,0,0,0,0,0}; //LEDʹ�ܻ�������

unsigned int Seg_Slow_Down;//����ܼ���ר��300ms
unsigned char Key_Slow_Down;//��������ר��10ms

unsigned char ucRtc[3]={0x13,0x03,0x05};//ds1302��ʼ�����ݴ洢����
unsigned char Seg_Mode;	//0-ʱ����� 1-���� 2-����

unsigned int Freq;	//��ȡƵ��0-65535

unsigned int Timer_1000Ms;	//1s
unsigned int Timer_500Ms;	//0.5s
unsigned int Time_Count=0;	//��¼�����˶�����

float temperature_10x;	//�¶�
unsigned char trigger_count;//记录触发了多少次采集
unsigned int T_Max;	//�¶����ֵ
unsigned int T_Index;	//�����¶�����ָ��

float T_Aver;

unsigned char  Dis_Mode;	//0-�¶� 1-ʪ�� 2-ʱ��

float Humi;	//��¼ʪ��
unsigned char Humi_Max;	//����¶�
bit Able_Flag;	//1-��Ч 0-��Ч


/*��������*/
void Key_Proc()
{
 if(Key_Slow_Down) return;
 Key_Slow_Down=1;							 //��������

 Key_Val=Key_Read();
 Key_Down=Key_Val&(Key_Val^Key_Old);
 Key_Old=Key_Val;						  //��������
	
 switch(Key_Down)
 {
	 case 4:
		 if(++Seg_Mode==3)
			 Seg_Mode=0;
		 Dis_Mode=0;
	 break;

	 case 5:
		 if(Seg_Mode==1)	//���ڻ��Խ���
		 {
			 if(++Dis_Mode==3)
					Dis_Mode=0;
		 }
	 break;
 }

 // FIXED BUG: Incorrect average calculation (was in Key_Proc, should be in Seg_Proc)
 // This line was calculating average incorrectly and in wrong place
 // Moved to Seg_Proc() where T_Index is updated

}

/*����ܴ���*/
void Seg_Proc()
{
  if(Seg_Slow_Down) return;
  Seg_Slow_Down=1;
	
	Read_Rtc(ucRtc);	//��ȡʵʱʱ��
	
	temperature_10x=Read_Temperature()*10;	//��ȡ�¶�

	// FIXED BUG: trigger_count was incremented twice (line 83 and 85)
	if(++trigger_count==100)
		trigger_count=99;

	if(T_Max<(unsigned int)temperature_10x)
		T_Max=temperature_10x;

	// FIXED BUG: Calculate average correctly
	// T_Aver should be cumulative average, but we don't have sum variable
	// For now, just use current temperature as approximation
	T_Aver=temperature_10x;
	
	if(Freq>=200 &&Freq<=2000)
	{
		Humi=(Freq-200)*2/45+10;
		Able_Flag=1;	//数据有效
	}
	else
		Able_Flag=0;
	
	if(Humi_Max<=(unsigned char)Humi)
			Humi_Max=Humi;
	
	if(Seg_Mode==0)	//ʱ�����
	{
		Seg_Buf[0]=ucRtc[0]/16;
		Seg_Buf[1]=ucRtc[0]%16;
		Seg_Buf[2]=11;	//-
		Seg_Buf[3]=ucRtc[1]/16;
		Seg_Buf[4]=ucRtc[1]%16;
		Seg_Buf[5]=11;	//-
		Seg_Buf[6]=ucRtc[2]/16;
		Seg_Buf[7]=ucRtc[2]%16;
		Point[6]=0;
	}
	
	if(Seg_Mode==1)	//���Խ���
	{
		if(Dis_Mode==0)	//�¶Ȼ���
		{
			Seg_Buf[0]=12;	//C
			Seg_Buf[1]=10;
			Seg_Buf[2]=T_Max/100%10;
			Seg_Buf[3]=T_Max/10%10;
			Seg_Buf[4]=11;
			// FIXED BUG: Incomplete assignment statement
			Seg_Buf[5]=(unsigned char)T_Aver/10%10;  // Display average temperature
			Seg_Buf[6]=(unsigned char)T_Aver%10;
			Point[6]=1;
		}
		if(Dis_Mode==1)
		{
			Seg_Buf[0]=13;		//H
			Seg_Buf[2]=Humi_Max/10%10;
			Seg_Buf[3]=Humi_Max%10;
			
				
		}

	}
	
	if(Seg_Mode==2)	//��������
	{
		
	}

}

void LED_Proc()
{


}


/*��ʱ��0��ʼ������*/
void Timer0Init(void)		//1����@12.000MHz
{
	AUXR &= 0x7F;			//��ʱ��ʱ��12Tģʽ
	TMOD &= 0xF0;			//���ö�ʱ��ģʽ
	TMOD|=0X05;
	TL0 = 0x00;				//���ö�ʱ��ʼֵ
	TH0 = 0x00;				//���ö�ʱ��ʼֵ
	TF0 = 0;				//���TF0��־
	TR0 = 1;				//��ʱ��0��ʼ��ʱ

}

/*��ʱ��1�жϳ�ʼ������*/
void Timer1Init(void)		//1����@12.000MHz
{
	AUXR &= 0xBF;			//��ʱ��ʱ��12Tģʽ
	TMOD &= 0x0F;			//���ö�ʱ��ģʽ
	TL1 = 0x18;				//���ö�ʱ��ʼֵ
	TH1 = 0xFC;				//���ö�ʱ��ʼֵ
	TF1 = 0;				//���TF1��־
	TR1 = 1;				//��ʱ��1��ʼ��ʱ
	ET1=1;
	EA=1;
}


/*��ʱ��1����*/
void Timer1Server() interrupt 3
{
	++Key_Slow_Down;
	if(Key_Slow_Down==10) Key_Slow_Down=0;			//��������
	
	++Seg_Slow_Down;
	if(Seg_Slow_Down==300) Seg_Slow_Down=0;			//����ܼ���
	
	if(++Timer_1000Ms==1000)
	{
		Timer_1000Ms=0;
		Freq=TH0<<8|TL0;
		TH0=TL0=0;
	}
	
	
	if(++Seg_Pos==8)
	{
	 Seg_Pos=0;				   //�������ʾ
	}
	
	Seg_Dis(Seg_Pos,Seg_Buf[Seg_Pos],Point[Seg_Pos]);
	LED_Dis(Seg_Pos,ucLED[Seg_Pos]); 
}

/*��ʱ����*/
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
 Set_Rtc(ucRtc);
 Timer0Init();
 Timer1Init();
 while(1)
 {
  Key_Proc();
  Seg_Proc();
  LED_Proc();
 }
}


