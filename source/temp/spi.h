//----------------------------------------------------------------------------
// Copyright (c) 2018, Christopher D. Farrar
//----------------------------------------------------------------------------
// I hereby grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//----------------------------------------------------------------------------
// sd card interface. based on MakerLisp low level code */
//----------------------------------------------------------------------------
#ifndef SPI_H
#define SPI_H
//----------------------------------------------------------------------------
#include <defines.h>
//----------------------------------------------------------------------------
void init_spi( void );
void mode_spi( void );
void baud_spi( void );
char read_spi( void );
void write_spi( char value );
//----------------------------------------------------------------------------
#endif /* SPI_H */
//----------------------------------------------------------------------------


