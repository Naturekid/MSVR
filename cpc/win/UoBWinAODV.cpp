
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
* Function in this file provide all the functionality
* required by the different threads that manage the 
* AODV operation. 
*
* @author : Asanga Udugama
* @date : 11-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "../cpc_prot.h"
#include "stdafx.h"
#include "UoBWinAODV.h"

char *pConfigFileName;

// AODV parameters
int ParaExecutionMode;
int ParaIPVersion;
IN_ADDR ParaIPAddress;
char ParaIfaceName[MAX_IFACE_SIZE];
IN_ADDR ParaGatewayIPAddress;
int ParaLoggingStatus;
int ParaLoggingMode;
int ParaLoggingLevel;
char ParaLogFile[MAX_LOG_FILE_NAME_SIZE];
int ParaOnlyDestination;
int ParaGratuitousRREP;
int ParaRREPAckRequired;
IN_ADDR ParaIPAddressMulticast;
int ParaRERRSendingMode;
int ParaDeletePeriodMode;
int ParaRouteDiscoveryMode;
int ParaPacketBuffering;
int ParaPrintFrequency;
int ParaActiveRouteTimeout;
int ParaAllowedHelloLoss;
int ParaHelloInterval;
int ParaLocalAddTTL;
int ParaNetDiameter;
int ParaNodeTraversalTime;
int ParaRERRRatelimit;
int ParaRREQRetries;
int ParaRREQRateLimit;
int ParaTimeoutBuffer;
int ParaTTLStart;
int ParaTTLIncrement;
int ParaTTLThreshold;
int ParaNetTraversalTime;
int ParaBlacklistTimeout;
int ParaMaxRepairTTL;
int ParaMyRouteTimeout;
int ParaNextHopWait;
int ParaPathDiscoveryTime;
int ParaDeletePeriod;

// Current info
ULONG CurrLocalSeqNum;
ULONG CurrLocalRREQID;

// lists used and pointers to them
IPPacketListEntry IPPacketListHead;
PIPPacketListEntry pIPPacketListHead;

RouteListEntry RouteListHead;
PRouteListEntry pRouteListHead;
RREQIDListEntry RREQIDListHead;
PRREQIDListEntry pRREQIDListHead;
RREQIDExpiryListEntry RREQIDExpiryListHead;
PRREQIDExpiryListEntry pRREQIDExpiryListHead;
RouteDiscoveryListEntry RouteDiscoveryListHead;
PRouteDiscoveryListEntry pRouteDiscoveryListHead;
RouteDiscoveryTimerListEntry RouteDiscoveryTimerListHead;
PRouteDiscoveryTimerListEntry pRouteDiscoveryTimerListHead;
RouteExpiryListEntry RouteExpiryListHead;
PRouteExpiryListEntry pRouteExpiryListHead;
DeleteExpiryListEntry DeleteExpiryListHead;
PDeleteExpiryListEntry pDeleteExpiryListHead;
HelloExpiryListEntry HelloExpiryListHead;
PHelloExpiryListEntry pHelloExpiryListHead;

// WinAODV lock
CRITICAL_SECTION WinAODVLock;
LPCRITICAL_SECTION pWinAODVLock;

// PtDriver lock
CRITICAL_SECTION PtDriverLock;
LPCRITICAL_SECTION pPtDriverLock;

/*
* Performs all initialization activities and starts the protocol
* handler.
*/
void BeginAODVOperations()
{
	// initializes the main lock
	pWinAODVLock = &WinAODVLock;
	InitializeCriticalSection(pWinAODVLock);

	// hold lock until all init operations
	EnterCriticalSection(pWinAODVLock);

	// initializes the PT driver lock
	pPtDriverLock = &PtDriverLock;
	InitializeCriticalSection(pPtDriverLock);

	// log init
	LoggingInit();

	EnterCriticalSection(pLoggingLock);
	LogMessage(LOGGING_SEVERAITY_INFO, 
		"UoBWinAODV : ---- Starting UoBWinAODV Initialization ----"); 
	LeaveCriticalSection(pLoggingLock);

	// initialize the routing environment
	OSOpsInit();

	// set list pointers
	pRouteListHead = &RouteListHead;
	pRREQIDListHead = &RREQIDListHead;
	pRREQIDExpiryListHead = &RREQIDExpiryListHead;

	pRouteDiscoveryListHead = &RouteDiscoveryListHead;
	pRouteDiscoveryTimerListHead = &RouteDiscoveryTimerListHead;
	pRouteExpiryListHead = &RouteExpiryListHead;
	pDeleteExpiryListHead = &DeleteExpiryListHead;
	pHelloExpiryListHead = &HelloExpiryListHead;

	// initialize all the lists
	RouteListInit(pRouteListHead);
	RREQIDListInit(pRREQIDListHead);
	RREQIDExpiryListInit(pRREQIDExpiryListHead);
	RouteDiscoveryListInit(pRouteDiscoveryListHead);
	RouteDiscoveryTimerListInit(pRouteDiscoveryTimerListHead);
	RouteExpiryListInit(pRouteExpiryListHead);
	DeleteExpiryListInit(pDeleteExpiryListHead);
	HelloExpiryListInit(pHelloExpiryListHead);

	// update all static routes
	UpdateStaticRoutes();

	// ShowRouteList();
	//DumpInternalRouteTable();
	
	// initialize packet manipulators
	PacketSenderInit();
	PacketReceiverInit();

	// initialize the threads that manage lifetimes
	//HelloSendThreadInit();
	//RREQIDExpiryThreadInit();
	RouteExpiryThreadInit();
	DeleteExpiryThreadInit();
	//HelloExpiryThreadInit();
	//RouteDiscoveryTimerThreadInit();
	//StatusPrinterThreadInit();

	// release lock
	LeaveCriticalSection(pWinAODVLock);

	EnterCriticalSection(pLoggingLock);
	LogMessage(LOGGING_SEVERAITY_INFO, 
		"UoBWinAODV : ---- UoBWinAODV Initialization Completed ----"); 
	LeaveCriticalSection(pLoggingLock);
}

/*
* Funtion that holds the main thread of WinAODV until the user
* presses Q, to quit.
*/
void LetAODVWork()
{
	if(ParaExecutionMode == EXECUTION_MODE_GUI)
	{
		_tprintf(_T("\nTo display status info graphically, run the GUI.\n") );
	}

	_tprintf(_T("\n") );
	_tprintf(_T("Press \"Q\" (uppercase Q) to terminate UoBWinAODV...") );
	while(_getch() != 'Q')
	{
		continue;
	}

	_tprintf(_T("\n\n") );
	_tprintf(_T("You have pressed Q\n") );
	_tprintf(_T("Stopping UoBWinAODV. Please wait....") );
}

/*
* Performs all termination activities
*/
void EndAODVOperations()
{

	EnterCriticalSection(pLoggingLock);
	LogMessage(LOGGING_SEVERAITY_INFO, 
		"UoBWinAODV : ---- Starting UoBWinAODV Termination ----"); 
	LeaveCriticalSection(pLoggingLock);

	// hold lock to start term operations
	EnterCriticalSection(pWinAODVLock);

	// terminate threads that manage lifetimes
	StatusPrinterThreadTerm();
	RouteDiscoveryTimerThreadTerm();
	HelloExpiryThreadTerm();
	DeleteExpiryThreadTerm();
	RouteExpiryThreadTerm();
	RREQIDExpiryThreadTerm();
	HelloSendThreadTerm();

	InactivateRoutes();

	// terminate packet manipulators
	PacketReceiverTerm();
	PacketSenderTerm();

	// terminate all the lists
	HelloExpiryListTerm(pHelloExpiryListHead);
	DeleteExpiryListTerm(pDeleteExpiryListHead);
	RouteExpiryListTerm(pRouteExpiryListHead);
	RouteDiscoveryTimerListTerm(pRouteDiscoveryTimerListHead);
	RouteDiscoveryListTerm(pRouteDiscoveryListHead);
	RREQIDExpiryListTerm(pRREQIDExpiryListHead);
	RREQIDListTerm(pRREQIDListHead);
	RouteListTerm(pRouteListHead);

	// initialize the routing environment
	OSOpsTerm();

	EnterCriticalSection(pLoggingLock);
	LogMessage(LOGGING_SEVERAITY_INFO, 
		"UoBWinAODV : ---- UoBWinAODV Termination Completed ----"); 
	LeaveCriticalSection(pLoggingLock);

	// stop logging
	LoggingTerm();

	// release lock
	LeaveCriticalSection(pWinAODVLock);
}

//--------------- Manage lifetime of a route -------------------------//
/*
* Checks the lifetime of the route that was to expire. If the lifetime
* has expired, then send it to the Delete State. Then it gets the next
* route that was to expire from the expiry list. This function
* is called by the thread that manages route expiry.  
*/
int CheckActiveRouteLifetime(IN_ADDR *pDestIPAddr)
{
	RouteEntry DestRouteEntry;
	int Lifetime, NextWaitTime;
	BOOL Successful;

	//EnterCriticalSection(pWinAODVLock);

	if(pDestIPAddr->S_un.S_addr == 0)
	{
		goto exit_CheckActiveRouteLifetime;
	}

	Successful = RouteListViewByValue(pRouteListHead, pDestIPAddr, &DestRouteEntry);
	if(!Successful) 
	{
		goto exit_CheckActiveRouteLifetime;
	}

	// if for some reason the considered route expiry is not ACTIVE_THREAD_ROUTE_LIFETIME_MINDER
	// then stop this route expiry
	if(DestRouteEntry.ActiveThreadType != ACTIVE_THREAD_ROUTE_LIFETIME_MINDER) 
	{
		goto exit_CheckActiveRouteLifetime;
	}

	// if for some reason the route has become invalid, then stop the thread
	if(DestRouteEntry.RouteStatusFlag != ROUTE_STATUS_FLAG_VALID) 	
	{
		goto exit_CheckActiveRouteLifetime;
	}

	Lifetime = DestRouteEntry.ExpiryTime - GetCurMiliSecTime();
	if(Lifetime <= 0) 
	{
		// simply expire route and start route delete process
		DestRouteEntry.RouteStatusFlag = ROUTE_STATUS_FLAG_INVALID;
		DestRouteEntry.DestSeqNum++;
		DestRouteEntry.ExpiryTime = GetCurMiliSecTime() + ParaDeletePeriod;
		DestRouteEntry.ActiveThreadType = ACTIVE_THREAD_DELETE_PERIOD_MINDER;
		UpdateRoute(&DestRouteEntry, UPDATE_TYPE_DELETE);

		goto exit_CheckActiveRouteLifetime;
	} 
	else
	{
		goto exit_CheckActiveRouteLifetime;
	}

exit_CheckActiveRouteLifetime:
	NextWaitTime = GetNextExpiringActiveRoute(pDestIPAddr);
	//LeaveCriticalSection(pWinAODVLock);
	return NextWaitTime;
}

/*
* Returns the lifetime of the next route entry that will expire 
* from the active routes. If no active route is present, returns 
* fixed values. 
*/
int GetNextExpiringActiveRoute(IN_ADDR *pDestIPAddr)
{
	BOOL Success;
	RouteExpiryEntry NextRtExpiryEntry;
	int NextWaitTime;

	Success = RouteExpiryListRemoveFromTail(pRouteExpiryListHead, 
													&NextRtExpiryEntry);
	if(Success)
	{
		pDestIPAddr->S_un.S_addr = NextRtExpiryEntry.DestIPAddr.S_un.S_addr;
		NextWaitTime = NextRtExpiryEntry.ExpiryTime - GetCurMiliSecTime();
		if(NextWaitTime <= 0)
		{
			NextWaitTime = 500;
		}
	}
	else
	{
		// if no entry in the list, wait for a fixed time
		pDestIPAddr->S_un.S_addr = 0;
		NextWaitTime = 500;
	}

	return NextWaitTime;
}

//--------------- Manage lifetime of a deleting route ------------------//

