
/*

UoB WinAODV 0.1 - An AODV (RFC 3561) protocol handler for Microsoft Windows XP
Copyright 2004 ComNets, University of Bremen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/




/*
* Functions in this file provide services for manipulating the  
* system routing table.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "OSOperations.h"

HINSTANCE Hinstance;
DWORD Error, Count;
HANDLE Handle;
OVERLAPPED Overlapped, OverlappedUnenableRouter;
//DWORD (WINAPI* EnableRouter)(PHANDLE, LPOVERLAPPED);
//DWORD (WINAPI* UnenableRouter)(LPOVERLAPPED, LPDWORD);

void OSOpsInit()
{
	OSOpsSetGatewayMAC();
	EnableRoutingCapability();
}

/*
* Adds the static routes to the internal route list so that 
* any packets that use these routes are let thru by the driver
*/
void OSOpsSetStaticRoutes(PRouteListEntry pRtListHead)
{
    PMIB_IPFORWARDTABLE pIpRouteTab = NULL; // Ip routing table
    MIB_IPFORWARDROW InternalRtEntry;            // Ip routing table row entry
	RouteEntry NewRtEntry;
    DWORD dwStatus, i;
	char PrintBuf[2048];

    if( (dwStatus = MyGetIpForwardTable(&pIpRouteTab)) == NO_ERROR)
    {
        for (i = 0; i < pIpRouteTab->dwNumEntries; i++)
        {
			memcpy(&InternalRtEntry, &(pIpRouteTab->table[i]), 
							sizeof(MIB_IPFORWARDROW));
			if(InternalRtEntry.dwForwardDest == 0)
			{
				continue;
			}
			memset(&NewRtEntry, 0, sizeof(NewRtEntry));
			NewRtEntry.DestIPAddr.S_un.S_addr = InternalRtEntry.dwForwardDest;
			NewRtEntry.DestIPAddrMask.S_un.S_addr = InternalRtEntry.dwForwardMask;
			NewRtEntry.NextHopIPAddr.S_un.S_addr = InternalRtEntry.dwForwardNextHop;
			NewRtEntry.RouteType = ROUTE_TYPE_STATIC;
			PrecursorListInit(&NewRtEntry.PrecursorListHead);

			RouteListInsertAtOrder(pRtListHead, &NewRtEntry);
			
			sprintf(PrintBuf, "OSOperation : Static route created - ");
			BuildRouteEntryAsString(&NewRtEntry, (PrintBuf + strlen(PrintBuf)));

			EnterCriticalSection(pLoggingLock);
			LogMessage(LOGGING_SEVERAITY_INFO, PrintBuf);
			LeaveCriticalSection(pLoggingLock);

        }
        free(pIpRouteTab);
        return;
    }
    else if ( dwStatus == ERROR_NO_DATA)
    {
        if (pIpRouteTab)
            free(pIpRouteTab);
        return;
    }
    else
    {
        if (pIpRouteTab)
            free(pIpRouteTab);
        return;
    }
}

void OSOpsAddRoute(PRouteEntry pRouteEntry)
{
    DWORD dwStatus;

    MIB_IPFORWARDROW routeEntry;            // Ip routing table row entry
    PMIB_IPADDRTABLE pIpAddrTable = NULL;   // Ip Addr Table
    DWORD dwIfIndex;                        // Interface index number  
    DWORD dwIfMask;                         // Interface Subnet Mask
    DWORD dwIfIpAddr;                       // Interface Ip Address
	DWORD dwMetric = 1;
	char PrintBuf[2048];

    memset(&routeEntry, 0, sizeof(MIB_IPFORWARDROW));
    
	routeEntry.dwForwardDest = pRouteEntry->DestIPAddr.S_un.S_addr;
	routeEntry.dwForwardMask = pRouteEntry->DestIPAddrMask.S_un.S_addr;
	routeEntry.dwForwardNextHop = pRouteEntry->NextHopIPAddr.S_un.S_addr;

	dwIfIpAddr = ParaIPAddress.S_un.S_addr;

    // Check if we have the given interface
    if ( (dwStatus = MyGetIpAddrTable(&pIpAddrTable)) != NO_ERROR)
    {
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"OSOperation : MyGetIpAddrTable() failed");
		LeaveCriticalSection(pLoggingLock);
        if (pIpAddrTable)
		{
            free(pIpAddrTable);
		}
        return;
    }
    //assert(pIpAddrTable);

	if ( InterfaceIpToIdxAndMask(pIpAddrTable, dwIfIpAddr, &dwIfIndex, &dwIfMask) == FALSE)
    {
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"OSOperation : InterfaceIpToIdxAndMask() failed");
		LeaveCriticalSection(pLoggingLock);
        return;
    }
    free(pIpAddrTable);

	routeEntry.dwForwardIfIndex = dwIfIndex;
	//if(pRouteEntry->DestIPAddr.S_un.S_addr == pRouteEntry->NextHopIPAddr.S_un.S_addr)
	//{
	//	// next hop is final destination
	//	routeEntry.dwForwardType = 3; 
	//}
	//else
	//{
	//	// next hop is not final destination
	//	routeEntry.dwForwardType = 4; 
	//}

    routeEntry.dwForwardMetric1 = dwMetric;

    // some default values
    //routeEntry.dwForwardProto = MIB_IPPROTO_LOCAL;
    routeEntry.dwForwardProto = MIB_IPPROTO_NT_STATIC;
    //routeEntry.dwForwardProto = MIB_IPPROTO_NETMGMT;
    routeEntry.dwForwardMetric2 = (DWORD)-1;
    routeEntry.dwForwardMetric3 = (DWORD)-1;
    routeEntry.dwForwardMetric4 = (DWORD)-1;
    
    dwStatus = SetIpForwardEntry(&routeEntry); 
    //dwStatus = CreateIpForwardEntry(&routeEntry); 
    if (dwStatus != NO_ERROR)
    {
		sprintf(PrintBuf, "OSOperation : Setting route to %s failed",
					inet_ntoa((struct in_addr) pRouteEntry->DestIPAddr));
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR, PrintBuf);
		LeaveCriticalSection(pLoggingLock);
    }
}

