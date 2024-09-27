#include <Grlib/grlib/grlib.h>
#include <LcdDriver/HAL_F28379D_Crystalfontz128x128_ST7735.h>
#include <stdint.h>

void HAL_LCD_PortInit(void)
{
    /* SPI B Module and GPIO Configuration */
    /* Pin 26   -> SPI  CS
     * Pin 63   -> SPIB MOSI
     * Pin 64   -> SPIB MISO
     * Pin 65   -> SPIB CLK
     * Pin 130  -> LCD RST
     * Pin 158  -> LCD DC
     */
    EALLOW;

    /* SPI B MOSI pin configuration */
    GpioCtrlRegs.GPBGMUX2.bit.GPIO63 = 0b11; // MUX pin for SPI
    GpioCtrlRegs.GPBMUX2.bit.GPIO63  = 0b11; // MUX pin for SPI
    GpioCtrlRegs.GPBDIR.bit.GPIO63   = 0b01; // SPIB SIMO pin direction is set to output
    GpioCtrlRegs.GPBQSEL2.bit.GPIO63 = 0b11; // No synchronization or input qualification

    /* SPI B MISO pin configuration not used for LCD */
    GpioCtrlRegs.GPCGMUX1.bit.GPIO64 = 0b11;
    GpioCtrlRegs.GPCMUX1.bit.GPIO64  = 0b11;
    GpioCtrlRegs.GPCQSEL1.bit.GPIO64 = 0b11;

    /* SPIB CLK pin configuration*/
    GpioCtrlRegs.GPCGMUX1.bit.GPIO65 = 0b11;   // MUX pin for SPI
    GpioCtrlRegs.GPCMUX1.bit.GPIO65  = 0b11;   // MUX pin for SPI
    GpioCtrlRegs.GPCDIR.bit.GPIO65   = 0b01;   // SPIB CLK pin direction is set to output
    GpioCtrlRegs.GPCQSEL1.bit.GPIO65 = 0b11;   // No synchronization or input qualification

    /* SPI CS pin configuration */
    GpioCtrlRegs.GPAGMUX2.bit.GPIO26  = 0b00;  // GPIO configuration
    GpioCtrlRegs.GPAMUX2.bit.GPIO26   = 0b00;  // GPIO configuration
    GpioDataRegs.GPASET.bit.GPIO26    = 0b01;  // Load output latch
    GpioCtrlRegs.GPADIR.bit.GPIO26    = 0b01;  // Set pin direction to output

    /* LCD RST pin configuration */
    GpioCtrlRegs.GPEGMUX1.bit.GPIO130  = 0b00;  // GPIO configuration
    GpioCtrlRegs.GPEMUX1.bit.GPIO130   = 0b00;  // GPIO configuration
    GpioDataRegs.GPESET.bit.GPIO130    = 0b01;  // Load output latch to one
    GpioCtrlRegs.GPEDIR.bit.GPIO130    = 0b01;  // Set pin direction to output

    /* LCD DC pin configuration */
    GpioCtrlRegs.GPCGMUX1.bit.GPIO66 = 0b00;   // GPIO configuration
    GpioCtrlRegs.GPCMUX1.bit.GPIO66  = 0b00;   // GPIO configuration
    GpioCtrlRegs.GPCDIR.bit.GPIO66 = 1;        //CS0 is set as an output
    GpioDataRegs.GPCSET.bit.GPIO66 = 1;        //latch to a 1

    EDIS;
}

void HAL_LCD_SpiInit(void)
{
    /* SPI B Module and GPIO Configuration */
    /* Pin 26   -> SPI  CS
     * Pin 63   -> SPIB MOSI
     * Pin 64   -> SPIB MISO
     * Pin 65   -> SPIB CLK
     * Pin 130  -> LCD RST
     * Pin 158  -> LCD DC
     */
    EALLOW;

    /* Configure SPI settings here*/
    /* Clear the SPI Software Reset bit (SPISWRESET) to 0 to force the SPI to the reset state. */

    SpibRegs.SPICCR.bit.SPISWRESET   = 0U;      // Set to zero to make changes
    SpibRegs.SPICTL.bit.MASTER_SLAVE = 1U;      // Set to MASTER mode
    SpibRegs.SPICCR.bit.CLKPOLARITY  = 0U;      // Rising edge with delay. Data is latched on SPI clock rising edge
    SpibRegs.SPICTL.bit.CLK_PHASE    = 1U;      // Rising edge with delay
    ClkCfgRegs.LOSPCP.bit.LSPCLKDIV  = 0U;      // LSPCLK = 200MHz / 1
    SpibRegs.SPIBRR.bit.SPI_BIT_RATE = 99U;     // Bauadrate = 200MHz / (99 + 1) = 2 MHz
    //SpibRegs.SPICCR.bit.HS_MODE = 1;             //enable the high speed mode
    SpibRegs.SPICCR.bit.SPICHAR      = 7U;      // 8 bit word
    SpibRegs.SPICTL.bit.TALK         = 1U;      // Enable transmission (Talk)
    SpibRegs.SPIPRI.bit.FREE         = 1U;      // Emulation Free. Halting on a breakpoint will not halt the SPI
    SpibRegs.SPICCR.bit.SPISWRESET   = 1U;      // Release the SPI from reset


    EDIS;
}


