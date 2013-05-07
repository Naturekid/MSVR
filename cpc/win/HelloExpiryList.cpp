
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
* Functions in this file manage the list maintained to effect the 
* removal of routes when hello messages are not heard.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "HelloExpiryList.h"

ULONG	HelloExpiryCount;

BOOL HelloExpiryListInit(PHelloExpiryListEntry pHelloExpiryListHead)
{
	pHelloExpiryListHead->pHelloExpiryValue = NULL;
	pHelloExpiryListHead->pNext = pHelloExpiryListHead;
	pHelloExpiryListHead->pPrev = pHelloExpiryListHead;
	HelloExpiryCount = 0;

	return(TRUE);
}

BOOL HelloExpiryListInsertAtHead(PHelloExpiryListEntry pHelloExpiryListHead, 
								 PHelloExpiryEntry pHelloExpiryEntry)
{
	PHelloExpiryEntry pNewHelloExpiryEntry;
	PHelloExpiryListEntry pNewHelloExpiryListEntry;

	if((pNewHelloExpiryEntry = (PHelloExpiryEntry) 
					malloc(sizeof(HelloExpiryEntry))) == NULL)
	{
		return (FALSE);
	}

	if((pNewHelloExpiryListEntry = (PHelloExpiryListEntry) 
					malloc(sizeof(HelloExpiryListEntry))) == NULL)
	{
		free(pNewHelloExpiryEntry);
		return (FALSE);
	}

	memcpy(pNewHelloExpiryEntry, pHelloExpiryEntry, sizeof(HelloExpiryEntry));
	pNewHelloExpiryListEntry->pHelloExpiryValue = pNewHelloExpiryEntry;

	// reset links
	pHelloExpiryListHead->pNext->pPrev = pNewHelloExpiryListEntry;
	pNewHelloExpiryListEntry->pNext = pHelloExpiryListHead->pNext;
	pHelloExpiryListHead->pNext = pNewHelloExpiryListEntry;
	pNewHelloExpiryListEntry->pPrev = pHelloExpiryListHead;

	HelloExpiryCount++;

	return(TRUE);
}

BOOL HelloExpiryListInsertAtOrder(PHelloExpiryListEntry pHelloExpiryListHead, 
								 PHelloExpiryEntry pHelloExpiryEntry)
{
	PHelloExpiryEntry pNewHelloExpiryEntry;
	PHelloExpiryListEntry pNewHelloExpiryListEntry;
	PHelloExpiryEntry pExistingHelloExpiryEntry ;
	PHelloExpiryListEntry pExistingHelloExpiryListEntry;
	PHelloExpiryListEntry pPrevHelloExpiryListEntry;
	ULONG i;

	if((pNewHelloExpiryEntry = (PHelloExpiryEntry) 
					malloc(sizeof(HelloExpiryEntry))) == NULL)
	{
		return (FALSE);
	}

	if((pNewHelloExpiryListEntry = (PHelloExpiryListEntry) 
					malloc(sizeof(HelloExpiryListEntry))) == NULL)
	{
		free(pNewHelloExpiryEntry);
		return (FALSE);
	}

	memcpy(pNewHelloExpiryEntry, pHelloExpiryEntry, sizeof(HelloExpiryEntry));
	pNewHelloExpiryListEntry->pHelloExpiryValue = pNewHelloExpiryEntry;

	if(HelloExpiryCount > 0)
	{
		pExistingHelloExpiryListEntry = pHelloExpiryListHead;
		pPrevHelloExpiryListEntry = pHelloExpiryListHead;
		for(i = 0; i < HelloExpiryCount; i++)
		{
			pExistingHelloExpiryListEntry = pExistingHelloExpiryListEntry->pNext;
			pExistingHelloExpiryEntry = pExistingHelloExpiryListEntry->pHelloExpiryValue;

			if(pExistingHelloExpiryEntry->ExpiryTime < pNewHelloExpiryEntry->ExpiryTime)
			{
				break;
			}
			pPrevHelloExpiryListEntry = pExistingHelloExpiryListEntry;
		}
	}
	else
	{
		pPrevHelloExpiryListEntry = pHelloExpiryListHead;
	}
		// reset links
	pPrevHelloExpiryListEntry->pNext->pPrev = pNewHelloExpiryListEntry;
	pNewHelloExpiryListEntry->pNext = pPrevHelloExpiryListEntry->pNext;
	pPrevHelloExpiryListEntry->pNext = pNewHelloExpiryListEntry;
	pNewHelloExpiryListEntry->pPrev = pPrevHelloExpiryListEntry;
	HelloExpiryCount++;

	return(TRUE);
}

BOOL HelloExpiryListRemoveFromTail(PHelloExpiryListEntry pHelloExpiryListHead, 
										PHelloExpiryEntry pHelloExpiryEntry)
{
	PHelloExpiryListEntry pExistingHelloExpiryListEntry;

	if(HelloExpiryCount <= 0) 
	{
		return (FALSE);
	}

	pExistingHelloExpiryListEntry = pHelloExpiryListHead->pPrev;
	pHelloExpiryListHead->pPrev = pExistingHelloExpiryListEntry->pPrev;
	pExistingHelloExpiryListEntry->pPrev->pNext = pHelloExpiryListHead;

	HelloExpiryCount--;

	memcpy(pHelloExpiryEntry, pExistingHelloExpiryListEntry->pHelloExpiryValue, 
										sizeof(HelloExpiryEntry));
	free(pExistingHelloExpiryListEntry->pHelloExpiryValue);
	free(pExistingHelloExpiryListEntry);

	return(TRUE);
}

ULONG HelloExpiryListSize() 
{
	return ( HelloExpiryCount );
}

BOOL HelloExpiryListViewTail(PHelloExpiryListEntry pHelloExpiryListHead, 
										PHelloExpiryEntry pHelloExpiryEntry)
{
	PHelloExpiryListEntry pExistingHelloExpiryListEntry;

	if(HelloExpiryCount <= 0) 
	{
		return (FALSE);
	}

	pExistingHelloExpiryListEntry = pHelloExpiryListHead->pPrev;

	memcpy(pHelloExpiryEntry, pExistingHelloExpiryListEntry->pHelloExpiryValue, 
										sizeof(HelloExpiryEntry));

	return(TRUE);
}

BOOL HelloExpiryListTerm(PHelloExpiryListEntry pHelloExpiryListHead)
{
	HelloExpiryEntry ExistingHelloExpiryEntry;

	while(HelloExpiryCount > 0) 
	{
		HelloExpiryListRemoveFromTail(pHelloExpiryListHead, &ExistingHelloExpiryEntry);
	}
	return (TRUE);
}