void OSOpsDeleteRoute(PRouteEntry pRouteEntry)
{
    DWORD dwStatus, dwDelStatus, i;
    PMIB_IPFORWARDTABLE pIpRouteTab = NULL; // Ip routing table
    MIB_IPFORWARDROW routeEntry;            // Ip routing table row entry
    DWORD dwForwardDest = 0;
    BOOL fDeleted = FALSE;
	char PrintBuf[2048];
    
    memset(&routeEntry, 0, sizeof(MIB_IPFORWARDROW));
	dwForwardDest = pRouteEntry->DestIPAddr.S_un.S_addr;

    if ( (dwStatus = MyGetIpForwardTable(&pIpRouteTab)) == NO_ERROR)
    {
        for (i = 0; i < pIpRouteTab->dwNumEntries; i++)
        {
            if (dwForwardDest == pIpRouteTab->table[i].dwForwardDest)
            {
                memcpy(&routeEntry, &(pIpRouteTab->table[i]), sizeof(MIB_IPFORWARDROW));
                dwDelStatus = DeleteIpForwardEntry(&routeEntry); 
                if (dwDelStatus != NO_ERROR)
                {
					sprintf(PrintBuf, "OSOperation : Deleting route to %s failed",
							inet_ntoa((struct in_addr) pRouteEntry->DestIPAddr));
					EnterCriticalSection(pLoggingLock);
					LogMessage(LOGGING_SEVERAITY_ERROR, PrintBuf);
					LeaveCriticalSection(pLoggingLock);
					free(pIpRouteTab);
                    return;
                }
                else
                    fDeleted = TRUE;
            }
        }
        free(pIpRouteTab);
        return;
    }
    else if ( dwStatus == ERROR_NO_DATA)
    {
        if (pIpRouteTab)
            free(pIpRouteTab);
        return;
    }
    else
    {
        if (pIpRouteTab)
            free(pIpRouteTab);
        return;
    }
}

void OSOpsSetGatewayMAC()
{
    DWORD dwInetAddr = 0; // ip address
    DWORD dwStatus;
    BYTE bPhysAddr[MAXLEN_PHYSADDR]; 
    MIB_IPNETROW arpEntry; // an arp entry
	char PrintBuf[2048];
    PMIB_IPADDRTABLE pIpAddrTable = NULL;   // Ip Addr Table
    DWORD dwIfIndex;                        // Interface index number  
    DWORD dwIfMask;                         // Interface Subnet Mask
    DWORD dwIfIpAddr;                       // Interface Ip Address
	DWORD dwMetric = 1;

	dwInetAddr = ParaGatewayIPAddress.S_un.S_addr;
	memset((void *) bPhysAddr, 0, MAXLEN_PHYSADDR);

    // Check if we have the given interface
    if ( (dwStatus = MyGetIpAddrTable(&pIpAddrTable)) != NO_ERROR)
    {
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"OSOperation : MyGetIpAddrTable() failed");
		LeaveCriticalSection(pLoggingLock);
        if (pIpAddrTable)
		{
            free(pIpAddrTable);
		}
        return;
    }
    //assert(pIpAddrTable);

	dwIfIpAddr = ParaIPAddress.S_un.S_addr;
	if ( InterfaceIpToIdxAndMask(pIpAddrTable, dwIfIpAddr, &dwIfIndex, &dwIfMask) == FALSE)
    {
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"OSOperation : InterfaceIpToIdxAndMask() failed");
		LeaveCriticalSection(pLoggingLock);
        return;
    }
    free(pIpAddrTable);

	arpEntry.dwIndex = dwIfIndex;
    arpEntry.dwPhysAddrLen = 6;
    memcpy(arpEntry.bPhysAddr, bPhysAddr, 6);
    arpEntry.dwAddr = dwInetAddr;
    arpEntry.dwType = MIB_IPNET_TYPE_STATIC; //static arp entry
    dwStatus = SetIpNetEntry(&arpEntry);
    if (dwStatus != NO_ERROR)
    {
		sprintf(PrintBuf, "OSOperation : Setting MAC to Gateway %s failed",
					inet_ntoa((struct in_addr) ParaGatewayIPAddress));
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR, PrintBuf);
		LeaveCriticalSection(pLoggingLock);
		return;
	}
}

