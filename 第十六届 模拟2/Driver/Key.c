#include <Key.h>

unsigned char Key_Read()
{
	unsigned char temp=0;      //temp每次都要重新赋初值
	ET0=0;			//关闭定时器0，防止串口冲突
	P30=0;P31=1;P32=1;P33=1;
	if(P34==0) temp=19;
	if(P35==0) temp=15;
	if(P42==0) temp=11;
	if(P44==0) temp=7;

	P30=1;P31=0;P32=1;P33=1;
	if(P34==0) temp=18;
	if(P35==0) temp=14;
	if(P42==0) temp=10;
	if(P44==0) temp=6;

	P30=1;P31=1;P32=0;P33=1;
	if(P34==0) temp=17;
	if(P35==0) temp=13;
	if(P42==0) temp=9;
	if(P44==0) temp=5;

	P30=1;P31=1;P32=1;P33=0;
	if(P34==0) temp=16;
	if(P35==0) temp=12;
	if(P42==0) temp=8;
	if(P44==0) temp=4;
	
	P3=0xff;
	ET0=1;
	return temp;
}