
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
* thread that manages printing of the status of the protocol
* handler.
*
* @author : Asanga Udugama
* @date : 19-jul-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "StatusPrinterThread.h"

DWORD StatusPrinterThreadID;
DWORD StatusPrinterThreadParam; 
HANDLE StatusPrinterThreadHandle; 

/*
* Thread function that calls the PrintStatus function to print
* current status.
*/
DWORD WINAPI StatusPrinterThreadFunc( LPVOID pParam ) 
{
	int TimeToWait;

	while(TRUE)
	{
		// hold lock
		EnterCriticalSection(pWinAODVLock);

		TimeToWait = PrintStatus();

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
VOID StatusPrinterThreadInit( VOID ) 
{ 

	StatusPrinterThreadParam = 1;
    StatusPrinterThreadHandle = CreateThread(NULL, 0, StatusPrinterThreadFunc,
							&StatusPrinterThreadParam, 0,
							&StatusPrinterThreadID);
 
   // Check the return value for success. 
   if (StatusPrinterThreadHandle == NULL) 
   {
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_FATAL,
							"StatusPrinterThread : Could not start the thread");
		LeaveCriticalSection(pLoggingLock);
   }
   else 
   {
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_INFO,
							"StatusPrinterThread : Thread started successfully");
		LeaveCriticalSection(pLoggingLock);
   }
}

/*
* Terminates the delete expiry thread
*/
VOID StatusPrinterThreadTerm( VOID ) 
{
	EnterCriticalSection(pLoggingLock);
	LogMessage(LOGGING_SEVERAITY_INFO,
						"StatusPrinterThread : Thread being terminated");
	LeaveCriticalSection(pLoggingLock);

	CloseHandle(StatusPrinterThreadHandle);
}
