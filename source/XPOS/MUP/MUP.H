/*++

Copyright (c) 2000  Microsoft Corporation

Module Name:

    mup.h

Abstract:

    This module is the main include file for the MUP.
    It includes other header files.

Author:

    Manny Weiser (mannyw)    17-Dec-2000

Revision History:

--*/

#ifndef _MUP_
#define _MUP_

//
// "System" include files
//

#ifdef MUPDBG
#include <xpos.h>
#include <string.h>
#include <fsrtl.h>
#else
#include <ntifs.h>
#endif

#include <ntddmup.h>

//
// Local, independent include files
//

#include "debug.h"

//
// Local, dependent include files (order is important)
//

#include "lock.h"
#include "mupdata.h"
#include "mupstruc.h"
#include "mupfunc.h"

#endif // def _MUP_
