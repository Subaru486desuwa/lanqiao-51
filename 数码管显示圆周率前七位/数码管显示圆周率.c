#include "reg52.h"
#include "intrins.h"

code unsigned char tab[] = {0xc0,0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90,0x7f};

void delay(unsigned int t)
{
	while(t--);
	while(t--);
}

void Delay10ms()		//@11.0592MHz
{
	unsigned char i, j;

	i = 18;
	j = 235;
	do
	{
		while (--j);
	} while (--i);
}





void pai()
{
 

 P2=(P2&0x1f)|0xc0; //Y6控制显示的位置
 P0=0x01;    
 P2=(P2&0x1f)|0xe0;  //Y7控制显示的数字
 P0=tab[3];			   //数字3

Delay10ms();


 P2=(P2&0x1f)|0xc0; //Y6控制显示的位置
 P0=0x02;			
 P2=(P2&0x1f)|0xe0;  //Y7控制显示的数字
 P0=tab[10];			  //小数点

Delay10ms();
 


 P2=(P2&0x1f)|0xc0; //Y6控制显示的位置
 P0=0x04;					
 P2=(P2&0x1f)|0xe0;  //Y7控制显示的数字
 P0=tab[1];			  //数字1

Delay10ms();

 

 P2=(P2&0x1f)|0xc0; //Y6控制显示的位置
 P0=0x08;
 P2=(P2&0x1f)|0xe0;  //Y7控制显示的数字
 P0=tab[4];			   //数字4

Delay10ms();

 


 	    
 P2=(P2&0x1f)|0xc0; //Y6控制显示的位置
 P0=0x10;
 P2=(P2&0x1f)|0xe0;  //Y7控制显示的数字
 P0=tab[1];				   //数字1

Delay10ms();

 P2=(P2&0x1f)|0xc0; //Y6控制显示的位置
 P0=0x20;  
 P2=(P2&0x1f)|0xe0;  //Y7控制显示的数字
 P0=tab[5];
 					 //数字5
Delay10ms();


 P2=(P2&0x1f)|0xc0; //Y6控制显示的位置
 P0=0x40; 
 P2=(P2&0x1f)|0xe0;  //Y7控制显示的数字
 P0=tab[9];				  //数字9

Delay10ms();


   


 P2=(P2&0x1f)|0xc0; //Y6控制显示的位置
 P0=0x80;  
 P2=(P2&0x1f)|0xe0;  //Y7控制显示的数字
 P0=tab[2];				  //数字2

Delay10ms();



}






main()
{
	while(1)
	{
		pai();
	}

}