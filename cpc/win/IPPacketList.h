
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
* Header file that provides the definitions for
* IPPacketList.cpp
*
* @author : Asanga Udugama
* @date : 11-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#ifndef __IPPACKETLIST__H
#define __IPPACKETLIST__H

BOOL IPPacketListInit(PIPPacketListEntry pIPPacketListHead);
BOOL IPPacketListInsertAtHead(PIPPacketListEntry pIPPacketListHead, 
							  PIPPacket pIPPacket);
BOOL IPPacketListRemoveFromTail(PIPPacketListEntry pIPPacketListHead, 
								PIPPacket pIPPacket);
ULONG IPPacketListSize(PIPPacketListEntry pIPPacketListHead) ;
BOOL IPPacketListViewTail(PIPPacketListEntry pIPPacketListHead, PIPPacket pIPPacket);
BOOL IPPacketListTerm(PIPPacketListEntry pIPPacketListHead);

void IPPacketSendPackets(PIPPacketListEntry pIPPacketListHead, struct in_addr dst);
void IPPacketDropPackets(PIPPacketListEntry pIPPacketListHead, struct in_addr dst);

#endif // __IPPACKETLIST__H
