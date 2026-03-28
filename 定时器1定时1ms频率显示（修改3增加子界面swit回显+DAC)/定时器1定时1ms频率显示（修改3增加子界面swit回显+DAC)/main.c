/*

定时器0计数模式；定时器中断模式
计555芯片产生的频率
按键S4，切换不同的显示界面；

*/



//#include <stc15f2k60s2.h>
#include "ext.h"
#include "intrins.h"
#include "delay.h"
//#include "ds18b20.h"
#include "ds1302.h"
#include "math.h"
#include "i2c.h" 

#define uchar unsigned char
#define uint unsigned int


uchar dspbuf[] = {
   10,10,10,10,10,10,10,10};
long int FRE;
uchar f_flag;

unsigned char dspcom = 0;  //显示位选

bit key_flag = 0;//按键扫描标志位
unsigned char key_value = 0xFF; //按键值

bit temper_flag = 0;//温度读取标志

unsigned char Seg_Disp_Mode=0;    ///界面显示模式：，频率参时间回显

idata unsigned char   freq;
signed int calibration=0; //??:-900~900  校准参数初始值0;定义有符号的整型
unsigned int freq_set=2000;//频率超限初始值
unsigned int freq_max=0;//记录频率最大值
bit	 freq_max_flag=0;	 
unsigned char *tim_max;//频率最大值时的时间指针	 
bit	 SetMode=0;
bit	 AddMode=0;	//加法标记
bit	 SubMode=0;	//减法法标记	 
//bit DateError=0;
	 
void time_display(void) //时钟显示
{
unsigned char *tim,i;
	tim=dspbuf;
  tim=ReadRTC();	
				{	 
					dspbuf[0]=*tim++;
					dspbuf[1]=*tim++;
          dspbuf[2]=15;
          dspbuf[3]=*tim++;	
          dspbuf[4]=*tim++;
          dspbuf[5]=15;
 					dspbuf[6]=*tim++;
					dspbuf[7]=*tim;	
	
															
         }
				tim=dspbuf;
}	 

void load()//频率显示缓冲区数据填充
{ unsigned char i=3;
  dspbuf[0] = 12;//F
	dspbuf[1] = 10;//灭
	dspbuf[2] = 10; 
	
if(FRE<0)  //频率小于0，显示错误
   {
	dspbuf[3]=10;	
	dspbuf[4] =10;
	dspbuf[5] = 10;
	dspbuf[6] =16;//L,0xc7
	dspbuf[7] =16;//L,0xc7;

    }
else
{
	dspbuf[3]=(FRE/10000);	
	dspbuf[4] = FRE/ 1000 % 10;
	dspbuf[5] = FRE /100 %10;;
	dspbuf[6] = FRE/10 % 10;;
	dspbuf[7] = FRE % 10;
}	
	//实现频率高位灭
while(dspbuf[i]==0)//为啥不灭
{
	dspbuf[i] = 10;//??
	i++;//????????
	if(i==7)//??????,?????????
		break;
}
	
}
void freq_time()//频率、时间最大值回显界面
{ unsigned char i=3;
	         dspbuf[0] = 13;//H      参数界面 
		      if(!SetMode)//频率回显，S5按下0，2，4次
	        { dspbuf[2] = 10; //灭
	         	dspbuf[1] = 12;  //F
						dspbuf[3] = 10;//灭	;
		        dspbuf[4] = freq_max / 1000 % 10;
		        dspbuf[5] = freq_max / 100 % 10;
		        dspbuf[6] = freq_max / 10 % 10;
		        dspbuf[7] = freq_max % 10;
						
//							//实现频率高位灭
//while(!dspbuf[i])//??????????????,????????0???????
//{
//	dspbuf[i] = 10;//??
//	i++;//????????
//	if(i == 7)//??????,?????????
//		break;
//}
		
						
	        }
	       else//时间回显，频率最大值时的时间  时间回显还有闪烁问题
	        { 
						//	tim_max=dspbuf;
            //  tim_max=ReadRTC();
							while(freq_max_flag==1)
			     {	
			       	freq_max_flag=0;
				      tim_max=dspbuf;       
				      tim_max=ReadRTC();
				      break;
          }
		        dspbuf[1] = 11;//A 						
            dspbuf[2]=*tim_max++;
            dspbuf[3]=*tim_max++;	
            dspbuf[4]=*tim_max++;
            dspbuf[5]=*tim_max++;
 					  dspbuf[6]=*tim_max++;
					  dspbuf[7]=*tim_max++;	
		        		             	
	      	}
	
}

