/**********************************************************************/
/**                       Microsoft Windows                          **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*

    Init.c

    Contains VxD initialization code


    FILE HISTORY:
        Johnl   24-Mar-1993     Created

*/

#include <vxdprocs.h>
#include <tdiinfo.h>
#include <ipinfo.h>
#include <llinfo.h>
#include <debug.h>
#include "local.h"

//
//  Renewal timer
//
CTETimer RenewalTimer ;

//
//  Event for rescheduling ProcessDhcpRequestForever
//

CTEEvent ProcessEvent ;


VOID
DelayedProcessDhcpRequest(
    CTEEvent * pCTEEvent,
    PVOID      pContext
    );


#ifdef DEBUG
//
//  Notifies clients every x seconds that there IP address has changed
//
CTETimer DbgNotificationTimer ;
ULONG    DbgNotifyTime = 0 ;        // Set to non-zero to enable (in ms)

VOID
DbgNotificationRoutine(
    CTEEvent * pCTEEvent,
    PVOID      pContext
    ) ;
#endif //DEBUG

//
//  Flag indicating refilling the heap is Ok
//
BOOL fInInit = TRUE ;

extern LIST_ENTRY NotifyListHead ;
extern DWORD DhcpGlobalDisplayPopups;

BOOL ScanIpEntities( BOOL ContextLookup, ushort IpContext );

TDI_STATUS FindHardwareAddr( TDIEntityID        Elist[],
                             UINT               cEntities,
                             IPAddrEntry      * pIAE,
                             HARDWARE_ADDRESS * pHardwareAddress,
                             BOOL             * pfFound,
                             ULONG            * pIfIndex ) ;

//
//  Some debug support stuff
//

#ifdef DEBUG
    DWORD DebugFlags = DEBUG_ERRORS ;
    char  DBOut[4096] ;
    int   iCurPos = 0 ;

void NbtDebugOut( char * str )
{
    if ( DebugFlags & (DBGFLAG_AUX_OUTPUT | DEBUG_ERRORS ))
        CTEPrint( str ) ;

    if ( sizeof(DBOut) - iCurPos < 256 )
        iCurPos = 0 ;
    else
        iCurPos += strlen( str ) + 1 ;
}

#endif

#pragma BEGIN_INIT

/*******************************************************************

    NAME:       DhcpInit

    SYNOPSIS:   Performs all driver initialization

    RETURNS:    TRUE if initialization successful, FALSE otherwise

    NOTES:

    HISTORY:
        Johnl   24-Mar-1993     Created

********************************************************************/

int DhcpInit( void )
{
    NTSTATUS            status ;
    int                 i ;
    DWORD               TimeToSleep ;
    PDHCP_CONTEXT       pDhcpContext ;
    PLIST_ENTRY         pEntry ;

    if ( CTEInitialize() )
    {
        DbgPrint("Init: CTEInitialize succeeded\n\r") ;
    }
    else
        return FALSE ;

    CTEInitTimer( &RenewalTimer ) ;
    CTERefillMem() ;

    if ( !InitFileSupport() )
    {
        CDbgPrint( DEBUG_ERRORS, ("Init: InitFileSupport failed\r\n")) ;
        return FALSE ;
    }

    if ( !InitMsgSupport() )
    {
        CDbgPrint( DEBUG_ERRORS, ("Init: InitMsgSupport failed\r\n")) ;
        return FALSE ;
    }

    DhcpGlobalDisplayPopups = TRUE;

    InitializeListHead( &DhcpGlobalRenewList ) ;
    InitializeListHead( &DhcpGlobalNICList ) ;
    InitializeListHead( &LocalDhcpBinList ) ;
    InitializeListHead( &NotifyListHead ) ;

    //
    // Get the Addresses/Leases from the configuration file from the last boot
    //

    if ( BuildDhcpWorkList() )
    {
        CDbgPrint( DEBUG_ERRORS, ("Init: Failed to build worker list\r\n")) ;
        return FALSE ;
    }

    //
    //  Analyze the list, attempt to get new IP addresses as appropriate.
    //
    DhcpInitialize( NULL ) ;

#if !defined(CHICAGO)

    //
    //  Find all Lanas that the IP driver wants DHCPed
    //

    if ( !ScanIpEntities( FALSE, 0 ) )
    {
        CDbgPrint( DEBUG_ERRORS, ("Init: Failed to get addresses from IP driver\r\n")) ;
        return FALSE ;
    }

#endif  // !CHICAGO

    //
    //  Kickoff the renewal timer
    //
    ProcessDhcpRequestForever( NULL, NULL ) ;

#ifdef DEBUG
    if ( DbgNotifyTime )
        DbgNotificationRoutine( NULL, NULL ) ;
#endif

    CTERefillMem() ;
    fInInit = FALSE ;
    return TRUE ;
}

