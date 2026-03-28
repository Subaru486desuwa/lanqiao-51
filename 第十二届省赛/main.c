/*ͷ�ļ�����*/
#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <iic.h>
#include <LED.h>
#include <onewire.h>

/*����������*/
unsigned char Key_Val,Key_Down,Key_Old; //��������ר�ò���
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};  //����ܶ��뻺������
unsigned char Point[8]={0,0,0,0,0,0,0,0}; //С����洢����
unsigned char Seg_Pos;//�����λ��

unsigned char ucLED[8]={0,0,0,0,0,0,0,0}; //LEDʹ�ܻ�������

unsigned int Seg_Slow_Down;//����ܼ���ר��500ms
unsigned char Key_Slow_Down;//��������ר��10ms

unsigned char Seg_Dis_Mode=0; //0-�¶���ʾģʽ 1-��������ģʽ 2-DAC�������

float temperature; //��ȡ�¶�
unsigned char temperature_set=25;	//�¶Ȳ���Ĭ��25��

unsigned char Set_Flag; //���ý����־λ
unsigned char Output_Mode; //0-ģʽ1 1-ģʽ2

float Seg_Output; //DAC�����ѹ

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
		++Seg_Dis_Mode;
		if(Seg_Dis_Mode==3) Seg_Dis_Mode=0;
		if(Seg_Dis_Mode==1)
		{
		 Set_Flag=1;
		}
		else Set_Flag=0;
	break;

	case 5:
		Output_Mode^=1;
	break;

	case 8:
		if(Set_Flag)
		{
		 ++temperature_set;
		}
	break;

	case 9:
		if(Set_Flag)
		{
		 --temperature_set;
		}
	break;
 }


}

/*����ܴ���*/
void Seg_Proc()
{
 if(Seg_Slow_Down) return;
 Seg_Slow_Down=1;



 if(Seg_Dis_Mode==0)	//�¶���ʾ����
 {
  	temperature=Read_t(); //��ȡ�¶�
  	Seg_Buf[0]=11; //C
	Seg_Buf[4]=(unsigned char)temperature /10 %10;
	Seg_Buf[5]=(unsigned char)temperature %10;
	Seg_Buf[6]=(unsigned char)(temperature*10) %10;
	Seg_Buf[7]=(unsigned char)(temperature*100) %10;
	Point[5]=1;
 }
 if(Seg_Dis_Mode==1)	//�������ý���
 {
	  Point[5]=0;
	  Seg_Buf[0]=12;	//P
	  Seg_Buf[4]=10;
	  Seg_Buf[5]=10;
	  Seg_Buf[6]=temperature_set /10 %10;
	  Seg_Buf[7]=temperature_set %10;
 }
 if(Seg_Dis_Mode==2)  	//DAC�������
 {
  	Seg_Buf[0]=13;	//A
	if(Output_Mode==0)	  //����ģʽ1
	{
	 if(temperature < temperature_set)
	 {
	  Da_Write(0x00);
	  Seg_Buf[5]=0;
	  Seg_Buf[6]=0;
	  Seg_Buf[7]=0;
	  Point[5]=1;

	 }
	 else
	 {
	  Da_Write(255);
	  Seg_Buf[5]=5;
	  Seg_Buf[6]=0;
	  Seg_Buf[7]=0;
	  Point[5]=1;
	 }	
	}
	if(Output_Mode==1)
	{
	 if(temperature<=20)
	 {
	  Da_Write(51);	//���1V
	  Seg_Buf[5]=1;
	  Seg_Buf[6]=0;
	  Seg_Buf[7]=0;
	  Point[5]=1;
	 }
	 // FIXED BUG: Missing "else if" - else was only for temperature>=40 case
	 else if(temperature>=40)
	 {
	  Da_Write(255);	//���1V
	  Seg_Buf[5]=5;
	  Seg_Buf[6]=0;
	  Seg_Buf[7]=0;
	  Point[5]=1;
	 }
	 else
	 {
	  Da_Write((unsigned char)((temperature*3/20.00 -2)*51));	//��������
	  Seg_Output=(temperature*3/20.00 -2);
	  Seg_Buf[5]=(unsigned char)Seg_Output%10;
	  Seg_Buf[6]=(unsigned char)(Seg_Output*10)%10;
	  Seg_Buf[7]=(unsigned char)(Seg_Output*100)%10;
	  Point[5]=1;
	 }

	}
 }

}

void LED_Proc()
{
	if(Output_Mode==0) ucLED[0]=1 ; //����ģʽ1ʱ������L1
	else ucLED[0]=0;

	if(Seg_Dis_Mode==0) ucLED[1]=1;	//�����¶���ʾ����ʱ������L2
	else ucLED[1]=0;

	if(Seg_Dis_Mode==1) ucLED[2]=1;	//���ڲ������ý���ʱ������L3
	else ucLED[2]=0;

	if(Seg_Dis_Mode==2) ucLED[3]=1;	//����DAC�������ʱ������L4
	else ucLED[3]=0;

}


/*��ʱ��0��ʼ������*/
void Timer0Init(void)		//1����@12.000MHz
{
	AUXR &= 0x7F;			//��ʱ��ʱ��12Tģʽ
	TMOD &= 0xF0;			//���ö�ʱ��ģʽ
	TL0 = 0x18;				//���ö�ʱ��ʼֵ
	TH0 = 0xFC;				//���ö�ʱ��ʼֵ
	TF0 = 0;				//���TF0��־
	TR0 = 1;				//��ʱ��0��ʼ��ʱ
	ET0=1;
	EA=1;
}


/*��ʱ��0����*/
void Timer0Server() interrupt 1
{
	TL0 = 0x18;				//���ö�ʱ��ʼֵ
	TH0 = 0xFC;				//���ö�ʱ��ʼֵ
	
	++Key_Slow_Down;
	if(Key_Slow_Down==10) Key_Slow_Down=0;			//��������
	
	++Seg_Slow_Down;
	if(Seg_Slow_Down==500) Seg_Slow_Down=0;			//����ܼ���
	
	if(++Seg_Pos==8)
	{
	 Seg_Pos=0;				   //�������ʾ
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


