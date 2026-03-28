/* ============================================
 * File: main.c
 * Project: DS1302 RTC Test
 * Description: Real-time clock display with DS1302
 * MCU: STC15F2K60S2
 * Optimized by: CC
 * Date: 2025-12-13
 * ============================================ */

#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <LED.h>
#include <ds1302.h>

/* ============================================
 * Configuration Macros
 * ============================================ */
#define KEY_SCAN_PERIOD      10     // Key scan period: 10ms
#define SEG_REFRESH_PERIOD   300    // Segment refresh: 300ms
#define SEG_DIGIT_COUNT      8      // Number of segment digits
#define RTC_DATA_SIZE        3      // RTC data: Hour, Minute, Second

// BCD conversion macros
#define BCD_HIGH_NIBBLE(x)   ((x) >> 4)
#define BCD_LOW_NIBBLE(x)    ((x) & 0x0F)

/* ============================================
 * Global Variables
 * ============================================ */
// Segment display buffers
unsigned char Seg_Buf[SEG_DIGIT_COUNT] = {10,10,10,10,10,10,10,10};
unsigned char Point[SEG_DIGIT_COUNT] = {0,0,0,0,0,0,0,0};
unsigned char Seg_Pos = 0;

// LED control array
unsigned char ucLED[SEG_DIGIT_COUNT] = {0,0,0,0,0,0,0,0};

// RTC data storage (BCD format: HH:MM:SS)
unsigned char ucRtc[RTC_DATA_SIZE] = {0x23, 0x59, 0x55};  // Default: 23:59:55

// Timing control
unsigned char Key_Slow_Down  = 0;
unsigned int  Seg_Slow_Down  = 0;

// Key state
unsigned char Key_Val  = 0;
unsigned char Key_Down = 0;
unsigned char Key_Old  = 0;

/* ============================================
 * Key Processing Function
 * ============================================ */
void Key_Proc(void)
{
    if (Key_Slow_Down) return;
    Key_Slow_Down = 1;

    // Read key value
    Key_Val = Key_Read();
    Key_Down = Key_Val & (Key_Val ^ Key_Old);
    Key_Old = Key_Val;

    // TODO: Add time adjustment logic if needed
}

/* ============================================
 * Segment Display Processing
 * ============================================ */
void Seg_Proc(void)
{
    if (Seg_Slow_Down) return;
    Seg_Slow_Down = 1;

    // Read RTC data (BCD format)
    Read_Rtc(ucRtc);

    // Convert BCD to decimal and display
    // Hour (HH)
    Seg_Buf[0] = BCD_HIGH_NIBBLE(ucRtc[0]);
    Seg_Buf[1] = BCD_LOW_NIBBLE(ucRtc[0]);

    // Minute (MM)
    Seg_Buf[3] = BCD_HIGH_NIBBLE(ucRtc[1]);
    Seg_Buf[4] = BCD_LOW_NIBBLE(ucRtc[1]);

    // Second (SS)
    Seg_Buf[6] = BCD_HIGH_NIBBLE(ucRtc[2]);
    Seg_Buf[7] = BCD_LOW_NIBBLE(ucRtc[2]);

    // Separator positions (2 and 5) remain as default (10 = off)
}

/* ============================================
 * LED Processing Function
 * ============================================ */
void LED_Proc(void)
{
    // TODO: Add LED control logic if needed
}

/* ============================================
 * Timer0 Initialization (1ms @ 12MHz)
 * ============================================ */
void Timer0Init(void)
{
    AUXR &= 0x7F;    // Timer clock: 12T mode
    TMOD &= 0xF0;    // Clear Timer0 mode bits
    TL0 = 0x18;      // Set initial value (low byte)
    TH0 = 0xFC;      // Set initial value (high byte)
    TF0 = 0;         // Clear overflow flag
    TR0 = 1;         // Start Timer0
    ET0 = 1;         // Enable Timer0 interrupt
    EA  = 1;         // Enable global interrupt
}

/* ============================================
 * Timer0 Interrupt Service Routine (1ms)
 * ============================================ */
void Timer0Server(void) interrupt 1
{
    // Reload timer value
    TL0 = 0x18;
    TH0 = 0xFC;

    // Key scan timing
    if (++Key_Slow_Down == KEY_SCAN_PERIOD)
        Key_Slow_Down = 0;

    // Segment refresh timing
    if (++Seg_Slow_Down == SEG_REFRESH_PERIOD)
        Seg_Slow_Down = 0;

    // Segment position scanning
    if (++Seg_Pos == SEG_DIGIT_COUNT)
        Seg_Pos = 0;

    // Display current digit
    Seg_Dis(Seg_Pos, Seg_Buf[Seg_Pos], Point[Seg_Pos]);
    LED_Dis(Seg_Pos, ucLED[Seg_Pos]);
}

/* ============================================
 * Main Function
 * ============================================ */
void main(void)
{
    // Initialize RTC with default time
    Set_Rtc(ucRtc);

    // Initialize hardware
    Timer0Init();

    // Main loop
    while (1)
    {
        Key_Proc();
        Seg_Proc();
        LED_Proc();
    }
}

/* ============================================
 * Optimization Notes by CC:
 * 1. Added macro definitions for magic numbers
 * 2. Added BCD conversion macros for clarity
 * 3. Improved code structure and readability
 * 4. All comments converted to English
 * 5. Added clear function documentation
 * 6. Defined RTC data size constant
 * ============================================ */
