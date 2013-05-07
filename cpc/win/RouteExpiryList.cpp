
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
* Functions in this file manage the list maintained to identify
* routes that are expiring.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "RouteExpiryList.h"

ULONG	RouteExpiryCount;

BOOL RouteExpiryListInit(PRouteExpiryListEntry pRouteExpiryListHead)
{
	pRouteExpiryListHead->pRouteExpiryValue = NULL;
	pRouteExpiryListHead->pNext = pRouteExpiryListHead;
	pRouteExpiryListHead->pPrev = pRouteExpiryListHead;
	RouteExpiryCount = 0;

	return(TRUE);
}

BOOL RouteExpiryListInsertAtHead(PRouteExpiryListEntry pRouteExpiryListHead, 
								 PRouteExpiryEntry pRouteExpiryEntry)
{
	PRouteExpiryEntry pNewRouteExpiryEntry;
	PRouteExpiryListEntry pNewRouteExpiryListEntry;

	if((pNewRouteExpiryEntry = (PRouteExpiryEntry) 
					malloc(sizeof(RouteExpiryEntry))) == NULL)
	{
		return (FALSE);
	}

	if((pNewRouteExpiryListEntry = (PRouteExpiryListEntry) 
					malloc(sizeof(RouteExpiryListEntry))) == NULL)
	{
		free(pNewRouteExpiryEntry);
		return (FALSE);
	}

	memcpy(pNewRouteExpiryEntry, pRouteExpiryEntry, sizeof(RouteExpiryEntry));
	pNewRouteExpiryListEntry->pRouteExpiryValue = pNewRouteExpiryEntry;

	// reset links
	pRouteExpiryListHead->pNext->pPrev = pNewRouteExpiryListEntry;
	pNewRouteExpiryListEntry->pNext = pRouteExpiryListHead->pNext;
	pRouteExpiryListHead->pNext = pNewRouteExpiryListEntry;
	pNewRouteExpiryListEntry->pPrev = pRouteExpiryListHead;

	RouteExpiryCount++;

	return(TRUE);
}

BOOL RouteExpiryListInsertAtOrder(PRouteExpiryListEntry pRouteExpiryListHead, 
								 PRouteExpiryEntry pRouteExpiryEntry)
{
	PRouteExpiryEntry pNewRouteExpiryEntry;
	PRouteExpiryListEntry pNewRouteExpiryListEntry;
	PRouteExpiryEntry pExistingRouteExpiryEntry ;
	PRouteExpiryListEntry pExistingRouteExpiryListEntry;
	PRouteExpiryListEntry pPrevRouteExpiryListEntry;
	ULONG i;

	if((pNewRouteExpiryEntry = (PRouteExpiryEntry) 
					malloc(sizeof(RouteExpiryEntry))) == NULL)
	{
		return (FALSE);
	}

	if((pNewRouteExpiryListEntry = (PRouteExpiryListEntry) 
					malloc(sizeof(RouteExpiryListEntry))) == NULL)
	{
		free(pNewRouteExpiryEntry);
		return (FALSE);
	}

	memcpy(pNewRouteExpiryEntry, pRouteExpiryEntry, sizeof(RouteExpiryEntry));
	pNewRouteExpiryListEntry->pRouteExpiryValue = pNewRouteExpiryEntry;

	if(RouteExpiryCount > 0)
	{
		pExistingRouteExpiryListEntry = pRouteExpiryListHead;
		pPrevRouteExpiryListEntry = pRouteExpiryListHead;
		for(i = 0; i < RouteExpiryCount; i++)
		{
			pExistingRouteExpiryListEntry = pExistingRouteExpiryListEntry->pNext;
			pExistingRouteExpiryEntry = pExistingRouteExpiryListEntry->pRouteExpiryValue;

			if(pExistingRouteExpiryEntry->ExpiryTime < pNewRouteExpiryEntry->ExpiryTime)
			{
				break;
			}
			pPrevRouteExpiryListEntry = pExistingRouteExpiryListEntry;
		}
	}
	else
	{
		pPrevRouteExpiryListEntry = pRouteExpiryListHead;
	}
		// reset links
	pPrevRouteExpiryListEntry->pNext->pPrev = pNewRouteExpiryListEntry;
	pNewRouteExpiryListEntry->pNext = pPrevRouteExpiryListEntry->pNext;
	pPrevRouteExpiryListEntry->pNext = pNewRouteExpiryListEntry;
	pNewRouteExpiryListEntry->pPrev = pPrevRouteExpiryListEntry;
	RouteExpiryCount++;

	return(TRUE);
}

