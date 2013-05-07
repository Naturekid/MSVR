
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
* Functions in this file manage the list maintained to hold 
* information related to routes that are being discovered.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "RouteDiscoveryList.h"


ULONG	RouteDiscoveryCount;

BOOL RouteDiscoveryListInit(PRouteDiscoveryListEntry pRouteDiscoveryListHead)
{
	pRouteDiscoveryListHead->pRouteDiscoveryValue = NULL;
	pRouteDiscoveryListHead->pNext = pRouteDiscoveryListHead;
	pRouteDiscoveryListHead->pPrev = pRouteDiscoveryListHead;
	RouteDiscoveryCount = 0;

	return(TRUE);
}

BOOL RouteDiscoveryListInsertAtHead(PRouteDiscoveryListEntry pRouteDiscoveryListHead, PRouteDiscoveryEntry pRouteDiscoveryEntry)
{
	PRouteDiscoveryEntry pNewRouteDiscoveryEntry;
	PRouteDiscoveryListEntry pNewRouteDiscoveryListEntry;

	if((pNewRouteDiscoveryEntry = (PRouteDiscoveryEntry) malloc(sizeof(RouteDiscoveryEntry))) == NULL)
	{
		return (FALSE);
	}

	if((pNewRouteDiscoveryListEntry = (PRouteDiscoveryListEntry) malloc(sizeof(RouteDiscoveryListEntry))) == NULL)
	{
		free(pNewRouteDiscoveryEntry);
		return (FALSE);
	}

	memcpy(pNewRouteDiscoveryEntry, pRouteDiscoveryEntry, sizeof(RouteDiscoveryEntry));
	pNewRouteDiscoveryListEntry->pRouteDiscoveryValue = pNewRouteDiscoveryEntry;

	// reset links
	pRouteDiscoveryListHead->pNext->pPrev = pNewRouteDiscoveryListEntry;
	pNewRouteDiscoveryListEntry->pNext = pRouteDiscoveryListHead->pNext;
	pRouteDiscoveryListHead->pNext = pNewRouteDiscoveryListEntry;
	pNewRouteDiscoveryListEntry->pPrev = pRouteDiscoveryListHead;

	RouteDiscoveryCount++;

	return(TRUE);
}

BOOL RouteDiscoveryListRemoveFromTail(PRouteDiscoveryListEntry pRouteDiscoveryListHead, PRouteDiscoveryEntry pRouteDiscoveryEntry)
{
	//PRouteDiscoveryEntry pExistingRouteDiscoveryEntry;
	PRouteDiscoveryListEntry pExistingRouteDiscoveryListEntry;

	if(RouteDiscoveryCount <= 0) 
	{
		return (FALSE);
	}

	pExistingRouteDiscoveryListEntry = pRouteDiscoveryListHead->pPrev;
	pRouteDiscoveryListHead->pPrev = pExistingRouteDiscoveryListEntry->pPrev;
	pExistingRouteDiscoveryListEntry->pPrev->pNext = pRouteDiscoveryListHead;

	RouteDiscoveryCount--;

	memcpy(pRouteDiscoveryEntry, pExistingRouteDiscoveryListEntry->pRouteDiscoveryValue, sizeof(RouteDiscoveryEntry));
	free(pExistingRouteDiscoveryListEntry->pRouteDiscoveryValue);
	free(pExistingRouteDiscoveryListEntry);

	return(TRUE);
}

ULONG RouteDiscoveryListSize() 
{
	return ( RouteDiscoveryCount );
}

BOOL RouteDiscoveryListViewTail(PRouteDiscoveryListEntry pRouteDiscoveryListHead, PRouteDiscoveryEntry pRouteDiscoveryEntry)
{
	PRouteDiscoveryListEntry pExistingRouteDiscoveryListEntry;

	if(RouteDiscoveryCount <= 0) 
	{
		return (FALSE);
	}

	pExistingRouteDiscoveryListEntry = pRouteDiscoveryListHead->pPrev;

	memcpy(pRouteDiscoveryEntry, pExistingRouteDiscoveryListEntry->pRouteDiscoveryValue, sizeof(RouteDiscoveryEntry));

	return(TRUE);
}