//*****************************************************************************
//
// Writes a command to the CFAF128128B-0145T.  This function implements the basic SPI
// interface to the LCD display.
//
//*****************************************************************************
void HAL_LCD_writeCommand(const uint16_t data)
{
    /* Set to command mode */
    setLcdCmdMode();
    HAL_LCD_csOutputLow();
    asm(" NOP"); asm(" NOP"); asm(" NOP");asm(" NOP");asm(" NOP");      // Add small NOPs for GPIO logic transition

    /* SPITXBUF is 16 bits, and it's left justified */
    uint16_t lcdCommand = ((data << 8U) | 0x00U);

    /* Wait until the existing valid data inside
     * the SPITXBUF has transferred into the SPIDAT Buffer */
    while(SpibRegs.SPISTS.bit.BUFFULL_FLAG == 1){};

    /* Master transmits SPI data */
    SpibRegs.SPITXBUF = lcdCommand;

    /* Transmission or reception is complete */
    while(SpibRegs.SPISTS.bit.INT_FLAG != 1){};

    /* Read the SPI RX BUFFER to clear INT FLAG */
    uint16_t dummyData = SpibRegs.SPIRXBUF;

    // Set back to data mode
    setLcdDataMode();
    HAL_LCD_csOutputHigh();
    asm(" NOP"); asm(" NOP"); asm(" NOP");asm(" NOP");asm(" NOP");      // Add small NOPs for GPIO logic transition
}


//*****************************************************************************
//
// Writes a data to the CFAF128128B-0145T.  This function implements the basic SPI
// interface to the LCD display.
//
//*****************************************************************************
void HAL_LCD_writeData(const uint16_t data)
{
    HAL_LCD_csOutputLow();
    asm(" NOP"); asm(" NOP"); asm(" NOP");asm(" NOP");asm(" NOP");      // Add small NOPs for GPIO logic transition

    /* SPITXBUF is 16 bits, and it's left justified */
    uint16_t lcdData = ((data << 8U) | 0x00U);

    /* Wait until the existing valid data inside
     * the SPITXBUF has transferred into the SPIDAT Buffer */
    while(SpibRegs.SPISTS.bit.BUFFULL_FLAG == 1);

    /* Master transmits SPI data */
    SpibRegs.SPITXBUF = lcdData;

    /* Transmission or reception is complete */
    while(SpibRegs.SPISTS.bit.INT_FLAG != 1){};

    /* Read the SPI RX BUFFER to clear INT FLAG */
    uint16_t dummyData = SpibRegs.SPIRXBUF;

    HAL_LCD_csOutputHigh();
    asm(" NOP"); asm(" NOP"); asm(" NOP");asm(" NOP");asm(" NOP");     // Add small NOPs for GPIO logic transition
}

void HAL_LCD_rstOutputLow(void)
{
    GpioDataRegs.GPECLEAR.bit.GPIO130    = 1;  // LCD Rst pin goes low/zero
}

void HAL_LCD_rstOutputHigh(void)
{
    GpioDataRegs.GPESET.bit.GPIO130      = 1;  // LCD Rst pin goes high/one
}

void HAL_LCD_csOutputLow(void)
{
    GpioDataRegs.GPACLEAR.bit.GPIO26   = 1;    // CS chip goes low
}

void HAL_LCD_csOutputHigh(void)
{
    GpioDataRegs.GPASET.bit.GPIO26   = 1;    // CS chip goes high/one
}

void setLcdCmdMode(void)
{
    GpioDataRegs.GPCCLEAR.bit.GPIO66 = 1;    // Set DC pin to a low/zero
}

void setLcdDataMode(void)
{
    GpioDataRegs.GPCSET.bit.GPIO66 = 1;        //latch to a 1
}
