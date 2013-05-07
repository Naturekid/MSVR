
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
* thread that manages the sending of hello messages when active
* routes exist.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "HelloSendThread.h"

DWORD HelloSendThreadID;
DWORD HelloSendThreadParam; 
HANDLE HelloSendThreadHandle; 

/*
* Thread function that calls the CheckDeleteRouteLifetime function
* to check for the next expiring route in Delete Mode and wait until 
* that time to call the function again.
*/
DWORD WINAPI HelloSendThreadFunc( LPVOID pParam ) 
{
	HELLOMessage LastHELLOMsg;
	int TimeToWait;

	LastHELLOMsg.ToIPAddr.S_un.S_addr = ParaIPAddressMulticast.S_un.S_addr;
	LastHELLOMsg.FromIPAddr.S_un.S_addr = ParaIPAddress.S_un.S_addr;
	LastHELLOMsg.TTLValue = 1;
	LastHELLOMsg.MultiCast = TRUE;
	strcpy(LastHELLOMsg.pIfaceName, ParaIfaceName);

	LastHELLOMsg.RepairFlag = FALSE;
	LastHELLOMsg.AckFlag = FALSE;
	LastHELLOMsg.PrefixSize = 0;
	LastHELLOMsg.HopCount = 0;
	LastHELLOMsg.DestIPAddr.S_un.S_addr = ParaIPAddress.S_un.S_addr;
	LastHELLOMsg.DestSeqNum = 0;
	LastHELLOMsg.OrigIPAddr.S_un.S_addr = ParaIPAddress.S_un.S_addr;
	LastHELLOMsg.LifeTime = (ParaAllowedHelloLoss * ParaHelloInterval);

	while(TRUE)
	{

		// hold lock
		EnterCriticalSection(pWinAODVLock);

		TimeToWait = SendNextHello(&LastHELLOMsg);

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
VOID HelloSendThreadInit( VOID ) 
{ 

	HelloSendThreadParam = 1;
    HelloSendThreadHandle = CreateThread(NULL, 0, HelloSendThreadFunc,
							&HelloSendThreadParam, 0,
							&HelloSendThreadID);
 
   // Check the return value for success. 
   if (HelloSendThreadHandle == NULL) 
   {
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_FATAL,
							"HelloSendThread : Could not start the thread");
		LeaveCriticalSection(pLoggingLock);
   }
   else 
   {
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_INFO,
							"HelloSendThread : Thread started successfully");
		LeaveCriticalSection(pLoggingLock);
   }
}

/*
* Terminates the delete expiry thread
*/
VOID HelloSendThreadTerm( VOID ) 
{
	EnterCriticalSection(pLoggingLock);
	LogMessage(LOGGING_SEVERAITY_INFO,
						"HelloSendThread : Thread being terminated");
	LeaveCriticalSection(pLoggingLock);

	CloseHandle(HelloSendThreadHandle);
}
