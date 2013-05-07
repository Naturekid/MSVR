
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
* thread that manages the lifetime of a route that is in the
* delete state.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "DeleteExpiryThread.h"

DWORD DeleteExpiryThreadID;
DWORD DeleteExpiryThreadParam; 
HANDLE DeleteExpiryThreadHandle; 

/*
* Thread function that calls the CheckDeleteRouteLifetime function
* to check for the next expiring route in Delete Mode and wait until 
* that time to call the function again.
*/
DWORD WINAPI DeleteExpiryThreadFunc( LPVOID pParam ) 
{
	int TimeToWait;
	IN_ADDR DestIPAddr;

	DestIPAddr.S_un.S_addr = 0;
	while(TRUE)
	{
		// hold lock
		EnterCriticalSection(pWinAODVLock);

		TimeToWait = CheckDeleteRouteLifetime(&DestIPAddr);

		// release lock
		LeaveCriticalSection(pWinAODVLock);

		Sleep((DWORD) TimeToWait);
	}

    return 0; 
} 

/*
* Initializes the delete expiry thread environment and starts the 
* thread
*/
VOID DeleteExpiryThreadInit( VOID ) 
{ 

	DeleteExpiryThreadParam = 1;
    DeleteExpiryThreadHandle = CreateThread(NULL, 0, DeleteExpiryThreadFunc,
							&DeleteExpiryThreadParam, 0,
							&DeleteExpiryThreadID);
 
   // Check the return value for success. 
   if (DeleteExpiryThreadHandle == NULL) 
   {
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_FATAL,
							"DeleteExpiryThread : Could not start the thread");
		LeaveCriticalSection(pLoggingLock);
   }
   else 
   {
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_INFO,
							"DeleteExpiryThread : Thread started successfully");
		LeaveCriticalSection(pLoggingLock);
   }
}

/*
* Terminates the delete expiry thread
*/
VOID DeleteExpiryThreadTerm( VOID ) 
{
	EnterCriticalSection(pLoggingLock);
	LogMessage(LOGGING_SEVERAITY_INFO,
						"DeleteExpiryThread : Thread being terminated");
	LeaveCriticalSection(pLoggingLock);

	CloseHandle(DeleteExpiryThreadHandle);
}