#pragma END_INIT

#ifndef CHICAGO
#pragma BEGIN_INIT
#endif  // !CHICAGO

/*******************************************************************

    NAME:       ScanIpEntities

    SYNOPSIS:   Scans the IP entities and the IP address tables managed
                by those entities looking for either a) zero IP addresses
                or b) an address with a specified IP context.

    ENTRY:      ContextLookup - If TRUE, then look for an address entry
                    with a specific IP context value.  Otherwise, look
                    for entries with zero IP addresses.

                IpContext - The IP context to search for.  Ignored if
                    ContextLookup is not TRUE.

    EXIT:       Newly active IP addresses are added to the DhcpGlobalNICList
                and previously configured addresses are moved from
                LocalDhcpBinList to DhcpGlobalNICList.

    RETURNS:    TRUE if successful, FALSE otherwise.

********************************************************************/
BOOL ScanIpEntities( BOOL ContextLookup, ushort IpContext )
{
    NTSTATUS            status ;
    TDI_STATUS          tdistatus ;
    DWORD               err ;
    int                 i, j ;
    PDHCP_CONTEXT       pDhcpContext ;
    PLIST_ENTRY         pEntry ;
    uchar               Context[CONTEXT_SIZE] ;
    TDIObjectID         ID ;
    TDIEntityID         EList[MAX_TDI_ENTITIES] ;
    ULONG               Size ;
    UINT                NumReturned ;
    NDIS_BUFFER         ndisbuff ;


    //
    //  The first thing to do is get the list of available entities, and make
    //  sure that there are some interface entities present.
    //
    ID.toi_entity.tei_entity   = GENERIC_ENTITY;
    ID.toi_entity.tei_instance = 0;
    ID.toi_class               = INFO_CLASS_GENERIC;
    ID.toi_type                = INFO_TYPE_PROVIDER;
    ID.toi_id                  = ENTITY_LIST_ID;

    Size = sizeof(EList);
    InitNDISBuff( &ndisbuff, &EList, Size, NULL ) ;
    memset(Context, 0, CONTEXT_SIZE);

    tdistatus = TdiVxdQueryInformationEx( 0,
                                          &ID,
                                          &ndisbuff,
                                          &Size,
                                          Context);

    if (tdistatus != TDI_SUCCESS)
    {
        CDbgPrint( DEBUG_ERRORS, ("ScanIpEntities: Querying entity list failed\r\n")) ;
        return FALSE ;
    }

    NumReturned  = (uint)Size/sizeof(TDIEntityID);

    for (i = 0; i < NumReturned; i++)
    {
        if ( EList[i].tei_entity == CL_NL_ENTITY )
        {
            IPSNMPInfo    IPStats ;
            IPAddrEntry * pIAE ;
            ULONG         NLType ;

            //
            //  Does this entity support IP?
            //

            ID.toi_entity.tei_entity   = EList[i].tei_entity ;
            ID.toi_entity.tei_instance = EList[i].tei_instance;
            ID.toi_class               = INFO_CLASS_GENERIC ;
            ID.toi_type                = INFO_TYPE_PROVIDER;
            ID.toi_id                  = ENTITY_TYPE_ID ;

            Size = sizeof( NLType );
            InitNDISBuff( &ndisbuff, &NLType, Size, NULL ) ;
            memset(Context, 0, CONTEXT_SIZE);
            tdistatus = TdiVxdQueryInformationEx( 0,
                                                  &ID,
                                                  &ndisbuff,
                                                  &Size,
                                                  Context);
            if ( tdistatus != TDI_SUCCESS )
            {
                CDbgPrint( DEBUG_ERRORS, ("ScanIpEntities: Getting NL type failed\r\n")) ;
                continue ;
            }

            if ( NLType != CL_NL_IP )
                continue ;

            //
            //  We've got an IP driver so get it's address table
            //

            ID.toi_class  = INFO_CLASS_PROTOCOL ;
            ID.toi_id     = IP_MIB_STATS_ID;
            Size = sizeof(IPStats);
            InitNDISBuff( &ndisbuff, &IPStats, Size, NULL ) ;
            memset(Context, 0, CONTEXT_SIZE);
            tdistatus = TdiVxdQueryInformationEx( 0,
                                                  &ID,
                                                  &ndisbuff,
                                                  &Size,
                                                  Context);
            if ( tdistatus != TDI_SUCCESS )
            {
                CDbgPrint( DEBUG_ERRORS, ("ScanIpEntities: Getting IPStats failed\r\n")) ;
                continue ;
            }

            if ( IPStats.ipsi_numaddr < 1 )
            {
                CDbgPrint( DEBUG_ERRORS, ("ScanIpEntities: No IP Addresses installed\r\n")) ;
                continue ;
            }

            Size = sizeof(IPAddrEntry) * IPStats.ipsi_numaddr ;
            if ( !(pIAE = (IPAddrEntry*) CTEAllocInitMem( (USHORT) Size )) )
            {
                CDbgPrint( DEBUG_ERRORS, ("ScanIpEntities: Couldn't allocate IP table buffer\r\n")) ;
                return FALSE ;
            }

            ID.toi_id = IP_MIB_ADDRTABLE_ENTRY_ID ;
            InitNDISBuff( &ndisbuff, pIAE, Size, NULL ) ;
            memset( Context, 0, CONTEXT_SIZE ) ;
            tdistatus = TdiVxdQueryInformationEx( 0,
                                                  &ID,
                                                  &ndisbuff,
                                                  &Size,
                                                  Context);
            if ( tdistatus != TDI_SUCCESS )
            {
                CDbgPrint( DEBUG_ERRORS, ("ScanIpEntities: Getting IP address table failed\r\n")) ;
                CTEFreeMem( pIAE ) ;
                continue ;
            }

            ASSERT( Size/sizeof(IPAddrEntry) == IPStats.ipsi_numaddr ) ;

            //
            //  We have the IP address table for this IP driver.  Look for
            //  DHCPable IP addresses and find the corresponding interface
            //  and get its hardware address
            //

            for ( j = 0 ; j < IPStats.ipsi_numaddr ; j++ )
            {
                if(
#if defined(CHICAGO)
                    ( ContextLookup && ( pIAE[j].iae_context == IpContext ) ) ||
#endif  // defined(CHICAGO)
                    ( pIAE[j].iae_addr == 0 ) )
                {
                    HARDWARE_ADDRESS HardwareAddress ;
                    ULONG            IfIndex ;
                    BOOL             fFound ;

                    tdistatus = FindHardwareAddr( EList,
                                                  NumReturned,
                                                  &pIAE[j],
                                                  &HardwareAddress,
                                                  &fFound,
                                                  &IfIndex ) ;

                    if ( tdistatus != TDI_SUCCESS )
                    {
                        CDbgPrint( DEBUG_ERRORS, ("ScanIpEntities: Hardware search failed\r\n")) ;
                        CTEFreeMem( pIAE ) ;
                        return FALSE ;
                    }
                    else if ( !fFound )
                    {
                        CDbgPrint( DEBUG_ERRORS, ("ScanIpEntities: Warning - Couldn't find hardware address\r\n")) ;
                        continue ;
                    }

                    //
                    //  Let DhcpInitializeAdapter do the dirty work.
                    //

                    status = DhcpInitializeAdapter(
                                pIAE[j].iae_context,
                                IfIndex,
                                EList[i].tei_instance,
                                &HardwareAddress );

                    if( ContextLookup )
                    {
                        return status == 0;
                    }
                } // matching context or addr == 0
            } // addr traversal

            CTEFreeMem( pIAE ) ;

        }  // if IP
    } // entity traversal

    return TRUE ;
}

