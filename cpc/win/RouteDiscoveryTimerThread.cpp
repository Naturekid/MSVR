
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
* Functions in this file is for the creation and running of the
* thread that manages the discovery of routes.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "RouteDiscoveryTimerThread.h"

DWORD RouteDiscoveryTimerThreadID;
DWORD RouteDiscoveryTimerThreadParam; 
HANDLE RouteDiscoveryTimerThreadHandle; 

/*
* Thread function that calls the ProcessRouteDiscoveryFurther function
* to check for the continuation or termination of a route discovery
* process.
*/
DWORD WINAPI RouteDiscoveryTimerThreadFunc( LPVOID pParam ) 
{
	int TimeToWait;
	IN_ADDR DestIPAddr;

	DestIPAddr.S_un.S_addr = 0;
	while(TRUE)
	{
		// hold lock
		EnterCriticalSection(pWinAODVLock);

		TimeToWait = ProcessRouteDiscoveryFurther(&DestIPAddr);

		// release lock
		LeaveCriticalSection(pWinAODVLock);

		Sleep((DWORD) TimeToWait);
	}

    return 0; 
} 

/*
* Initializes the route discovery timer thread environment and starts the 
* thread
*/
VOID RouteDiscoveryTimerThreadInit( VOID ) 
{ 

	RouteDiscoveryTimerThreadParam = 1;
    RouteDiscoveryTimerThreadHandle = CreateThread(NULL, 0, RouteDiscoveryTimerThreadFunc,
							&RouteDiscoveryTimerThreadParam, 0,
							&RouteDiscoveryTimerThreadID);
 
   // Check the return value for success. 
   if (RouteDiscoveryTimerThreadHandle == NULL) 
   {
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_FATAL,
							"RouteDiscoveryTimerThread : Could not start the thread");
		LeaveCriticalSection(pLoggingLock);
   }
   else 
   {
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_INFO,
							"RouteDiscoveryTimerThread : Thread started successfully");
		LeaveCriticalSection(pLoggingLock);
   }
}

/*
* Terminates the route discovery timer thread
*/
VOID RouteDiscoveryTimerThreadTerm( VOID ) 
{
	BOOL Success;

	EnterCriticalSection(pLoggingLock);
	LogMessage(LOGGING_SEVERAITY_INFO,
						"RouteDiscoveryTimerThread : Thread being terminated");
	LeaveCriticalSection(pLoggingLock);

	
	Success = CloseHandle(RouteDiscoveryTimerThreadHandle);
	if(!Success)
	{
		printf("code %u \n", GetLastError());
	}
}
