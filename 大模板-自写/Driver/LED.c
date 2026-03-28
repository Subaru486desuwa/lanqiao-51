#include <LED.h>

void LED_Dis(unsigned char addr,enable)
{
	static unsigned char temp=0x00;
	static unsigned char temp_old=0xff;
	if(enable)
		temp|=0x01<<addr;
	else
		temp&=~(0x01<<addr);
	if(temp_old!=temp)
	{
		P0=~temp;
		P2=P2&0x1f|0x80;
		P2&=0x1f;
		temp_old=temp;
	}
}

static unsigned char temp_1=0x00;
static unsigned char temp_old_1=0xff;

void Relay(unsigned char flag)
{

	if(flag)
		temp_1|=0x10;
	else
		temp_1&=~(0x10);
	if(temp_old_1!=temp_1)
	{
		P0=temp_1;
		P2=P2&0x1f|0xa0;
		P2&=0x1f;
		temp_old_1=temp_1;
	}
}

void Buzz(unsigned char flag)
{

	if(flag)
		temp_1|=0x40;
	else
		temp_1&=~(0x40);
	if(temp_old_1!=temp_1)
	{
		P0=temp_1;
		P2=P2&0x1f|0xa0;
		P2&=0x1f;
		temp_old_1=temp_1;
	}
}

void Motor(bit flag)
{
	if(flag)
		temp_1|=0x20;
	else
		temp_1&=~(0x20);
	
	if(temp_old_1!=temp_1)
	{
		P0=temp_1;
		P2=P2&0X1F|0XA0;
		P2&=0X1F;
		temp_old_1=temp_1;
	}
}