/*******************************************************************

    NAME:       FindHardwareAddr

    SYNOPSIS:   Given an entity list and an IP Address Entry, all interfaces
                are searched until a match is found.

    ENTRY:      EList - Filled in entity list
                cEntities - Number of entities in the list
                pIAE      - IP Address entry to use as the key
                pHardwareAddress - Physical address will be copied here
                pfFound   - TRUE if address found, FALSE otherwise
                pIfIndex  - Interface index of hardware address

    RETURNS:    TDI error

    NOTES:

********************************************************************/

TDI_STATUS FindHardwareAddr( TDIEntityID        EList[],
                             UINT               cEntities,
                             IPAddrEntry      * pIAE,
                             HARDWARE_ADDRESS * pHardwareAddress,
                             BOOL             * pfFound,
                             ULONG            * pIfIndex )
{
    int                 i ;
    uchar               Context[CONTEXT_SIZE] ;
    TDIObjectID         ID ;
    TDI_STATUS          tdistatus ;
    ULONG               Size ;
    NDIS_BUFFER         ndisbuff ;

    *pfFound = FALSE ;

    ID.toi_entity.tei_entity   = IF_MIB ;
    ID.toi_type                = INFO_TYPE_PROVIDER;

    for ( i = 0 ; i < cEntities ; i++ )
    {
        if (EList[i].tei_entity == IF_ENTITY)
        {
            IFEntry IFE ;
            ULONG   IFType ;

            //
            //  Check and make sure the interface supports MIB-2
            //

            ID.toi_entity.tei_entity   = EList[i].tei_entity ;
            ID.toi_entity.tei_instance = EList[i].tei_instance;
            ID.toi_class               = INFO_CLASS_GENERIC ;
            ID.toi_id                  = ENTITY_TYPE_ID ;
            Size = sizeof( IFType );
            InitNDISBuff( &ndisbuff, &IFType, Size, NULL ) ;
            memset(Context, 0, CONTEXT_SIZE);
            tdistatus = TdiVxdQueryInformationEx( 0,
                                                  &ID,
                                                  &ndisbuff,
                                                  &Size,
                                                  Context);
            if ( tdistatus != TDI_SUCCESS )
            {
                CDbgPrint( DEBUG_ERRORS, ("FindHardwareAddr: Getting IF type failed\r\n")) ;
                return tdistatus ;
            }

            if ( IFType != IF_MIB )
                continue ;

            //
            //  We've found an interface, get its index and see if it
            //  matches the IP Address entry
            //

            ID.toi_class = INFO_CLASS_PROTOCOL ;
            ID.toi_id    = IF_MIB_STATS_ID;
            Size = sizeof(IFEntry);

            memset(Context, 0, CONTEXT_SIZE);
            InitNDISBuff( &ndisbuff, &IFE, Size, NULL ) ;
            tdistatus = TdiVxdQueryInformationEx( 0,
                                                  &ID,
                                                  &ndisbuff,
                                                  &Size,
                                                  Context);

            if ( tdistatus != TDI_SUCCESS &&
                 tdistatus != TDI_BUFFER_OVERFLOW )
            {
                CDbgPrint( DEBUG_ERRORS, ("FindHardwareAddr: Getting interface info failed\r\n")) ;
                return tdistatus ;
            }

            if ( IFE.if_index == pIAE->iae_index )
            {
                ASSERT( IFE.if_physaddrlen <= sizeof( pHardwareAddress->Address )) ;

                pHardwareAddress->Length =
                                     min( sizeof( pHardwareAddress->Address ),
                                          IFE.if_physaddrlen ) ;
                memcpy( pHardwareAddress->Address,
                        IFE.if_physaddr,
                        pHardwareAddress->Length ) ;

                *pIfIndex = IFE.if_index ;
                *pfFound  = TRUE ;
                return TDI_SUCCESS ;
            }
        }
    }

    return TDI_SUCCESS ;
}

