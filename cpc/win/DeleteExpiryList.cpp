
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
* Functions in this file manage the list maintained to effect 
* removal of routes after being in the delete state.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "DeleteExpiryList.h"


ULONG	DeleteExpiryCount;

BOOL DeleteExpiryListInit(PDeleteExpiryListEntry pDeleteExpiryListHead)
{
	pDeleteExpiryListHead->pDeleteExpiryValue = NULL;
	pDeleteExpiryListHead->pNext = pDeleteExpiryListHead;
	pDeleteExpiryListHead->pPrev = pDeleteExpiryListHead;
	DeleteExpiryCount = 0;

	return(TRUE);
}

BOOL DeleteExpiryListInsertAtHead(PDeleteExpiryListEntry pDeleteExpiryListHead, 
								 PDeleteExpiryEntry pDeleteExpiryEntry)
{
	PDeleteExpiryEntry pNewDeleteExpiryEntry;
	PDeleteExpiryListEntry pNewDeleteExpiryListEntry;

	if((pNewDeleteExpiryEntry = (PDeleteExpiryEntry) 
					malloc(sizeof(DeleteExpiryEntry))) == NULL)
	{
		return (FALSE);
	}

	if((pNewDeleteExpiryListEntry = (PDeleteExpiryListEntry) 
					malloc(sizeof(DeleteExpiryListEntry))) == NULL)
	{
		free(pNewDeleteExpiryEntry);
		return (FALSE);
	}

	memcpy(pNewDeleteExpiryEntry, pDeleteExpiryEntry, sizeof(DeleteExpiryEntry));
	pNewDeleteExpiryListEntry->pDeleteExpiryValue = pNewDeleteExpiryEntry;

	// reset links
	pDeleteExpiryListHead->pNext->pPrev = pNewDeleteExpiryListEntry;
	pNewDeleteExpiryListEntry->pNext = pDeleteExpiryListHead->pNext;
	pDeleteExpiryListHead->pNext = pNewDeleteExpiryListEntry;
	pNewDeleteExpiryListEntry->pPrev = pDeleteExpiryListHead;

	DeleteExpiryCount++;

	return(TRUE);
}

BOOL DeleteExpiryListInsertAtOrder(PDeleteExpiryListEntry pDeleteExpiryListHead, 
								 PDeleteExpiryEntry pDeleteExpiryEntry)
{
	PDeleteExpiryEntry pNewDeleteExpiryEntry;
	PDeleteExpiryListEntry pNewDeleteExpiryListEntry;
	PDeleteExpiryEntry pExistingDeleteExpiryEntry ;
	PDeleteExpiryListEntry pExistingDeleteExpiryListEntry;
	PDeleteExpiryListEntry pPrevDeleteExpiryListEntry;
	ULONG i;

	if((pNewDeleteExpiryEntry = (PDeleteExpiryEntry) 
					malloc(sizeof(DeleteExpiryEntry))) == NULL)
	{
		return (FALSE);
	}

	if((pNewDeleteExpiryListEntry = (PDeleteExpiryListEntry) 
					malloc(sizeof(DeleteExpiryListEntry))) == NULL)
	{
		free(pNewDeleteExpiryEntry);
		return (FALSE);
	}

	memcpy(pNewDeleteExpiryEntry, pDeleteExpiryEntry, sizeof(DeleteExpiryEntry));
	pNewDeleteExpiryListEntry->pDeleteExpiryValue = pNewDeleteExpiryEntry;

	if(DeleteExpiryCount > 0)
	{
		pExistingDeleteExpiryListEntry = pDeleteExpiryListHead;
		pPrevDeleteExpiryListEntry = pDeleteExpiryListHead;
		for(i = 0; i < DeleteExpiryCount; i++)
		{
			pExistingDeleteExpiryListEntry = pExistingDeleteExpiryListEntry->pNext;
			pExistingDeleteExpiryEntry = pExistingDeleteExpiryListEntry->pDeleteExpiryValue;

			if(pExistingDeleteExpiryEntry->ExpiryTime < pNewDeleteExpiryEntry->ExpiryTime)
			{
				break;
			}
			pPrevDeleteExpiryListEntry = pExistingDeleteExpiryListEntry;
		}
	}
	else
	{
		pPrevDeleteExpiryListEntry = pDeleteExpiryListHead;
	}
		// reset links
	pPrevDeleteExpiryListEntry->pNext->pPrev = pNewDeleteExpiryListEntry;
	pNewDeleteExpiryListEntry->pNext = pPrevDeleteExpiryListEntry->pNext;
	pPrevDeleteExpiryListEntry->pNext = pNewDeleteExpiryListEntry;
	pNewDeleteExpiryListEntry->pPrev = pPrevDeleteExpiryListEntry;
	DeleteExpiryCount++;

	return(TRUE);
}

