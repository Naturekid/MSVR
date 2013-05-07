
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
* precursor entries for each route.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "PrecursorList.h"

BOOL PrecursorListInit(PPrecursorListEntry pPrecursorListHead)
{
	pPrecursorListHead->pPrecursorValue = NULL;
	pPrecursorListHead->pNext = pPrecursorListHead;
	pPrecursorListHead->pPrev = pPrecursorListHead;
	pPrecursorListHead->PrecursorCount = 0;

	return(TRUE);
}

BOOL PrecursorListInsertAtHead(PPrecursorListEntry pPrecursorListHead, 
							   PPrecursorEntry pPrecursorEntry)
{
	PPrecursorEntry pNewPrecursorEntry;
	PPrecursorListEntry pNewPrecursorListEntry;

	if((pNewPrecursorEntry = (PPrecursorEntry) malloc(sizeof(PrecursorEntry))) == NULL)
	{
		return (FALSE);
	}

	if((pNewPrecursorListEntry = (PPrecursorListEntry) malloc(sizeof(PrecursorListEntry))) == NULL)
	{
		free(pNewPrecursorEntry);
		return (FALSE);
	}

	memcpy(pNewPrecursorEntry, pPrecursorEntry, sizeof(PrecursorEntry));
	pNewPrecursorListEntry->pPrecursorValue = pNewPrecursorEntry;

	// reset links
	pPrecursorListHead->pNext->pPrev = pNewPrecursorListEntry;
	pNewPrecursorListEntry->pNext = pPrecursorListHead->pNext;
	pPrecursorListHead->pNext = pNewPrecursorListEntry;
	pNewPrecursorListEntry->pPrev = pPrecursorListHead;

	pPrecursorListHead->PrecursorCount++;

	return(TRUE);
}

BOOL PrecursorListRemoveFromTail(PPrecursorListEntry pPrecursorListHead, 
								 PPrecursorEntry pPrecursorEntry)
{
	//PPrecursorEntry pExistingPrecursorEntry;
	PPrecursorListEntry pExistingPrecursorListEntry;

	if(pPrecursorListHead->PrecursorCount <= 0) 
	{
		return (FALSE);
	}

	pExistingPrecursorListEntry = pPrecursorListHead->pPrev;
	pPrecursorListHead->pPrev = pExistingPrecursorListEntry->pPrev;
	pExistingPrecursorListEntry->pPrev->pNext = pPrecursorListHead;

	pPrecursorListHead->PrecursorCount--;

	memcpy(pPrecursorEntry, pExistingPrecursorListEntry->pPrecursorValue, sizeof(PrecursorEntry));
	free(pExistingPrecursorListEntry->pPrecursorValue);
	free(pExistingPrecursorListEntry);

	return(TRUE);
}

ULONG PrecursorListSize(PPrecursorListEntry pPrecursorListHead) 
{
	return ( pPrecursorListHead->PrecursorCount );
}

BOOL PrecursorListViewTail(PPrecursorListEntry pPrecursorListHead, 
						   PPrecursorEntry pPrecursorEntry)
{
	PPrecursorListEntry pExistingPrecursorListEntry;

	if(pPrecursorListHead->PrecursorCount <= 0) 
	{
		return (FALSE);
	}

	pExistingPrecursorListEntry = pPrecursorListHead->pPrev;

	memcpy(pPrecursorEntry, pExistingPrecursorListEntry->pPrecursorValue, sizeof(PrecursorEntry));

	return(TRUE);
}

BOOL PrecursorListRemoveByValue(PPrecursorListEntry pPrecursorListHead, 
								IN_ADDR *IPAddr, 
								PPrecursorEntry pPrecursorEntry)
{
	PPrecursorEntry pExistingPrecursorEntry;
	PPrecursorListEntry pExistingPrecursorListEntry;
	BOOL found;
	ULONG i;

	if(pPrecursorListHead->PrecursorCount <= 0) 
	{
		return (FALSE);
	}

	pExistingPrecursorListEntry = pPrecursorListHead;
	found = FALSE;
	for(i = 0; i < pPrecursorListHead->PrecursorCount; i++)
	{
		pExistingPrecursorListEntry = pExistingPrecursorListEntry->pNext;
		pExistingPrecursorEntry = pExistingPrecursorListEntry->pPrecursorValue;
		if(IPAddr->S_un.S_addr == pExistingPrecursorEntry->DestIPAddr.S_un.S_addr) 
		{
			found = TRUE;
			break;
		}
	}

	if(!found)
	{
		return (FALSE);
	}

	pExistingPrecursorListEntry->pPrev->pNext = pExistingPrecursorListEntry->pNext;
	pExistingPrecursorListEntry->pNext->pPrev = pExistingPrecursorListEntry->pPrev;

	pPrecursorListHead->PrecursorCount--;

	memcpy(pPrecursorEntry, pExistingPrecursorListEntry->pPrecursorValue, sizeof(PrecursorEntry));
	free(pExistingPrecursorListEntry->pPrecursorValue);
	free(pExistingPrecursorListEntry);

	return(TRUE);
}

