// 包含 STC15F2K60S2 单片机的头文件，定义了该型号单片机的寄存器
#include <STC15F2K60S2.H>

// 包含自定义的头文件
#include "init.h" // 包含系统初始化函数
#include "led.h"  // 包含 LED 驱动函数
#include "seg.h"  // 包含数码管驱动函数
#include "key.h"  // 包含按键扫描函数

#include "ds1302.h"		// 包含 DS1302 实时时钟芯片驱动函数
#include "onewire.h"	// 包含 OneWire 单总线通信协议，用于 DS18B20 温度传感器
#include "iic.h"		// 包含 IIC 通信协议，用于 EEPROM 和 AD/DA 转换器
#include "ultrasound.h" // 包含超声波模块驱动函数
#include "uart.h"		// 包含串口通信函数

#include "string.h" // 包含字符串处理函数库
#include "stdio.h"	// 包含标准输入输出函数库，如 printf 和 sscanf

/* ================= 全局变量定义 ================= */

// `pdata` 关键字指定变量存储在分页外部 RAM 中
// LED 显示数组，每个元素对应一个 LED 的亮(1)灭(0)状态
pdata unsigned char ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// 数码管显示缓冲区，共8位数码管。初始值10表示不显示任何数字（熄灭）
pdata unsigned char Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10};
// `idata` 关键字指定变量存储在内部 RAM 中
// 数码管动态扫描的当前位置（0-7）
idata unsigned char Seg_Pos = 0;
// 数码管处理函数的减速计数器，用于降低CPU占用率
idata unsigned char Seg_Slow_Down;

// 按键处理相关变量
idata unsigned char Key_Val, Key_Old, Key_Up, Key_Down; // 分别存储当前键值、旧键值、弹起键值、按下键值
idata unsigned char Key_Slow_Down;						// 按键扫描的减速计数器，用于按键消抖

// RTC实时时钟数据存储数组，ucRtc[0]存时, ucRtc[1]存分, ucRtc[2]存秒
pdata unsigned char ucRtc[3] = {11, 12, 13};
// RTC时间读取函数的减速计数器
idata unsigned char Time_Slow_Down;

// 温度值变量，存放的是实际温度值乘以10之后的结果，以处理一位小数
idata unsigned int Temperature_10x;
// 温度读取函数的减速计数器
idata unsigned int Temperature_Slow_Down;

// AD转换通道1和通道3的数据，同样乘以100以处理两位小数
idata unsigned int AD_1_Data_100x, AD_3_Data_100x;
// AD/DA处理函数的减速计数器
idata unsigned char AD_DA_Slow_Down;

