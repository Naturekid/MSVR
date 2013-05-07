
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
* Functions in this file provide services for calling the driver 
* to set filters to filter IP traffic.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "FilterSetter.h"

PIPv4BlockAddrArray  pOldARFilter = NULL;

/*
* Calls the packet blocking function of the device
* driver to let the packets of existing routes, to
* go through
*/
void SetFilter(PIPv4BlockAddrArray  pARFilter)
{
	HANDLE	LowerHandle;
	LPTSTR	TmpAdapterName;
	BOOL	Success, CallSetFilter;
	UINT	CheckSize;

	CallSetFilter = TRUE;

	// check if it is an attempt to set the same old filter
	if(pOldARFilter != NULL)
	{
		if(pOldARFilter->NumberElements != pARFilter->NumberElements) 
		{
			CallSetFilter = TRUE;
		}
		else
		{
			CheckSize = sizeof(IPv4BlockAddrArray) 
							+ (sizeof(IPAddrInfo) * (pARFilter->NumberElements - 1));
			if(memcmp(pOldARFilter, pARFilter, CheckSize) == 0)
			{
				CallSetFilter = FALSE;
			}
		}
		free(pOldARFilter);
	}

	pOldARFilter = pARFilter;

	// if same old filter, then dont set
	if(CallSetFilter)
	{

		// hold PT Driver lock to read a packet
		EnterCriticalSection(pPtDriverLock);

		TmpAdapterName = _tcsdup( ( wchar_t *)&ParaIfaceName );
		LowerHandle = PtOpenAdapter(TmpAdapterName);

		if( LowerHandle != INVALID_HANDLE_VALUE )
		{
			Success = PtSetIPv4BlockingFilter(LowerHandle, pARFilter);

			if(!Success) 
			{
				EnterCriticalSection(pLoggingLock);
				LogMessage(LOGGING_SEVERAITY_ERROR,
						"FilterSetter : PT set filter has failed");
				LeaveCriticalSection(pLoggingLock);

				// if there was an error in setting th filter,
				// make it set the filter next time also
				free(pOldARFilter);
				pOldARFilter = NULL;
			}

			PtCloseAdapter(LowerHandle);
		} 
		else
		{
			EnterCriticalSection(pLoggingLock);
			LogMessage(LOGGING_SEVERAITY_ERROR,
						"FilterSetter : Unable to open PT device to set filter");
			LeaveCriticalSection(pLoggingLock);

			// if there was an error in opening the handle,
			// make it set the filter next time also
			free(pOldARFilter);
			pOldARFilter = NULL;
		}

		// release lock
		LeaveCriticalSection(pPtDriverLock);
	}
}

/*
* Calls the packet blocking function of the device
* driver to remove all filtering done
*/
void ClearFilter()
{
	HANDLE	LowerHandle;
	LPTSTR	TmpAdapterName;
	BOOL Rtn;

	// hold PT Driver lock to read a packet
	EnterCriticalSection(pPtDriverLock);

	//TmpAdapterName = _tcsdup( ParaIfaceName );
	TmpAdapterName = _tcsdup( (wchar_t *)&ParaIfaceName );
	LowerHandle = PtOpenAdapter(TmpAdapterName);

	if( LowerHandle != INVALID_HANDLE_VALUE )
	{
		Rtn = PtSetIPv4BlockingFilter(LowerHandle, NULL);

		PtCloseAdapter(LowerHandle);
	}

	// release lock
	LeaveCriticalSection(pPtDriverLock);
}
