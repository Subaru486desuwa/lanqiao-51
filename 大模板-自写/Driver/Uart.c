#include <Uart.h>

/* 串口初始化函数 */
void UartInit(void)		//9600bps@12.000MHz
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x01;		//串口1选择定时器2为波特率发生器  定时器0-可能用于NE555计数 定时器1-计数	定时器PCA- 超声波		/0-计数 PCA超声波
	AUXR |= 0x04;		//定时器时钟1T模式
	T2L = 0xC7;		//设置定时初始值
	T2H = 0xFE;		//设置定时初始值
	AUXR |= 0x10;		//定时器2开始计时
	ES = 1;
	EA = 1;
}

/*putchar重定向*/
extern char putchar (char ch)		//串口发送时只需用printf()	十分方便	无需声明	
{
	SBUF= ch;
	while(TI==0);		//TI=1	发送完毕
	TI=0;
	return ch;		
}
