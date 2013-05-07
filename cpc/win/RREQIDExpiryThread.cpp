
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
* thread that manages the lifetime of a RREQ ID which has been
* used.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "RREQIDExpiryThread.h"

DWORD RREQIDExpiryThreadID;
DWORD RREQIDExpiryThreadParam; 
HANDLE RREQIDExpiryThreadHandle; 

/*
* Thread function that calls the CheckRREQIDLifetime function
* to check for the next expiring route and wait until that time
* to call the function again.
*/
DWORD WINAPI RREQIDExpiryThreadFunc( LPVOID pParam ) 
{
	int TimeToWait;
	IN_ADDR OrigIPAddr;
	ULONG OrigRREQID;

	OrigIPAddr.S_un.S_addr = 0;
	OrigRREQID = 0;
	while(TRUE)
	{
		// hold lock
		EnterCriticalSection(pWinAODVLock);

		TimeToWait = RemoveRREQIDAtExpiry(&OrigIPAddr, &OrigRREQID);

		// release lock
		LeaveCriticalSection(pWinAODVLock);

		Sleep((DWORD) TimeToWait);
	}

    return 0; 
} 

/*
* Initializes the route expiry thread environment and starts the 
* thread
*/
VOID RREQIDExpiryThreadInit( VOID ) 
{ 

	RREQIDExpiryThreadParam = 1;
    RREQIDExpiryThreadHandle = CreateThread(NULL, 0, RREQIDExpiryThreadFunc,
							&RREQIDExpiryThreadParam, 0,
							&RREQIDExpiryThreadID);
 
   // Check the return value for success. 
   if (RREQIDExpiryThreadHandle == NULL) 
   {
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_FATAL,
							"RREQIDExpiryThread : Could not start the thread");
		LeaveCriticalSection(pLoggingLock);
   }
   else 
   {
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_INFO,
							"RREQIDExpiryThread : Thread started successfully");
		LeaveCriticalSection(pLoggingLock);
   }
}

/*
* Terminates the route expiry thread
*/
VOID RREQIDExpiryThreadTerm( VOID ) 
{
	EnterCriticalSection(pLoggingLock);
	LogMessage(LOGGING_SEVERAITY_INFO,
						"RREQIDExpiryThread : Thread being terminated");
	LeaveCriticalSection(pLoggingLock);

	CloseHandle(RREQIDExpiryThreadHandle);
}