#ifndef CHICAGO
#pragma END_INIT
#endif  // !CHICAGO

VOID
ProcessDhcpRequestForever(
    CTEEvent * pCTEEvent,
    PVOID      pContext
    )
/*++

Routine Description:

    This function processes DHCP renewals.  To reset the sleep time, simply
    call this function again.  The existing timer will be stopped and the
    new sleep time will be recalculated.

Arguments:

    None.

--*/
{
    //
    //  If we're in the midst of initializing ourselves, just
    //  call through directly to DelayedProcessDhcpRequest.  Otherwise,
    //  schedule an event so we'll get called at a more friendly time.
    //

    if( fInInit )
    {
        DelayedProcessDhcpRequest( pCTEEvent, pContext );
    }
    else
    {
        CTEInitEvent( &ProcessEvent, DelayedProcessDhcpRequest );
        CTEScheduleEvent( &ProcessEvent, pContext );
    }

}   // ProcessDhcpRequestForever

VOID
DelayedProcessDhcpRequest(
    CTEEvent * pCTEEvent,
    PVOID      pContext
    )
/*++

Routine Description:

    This function processes DHCP renewals.  To reset the sleep time, simply
    call this function again.  The existing timer will be stopped and the
    new sleep time will be recalculated.

Arguments:

    None.

--*/
{
    DWORD LocalTimeToSleep  ;
    PDHCP_CONTEXT DhcpContext;
    time_t TimeNow;
    PLIST_ENTRY ListEntry;

    LocalTimeToSleep = INFINIT_LEASE;
    TimeNow = time( NULL );

    //
    // Loop through the list of DHCP contexts looking for any
    // renewals to run.  Also, reset timeToSleep to the nearest
    // future renewal.
    //

    for( ListEntry = DhcpGlobalRenewList.Flink;
         ListEntry != &DhcpGlobalRenewList;
         ListEntry = ListEntry->Flink ) {

        DhcpContext = CONTAINING_RECORD(
                        ListEntry,
                        DHCP_CONTEXT,
                        RenewalListEntry );

        //
        // If it is time to run this renewal function, remove the
        // renewal context from the list.
        //

        if ( DhcpContext->RunTime <= TimeNow ) {

            //
            // This client has to renew NOW.
            //
            // This client is removed from the list for renewal,
            // when the renewal is performed his entry will be
            // queued again
            //

            RemoveEntryList( &DhcpContext->RenewalListEntry );
            DhcpContext->RenewalFunction( DhcpContext, &LocalTimeToSleep );

        } else {

            DWORD ElapseTime;

            ElapseTime = DhcpContext->RunTime - TimeNow;

            if ( LocalTimeToSleep > ElapseTime ) {
                LocalTimeToSleep = ElapseTime;
            }
        }
    }

    //
    //  This list may be empty or only contain infinitely leased items
    //
    if ( LocalTimeToSleep == INFINIT_LEASE )
    {
#ifndef CHICAGO
        //
        //  This is S.O.P. for Chicago Plug & Play.  For Snowball,
        //  we'll whine about it.
        //

        DbgPrint("DelayedProcessDhcpRequest: Infinite lease, disabling renewal list timer\r\n") ;
#endif  // !CHICAGO
        CTEStopTimer( &RenewalTimer ) ;
        return ;
    }

    DhcpPrint(( DEBUG_MISC, "DelayedProcessDhcpRequest: Sleeping for 0x%x seconds", LocalTimeToSleep )) ;

    CTEStopTimer( &RenewalTimer ) ;     // Ok even if timer not running
    CTEInitTimer( &RenewalTimer ) ;

    //
    // don't sleep more than a day long.
    //

    if(  LocalTimeToSleep > DAY_LONG_SLEEP ) {
        LocalTimeToSleep = DAY_LONG_SLEEP;
    }

    //
    //  For test purposes!!  Renew every thirty seconds
    //
    //LocalTimeToSleep = 30 ;
    //

    if ( !CTEStartTimer( &RenewalTimer,
                         LocalTimeToSleep * 1000,
                         ProcessDhcpRequestForever,
                         NULL ))
    {
        CDbgPrint( DEBUG_ERRORS, ("DelayedProcessDhcpRequest: Warning - Failed to start timer!!\r\n")) ;
    }
}


