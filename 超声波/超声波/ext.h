#ifndef __ext_H__
#define __ext_H__




void buzz(char n);
void led(char n);
void relay(char n);
//void time0_init();
void Timer0_Init(void);
void Timer1_Init(void) ;

void display(char disbuf[8])  ;
char read_keyboard(void);
void uart0_init(void);
void uart_sendstring(unsigned char *str); //´®¿Ú·¢ËÍ×Ö·û´®

#endif