/*++

Copyright (c) 2000-1993  Microsoft Corporation

Module Name:

    nwreg.h

Abstract:

    Header which specifies the registry location and value names
    of the logon credential information written by the provider and
    read by the workstation service.

    Also contains helper routine prototypes.

Author:

    Rita Wong      (ritaw)      22-Mar-1993

Revision History:

--*/

#ifndef _NWRNAMES_INCLUDED_
#define _NWRNAMES_INCLUDED_


#define NW_WORKSTATION_REGKEY              L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Parameters"
#define NW_WORKSTATION_OPTION_REGKEY       L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Parameters\\Option"
#define NW_WORKSTATION_LOGON_REGKEY        L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Parameters\\Logon"
#define NW_SERVICE_LOGON_REGKEY            L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Parameters\\ServiceLogon"
#define NW_WORKSTATION_GATEWAY_DRIVES      L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Drives"
#define NW_WORKSTATION_GATEWAY_SHARES      L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Shares"


#define NW_CURRENTUSER_VALUENAME     L"CurrentUser"
#define NW_GATEWAYACCOUNT_VALUENAME  L"GatewayAccount"
#define NW_GATEWAY_ENABLE            L"GatewayEnabled"

#define NW_SERVER_VALUENAME          L"PreferredServer"
#define NW_LOGONID_VALUENAME         L"LogonID"
#define NW_PRINTOPTION_VALUENAME     L"PrintOption"

#endif // _NWRNAMES_INCLUDED_
