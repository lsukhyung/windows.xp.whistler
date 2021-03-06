/***************************************************
Copyright (C) Maynard, An Archive Company. 2000

        Name:         D_O_RSET.H

        Description:

        $Log:   G:/UI/LOGFILES/D_O_RSET.H_V  $

   Rev 1.4   04 Oct 2000 19:46:54   DAVEV
UNICODE AWK PASS

   Rev 1.3   28 Jul 2000 14:55:00   CHUCKB
Fixed warnings for NT.

   Rev 1.2   22 Jan 2000 16:21:46   CHUCKB
Changed prototype for RestoreSetSave.

   Rev 1.1   20 Jan 2000 09:46:10   GLENN
Moved configuration definitions to MUICONF.H

   Rev 1.0   20 Nov 2000 19:35:18   SYSTEM
Initial revision.

*****************************************************/
#ifndef  d_o_rset_h
#define  d_o_rset_h

// For restore definitions, see muiconf.h

BOOL     RestoreSetSave( HWND ) ;
VOID     RestoreSetRetrieve( HWND ) ;
INT      RestoreSetDefaultDescription( VOID ) ;

BSD_PTR  GetTapeBSDPointer( INT ) ;
VOID     GetCurrentRestoreDriveList( HWND ) ;
VOID     GetRestoreDrive( HWND ) ;
VOID     SetRestoreDrive( HWND ) ;
VOID     RestoreSaveTargetPaths( VOID ) ;
VOID     SetDefaultDLE( HWND ) ;
INT      GetMaxBSDCount( VOID ) ;

#endif
