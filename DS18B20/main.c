/* ============================================
 * File: main.c
 * Project: DS18B20 Temperature Sensor Test
 * Description: Temperature measurement with DS18B20
 * MCU: STC15F2K60S2
 * Optimized by: CC
 * Date: 2025-12-13
 * ============================================ */

#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <onewire.h>

/* ============================================
 * Configuration Macros
 * ============================================ */
#define KEY_SCAN_PERIOD      10     // Key scan period: 10ms
#define SEG_REFRESH_PERIOD   500    // Segment refresh: 500ms
#define SEG_DIGIT_COUNT      8      // Number of segment digits

/* ============================================
 * Global Variables
 * ============================================ */
// Timing control
unsigned int  Seg_Slow_Down  = 0;
unsigned char Key_Slow_Down  = 0;

// Key state
unsigned char Key_Val  = 0;
unsigned char Key_Down = 0;
unsigned char Key_Old  = 0;

// Segment display buffers
unsigned char Seg_Buf[SEG_DIGIT_COUNT] = {10,10,10,10,10,10,10,10};
unsigned char Seg_Point[SEG_DIGIT_COUNT] = {0,0,0,0,0,0,0,0};
unsigned char Seg_Pos = 0;

// Temperature value
float t = 0.0;

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

    // TODO: Add key event handling if needed
}

/* ============================================
 * Segment Display Processing
 * ============================================ */
void Seg_Proc(void)
{
    if (Seg_Slow_Down) return;
    Seg_Slow_Down = 1;

    // Read temperature from DS18B20
    t = Read_t();

    // Display format: XX.X (e.g., 25.6°C)
    Seg_Buf[0] = (unsigned char)t / 10 % 10;   // Tens digit
    Seg_Buf[1] = (unsigned char)t % 10;         // Ones digit
    Seg_Buf[2] = (unsigned int)(t * 10) % 10;   // Decimal digit

    // Set decimal point after ones digit
    Seg_Point[1] = 1;
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

    // Segment refresh timing
    if (++Seg_Slow_Down == SEG_REFRESH_PERIOD)
        Seg_Slow_Down = 0;

    // Key scan timing
    if (++Key_Slow_Down == KEY_SCAN_PERIOD)
        Key_Slow_Down = 0;

    // Segment position scanning
    if (++Seg_Pos == SEG_DIGIT_COUNT)
        Seg_Pos = 0;

    // Display current digit
    Seg_Dis(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
}

/* ============================================
 * Main Function
 * ============================================ */
void main(void)
{
    // Initialize hardware
    Timer0Init();

    // Main loop
    while (1)
    {
        Key_Proc();
        Seg_Proc();
    }
}

/* ============================================
 * Optimization Notes by CC:
 * 1. Added macro definitions for magic numbers
 * 2. Improved code structure and readability
 * 3. All comments converted to English
 * 4. Added clear function documentation
 * 5. Fixed variable declaration order
 * ============================================ */
