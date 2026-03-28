/*

13届省赛真题练习2

*/



#include <stc15f2k60s2.h>
#include <wave.h>
#include "ext.h"
//#include "intrins.h"
//#include "delay.h"
////#include "ds18b20.h"
//#include "ds1302.h"
//#include "math.h"
//#include "i2c.h" 
//#include "N555.h"
#define uchar unsigned char
#define uint unsigned int


uchar dspbuf[] = { 10,10,10,10,10,10,10,10};
//uchar dspbuf1[] = {0};//存储频率最大时的时间
//long int FRE;
//uchar f_flag;


bit key_flag = 0;//按键扫描标志位
unsigned char key_value = 0xFF; //按键值

bit temper_flag = 0;//温度读取标志
bit time_flag=0;//读时间的标记位


unsigned char Seg_Disp_Mode=0;    ///界面显示模式：，频率参时间回显

idata unsigned char   freq;
signed int calibration=0; //??:-900~900  校准参数初始值0;定义有符号的整型
unsigned int freq_set=2000;//频率超限初始值
unsigned int freq_max=0;//记录频率最大值
bit	 wave_flag=0;	 //定时发送超声波
bit	 freq_over_flag=0;////频率超限标志位
bit	 freq_max_count=0;//读频率最大值的时间标志位
bit  freq_sign=0;//频率为负标记位
	 
unsigned char *tim_max;//频率最大值时的时间指针	 
bit	 SetMode=0;
bit	 AddMode=0;	//加法标记
bit	 SubMode=0;	//减法法标记	

 bit ledMode=0;
 bit flag_L2 = 0;//L2定时0.2秒到标志
	 
//void time_display(void) //时钟显示
//{
//unsigned char *tim,i;
//	tim=dspbuf;
//  tim=ReadRTC();	
//				{	 
//					dspbuf[0]=*tim++;
//					dspbuf[1]=*tim++;
//          dspbuf[2]=15;
//          dspbuf[3]=*tim++;	
//          dspbuf[4]=*tim++;
//          dspbuf[5]=15;
// 					dspbuf[6]=*tim++;
//					dspbuf[7]=*tim;	
//	
//											
//         }
//				tim=dspbuf;
//}	 

//void load()//频率显示缓冲区数据填充
//{ unsigned char i=3;
//  dspbuf[0] = 12;//F
//	dspbuf[1] = 10;//灭
//	dspbuf[2] = 10; 
//	
//if(FRE<0)  //频率小于0，显示错误
//   { 
//	dspbuf[3]=10;	
//	dspbuf[4] =10;
//	dspbuf[5] = 10;
//	dspbuf[6] =16;//L,0xc7
//	dspbuf[7] =16;//L,0xc7;
//  freq_sign=1;//频率为负标记
//    }
//else
//{ freq_sign=0;
//	dspbuf[3]=(FRE/10000);	
//	dspbuf[4] = FRE/ 1000 % 10;
//	dspbuf[5] = FRE /100 %10;;
//	dspbuf[6] = FRE/10 % 10;;
//	dspbuf[7] = FRE % 10;
//}	
//	//实现频率高位灭
//while(i<8)//为啥不灭
//{ 
//	if(dspbuf[i]!=0)//??????,?????????	 
//		break;
//	dspbuf[i] = 10;//??
//	i++;//????????	
//}
//	
//}
//void freq_time()//频率、时间最大值回显界面
//{ unsigned char i=3;
//	         dspbuf[0] = 13;//H      参数界面 
//		      if(!SetMode)//频率回显，S5按下0，2，4次
//	        { dspbuf[2] = 10; //灭
//	         	dspbuf[1] = 12;  //F
//						dspbuf[3] = 10;//灭	;
//		        dspbuf[4] = freq_max / 1000 % 10;
//		        dspbuf[5] = freq_max / 100 % 10;
//		        dspbuf[6] = freq_max / 10 % 10;
//		        dspbuf[7] = freq_max % 10;
//					
//	        }
//	       else//时间回显，频率最大值时的时间  时间回显还有闪烁问题
//	        { 
//							
//						dspbuf[2]=dspbuf1[0];
//            dspbuf[3]=dspbuf1[1];	
//            dspbuf[4]=dspbuf1[2];
//            dspbuf[5]=dspbuf1[3];
// 					  dspbuf[6]=dspbuf1[4];
//					  dspbuf[7]=dspbuf1[5];			
//						 
//		        dspbuf[1] = 11;//A 						
//  	      	
//				}
//}

