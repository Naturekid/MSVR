
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
* Functions in this file manage the list maintained to hold timers
* that expire to perform activities of routes being discovered.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "RouteDiscoveryTimerList.h"

ULONG	RouteDiscoveryTimerCount;

BOOL RouteDiscoveryTimerListInit(
						PRouteDiscoveryTimerListEntry pRouteDiscoveryTimerListHead)
{
	pRouteDiscoveryTimerListHead->pRouteDiscoveryTimerValue = NULL;
	pRouteDiscoveryTimerListHead->pNext = pRouteDiscoveryTimerListHead;
	pRouteDiscoveryTimerListHead->pPrev = pRouteDiscoveryTimerListHead;
	RouteDiscoveryTimerCount = 0;

	return(TRUE);
}

BOOL RouteDiscoveryTimerListInsertAtOrder(
			PRouteDiscoveryTimerListEntry pRouteDiscoveryTimerListHead, 
			PRouteDiscoveryTimerEntry pRouteDiscoveryTimerEntry)
{
	PRouteDiscoveryTimerEntry pNewRouteDiscoveryTimerEntry;
	PRouteDiscoveryTimerListEntry pNewRouteDiscoveryTimerListEntry;
	PRouteDiscoveryTimerEntry pExistingRouteDiscoveryTimerEntry ;
	PRouteDiscoveryTimerListEntry pExistingRouteDiscoveryTimerListEntry;
	PRouteDiscoveryTimerListEntry pPrevRouteDiscoveryTimerListEntry;
	ULONG i;

	if((pNewRouteDiscoveryTimerEntry = (PRouteDiscoveryTimerEntry) 
					malloc(sizeof(RouteDiscoveryTimerEntry))) == NULL)
	{
		return (FALSE);
	}

	if((pNewRouteDiscoveryTimerListEntry = (PRouteDiscoveryTimerListEntry) 
					malloc(sizeof(RouteDiscoveryTimerListEntry))) == NULL)
	{
		free(pNewRouteDiscoveryTimerEntry);
		return (FALSE);
	}

	memcpy(pNewRouteDiscoveryTimerEntry, pRouteDiscoveryTimerEntry, sizeof(RouteDiscoveryTimerEntry));
	pNewRouteDiscoveryTimerListEntry->pRouteDiscoveryTimerValue = pNewRouteDiscoveryTimerEntry;

	if(RouteDiscoveryTimerCount > 0)
	{
		pExistingRouteDiscoveryTimerListEntry = pRouteDiscoveryTimerListHead;
		pPrevRouteDiscoveryTimerListEntry = pRouteDiscoveryTimerListHead;
		for(i = 0; i < RouteDiscoveryTimerCount; i++)
		{
			pExistingRouteDiscoveryTimerListEntry = pExistingRouteDiscoveryTimerListEntry->pNext;
			pExistingRouteDiscoveryTimerEntry = pExistingRouteDiscoveryTimerListEntry->pRouteDiscoveryTimerValue;

			if(pExistingRouteDiscoveryTimerEntry->ExpiryTime < pNewRouteDiscoveryTimerEntry->ExpiryTime)
			{
				break;
			}
			pPrevRouteDiscoveryTimerListEntry = pExistingRouteDiscoveryTimerListEntry;
		}
	}
	else
	{
		pPrevRouteDiscoveryTimerListEntry = pRouteDiscoveryTimerListHead;
	}

	// reset links
	pPrevRouteDiscoveryTimerListEntry->pNext->pPrev = pNewRouteDiscoveryTimerListEntry;
	pNewRouteDiscoveryTimerListEntry->pNext = pPrevRouteDiscoveryTimerListEntry->pNext;
	pPrevRouteDiscoveryTimerListEntry->pNext = pNewRouteDiscoveryTimerListEntry;
	pNewRouteDiscoveryTimerListEntry->pPrev = pPrevRouteDiscoveryTimerListEntry;
	RouteDiscoveryTimerCount++;

	return(TRUE);
}

BOOL RouteDiscoveryTimerListRemoveFromTail(
		PRouteDiscoveryTimerListEntry pRouteDiscoveryTimerListHead, 
		PRouteDiscoveryTimerEntry pRouteDiscoveryTimerEntry)
{
	//PRouteDiscoveryTimerEntry pExistingRouteDiscoveryTimerEntry;
	PRouteDiscoveryTimerListEntry pExistingRouteDiscoveryTimerListEntry;

	if(RouteDiscoveryTimerCount <= 0) 
	{
		return (FALSE);
	}

	pExistingRouteDiscoveryTimerListEntry = pRouteDiscoveryTimerListHead->pPrev;
	pRouteDiscoveryTimerListHead->pPrev = pExistingRouteDiscoveryTimerListEntry->pPrev;
	pExistingRouteDiscoveryTimerListEntry->pPrev->pNext = pRouteDiscoveryTimerListHead;

	RouteDiscoveryTimerCount--;

	memcpy(pRouteDiscoveryTimerEntry, pExistingRouteDiscoveryTimerListEntry->pRouteDiscoveryTimerValue, sizeof(RouteDiscoveryTimerEntry));
	free(pExistingRouteDiscoveryTimerListEntry->pRouteDiscoveryTimerValue);
	free(pExistingRouteDiscoveryTimerListEntry);

	return(TRUE);
}

ULONG RouteDiscoveryTimerListSize() 
{
	return ( RouteDiscoveryTimerCount );
}

BOOL RouteDiscoveryTimerListTerm(
					PRouteDiscoveryTimerListEntry pRouteDiscoveryTimerListHead)
{
	RouteDiscoveryTimerEntry ExistingRouteDiscoveryTimerEntry;

	while(RouteDiscoveryTimerCount > 0) 
	{
		RouteDiscoveryTimerListRemoveFromTail(pRouteDiscoveryTimerListHead, 
								&ExistingRouteDiscoveryTimerEntry);
	}

	return (TRUE);
}