BOOL DeleteExpiryListRemoveByValue(PDeleteExpiryListEntry pDeleteExpiryListHead, 
								  IN_ADDR *pIPAddr, 
								  PDeleteExpiryEntry pDeleteExpiryEntry)
{
	PDeleteExpiryEntry pExistingDeleteExpiryEntry;
	PDeleteExpiryListEntry pExistingDeleteExpiryListEntry;
	BOOL found;
	ULONG i;

	if(DeleteExpiryCount <= 0) 
	{
		return (FALSE);
	}

	pExistingDeleteExpiryListEntry = pDeleteExpiryListHead;
	found = FALSE;
	for(i = 0; i < DeleteExpiryCount; i++)
	{
		pExistingDeleteExpiryListEntry = pExistingDeleteExpiryListEntry->pNext;
		pExistingDeleteExpiryEntry = pExistingDeleteExpiryListEntry->pDeleteExpiryValue;
		if( pIPAddr->S_un.S_addr 
					== pExistingDeleteExpiryEntry->DestIPAddr.S_un.S_addr) 
		{
			found = TRUE;
			break;
		}
	}

	if(!found)
	{
		return (FALSE);
	}

	pExistingDeleteExpiryListEntry->pPrev->pNext = pExistingDeleteExpiryListEntry->pNext;
	pExistingDeleteExpiryListEntry->pNext->pPrev = pExistingDeleteExpiryListEntry->pPrev;

	DeleteExpiryCount--;

	memcpy(pDeleteExpiryEntry, pExistingDeleteExpiryListEntry->pDeleteExpiryValue, sizeof(DeleteExpiryEntry));
	free(pExistingDeleteExpiryListEntry->pDeleteExpiryValue);
	free(pExistingDeleteExpiryListEntry);

	return(TRUE);
}



BOOL DeleteExpiryListRemoveFromTail(PDeleteExpiryListEntry pDeleteExpiryListHead, 
										PDeleteExpiryEntry pDeleteExpiryEntry)
{
	//PDeleteExpiryEntry pExistingDeleteExpiryEntry;
	PDeleteExpiryListEntry pExistingDeleteExpiryListEntry;

	if(DeleteExpiryCount <= 0) 
	{
		return (FALSE);
	}

	pExistingDeleteExpiryListEntry = pDeleteExpiryListHead->pPrev;
	pDeleteExpiryListHead->pPrev = pExistingDeleteExpiryListEntry->pPrev;
	pExistingDeleteExpiryListEntry->pPrev->pNext = pDeleteExpiryListHead;

	DeleteExpiryCount--;

	memcpy(pDeleteExpiryEntry, pExistingDeleteExpiryListEntry->pDeleteExpiryValue, 
										sizeof(DeleteExpiryEntry));
	free(pExistingDeleteExpiryListEntry->pDeleteExpiryValue);
	free(pExistingDeleteExpiryListEntry);

	return(TRUE);
}

ULONG DeleteExpiryListSize() 
{
	return ( DeleteExpiryCount );
}

BOOL DeleteExpiryListViewTail(PDeleteExpiryListEntry pDeleteExpiryListHead, 
										PDeleteExpiryEntry pDeleteExpiryEntry)
{
	PDeleteExpiryListEntry pExistingDeleteExpiryListEntry;

	if(DeleteExpiryCount <= 0) 
	{
		return (FALSE);
	}

	pExistingDeleteExpiryListEntry = pDeleteExpiryListHead->pPrev;

	memcpy(pDeleteExpiryEntry, pExistingDeleteExpiryListEntry->pDeleteExpiryValue, 
										sizeof(DeleteExpiryEntry));

	return(TRUE);
}

BOOL DeleteExpiryListTerm(PDeleteExpiryListEntry pDeleteExpiryListHead)
{
	DeleteExpiryEntry ExistingDeleteExpiryEntry;

	while(DeleteExpiryCount > 0)
	{
		DeleteExpiryListRemoveFromTail(pDeleteExpiryListHead, 
			&ExistingDeleteExpiryEntry);
	}
	return (TRUE);
}
