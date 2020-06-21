//----------------------------------------------------------------------------
// This file is based MakerLisp Machine low-level hardware support
// from the file mlmhw.c.
//----------------------------------------------------------------------------
#define INCLUDED_BY_TIME_C
#include "time.h"
#undef INCLUDED_BY_TIME_C
//----------------------------------------------------------------------------
#include <z20x.h>
//----------------------------------------------------------------------------
#include <ez80.h>
//----------------------------------------------------------------------------
UINT8 *get_time()
{
    /* Snapshot the time */
    RTC_CTRL = 0x21;
    datetime[0] = 0;
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
//----------------------------------------------------------------------------