DWORD MyGetIpForwardTable(PMIB_IPFORWARDTABLE *pIpRouteTab)
{
    DWORD status = NO_ERROR;
    DWORD statusRetry = NO_ERROR;
    DWORD dwActualSize = 0;
    
    // query for buffer size needed
    status = GetIpForwardTable(*pIpRouteTab, &dwActualSize, TRUE);

    if (status == NO_ERROR)
    {
        return status;
    }
    else if (status == ERROR_INSUFFICIENT_BUFFER)
    {
        // need more space

        (*pIpRouteTab) = (PMIB_IPFORWARDTABLE) malloc(dwActualSize);
        //assert((*pIpRouteTab));
        if((*pIpRouteTab) == NULL)
		{
			return status;
		}

        statusRetry = GetIpForwardTable(*pIpRouteTab, &dwActualSize, TRUE);
        return statusRetry;
    }
    else
    {
        return status;
    }
}

DWORD MyGetIpAddrTable(PMIB_IPADDRTABLE *pIpAddrTable)
{
    DWORD status = NO_ERROR;
    DWORD statusRetry = NO_ERROR;
    DWORD dwActualSize = 0;
    
    // query for buffer size needed
    status = GetIpAddrTable(*pIpAddrTable, &dwActualSize, TRUE);

    if (status == NO_ERROR)
    {
        return status;
    }
    else if (status == ERROR_INSUFFICIENT_BUFFER)
    {
        // need more space

        (*pIpAddrTable) = (PMIB_IPADDRTABLE) malloc(dwActualSize);
        //assert((*pIpAddrTable));
		if((*pIpAddrTable) == NULL)
		{
			return status;
		}
        
        statusRetry = GetIpAddrTable(*pIpAddrTable, &dwActualSize, TRUE);
        return statusRetry;
    }
    else
    {
        return status;
    }
}

BOOL InterfaceIpToIdxAndMask(PMIB_IPADDRTABLE pIpAddrTable, DWORD dwIfIpAddr, DWORD *dwIndex, DWORD *dwMask)
{
    if (pIpAddrTable == NULL)
        return FALSE;

    if (dwIfIpAddr == INADDR_NONE)
        return FALSE;

    for (DWORD dwIdx = 0; dwIdx < pIpAddrTable->dwNumEntries; dwIdx++)
    {
        if (dwIfIpAddr == pIpAddrTable->table[dwIdx].dwAddr)
        {
            *dwIndex = pIpAddrTable->table[dwIdx].dwIndex;
            *dwMask = pIpAddrTable->table[dwIdx].dwMask;
            return TRUE;
        }
    }
    return FALSE;
}

/*
* Function enables the routing capability of XP
*/
void EnableRoutingCapability()
{
	Hinstance = LoadLibrary(TEXT("IPHLPAPI.DLL"));
    if (!Hinstance)
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"OSOperation : OSOpsInit() failed");
		LeaveCriticalSection(pLoggingLock);
		return;
    }

//	EnableRouter = (DWORD (WINAPI*)(PHANDLE, LPOVERLAPPED))
//						GetProcAddress(Hinstance, "EnableRouter");
//	UnenableRouter = (DWORD (WINAPI*)(LPOVERLAPPED, LPDWORD))
//						GetProcAddress(Hinstance, "UnenableRouter");
//	if (!EnableRouter || !UnenableRouter)
//	{
//		EnterCriticalSection(pLoggingLock);
//		LogMessage(LOGGING_SEVERAITY_ERROR,
//					"OSOperation : OSOpsInit() failed");
//		LeaveCriticalSection(pLoggingLock);
//		return;
//	}

	ZeroMemory(&Overlapped, sizeof(Overlapped));
	Overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!Overlapped.hEvent)
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"OSOperation : EnableRoutingCapability() failed");
		LeaveCriticalSection(pLoggingLock);
		return;
	}

	Error = EnableRouter(&Handle, &Overlapped);
	if (Error != ERROR_IO_PENDING)
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"OSOperation : EnableRoutingCapability() failed");
		LeaveCriticalSection(pLoggingLock);
		return;
	}
	
	EnterCriticalSection(pLoggingLock);
	LogMessage(LOGGING_SEVERAITY_INFO,
					"OSOperation : Forwarding capability enabled ");
	LeaveCriticalSection(pLoggingLock);
}

/*
* Function disables the routing capability of XP
*/
void DisableRoutingCapability()
{
	Error = UnenableRouter(&Overlapped, &Count);
	if (Error)
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"OSOperation : DisableRoutingCapability() failed");
		LeaveCriticalSection(pLoggingLock);
		return;
    }

	EnterCriticalSection(pLoggingLock);
	LogMessage(LOGGING_SEVERAITY_INFO,
					"OSOperation : Forwarding capability disabled ");
	LeaveCriticalSection(pLoggingLock);
}

/*
* Terminates all OS operations
*/
void OSOpsTerm()
{
	DisableRoutingCapability();
}
