//----------------------------------------------------------------------------
// Copyright (c) 2018, Christopher D. Farrar
//----------------------------------------------------------------------------
// I hereby grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//----------------------------------------------------------------------------
// sd card interface. based on MakerLisp low level code */
//----------------------------------------------------------------------------
#ifndef Z20X_H
#define Z20X_H
//----------------------------------------------------------------------------
#include <defines.h>
//----------------------------------------------------------------------------
#ifndef TRUE
#define TRUE    1       //!< The TRUE definition.
#endif
#ifndef FALSE
#define FALSE   0       //!< The FALSE definition.
#endif
//----------------------------------------------------------------------------
#define MOSI_PIN    ( 7 )
#define MISO_PIN    ( 6 )
#define T5_PIN      ( 5 )
#define T4_PIN      ( 4 )
#define SCLK_PIN    ( 3 )
#define SS_PIN      ( 2 )
//----------------------------------------------------------------------------
// Clear, set bits in registers
//----------------------------------------------------------------------------
#define CS_PIN      ( SS_PIN )
//----------------------------------------------------------------------------
#define CLEAR_BIT( reg, n ) reg &= ~( 1 << n )
#define SET_BIT( reg, n ) reg |= ( 1 << n )
//----------------------------------------------------------------------------
#endif /* Z20X_H */
//----------------------------------------------------------------------------


