/* this file based on mlmhw.c of makerlisp project */
#include <defines.h>
#include <ez80.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "sdcard.h"

/* Delays, SPI and SD card retry constants */
#define WAIT4RESET 4000
#define WAIT4CARD 4000
#define SDRETRY 2000
#define SPIRETRY 1000

#define SD_CS 4
#define TP_CS 1
#define SPI_MOSI 7
#define SPI_MISO 6
#define SPI_CLK 3




/* Clear, set bits in registers */
#define BIT( n ) ( 1 << n )
#define CLEAR_BIT(reg, n) reg &= ~(1 << n)
#define SET_BIT(reg, n) reg |= (1 << n)

/* Reads and writes are SPI full duplex exchanges */
#define read_spi() _xchg_spi(0xFF)
#define write_spi(d) _xchg_spi(d)

/* Select and de-select card with GPIO pin */
#define select()  PB_DR &= ~BIT( SD_CS )
#define unselect() PB_DR |= BIT( SD_CS )
#define deselect() unselect(); read_spi()

/* SD card commands */
typedef struct sdcmd {
    char num;
    UINT32 arg;
    char crc;
} SDCMD;
static SDCMD cmd0 = { 0x40, 0, 0x95 };
static SDCMD cmd8 = { 0x48, 0x1AA, 0x87 };
static SDCMD cmd12 = { 0x4c, 0, 0x87 };
static SDCMD cmd17 = { 0x51, 0, 0 };
static SDCMD cmd18 = { 0x52, 0, 0 };
static SDCMD cmd24 = { 0x58, 0, 0 };
static SDCMD cmd41 = { 0x69, 0x40000000, 0x77 };
static SDCMD cmd55 = { 0x77, 0, 0x65 };
static SDCMD cmd58 = { 0x7a, 0, 0 };
static SDCMD cmd59 = { 0x7b, 0, 0x91 };

/* RTC date and time array */
static unsigned char datetime[7];

/* Expansion board detected */
static char ebhere;

/* Reset system devices */
void init_hw()
{
    long i;

    /* turn on backlight port C bit 2 */
    PC_DR |= BIT( 2 );
    PC_ALT1 &= ~BIT( 2 );
    PC_ALT2 &= ~BIT( 2 );
    PC_DDR &= ~BIT( 2 );

    /* SS must remain high for spi to work properly */
    PB_DR |= BIT( 2 );
    PB_ALT1 &= ~BIT( 2 );
    PB_ALT2 &= ~BIT( 2 );
    PB_DDR &= ~BIT( 2 );

    // Enable the chip select outputs and de-select
    PB_DR |= ( BIT( SD_CS ) | BIT( TP_CS ) );
    PB_ALT1 &= ~( BIT( SD_CS ) | BIT( TP_CS ) );
    PB_ALT2 &= ~( BIT( SD_CS ) | BIT( TP_CS ) );
    PB_DDR &= ~( BIT( SD_CS ) | BIT( TP_CS ) );

    /* Set port B pins 7 (MOSI), 6 (MISO), 3 (SCK), 2 (/SS) to SPI */
    PB_ALT1 &= ~( BIT( SPI_MOSI ) | BIT( SPI_MISO ) | BIT( SPI_CLK ) );
    PB_ALT2 |= ( BIT( SPI_MOSI ) | BIT( SPI_MISO ) | BIT( SPI_CLK ) );

    i = 0;
    while (i < WAIT4RESET) {
        ++i;
    }

}

void mode_spi( d )
{
    SPI_CTL = 0;
    SPI_BRG_H = d / 256;
    SPI_BRG_L = d % 256;
    SPI_CTL = 0x30;
}

/* SPI exchange transfer */
char _xchg_spi(char d)
{
    int i;
    volatile int delay;

    /* Write the data to exchange */
    SPI_TSR = d;

    /* Wait for slave data to arrive */
    i = 0;
    while (i < SPIRETRY) {
        if (SPI_SR & (1 << 7)) {
            break;
        }
        ++i;
    }

    /* Delay, then read data */
    delay = 0;
    while (delay < 1) {
        ++delay;
    }

    return SPI_RBR;
}

