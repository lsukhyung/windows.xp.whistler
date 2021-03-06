//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 2000 - 1993.
//
//  File:	safedecl.hxx
//
//  Contents:	Declare some globally useful safe pointer classes
//
//  History:	18-Oct-93	DrewB	Created
//
//----------------------------------------------------------------------------

#ifndef __SAFEDECL_HXX__
#define __SAFEDECL_HXX__

#include <safepnt.hxx>

SAFE_INTERFACE_PTR(SafeIUnknown, IUnknown);
SAFE_INTERFACE_PTR(SafeIStorage, IStorage);
SAFE_INTERFACE_PTR(SafeIStream, IStream);
SAFE_HEAP_MEMPTR(SafeBytePtr, BYTE);

// Declare only on Nt-based platforms
#ifdef NT_INCLUDED
SAFE_NT_HANDLE(SafeNtHandle);
SAFE_NT_HANDLE_NO_UNWIND(NuSafeNtHandle);
#endif

#endif // #ifndef __SAFEDECL_HXX__