/*
* Checks the lifetime of the route that was in Delete State and that was 
* to expire. If the lifetime has expired, then the route is removed from 
* the list. Then it gets the next route that was to expire in the Delete 
* State from the delete expiry list. This function is called by the thread 
* that manages delete expiry.  
*/
int CheckDeleteRouteLifetime(IN_ADDR *pDestIPAddr)
{
	RouteEntry DestRouteEntry;
	int Lifetime, NextWaitTime;
	BOOL Successful;

	//EnterCriticalSection(pWinAODVLock);

	//printf("Got control by CheckDeleteRouteLifetime() \n");

	if(pDestIPAddr->S_un.S_addr == 0)
	{
		goto exit_CheckDeleteRouteLifetime;
	}

	Successful = RouteListViewByValue(pRouteListHead, pDestIPAddr, &DestRouteEntry);
	if(!Successful) 
	{
		goto exit_CheckDeleteRouteLifetime;
	}

	// if for some reason the route has got some status other than invalid
	// then stop the thread
	if(DestRouteEntry.RouteStatusFlag != ROUTE_STATUS_FLAG_INVALID)
	{
		goto exit_CheckDeleteRouteLifetime;
	}

	// if delMinder is not the current thread that is managing the route,
	// then stop the thread
	if(DestRouteEntry.ActiveThreadType != ACTIVE_THREAD_DELETE_PERIOD_MINDER)
	{
		goto exit_CheckDeleteRouteLifetime;
	}

	Lifetime = DestRouteEntry.ExpiryTime - GetCurMiliSecTime();
	if(Lifetime <= 0) 
	{

		// if lifetime expired, delete the route from list
		RemoveRoute(&DestRouteEntry);

		goto exit_CheckDeleteRouteLifetime;

	} 
	else
	{
		goto exit_CheckDeleteRouteLifetime;
	}

exit_CheckDeleteRouteLifetime:
	NextWaitTime = GetNextExpiringDeleteRoute(pDestIPAddr);
	//LeaveCriticalSection(pWinAODVLock);
	return NextWaitTime;
}

/*
* Returns the lifetime of the next route entry that will expire 
* from the deleting routes. If no deleting route is present, returns 
* a fixed values. 
*/
int GetNextExpiringDeleteRoute(IN_ADDR *pDestIPAddr)
{
	BOOL Success;
	DeleteExpiryEntry NextDelExpiryEntry;
	int NextWaitTime;

	Success = DeleteExpiryListRemoveFromTail(pDeleteExpiryListHead, 
													&NextDelExpiryEntry);
	if(Success)
	{
		pDestIPAddr->S_un.S_addr = NextDelExpiryEntry.DestIPAddr.S_un.S_addr;
		NextWaitTime = NextDelExpiryEntry.ExpiryTime - GetCurMiliSecTime();
		if(NextWaitTime <= 0)
		{
			NextWaitTime = 500;
		}
	}
	else
	{
		pDestIPAddr->S_un.S_addr = 0;
		NextWaitTime = 500;
	}

	return NextWaitTime;
}

//-- Manage lifetime hellos (to determine link breakage) ------------//

/*
* Checks the lifetime of the route whose hello was expiring. If the lifetime 
* has expired, then the the route delete process is initiated by placing
* an entry in the delete expiry list. Then it gets the next hello expiry 
* from the hello expiry list. This function is called by the thread 
* that manages hello expiry.  
*/
int CheckHelloReceived(IN_ADDR *pDestIPAddr)
{
	RouteEntry DestRouteEntry;
	int NextWaitTime;
	BOOL Successful;

	//EnterCriticalSection(pWinAODVLock);

	if(pDestIPAddr->S_un.S_addr == 0)
	{
		goto exit_CheckHelloReceived;
	}

	Successful = RouteListViewByValue(pRouteListHead, pDestIPAddr, &DestRouteEntry);
	if(!Successful) 
	{
		goto exit_CheckHelloReceived;
	}

	if(DestRouteEntry.HopCount > 1)
	{
		goto exit_CheckHelloReceived;
	}

	// if no hellos heard but route is active, then generate RERRs
	// to all precursors and expire all the destinations which are
	// reachable thru this route and also expire himself
	if((GetCurMiliSecTime() > DestRouteEntry.NextHelloReceiveTime)
		&& (DestRouteEntry.RouteStatusFlag == ROUTE_STATUS_FLAG_VALID)) 
	{
		InvalidateDestinations(&DestRouteEntry);

		// remove the route
		DestRouteEntry.RouteStatusFlag = ROUTE_STATUS_FLAG_INVALID;
		DestRouteEntry.DestSeqNum++;
		DestRouteEntry.ExpiryTime = GetCurMiliSecTime() + ParaDeletePeriod;
		DestRouteEntry.ActiveThreadType = ACTIVE_THREAD_DELETE_PERIOD_MINDER;
		UpdateRoute(&DestRouteEntry, UPDATE_TYPE_DELETE);

		goto exit_CheckHelloReceived;
	}
	else
	{
		goto exit_CheckHelloReceived;
	}

exit_CheckHelloReceived:
	NextWaitTime = GetNextExpiringHelloReceived(pDestIPAddr);
	//LeaveCriticalSection(pWinAODVLock);
	return NextWaitTime;
}

/*
* Returns the lifetime of the next route entry that will expire 
* from the hellos expirint list. If no hello expiring route is 
* present, it returns a fixed values. 
*/
int GetNextExpiringHelloReceived(IN_ADDR *pDestIPAddr)
{
	BOOL Success;
	HelloExpiryEntry NextHelloExpiryEntry;
	int NextWaitTime;

	Success = HelloExpiryListRemoveFromTail(pHelloExpiryListHead, 
													&NextHelloExpiryEntry);
	if(Success)
	{
		pDestIPAddr->S_un.S_addr = NextHelloExpiryEntry.DestIPAddr.S_un.S_addr;
		NextWaitTime = NextHelloExpiryEntry.ExpiryTime - GetCurMiliSecTime();
		if(NextWaitTime <= 0)
		{
			NextWaitTime = 500;
		}
	}
	else
	{
		pDestIPAddr->S_un.S_addr = 0;
		NextWaitTime = 500;
	}

	return NextWaitTime;
}

/*
* Send RERRs to precursors dependent on a route that has expired 
* and no hellos have been heard.
*/
void InvalidateDestinations(PRouteEntry pDestRouteEntry)
{
	// if RERR unicast, send a RERR to each precursor in a
	// invalidating route
	if(ParaRERRSendingMode == RERR_SEND_MODE_UNICAST)
	{
		InvalidateDestinationsUnicast(pDestRouteEntry);
	}
	else 
	{
		// multicast not implemented 
		InvalidateDestinationsUnicast(pDestRouteEntry);
	}
}

/*
* Send unicast RERRs to precursors dependent on a route that has 
* expired and no hellos have been heard.
*/
void InvalidateDestinationsUnicast(PRouteEntry pDestRouteEntry)
{
	RouteEntry CheckingRtEntry;
	RERRMessage NewRERRMsg;
	ULONG i, j;
	BOOL Success;

	for(i = 0; i < RouteListSize(); i++) 
	{
		Success = RouteListViewByIndex(pRouteListHead, (ULONG) i, &CheckingRtEntry);
		if(!Success)
		{
			continue;
		}

		// send RERR only if the 'link break' (destRte) route
		// was the next hop
		if((CheckingRtEntry.DestIPAddr.S_un.S_addr 
								!= pDestRouteEntry->DestIPAddr.S_un.S_addr)
			&& (CheckingRtEntry.NextHopIPAddr.S_un.S_addr 
								== pDestRouteEntry->DestIPAddr.S_un.S_addr))
		{

			if(CheckingRtEntry.ValidDestSeqNumFlag == DEST_SEQ_FLAG_VALID) {
				CheckingRtEntry.DestSeqNum++;
			}

			// send RERR to each precursor
			for(j = 0; j < CheckingRtEntry.PrecursorListHead.PrecursorCount; j++) 
			{
				PrecursorEntry CheckingPrecurEntry;

				Success = PrecursorListViewByIndex(&CheckingRtEntry.PrecursorListHead, 
														j,
														&CheckingPrecurEntry);
				if(!Success)
				{
					continue;
				}

				memset(&NewRERRMsg, 0, sizeof(NewRERRMsg));
				NewRERRMsg.ToIPAddr.S_un.S_addr 
					= CheckingPrecurEntry.DestIPAddr.S_un.S_addr;
				NewRERRMsg.FromIPAddr.S_un.S_addr 
					= ParaIPAddress.S_un.S_addr;
				NewRERRMsg.TTLValue = 1;
				NewRERRMsg.MultiCast = FALSE;
				strcpy(NewRERRMsg.pIfaceName, ParaIfaceName);

				NewRERRMsg.NoDelFlag = FALSE;
				NewRERRMsg.DestCount = 1;

				NewRERRMsg.Unreachable[0].DestIPAddr.S_un.S_addr 
						= CheckingRtEntry.DestIPAddr.S_un.S_addr;
				NewRERRMsg.Unreachable[0].DestSeqNum 
					= CheckingRtEntry.DestSeqNum;

				SendRERR(&NewRERRMsg);
			}

			// simply expire route and start route delete process
			CheckingRtEntry.RouteStatusFlag = ROUTE_STATUS_FLAG_INVALID;
			//CheckingRtEntry.DestSeqNum++;
			CheckingRtEntry.ExpiryTime = GetCurMiliSecTime() + ParaDeletePeriod;
			CheckingRtEntry.ActiveThreadType = ACTIVE_THREAD_DELETE_PERIOD_MINDER;
			UpdateRoute(&CheckingRtEntry, UPDATE_TYPE_DELETE);
		}
	}

	// send RERR to the precursors of the link broken route
	for(j = 0; j < pDestRouteEntry->PrecursorListHead.PrecursorCount; j++) 
	{
		PrecursorEntry CheckingPrecurEntry;

		Success = PrecursorListViewByIndex(&pDestRouteEntry->PrecursorListHead, 
											j,
											&CheckingPrecurEntry);
		if(!Success)
		{
			continue;
		}

		memset(&NewRERRMsg, 0, sizeof(NewRERRMsg));
		NewRERRMsg.ToIPAddr.S_un.S_addr 
					= CheckingPrecurEntry.DestIPAddr.S_un.S_addr;
		NewRERRMsg.FromIPAddr.S_un.S_addr 
					= ParaIPAddress.S_un.S_addr;
		NewRERRMsg.TTLValue = 1;
		NewRERRMsg.MultiCast = FALSE;
		strcpy(NewRERRMsg.pIfaceName, ParaIfaceName);

		NewRERRMsg.NoDelFlag = FALSE;
		NewRERRMsg.DestCount = 1;

		NewRERRMsg.Unreachable[0].DestIPAddr.S_un.S_addr 
						= pDestRouteEntry->DestIPAddr.S_un.S_addr;
		NewRERRMsg.Unreachable[0].DestSeqNum 
					= pDestRouteEntry->DestSeqNum;

		SendRERR(&NewRERRMsg);
	}
}

//--------------- Manage lifetime of a RREQID ------------------//
/*
* Remove RREQIDs from the list when they expire. Then, takes the 
* next entry to expire and the duration. This is called from 
* the thread RREQIDExpiryThread.
*/
int RemoveRREQIDAtExpiry(IN_ADDR *pOrigIPAddr, ULONG *pOrigRREQID)
{
	RREQIDEntry OldRREQIDEntry;
	int NextWaitTime;

	//EnterCriticalSection(pWinAODVLock);

	if(pOrigIPAddr->S_un.S_addr == 0 && *pOrigRREQID == 0)
	{
		goto exit_RemoveRREQIDAtExpiry;
	}

	RREQIDListRemoveByValue(pRREQIDListHead, pOrigIPAddr, pOrigRREQID,
							&OldRREQIDEntry);
	goto exit_RemoveRREQIDAtExpiry;

exit_RemoveRREQIDAtExpiry:
	NextWaitTime = GetNextExpiringRREQID(pOrigIPAddr, pOrigRREQID);
	//LeaveCriticalSection(pWinAODVLock);
	return NextWaitTime;
}

/*
* Returns the lifetime of the next RREQID that will expire 
* from the expiriy list. If entries are not present, it returns a 
* fixed values. 
*/
int GetNextExpiringRREQID(IN_ADDR *pOrigIPAddr, ULONG *pOrigRREQID)
{
	BOOL Success;
	RREQIDExpiryEntry NextRREQIDExpiryEntry;
	int NextWaitTime;

	Success = RREQIDExpiryListRemoveFromTail(pRREQIDExpiryListHead, &NextRREQIDExpiryEntry);
	if(Success)
	{
		pOrigIPAddr->S_un.S_addr = NextRREQIDExpiryEntry.OrigIPAddr.S_un.S_addr;
		*pOrigRREQID = NextRREQIDExpiryEntry.RREQIDNum;
		NextWaitTime = NextRREQIDExpiryEntry.ExpiryTime - GetCurMiliSecTime();
		if(NextWaitTime <= 0)
		{
			NextWaitTime = 500;
		}
	}
	else
	{
		pOrigIPAddr->S_un.S_addr = 0;
		*pOrigRREQID = 0;
		NextWaitTime = 500;
	}

	return NextWaitTime;
}



// ------------- Managed by the thread that sends hellos --------- //

/*
* Multicasts hello messages for other nodes in the vicinity to detect
* link breakages.
*/
int SendNextHello(PHELLOMessage pLastHELLOMsg)
{
	int NextWaitTime;
	char PrintBuf[2048];

	//EnterCriticalSection(pWinAODVLock);

	//printf("Got control by SendNextHello() \n");

	// send hello only if any active unexpired routes exist
	if(UnexpiredRouteSize() > 0) 
	{
		pLastHELLOMsg->DestSeqNum = CurrLocalSeqNum;

		SendHELLO(pLastHELLOMsg);

		// log
		sprintf(PrintBuf, "UoBWinAODV : HELLO sent to %s ", 
				inet_ntoa((struct in_addr) pLastHELLOMsg->ToIPAddr));
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_INFO, PrintBuf);
		LogMessage(LOGGING_SEVERAITY_MORE_INFO, 
				BuildHELLOAsString(pLastHELLOMsg, PrintBuf)); 
		LeaveCriticalSection(pLoggingLock);
	}

	goto exit_SendNextHello;

