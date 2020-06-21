//----------------------------------------------------------------------------
// This file is based MakerLisp Machine low-level hardware support
// from the file mlmhw.c.
//----------------------------------------------------------------------------
#include "spi.h"
//----------------------------------------------------------------------------
#include <z20x.h>
//----------------------------------------------------------------------------
#include <defines.h>
#include <ez80.h>
#include <stddef.h>
#include <string.h>
//----------------------------------------------------------------------------
#define SPIRETRY   ( 1000 )
//----------------------------------------------------------------------------
static char exchange_spi( char value )
{
    int i;
    volatile int delay;

    /* Write the data to exchange */
    SPI_TSR = value;

    /* Wait for slave data to arrive */
    i = 0;
    while ( i < SPIRETRY )
    {
        if ( SPI_SR & (1 << 7) )
        {
            break;
        }
        ++i;
    }

    /* Delay, then read data */
    delay = 0;
    while ( delay < 1 )
    {
        ++delay;
    }

    return SPI_RBR;
}
//----------------------------------------------------------------------------
void mode_spi( void )
{
    // Set SPI in master mode, "mode 0" transfers, 200 kHz
    SPI_BRG_H = 125 / 256;
    SPI_BRG_L = 125 % 256;
    SPI_CTL = 0x30;
}
//----------------------------------------------------------------------------
void baud_spi( void )
{
    // Set SPI data rate to 8.33 MHz
    SPI_CTL = 0;
    SPI_BRG_H = 3 / 256;
    SPI_BRG_L = 3 % 256;
    SPI_CTL = 0x30;
}
//----------------------------------------------------------------------------
void init_spi( void )
{
    // set unused t4 and t5 pins to output
    CLEAR_BIT( PB_DR, T4_PIN );
    CLEAR_BIT( PB_ALT1, T4_PIN );
    CLEAR_BIT( PB_ALT2, T4_PIN );
    CLEAR_BIT( PB_DDR, T4_PIN );
    CLEAR_BIT( PB_DR, T5_PIN );
    CLEAR_BIT( PB_ALT1, T5_PIN );
    CLEAR_BIT( PB_ALT2, T5_PIN );
    CLEAR_BIT( PB_DDR, T5_PIN );

    // Set port B pins 7 ( MOSI ),
    // 6 ( MISO ), 3 ( SCK ), to SPI
    CLEAR_BIT( PB_ALT1, MOSI_PIN );
    CLEAR_BIT( PB_ALT1, MISO_PIN );
    CLEAR_BIT( PB_ALT1, SCLK_PIN );
    SET_BIT( PB_ALT2, MOSI_PIN );
    SET_BIT( PB_ALT2, MISO_PIN );
    SET_BIT( PB_ALT2, SCLK_PIN );
}
//----------------------------------------------------------------------------
char read_spi( void )
{
    return exchange_spi( 0xFF );
}
//----------------------------------------------------------------------------
void write_spi( char value )
{
    exchange_spi( value );
}
//----------------------------------------------------------------------------

