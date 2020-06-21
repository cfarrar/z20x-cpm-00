//------------------------------------------------------------------------
// Copyright (c) 2019, Christopher D. Farrar
//------------------------------------------------------------------------
// I here grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//------------------------------------------------------------------------
#include <ez80.h>
#include <stdio.h>
//------------------------------------------------------------------------
#include "cpmload.h"
#include "sdcard.h"
//------------------------------------------------------------------------
int main()
{
    init_hw();

    /* Start CP/M */
    for ( ; ; )
    {
        startCpm();
    }

    return 0;
}
//------------------------------------------------------------------------
















