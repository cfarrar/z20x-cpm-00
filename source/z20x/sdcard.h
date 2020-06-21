#ifndef _SDCARD_H_
#define _SDCARD_H_

#include <defines.h>

extern void init_hw();

/* SD Card access */
extern int init_sdcard();
extern int read_sdcard(BYTE *buf, UINT32 n, UINT blks);
extern int write_sdcard(BYTE *buf, UINT32 n);

/* Read and write registers */
extern int _readreg(int regaddr);
extern void _writereg(int regaddr, int d);

/* SPI data read/write */
extern void mode_spi( d );
extern char _xchg_spi(char d);

UINT8 *get_time();

#endif /* _SDCARD_H_ */



