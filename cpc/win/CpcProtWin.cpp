#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

#include "../cpc_prot.h"
#include "CpcProtWin.h"
#include "ReadParameters.h"
#include "UoBWinAODV.h"
#include "PacketSender.h"

extern PIPPacketListEntry pIPPacketListHead;

void CpcProtSendWin(char *p, int n, struct in_addr dst)
{
	char buf[1024];

	// maybe 7 hops need to be redesigned
	BuildIPHeader(buf, ParaIPAddress, dst, 7, 4);
	BuildUDPHeader(buf+40, ROUTING_PORT, ROUTING_PORT, 2);
	memcpy(buf+6, p, n);
	SendPacketOut(buf, n+6, dst, ROUTING_PORT);
}

void CpcProtSendWinWithData(char *p, int n, struct in_addr dst)
{
	BOOL successful;
	char buf[1024];
	IPPacket pIPPacket;
	PIPPacket tmp;

	successful = IPPacketListRemoveFromTail(pIPPacketListHead, &pIPPacket);
	while (successful) {
		// maybe 7 hops need to be redesigned
		BuildIPHeader(buf, ParaIPAddress, dst, 7, 4);
		BuildUDPHeader(buf+40, ROUTING_PORT, ROUTING_PORT, 2);

		// copy routing packet data
		memcpy(buf+6, &n, sizeof(n));
		memcpy(buf+6+sizeof(n), p, n);

		// copy data packet
		tmp = (PIPPacket) buf+6+sizeof(n)+n;
		memcpy(tmp->pIPPacket, pIPPacket.pIPPacket, pIPPacket.TotalPacketSize);
		
		SendPacketOut(buf, n+6, dst, ROUTING_PORT);
	}
}

void CpcProtInitWin(void)
{
	if (!ReadConfigFile())
		exit(-1);

	BeginAODVOperations();
}

void CpcProtDesWin(void)
{
	EndAODVOperations();
}