#pragma BEGIN_INIT

/*******************************************************************

    NAME:       VxdReadIniString

    SYNOPSIS:   Vxd stub for CTEReadIniString

    ENTRY:      pchKey - Key value to look for in the Network section
                ppchString - Pointer to buffer found string is returned in

    EXIT:       ppchString will point to an allocated buffer

    RETURNS:    STATUS_SUCCESS if found

    NOTES:      The client must free ppchString when done with it

    HISTORY:
        Johnl   30-Aug-1993     Created

********************************************************************/

CHAR * DhcpGetProfileString( LPTSTR pchKey, LPTSTR * pchDefault ) ;

NTSTATUS VxdReadIniString( LPSTR pchKey, LPSTR * ppchString )
{
    char * pchTmp ;

    if ( pchTmp = DhcpGetProfileString( pchKey, NULL ) )
    {
        if ( *ppchString = CTEAllocInitMem( (USHORT) (strlen( pchTmp ) + 1)))
        {
            strcpy( *ppchString, pchTmp ) ;
            return STATUS_SUCCESS ;
        }
        else
            return STATUS_INSUFFICIENT_RESOURCES ;
    }

    return STATUS_UNSUCCESSFUL ;
}
#pragma END_INIT

//
//  Allow this to be called during non-init but don't retry if non-init
//
PVOID CTEAllocInitMem( USHORT cbBuff )
{
    PVOID pv = CTEAllocMem( cbBuff ) ;

    if ( pv )
    {
        return pv ;
    }
    else if ( fInInit )
    {
        CDbgPrint( DEBUG_ERRORS, ("CTEAllocInitMem: Failed allocation, trying again\r\n")) ;
        CTERefillMem() ;
        pv = CTEAllocMem( cbBuff ) ;
    }

    return pv ;
}


