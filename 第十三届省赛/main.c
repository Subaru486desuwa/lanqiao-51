/*ͷ�ļ�����*/
#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <LED.h>
#include <onewire.h>
#include <ds1302.h>

/*����������*/
unsigned char Key_Val,Key_Down,Key_Old,Key_Up; //��������ר�ò���
unsigned char Seg_Buf[8]={10,10,10,10,10,10,10,10};  //����ܶ��뻺������
unsigned char Point[8]={0,0,0,0,0,0,0,0}; //С����洢����
unsigned char Seg_Pos;//�����λ��

unsigned char ucLED[8]={0,0,0,0,0,0,0,0}; //LEDʹ�ܻ�������

unsigned int Seg_Slow_Down;//����ܼ���ר��200ms
unsigned char Key_Slow_Down;//��������ר��10ms

unsigned char ucRtc[3]={0x23,0x59,0x55};//ds1302��ʼ�����ݴ洢����
float temperature;	//ds18b20��ȡ�¶�
unsigned char T_Set=23;		//��ǰ�¶Ȳ���
unsigned char Seg_Mode;//0-�¶���ʾ 1-ʱ����ʾ 2-��������

unsigned char Time_Dis_Flag;//0-ʱ�� 1-����
unsigned char Work_Mode;//0-�¶ȿ��� 1-ʱ�����
unsigned int Timer_5000Ms;	//5s
unsigned char Timer_100Ms;	//0.1s
unsigned char Work_Flag;	//0-�ر� 1-����
unsigned char LED_Flag;	//0-�ر� 1-����



/*��������*/
void Key_Proc()
{
 if(Key_Slow_Down) return;
 Key_Slow_Down=1;							 //��������

 Key_Val=Key_Read();
 Key_Down=Key_Val&(Key_Val^Key_Old);
 // FIXED BUG: Key_Up detection was identical to Key_Down
 Key_Up=(~Key_Val)&(Key_Val^Key_Old);  // Correct: detects key release
 Key_Old=Key_Val;						  //��������

 switch(Key_Down)
 {
  case 12:
  	++Seg_Mode;
	if(Seg_Mode==3)	Seg_Mode=0;
  break;

  case 13:
  	Work_Mode^=1;
  break;

  case 16:
  	if(Seg_Mode==2)	
	{
		++T_Set;
		if(T_Set==100)	T_Set=99;
	}
  break;

  case 17:
  	if(Seg_Mode==2)
	{
	 	--T_Set;
		if(T_Set==255)	T_Set=0;	
	}
  break;
 }

 if(Seg_Mode==1)
 {
 	if(Key_Old==17)
		Time_Dis_Flag=1;
	else
		Time_Dis_Flag=0;
 }


}

/*����ܴ���*/
void Seg_Proc()
{
 if(Seg_Slow_Down) return;
 Seg_Slow_Down=1;
 /*�������ý���*/
 temperature=Read_Temperature();
 Read_Rtc(ucRtc);


 if(Seg_Mode==0)		//�����¶���ʾ����
 {
  Seg_Buf[0]=11;	//U
  Seg_Buf[1]=1;
  Seg_Buf[3]=10;
  Seg_Buf[4]=10;
  Seg_Buf[5]=(unsigned char)temperature/10 %10;
  Seg_Buf[6]=(unsigned char)temperature %10;
  Seg_Buf[7]=(unsigned char)(temperature*10) %10;
  Point[6]=1;
 }

 if(Seg_Mode==1)		//ʱ����ʾ����
 {
 	if(Time_Dis_Flag==0)
	{
	  Seg_Buf[0]=11;//U
	  Seg_Buf[1]=2;
	  Seg_Buf[3]=ucRtc[0]/16;
	  Seg_Buf[4]=ucRtc[0]%16;
	  Seg_Buf[5]=12;//-
	  Seg_Buf[6]=ucRtc[1]/16;
	  Seg_Buf[7]=ucRtc[1]%16;
	  Point[6]=0;
	}
	else
	{
	  Seg_Buf[0]=11;//U
	  Seg_Buf[1]=2;
	  Seg_Buf[3]=ucRtc[1]/16;
	  Seg_Buf[4]=ucRtc[1]%16;
	  Seg_Buf[5]=12;//-
	  Seg_Buf[6]=ucRtc[2]/16;
	  Seg_Buf[7]=ucRtc[2]%16;
	  Point[6]=0;
	}
 }

 if(Seg_Mode==2)		//�������ý���
 {
  Seg_Buf[0]=11;//U
  Seg_Buf[1]=3;
  Seg_Buf[3]=10;
  Seg_Buf[4]=10;
  Seg_Buf[5]=10;
  Seg_Buf[6]=T_Set/10 %10;
  Seg_Buf[7]=T_Set%10;
  Point[6]=0;
 }

}

void LED_Proc()
{
 if(Work_Mode==0)		//�¶ȿ��ƽ���
 {
 if((unsigned char)temperature>T_Set || ((unsigned char)temperature==T_Set && (unsigned char)(temperature*10)%10)>0)
 	Relay(1);
 else
 	Relay(0);
 }
 else
 {
  if(Work_Flag==1)
  {
  	Relay(1);
	ucLED[0]=1;
  }
  else
  {
   Relay(0);
   ucLED[0]=0;
  }
 }

 if(Work_Mode==0) 	//�¶ȿ����� L2��
 	ucLED[1]=1;
 else
	ucLED[1]=0;
 
 if(Work_Flag==1 || (Work_Mode==0 &&(unsigned char)temperature>T_Set || ((unsigned char)temperature==T_Set && (unsigned char)(temperature*10)%10)>0))	//�̵������ڹ���״̬
 {
  	if(LED_Flag==1)
		ucLED[2]=1;
	if(LED_Flag==0)
		ucLED[2]=0;
 }
 else
 	ucLED[2]=0;
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
	if(Key_Slow_Down==10)
		Key_Slow_Down=0;			//��������
	
	++Seg_Slow_Down;
	if(Seg_Slow_Down==200)
		Seg_Slow_Down=0;			//����ܼ���

	if(ucRtc[1]/16==0&&ucRtc[1]%16==0&&ucRtc[2]/16==0&&ucRtc[2]%16==0)	//����
	{
	 Work_Flag=1;
	 Timer_5000Ms=0;
	}

	++Timer_5000Ms;
	if(Timer_5000Ms==5000)
	{
	 Timer_5000Ms=0;
	 Work_Flag=0;	//��λ
	}
		
	++Timer_100Ms;
	// FIXED BUG: Was checking if(Timer_100Ms) which is always true after first ms
	// Should check if it reaches 100ms
	if(Timer_100Ms==100)
	{
		Timer_100Ms=0;
		LED_Flag^=1;
	}
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
 Set_Rtc(ucRtc);
 Timer0Init();
 while(1)
 {
  Key_Proc();
  Seg_Proc();
  LED_Proc();
 }
}


