#include <winsock2.h>

#include "UoBWinAODVCommon.h"
#include "CpcQueueWin.h"
#include "IPPacketList.h"

extern PIPPacketListEntry pIPPacketListHead;

void CpcQueueSendPackets(struct in_addr dst)
{
	IPPacketSendPackets(pIPPacketListHead, dst);
}

void CpcQueueDropPackets(struct in_addr dst)
{
	IPPacketDropPackets(pIPPacketListHead, dst);
}