#ifdef DEBUG

VOID
DbgNotificationRoutine(
    CTEEvent * pCTEEvent,
    PVOID      pContext
    )
/*++

Routine Description:

    This function can be used for testing the client's DHCP change of
    address notification code

Arguments:

    None.

--*/
{
    PDHCP_CONTEXT DhcpContext;
    PLIST_ENTRY ListEntry;

    for( ListEntry = DhcpGlobalRenewList.Flink;
         ListEntry != &DhcpGlobalRenewList;
         ListEntry = ListEntry->Flink ) {

        DhcpContext = CONTAINING_RECORD(
                        ListEntry,
                        DHCP_CONTEXT,
                        RenewalListEntry );

        //
        //  This just sets the old address to the new address
        //
        NotifyClients( DhcpContext,
                       DhcpContext->IpAddress,      // Old address
                       DhcpContext->IpAddress,      // New address
                       DhcpContext->SubnetMask ) ;

    }

    CTEInitTimer( &DbgNotificationTimer ) ;
    CTEStartTimer( &DbgNotificationTimer,
                   DbgNotifyTime,
                   DbgNotificationRoutine,
                   NULL ) ;
}

#endif //DEBUG

#ifdef CHICAGO

DWORD
DhcpRequestAddress(
    ushort IpContext
    )
/*++

Routine Description:

    This function is invoked by IP when a new network adapter comes
    on line and requires a DHCP lease aquisition/renewal.

Arguments:

    IpContext - The IP context for the newly installed adapter.

Return Value:

    DWORD - Completion status.  !0 if successful, 0 if failed.

--*/
{
    return (DWORD)ScanIpEntities( TRUE, IpContext );

}   // DhcpRequestAddress

#endif  // CHICAGO

