/******************************Module*Header*******************************\
* Module Name: debugint.c
*
*
* Created: 13-Sep-1993 08:55:20
* Author:  Eric Kutter [erick]
*
* Copyright (c) 1993 Microsoft Corporation
*
\**************************************************************************/

#include "stdarg.h"
#include "stdio.h"
#include "engine.h"

#if DBG

LONG gWarningLevel = 0;

// DoWarning1 is for ASM functions to call at Warning Level 1

VOID DoWarning1(PSZ psz)
{
    if (1 <= gWarningLevel)
    {
        DbgPrint("GDISRV Warning: ");
        DbgPrint(psz);
        DbgPrint("\n");
    }
}

VOID DoWarning(PSZ psz, LONG ulLevel)
{
    if (ulLevel <= gWarningLevel)
    {
        DbgPrint("GDISRV Warning: ");
        DbgPrint(psz);
        DbgPrint("\n");
    }
}

VOID DoRip(PSZ psz)
{
    if (gWarningLevel >= 0)
    {
        DbgPrint("GDI Assertion Failure: ");
        DbgPrint(psz);
        DbgPrint("\n");
        DbgBreakPoint();
    }
}
#endif

/******************************Public*Routine******************************\
* DbgPrint()
*
*   defines a DbgPrint that will work with WINDBG.  The base DbgPrint()
*   always goes to the kernel debugger.
*
* History:
*  15-Sep-1993 -by-  Eric Kutter [erick]
* stole from vga driver.
\**************************************************************************/

ULONG
DbgPrint(
    PCH DebugMessage,
    ...
    )
{
    va_list ap;
    char buffer[256];

    va_start(ap, DebugMessage);

    vsprintf(buffer, DebugMessage, ap);

    OutputDebugStringA(buffer);

    va_end(ap);

    return(0);
}