// EEPROM 写入和读取的数据缓冲区
pdata unsigned char EEPROM_Data_W[8] = {1, 2, 3, 4, 5, 6, 7, 8};
pdata unsigned char EEPROM_Data_R[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// 超声波测得的距离值（单位：厘米）
idata unsigned char Distance;
// 超声波读取函数的减速计数器
idata unsigned char Distance_Slow_Down;

// 频率计测得的频率值
idata unsigned int Freq;
// 用于产生1秒时间基准的计数器
idata unsigned int Time_1s;

// PWM（脉宽调制）相关变量，用于LED调光
idata unsigned char pwm_period;		 // PWM周期计数器
idata unsigned char pwm_compare = 6; // PWM比较值，决定占空比，即LED亮度（0-9）

// 串口接收相关变量
idata unsigned char Uart_Rx_Index;									  // 串口接收缓冲区当前索引
pdata unsigned char Uart_Rx_Buf[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 串口接收数据缓冲区
idata unsigned char Uart_Rx_Flag;									  // 串口接收状态标志位
idata unsigned char Uart_Rx_Tick;									  // 串口接收超时计时器，用于判断一帧数据是否接收完毕

// 数码管显示模式标志
// 0:时间, 1:温度, 2:AD转换, 3:超声波, 4:频率, 5:PWM参数
idata unsigned char Seg_Show_Mode;

/*
 * @brief 按键处理函数
 * @note  此函数在主循环中被调用，通过减速器实现按键消抖。
 * 检测按键的按下和释放事件，并根据不同的按键执行相应操作。
 */
void Key_Proc()
{
	// 减速计数，每10ms进入一次，实现按键消抖
	if (Key_Slow_Down < 10)
		return;
	Key_Slow_Down = 0;

	// 读取当前按键状态
	Key_Val = Key_Read();
	// 通过位运算计算出刚刚被按下的键（下降沿检测）
	Key_Down = Key_Val & (Key_Val ^ Key_Old);
	// 通过位运算计算出刚刚被弹起的键（上升沿检测）
	Key_Up = ~Key_Val & (Key_Val ^ Key_Old);
	// 更新旧键值，为下一次检测做准备
	Key_Old = Key_Val;

	// 如果S4键被按下
	if (Key_Down == 4)
		// PWM比较值加1，并对10取模，实现亮度0-9循环调节
		pwm_compare = (++pwm_compare) % 10;

	// 如果有任何按键被按下，通过串口打印键值，用于调试
	if (Key_Down != 0)
		printf("Key_Down: %bu", Key_Down);

	// 如果S5键被按下
	if (Key_Down == 5)
		// 数码管显示模式加1，并对6取模，实现0-5种模式循环切换
		Seg_Show_Mode = (++Seg_Show_Mode) % 6;
}

/*
 * @brief 数码管显示处理函数
 * @note  根据 Seg_Show_Mode 的值，将不同的数据格式化到 Seg_Buf 缓冲区中，以便在数码管上显示。
 */
void Seg_Proc()
{
	// 减速计数，每20ms进入一次，避免频繁刷新
	if (Seg_Slow_Down < 20)
		return;
	Seg_Slow_Down = 0;

	// 根据当前显示模式，准备不同的显示内容
	switch (Seg_Show_Mode)
	{
	case 0:							// 时间显示界面
		Seg_Buf[0] = ucRtc[0] / 10; // 时(十位)
		Seg_Buf[1] = ucRtc[0] % 10; // 时(个位)
		Seg_Buf[2] = 0 + ',';		// 显示一个分隔符(小数点)
		Seg_Buf[3] = ucRtc[1] / 10; // 分(十位)
		Seg_Buf[4] = ucRtc[1] % 10; // 分(个位)
		Seg_Buf[5] = 0 + ',';		// 显示一个分隔符(小数点)
		Seg_Buf[6] = ucRtc[2] / 10; // 秒(十位)
		Seg_Buf[7] = ucRtc[2] % 10; // 秒(个位)
		break;

	case 1:											  // 温度显示界面
		Seg_Buf[0] = Temperature_10x / 100;			  // 温度(十位)
		Seg_Buf[1] = Temperature_10x / 10 % 10 + ','; // 温度(个位)并附加小数点
		Seg_Buf[2] = Temperature_10x % 10;			  // 温度(小数位)
		Seg_Buf[3] = 10;							  // 熄灭
		Seg_Buf[4] = 10;							  // 熄灭
		Seg_Buf[5] = 10;							  // 熄灭
		Seg_Buf[6] = 10;							  // 熄灭
		Seg_Buf[7] = 10;							  // 熄灭
		break;

	case 2:											  // AD转换界面
		Seg_Buf[0] = AD_1_Data_100x / 100 % 10 + ','; // 通道1(个位)并附加小数点
		Seg_Buf[1] = AD_1_Data_100x / 10 % 10;		  // 通道1(小数位)
		Seg_Buf[2] = AD_1_Data_100x % 10;			  // 通道1(小数位)
		Seg_Buf[3] = 10;							  // 熄灭
		Seg_Buf[4] = 10;							  // 熄灭
		Seg_Buf[5] = AD_3_Data_100x / 100 % 10 + ','; // 通道3(个位)并附加小数点
		Seg_Buf[6] = AD_3_Data_100x / 10 % 10;		  // 通道3(小数位)
		Seg_Buf[7] = AD_3_Data_100x % 10;			  // 通道3(小数位)
		break;

	case 3:								 // 超声波界面
		Seg_Buf[0] = Distance / 100;	 // 距离(百位)
		Seg_Buf[1] = Distance / 10 % 10; // 距离(十位)
		Seg_Buf[2] = Distance % 10;		 // 距离(个位)
		Seg_Buf[3] = 10;				 // 熄灭
		Seg_Buf[4] = 10;				 // 熄灭
		Seg_Buf[5] = 10;				 // 熄灭
		Seg_Buf[6] = 10;				 // 熄灭
		Seg_Buf[7] = 10;				 // 熄灭
		break;

	case 4: // 频率界面
		// 动态显示频率值，高位为0时不显示
		Seg_Buf[0] = (Freq > 10000000) ? Freq / 10000000 % 10 : 10;
		Seg_Buf[1] = (Freq > 1000000) ? Freq / 1000000 % 10 : 10;
		Seg_Buf[2] = (Freq > 100000) ? Freq / 100000 % 10 : 10;
		Seg_Buf[3] = (Freq > 10000) ? Freq / 10000 % 10 : 10;
		Seg_Buf[4] = (Freq > 1000) ? Freq / 1000 % 10 : 10;
		Seg_Buf[5] = (Freq > 100) ? Freq / 100 % 10 : 10;
		Seg_Buf[6] = (Freq > 10) ? Freq / 10 % 10 : 10;
		Seg_Buf[7] = Freq % 10;
		break;

	case 5:						  // PWM参数界面
		Seg_Buf[0] = pwm_compare; // 显示当前PWM比较值（亮度等级）
		Seg_Buf[1] = 10;		  // 熄灭
		Seg_Buf[2] = 10;		  // 熄灭
		Seg_Buf[3] = 10;		  // 熄灭
		Seg_Buf[4] = 10;		  // 熄灭
		Seg_Buf[5] = 10;		  // 熄灭
		Seg_Buf[6] = 10;		  // 熄灭
		Seg_Buf[7] = 10;		  // 熄灭
		break;
	}
}

/*
 * @brief LED处理函数
 * @note  设置 ucLed 数组中的值来控制LED灯的亮灭。实际的亮灭控制在Timer1中断中完成以实现PWM调光。
 */
void Led_Proc()
{
	// 设置一个交替亮灭的模式
	ucLed[0] = 1;
	ucLed[1] = 0;
	ucLed[2] = 1;
	ucLed[3] = 0;
	ucLed[4] = 1;
	ucLed[5] = 0;
	ucLed[6] = 1;
	ucLed[7] = 0;
	// Led_Disp(ucLed); // 实际的显示操作被移到了Timer1中断服务程序中，以实现PWM调光
}

/*
 * @brief 获取RTC时间函数
 * @note  每100ms读取一次DS1302芯片的时间。
 */
void Get_Time()
{
	// 减速计数，每100ms进入一次
	if (Time_Slow_Down < 100)
		return;
	Time_Slow_Down = 0;

	// 调用驱动函数，从DS1302读取时间到ucRtc数组
	Read_Rtc(ucRtc);
}

/*
 * @brief 获取温度函数
 * @note  每300ms读取一次DS18B20传感器的温度。
 */
void Get_Temperature()
{
	// 减速计数，每300ms进入一次
	if (Temperature_Slow_Down < 300)
		return;
	Temperature_Slow_Down = 0;

	// 读取温度值，并乘以10，存入变量
	Temperature_10x = rd_temperature() * 10;
}

/*
 * @brief AD/DA处理函数
 * @note  每120ms读取一次光敏电阻和电位器的AD值，并设置DA输出。
 */
void AD_DA()
{
	// 减速计数，每120ms进入一次
	if (AD_DA_Slow_Down < 120)
		return;
	AD_DA_Slow_Down = 0;

	// 读取AIN1通道(光敏电阻)的AD值，并进行转换，结果乘以10
	// 0x41是PCF8591 AIN1通道的控制字节
	// 原始值0-255，乘以10再除以51，得到0-5.0V对应的0-50的数值
	AD_1_Data_100x = (float)Ad_Read(0x41) / 51.0f * 100;
	// 读取AIN3通道(电位器)的AD值
	AD_3_Data_100x = (float)Ad_Read(0x43) / 51.0f * 100;
	// 设置DA输出电压为3V (3 * 51)
	Da_Write(3 * 51);
}

/*
 * @brief 获取超声波距离函数
 * @note  每150ms启动一次超声波测距。
 */
void Get_Distance()
{
	// 减速计数，每150ms进入一次
	if (Distance_Slow_Down < 150)
		return;
	Distance_Slow_Down = 0;
	// 获取超声波模块返回的距离数据
	Distance = Ut_Wave_Data();
}

/*
 * @brief 串口数据处理函数
 * @note  当接收到数据后，通过超时机制判断一帧数据是否结束，然后进行解析。
 */
void Uart_Proc()
{
	unsigned char x, y;
	// 如果接收缓冲区为空，则直接返回
	if (Uart_Rx_Index == 0)
		return;

	// 超时判断，如果距离上一个字符接收超过10ms，则认为一帧数据接收完成
	if (Uart_Rx_Tick >= 10)
	{
		// 清除接收标志和超时计数器
		Uart_Rx_Flag = 0;
		Uart_Rx_Tick = 0;

		// 将接收到的数据回显到串口，用于确认接收内容是否正确
		printf("%s", Uart_Rx_Buf);

		// 使用sscanf尝试按"(x,y)"的格式解析缓冲区数据
		if (sscanf(Uart_Rx_Buf, "(%bu,%bu)", &x, &y) == 2)
			// 如果成功解析出2个值，打印解析结果
			printf("\r\nI Get x=%bu, y=%bu\r\n", x, y);
		else
			// 如果格式不匹配，打印错误信息
			printf("\r\nERROR\r\n");

		// 清空接收缓冲区，为下一次接收做准备
		memset(Uart_Rx_Buf, 0, Uart_Rx_Index);
		Uart_Rx_Index = 0;
	}
}

/*
 * @brief 定时器0初始化函数
 * @note  配置为计数器模式，用于测量外部脉冲频率。
 */
void Timer0_Init(void) // 1毫秒@12.000MHz
{
	AUXR &= 0x7F; // 定时器时钟12T模式
	TMOD &= 0xF0; // 清空T0的模式位
	TMOD |= 0x05; // 设置T0为模式1（16位），并配置为计数器模式（C/T=1），对T0(P3.4)引脚的外部脉冲计数
	TL0 = 0x00;	  // 设置计数初始值低8位
	TH0 = 0x00;	  // 设置计数初始值高8位
	TF0 = 0;	  // 清除T0溢出标志
	TR0 = 1;	  // 启动定时器0开始计数
}

/*
 * @brief 定时器1初始化函数
 * @note  配置为定时器模式，每1ms触发一次中断，作为系统的基本时钟滴答。
 */
void Timer1_Init(void) // 1毫秒@12.000MHz
{
	AUXR &= 0xBF; // 定时器时钟12T模式
	TMOD &= 0x0F; // 清空T1的模式位，设置为模式1（16位定时器）
	// 计算初值: 65536 - 12MHz / 12 / 1000Hz = 64536 = 0xFC18
	TL1 = 0x18; // 设置定时初始值低8位
	TH1 = 0xFC; // 设置定时初始值高8位
	TF1 = 0;	// 清除T1溢出标志
	TR1 = 1;	// 启动定时器1
	ET1 = 1;	// 使能定时器1中断
	EA = 1;		// 开启总中断
}

/*
 * @brief 定时器1中断服务程序
 * @note  中断号为3。这是整个系统的“心跳”，每1ms执行一次。
 */
void Timer1_Isr(void) interrupt 3
{
	// 各种任务的减速计数器自增
	Seg_Slow_Down++;
	Key_Slow_Down++;
	Time_Slow_Down++;
	Temperature_Slow_Down++;
	AD_DA_Slow_Down++;
	Distance_Slow_Down++;

	// --- 数码管动态扫描 ---
	Seg_Pos = (++Seg_Pos) % 8; // 扫描位置循环移位
	// 判断显示缓冲区中的值是否需要显示小数点
	// 通过给数字加上一个字符','的ASCII码来实现，这是一个技巧
	if (Seg_Buf[Seg_Pos] > 20)
		// 显示数字（减去偏移）并点亮小数点
		Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos] - ',', 1);
	else
		// 正常显示数字，不点亮小数点
		Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], 0);

	// --- 频率计算 ---
	// 每1000ms（1秒）执行一次
	if (++Time_1s == 1000)
	{
		Time_1s = 0; // 1秒计时器清零
		// 读取定时器0的计数值，即为过去1秒内的脉冲数（频率Hz）
		Freq = (TH0 << 8) | TL0;
		// 清零定时器0，准备下一个周期的计数
		TH0 = TL0 = 0;
	}

	// --- LED软件PWM调光 ---
	// PWM周期计数器自增，对10取模，形成一个10ms的PWM周期
	pwm_period = (++pwm_period) % 10;
	// 在一个PWM周期内，当周期计数器小于比较值时，点亮LED
	if (pwm_period < pwm_compare)
		Led_Disp(ucLed); // 根据ucLed数组的状态点亮LED
	else
		// 否则，熄灭所有LED
		Led_Off();

	// 如果串口正在接收数据，超时计数器自增
	if (Uart_Rx_Flag)
		Uart_Rx_Tick++;
}