exit_SendNextHello:
	NextWaitTime = ParaHelloInterval;
	//LeaveCriticalSection(pWinAODVLock);
	return NextWaitTime;
}


// ------------------ Process received AODV messages ----------------//

/*
* This method handles a RREQ messge received by the protocol handle. In
* summary, either it will send a RREP or propogate the RREQ. Following
* text describes the
*
*	create or update a route to the prev hop increase lifetime
*		by ACTIVE_ROUTE_TIMEOUT (without a valid seq num, i.e. validDestSeqNumFlag = invalid
* 	check RREQ prevously recvd (check from RREQ ID + orig ip list), if so drop
*
*	in RREQ increment hop count
*	serach a route to the originator of RREQ, then add or update route
*			(use orig seq num in RREQ to update, see FORMULA
*			 set validDestSeqNumFlag = valid)
*
*	route lifetime to originator should be updated using FORMULA
*
*	check if i am the destnation, then RREP generated
*	check if route to dest available AND active AND D flag is not set AND my dest seq num is valid
*				AND my dest seq num >= to dest seq num in RREQ, then RREP generated
*	if RREP generated
*		if destination
*			seq num FORMULA
*			hop count = 0, lifetime = MY_ROUTE_TIME, prefix = 0, R flag 0,
*			A flag from parametrs, rest from RREQ
*			unicast send, to sender of RREQ (prev hop)
*
*		if not destination (intermediate)
*			seq num = what is route entry
*			hop count = what is route entry
*			lifetime = what is route entry (route time - curr time)
*			rest from RREQ
*			unicast send, to sender of RREQ (prev hop)
*			if G flag set in RREP (send RREP to destination)
*				hop count = from route to originator
*				dest ip adddress = originate of RREQ
*				dest seq num = originator seq num of RREQ
*				originator ip address = dest ip address
*				lifetime = (route time - curr time) to originator
*				unicast send to next hop to destination (from route)
*
*	if no RREP generated then
*		check the TTL, should be > 1 else drop packet
*		reduce TTL by 1
*		place highest of dest seq considering RREQ and route to dest in my route list
*		put RREQ ID + originator ip in list for RREQ minder with PATH_DISCOVERY_TIME
*		propogate RREQ
*
*/
void ProcessRREQMsg(PRREQMessage pRREQMsg)
{
	RouteEntry PrevHopRouteEntry;
	RouteEntry OrigRouteEntry;
	RouteEntry DestRouteEntry;
	BOOL Success, GenerateRREP;
	ULONG MinimalLifetime;
	RREPMessage NewRREPMsg;
	RREQMessage NewRREQMsg;
	char PrintBuf[2048];
	RREQIDEntry CheckRREQIDEntry;


	//EnterCriticalSection(pWinAODVLock);

	// log
	sprintf(PrintBuf, "UoBWinAODV : RREQ Received from %s ", 
		inet_ntoa((struct in_addr) pRREQMsg->FromIPAddr));
	EnterCriticalSection(pLoggingLock);
	LogMessage(LOGGING_SEVERAITY_INFO, PrintBuf);
	LogMessage(LOGGING_SEVERAITY_MORE_INFO, 
		BuildRREQAsString(pRREQMsg, PrintBuf)); 
	LeaveCriticalSection(pLoggingLock);

	// create or update a route to the prev hop increase lifetime
	//		by ACTIVE_ROUTE_TIMEOUT (without a valid seq num, 
	//		i.e. validDestSeqNumFlag = invalid)
	Success = RouteListViewByValue(pRouteListHead, &pRREQMsg->FromIPAddr, 
									&PrevHopRouteEntry);

	// if no route entry available
	if(!Success) 
	{
		// create entry
		memset(&PrevHopRouteEntry, 0, sizeof(PrevHopRouteEntry));
		PrevHopRouteEntry.DestIPAddr.S_un.S_addr = pRREQMsg->FromIPAddr.S_un.S_addr;
		PrevHopRouteEntry.DestIPAddrMask.S_un.S_addr = HOST_MASK;
		PrevHopRouteEntry.DestSeqNum = 0;
		PrevHopRouteEntry.ValidDestSeqNumFlag = DEST_SEQ_FLAG_INVALID;
		PrevHopRouteEntry.RouteStatusFlag = ROUTE_STATUS_FLAG_VALID;
		strcpy(PrevHopRouteEntry.pIfaceName, pRREQMsg->pIfaceName);
		PrevHopRouteEntry.HopCount = 1;
		PrevHopRouteEntry.NextHopIPAddr.S_un.S_addr 
							= pRREQMsg->FromIPAddr.S_un.S_addr;
		PrecursorListInit(&PrevHopRouteEntry.PrecursorListHead);
		PrevHopRouteEntry.ExpiryTime = GetCurMiliSecTime() + ParaActiveRouteTimeout;
		PrevHopRouteEntry.KernelRouteSet = FALSE;
		PrevHopRouteEntry.RouteType = ROUTE_TYPE_AODV;
		PrevHopRouteEntry.ActiveThreadType = ACTIVE_THREAD_ROUTE_LIFETIME_MINDER;
	} 
	// if available and not expired
	else if(PrevHopRouteEntry.RouteStatusFlag == ROUTE_STATUS_FLAG_VALID) 
	{
		// route is active, only extend lifetime
		PrevHopRouteEntry.ExpiryTime = GetCurMiliSecTime() + ParaActiveRouteTimeout;

	} 
	// if available but expired
	else 
	{
		// set kernel route, start the minder and extend lifetime
		PrevHopRouteEntry.HopCount = 1;
		PrevHopRouteEntry.NextHopIPAddr.S_un.S_addr 
							= pRREQMsg->FromIPAddr.S_un.S_addr;
		PrevHopRouteEntry.RouteStatusFlag = ROUTE_STATUS_FLAG_VALID;
		PrevHopRouteEntry.ExpiryTime = GetCurMiliSecTime() + ParaActiveRouteTimeout;
	}

	UpdateRoute(&PrevHopRouteEntry, UPDATE_TYPE_NOMAL);

	Success = RREQIDListViewByValue(pRREQIDListHead, &pRREQMsg->OrigIPAddr,
									&pRREQMsg->OrigSeqNum,
									&CheckRREQIDEntry);

	// check RREQ prevously recvd (check from RREQ ID + orig IP in list), if so drop
	if(Success)
	{
		// log

		sprintf(PrintBuf, "UoBWinAODV : RREQ with %s:%ld disregarded as previously processed", 
			inet_ntoa((struct in_addr) pRREQMsg->OrigIPAddr),
			pRREQMsg->OrigSeqNum);
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_INFO, PrintBuf);
		LeaveCriticalSection(pLoggingLock);

		goto exit_ProcessRREQMsg;
	}

	// in RREQ, increment hop count
	pRREQMsg->HopCount++;

	// serach a route to the originator of RREQ, then add or update route
	//			(use orig seq num in RREQ to update, see FORMULA
	//			 set validDestSeqNumFlag = valid)
	Success = RouteListViewByValue(pRouteListHead, &pRREQMsg->OrigIPAddr, 
									&OrigRouteEntry);

	// if route not available
	if(!Success) 
	{
		memset(&OrigRouteEntry, 0, sizeof(OrigRouteEntry));
		OrigRouteEntry.DestIPAddr.S_un.S_addr = pRREQMsg->OrigIPAddr.S_un.S_addr;
		OrigRouteEntry.DestIPAddrMask.S_un.S_addr = HOST_MASK;
		OrigRouteEntry.DestSeqNum = pRREQMsg->OrigSeqNum;
		OrigRouteEntry.ValidDestSeqNumFlag = DEST_SEQ_FLAG_VALID;
		OrigRouteEntry.RouteStatusFlag = ROUTE_STATUS_FLAG_VALID;
		strcpy(OrigRouteEntry.pIfaceName, pRREQMsg->pIfaceName);
		OrigRouteEntry.HopCount = pRREQMsg->HopCount;
		OrigRouteEntry.NextHopIPAddr.S_un.S_addr = pRREQMsg->FromIPAddr.S_un.S_addr;
		PrecursorListInit(&OrigRouteEntry.PrecursorListHead);
		OrigRouteEntry.ExpiryTime = GetCurMiliSecTime();
		OrigRouteEntry.KernelRouteSet = FALSE;
		OrigRouteEntry.RouteType = ROUTE_TYPE_AODV;
		OrigRouteEntry.ActiveThreadType = ACTIVE_THREAD_ROUTE_LIFETIME_MINDER;
	}

	// if available and not expired
	else if(OrigRouteEntry.RouteStatusFlag == ROUTE_STATUS_FLAG_VALID) 
	{
		 // only lifetime need to be extended, done later
			;
	}

	// if available but expired
	else 
	{
		// create whole route
		OrigRouteEntry.DestSeqNum = pRREQMsg->OrigSeqNum;
		OrigRouteEntry.ValidDestSeqNumFlag = DEST_SEQ_FLAG_VALID;
		OrigRouteEntry.RouteStatusFlag = ROUTE_STATUS_FLAG_VALID;
		strcpy(OrigRouteEntry.pIfaceName, pRREQMsg->pIfaceName);
		OrigRouteEntry.HopCount = pRREQMsg->HopCount;
		OrigRouteEntry.NextHopIPAddr.S_un.S_addr = pRREQMsg->FromIPAddr.S_un.S_addr;
		//PrecursorListTerm(&OrigRouteEntry.PrecursorListHead);
		PrecursorListInit(&OrigRouteEntry.PrecursorListHead);
		OrigRouteEntry.ExpiryTime = GetCurMiliSecTime();
	}


	// update lifetime
	// route lifetime to originator should be updated using FORMULA
	//	maximum of (ExistingLifetime, MinimalLifetime)
	//   MinimalLifetime = (current time + 2*NET_TRAVERSAL_TIME -
	//                                    2*HopCount*NODE_TRAVERSAL_TIME).
	MinimalLifetime = (2 * ParaNetTraversalTime)
						- (2 * OrigRouteEntry.HopCount * ParaNodeTraversalTime)
						+ GetCurMiliSecTime();
	if(MinimalLifetime > OrigRouteEntry.ExpiryTime)
	{
		OrigRouteEntry.ExpiryTime = MinimalLifetime;
	}

	UpdateRoute(&OrigRouteEntry, UPDATE_TYPE_NOMAL);

	// check if I am the destnation, then RREP generated
	if(pRREQMsg->DestIPAddr.S_un.S_addr == ParaIPAddress.S_un.S_addr)
	{
		GenerateRREP = TRUE;
		memset(&DestRouteEntry, 0, sizeof(DestRouteEntry));
	}
	// check if route to dest available AND active AND D flag is not set AND my dest seq num is valid
	// 			AND my dest seq num >= to dest seq num in RREQ, then RREP generated
	else 
	{
		Success = RouteListViewByValue(pRouteListHead, &pRREQMsg->DestIPAddr, 
										&DestRouteEntry);
		if(Success
			&& (DestRouteEntry.RouteStatusFlag == ROUTE_STATUS_FLAG_VALID)
			&& !(pRREQMsg->DestOnlyFlag)
			&& (DestRouteEntry.ValidDestSeqNumFlag == DEST_SEQ_FLAG_VALID)
			&& (DestRouteEntry.DestSeqNum >= pRREQMsg->DestSeqNum))
		{
			GenerateRREP = TRUE;
		}

		// if none of above, propogate the RREQ
		else 
		{
			GenerateRREP = FALSE;
			memset(&DestRouteEntry, 0, sizeof(DestRouteEntry));
		}
	}

	// if RREP to be generated and I am destination
	if(GenerateRREP && (pRREQMsg->DestIPAddr.S_un.S_addr == ParaIPAddress.S_un.S_addr)) 
	{
		// seq num see FORMULA
		// hop count = 0, lifetime = MY_ROUTE_TIME, prefix = 0, R flag 0,
		// A flag from parametrs, rest from RREQ
		// unicast send, to sender of RREQ (prev hop)

		memset(&NewRREPMsg, 0, sizeof(NewRREPMsg));

		NewRREPMsg.MultiCast = FALSE;
		NewRREPMsg.FromIPAddr.S_un.S_addr = ParaIPAddress.S_un.S_addr;
		NewRREPMsg.ToIPAddr.S_un.S_addr = PrevHopRouteEntry.DestIPAddr.S_un.S_addr;
		NewRREPMsg.TTLValue = 255;
		NewRREPMsg.RepairFlag = FALSE;
		NewRREPMsg.AckFlag = ParaRREPAckRequired;
		NewRREPMsg.PrefixSize = 0;
		NewRREPMsg.HopCount = 0;
		NewRREPMsg.DestIPAddr.S_un.S_addr = ParaIPAddress.S_un.S_addr;

		if(pRREQMsg->DestSeqNum > CurrLocalSeqNum)
		{
			CurrLocalSeqNum = pRREQMsg->DestSeqNum;
		}
		else if(pRREQMsg->DestSeqNum == CurrLocalSeqNum)
		{
			IncrementSeqNum(&CurrLocalSeqNum);
		} 
		else 
		{
			// use existing value
			;
		}
		NewRREPMsg.DestSeqNum = CurrLocalSeqNum; 
		NewRREPMsg.OrigIPAddr.S_un.S_addr = OrigRouteEntry.DestIPAddr.S_un.S_addr;
		NewRREPMsg.LifeTime = ParaMyRouteTimeout;
		SendRREP(&NewRREPMsg);

		// log
		sprintf(PrintBuf, "UoBWinAODV : RREP sent to %s ", 
				inet_ntoa((struct in_addr) NewRREPMsg.ToIPAddr));
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_INFO, PrintBuf);
		LogMessage(LOGGING_SEVERAITY_MORE_INFO, 
				BuildRREPAsString(&NewRREPMsg, PrintBuf)); 
		LeaveCriticalSection(pLoggingLock);
	}
	// if I am not destination (i.e. intermediate node)
	else if (GenerateRREP && pRREQMsg->DestIPAddr.S_un.S_addr != ParaIPAddress.S_un.S_addr)
	{
		// seq num = what is route entry
		// hop count = what is route entry
		// lifetime = what is route entry (route time - curr time)
		// rest from RREQ
		// unicast send, to sender of RREQ (prev hop)

		memset(&NewRREPMsg, 0, sizeof(NewRREPMsg));

		NewRREPMsg.MultiCast = FALSE;
		NewRREPMsg.FromIPAddr.S_un.S_addr = ParaIPAddress.S_un.S_addr;
		NewRREPMsg.ToIPAddr.S_un.S_addr = PrevHopRouteEntry.DestIPAddr.S_un.S_addr;
		NewRREPMsg.TTLValue = 255;
		NewRREPMsg.RepairFlag = FALSE;
		NewRREPMsg.AckFlag = ParaRREPAckRequired;
		NewRREPMsg.PrefixSize = 0;
		NewRREPMsg.HopCount = DestRouteEntry.HopCount;
		NewRREPMsg.DestIPAddr.S_un.S_addr = DestRouteEntry.DestIPAddr.S_un.S_addr;
		NewRREPMsg.DestSeqNum = DestRouteEntry.DestSeqNum; 
		NewRREPMsg.OrigIPAddr.S_un.S_addr = pRREQMsg->OrigIPAddr.S_un.S_addr;
		NewRREPMsg.LifeTime = DestRouteEntry.ExpiryTime - GetCurMiliSecTime();
		SendRREP(&NewRREPMsg);

		// log
		sprintf(PrintBuf, "UoBWinAODV : RREP sent to %s ", 
			inet_ntoa((struct in_addr) NewRREPMsg.ToIPAddr));
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_INFO, PrintBuf);
		LogMessage(LOGGING_SEVERAITY_MORE_INFO, 
				BuildRREPAsString(&NewRREPMsg, PrintBuf)); 
		LeaveCriticalSection(pLoggingLock);

		// if G flag set in RREQ (send RREP to destination)
		if(pRREQMsg->GratRREPFlag) 
		{
			// hop count = from route to originator
			// dest ip adddress = originate of RREQ
			// dest seq num = originator seq num of RREQ
			// originator ip address = dest ip address
			// lifetime = (route time - curr time) to originator
			// unicast send to next hop to destination (from route)

			memset(&NewRREPMsg, 0, sizeof(NewRREPMsg));

			NewRREPMsg.MultiCast = FALSE;
			NewRREPMsg.FromIPAddr.S_un.S_addr = ParaIPAddress.S_un.S_addr;
			NewRREPMsg.ToIPAddr.S_un.S_addr = DestRouteEntry.NextHopIPAddr.S_un.S_addr;
			NewRREPMsg.TTLValue = 255;
			NewRREPMsg.RepairFlag = FALSE;
			NewRREPMsg.AckFlag = ParaRREPAckRequired;
			NewRREPMsg.PrefixSize = 0;
			NewRREPMsg.HopCount = OrigRouteEntry.HopCount;
			NewRREPMsg.DestIPAddr.S_un.S_addr = OrigRouteEntry.DestIPAddr.S_un.S_addr;
			NewRREPMsg.DestSeqNum = OrigRouteEntry.DestSeqNum; 
			NewRREPMsg.OrigIPAddr.S_un.S_addr = DestRouteEntry.DestIPAddr.S_un.S_addr;
			NewRREPMsg.LifeTime = OrigRouteEntry.ExpiryTime - GetCurMiliSecTime();
			SendRREP(&NewRREPMsg);
			
			// log
			sprintf(PrintBuf, "UoBWinAODV : RREP sent to %s ", 
				inet_ntoa((struct in_addr) NewRREPMsg.ToIPAddr));
			EnterCriticalSection(pLoggingLock);
			LogMessage(LOGGING_SEVERAITY_INFO, PrintBuf);
			LogMessage(LOGGING_SEVERAITY_MORE_INFO, 
				BuildRREPAsString(&NewRREPMsg, PrintBuf)); 
			LeaveCriticalSection(pLoggingLock);
		}
	}

	// if no RREP generated, then propogate RREQ
	// provided the TTL is > 1 else drop packet
	else if( !GenerateRREP && pRREQMsg->TTLValue > 1)
	{
		// reduce TTL by 1
		// place highest of dest seq considering RREQ and route to dest in my route list
		// put RREQ ID + originator ip in list for RREQ minder with PATH_DISCOVERY_TIME
		// propogate RREQ
		pRREQMsg->TTLValue--;
		if(DestRouteEntry.DestIPAddr.S_un.S_addr != 0)
		{
			if(!pRREQMsg->UnknownSeqNumFlag
				&& pRREQMsg->DestSeqNum > DestRouteEntry.DestSeqNum)
			{
				DestRouteEntry.DestSeqNum = pRREQMsg->DestSeqNum;
				UpdateRoute(&DestRouteEntry, UPDATE_TYPE_NOMAL);
				pRREQMsg->UnknownSeqNumFlag = FALSE;
			} 
			else 
			{
				pRREQMsg->DestSeqNum = DestRouteEntry.DestSeqNum;
			}
		}

		memset(&NewRREQMsg, 0, sizeof(NewRREQMsg));

		NewRREQMsg.ToIPAddr.S_un.S_addr = pRREQMsg->ToIPAddr.S_un.S_addr;
		NewRREQMsg.FromIPAddr.S_un.S_addr = ParaIPAddress.S_un.S_addr;
		NewRREQMsg.TTLValue = pRREQMsg->TTLValue;
		NewRREQMsg.JoinFlag = pRREQMsg->JoinFlag;
		NewRREQMsg.RepairFlag = pRREQMsg->RepairFlag;
		NewRREQMsg.GratRREPFlag = pRREQMsg->GratRREPFlag;
		NewRREQMsg.DestOnlyFlag = pRREQMsg->DestOnlyFlag;
		NewRREQMsg.UnknownSeqNumFlag = pRREQMsg->UnknownSeqNumFlag;
		NewRREQMsg.HopCount = pRREQMsg->HopCount;
		NewRREQMsg.RREQID = pRREQMsg->RREQID;
		NewRREQMsg.DestIPAddr.S_un.S_addr = pRREQMsg->DestIPAddr.S_un.S_addr;
		NewRREQMsg.DestSeqNum = pRREQMsg->DestSeqNum;
		NewRREQMsg.OrigIPAddr.S_un.S_addr = pRREQMsg->OrigIPAddr.S_un.S_addr;
		NewRREQMsg.OrigSeqNum = pRREQMsg->OrigSeqNum;

		UpdateRREQID(&pRREQMsg->OrigIPAddr, &pRREQMsg->RREQID);

		SendRREQ(&NewRREQMsg);

		// log
		sprintf(PrintBuf, "UoBWinAODV : RREQ sent to %s ", 
				inet_ntoa((struct in_addr) NewRREQMsg.ToIPAddr));
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_INFO, PrintBuf);
		LogMessage(LOGGING_SEVERAITY_MORE_INFO, 
				BuildRREQAsString(&NewRREQMsg, PrintBuf)); 
		LeaveCriticalSection(pLoggingLock);
	} 
	else
	{
		// log
		sprintf(PrintBuf, 
			"UoBWinAODV : RREQ Received from %s is discarded as TTL exceeded", 
			inet_ntoa((struct in_addr) pRREQMsg->FromIPAddr));
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_INFO, PrintBuf);
		LeaveCriticalSection(pLoggingLock);
	}

