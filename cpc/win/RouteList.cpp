
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
* Functions in this file manage the list maintained to hold information
* of routes.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "RouteList.h"

ULONG	RouteCount;
ULONG	UnexpiredRouteCount;

BOOL RouteListInit(PRouteListEntry pRouteListHead)
{
	pRouteListHead->pRouteValue = NULL;
	pRouteListHead->pNext = pRouteListHead;
	pRouteListHead->pPrev = pRouteListHead;
	RouteCount = 0;
	UnexpiredRouteCount = 0;

	return(TRUE);
}

BOOL RouteListInsertAtHead(PRouteListEntry pRouteListHead, PRouteEntry pRouteEntry)
{
	PRouteEntry pNewRouteEntry;
	PRouteListEntry pNewRouteListEntry;

	if((pNewRouteEntry = (PRouteEntry) malloc(sizeof(RouteEntry))) == NULL)
	{
		return (FALSE);
	}

	if((pNewRouteListEntry = (PRouteListEntry) malloc(sizeof(RouteListEntry))) == NULL)
	{
		free(pNewRouteEntry);
		return (FALSE);
	}

	memcpy(pNewRouteEntry, pRouteEntry, sizeof(RouteEntry));
	pNewRouteListEntry->pRouteValue = pNewRouteEntry;

	// reset links
	pRouteListHead->pNext->pPrev = pNewRouteListEntry;
	pNewRouteListEntry->pNext = pRouteListHead->pNext;
	pRouteListHead->pNext = pNewRouteListEntry;
	pNewRouteListEntry->pPrev = pRouteListHead;

	RouteCount++;
	if(pRouteEntry->RouteStatusFlag == ROUTE_STATUS_FLAG_VALID)
	{
		UnexpiredRouteCount++;
	}

	return(TRUE);
}

BOOL RouteListInsertAtOrder(PRouteListEntry pRouteListHead, 
								 PRouteEntry pRouteEntry)
{
	PRouteEntry pNewRouteEntry;
	PRouteListEntry pNewRouteListEntry;
	PRouteEntry pExistingRouteEntry ;
	PRouteListEntry pExistingRouteListEntry;
	PRouteListEntry pPrevRouteListEntry;
	ULONG i;

	if((pNewRouteEntry = (PRouteEntry) 
					malloc(sizeof(RouteEntry))) == NULL)
	{
		return (FALSE);
	}

	if((pNewRouteListEntry = (PRouteListEntry) 
					malloc(sizeof(RouteListEntry))) == NULL)
	{
		free(pNewRouteEntry);
		return (FALSE);
	}

	memcpy(pNewRouteEntry, pRouteEntry, sizeof(RouteEntry));
	pNewRouteListEntry->pRouteValue = pNewRouteEntry;

	if(RouteCount > 0)
	{
		pExistingRouteListEntry = pRouteListHead;
		pPrevRouteListEntry = pRouteListHead;
		for(i = 0; i < RouteCount; i++)
		{
			pExistingRouteListEntry = pExistingRouteListEntry->pNext;
			pExistingRouteEntry = pExistingRouteListEntry->pRouteValue;

			if( (pExistingRouteEntry->DestIPAddr.S_un.S_addr 
					& pExistingRouteEntry->DestIPAddrMask.S_un.S_addr)
				> (pNewRouteEntry->DestIPAddr.S_un.S_addr 
					& pNewRouteEntry->DestIPAddrMask.S_un.S_addr))
			{
				break;
			}
			pPrevRouteListEntry = pExistingRouteListEntry;
		}
	}
	else
	{
		pPrevRouteListEntry = pRouteListHead;
	}
		// reset links
	pPrevRouteListEntry->pNext->pPrev = pNewRouteListEntry;
	pNewRouteListEntry->pNext = pPrevRouteListEntry->pNext;
	pPrevRouteListEntry->pNext = pNewRouteListEntry;
	pNewRouteListEntry->pPrev = pPrevRouteListEntry;

	RouteCount++;
	if(pRouteEntry->RouteStatusFlag == ROUTE_STATUS_FLAG_VALID)
	{
		UnexpiredRouteCount++;
	}

	return(TRUE);
}

BOOL RouteListRemoveFromTail(PRouteListEntry pRouteListHead, PRouteEntry pRouteEntry)
{
	//PRouteEntry pExistingRouteEntry;
	PRouteListEntry pExistingRouteListEntry;

	if(RouteCount <= 0) 
	{
		return (FALSE);
	}

	pExistingRouteListEntry = pRouteListHead->pPrev;
	pRouteListHead->pPrev = pExistingRouteListEntry->pPrev;
	pExistingRouteListEntry->pPrev->pNext = pRouteListHead;

	RouteCount--;
	if(pExistingRouteListEntry->pRouteValue->RouteStatusFlag 
					== ROUTE_STATUS_FLAG_VALID)
	{
		UnexpiredRouteCount--;
	}

	memcpy(pRouteEntry, pExistingRouteListEntry->pRouteValue, sizeof(RouteEntry));
	free(pExistingRouteListEntry->pRouteValue);
	free(pExistingRouteListEntry);

	return(TRUE);
}

ULONG RouteListSize() 
{
	return ( RouteCount );
}

ULONG UnexpiredRouteSize() 
{
	return(UnexpiredRouteCount);
}

BOOL RouteListViewTail(PRouteListEntry pRouteListHead, PRouteEntry pRouteEntry)
{
	PRouteListEntry pExistingRouteListEntry;

	if(RouteCount <= 0) 
	{
		return (FALSE);
	}

	pExistingRouteListEntry = pRouteListHead->pPrev;

	memcpy(pRouteEntry, pExistingRouteListEntry->pRouteValue, sizeof(RouteEntry));

	return(TRUE);
}

