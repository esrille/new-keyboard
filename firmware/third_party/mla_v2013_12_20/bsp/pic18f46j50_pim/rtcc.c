
#include <xc.h>
#include "rtcc.h"

void BSP_RTCC_Initialize (BSP_RTCC_DATETIME * value)
{
    // This is a placeholder function for this part
}

void BSP_RTCC_TimeGet (BSP_RTCC_DATETIME * value)
{
    // This part does not have an RTCC module
    // This function will return a static value

    if (value->bcdFormat)
    {
        value->year = 0x13;
        value->month = 0x01;
        value->day = 0x01;
        value->weekday = 0x02;
        value->hour = 0x12;
        value->minute = 0x15;
        value->second = 0x30;
    }
    else
    {
        value->year = 13;
        value->month = 01;
        value->day = 01;
        value->weekday = 02;
        value->hour = 12;
        value->minute = 15;
        value->second = 30;
    }
}