BOOL RouteDiscoveryListRemoveByValue(PRouteDiscoveryListEntry pRouteDiscoveryListHead,
								IN_ADDR *pDestIPAddr,
								PRouteDiscoveryEntry pRouteDiscoveryEntry)
{
	PRouteDiscoveryEntry pExistingRouteDiscoveryEntry;
	PRouteDiscoveryListEntry pExistingRouteDiscoveryListEntry;
	BOOL found;
	ULONG i;

	if(RouteDiscoveryCount <= 0) 
	{
		return (FALSE);
	}

	pExistingRouteDiscoveryListEntry = pRouteDiscoveryListHead;
	found = FALSE;
	for(i = 0; i < RouteDiscoveryCount; i++)
	{
		pExistingRouteDiscoveryListEntry = pExistingRouteDiscoveryListEntry->pNext;
		pExistingRouteDiscoveryEntry = pExistingRouteDiscoveryListEntry->pRouteDiscoveryValue;
		if(pDestIPAddr->S_un.S_addr 
				== pExistingRouteDiscoveryEntry->DestIPAddr.S_un.S_addr) 
		{
			found = TRUE;
			break;
		}
	}

	if(!found)
	{
		return (FALSE);
	}

	pExistingRouteDiscoveryListEntry->pPrev->pNext = pExistingRouteDiscoveryListEntry->pNext;
	pExistingRouteDiscoveryListEntry->pNext->pPrev = pExistingRouteDiscoveryListEntry->pPrev;

	RouteDiscoveryCount--;

	memcpy(pRouteDiscoveryEntry, pExistingRouteDiscoveryListEntry->pRouteDiscoveryValue, sizeof(RouteDiscoveryEntry));
	free(pExistingRouteDiscoveryListEntry->pRouteDiscoveryValue);
	free(pExistingRouteDiscoveryListEntry);

	return(TRUE);
}

BOOL RouteDiscoveryListViewByValue(PRouteDiscoveryListEntry pRouteDiscoveryListHead,
								IN_ADDR *pDestIPAddr,
								PRouteDiscoveryEntry pRouteDiscoveryEntry)
{
	PRouteDiscoveryEntry pExistingRouteDiscoveryEntry;
	PRouteDiscoveryListEntry pExistingRouteDiscoveryListEntry;
	BOOL found;
	ULONG i;

	if(RouteDiscoveryCount <= 0) 
	{
		return (FALSE);
	}

	pExistingRouteDiscoveryListEntry = pRouteDiscoveryListHead;
	found = FALSE;
	for(i = 0; i < RouteDiscoveryCount; i++)
	{
		pExistingRouteDiscoveryListEntry = pExistingRouteDiscoveryListEntry->pNext;
		pExistingRouteDiscoveryEntry = pExistingRouteDiscoveryListEntry->pRouteDiscoveryValue;
		if( pDestIPAddr->S_un.S_addr == pExistingRouteDiscoveryEntry->DestIPAddr.S_un.S_addr) 
		{
			found = TRUE;
			break;
		}
	}

	if(!found)
	{
		return (FALSE);
	}

	memcpy(pRouteDiscoveryEntry, pExistingRouteDiscoveryListEntry->pRouteDiscoveryValue, sizeof(RouteDiscoveryEntry));

	return(TRUE);
}

BOOL RouteDiscoveryListReplaceByValue(PRouteDiscoveryListEntry pRouteDiscoveryListHead,
								IN_ADDR *pDestIPAddr,
								PRouteDiscoveryEntry pRouteDiscoveryEntry)
{
	PRouteDiscoveryEntry pExistingRouteDiscoveryEntry;
	PRouteDiscoveryListEntry pExistingRouteDiscoveryListEntry;
	BOOL found;
	ULONG i;

	pExistingRouteDiscoveryListEntry = pRouteDiscoveryListHead;
	found = FALSE;
	for(i = 0; i < RouteDiscoveryCount; i++)
	{
		pExistingRouteDiscoveryListEntry = pExistingRouteDiscoveryListEntry->pNext;
		pExistingRouteDiscoveryEntry = pExistingRouteDiscoveryListEntry->pRouteDiscoveryValue;
		if( pDestIPAddr->S_un.S_addr == pExistingRouteDiscoveryEntry->DestIPAddr.S_un.S_addr) 
		{
			found = TRUE;
			break;
		}
	}

	if(!found)
	{
		if(!RouteDiscoveryListInsertAtHead(pRouteDiscoveryListHead, pRouteDiscoveryEntry))
		{
			return (FALSE);
		}
	}
	else
	{
		memcpy(pExistingRouteDiscoveryListEntry->pRouteDiscoveryValue, pRouteDiscoveryEntry, sizeof(RouteDiscoveryEntry));
	}

	return(TRUE);
}

BOOL RouteDiscoveryListTerm(PRouteDiscoveryListEntry pRouteDiscoveryListHead)
{
	RouteDiscoveryEntry ExistingRouteDiscoveryEntry;

	while(RouteDiscoveryCount > 0) 
	{
		RouteDiscoveryListRemoveFromTail(pRouteDiscoveryListHead, &ExistingRouteDiscoveryEntry);
	}

	return (TRUE);
}