/* Send a command to the SD card and get response, move data */
static int cmd_sdcard(SDCMD *cmd, BYTE *buf, int tok, int dlen, int ddir)
{
    char d, r;
    int i, l;

    //printf( "\r\n>00<%04X>", cmd->num );
    /* Starting a command ? */
    r = 0xFF;
    if (cmd) {

        /* Select card, wait for ready */
        select();
        i = 0;
        while (i < SDRETRY) {
            d = read_spi();
            if (d == (char)0xFF) {
                break;
            }
            ++i;
        }
        if (!(i < SDRETRY)) {
            deselect();
            return -1;
        }

        /* Write command */
        write_spi(cmd->num);
        write_spi(cmd->arg >> 24);
        write_spi(cmd->arg >> 16);
        write_spi(cmd->arg >> 8);
        write_spi(cmd->arg >> 0);
        write_spi(cmd->crc);

        /* Wait for response */
        if (cmd->num == cmd12.num) {

            /* CMD12 stuff byte */
            read_spi();
        }
        i = 0;
        while (i < SDRETRY) {
            r = read_spi();
            if (!(r < 0)) {
                break;
            }
            ++i;
        }
        if (!(i < SDRETRY)) {
            deselect();
            //printf( "\r\n>07<-1>" );
            return -1;
        }
    }

    /* Wait for or write data token */
    if (!(tok < 0)) {
        if (!ddir) {
            i = 0;
            while (i < SDRETRY) {
                r = read_spi();
                if (r == (char)tok) {
                    break;
                }
                ++i;
            }
            if (!(i < SDRETRY)) {
                deselect();
                //printf( "\r\n>10<-1>" );
                return -1;
            }
        } else {
            read_spi();
            write_spi(tok);
        }
    }

    /* Move response or data */
    i = 0;
    l = (dlen < 0) ? -dlen : dlen;
    while (i < l) {
        if (!ddir) {
            buf[i] = read_spi();
        } else {
            write_spi(buf[i]);
        }
        ++i;
    }

    /* Wait for not busy, then deselect, if this is the end */
    if (!(dlen < 0)) {
        i = 0;
        while (i < SDRETRY) {
            d = read_spi();
            if (d == (char)0xFF) {
                break;
            }
            ++i;
        }
        deselect();
    }
    //printf( "\r\n>15<%04X>", r );

    return (unsigned char)r;
}

/* Set up SD card for reads and writes - expect SD v2, HC */
int init_sdcard()
{
    BYTE response[4];
    int i;

    /* Set SPI in master mode, "mode 0" transfers, 200 kHz */
    mode_spi( 50 );

    /* Write 80 clock pulses with /CS and DI (MOSI) high */
    deselect();
    i = 0;
    while (i < 10) {
        write_spi(0xFF);
        ++i;
    }

    /* Reset, put card in "SPI" mode */
    if (cmd_sdcard(&cmd0, NULL, -1, 0, 0) != 0x01) {
        return 1;
    }
    /* Check voltage range, SDC version 2 */
    if (cmd_sdcard(&cmd8, response, -1, 4, 0) != 0x01) {
        return 1;
    }

    /* Turn off CRC checking */
    if (cmd_sdcard(&cmd59, NULL, -1, 0, 0) != 0x01) {
        return 1;
    }

    /* Initialize */
    i = 0;
    while (i < SDRETRY) {
        cmd_sdcard(&cmd55, NULL, -1, 0, 0);
        if (cmd_sdcard(&cmd41, NULL, -1, 0, 0) == 0x00) {
            break;
        }
        ++i;
    }
    if (!(i < SDRETRY)) {
        return 1;
    }

    /* Read OCR register, check for block addressing */
    if (cmd_sdcard(&cmd58, response, -1, 4, 0) != 0x00) {
        return 1;
    }
    if (!(response[0] & 0x40)) {
        return 1;
    }

    /* Set SPI data rate to 8.33 MHz */
    mode_spi( 3 );

    return 0;
}

/* Read blocks */
int read_sdcard(BYTE *buf, UINT32 n, UINT blks)
{
    BYTE crc[2];
    int i, r;

    if (blks == 1) {

        /* Single block ? */
        cmd17.arg = n;
        r = cmd_sdcard(&cmd17, buf, 0xFE, -512, 0);
        if (!(r == 0xFE)) {
            return 1;
        }
        cmd_sdcard(NULL, crc, -1, sizeof(crc), 0);
    } else {

        /* Multiple blocks */
        cmd18.arg = n;
        r = cmd_sdcard(&cmd18, NULL, -1, -1, 0);
        i = 0;
        while (i < blks) {
            r = cmd_sdcard(NULL, buf + i*512, 0xFE, -512, 0);;
            if (!(r == 0xFE)) {
                return 1;
            }
            cmd_sdcard(NULL, crc, -1, -sizeof(crc), 0);
            ++i;
        }
        r = cmd_sdcard(&cmd12, NULL, -1, 0, 0);
        if (r) {
            return 1;
        }
    }

    return 0;
}

/* Write a block */
int write_sdcard(BYTE *buf, UINT32 n)
{
    BYTE dr, crc[2];
    int i, r;

    cmd24.arg = n;
    r = cmd_sdcard(&cmd24, buf, 0xFE, -512, 1);
    cmd_sdcard(NULL, crc, -1, -sizeof(crc), 1);
    cmd_sdcard(NULL, &dr, -1, 1, 0);

    if (!(r == 0x00)) {
        return 1;
    }
    if (!((dr & 0x1F) == 0x05)) {
        return 1;
    }

    return 0;
}

/* Read and write registers */
int _readreg(int regaddr)
{
    return *(volatile unsigned char __INTIO *)regaddr;
}
void _writereg(int regaddr, int d)
{
    *(volatile unsigned char __INTIO *)regaddr = d;
}

/* Get time from RTC */
UINT8 *get_time()
{
    /* Snapshot the time */
    RTC_CTRL = 0x21;
    datetime[6] = RTC_CEN;
    datetime[5] = RTC_YR;
    datetime[4] = RTC_MON;
    datetime[3] = RTC_DOM;
    datetime[2] = RTC_HRS;
    datetime[1] = RTC_MIN;
    datetime[0] = RTC_SEC;
    RTC_CTRL = 0x20;

    return &datetime;
}



