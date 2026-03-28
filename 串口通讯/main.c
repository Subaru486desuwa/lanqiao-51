/* ============================================
 * File: main.c
 * Project: UART Communication Test
 * Description: UART send/receive test with interrupt
 * MCU: STC15F2K60S2
 * Optimized by: CC
 * Date: 2025-12-13
 * ============================================ */

#include <STC15F2K60S2.H>
#include <Key.h>
#include <Seg.h>
#include <LED.h>

/* ============================================
 * Configuration Macros
 * ============================================ */
#define KEY_SCAN_PERIOD      10    // Key scan period: 10ms
#define SEG_REFRESH_PERIOD   300   // Segment refresh: 300ms
#define SEG_DIGIT_COUNT      8     // Number of segment digits
#define UART_BUFFER_SIZE     10    // UART buffer size

/* ============================================
 * Global Variables
 * ============================================ */
// Segment display buffers
unsigned char Seg_Buf[SEG_DIGIT_COUNT] = {10,10,10,10,10,10,10,10};
unsigned char Point[SEG_DIGIT_COUNT]   = {0,0,0,0,0,0,0,0};
unsigned char Seg_Pos = 0;

// LED control array
unsigned char ucLED[SEG_DIGIT_COUNT] = {0,0,0,0,0,0,0,0};

// Timing control
unsigned char Key_Slow_Down  = 0;
unsigned int  Seg_Slow_Down  = 0;

// Key state
unsigned char Key_Val  = 0;
unsigned char Key_Down = 0;
unsigned char Key_Old  = 0;

// UART communication buffers
unsigned char Uart_Recv[UART_BUFFER_SIZE];              // Receive buffer
unsigned char Uart_Recv_Index = 0;                       // Receive index
unsigned char Uart_Send[UART_BUFFER_SIZE] = {"Hello!"}; // Send buffer

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

    // TODO: Update segment display based on application logic
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
 * UART1 Interrupt Service Routine
 * ============================================ */
void Uart1Server(void) interrupt 4
{
    if (RI == 1)
    {
        // Receive data
        if (Uart_Recv_Index < UART_BUFFER_SIZE)
        {
            Uart_Recv[Uart_Recv_Index] = SBUF;
            Uart_Recv_Index++;
        }

        // CRITICAL BUG FIXED: Was RI==0 (comparison), now RI=0 (assignment)
        RI = 0;  // Clear receive interrupt flag
    }
}

/* ============================================
 * Main Function
 * ============================================ */
void main(void)
{
    // Initialize UART
    UartInit();

    // Send greeting message
    Uart_Send_String(Uart_Send);

    // Initialize Timer0
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
 * 1. CRITICAL BUG FIXED: Line 93 - RI=0 (was RI==0, comparison instead of assignment)
 * 2. Added buffer overflow protection for UART receive
 * 3. Added macro definitions for configuration values
 * 4. Improved code structure and comments
 * 5. All comments converted to English
 * 6. Added TODO markers for empty functions
 * ============================================ */