//void select(Seg_Disp_Mode) //S4
//{
//	switch(Seg_Disp_Mode)	
//	{	case 0:load();//调用频率显示
//		
//		break;
//		
//		case 1: dspbuf[0] = 14;//P	      参数界面 
//	          dspbuf[2] = 10; //灭
//	          
//		      if(!SetMode)//频率设置参数
//	        {
//						 if(AddMode==1)  //频率参数加、减法
//							{ 
//								AddMode=0;
//						    freq_set+=1000;
//					    }
//	           if(SubMode==1)
//						  { SubMode=0;
//						    freq_set-=1000;
//					    }
//						dspbuf[1] = 1;
//						dspbuf[3] = 10;//灭	
//		        dspbuf[4] = freq_set / 1000 % 10;
//		        dspbuf[5] = freq_set / 100 % 10;
//		        dspbuf[6] = freq_set / 10 % 10;
//		        dspbuf[7] = freq_set % 10;
//	        }
//	       else//校准参数界面
//	        {
//						 if(AddMode==1)  //校准参数参数加、减法
//							{ 
//								AddMode=0;
//						    calibration+=100;
//					    }
//	           if(SubMode==1)
//						  { SubMode=0;
//						    calibration-=100;
//					    }
//					///////////////////////////////	
//		           dspbuf[1] = 2;
//		           if(calibration > 0)
//		             {
//		             	dspbuf[4] = 10;
//			           dspbuf[5] = calibration / 100 % 10;
//			           dspbuf[6] = calibration / 10 % 10;
//			            dspbuf[7] = calibration % 10;
//		              }
//		          else if(calibration == 0)
//		              {
//			              dspbuf[4] = 10;
//			               dspbuf[5] = 10;
//			               dspbuf[6] = 10;
//			               dspbuf[7] = 0;
//		              }
//		           else
//		              {
//		             	unsigned int positive = -calibration;
//			               dspbuf[4] = 15;//'-'
//			               dspbuf[5] = positive / 100 % 10;
//			               dspbuf[6] = positive / 10 % 10;
//			               dspbuf[7] = positive % 10;
//		               }
//	           }
//			
//	             	break;
//		
//		case 2:time_display();//时间显示界面
//			
//		break;
//		
//		case 3:freq_time();//回显界面
//			
//		break;
//	}
//}