exit_ProcessRREQMsg:
	//LeaveCriticalSection(pWinAODVLock);
	;
}

/*
* This method is responsible for handling RREP messages recived by the
* node. The following is the procedure,
*
*		find route to prev hop (who sent RREP, i.e dest=prev hop)
*		if not, create route without a valid seq num
*		increment hop count in RREP
*
*		find route to dest
*		if route found, compare dest seq num
*			if seq num invalid in route
*			   OR (dest seq in RREP > what is in route (2s comp) AND dest seq valid)
*			   OR (seq == seq AND route is inactive route)
*			   OR (seq num == seq num AND active route AND hop count in RREP is < hop count in route)
*				update route
*					do as (100)
*		if route not found
*			(100) create route - route flag = active, dest seq flag = valid,
*				 next hop = src ip in RREP, hop count = hop count in RREP
*				 expiry time = current time + lifetime in RREP
*				 dest seq num = dest seq num of RREP
*				 dest ip = dest ip in RREP
*				 iface = iface from which RREP recvd
*
*
*		if i am not originator of RREP
*			find route to originator
*			update lifetime of route to max of (existing lifetime, currtime + ACTIVE_ROUTE_TIMEOUT)
*			update precursor list - using from the src ip from whom the RREP was
*											recvd (i.e. next hop)
*
*			find route to dest
*			update precuror list to dest (i.e. next hop to dest) - put ip of next hop to which RREP is
*					 forwarded (not	ncessarily originator)
*			send RREP to next hop to originator
*
*		if i am originator
*			unicast RREP-ACK to dest
*
*/
void ProcessRREPMsg(PRREPMessage pRREPMsg)
{
	char PrintBuf[2048];
	RouteEntry PrevHopRouteEntry;
	RouteEntry OrigRouteEntry;
	RouteEntry DestRouteEntry;
	BOOL Success;
	RREPMessage NewRREPMsg;
	MILISECTIME ActiveRouteExpiryTime;
	RouteDiscoveryEntry CheckRouteDiscEntry;
	IPPacket ExistingIPPkt;

	//EnterCriticalSection(pWinAODVLock);

	// log
	sprintf(PrintBuf, "UoBWinAODV : RREP Received from %s ", 
		inet_ntoa((struct in_addr) pRREPMsg->FromIPAddr));
	EnterCriticalSection(pLoggingLock);
	LogMessage(LOGGING_SEVERAITY_INFO, PrintBuf);
	LogMessage(LOGGING_SEVERAITY_MORE_INFO, 
					BuildRREPAsString(pRREPMsg, PrintBuf)); 
	LeaveCriticalSection(pLoggingLock);

	// find route to prev hop (who sent RREP, i.e dest=prev hop)
	// if not, create route without a valid seq num
	Success = RouteListViewByValue(pRouteListHead, &pRREPMsg->FromIPAddr, 
									&PrevHopRouteEntry);

	// if no route entry available
	if(!Success) 
	{
			// create entry
		memset(&PrevHopRouteEntry, 0, sizeof(PrevHopRouteEntry));
		PrevHopRouteEntry.DestIPAddr.S_un.S_addr
							= pRREPMsg->FromIPAddr.S_un.S_addr;
		PrevHopRouteEntry.DestIPAddrMask.S_un.S_addr = HOST_MASK;
		PrevHopRouteEntry.DestSeqNum = 0;

		PrevHopRouteEntry.ValidDestSeqNumFlag = DEST_SEQ_FLAG_INVALID;
		PrevHopRouteEntry.RouteStatusFlag = ROUTE_STATUS_FLAG_VALID;
		strcpy(PrevHopRouteEntry.pIfaceName, pRREPMsg->pIfaceName);
		PrevHopRouteEntry.HopCount = 1;
		PrevHopRouteEntry.NextHopIPAddr.S_un.S_addr 
						= pRREPMsg->FromIPAddr.S_un.S_addr;
		PrecursorListInit(&PrevHopRouteEntry.PrecursorListHead);
		PrevHopRouteEntry.ExpiryTime = GetCurMiliSecTime() + ParaActiveRouteTimeout;
		PrevHopRouteEntry.KernelRouteSet = FALSE;
		PrevHopRouteEntry.RouteType = ROUTE_TYPE_AODV;
		PrevHopRouteEntry.ActiveThreadType = ACTIVE_THREAD_ROUTE_LIFETIME_MINDER;
	}
	// if available and not expired
	else if(PrevHopRouteEntry.RouteStatusFlag == ROUTE_STATUS_FLAG_VALID) 
	{

		// route is active, only extend lifetime
		PrevHopRouteEntry.ExpiryTime = GetCurMiliSecTime() + ParaActiveRouteTimeout;

	}
	// if available but expired
	else 
	{

		// set kernel route, start the minder and extend lifetime
		PrevHopRouteEntry.HopCount = 1;
		PrevHopRouteEntry.NextHopIPAddr.S_un.S_addr 
						= pRREPMsg->FromIPAddr.S_un.S_addr;

		PrevHopRouteEntry.ExpiryTime = GetCurMiliSecTime() + ParaActiveRouteTimeout;
		PrevHopRouteEntry.RouteStatusFlag = ROUTE_STATUS_FLAG_VALID;
	}

	UpdateRoute(&PrevHopRouteEntry, UPDATE_TYPE_NOMAL);

	// increment hop count in RREP
	pRREPMsg->HopCount++;

	// find route to dest
	Success = RouteListViewByValue(pRouteListHead, &pRREPMsg->DestIPAddr, 
										&DestRouteEntry);


	// if route found ( compare dest seq num)
		// AND (seq num invalid in route
		//    OR (dest seq in RREP > what is in route (2s comp) AND dest seq valid)
		//    OR (seq == seq AND route is inactive route)
		//    OR (seq num == seq num AND active route AND hop count in RREP is < hop count in route))
			// update route
	if(Success
		&& (DestRouteEntry.ValidDestSeqNumFlag == DEST_SEQ_FLAG_INVALID
			|| (pRREPMsg->DestSeqNum > DestRouteEntry.DestSeqNum
				&& DestRouteEntry.ValidDestSeqNumFlag == DEST_SEQ_FLAG_VALID)
			|| (pRREPMsg->DestSeqNum == DestRouteEntry.DestSeqNum
				&& DestRouteEntry.RouteStatusFlag == ROUTE_STATUS_FLAG_INVALID)
			|| (pRREPMsg->DestSeqNum == DestRouteEntry.DestSeqNum 
				&& DestRouteEntry.RouteStatusFlag == ROUTE_STATUS_FLAG_VALID
				&& pRREPMsg->HopCount < DestRouteEntry.HopCount))) 
	{

		DestRouteEntry.DestIPAddr.S_un.S_addr = pRREPMsg->DestIPAddr.S_un.S_addr;
		DestRouteEntry.DestSeqNum = pRREPMsg->DestSeqNum;
		DestRouteEntry.ValidDestSeqNumFlag = DEST_SEQ_FLAG_VALID;
		DestRouteEntry.RouteStatusFlag = ROUTE_STATUS_FLAG_VALID;
		strcpy(DestRouteEntry.pIfaceName, pRREPMsg->pIfaceName);
		DestRouteEntry.HopCount = pRREPMsg->HopCount;
		DestRouteEntry.NextHopIPAddr.S_un.S_addr = pRREPMsg->FromIPAddr.S_un.S_addr;
		DestRouteEntry.ExpiryTime = GetCurMiliSecTime() + pRREPMsg->LifeTime;

			// log
	}
		// if route not found
	else if(!Success) 
	{
		// (100) create route - route flag = active, dest seq flag = valid,
		// 	 next hop = src ip in RREP, hop count = hop count in RREP
		// 	 expiry time = current time + lifetime in RREP
		// 	 dest seq num = dest seq num of RREP
		// 	 dest ip = dest ip in RREP
		// 	 iface = iface from which RREP recvd

		memset(&DestRouteEntry, 0, sizeof(DestRouteEntry));
		DestRouteEntry.DestIPAddr.S_un.S_addr = pRREPMsg->DestIPAddr.S_un.S_addr;
		DestRouteEntry.DestIPAddrMask.S_un.S_addr = HOST_MASK;
		DestRouteEntry.DestSeqNum = pRREPMsg->DestSeqNum;
		DestRouteEntry.ValidDestSeqNumFlag = DEST_SEQ_FLAG_VALID;
		DestRouteEntry.RouteStatusFlag = ROUTE_STATUS_FLAG_VALID;
		strcpy(DestRouteEntry.pIfaceName, pRREPMsg->pIfaceName);
		DestRouteEntry.HopCount = pRREPMsg->HopCount;
		DestRouteEntry.NextHopIPAddr.S_un.S_addr = pRREPMsg->FromIPAddr.S_un.S_addr;
		PrecursorListInit(&DestRouteEntry.PrecursorListHead);
		DestRouteEntry.ExpiryTime = GetCurMiliSecTime() + pRREPMsg->LifeTime;
		DestRouteEntry.KernelRouteSet = FALSE;
		DestRouteEntry.RouteType = ROUTE_TYPE_AODV;
		DestRouteEntry.ActiveThreadType = ACTIVE_THREAD_ROUTE_LIFETIME_MINDER;
	}
	else 
	{
		DestRouteEntry.ExpiryTime = GetCurMiliSecTime() + pRREPMsg->LifeTime;
	}

	UpdateRoute(&DestRouteEntry, UPDATE_TYPE_NOMAL);

	// if i am not originator of RREP
	if(ParaIPAddress.S_un.S_addr != pRREPMsg->OrigIPAddr.S_un.S_addr) 
	{
		//find route to originator
		Success = RouteListViewByValue(pRouteListHead, &pRREPMsg->OrigIPAddr, 
										&OrigRouteEntry);
		if(!Success) 
		{
			// somethin wrong
			//log
			sprintf(PrintBuf, "UoBWinAODV : No originator route entry found for %s. Try extending lifetime", 
				inet_ntoa((struct in_addr) pRREPMsg->OrigIPAddr));
			EnterCriticalSection(pLoggingLock);
			LogMessage(LOGGING_SEVERAITY_ERROR, PrintBuf);
			LeaveCriticalSection(pLoggingLock);

			goto exit_ProcessRREPMsg;
		} 
		else 
		{
			// update lifetime of route to max of (existing lifetime, currtime + ACTIVE_ROUTE_TIMEOUT)
			// update precursor list - using from the src ip from whom the RREP was
			// 							recvd (i.e. next hop)
			ActiveRouteExpiryTime = ParaActiveRouteTimeout + GetCurMiliSecTime();

			if(ActiveRouteExpiryTime > OrigRouteEntry.ExpiryTime) 
			{
				OrigRouteEntry.ExpiryTime = ActiveRouteExpiryTime;
			}

			UpdatePrecursorList(&OrigRouteEntry.PrecursorListHead, &pRREPMsg->FromIPAddr);

			UpdateRoute(&OrigRouteEntry, UPDATE_TYPE_NOMAL);

			// find route to dest
			// update precuror list to dest (i.e. next hop to dest) - put ip of
			//  next hop to which RREP is forwarded (not necessarily originator)
			UpdatePrecursorList(&DestRouteEntry.PrecursorListHead, 
									&OrigRouteEntry.NextHopIPAddr);

			UpdateRoute(&DestRouteEntry, UPDATE_TYPE_NOMAL);


			// send RREP to next hop to originator

			NewRREPMsg.MultiCast = FALSE;
			NewRREPMsg.FromIPAddr.S_un.S_addr = ParaIPAddress.S_un.S_addr;
			NewRREPMsg.ToIPAddr.S_un.S_addr = OrigRouteEntry.NextHopIPAddr.S_un.S_addr;
			NewRREPMsg.TTLValue = 255;
			NewRREPMsg.RepairFlag = pRREPMsg->RepairFlag;
			NewRREPMsg.AckFlag = pRREPMsg->AckFlag;
			NewRREPMsg.PrefixSize = pRREPMsg->PrefixSize;
			NewRREPMsg.HopCount = pRREPMsg->HopCount;
			NewRREPMsg.DestIPAddr.S_un.S_addr = pRREPMsg->DestIPAddr.S_un.S_addr;
			NewRREPMsg.DestSeqNum = pRREPMsg->DestSeqNum; 
			NewRREPMsg.OrigIPAddr.S_un.S_addr = pRREPMsg->OrigIPAddr.S_un.S_addr;
			NewRREPMsg.LifeTime = pRREPMsg->LifeTime;
			SendRREP(&NewRREPMsg);

			// log
			sprintf(PrintBuf, "UoBWinAODV : RREP sent to %s ", 
				inet_ntoa((struct in_addr) NewRREPMsg.ToIPAddr));
			EnterCriticalSection(pLoggingLock);
			LogMessage(LOGGING_SEVERAITY_INFO, PrintBuf);
			LogMessage(LOGGING_SEVERAITY_MORE_INFO, 
				BuildRREPAsString(&NewRREPMsg, PrintBuf)); 
			LeaveCriticalSection(pLoggingLock);

		}
	}

	// if i am originator
	else 
	{

		//	unicast RREP-ACK to dest
	}

	// due to this RREP, if a route was made for a route being
	// discovered, release the packets and delete the route discovery
	// entry
	Success = RouteDiscoveryListViewByValue(pRouteDiscoveryListHead,
											&pRREPMsg->DestIPAddr,
											&CheckRouteDiscEntry);
	if(Success)
	{
		Success = IPPacketListRemoveFromTail(
								&CheckRouteDiscEntry.IPPacketListHead, 
								&ExistingIPPkt);
		while(Success) 
		{
			SendIPPacket(&ExistingIPPkt);
			Success = IPPacketListRemoveFromTail(
								&CheckRouteDiscEntry.IPPacketListHead, 
								&ExistingIPPkt);
		}
		IPPacketListTerm(&CheckRouteDiscEntry.IPPacketListHead);
		
		RouteDiscoveryListRemoveByValue(pRouteDiscoveryListHead,
											&pRREPMsg->DestIPAddr,
											&CheckRouteDiscEntry);

		// log
		sprintf(PrintBuf, 
			"UoBWinAODV : Route discovery terminated as route made to %s ", 
			inet_ntoa((struct in_addr) pRREPMsg->DestIPAddr));
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_INFO, PrintBuf);
		LeaveCriticalSection(pLoggingLock);
	}