BOOL RouteListRemoveByValue(PRouteListEntry pRouteListHead, IN_ADDR *pIPAddr, 
								PRouteEntry pRouteEntry)
{
	PRouteEntry pExistingRouteEntry;
	PRouteListEntry pExistingRouteListEntry;
	BOOL found;
	ULONG i;

	if(RouteCount <= 0) 
	{
		return (FALSE);
	}

	pExistingRouteListEntry = pRouteListHead;
	found = FALSE;
	for(i = 0; i < RouteCount; i++)
	{
		pExistingRouteListEntry = pExistingRouteListEntry->pNext;
		pExistingRouteEntry = pExistingRouteListEntry->pRouteValue;
		if( (pIPAddr->S_un.S_addr & pExistingRouteEntry->DestIPAddrMask.S_un.S_addr) 
				== pExistingRouteEntry->DestIPAddr.S_un.S_addr) 
		{
			found = TRUE;
			break;
		}
	}

	if(!found)
	{
		return (FALSE);
	}

	pExistingRouteListEntry->pPrev->pNext = pExistingRouteListEntry->pNext;
	pExistingRouteListEntry->pNext->pPrev = pExistingRouteListEntry->pPrev;

	RouteCount--;
	if(pExistingRouteListEntry->pRouteValue->RouteStatusFlag 
						== ROUTE_STATUS_FLAG_VALID)
	{
		UnexpiredRouteCount--;
	}

	memcpy(pRouteEntry, pExistingRouteListEntry->pRouteValue, sizeof(RouteEntry));
	free(pExistingRouteListEntry->pRouteValue);
	free(pExistingRouteListEntry);

	return(TRUE);
}

BOOL RouteListViewByValue(PRouteListEntry pRouteListHead, IN_ADDR *pIPAddr, PRouteEntry pRouteEntry)
{
	PRouteEntry pExistingRouteEntry;
	PRouteListEntry pExistingRouteListEntry;
	BOOL found;
	ULONG i;

	if(RouteCount <= 0) 
	{
		return (FALSE);
	}

	pExistingRouteListEntry = pRouteListHead;
	found = FALSE;
	for(i = 0; i < RouteCount; i++)
	{
		pExistingRouteListEntry = pExistingRouteListEntry->pNext;
		pExistingRouteEntry = pExistingRouteListEntry->pRouteValue;
		if( (pIPAddr->S_un.S_addr & pExistingRouteEntry->DestIPAddrMask.S_un.S_addr) 
			== pExistingRouteEntry->DestIPAddr.S_un.S_addr) 
		{
			found = TRUE;
			break;
		}
	}

	if(!found)
	{
		return (FALSE);
	}

	memcpy(pRouteEntry, pExistingRouteListEntry->pRouteValue, sizeof(RouteEntry));

	return(TRUE);
}

BOOL RouteListViewByIndex(PRouteListEntry pRouteListHead, ULONG Index, PRouteEntry pRouteEntry)
{
	PRouteEntry pExistingRouteEntry;
	PRouteListEntry pExistingRouteListEntry;
	BOOL found;
	ULONG i;

	if(RouteCount <= 0)
	{
		return (FALSE);
	}

	if(Index >= RouteCount) 
	{
		return (FALSE);
	}

	pExistingRouteListEntry = pRouteListHead;
	found = FALSE;
	for(i = 0; i < RouteCount; i++)
	{
		pExistingRouteListEntry = pExistingRouteListEntry->pNext;
		pExistingRouteEntry = pExistingRouteListEntry->pRouteValue;
		if(i == Index) 
		{
			found = TRUE;
			break;
		}
	}

	if(!found)
	{
		return (FALSE);
	}

	memcpy(pRouteEntry, pExistingRouteListEntry->pRouteValue, sizeof(RouteEntry));

	return(TRUE);
}

BOOL RouteListReplaceByValue(PRouteListEntry pRouteListHead, IN_ADDR *pIPAddr, PRouteEntry pRouteEntry)
{
	PRouteEntry pExistingRouteEntry;
	PRouteListEntry pExistingRouteListEntry;
	BOOL found;
	ULONG i;

	pExistingRouteListEntry = pRouteListHead;
	found = FALSE;
	for(i = 0; i < RouteCount; i++)
	{
		pExistingRouteListEntry = pExistingRouteListEntry->pNext;
		pExistingRouteEntry = pExistingRouteListEntry->pRouteValue;
		if( (pIPAddr->S_un.S_addr & pExistingRouteEntry->DestIPAddrMask.S_un.S_addr) 
				== pExistingRouteEntry->DestIPAddr.S_un.S_addr)
		{
			found = TRUE;
			break;
		}
	}

	if(!found)
	{
		if(!RouteListInsertAtOrder(pRouteListHead, pRouteEntry))
		{
			return (FALSE);
		}
	}
	else
	{
		if(pExistingRouteListEntry->pRouteValue->RouteStatusFlag 
							== ROUTE_STATUS_FLAG_VALID)
		{
			UnexpiredRouteCount--;
		}

		memcpy(pExistingRouteListEntry->pRouteValue, pRouteEntry, sizeof(RouteEntry));
		if(pExistingRouteListEntry->pRouteValue->RouteStatusFlag 
								== ROUTE_STATUS_FLAG_VALID)
		{
			UnexpiredRouteCount++;
		}
	}

	return(TRUE);
}

BOOL RouteListTerm(PRouteListEntry pRouteListHead)
{
	RouteEntry ExistingRouteEntry;

	while(RouteCount > 0) 
	{
		RouteListRemoveFromTail(pRouteListHead, &ExistingRouteEntry);
	}

	return (TRUE);
}
