//----------------------------------------------------------------------------
// This file is based MakerLisp Machine low-level hardware support
// from the file mlmhw.c.
//----------------------------------------------------------------------------
#include "sdcard.h"
//----------------------------------------------------------------------------
#include <spi.h>
#include <z20x.h>
//----------------------------------------------------------------------------
#include <defines.h>
#include <ez80.h>
#include <stddef.h>
#include <string.h>
//----------------------------------------------------------------------------
// Delays, SPI and SD card retry constants
//----------------------------------------------------------------------------
#define WAIT4RESET ( 4000 )
#define WAIT4CARD  ( 4000 )
#define SDRETRY    (  800 )
//----------------------------------------------------------------------------
// Select and de-select card with GPIO pin
//----------------------------------------------------------------------------
#define select()   ( CLEAR_BIT( PB_DR, CS_PIN ) )
#define deselect() ( SET_BIT( PB_DR, CS_PIN ), read_spi() )
//----------------------------------------------------------------------------
// SD card commands
//----------------------------------------------------------------------------
typedef struct SdCmdStruct
{
    char   num;
    UINT32 arg;
    char   crc;
}
SdCmd;
//----------------------------------------------------------------------------
static SdCmd cmdGoIdleState        = { 0x40, 0, 0x95 };
static SdCmd cmdSendIfCond         = { 0x48, 0x1AA, 0x87 };
static SdCmd cmdStopTransmission   = { 0x4c, 0, 0x87 };
static SdCmd cmdReadSingleBlock    = { 0x51, 0, 0 };
static SdCmd cmdReadMultipleBlock  = { 0x52, 0, 0 };
static SdCmd cmdWriteBlock         = { 0x58, 0, 0 };
static SdCmd cmdSendOpCond         = { 0x69, 0x40000000, 0x77 };
static SdCmd cmdApplicationCommand = { 0x77, 0, 0x65 };
static SdCmd cmdReadOCR            = { 0x7a, 0, 0 };
static SdCmd cmdCrcOnOff           = { 0x7b, 0, 0x91 };
//----------------------------------------------------------------------------
static int cmd_sdcard(
    SdCmd *commandP
  , BYTE *bufferP
  , int dataToken
  , int dataLength
  , int dataDirection )
{
    char readWriteBuffer;
    char result;
    int i;
    int length;

    // Starting a command ?
    result = 0xFF;

    if ( commandP )
    {
        // Select card, wait for ready
        select();

        i = 0;
        while ( i < SDRETRY )
        {
            readWriteBuffer = read_spi();
            if ( readWriteBuffer == (char) 0xFF )
            {
                break;
            }
          ++i;
        }

        if ( i >= SDRETRY )
        {
            deselect();
            return -1;
        }

        // Write command
        write_spi( commandP->num );
        write_spi( commandP->arg >> 24 );
        write_spi( commandP->arg >> 16 );
        write_spi( commandP->arg >> 8 );
        write_spi( commandP->arg >> 0 );
        write_spi( commandP->crc );

        // Wait for response
        if ( commandP->num == cmdStopTransmission.num )
        {
            // Stop Transmission command stuff byte
            read_spi();
        }

        i = 0;
        while ( i < SDRETRY )
        {
            result = read_spi();
            if ( result >= 0 )
            {
                break;
            }
          ++i;
        }

        if ( i >= SDRETRY )
        {
            deselect();
            return -1;
        }
    }

    // Wait for or write data token
    if ( dataToken >= 0 )
    {
        if ( ! dataDirection )
        {
            i = 0;
            while ( i < SDRETRY )
            {
                result = read_spi();
                if ( result == (char) dataToken )
                {
                    break;
                }
              ++i;
            }
            if ( i >= SDRETRY )
            {
                deselect();
                return -1;
            }
        }
        else
        {
            read_spi();
            write_spi( dataToken );
        }
    }

    // Move response or data
    i = 0;
    length = ( dataLength < 0 )
        ? -dataLength
        : dataLength;

    while ( i < length )
    {
        if ( ! dataDirection )
        {
            bufferP[i] = read_spi();
        }
        else
        {
            write_spi( bufferP[i] );
        }
      ++i;
    }

    // Wait for not busy, then deselect, if this is the end
    if ( dataLength >= 0 )
    {
        i = 0;
        while ( i < SDRETRY )
        {
            readWriteBuffer = read_spi();
            if ( readWriteBuffer == (char) 0xFF )
            {
                break;
            }
          ++i;
        }
        deselect();
    }

    return result;
}
//----------------------------------------------------------------------------
int init_sdcard( void )
{
    BYTE response[4];
    int i;

    init_spi();

    // Set port B pin 2 (SS_PIN) as output, high
    // for use as /CS for SDCARD
    SET_BIT( PB_DR, CS_PIN );
    CLEAR_BIT( PB_ALT1, CS_PIN );
    CLEAR_BIT( PB_ALT2, CS_PIN );
    CLEAR_BIT( PB_DDR, CS_PIN );

    // Wait for the SD card to power up
    i = 0;
    while ( i < WAIT4CARD )
    {
      ++i;
    }

    // Set SPI in master mode, "mode 0" transfers, 200 kHz
    mode_spi();

    // Write 80 clock pulses with /CS and DI ( MOSI ) high
    deselect();

    i = 0;
    while ( i < 10 )
    {
        write_spi( 0xFF );
      ++i;
    }

    // Reset, put card in "SPI" mode
    if ( cmd_sdcard( &cmdGoIdleState, NULL, -1, 0, 0 ) != 0x01 )
    {
        return 1;
    }

    // Check voltage range, SDC version 2
    if ( cmd_sdcard( &cmdSendIfCond, response, -1, 4, 0 ) != 0x01 )
    {
        return 1;
    }

    // Turn off CRC checking
    if ( cmd_sdcard( &cmdCrcOnOff, NULL, -1, 0, 0 ) != 0x01 )
    {
        return 1;
    }

    // Initialize
    i = 0;
    while ( i < SDRETRY )
    {
        cmd_sdcard( &cmdApplicationCommand, NULL, -1, 0, 0 );
        if ( cmd_sdcard( &cmdSendOpCond, NULL, -1, 0, 0 ) == 0x00 )
        {
            break;
        }
      ++i;
    }

    if ( ! ( i < SDRETRY ) )
    {
        return 1;
    }

    // Read OCR register, check for block addressing
    if ( cmd_sdcard( &cmdReadOCR, response, -1, 4, 0 ) != 0x00 )
    {
        return 1;
    }

    if ( ! ( response[0] & 0x40 ) )
    {
        return 1;
    }

    // Set SPI data rate to 8.33 MHz
    baud_spi();

    return 0;
}
//----------------------------------------------------------------------------
int read_sdcard(
    BYTE *bufferP
  , UINT32 location
  , UINT numberOfBlocks )
{
    BYTE crc[2];
    int i;
    int result;

    if ( numberOfBlocks == 1 )
    {
        // Single block ?
        cmdReadSingleBlock.arg = location;

        result = cmd_sdcard( &cmdReadSingleBlock, bufferP, 0xFE, -512, 0 );

        if ( !( result == 0xFE ) )
        {
            return 1;
        }
        cmd_sdcard( NULL, crc, -1, sizeof( crc ), 0 );
    }
    else
    {
        // Multiple blocks
        cmdReadMultipleBlock.arg = location;
        result = cmd_sdcard( &cmdReadMultipleBlock, NULL, -1, -1, 0 );
        i = 0;

        while ( i < numberOfBlocks )
        {
            result = cmd_sdcard( NULL, bufferP + i * 512, 0xFE, -512, 0 );

            if ( ! ( result == 0xFE ) )
            {
                return 1;
            }
            cmd_sdcard( NULL, crc, -1, -sizeof( crc ), 0 );
          ++i;
        }

        result = cmd_sdcard( &cmdStopTransmission, NULL, -1, 0, 0 );

        if ( result )
        {
            return 1;
        }
    }

    return 0;
}
//----------------------------------------------------------------------------
int write_sdcard(
    BYTE *bufferP
  , UINT32 location )
{
    BYTE dr;
    BYTE crc[2];
    int i;
    int result;

    cmdWriteBlock.arg = location;
    result = cmd_sdcard( &cmdWriteBlock, bufferP, 0xFE, -512, 1 );
    cmd_sdcard( NULL, crc, -1, -sizeof( crc ), 1 );
    cmd_sdcard( NULL, &dr, -1, 1, 0 );

    if ( ! ( result == 0x00 ) )
    {
        return 1;
    }

    if ( ! ( ( dr & 0x1F ) == 0x05 ) )
    {
        return 1;
    }

    return 0;
}
//----------------------------------------------------------------------------