BOOL RouteExpiryListRemoveByValue(PRouteExpiryListEntry pRouteExpiryListHead, 
								  IN_ADDR *pIPAddr, 
								  PRouteExpiryEntry pRouteExpiryEntry)
{
	PRouteExpiryEntry pExistingRouteExpiryEntry;
	PRouteExpiryListEntry pExistingRouteExpiryListEntry;
	BOOL found;
	ULONG i;

	if(RouteExpiryCount <= 0) 
	{
		return (FALSE);
	}

	pExistingRouteExpiryListEntry = pRouteExpiryListHead;
	found = FALSE;
	for(i = 0; i < RouteExpiryCount; i++)
	{
		pExistingRouteExpiryListEntry = pExistingRouteExpiryListEntry->pNext;
		pExistingRouteExpiryEntry = pExistingRouteExpiryListEntry->pRouteExpiryValue;
		if( pIPAddr->S_un.S_addr 
					== pExistingRouteExpiryEntry->DestIPAddr.S_un.S_addr) 
		{
			found = TRUE;
			break;
		}
	}

	if(!found)
	{
		return (FALSE);
	}

	pExistingRouteExpiryListEntry->pPrev->pNext = pExistingRouteExpiryListEntry->pNext;
	pExistingRouteExpiryListEntry->pNext->pPrev = pExistingRouteExpiryListEntry->pPrev;

	RouteExpiryCount--;

	memcpy(pRouteExpiryEntry, pExistingRouteExpiryListEntry->pRouteExpiryValue, sizeof(RouteExpiryEntry));
	free(pExistingRouteExpiryListEntry->pRouteExpiryValue);
	free(pExistingRouteExpiryListEntry);

	return(TRUE);
}

BOOL RouteExpiryListRemoveFromTail(PRouteExpiryListEntry pRouteExpiryListHead, 
										PRouteExpiryEntry pRouteExpiryEntry)
{
	//PRouteExpiryEntry pExistingRouteExpiryEntry;
	PRouteExpiryListEntry pExistingRouteExpiryListEntry;

	if(RouteExpiryCount <= 0) 
	{
		return (FALSE);
	}

	pExistingRouteExpiryListEntry = pRouteExpiryListHead->pPrev;
	pRouteExpiryListHead->pPrev = pExistingRouteExpiryListEntry->pPrev;
	pExistingRouteExpiryListEntry->pPrev->pNext = pRouteExpiryListHead;

	RouteExpiryCount--;

	memcpy(pRouteExpiryEntry, pExistingRouteExpiryListEntry->pRouteExpiryValue, 
										sizeof(RouteExpiryEntry));
	free(pExistingRouteExpiryListEntry->pRouteExpiryValue);
	free(pExistingRouteExpiryListEntry);

	return(TRUE);
}

BOOL RouteExpiryListInsertWithLogic(PRouteExpiryListEntry pRouteExpiryListHead, 
									IN_ADDR *pIPAddr, 
									PRouteExpiryEntry pRouteExpiryEntry)
{
	RouteExpiryEntry OldRouteExpiryEntry;
	BOOL Success;

	RouteExpiryListRemoveByValue(pRouteExpiryListHead, pIPAddr, &OldRouteExpiryEntry);
	Success = RouteExpiryListInsertAtOrder(pRouteExpiryListHead, pRouteExpiryEntry);

	return(Success);
}

ULONG RouteExpiryListSize() 
{
	return ( RouteExpiryCount );
}

BOOL RouteExpiryListViewTail(PRouteExpiryListEntry pRouteExpiryListHead, 
										PRouteExpiryEntry pRouteExpiryEntry)
{
	PRouteExpiryListEntry pExistingRouteExpiryListEntry;

	if(RouteExpiryCount <= 0) 
	{
		return (FALSE);
	}

	pExistingRouteExpiryListEntry = pRouteExpiryListHead->pPrev;

	memcpy(pRouteExpiryEntry, pExistingRouteExpiryListEntry->pRouteExpiryValue, 
										sizeof(RouteExpiryEntry));

	return(TRUE);
}

BOOL RouteExpiryListTerm(PRouteExpiryListEntry pRouteExpiryListHead)
{
	RouteExpiryEntry ExistingRouteExpiryEntry;

	while(RouteExpiryCount > 0) 
	{
		RouteExpiryListRemoveFromTail(pRouteExpiryListHead, &ExistingRouteExpiryEntry);
	}

	return (TRUE);
}
