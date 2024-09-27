
#include "F28x_Project.h"
#include <Grlib/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_F28379D_Crystalfontz128x128_ST7735.h"
#include "stdint.h"
#include "stdio.h"

//
// Defines
//
#define BLINKY_LED_GPIO    31

/* Graphic library context */
Graphics_Context g_sContext;
void drawTitle(void);
void drawAccelData(void);
/* ADC results buffer */
static uint16_t resultsBuffer[3] = {1,2,3};

void main(void)
{
//
// Step 1. Initialize System Control:
// PLL, WatchDog, enable Peripheral Clocks
// This example function is found in the F2837xD_SysCtrl.c file.
// Set SYSCLK to 200 MHz, disables watchdog timer, turns on peripheral clocks
    InitSysCtrl();


// Step 3. Clear all interrupts and initialize PIE vector table:
// Disable CPU interrupts
//
    DINT;

//
// Initialize the PIE control registers to their default state.
// The default state is all PIE interrupts disabled and flags
// are cleared.
// This function is found in the F2837xD_PieCtrl.c file.
//
    InitPieCtrl();

//
// Disable CPU interrupts and clear all CPU interrupt flags:
//
    IER = 0x0000;
    IFR = 0x0000;

//
// Initialize the PIE vector table with pointers to the shell Interrupt
// Service Routines (ISR).
// This will populate the entire table, even if the interrupt
// is not used in this example.  This is useful for debug purposes.
// The shell ISR routines are found in F2837xD_DefaultIsr.c.
// This function is found in F2837xD_PieVect.c.
//
    InitPieVectTable();

//
// Enable global Interrupts and higher priority real-time debug events:
//
    EINT;  // Enable Global interrupt INTM
    ERTM;  // Enable Global realtime interrupt DBGM
    EALLOW;
//
// Step 6. IDLE loop. Just sit and loop forever (optional):
//
    /* Initializes display */
    Crystalfontz128x128_Init();

    /* Set default screen orientation */
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

    /* Initializes graphics context */
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
    drawTitle();

    for(;;)
    {

    }
}

/*
 * Clear display and redraw title + accelerometer data
 */
void drawTitle()
{
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext,
                                    (int16_t *)"Accelerometer:",
                                    AUTO_STRING_LENGTH,
                                    64,
                                    30,
                                    OPAQUE_TEXT);
    drawAccelData();
}


/*
 * Redraw accelerometer data
 */
void drawAccelData()
{
    char string[8U];
    sprintf(string, "X: %5d", resultsBuffer[0U]);
    Graphics_drawStringCentered(&g_sContext,
                                    (int16_t *)string,
                                    8,
                                    64,
                                    50,
                                    OPAQUE_TEXT);

    sprintf(string, "Y: %5d", resultsBuffer[1U]);
    Graphics_drawStringCentered(&g_sContext,
                                    (int16_t *)string,
                                    8,
                                    64,
                                    70,
                                    OPAQUE_TEXT);

    sprintf(string, "Z: %5d", resultsBuffer[2U]);
    Graphics_drawStringCentered(&g_sContext,
                                    (int16_t *)string,
                                    8,
                                    64,
                                    90,
                                    OPAQUE_TEXT);
}