//void led_disp()   //led灯显示要求
//{  
//	if(freq_over_flag)//当前频率大于超限频率，L2闪烁
//	{ if(ledMode)
//		led(0xfd); //灯灭 L2  //led(0xfd); //灯灭 L2
//    else
//     led(0xff);  //灯亮    
//   }
//	else
//		led(0xff);
//	
//	
//	if(freq_sign) //频率为负,L2一直亮
//	 led(0xfd);
///********************************************************/	
//	
//	if(Seg_Disp_Mode==0)//频率界面L1闪烁，0.2秒间隔  。2个灯一起控制，为啥L1会暗？
//	{
//		 if(ledMode)
//		  led(0xff); // L1灭			 
//     else
//			 led(0xfe);    
//	}
//	else
//		led(0xff);  //其他界面，灭
///////////////////////////////////////////////////////////////////////////	
//	 
//	
// }	
//void freq_proc() //频率显示处理函数
//{
//	if(f_flag) //定时1秒标志
//		{
//      FRE=Get_Frequency();
//			f_flag = 0;
////			TR0 = 0;
////			FRE = TH0 * 256 + TL0;
////			FRE *= 2;
////			TH0 = TL0 = 0;
////			TR0 = 1;
//			FRE=FRE-calibration;//校准后的频率
//		if(FRE-freq_max>FRE*0.1) //计算最大值频率，当前频率的最大值
//		//计算最大值频率，当前频率的最大值	FRE-freq_max>FRE*0.1
//			{  freq_max=FRE;			
//				 tim_max=dspbuf1;       
//				 tim_max=ReadRTC();
//				
//				    dspbuf1[0]=*tim_max++;
//            dspbuf1[1]=*tim_max++;	
//            dspbuf1[2]=*tim_max++;
//            dspbuf1[3]=*tim_max++;
// 					  dspbuf1[4]=*tim_max++;
//					  dspbuf1[5]=*tim_max;
//						
//			}
//						
//       if(FRE>freq_set)
//				 freq_over_flag=1;		//频率超限标志	
//			 else
//				 freq_over_flag=0;
//				}
//}
//	

#include <STC15F2K60S2.H>
#include <stdio.H>
#define u8 unsigned char
#define u16 unsigned int
sbit ult_tx = P1^0;
sbit ult_rx = P1^1;
code unsigned char Seg_Table[] = 
{
0xc0, //0
0xf9, //1
0xa4, //2
0xb0, //3
0x99, //4
0x92, //5
0x82, //6
0xf8, //7
0x80, //8
0x90, //9
0x88, //A
0x83, //b
0xc6, //C
0xa1, //d
0x86, //E
0x8e //F
};
 
u16 ms_count;

u8 COD[8],COT[9],PSI,seg_delay;

u8 ult_sign;
void All_Close();
void Timer0_Init(void);
void SEG_Proc();
void Timer1_Init(void);
u8 Ultrasonic();

void main()
{  u8 show_num;
 
    Timer0_Init();
    Timer1_Init();
    while(1)
    {
        SEG_Proc();	
			
    }
}
 
 
 

 

/*******************???***************************/
void Timer0_Isr(void) interrupt 1    //定时器0定时中断1ms
{
      ms_count++;
    if(ms_count == seg_delay)
			 seg_delay = 0;
    if(ms_count == 1000)
			 ms_count = 0;
    
		 display(dspbuf); 
}
 

 

/******************???*************************/
u8 Ultrasonic()
{
    u8 ult_num=10;
    ult_tx = 0;
    //??40kHZ 50%?????
    TL1 = 0xF4;                
    TH1 = 0xff;                
    TR1 = 1;
    
    while(ult_num--)
    {
        while(TF1 == 0);
        TF1 = 0;
        ult_tx = !ult_tx;
    }
    TR1 = 0;
    //等待接收
    TL1 = 0;                
    TH1 = 0;        
    TR1 = 1;
    //接收到信号或计数超时
    while((ult_rx == 1) && (TF1 == 0));
    TR1 = 0;
    
    if(TF1 == 1)
    {
        TF1 = 0;
        ult_sign = 0;
        return 0xff;
    }
    else
    {
        ult_sign = 1; 
        return ((TH1<<8)|TL1)*0.017;
    }
    
}
/*************************************************/
void SEG_Proc()
{
    u8 show_num;
    if(seg_delay) return;
    seg_delay = 1000;
    
    show_num = Ultrasonic();
    if(ult_sign == 1)
		{// sprintf(COT,"     %3u",(u16)show_num);
				dspbuf[5]=show_num/100;
			  dspbuf[6]=show_num%100/10;
			  dspbuf[7]=show_num%10;		
		}
		
    else
		{ dspbuf[0]=12;
			dspbuf[5]=10;
		  dspbuf[6]=10;
		  dspbuf[7]=10;	
		}
		
}


 

 
