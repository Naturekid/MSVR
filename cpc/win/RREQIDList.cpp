
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
* related to RREQ IDs currently used.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "RREQIDList.h"

ULONG	RREQIDCount;

BOOL RREQIDListInit(PRREQIDListEntry pRREQIDListHead)
{
	pRREQIDListHead->pRREQIDValue = NULL;
	pRREQIDListHead->pNext = pRREQIDListHead;
	pRREQIDListHead->pPrev = pRREQIDListHead;
	RREQIDCount = 0;

	return(TRUE);
}

BOOL RREQIDListInsertAtHead(PRREQIDListEntry pRREQIDListHead, PRREQIDEntry pRREQIDEntry)
{
	PRREQIDEntry pNewRREQIDEntry;
	PRREQIDListEntry pNewRREQIDListEntry;

	if((pNewRREQIDEntry = (PRREQIDEntry) malloc(sizeof(RREQIDEntry))) == NULL)
	{
		return (FALSE);
	}

	if((pNewRREQIDListEntry = (PRREQIDListEntry) malloc(sizeof(RREQIDListEntry))) == NULL)
	{
		free(pNewRREQIDEntry);
		return (FALSE);
	}

	memcpy(pNewRREQIDEntry, pRREQIDEntry, sizeof(RREQIDEntry));
	pNewRREQIDListEntry->pRREQIDValue = pNewRREQIDEntry;

	// reset links
	pRREQIDListHead->pNext->pPrev = pNewRREQIDListEntry;
	pNewRREQIDListEntry->pNext = pRREQIDListHead->pNext;
	pRREQIDListHead->pNext = pNewRREQIDListEntry;
	pNewRREQIDListEntry->pPrev = pRREQIDListHead;

	RREQIDCount++;

	return(TRUE);
}

BOOL RREQIDListRemoveFromTail(PRREQIDListEntry pRREQIDListHead, 
							  PRREQIDEntry pRREQIDEntry)
{
	//PRREQIDEntry pExistingRREQIDEntry;
	PRREQIDListEntry pExistingRREQIDListEntry;

	if(RREQIDCount <= 0) 
	{
		return (FALSE);
	}

	pExistingRREQIDListEntry = pRREQIDListHead->pPrev;
	pRREQIDListHead->pPrev = pExistingRREQIDListEntry->pPrev;
	pExistingRREQIDListEntry->pPrev->pNext = pRREQIDListHead;

	RREQIDCount--;

	memcpy(pRREQIDEntry, pExistingRREQIDListEntry->pRREQIDValue, sizeof(RREQIDEntry));
	free(pExistingRREQIDListEntry->pRREQIDValue);
	free(pExistingRREQIDListEntry);

	return(TRUE);
}

ULONG RREQIDListSize() 
{
	return ( RREQIDCount );
}

BOOL RREQIDListRemoveByValue(PRREQIDListEntry pRREQIDListHead,
								IN_ADDR *pOrigIPAddr, ULONG *pRREQIDNum,
								PRREQIDEntry pRREQIDEntry)
{
	PRREQIDEntry pExistingRREQIDEntry;
	PRREQIDListEntry pExistingRREQIDListEntry;
	BOOL found;
	ULONG i;

	if(RREQIDCount <= 0) 
	{
		return (FALSE);
	}

	pExistingRREQIDListEntry = pRREQIDListHead;
	found = FALSE;
	for(i = 0; i < RREQIDCount; i++)
	{
		pExistingRREQIDListEntry = pExistingRREQIDListEntry->pNext;
		pExistingRREQIDEntry = pExistingRREQIDListEntry->pRREQIDValue;
		if( pOrigIPAddr->S_un.S_addr == pExistingRREQIDEntry->OrigIPAddr.S_un.S_addr 
			&& *pRREQIDNum == pExistingRREQIDEntry->RREQIDNum) 
		{
			found = TRUE;
			break;
		}
	}

	if(!found)
	{
		return (FALSE);
	}

	pExistingRREQIDListEntry->pPrev->pNext = pExistingRREQIDListEntry->pNext;
	pExistingRREQIDListEntry->pNext->pPrev = pExistingRREQIDListEntry->pPrev;

	RREQIDCount--;

	memcpy(pRREQIDEntry, pExistingRREQIDListEntry->pRREQIDValue, sizeof(RREQIDEntry));
	free(pExistingRREQIDListEntry->pRREQIDValue);
	free(pExistingRREQIDListEntry);

	return(TRUE);
}

BOOL RREQIDListViewByValue(PRREQIDListEntry pRREQIDListHead, IN_ADDR *pIPAddr, 
						   ULONG *RREQIDNum, PRREQIDEntry pRREQIDEntry)
{
	PRREQIDEntry pExistingRREQIDEntry;
	PRREQIDListEntry pExistingRREQIDListEntry;
	BOOL found;
	ULONG i;

	if(RREQIDCount <= 0) 
	{
		return (FALSE);
	}

	pExistingRREQIDListEntry = pRREQIDListHead;
	found = FALSE;
	for(i = 0; i < RREQIDCount; i++)
	{
		pExistingRREQIDListEntry = pExistingRREQIDListEntry->pNext;
		pExistingRREQIDEntry = pExistingRREQIDListEntry->pRREQIDValue;
		if( (pExistingRREQIDEntry->OrigIPAddr.S_un.S_addr == pIPAddr->S_un.S_addr) 
			&& (pExistingRREQIDEntry->RREQIDNum == (*RREQIDNum))) 
		{
			found = TRUE;
			break;
		}
	}

	if(!found)
	{
		return (FALSE);
	}

	memcpy(pRREQIDEntry, pExistingRREQIDEntry, sizeof(RREQIDEntry));

	return(TRUE);
}

BOOL RREQIDListTerm(PRREQIDListEntry pRREQIDListHead)
{
	RREQIDEntry ExistingRREQIDEntry;

	while(RREQIDCount > 0) 
	{
		RREQIDListRemoveFromTail(pRREQIDListHead, &ExistingRREQIDEntry);
	}

	return(TRUE);
}