BOOL PrecursorListViewByValue(PPrecursorListEntry pPrecursorListHead, 
							  IN_ADDR *IPAddr, 
							  PPrecursorEntry pPrecursorEntry)
{
	PPrecursorEntry pExistingPrecursorEntry;
	PPrecursorListEntry pExistingPrecursorListEntry;
	BOOL found;
	ULONG i;

	if(pPrecursorListHead->PrecursorCount <= 0) 
	{
		return (FALSE);
	}

	pExistingPrecursorListEntry = pPrecursorListHead;
	found = FALSE;
	for(i = 0; i < pPrecursorListHead->PrecursorCount; i++)
	{
		pExistingPrecursorListEntry = pExistingPrecursorListEntry->pNext;
		pExistingPrecursorEntry = pExistingPrecursorListEntry->pPrecursorValue;
		if(IPAddr->S_un.S_addr == pExistingPrecursorEntry->DestIPAddr.S_un.S_addr) 
		{
			found = TRUE;
			break;
		}
	}

	if(!found)
	{
		return (FALSE);
	}

	memcpy(pPrecursorEntry, pExistingPrecursorListEntry->pPrecursorValue, sizeof(PrecursorEntry));

	return(TRUE);
}

BOOL PrecursorListViewByIndex(PPrecursorListEntry pPrecursorListHead, 
							  ULONG Index, 
							  PPrecursorEntry pPrecursorEntry)
{
	PPrecursorEntry pExistingPrecursorEntry;
	PPrecursorListEntry pExistingPrecursorListEntry;
	BOOL found;
	ULONG i;

	if(pPrecursorListHead->PrecursorCount <= 0) 
	{
		return (FALSE);
	}

	pExistingPrecursorListEntry = pPrecursorListHead;
	found = FALSE;
	for(i = 0; i < pPrecursorListHead->PrecursorCount; i++)
	{
		pExistingPrecursorListEntry = pExistingPrecursorListEntry->pNext;
		pExistingPrecursorEntry = pExistingPrecursorListEntry->pPrecursorValue;
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

	memcpy(pPrecursorEntry, pExistingPrecursorListEntry->pPrecursorValue, sizeof(PrecursorEntry));

	return(TRUE);
}

BOOL PrecursorListReplaceByValue(PPrecursorListEntry pPrecursorListHead, 
								 IN_ADDR *IPAddr, 
								 PPrecursorEntry pPrecursorEntry)
{
	PPrecursorEntry pExistingPrecursorEntry;
	PPrecursorListEntry pExistingPrecursorListEntry;
	BOOL found;
	ULONG i;

	pExistingPrecursorListEntry = pPrecursorListHead;
	found = FALSE;
	for(i = 0; i < pPrecursorListHead->PrecursorCount; i++)
	{
		pExistingPrecursorListEntry = pExistingPrecursorListEntry->pNext;
		pExistingPrecursorEntry = pExistingPrecursorListEntry->pPrecursorValue;
		if( IPAddr->S_un.S_addr == pExistingPrecursorEntry->DestIPAddr.S_un.S_addr) 
		{
			found = TRUE;
			break;
		}
	}

	if(!found)
	{
		if(!PrecursorListInsertAtHead(pPrecursorListHead, pPrecursorEntry))
		{
			return (FALSE);
		}
	}
	else
	{
		memcpy(pExistingPrecursorListEntry->pPrecursorValue, pPrecursorEntry, sizeof(PrecursorEntry));
	}

	return(TRUE);
}

BOOL PrecursorListTerm(PPrecursorListEntry pPrecursorListHead)
{
	PrecursorEntry ExistingPrecursorEntry;

	while(pPrecursorListHead->PrecursorCount > 0)
	{
		PrecursorListRemoveFromTail(pPrecursorListHead, &ExistingPrecursorEntry);
	}

	return (TRUE);
}
