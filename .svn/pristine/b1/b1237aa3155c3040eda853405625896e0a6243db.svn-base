//----------------------------------------------------------------------------
// Copyright (c) 2018, Christopher D. Farrar
//----------------------------------------------------------------------------
#include "kbhit.h"
//----------------------------------------------------------------------------
#include "z20x.h"
//----------------------------------------------------------------------------
extern UCHAR g_mode_UART0;
extern FIFO g_RxFIFO_UART0;
//----------------------------------------------------------------------------
UCHAR kbhit_UART0( VOID )
{
	if( INTERRUPT == g_mode_UART0 )									//! The interrupt mode to be used.
	{
		if( FifoEmpty( &g_RxFIFO_UART0 ) )							//! FIFO is empty.
		{
			UART0_SPR = UART_ERR_RECEIVEFIFOEMPTY ;					//! Update SPR register.
			return 0 ;
		}
		else														//! A keyboard stroke detected!
		{
			return 1 ;
		}
	}
	else															//! The polling mode to be used.
	{
		return ( TRUE == (UART0_LSR & UART_LSR_DATA_READY) ) ;		//! Find it reading line status register.
	}
}
//----------------------------------------------------------------------------
extern UCHAR g_mode_UART1;
extern FIFO g_RxFIFO_UART1;
//----------------------------------------------------------------------------
UCHAR kbhit_UART1( VOID )
{
	if( INTERRUPT == g_mode_UART1 )									//! The interrupt mode to be used.
	{
		if( FifoEmpty( &g_RxFIFO_UART1 ) )							//! FIFO is empty.
		{
			UART1_SPR = UART_ERR_RECEIVEFIFOEMPTY ;					//! Update SPR register.
			return 0 ;
		}
		else														//! A keyboard stroke detected!
		{
			return 1 ;
		}
	}
	else															//! The polling mode to be used.
	{
		return ( TRUE == (UART1_LSR & UART_LSR_DATA_READY) ) ;		//! Find it reading line status register.
	}
}
//----------------------------------------------------------------------------



/*!
 * \brief Detect keyboard from default UART.
 *
 * This API detects for any keystrokes from the default UART. If a keystroke is detected
 * this function returns a 1 else returns 0. The API returns immediately
 * without blocking both when the uart is configured to work in poll mode and in the
 * interrupt mode. Also note that the API does not read the data but only returns
 * the status. However, this function will return a 0xFF if FIFO has an error in it.
 * When it returns 0xFF error code, the application can use geterr_UART() to know the
 * exact error that occurred.
 */
UCHAR kbhit( VOID )
{

#ifdef DEFAULT_UART0
	return kbhit_UART0() ;
#endif
#ifdef DEFAULT_UART1
	return kbhit_UART1() ;
#endif

}//! end of kbhit()

/*! End of File */