void select(Seg_Disp_Mode) //S4
{
	switch(Seg_Disp_Mode)	
	{	case 0:load();//调用频率显示
		
		break;
		
		case 1: dspbuf[0] = 14;//P	      参数界面 
	          dspbuf[2] = 10; //灭
	          
		      if(!SetMode)//频率设置参数
	        {
						 if(AddMode==1)  //频率参数加、减法
							{ 
								AddMode=0;
						    freq_set+=1000;
					    }
	           if(SubMode==1)
						  { SubMode=0;
						    freq_set-=1000;
					    }
						dspbuf[1] = 1;
						dspbuf[3] = 10;//灭	
		        dspbuf[4] = freq_set / 1000 % 10;
		        dspbuf[5] = freq_set / 100 % 10;
		        dspbuf[6] = freq_set / 10 % 10;
		        dspbuf[7] = freq_set % 10;
	        }
	       else//校准参数界面
	        {
						 if(AddMode==1)  //校准参数参数加、减法
							{ 
								AddMode=0;
						    calibration+=100;
					    }
	           if(SubMode==1)
						  { SubMode=0;
						    calibration-=100;
					    }
					///////////////////////////////	
		           dspbuf[1] = 2;
		           if(calibration > 0)
		             {
		             	dspbuf[4] = 10;
			           dspbuf[5] = calibration / 100 % 10;
			           dspbuf[6] = calibration / 10 % 10;
			            dspbuf[7] = calibration % 10;
		              }
		          else if(calibration == 0)
		              {
			              dspbuf[4] = 10;
			               dspbuf[5] = 10;
			               dspbuf[6] = 10;
			               dspbuf[7] = 0;
		              }
		           else
		              {
		             	unsigned int positive = -calibration;
			               dspbuf[4] = 15;//'-'
			               dspbuf[5] = positive / 100 % 10;
			               dspbuf[6] = positive / 10 % 10;
			               dspbuf[7] = positive % 10;
		               }
	           }
			
	             	break;
		
		case 2:time_display();//时间显示
			
		break;
		
		case 3:freq_time();//回显
			
		break;
	}
}

//void ledmode()
//{ if(Seg_Disp_Mode==0)
//	{led(0xfE); //灯灭 L1
//     delay_ms(200);
//     led(0xff);  //灯亮
//     delay_ms(200);
//	}
//	if(freq_max_flag==1)
//	{ led(0xfd); //灯灭 L2
//     delay_ms(200);
//     led(0xff);  //灯亮
//     delay_ms(200);
//   }
// }	
void main()
{
 
	unsigned char dac_value=0;//DAC的值
  SetRTC();
  init_pcf8591();
	time0_init();
	time1_init();
  led(0xff); //灯灭
	while(1)
	{
    
		if(f_flag)
		{
   
			f_flag = 0;
			TR0 = 0;
			FRE = TH0 * 256 + TL0;
			FRE *= 2;
			TH0 = TL0 = 0;
			TR0 = 1;
			FRE=FRE-calibration;//校准后的频率
			if(FRE>freq_max)
			{ freq_max=FRE;
				freq_max_flag=1;			
			}
						
			if(key_flag==0)
				load();
		}
		 if(key_flag)   //按键处理
        {
            key_flag = 0;
            key_value=read_keyboard();
            if(key_value != 0xFF)
            {
               
							if(key_value==12)  //S4
							{ //S5_flag=1;	
								Seg_Disp_Mode++;
								key_value=0;
								if(Seg_Disp_Mode==4)
									Seg_Disp_Mode=0;
							 }						
								
						   if(key_value==8)   //S5
							     { SetMode=~SetMode	;									
										 key_value=0;																	 
									 }	
               if(key_value==13)//S8按键 ，加功能
							     { AddMode=1	;									
										 key_value=0;																	 
									 }	
    						if(key_value==9)//S9按键 ，减法     
									{ SubMode=1	;									
										 key_value=0;																	 
									 }	
    											 
	           }
          }

			select(Seg_Disp_Mode);
			
		  if(FRE<0)
				dac_pcf8591(0);	
       else if(FRE>freq_set) //大于超限频率
				 dac_pcf8591(255);	//255对应模拟输出4.69V	
       else if(FRE<500)
        dac_pcf8591(55);	//对应的模拟电压1V
			 else
			 { dac_value=((200.0/(freq_set-500.0))*(FRE-500.0))+55;
				 dac_pcf8591(dac_value);					 
			 }				 
	}
}

void time1() interrupt 3
{
   
	static unsigned int intf = 0;
	 static unsigned char intr = 0;	
   static unsigned char temp_t;
//	
//    if(++temp_t == 50)  //50ms执行一次
//    {
//        temp_t = 0;
//        temper_flag = 1;  //50ms温度读取标志位置1
//    }
//     
//    display( dspbuf);  //1ms执行一次
	if(++intr == 10)   //10
    {
        intr = 0;
        key_flag = 1;  //10ms按键扫描标志位置1
    }
	
	
	if(++intf == 500)//500ms
	{
   intf = 0;
		f_flag = 1;
	}
	display(dspbuf);
}