exit_ProcessRREPMsg:
	//LeaveCriticalSection(pWinAODVLock);
	;
}

/*
	// RERR
		// compile list of routes to (unrechable dest in RERR
		//	AND that have the sender of RERR as next hop)
		//
		// send RERR to all precursors of the above list
		//			{ copy dest seq from RERR
		//			  set route status = INVALID
		//			  set lifetime to DELETE_PERIOD
		//			  start route deleters for all }
		//
*/
void ProcessRERRMsg(PRERRMessage pRERRMsg)
{

	//EnterCriticalSection(pWinAODVLock);

	// if RERR unicast, send a RERR to each precursor in a
	// invalidating route
	if(ParaRERRSendingMode == RERR_SEND_MODE_UNICAST)
	{
		ProcessRERRMsgUnicast(pRERRMsg);
	}

	// if RERR multicast, regenerate one RERR with all the invalidating
	// destinations
	else 
	{
		// multicast not implemented 
		ProcessRERRMsgUnicast(pRERRMsg);
	}

	//LeaveCriticalSection(pWinAODVLock);
}

/*
* Process unicast RERR message
*/
void ProcessRERRMsgUnicast(PRERRMessage pRERRMsg)
{
	RouteEntry UnreachRtEntry;
	BOOL Success;
	RERRMessage NewRERRMsg;
	ULONG i, j;
	PrecursorEntry UnreachPrecEntry;

	for(i = 0; i < pRERRMsg->DestCount; i++) {
		Success = RouteListViewByValue(pRouteListHead, 
								&pRERRMsg->Unreachable[i].DestIPAddr, 
								&UnreachRtEntry);

		// regenerate RERR only if the nexthop of this route is the
		// sender of the RERR but dont remove the route to the
		// sender of the RERR
		if(Success 
			&& (UnreachRtEntry.NextHopIPAddr.S_un.S_addr 
						== pRERRMsg->FromIPAddr.S_un.S_addr)
			&& (UnreachRtEntry.DestIPAddr.S_un.S_addr 
						!= pRERRMsg->FromIPAddr.S_un.S_addr))
		{						
			UnreachRtEntry.DestSeqNum = pRERRMsg->Unreachable[i].DestSeqNum;

			// regenerate RERR to each precursor
			for(j = 0; j < UnreachRtEntry.PrecursorListHead.PrecursorCount;	j++) 
			{
				Success = PrecursorListViewByIndex(
								&UnreachRtEntry.PrecursorListHead, 
								j, 
								&UnreachPrecEntry);

				memset(&NewRERRMsg, 0, sizeof(NewRERRMsg));
				NewRERRMsg.ToIPAddr.S_un.S_addr 
					= UnreachPrecEntry.DestIPAddr.S_un.S_addr;
				NewRERRMsg.FromIPAddr.S_un.S_addr 
					= ParaIPAddress.S_un.S_addr;
				NewRERRMsg.TTLValue = 1;
				NewRERRMsg.MultiCast = FALSE;
				strcpy(NewRERRMsg.pIfaceName, ParaIfaceName);

				NewRERRMsg.NoDelFlag = FALSE;
				NewRERRMsg.DestCount = 1;

				NewRERRMsg.Unreachable[0].DestIPAddr.S_un.S_addr 
						= UnreachRtEntry.DestIPAddr.S_un.S_addr;
				NewRERRMsg.Unreachable[0].DestSeqNum 
					= UnreachRtEntry.DestSeqNum;

				SendRERR(&NewRERRMsg);
			}

			// invalidate route & start route delete
			UnreachRtEntry.RouteStatusFlag = ROUTE_STATUS_FLAG_INVALID;
			UnreachRtEntry.ExpiryTime = GetCurMiliSecTime() + ParaDeletePeriod;
			UnreachRtEntry.ActiveThreadType = ACTIVE_THREAD_DELETE_PERIOD_MINDER;

			UpdateRoute(&UnreachRtEntry, UPDATE_TYPE_DELETE);
		}
	}
}

