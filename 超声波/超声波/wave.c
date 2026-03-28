

#include <stc15f2k60s2.h>
#include "intrins.h"
#include <wave.h>
 unsigned int dat_f;

//u8 Ultrasonic()
//{
//    u8 ult_num=10;
//    ult_tx = 0;
//    //??40kHZ 50%?????
//    TL1 = 0xF4;                
//    TH1 = 0xff;                
//    TR1 = 1;
//    
//    while(ult_num--)
//    {
//        while(TF1 == 0);
//        TF1 = 0;
//        ult_tx = !ult_tx;
//    }
//    TR1 = 0;
//    //等待接收
//    TL1 = 0;                
//    TH1 = 0;        
//    TR1 = 1;
//    //接收到信号或计数超时
//    while((ult_rx == 1) && (TF1 == 0));
//    TR1 = 0;
//    
//    if(TF1 == 1)
//    {
//        TF1 = 0;
//        ult_sign = 0;
//        return 0xff;
//    }
//    else
//    {
//        ult_sign = 1; 
//        return ((TH1<<8)|TL1)*0.017;
//    }
//    
//}