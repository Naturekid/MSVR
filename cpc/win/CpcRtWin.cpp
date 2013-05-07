#include <winsock2.h>

#include "CpcRtWin.h"
#include "UoBWinAODVCommon.h"
#include "RouteList.h"


extern PRouteListEntry pRouteListHead;

void CpcRtAddWin(struct in_addr dst, struct in_addr next)
{
	BOOL successful;
	RouteEntry DestRouteEntry;

	successful = RouteListRemoveByValue(pRouteListHead, &dst, &DestRouteEntry);
	if (successful)
		CpcRtUpdateWin(dst, next);

	RouteListViewByIndex(pRouteListHead, 0, &DestRouteEntry);
	DestRouteEntry.DestIPAddr = next;

	RouteListInsertAtHead(pRouteListHead, &DestRouteEntry);
}

void CpcRtRemoveWin(struct in_addr dst)
{
	RouteEntry DestRouteEntry;

	RouteListRemoveByValue(pRouteListHead, &dst, &DestRouteEntry);
}

void CpcRtUpdateWin(struct in_addr dst, struct in_addr next)
{
	BOOL successful;
	RouteEntry DestRouteEntry;

	successful = RouteListRemoveByValue(pRouteListHead, &dst, &DestRouteEntry);
	if (!successful)
		return;

	DestRouteEntry.DestIPAddr = next;
	RouteListReplaceByValue(pRouteListHead, &dst, &DestRouteEntry);
}

struct in_addr *CpcRtFindWin(struct in_addr dst)
{
	BOOL successful;
	RouteEntry DestRouteEntry;

	successful = RouteListViewByValue(pRouteListHead, &dst, &DestRouteEntry);
	if (!successful)
		return NULL;

	return &DestRouteEntry.NextHopIPAddr;
}