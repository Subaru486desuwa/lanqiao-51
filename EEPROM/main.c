/* ============================================
 * File: main.c
 * Project: EEPROM Test
 * Description: EEPROM read/write test with segment display
 * MCU: STC15F2K60S2
 * Optimized by: CC
 * Date: 2025-12-13
 * ============================================ */

#include <STC15F2K60S2.H>
#include <Seg.h>
#include <Key.h>
#include <LED.h>
#include <iic.h>

/* ============================================
 * Configuration Macros
 * ============================================ */
#define KEY_SCAN_PERIOD      10    // Key scan period: 10ms
#define SEG_REFRESH_PERIOD   500   // Segment refresh: 500ms
#define SEG_DIGIT_COUNT      8     // Number of segment digits
#define EEPROM_DATA_SIZE     2     // EEPROM data size
#define EEPROM_START_ADDR    0     // EEPROM start address

// Key codes for matrix keyboard
#define KEY_S4   19
#define KEY_S5   18
#define KEY_S6   17
#define KEY_S7   16
#define KEY_S8   15

/* ============================================
 * Global Variables
 * ============================================ */
// Segment display buffers
unsigned char Seg_Buf[SEG_DIGIT_COUNT] = {10,10,10,10,10,10,10,10};
unsigned char Seg_Pos = 0;
unsigned char Seg_Point[SEG_DIGIT_COUNT] = {0,0,0,0,0,0,0,0};

// LED control array
unsigned char ucLED[SEG_DIGIT_COUNT] = {0,0,0,0,0,0,0,0};

// Timing control
unsigned char Key_Slow_Down  = 0;
unsigned int  Seg_Slow_Down  = 0;

// Key state
unsigned char Key_Val  = 0;
unsigned char Key_Down = 0;
unsigned char Key_Old  = 0;

// EEPROM data storage
unsigned char dat[EEPROM_DATA_SIZE] = {30, 60};

/* ============================================
 * Segment Display Processing
 * ============================================ */
void Seg_Proc(void)
{
    if (Seg_Slow_Down) return;
    Seg_Slow_Down = 1;

    // Display first data value (left side)
    Seg_Buf[0] = dat[0] / 10 % 10;
    Seg_Buf[1] = dat[0] % 10;

    // Display second data value (right side)
    Seg_Buf[6] = dat[1] / 10 % 10;
    Seg_Buf[7] = dat[1] % 10;
}

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

    // Key event handling
    switch (Key_Down)
    {
        case KEY_S4:  // Decrease first value by 10
            if (dat[0] >= 10) dat[0] -= 10;
            break;

        case KEY_S5:  // Decrease second value by 10
            if (dat[1] >= 10) dat[1] -= 10;
            break;

        case KEY_S6:  // Increase first value by 10
            if (dat[0] <= 245) dat[0] += 10;
            break;

        case KEY_S7:  // Increase second value by 10
            if (dat[1] <= 245) dat[1] += 10;
            break;

        case KEY_S8:  // Write to EEPROM
            EEPROM_Write(dat, EEPROM_START_ADDR, EEPROM_DATA_SIZE);
            break;

        default:
            break;
    }
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

    // Segment refresh timing
    if (++Seg_Slow_Down == SEG_REFRESH_PERIOD)
        Seg_Slow_Down = 0;

    // Key scan timing (FIXED BUG: was Seg_Slow_Down=0)
    if (++Key_Slow_Down == KEY_SCAN_PERIOD)
        Key_Slow_Down = 0;

    // Segment position scanning
    if (++Seg_Pos == SEG_DIGIT_COUNT)
        Seg_Pos = 0;

    // Display current digit
    Seg_Dis(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    LED_Dis(Seg_Pos, ucLED[Seg_Pos]);
}

/* ============================================
 * Main Function
 * ============================================ */
void main(void)
{
    // Read data from EEPROM on startup
    EEPROM_Read(dat, EEPROM_START_ADDR, EEPROM_DATA_SIZE);

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
 * 1. CRITICAL BUG FIXED: Line 104 - Key_Slow_Down reset (was Seg_Slow_Down)
 * 2. CRITICAL BUG FIXED: Removed Timer0Init() call from ISR
 * 3. Added macro definitions for magic numbers
 * 4. Added bounds checking for data values
 * 5. Improved code structure and comments
 * 6. All comments converted to English
 * ============================================ */