/*
* Process a received RREP-ACK message
*/
void ProcessRREPACKMsg(PRREPACKMessage pRREPACKMsg)
{
	//EnterCriticalSection(pWinAODVLock);

	// RREP-ACK receipt not implemented

	//LeaveCriticalSection(pWinAODVLock);
}

/*
* Processs a received Hello message from a neighbour.
*/
void ProcessHELLOMsg(PHELLOMessage pHELLOMsg)
{
	char PrintBuf[2048];
	RouteEntry PrevHopRouteEntry;
	BOOL Success;
	HelloExpiryEntry NextHelloExpiryEntry;

	//EnterCriticalSection(pWinAODVLock);

	// log
	sprintf(PrintBuf, "UoBWinAODV : HELLO Received from %s ", 
		inet_ntoa((struct in_addr) pHELLOMsg->FromIPAddr));
	EnterCriticalSection(pLoggingLock);
	LogMessage(LOGGING_SEVERAITY_INFO, PrintBuf);
	LogMessage(LOGGING_SEVERAITY_MORE_INFO, 
					BuildHELLOAsString(pHELLOMsg, PrintBuf)); 
	LeaveCriticalSection(pLoggingLock);

	// if no active routes exist, don't react to HELLOs
	if(UnexpiredRouteSize() <= 0) 
	{
		goto exit_ProcessHELLOMsg;
	}

	// find route to prev hop (who sent RREP, i.e dest=prev hop)
	// if not, create route without a valid seq num
	Success = RouteListViewByValue(pRouteListHead, &pHELLOMsg->FromIPAddr, 
									&PrevHopRouteEntry);

	// if no route entry available, means no active route
	// for this dest, so don't do anything
	if(!Success) 
	{
		goto exit_ProcessHELLOMsg;
	}

	if(PrevHopRouteEntry.RouteStatusFlag != ROUTE_STATUS_FLAG_VALID)
	{
		goto exit_ProcessHELLOMsg;
	}

	PrevHopRouteEntry.NextHelloReceiveTime = GetCurMiliSecTime() 
												+ pHELLOMsg->LifeTime;
	PrevHopRouteEntry.DestSeqNum = pHELLOMsg->DestSeqNum;
	PrevHopRouteEntry.ValidDestSeqNumFlag = DEST_SEQ_FLAG_VALID;

	UpdateRoute(&PrevHopRouteEntry, UPDATE_TYPE_NOMAL);

	NextHelloExpiryEntry.DestIPAddr.S_un.S_addr = pHELLOMsg->FromIPAddr.S_un.S_addr;
	NextHelloExpiryEntry.ExpiryTime = PrevHopRouteEntry.NextHelloReceiveTime;

	HelloExpiryListInsertAtOrder(pHelloExpiryListHead, &NextHelloExpiryEntry);

exit_ProcessHELLOMsg:
	//LeaveCriticalSection(pWinAODVLock);
	;
}

//------------------- Processing of IP packets ----------------------------//
/*
* Processes a single IP packet. Decides the path to take from 
* 3 possibilities.
*	- Packet using an existing route
*	- Packet from a destination which is being discovered
*	- Packet for which no route exist, so discover route
*/
void ProcessNormalPacket(PIPPacket pIPPkt)
{
	RouteEntry DestRouteEntry;
	BOOL Success;
	RouteDiscoveryEntry CheckRouteDiscEntry;

	//EnterCriticalSection(pWinAODVLock);

	// if I gerenrated the packet
	if(pIPPkt->FromIPAddr.S_un.S_addr == ParaIPAddress.S_un.S_addr)
	{
		Success = RouteListViewByValue(pRouteListHead, &pIPPkt->ToIPAddr, 
										&DestRouteEntry);
		if(Success && (DestRouteEntry.RouteType == ROUTE_TYPE_STATIC))
		{
			// this packet is using a static route (non-AODV), so disregard
			;
		}
		else if(Success && (DestRouteEntry.RouteStatusFlag == ROUTE_STATUS_FLAG_VALID)
			&& (pIPPkt->PacketTypeFlag == PACKET_TYPE_DISALLOWED))
		{
			// somehow a pkt is disallowed even when there is a valid route
			SendIPPacket(pIPPkt);
		}
		else if(Success && (DestRouteEntry.RouteStatusFlag == ROUTE_STATUS_FLAG_VALID)
				&& (pIPPkt->PacketTypeFlag == PACKET_TYPE_ALLOWED))
		{
			ProcessExistingRouteUse(pIPPkt);
		} 
		else
		{
			Success = RouteDiscoveryListViewByValue(pRouteDiscoveryListHead,
												&pIPPkt->ToIPAddr,
												&CheckRouteDiscEntry);
			if(Success)
			{
				// if already a route is being discovered for the
				// given destination, then add the packet to the list;
				ProcessInprogressRouteDiscovery(pIPPkt, &CheckRouteDiscEntry);
			}
			else
			{
				ProcessRouteDiscovery(pIPPkt);
			}
		}
	}

	// Can be 2 reasons
	// - I am not the packet generator or receiver (then I am a router)
	// - Packet is destined to me
	// simply update the route lifetime
	else
	{
		ProcessExistingRouteUse(pIPPkt);
	}

	//LeaveCriticalSection(pWinAODVLock);
}

/*
* Insert the packet into a packet queue related to the 
* route discovery entry.
*/
void ProcessInprogressRouteDiscovery(PIPPacket pIPPkt, 
									 PRouteDiscoveryEntry pRouteDiscEntry)
{
	if(ParaPacketBuffering) 
	{
		IPPacketListInsertAtHead(&pRouteDiscEntry->IPPacketListHead, pIPPkt);
	}
	RouteDiscoveryListReplaceByValue(pRouteDiscoveryListHead,
									&pIPPkt->ToIPAddr,
									pRouteDiscEntry);
}

/*
	// get routes to dest & nextop to dest
	// update lifetime = curr time + ACTIVE_ROUTE_TIMEOUT
	// get routes to originator & next hop to orig
	// update lifetime = curr time + ACTIVE_ROUTE_TIMEOUT
*/
void ProcessExistingRouteUse(PIPPacket pIPPkt)
{
	RouteEntry OrigRouteEntry;
	RouteEntry NextHopRouteEntry;
	RouteEntry DestRouteEntry;
	BOOL Success;

	// if I am not the originator of the packet, then update route to originator
	// of packet and also the next hop, if one exists
	if(pIPPkt->FromIPAddr.S_un.S_addr != ParaIPAddress.S_un.S_addr)
	{
		Success = RouteListViewByValue(pRouteListHead, &pIPPkt->FromIPAddr, 
										&OrigRouteEntry);
		if(Success
			&& OrigRouteEntry.RouteStatusFlag == ROUTE_STATUS_FLAG_VALID) 
		{
			OrigRouteEntry.ExpiryTime = GetCurMiliSecTime() 
											+ ParaActiveRouteTimeout;
			UpdateRoute(&OrigRouteEntry, UPDATE_TYPE_NOMAL);

			if(OrigRouteEntry.HopCount > 1) 
			{
				Success = RouteListViewByValue(pRouteListHead, 
												&OrigRouteEntry.NextHopIPAddr, 
												&NextHopRouteEntry);
				if(Success
					&& NextHopRouteEntry.RouteStatusFlag == ROUTE_STATUS_FLAG_VALID) 
				{

					NextHopRouteEntry.ExpiryTime = GetCurMiliSecTime() 
														+ ParaActiveRouteTimeout;
					UpdateRoute(&NextHopRouteEntry, UPDATE_TYPE_NOMAL);
				}
			}
		}
	}

	// if I am not the destination of the packet, then update route to destination
	// of packet and also the next hop, if one exists
	if(pIPPkt->ToIPAddr.S_un.S_addr != ParaIPAddress.S_un.S_addr)
	{

		Success = RouteListViewByValue(pRouteListHead, &pIPPkt->ToIPAddr, 
										&DestRouteEntry);
		if(Success
			&& DestRouteEntry.RouteStatusFlag == ROUTE_STATUS_FLAG_VALID) 
		{
			DestRouteEntry.ExpiryTime = GetCurMiliSecTime() 
											+ ParaActiveRouteTimeout;
			UpdateRoute(&DestRouteEntry, UPDATE_TYPE_NOMAL);

			if(DestRouteEntry.HopCount > 1) 
			{
				Success = RouteListViewByValue(pRouteListHead, 
												&DestRouteEntry.NextHopIPAddr, 
												&NextHopRouteEntry);
				if(Success
					&& NextHopRouteEntry.RouteStatusFlag == ROUTE_STATUS_FLAG_VALID) 
				{
					NextHopRouteEntry.ExpiryTime = GetCurMiliSecTime() 
											+ ParaActiveRouteTimeout;
					UpdateRoute(&NextHopRouteEntry, UPDATE_TYPE_NOMAL);
				}
			}
		}
	}
}

/*
* If a ARP is issued to the gateway, then it means the dummy MAC
* address given has been removed, so reset Gateway MAC in ARP 
* table.
*/
void ProcessGatewayARP()
{
	//EnterCriticalSection(pWinAODVLock);

	OSOpsSetGatewayMAC();

	//LeaveCriticalSection(pWinAODVLock);
}

//----------------- Route discoverer interface -----------------------------//

