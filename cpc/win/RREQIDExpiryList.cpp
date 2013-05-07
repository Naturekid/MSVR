
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
* Functions in this file manage the list maintained to hold the
* expiry timers of RREQ IDs already used.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "RREQIDExpiryList.h"

ULONG	RREQIDExpiryCount;

BOOL RREQIDExpiryListInit(PRREQIDExpiryListEntry pRREQIDExpiryListHead)
{
	pRREQIDExpiryListHead->pRREQIDExpiryValue = NULL;
	pRREQIDExpiryListHead->pNext = pRREQIDExpiryListHead;
	pRREQIDExpiryListHead->pPrev = pRREQIDExpiryListHead;
	RREQIDExpiryCount = 0;

	return(TRUE);
}

BOOL RREQIDExpiryListInsertAtOrder(PRREQIDExpiryListEntry pRREQIDExpiryListHead, 
								 PRREQIDExpiryEntry pRREQIDExpiryEntry)
{
	PRREQIDExpiryEntry pNewRREQIDExpiryEntry;
	PRREQIDExpiryListEntry pNewRREQIDExpiryListEntry;
	PRREQIDExpiryEntry pExistingRREQIDExpiryEntry ;
	PRREQIDExpiryListEntry pExistingRREQIDExpiryListEntry;
	PRREQIDExpiryListEntry pPrevRREQIDExpiryListEntry;
	ULONG i;

	if((pNewRREQIDExpiryEntry = (PRREQIDExpiryEntry) 
					malloc(sizeof(RREQIDExpiryEntry))) == NULL)
	{
		return (FALSE);
	}

	if((pNewRREQIDExpiryListEntry = (PRREQIDExpiryListEntry) 
					malloc(sizeof(RREQIDExpiryListEntry))) == NULL)
	{
		free(pNewRREQIDExpiryEntry);
		return (FALSE);
	}

	memcpy(pNewRREQIDExpiryEntry, pRREQIDExpiryEntry, sizeof(RREQIDExpiryEntry));
	pNewRREQIDExpiryListEntry->pRREQIDExpiryValue = pNewRREQIDExpiryEntry;

	if(RREQIDExpiryCount > 0)
	{
		pExistingRREQIDExpiryListEntry = pRREQIDExpiryListHead;
		pPrevRREQIDExpiryListEntry = pRREQIDExpiryListHead;
		for(i = 0; i < RREQIDExpiryCount; i++)
		{
			pExistingRREQIDExpiryListEntry = pExistingRREQIDExpiryListEntry->pNext;
			pExistingRREQIDExpiryEntry = pExistingRREQIDExpiryListEntry->pRREQIDExpiryValue;

			if(pExistingRREQIDExpiryEntry->ExpiryTime < pNewRREQIDExpiryEntry->ExpiryTime)
			{
				break;
			}
			pPrevRREQIDExpiryListEntry = pExistingRREQIDExpiryListEntry;
		}
	}
	else
	{
		pPrevRREQIDExpiryListEntry = pRREQIDExpiryListHead;
	}

	// reset links
	pPrevRREQIDExpiryListEntry->pNext->pPrev = pNewRREQIDExpiryListEntry;
	pNewRREQIDExpiryListEntry->pNext = pPrevRREQIDExpiryListEntry->pNext;
	pPrevRREQIDExpiryListEntry->pNext = pNewRREQIDExpiryListEntry;
	pNewRREQIDExpiryListEntry->pPrev = pPrevRREQIDExpiryListEntry;
	RREQIDExpiryCount++;

	return(TRUE);
}

BOOL RREQIDExpiryListRemoveFromTail(PRREQIDExpiryListEntry pRREQIDExpiryListHead, PRREQIDExpiryEntry pRREQIDExpiryEntry)
{
	//PRREQIDExpiryEntry pExistingRREQIDExpiryEntry;
	PRREQIDExpiryListEntry pExistingRREQIDExpiryListEntry;

	if(RREQIDExpiryCount <= 0) 
	{
		return (FALSE);
	}

	pExistingRREQIDExpiryListEntry = pRREQIDExpiryListHead->pPrev;
	pRREQIDExpiryListHead->pPrev = pExistingRREQIDExpiryListEntry->pPrev;
	pExistingRREQIDExpiryListEntry->pPrev->pNext = pRREQIDExpiryListHead;

	RREQIDExpiryCount--;

	memcpy(pRREQIDExpiryEntry, pExistingRREQIDExpiryListEntry->pRREQIDExpiryValue, sizeof(RREQIDExpiryEntry));
	free(pExistingRREQIDExpiryListEntry->pRREQIDExpiryValue);
	free(pExistingRREQIDExpiryListEntry);

	return(TRUE);
}

ULONG RREQIDExpiryListSize() 
{
	return ( RREQIDExpiryCount );
}

BOOL RREQIDExpiryListTerm(PRREQIDExpiryListEntry pRREQIDExpiryListHead)
{
	RREQIDExpiryEntry ExistingRREQIDExpiryEntry;

	while(RREQIDExpiryCount > 0) 
	{
		RREQIDExpiryListRemoveFromTail(pRREQIDExpiryListHead, 
					&ExistingRREQIDExpiryEntry);
	}

	return(TRUE);
}
