/*++

Copyright (c) 2000  Microsoft Corporation
Copyright (c) 2000  Silicon Graphics, Inc.

Module Name:

    s3maptb.c

Abstract:

    This module implements the mapping of fixed TB entries for the
    SGI Indigo system.

Author:

    David N. Cutler (davec) 28-Apr-2000

Environment:

    Kernel mode

Revision History:

--*/

#include "halp.h"


BOOLEAN
HalpMapFixedTbEntries (
    VOID
    )

/*++

Routine Description:

    It does not do any DMA mapping because the DMA device(s) are
    located in 'unmapped' space.

Arguments:

    None.

Return Value:

    Always TRUE.

--*/

{
    return TRUE;
}