/*
* Method to be called when the local(my) machine requires a route
* to some destination. This method is called by the packet listener
* when it receives a packet with the given destination MAC address.
*
* The following text defines the procedure
*	find route to dest in table
*	if found( found means (route status = valid AND expired) OR (route status = invalid) )
*		(200) dest seq = last know seq num
*		increment own seq num
*		orig seq num = own num
*		increment RREQ ID
*		RREQ ID of originator = RREQ ID
*		hop count = 0
*		G flag from parameters
*		D flag from parameters
*		set TTL to (hop count of route + TTL_INCREMENT)
*
*	if not found
*		do as (200)
*		U flag = true
*		set TTL to TTL_START
*
*	put RREQ ID + originator ip in list for RREQ minder with PATH_DISCOVERY_TIME
*	multicast RREQ
*	start route discoverer with NET_TRAVERSAL_TIME and RREQ packet
*
*/
void ProcessRouteDiscovery(PIPPacket pIPPkt)
{
	RouteEntry DestRouteEntry;
	RouteDiscoveryEntry NewRtDiscEntry;
	RREQMessage NewRREQMsg;
	char PrintBuf[2048];
	BOOL Success;

	// log
	sprintf(PrintBuf, "UoBWinAODV : Route discovery started for %s ", 
		inet_ntoa((struct in_addr) pIPPkt->ToIPAddr));
	EnterCriticalSection(pLoggingLock);
	LogMessage(LOGGING_SEVERAITY_INFO, PrintBuf);
	LeaveCriticalSection(pLoggingLock);

	Success = RouteListViewByValue(pRouteListHead, &pIPPkt->ToIPAddr, 
										&DestRouteEntry);

	// generate RREQ

#if 0
	//( found means (route status = invalid) )
	if(Success) 
	{
		// multicast RREQ
		// set TTL to (hop count of route + TTL_INCREMENT)
		// join flag & repair flag is not set as simple dest search
		// G flag from parameters
		// D flag from parameters
		// dest seq num known
		// hop count = 0
		// increment RREQ ID, RREQ ID of originator = RREQ ID
		// destination adr from the IP packet
		// dest seq = last know seq num
		// has to be always my own IP adr
		// increment own seq num, orig seq num = own num

		NewRREQMsg.ToIPAddr.S_un.S_addr = ParaIPAddressMulticast.S_un.S_addr;
		NewRREQMsg.FromIPAddr.S_un.S_addr = ParaIPAddress.S_un.S_addr;
		NewRREQMsg.TTLValue = DestRouteEntry.HopCount + ParaTTLIncrement;
		NewRREQMsg.MultiCast = TRUE;
		strcpy(NewRREQMsg.pIfaceName, ParaIfaceName);

		NewRREQMsg.JoinFlag = FALSE;
		NewRREQMsg.RepairFlag = FALSE;
		NewRREQMsg.GratRREPFlag = ParaGratuitousRREP;
		NewRREQMsg.DestOnlyFlag = ParaOnlyDestination;
		NewRREQMsg.UnknownSeqNumFlag = FALSE;
		NewRREQMsg.HopCount = 0;
		NewRREQMsg.RREQID = IncrementRREQID(&CurrLocalRREQID);
		NewRREQMsg.DestIPAddr.S_un.S_addr = pIPPkt->ToIPAddr.S_un.S_addr;
		NewRREQMsg.DestSeqNum = DestRouteEntry.DestSeqNum;
		NewRREQMsg.OrigIPAddr.S_un.S_addr = pIPPkt->FromIPAddr.S_un.S_addr;
		NewRREQMsg.OrigSeqNum = IncrementRREQID(&CurrLocalSeqNum);
	} 
	else 
	{ //if not found

		// multicast RREQ
		// set TTL to TTL_START
		// join flag & repair flag is not set as simple dest search
		// G flag from parameters
		// D flag from parameters
		// since not in route, dest seq num not known
		// hop count = 0
		// increment RREQ ID, RREQ ID of originator = RREQ ID
		// destination adr from the IP packet
		// dest seq not known
		// has to be always my own IP adr
		// increment own seq num, orig seq num = own num

		NewRREQMsg.ToIPAddr.S_un.S_addr = ParaIPAddressMulticast.S_un.S_addr;
		NewRREQMsg.FromIPAddr.S_un.S_addr = ParaIPAddress.S_un.S_addr;
		NewRREQMsg.TTLValue = ParaTTLStart;
		NewRREQMsg.MultiCast = TRUE;
		strcpy(NewRREQMsg.pIfaceName, ParaIfaceName);

		NewRREQMsg.JoinFlag = FALSE;
		NewRREQMsg.RepairFlag = FALSE;
		NewRREQMsg.GratRREPFlag = ParaGratuitousRREP;
		NewRREQMsg.DestOnlyFlag = ParaOnlyDestination;
		NewRREQMsg.UnknownSeqNumFlag = FALSE;
		NewRREQMsg.HopCount = 0;
		NewRREQMsg.RREQID = IncrementRREQID(&CurrLocalRREQID);
		NewRREQMsg.DestIPAddr.S_un.S_addr = pIPPkt->ToIPAddr.S_un.S_addr;
		NewRREQMsg.DestSeqNum = 0;
		NewRREQMsg.OrigIPAddr.S_un.S_addr = pIPPkt->FromIPAddr.S_un.S_addr;
		NewRREQMsg.OrigSeqNum = IncrementRREQID(&CurrLocalSeqNum);
	}

	// put RREQ ID + originator ip in list for RREQ minder with PATH_DISCOVERY_TIME
	UpdateRREQID(&NewRREQMsg.OrigIPAddr, &NewRREQMsg.OrigSeqNum);

	// multicast the RREQ
	SendRREQ(&NewRREQMsg);

	// log
	sprintf(PrintBuf, "UoBWinAODV : RREQ sent to %s ", 
			inet_ntoa((struct in_addr) NewRREQMsg.ToIPAddr));
	EnterCriticalSection(pLoggingLock);
	LogMessage(LOGGING_SEVERAITY_INFO, PrintBuf);
	LogMessage(LOGGING_SEVERAITY_MORE_INFO, 
				BuildRREQAsString(&NewRREQMsg, PrintBuf)); 
	LeaveCriticalSection(pLoggingLock);

	NewRtDiscEntry.DestIPAddr.S_un.S_addr = NewRREQMsg.DestIPAddr.S_un.S_addr;
	memcpy(&NewRtDiscEntry.LastRREQMsg, &NewRREQMsg, sizeof(NewRtDiscEntry.LastRREQMsg));
	IPPacketListInit(&NewRtDiscEntry.IPPacketListHead);
	NewRtDiscEntry.RREQRetries = 0;

	// if route discovery is ERS
	if(ParaRouteDiscoveryMode == ROUTE_DISCOVERY_MODE_ERS) 
	{
		NewRtDiscEntry.WaitDuration = (2 * ParaNodeTraversalTime 
									* ( NewRREQMsg.TTLValue + ParaTimeoutBuffer));
	}
	// else assumes, route discovery is non-ERS
	else 
	{
		NewRtDiscEntry.WaitDuration = ParaNetTraversalTime;
	}
#endif

	// add the fist packet to the packet buffer (if enabled)
	if(ParaPacketBuffering) 
	{
		//IPPacketListInsertAtHead(&NewRtDiscEntry.IPPacketListHead, pIPPkt); 
		IPPacketListInsertAtHead(pIPPacketListHead, pIPPkt); 
	}

	callback_set.route_req_callback(pIPPkt->ToIPAddr);
	//UpdateRouteDiscovery(&NewRtDiscEntry, &NewRREQMsg.DestIPAddr);
}

/*
* Process an active route discovery further. Called by the 
* RouteDiscoveryTimerThread.
*/
int ProcessRouteDiscoveryFurther(IN_ADDR *pDestIPAddr)
{
	RouteEntry DestRouteEntry;
	BOOL Success;
	RouteDiscoveryEntry ExistRtDiscEntry;
	RREQMessage NewRREQMsg;
	char PrintBuf[2048];
	int NextWaitTime;
	IPPacket ExistingIPPkt;

	//EnterCriticalSection(pWinAODVLock);

	if(pDestIPAddr->S_un.S_addr == 0)
	{
		goto exit_ProcessRouteDiscoveryFurther;
	}

	Success = RouteListViewByValue(pRouteListHead, pDestIPAddr, 
										&DestRouteEntry);
	// if route made, stop discovery
	if(Success && DestRouteEntry.RouteStatusFlag == ROUTE_STATUS_FLAG_VALID) 
	{
		goto exit_ProcessRouteDiscoveryFurther;
	}

	// get entry from the discovery list
	Success = RouteDiscoveryListViewByValue(pRouteDiscoveryListHead,
											pDestIPAddr,
											&ExistRtDiscEntry);
	if(!Success) 
	{
		// log
		sprintf(PrintBuf, "UoBWinAODV : Route discovery terminated due to no RDE entry for %s ", 
				inet_ntoa((struct in_addr) *pDestIPAddr));
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR, PrintBuf);
		LeaveCriticalSection(pLoggingLock);

		goto exit_ProcessRouteDiscoveryFurther;
	}

	// if max retries exceeded, stop discovery
	if((ExistRtDiscEntry.RREQRetries + 1) > ParaRREQRetries)
	{
		// remove all buffered packets
		Success = IPPacketListRemoveFromTail(
								&ExistRtDiscEntry.IPPacketListHead, 
								&ExistingIPPkt);
		while(Success) 
		{
			free(ExistingIPPkt.pIPPacket);
			Success = IPPacketListRemoveFromTail(
								&ExistRtDiscEntry.IPPacketListHead, 
								&ExistingIPPkt);
		}
		IPPacketListTerm(&ExistRtDiscEntry.IPPacketListHead);

		Success = RouteDiscoveryListRemoveByValue(pRouteDiscoveryListHead,
											pDestIPAddr,
											&ExistRtDiscEntry);
		// log
		sprintf(PrintBuf, "UoBWinAODV : Route discovery to %s terminated as max retries (%d) reached", 
			inet_ntoa((struct in_addr) *pDestIPAddr),
			ParaRREQRetries);
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR, PrintBuf);
		LeaveCriticalSection(pLoggingLock);

		goto exit_ProcessRouteDiscoveryFurther;
	}


	// create the new RREQ, using the old RREQ (except for TTL and RREQID)

	NewRREQMsg.ToIPAddr.S_un.S_addr 
				= ExistRtDiscEntry.LastRREQMsg.ToIPAddr.S_un.S_addr;
	NewRREQMsg.FromIPAddr.S_un.S_addr 
				= ExistRtDiscEntry.LastRREQMsg.FromIPAddr.S_un.S_addr;
	NewRREQMsg.TTLValue 
			= ExistRtDiscEntry.LastRREQMsg.TTLValue + ParaTTLIncrement;
	if(NewRREQMsg.TTLValue > ParaTTLThreshold)
	{
		NewRREQMsg.TTLValue = ParaNetDiameter;
	}
	NewRREQMsg.JoinFlag = ExistRtDiscEntry.LastRREQMsg.JoinFlag;
	NewRREQMsg.RepairFlag = ExistRtDiscEntry.LastRREQMsg.RepairFlag;
	NewRREQMsg.GratRREPFlag = ExistRtDiscEntry.LastRREQMsg.GratRREPFlag;
	NewRREQMsg.DestOnlyFlag = ExistRtDiscEntry.LastRREQMsg.DestOnlyFlag;
	NewRREQMsg.UnknownSeqNumFlag = ExistRtDiscEntry.LastRREQMsg.UnknownSeqNumFlag;
	NewRREQMsg.HopCount = ExistRtDiscEntry.LastRREQMsg.HopCount;
	NewRREQMsg.RREQID = IncrementRREQID(&CurrLocalRREQID);
	NewRREQMsg.DestIPAddr.S_un.S_addr 
			= ExistRtDiscEntry.LastRREQMsg.DestIPAddr.S_un.S_addr;
	NewRREQMsg.DestSeqNum = ExistRtDiscEntry.LastRREQMsg.DestSeqNum;
	NewRREQMsg.OrigIPAddr.S_un.S_addr 
			= ExistRtDiscEntry.LastRREQMsg.OrigIPAddr.S_un.S_addr;
	NewRREQMsg.OrigSeqNum = ExistRtDiscEntry.LastRREQMsg.OrigSeqNum;

	// put RREQ ID + originator ip in list for RREQ minder with PATH_DISCOVERY_TIME
	UpdateRREQID(&NewRREQMsg.OrigIPAddr, &NewRREQMsg.OrigSeqNum);

	// multicast the RREQ
	SendRREQ(&NewRREQMsg);

	memcpy(&ExistRtDiscEntry.LastRREQMsg, &NewRREQMsg, 
						sizeof(ExistRtDiscEntry.LastRREQMsg));

	// increment the number of RREQs re-send
	ExistRtDiscEntry.RREQRetries++;

	// if route discovery is ERS
	if(ParaRouteDiscoveryMode == ROUTE_DISCOVERY_MODE_ERS) 
	{
		if(NewRREQMsg.TTLValue >= ParaTTLThreshold)
		{
			ExistRtDiscEntry.WaitDuration = ParaNetTraversalTime;
		}
		else
		{
			ExistRtDiscEntry.WaitDuration = (2 * ParaNodeTraversalTime 
									* ( NewRREQMsg.TTLValue + ParaTimeoutBuffer));
		}
	}
	// else assumes, route discovery is non-ERS
	else 
	{
		ExistRtDiscEntry.WaitDuration = ExistRtDiscEntry.WaitDuration * 2;
	}

	UpdateRouteDiscovery(&ExistRtDiscEntry, &NewRREQMsg.DestIPAddr);

exit_ProcessRouteDiscoveryFurther:
	NextWaitTime = GetNextExpiringRouteDiscovery(pDestIPAddr);
	//LeaveCriticalSection(pWinAODVLock);
	return NextWaitTime;
}

/*
* Returns the duration of the next route discovery to consider 
* from the list. If no next route discovery is present, it 
* returns a fixed values. 
*/
int GetNextExpiringRouteDiscovery(IN_ADDR *pDestIPAddr)
{
	BOOL Success;
	RouteDiscoveryTimerEntry NextRtDiscTimerEntry;
	int NextWaitTime;

	Success = RouteDiscoveryTimerListRemoveFromTail(
										pRouteDiscoveryTimerListHead, 
										&NextRtDiscTimerEntry);

	if(Success)
	{
		pDestIPAddr->S_un.S_addr = NextRtDiscTimerEntry.DestIPAddr.S_un.S_addr;
		NextWaitTime = NextRtDiscTimerEntry.ExpiryTime - GetCurMiliSecTime();
	}
	else
	{
		pDestIPAddr->S_un.S_addr = 0;
		NextWaitTime = 500;
	}

	return NextWaitTime;
}

