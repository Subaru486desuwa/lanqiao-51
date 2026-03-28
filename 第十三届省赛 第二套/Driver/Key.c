#include <Key.h>

unsigned char Key_Read()
{
	unsigned char temp=0;      //temp每次都要重新赋初值
	//ET0=0;			//关闭定时器0，防止串口冲突
	if(P30==0) temp=7;
	if(P31==0) temp=6;
	if(P32==0) temp=5;
	if(P33==0) temp=4;

	
//	P3=0xff;
//	ET0=1;
	return temp;
}