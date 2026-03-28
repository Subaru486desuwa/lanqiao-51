#include "STC15F2K60S2.H"
#include "intrins.h"

#define BAUD	       9600  		//²¨ÌØÂÊ
#define SYSTEMCLOCK  11059200L  //ÏµÍ³Ê±ÖÓÆµÂÊ
                            //  0    1      2      3     4     5     6     7    8    9      Ï¨Ã ð     11
//code unsigned char tab[] = { 0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0xFF,0xc1};
code unsigned char tab[] =    {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xff,0x88,0x8e,0x89,0x8c,0xBF,0xc7};//0-9,10-Ãð,11-A,12-F,13-H,14-P,15--


//unsigned char code SMG_duanma[18] = 
//  {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
//   0x88,0x80,0xc6,0xc0,0x86,0x8e,
//   0xbf,0x7f};//???0-9(????),A-F,“-”,“.”
	
//unsigned char dspbuf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; //ÏÔÊ¾»º³åÇø
//unsigned char dspcom = 0;  //ÏÔÊ¾Î»Ñ¡

//¹Ø±Õ·äÃùÆ÷
/*
//P0.6¿ØÖÆ P0 = 0x40 ;Ïì¬¸
           P0=0x00;²»Ïì
*/
void buzz(char n) //·äÃùÆ÷
{
    P2 = (P2 & 0x1F | 0xA0);
    P0 = n;//n=0x00£»²»Ïì
    P2 &= 0x1F;
}
void led(char n)  //·¢¹â¶þ¼«¹Ü
{
        P2 = ((P2 & 0x1f) | 0x80);
        P0 = n;  //LED¹Ø
        P2 &= 0x1f; //573Ëø´æÆ÷±£´æ0xff
    
}
void relay(char n) //¼ÌµçÆ÷
{
        P2 = ((P2 & 0x1f) | 0xA0);
        P0 = n;  //
        P2 &= 0x1f; //573Ëø´æÆ÷±£´æ0xff
    
}

void display(char dspbuf[8])  //ÊýÂë¹Ü
{
    static char com=0;
	 
	  P2 = ((P2 & 0x1f) | 0xe0);
    P0 = 0xff;
    P2 &= 0x1f;

    P2 = ((P2 & 0x1f) | 0xc0);//Î»Âë
    P0 = (1 << com);
    P2 &= 0x1f;

    P2 = ((P2 & 0x1f) | 0xe0);//¶ÎÂë
    P0 = tab[dspbuf[com]];
    P2 &= 0x1f;
    if(++com == 8)
    {
        com = 0;
    }
	}		
void Timer0_Init(void)        //1ms@12MHz ¶¨Ê±
{
    AUXR &= 0x7F;            //?????12T??
    TMOD &= 0xF0;            //???????
    TL0 = 0x18;                //???????
    TH0 = 0xFC;                //???????
    TF0 = 0;                //??TF0??
    TR0 = 1;                //???0????
    ET0 = 1;                //?????0??
    EA = 1;
}


void Timer1_Init(void)        //@12MHz
{
    AUXR &= 0xBF;            //?????12T??
    TMOD &= 0x0F;            //???????
//    TL1 = 0x18;                //???????
//    TH1 = 0xFC;                //???????
    TF1 = 0;                //??TF1??
//    TR1 = 1;                //???1????
}

void time2_init() //¶¨Ê±Æ÷2µÄ³õÊ¼»¯
{
    AUXR |= 0x04;
    T2L = 0x20;//12M¶¨Ê±Æ÷¶¨Ê±1ms
	  T2H = 0xD1;
	  AUXR |= 0x10;
    IE2 |= 0x04;
	  EA = 1;
}