/*
* Manages all the routing tables of the system. Does the 
* following activities,
*	- Updates the internal routing table
*	- Remove any entries in the route expiry timer list
*	- Updates the route expiry timer list with this entry
*	- Updates the address list with the passthru driver
*	- Updates the system routing table
*/
void UpdateRoute(PRouteEntry pRouteEntry, ULONG UpdateType)
{
	RouteExpiryEntry NewRouteExpEntry;
	RouteExpiryEntry OldRouteExpEntry;
	char PrintBuf[2048];
	DeleteExpiryEntry NextDelExpiryEntry;
	DeleteExpiryEntry OldDelExpiryEntry;

	// Remove entry in the route expiry timer list related to dest
	RouteExpiryListRemoveByValue(pRouteExpiryListHead, 
									&pRouteEntry->DestIPAddr, 
									&OldRouteExpEntry);

	// Remove entry in the delete expiry timer list related to dest
	DeleteExpiryListRemoveByValue(pDeleteExpiryListHead, 
									&pRouteEntry->DestIPAddr, 
									&OldDelExpiryEntry);

	// Updates the route expiry timer list with this entry
	if(UpdateType == UPDATE_TYPE_NOMAL)
	{
		pRouteEntry->ActiveThreadType = ACTIVE_THREAD_ROUTE_LIFETIME_MINDER;

		NewRouteExpEntry.DestIPAddr.S_un.S_addr = 
									pRouteEntry->DestIPAddr.S_un.S_addr;
		NewRouteExpEntry.ExpiryTime = pRouteEntry->ExpiryTime;
		RouteExpiryListInsertAtOrder(pRouteExpiryListHead, &NewRouteExpEntry);
	}
	
	// update the delete expiry list as the route is expiring
	// i.e. UPDATE_TYPE_DELETE
	else
	{
		pRouteEntry->ActiveThreadType = ACTIVE_THREAD_DELETE_PERIOD_MINDER;

		NextDelExpiryEntry.DestIPAddr.S_un.S_addr 
						= pRouteEntry->DestIPAddr.S_un.S_addr;
		NextDelExpiryEntry.ExpiryTime = pRouteEntry->ExpiryTime; 
		DeleteExpiryListInsertAtOrder(pDeleteExpiryListHead, &NextDelExpiryEntry);
	}

	// Updates the system routing table
	UpdateSystemRoutingTable(pRouteEntry);

	// Updates the internal routing table
	RouteListReplaceByValue(pRouteListHead, &pRouteEntry->DestIPAddr, 
								pRouteEntry);

	// Updates the address list with the passthru driver
	UpdatePTDriverWithNewValues();

	// log message
	sprintf(PrintBuf, "UoBWinAODV : Route updated - ");
	BuildRouteEntryAsString(pRouteEntry, (PrintBuf + strlen(PrintBuf)));
	EnterCriticalSection(pLoggingLock);
	LogMessage(LOGGING_SEVERAITY_INFO, PrintBuf);
	LeaveCriticalSection(pLoggingLock);

	//DumpInternalRouteTable();
}

/*
* Removes a route entry from the route list, after the delete
* period also is expired.
*/
void RemoveRoute(PRouteEntry pRouteEntry)
{
	RouteListRemoveByValue(pRouteListHead, &pRouteEntry->DestIPAddr, pRouteEntry);
	//DumpInternalRouteTable();
}

/*
* Updates the RREQID list and the timer list with this RREQID so that
* this RREQ is not acted upon more than one time
*/
void UpdateRREQID(IN_ADDR *pOrigIPAddr, ULONG *pRREQIDNum)
{
	RREQIDEntry NewRREQIDEntry;
	RREQIDExpiryEntry NewRREQIDExpiryEntry;

	NewRREQIDEntry.OrigIPAddr.S_un.S_addr = pOrigIPAddr->S_un.S_addr;
	NewRREQIDEntry.RREQIDNum = *pRREQIDNum;
	NewRREQIDEntry.ExpiryTime = GetCurMiliSecTime() + ParaPathDiscoveryTime;
	RREQIDListInsertAtHead(pRREQIDListHead, &NewRREQIDEntry);

	NewRREQIDExpiryEntry.OrigIPAddr.S_un.S_addr = NewRREQIDEntry.OrigIPAddr.S_un.S_addr;
	NewRREQIDExpiryEntry.RREQIDNum = NewRREQIDEntry.RREQIDNum;
	NewRREQIDExpiryEntry.ExpiryTime = NewRREQIDEntry.ExpiryTime;

	RREQIDExpiryListInsertAtOrder(pRREQIDExpiryListHead, &NewRREQIDExpiryEntry);
}

/*
* Updates the precursor list of a route with the given destination
* IP address
*/
void UpdatePrecursorList(PPrecursorListEntry pPrecursorListHead, IN_ADDR *pIPAddr)
{
	PrecursorEntry NewPrecursorEntry;

	NewPrecursorEntry.DestIPAddr.S_un.S_addr = pIPAddr->S_un.S_addr;
	PrecursorListReplaceByValue(pPrecursorListHead, pIPAddr, &NewPrecursorEntry);
}

/*
* Updates the route discovery expiry list with a new entry
*/
void UpdateRouteDiscovery(PRouteDiscoveryEntry pNewRtDiscEntry, IN_ADDR *pDestIPAddr)
{
	RouteDiscoveryTimerEntry RtDiscTimerEntry;

	RouteDiscoveryListReplaceByValue(pRouteDiscoveryListHead,
								pDestIPAddr,
								pNewRtDiscEntry);

	RtDiscTimerEntry.DestIPAddr.S_un.S_addr = pDestIPAddr->S_un.S_addr;
	RtDiscTimerEntry.ExpiryTime = GetCurMiliSecTime() 
										+ pNewRtDiscEntry->WaitDuration;
	RouteDiscoveryTimerListInsertAtOrder(pRouteDiscoveryTimerListHead, 
											&RtDiscTimerEntry);
}

/*
* Updates the Passthru driver with the IP address list for
* which packets must be let thru.
*/
void UpdatePTDriverWithNewValues()
{
	PIPv4BlockAddrArray	pARFilter;
	RouteEntry ExistRouteEntry;
	ULONG i, j, RtCount, BlockSize;
	BOOL Success;

	RtCount = RouteListSize();

	if(RtCount <= 0)
	{
		return;
	}

	BlockSize = sizeof(IPv4BlockAddrArray) + (sizeof(IPAddrInfo) * (RtCount - 1));

	pARFilter = (PIPv4BlockAddrArray) malloc(BlockSize);
	if(pARFilter == NULL)
	{
		return;
	}
	memset(pARFilter, 0, BlockSize);

	pARFilter->NumberElements = 0;
	j = 0;
	for(i = 0; i < RtCount; i++)
	{
		Success = RouteListViewByIndex(pRouteListHead, i, &ExistRouteEntry);
		if(!Success)
		{
			break;
		}
		if(ExistRouteEntry.RouteType == ROUTE_TYPE_AODV
			 && ExistRouteEntry.RouteStatusFlag != ROUTE_STATUS_FLAG_VALID)
		{
			continue;
		}
		pARFilter->NumberElements++;
		pARFilter->IPAddrInfoArray[j].IPAddr 
					= ExistRouteEntry.DestIPAddr.S_un.S_addr;
		pARFilter->IPAddrInfoArray[j].MaskIPAddr 
					= ExistRouteEntry.DestIPAddrMask.S_un.S_addr;
		j++;
	}
	if(pARFilter->NumberElements <= 0)
	{
		free(pARFilter);
		return;
	}

	SetFilter(pARFilter);

	// pARFilter freeing is done in SetFilter

	return;
}

/*
* Initialize the passthru driver to not filter any IP traffic
*/
void UpdatePTDriverWithNullValues()
{
	ClearFilter();
}

/*
* Updates the system (kernel) routing table checking whether it is
* currently set or not set
*/
void UpdateSystemRoutingTable(PRouteEntry pRouteEntry)
{
	RouteEntry OldRouteEntry;
	BOOL Success;

	Success = RouteListViewByValue(pRouteListHead, &pRouteEntry->DestIPAddr, 
									&OldRouteEntry);

	// update kernel routing table

	// if kernel route is SET in old entry and also should SET for new entry
	if(Success && OldRouteEntry.KernelRouteSet
		&& pRouteEntry->RouteStatusFlag == ROUTE_STATUS_FLAG_VALID) 
	{
		pRouteEntry->KernelRouteSet = TRUE;

	} 
	// if kernel route is NOT SET in old entry but should SET for new entry
	else if((!Success || !OldRouteEntry.KernelRouteSet)
			&& pRouteEntry->RouteStatusFlag == ROUTE_STATUS_FLAG_VALID) 
	{
		// set kernel route
		pRouteEntry->KernelRouteSet = TRUE;
		OSOpsAddRoute(pRouteEntry);
	}
    // if kernel route is SET in old entry but should NOT SET in new entry
	else if(Success && OldRouteEntry.KernelRouteSet
		&& (pRouteEntry->RouteStatusFlag != ROUTE_STATUS_FLAG_VALID)) 
	{
		// remove the kernel route entry
		pRouteEntry->KernelRouteSet = FALSE;
		OSOpsDeleteRoute(pRouteEntry);
	}
	// if kernel route is NOT SET in old entry and also should NOT SET for new entry
	else 
	{
		pRouteEntry->KernelRouteSet = FALSE;
	}
}

/*
* Reads the static routes and gets them in to the internal
* routing table at the begining of the protocol handler
*/
void UpdateStaticRoutes()
{
	// place static routes in internal table
	OSOpsSetStaticRoutes(pRouteListHead);

	// Updates the address list with the passthru driver
	UpdatePTDriverWithNewValues();
}

/*
* Undo the work done for static routes at the begining of the
* protocol handler. This is called at the termination of the 
* protocol handler.
*/
void InactivateRoutes()
{
	RouteEntry NextRouteEntry;
	ULONG NumRoutes, i;
	BOOL Success;

	NumRoutes = RouteListSize();
	for(i = 0; i < NumRoutes; i++)
	{
		Success = RouteListViewByIndex(pRouteListHead, i, &NextRouteEntry);
		if(!Success)
		{
			continue;
		}
		if(NextRouteEntry.RouteStatusFlag != ROUTE_STATUS_FLAG_VALID)
		{
			continue;
		}
		
		NextRouteEntry.RouteStatusFlag = ROUTE_STATUS_FLAG_INVALID;
		UpdateRoute(&NextRouteEntry, UPDATE_TYPE_NOMAL);
	}

	// Updates the address list with the passthru driver
	UpdatePTDriverWithNullValues();
}

/*
* Function prints the current status of protocol handler
*/
int PrintStatus()
{
	int NextWaitTime;

	//EnterCriticalSection(pWinAODVLock);

	if(ParaExecutionMode == EXECUTION_MODE_CONSOLE)
	{
		DumpInternalRouteTable();
	} 
	else if(ParaExecutionMode == EXECUTION_MODE_GUI)
	{
		DumpInternalRouteTableToGUI();
	} 

	// default to 1 sec if 0 is specified
	NextWaitTime = (ParaPrintFrequency == 0 ? 1000 : ParaPrintFrequency);

	//LeaveCriticalSection(pWinAODVLock);

	return (NextWaitTime);
}

/*
* Shows the usage of WinAODV
*/
void ShowUsage( void )
{
	_tprintf(_T("\n") );
	_tprintf(_T("Usage: UoBWinAODV [ options ]\r\n") );

	_tprintf(_T("\nOptions:\n") );
	_tprintf(_T("  /config filename - Start AODV with the given config file, filename\n") );
	_tprintf(_T("  /help            - Display this usage information\n") );
}

#if 0
/*
* Entry point to the protocol handler
*/
int _tmain(int argc, _TCHAR* argv[])
{
	int		RetCode = 0;
	FILE	*pConfigFile;

	//
	// Say Hello
	//
	_tprintf(_T("\n") );
	_tprintf(_T("UoBWinAODV - AODV Protocol Handler for Windows \n") );
	_tprintf(_T("Copyright (c) 2004 ComNets, University of Bremen, Germany\n") );

//	// initialize MFC and print and error on failure
//	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
//	{
//		// TODO: change error code to suit your needs
//		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
//		nRetCode = 1;
//	}
//	else
//	{
//		// TODO: code your application's behavior here.
//	}
//
	//
	// Parse Command Line
	//
	if( argc == 3 )
	{
		if( !_tcscmp( argv[1], _T("/config") ) )
		{
			pConfigFile = _tfopen( argv[ 2 ], _T("r") );

			if( !pConfigFile )
			{
				_tprintf( _T("Couldn't open Config File %s\n"), argv[ 2 ] );
				RetCode = 1;
			} 
			else
			{
				fclose( pConfigFile );
			}

			//pConfigFileName = _tcsdup(argv[ 2 ]);
			pConfigFileName = (char *)_tcsdup(argv[ 2 ]);
			if(!ReadConfigFile())
			{
				return 1;
			}

			BeginAODVOperations();
			LetAODVWork();
			EndAODVOperations();
			_tprintf(_T("\n\nUoBWinAODV terminated. \n\n") );

			return RetCode;
		}
	}

	ShowUsage();

	return RetCode;
}
#endif