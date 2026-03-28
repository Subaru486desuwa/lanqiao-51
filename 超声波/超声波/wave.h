
#ifndef __wave_H
#define __wave_H

#include <stc15f2k60s2.h>
sbit TX = P1^0;
sbit RX = P1^1;
 
 
extern unsigned int dat_f;
void Delay12us() ;
void Send_Wave()  ;
//void Ultrasonic();
#endif 