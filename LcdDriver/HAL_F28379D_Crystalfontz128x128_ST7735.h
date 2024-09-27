#include <stdint.h>
#include "F2837xD_Device.h"
#include "F2837xD_Examples.h"
//*****************************************************************************
//
// User Configuration for the LCD Driver
//
//*****************************************************************************


//*****************************************************************************
//
// Prototypes for the globals exported by this driver.
//
//*****************************************************************************
extern void HAL_LCD_writeCommand(uint16_t command);
extern void HAL_LCD_writeData(uint16_t data);
extern void HAL_LCD_PortInit(void);
extern void HAL_LCD_SpiInit(void);
extern void HAL_LCD_rstOutputLow(void);
extern void HAL_LCD_rstOutputHigh(void);
void HAL_LCD_csOutputLow(void);
void HAL_LCD_csOutputHigh(void);
void setLcdCmdMode(void);
void setLcdDataMode(void);

#define HAL_LCD_delay(x)                       \
    do {                                       \
        DELAY_US((long double)x * 1000.0L);    \
    } while (0U)