/*
 * @brief 串口1中断服务程序
 * @note  中断号为4。当串口接收到数据时触发。
 */
void Uart1_Isr(void) interrupt 4
{
	// 判断是否是接收中断（RI标志位置1）
	if (RI)
	{
		Uart_Rx_Flag = 1; // 置起接收标志，通知主循环开始超时计时
		Uart_Rx_Tick = 0; // 重置超时计数器
		// 将接收到的数据从SBUF寄存器读到自定义的缓冲区中
		Uart_Rx_Buf[Uart_Rx_Index++] = SBUF;
		RI = 0; // 手动清除接收中断标志位

		// 防止缓冲区溢出
		if (Uart_Rx_Index > 10)
		{
			Uart_Rx_Index = 0;
			memset(Uart_Rx_Buf, 0, 10);
		}
	}
}

/*
 * @brief 主函数
 */
void main()
{
	// 系统底层初始化（时钟等）
	System_Init();
	// 设置DS1302的初始时间
	Set_Rtc(ucRtc);

	// EEPROM读写测试
	EEPROM_Read(EEPROM_Data_R, 0, 8);  // 读取原始数据
	EEPROM_Write(EEPROM_Data_W, 0, 8); // 写入新数据
	EEPROM_Read(EEPROM_Data_R, 0, 8);  // 再次读取以验证写入

	// 初始化各个模块
	Timer0_Init(); // 初始化定时器0（频率计）
	Uart1_Init();  // 初始化串口
	Timer1_Init(); // 初始化定时器1（系统时钟）

	// 进入主循环
	while (1)
	{
		// 循环调用各个任务处理函数
		Key_Proc();		   // 处理按键输入
		Seg_Proc();		   // 处理数码管显示逻辑
		Led_Proc();		   // 处理LED显示逻辑
		Get_Time();		   // 获取实时时间
		Get_Temperature(); // 获取温度
		AD_DA();		   // 处理AD/DA转换
		Get_Distance();	   // 获取超声波距离
		Uart_Proc();	   // 处理串口接收的数据
	}
}