
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
* IP packets until routes are discovered.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "IPPacketList.h"
#include "PacketSender.h"

BOOL IPPacketListInit(PIPPacketListEntry pIPPacketListHead)
{
	pIPPacketListHead->pIPPacketValue = NULL;
	pIPPacketListHead->pNext = pIPPacketListHead;
	pIPPacketListHead->pPrev = pIPPacketListHead;
	pIPPacketListHead->IPPacketCount = 0;

	return(TRUE);
}

BOOL IPPacketListInsertAtHead(PIPPacketListEntry pIPPacketListHead, 
							  PIPPacket pIPPacket)
{
	PIPPacket pNewIPPacket;
	PIPPacketListEntry pNewIPPacketListEntry;

	if((pNewIPPacket = (PIPPacket) malloc(sizeof(IPPacket))) == NULL)
	{
		return (FALSE);
	}
	memset(pNewIPPacket, 0, sizeof(IPPacket));

	if((pNewIPPacketListEntry = (PIPPacketListEntry) malloc(sizeof(IPPacketListEntry))) == NULL)
	{
		free(pNewIPPacket);
		return (FALSE);
	}
	memset(pNewIPPacketListEntry, 0, sizeof(IPPacketListEntry));

	memcpy(pNewIPPacket, pIPPacket, sizeof(IPPacket));
	pNewIPPacketListEntry->pIPPacketValue = pNewIPPacket;

	// reset links
	pIPPacketListHead->pNext->pPrev = pNewIPPacketListEntry;
	pNewIPPacketListEntry->pNext = pIPPacketListHead->pNext;
	pIPPacketListHead->pNext = pNewIPPacketListEntry;
	pNewIPPacketListEntry->pPrev = pIPPacketListHead;

	pIPPacketListHead->IPPacketCount++;

	return(TRUE);
}

BOOL IPPacketListRemoveFromTail(PIPPacketListEntry pIPPacketListHead, 
								PIPPacket pIPPacket)
{
	//PIPPacket pExistingIPPacket;
	PIPPacketListEntry pExistingIPPacketListEntry;

	if(pIPPacketListHead->IPPacketCount <= 0) 
	{
		return (FALSE);
	}

	pExistingIPPacketListEntry = pIPPacketListHead->pPrev;
	pIPPacketListHead->pPrev = pExistingIPPacketListEntry->pPrev;
	pExistingIPPacketListEntry->pPrev->pNext = pIPPacketListHead;

	pIPPacketListHead->IPPacketCount--;

	memcpy(pIPPacket, pExistingIPPacketListEntry->pIPPacketValue, sizeof(IPPacket));
	free(pExistingIPPacketListEntry->pIPPacketValue);
	free(pExistingIPPacketListEntry);

	return(TRUE);
}

ULONG IPPacketListSize(PIPPacketListEntry pIPPacketListHead) 
{
	return ( pIPPacketListHead->IPPacketCount );
}

BOOL IPPacketListViewTail(PIPPacketListEntry pIPPacketListHead, PIPPacket pIPPacket)
{
	PIPPacketListEntry pExistingIPPacketListEntry;

	if(pIPPacketListHead->IPPacketCount <= 0) 
	{
		return (FALSE);
	}

	pExistingIPPacketListEntry = pIPPacketListHead->pPrev;

	memcpy(pIPPacket, pExistingIPPacketListEntry->pIPPacketValue, sizeof(IPPacket));

	return(TRUE);
}

BOOL IPPacketListTerm(PIPPacketListEntry pIPPacketListHead)
{
	IPPacket ExistingIPPacket;

	while(pIPPacketListHead->IPPacketCount > 0) 
	{
		IPPacketListRemoveFromTail(pIPPacketListHead, &ExistingIPPacket);
	}

	return (TRUE);
}

BOOL IPPacketListGetByAddr(PIPPacketListEntry pIPPacketListHead, IN_ADDR *pIPAddr,
								PIPPacket pIPPacket)
{
	return TRUE;
}

void IPPacketSendPackets(PIPPacketListEntry pIPPacketListHead, struct in_addr dst)
{
	PIPPacketListEntry pIPPacketListEntry;

	if (pIPPacketListEntry->IPPacketCount == 0)
		return;

	for (pIPPacketListEntry = pIPPacketListHead; pIPPacketListEntry != NULL;
		pIPPacketListEntry = pIPPacketListEntry->pNext) {
			PIPPacketListEntry tmp = pIPPacketListEntry;
			PIPPacket pIPPacket = pIPPacketListEntry->pIPPacketValue;

			if (pIPPacket->ToIPAddr.S_un.S_addr == dst.S_un.S_addr) {
				SendIPPacket(pIPPacket);

				tmp->pPrev->pNext = tmp->pNext;
				tmp->pNext->pPrev = tmp->pPrev;

				pIPPacketListEntry = tmp->pPrev;
				free(tmp->pIPPacketValue);
				free(tmp);
			}
	}
}

void IPPacketDropPackets(PIPPacketListEntry pIPPacketListHead, struct in_addr dst)
{
	PIPPacketListEntry pIPPacketListEntry;
	
	if (pIPPacketListEntry->IPPacketCount == 0)
		return;
	
	for (pIPPacketListEntry = pIPPacketListHead; pIPPacketListEntry != NULL;
		pIPPacketListEntry = pIPPacketListEntry->pNext) {
			PIPPacketListEntry tmp = pIPPacketListEntry;
			PIPPacket pIPPacket = pIPPacketListEntry->pIPPacketValue;

			if (pIPPacket->ToIPAddr.S_un.S_addr == dst.S_un.S_addr) {
				tmp->pPrev->pNext = tmp->pNext;
				tmp->pNext->pPrev = tmp->pPrev;

				pIPPacketListEntry = tmp->pPrev;
				free(tmp->pIPPacketValue);
				free(tmp);
			}
	}
}