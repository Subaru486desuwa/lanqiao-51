#ifndef __DS1302_H
#define __DS1302_H

#include "STC15F2K60S2.H"
#include "intrins.h"

sbit SCK=P1^7;		
sbit SDA=P2^3;		
sbit RST = P1^3;   // DS1302??

void SetRTC(void);
unsigned char* ReadRTC(void);


#endif
