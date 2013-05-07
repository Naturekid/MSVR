#include "stdafx.h"
#include "UoBWinAODV.h"


int _tmain(int argc, _TCHAR* argv[])
{
	PrecursorListEntry Head;
	PPrecursorListEntry pHead = &Head;
	PrecursorEntry NewPrecursorEntry;
	IN_ADDR IPAddr;

	PrecursorListInit(pHead);

	IPAddr.S_un.S_addr = inet_addr("134.102.158.209");
	NewPrecursorEntry.DestIPAddr.S_un.S_addr = IPAddr.S_un.S_addr;

	PrecursorListReplaceByValue(pHead, &IPAddr, &NewPrecursorEntry);


	IPAddr.S_un.S_addr = inet_addr("134.102.158.205");
	NewPrecursorEntry.DestIPAddr.S_un.S_addr = IPAddr.S_un.S_addr;

	PrecursorListReplaceByValue(pHead, &IPAddr, &NewPrecursorEntry);


	PrecursorListTerm(pHead);
}