char read_keyboard(void) //¶Á¼üÅÌ£¬·µ»Ø¼üÖµ
{
    static unsigned char hang;
    static unsigned char key_state = 0;
	  char key_value;
    switch(key_state)
    {
    case 0:
    {
        P3 = 0x0f;
        P42 = 0;
        P44 = 0;
        if(P3 != 0x0f) //ÓÐ°´¼ü°´ÏÂ
            key_state = 1;
    }
    break;
    case 1:  //ÓÐ¼ü°´ÏÂ
    {
        P3 = 0x0f;
        P42 = 0;
        P44 = 0;
        if(P3 != 0x0f) //ÓÐ°´¼ü°´ÏÂ
        {
            if(P30 == 0)hang = 1;
            if(P31 == 0)hang = 2;
            if(P32 == 0)hang = 3;
            if(P33 == 0)hang = 4;//È·¶¨ÐÐ
            switch(hang)
            {
            case 1:
            {
                P3 = 0xf0;
                P42 = 1;
                P44 = 1;
                if(P44 == 0)
                {
                    key_value = 0;
                    key_state = 2;
                }
                else if(P42 == 0)
                {
                    key_value = 1;
                    key_state = 2;
                }
                else if(P35 == 0)
                {
                    key_value = 2;
                    key_state = 2;
                }
                else if(P34 == 0)
                {
                    key_value = 3;
                    key_state = 2;
                }
            }
            break;
            case 2:
            {
                P3 = 0xf0;
                P42 = 1;
                P44 = 1;
                if(P44 == 0)
                {
                    key_value = 4;
                    key_state = 2;
                }
                else if(P42 == 0)
                {
                    key_value = 5;
                    key_state = 2;
                }
                else if(P35 == 0)
                {
                    key_value = 6;
                    key_state = 2;
                }
                else if(P34 == 0)
                {
                    key_value = 7;
                    key_state = 2;
                }
            }
            break;
            case 3:
            {
                P3 = 0xf0;
                P42 = 1;
                P44 = 1;
                if(P44 == 0)
                {
                    key_value = 8;
                    key_state = 2;
                }
                else if(P42 == 0)
                {
                    key_value = 9;
                    key_state = 2;
                }
                else if(P35 == 0)
                {
                    key_value = 10;
                    key_state = 2;
                }
                else if(P34 == 0)
                {
                    key_value = 11;
                    key_state = 2;
                }
            }
            break;
            case 4:
            {
                P3 = 0xf0;
                P42 = 1;
                P44 = 1;
                if(P44 == 0)
                {
                    key_value = 12;
                    key_state = 2;
                }
                else if(P42 == 0)
                {
                    key_value = 13;
                    key_state = 2;
                }
                else if(P35 == 0)
                {
                    key_value = 14;
                    key_state = 2;
                }
                else if(P34 == 0)
                {
                    key_value = 15;
                    key_state = 2;
                }
            }
            break;
            }
        }
        else
        {
            key_state = 0;
        }
    }
    break;
    case 2: 
    {
        P3 = 0x0f;
        P42 = 0;
        P44 = 0;
        if(P3 == 0x0f) //°´¼ü·Å¿ª
            key_state = 0;
    }
    break;

    }
		return key_value;

}

//??HC138
void SelectHC138(unsigned char channel)
{
  switch(channel)
  {
    case 4:
      P2 = (P2 & 0X1F) | 0X80;
    break;
    case 5:
      P2 = (P2 & 0X1F) | 0Xa0;
    break;
    case 6:
      P2 = (P2 & 0X1F) | 0Xc0;
    break;
    case 7:
      P2 = (P2 & 0X1F) | 0Xe0;
    break;
  }
}

void uart0_init(void) //´®¿Ú0µÄ³õÊ¼»¯
{
    SCON = 0x50;
    AUXR = 0x40;                //1T
    TMOD = 0x00;
    TL1 = (65536 - (SYSTEMCLOCK / 4 / BAUD));
    TH1 = (65536 - (SYSTEMCLOCK / 4 / BAUD)) >> 8;
    TR1 = 1;
    ES = 1;
    EA = 1;

}


//Í¨¹ý´®¿Ú·¢ËÍ×Ö·û´®
void uart_sendstring(unsigned char *str)
{
    unsigned char *p;
    p = str;
    while(*p != '\0')
    {
        SBUF = *p;
        while(TI == 0);  //µÈ´ý·¢ËÍ±êÖ¾Î»ÖÃÎ»
        TI = 0;
        p++;
    }
}

