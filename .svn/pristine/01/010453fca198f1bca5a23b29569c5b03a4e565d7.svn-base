//------------------------------------------------------------------------
// Copyright (c) 2018, Christopher D. Farrar
//------------------------------------------------------------------------
// I here grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//----------------------------------------------------------------------------
#include <z20x.h>
//----------------------------------------------------------------------------
#include <ez80.h>
#include <defines.h>
#include <uart.h>
//------------------------------------------------------------------------
#include <uart.h>
//------------------------------------------------------------------------
#include "chario.h"
//------------------------------------------------------------------------
//This function should not of been static
//------------------------------------------------------------------------
extern UCHAR FifoEmpty( FIFO *f );
extern UCHAR g_mode_UART0;
extern FIFO g_RxFIFO_UART0;
//------------------------------------------------------------------------
static UCHAR kbhit_UART0( VOID )
{
    if( INTERRUPT == g_mode_UART0 )                                 //! The interrupt mode to be used.
    {
        if( FifoEmpty( &g_RxFIFO_UART0 ) )                          //! FIFO is empty.
        {
            UART0_SPR = UART_ERR_RECEIVEFIFOEMPTY ;                 //! Update SPR register.

            return 0 ;

        }
        else                                                        //! A keyboard stroke detected!
        {
            return 1 ;
        }
    }
    else                                                            //! The polling mode to be used.
    {
        return ( TRUE == (UART0_LSR & UART_LSR_DATA_READY) ) ;      //! Find it reading line status register.
    }
}
//------------------------------------------------------------------------
UINT8 consoleCharInReady( void )
{
    return ( kbhit_UART0() ) ? 0xFF : 0x00;
}
//------------------------------------------------------------------------
char consoleCharIn( void )
{
    char c;
    unsigned n = 1;
    do
    {
        read_UART0( &c, &n );
    }
    while ( 0 == n );
    return c;
}
//------------------------------------------------------------------------
UINT8 consoleCharOutReady( void )
{
    return 0xFF;
}
//------------------------------------------------------------------------
void consoleCharOut( char c )
{
    UCHAR result;
    do
    {
        result = write_UART0( &c, 1 );
    }
    while ( result != UART_ERR_NONE );
}
//------------------------------------------------------------------------



