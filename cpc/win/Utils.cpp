
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
* This file contains functions that are used a utility 
* functions.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "Utils.h"

/*
* Returns a string that lists all the information in a RREQ
* message
*/
char *BuildRREQAsString(PRREQMessage pRREQMsgInternal, char *BuildStr)
{
	sprintf(BuildStr, 
		"DestIP %s, ",
		inet_ntoa((struct in_addr) pRREQMsgInternal->ToIPAddr));
		
	sprintf(BuildStr + strlen(BuildStr), 
		"SrcIP %s, TTL %d, Iface %s, Join %s, Repair %s, GratRREP %s, DestOnly %s, ",
		inet_ntoa((struct in_addr) pRREQMsgInternal->FromIPAddr),
		pRREQMsgInternal->TTLValue,
		pRREQMsgInternal->pIfaceName,
		(pRREQMsgInternal->JoinFlag == TRUE ? "TRUE" : "FALSE"),
		(pRREQMsgInternal->RepairFlag == TRUE ? "TRUE" : "FALSE"),
		(pRREQMsgInternal->GratRREPFlag == TRUE ? "TRUE" : "FALSE"),
		(pRREQMsgInternal->DestOnlyFlag == TRUE ? "TRUE" : "FALSE"));

	sprintf(BuildStr + strlen(BuildStr), 
		"UnknownSeq %s, Hops %d, RREQID %ld, DestIP %s, DestSeqNum %ld, ",
		(pRREQMsgInternal->UnknownSeqNumFlag == TRUE ? "TRUE" : "FALSE"),
		pRREQMsgInternal->HopCount,
		pRREQMsgInternal->RREQID,
		inet_ntoa((struct in_addr) pRREQMsgInternal->DestIPAddr),
		pRREQMsgInternal->DestSeqNum);

	sprintf(BuildStr + strlen(BuildStr), 
		"OrigIP %s, OrigSeqNum %ld",
		inet_ntoa((struct in_addr) pRREQMsgInternal->OrigIPAddr),
		pRREQMsgInternal->OrigSeqNum);

	return BuildStr;
}

/*
* Returns a string that lists all the information in a RREP
* message.
*/
char *BuildRREPAsString(PRREPMessage pRREPMsgInternal, char *BuildStr)
{
	sprintf(BuildStr, 
		"DestIP %s, ",
		inet_ntoa((struct in_addr) pRREPMsgInternal->ToIPAddr));

	sprintf(BuildStr + strlen(BuildStr), 
		"SrcIP %s, TTL %d, Iface %s, Repair %s, Ack %s, ",
		inet_ntoa((struct in_addr) pRREPMsgInternal->FromIPAddr),
		pRREPMsgInternal->TTLValue,
		pRREPMsgInternal->pIfaceName,
		(pRREPMsgInternal->RepairFlag == TRUE ? "TRUE" : "FALSE"),
		(pRREPMsgInternal->AckFlag == TRUE ? "TRUE" : "FALSE"));

	sprintf(BuildStr + strlen(BuildStr), 
		"Prefix %d Hops %d DestIP %s, DestSeqNum %d, OrigIP %s, Lifetime %d",
		pRREPMsgInternal->PrefixSize,
		pRREPMsgInternal->HopCount,
		inet_ntoa((struct in_addr) pRREPMsgInternal->DestIPAddr),
		pRREPMsgInternal->DestSeqNum,
		inet_ntoa((struct in_addr) pRREPMsgInternal->OrigIPAddr),
		pRREPMsgInternal->LifeTime);

	return BuildStr;
}

/*
* Returns a string that lists all the information in a RERR
* message.
*/
char *BuildRERRAsString(PRERRMessage pRERRMsgInternal, char *BuildStr)
{
	ULONG i;

	sprintf(BuildStr, 
		"DestIP %s, ",
		inet_ntoa((struct in_addr) pRERRMsgInternal->ToIPAddr));

	sprintf(BuildStr + strlen(BuildStr), 
		"SrcIP %s, TTL %d, Iface %s, NoDel %s, DestCount %d, ",
		inet_ntoa((struct in_addr) pRERRMsgInternal->FromIPAddr),
		pRERRMsgInternal->TTLValue,
		pRERRMsgInternal->pIfaceName,
		(pRERRMsgInternal->NoDelFlag == TRUE ? "TRUE" : "FALSE"),
		pRERRMsgInternal->DestCount);

	sprintf(BuildStr + strlen(BuildStr), 
		"Destinations ");
	for(i = 0; i < pRERRMsgInternal->DestCount; i++)
	{
		sprintf(BuildStr + strlen(BuildStr), 
			"No %d - %s %d ", 
			(i + 1),
			inet_ntoa((struct in_addr) pRERRMsgInternal->Unreachable[i].DestIPAddr),
			pRERRMsgInternal->Unreachable[i].DestSeqNum);
	}

	return BuildStr;
}

/*
* Returns a string that lists all the information in a RREPACK
* message.
*/
char *BuildRREPACKAsString(PRREPACKMessage pRREPACKMsgInternal, char *BuildStr)
{
	strcpy(BuildStr, "");
	return BuildStr;
}

/*
* Returns a string that lists all the information in a HELLO
* message.
*/
char *BuildHELLOAsString(PHELLOMessage pHELLOMsgInternal, char *BuildStr)
{
	sprintf(BuildStr, 
		"DestIP %s, ",
		inet_ntoa((struct in_addr) pHELLOMsgInternal->ToIPAddr));

	sprintf(BuildStr + strlen(BuildStr), 
		"SrcIP %s, TTL %d, Iface %s, Repair %s, Ack %s, ",
		inet_ntoa((struct in_addr) pHELLOMsgInternal->FromIPAddr),
		pHELLOMsgInternal->TTLValue,
		pHELLOMsgInternal->pIfaceName,
		(pHELLOMsgInternal->RepairFlag == TRUE ? "TRUE" : "FALSE"),
		(pHELLOMsgInternal->AckFlag == TRUE ? "TRUE" : "FALSE"));

	sprintf(BuildStr + strlen(BuildStr), 
		"Prefix %d Hops %d DestIP %s, DestSeqNum %d, OrigIP %s, Lifetime %d",
		pHELLOMsgInternal->PrefixSize,
		pHELLOMsgInternal->HopCount,
		inet_ntoa((struct in_addr) pHELLOMsgInternal->DestIPAddr),
		pHELLOMsgInternal->DestSeqNum,
		inet_ntoa((struct in_addr) pHELLOMsgInternal->OrigIPAddr),
		pHELLOMsgInternal->LifeTime);

	return BuildStr;
}

/*
* Returns a string that lists all the information in a route
* entry.
*/
char *BuildRouteEntryAsString(PRouteEntry pRouteEntry, char *BuildStr)
{
	ULONG i;

	sprintf(BuildStr, "DestIPAddr %s, ", 
			inet_ntoa((struct in_addr) pRouteEntry->DestIPAddr));
	sprintf(BuildStr + strlen(BuildStr), "DestIPAddrMask %s, ", 
			inet_ntoa((struct in_addr) pRouteEntry->DestIPAddrMask));
	sprintf(BuildStr + strlen(BuildStr), "DestSeqNum %d, ", 
			pRouteEntry->DestSeqNum);
	sprintf(BuildStr + strlen(BuildStr), "ValidDestSeqNumFlag %d, ", 
			pRouteEntry->ValidDestSeqNumFlag);
	sprintf(BuildStr + strlen(BuildStr), "RouteStatusFlag %d, ", 
			pRouteEntry->RouteStatusFlag);
	sprintf(BuildStr + strlen(BuildStr), "pIfaceName %s, ", 
			pRouteEntry->pIfaceName);
	sprintf(BuildStr + strlen(BuildStr), "HopCount %d, ", 
			pRouteEntry->HopCount);
	sprintf(BuildStr + strlen(BuildStr), "NextHopIPAddr %s, ", 
			inet_ntoa((struct in_addr) pRouteEntry->NextHopIPAddr));
	for(i = 0; i < pRouteEntry->PrecursorListHead.PrecursorCount; i++)
	{
		PrecursorEntry PrecursEntry;

		PrecursorListViewByIndex(&pRouteEntry->PrecursorListHead, 
									i, 
									&PrecursEntry);
		sprintf(BuildStr + strlen(BuildStr), "PrecursorEntry %d %s, ", 
					i, 
					inet_ntoa((struct in_addr) PrecursEntry.DestIPAddr));
	}
	sprintf(BuildStr + strlen(BuildStr), "ExpiryTime %d, ", 
			pRouteEntry->ExpiryTime);
	sprintf(BuildStr + strlen(BuildStr), "ActiveThreadType %d, ", 
			pRouteEntry->ActiveThreadType); 
	sprintf(BuildStr + strlen(BuildStr), "KernelRouteSet %d, ", 
			pRouteEntry->KernelRouteSet);
	sprintf(BuildStr + strlen(BuildStr), "NextHelloReceiveTime %d, ", 
			pRouteEntry->NextHelloReceiveTime);
	sprintf(BuildStr + strlen(BuildStr), "RouteType %d, ", 
			pRouteEntry->RouteType);

	return BuildStr;
}

/*
* Returns the current time in the milisecond format.
*/
MILISECTIME GetCurMiliSecTime() 
{
	MILISECTIME CurrentTime;
	struct _timeb SysTime; 

	_ftime(&SysTime);
	CurrentTime = (MILISECTIME) ((SysTime.time * 1000) + SysTime.millitm);

	return CurrentTime;
}

/*
* Returns the current time in a string format to be used
* when writing to the log.
*/
char *GetCurTimeForLogging(char *TimeStr)
{
	struct tm *DetailedTime;
	__time64_t SysTime;
	struct __timeb64 SysTime2; 

	_ftime64(&SysTime2); 

	SysTime = (__time64_t) SysTime2.time;
	//_time64(&SysTime);

	DetailedTime = _localtime64(&SysTime);
	sprintf(TimeStr, "%04d-%02d-%02d %02d:%02d:%02d:%03d", 
							(DetailedTime->tm_year + 1900),
							DetailedTime->tm_mon,
							DetailedTime->tm_mday,
							DetailedTime->tm_hour,
							DetailedTime->tm_min,
							DetailedTime->tm_sec,
							SysTime2.millitm);
	return (TimeStr);
}

/*
* Increments the given Sequence Number used in routes. This
* is speciically used with the local (own) Seq Num.
*/
ULONG IncrementSeqNum(ULONG *SeqNum)
{
	(*SeqNum)++;
	return (*SeqNum);
}

/*
* Increments the given RREQ ID used in route discovery. This
* is speciically used with the local (own) RREQ ID.
*/
ULONG IncrementRREQID(ULONG *RREQID)
{
	(*RREQID)++;
	return (*RREQID);
}

/*
* Debug functions : Lists the contents of the internal routing 
* table.
*/
void ShowRouteList()
{
	ULONG i;
	RouteEntry RtEntry;

	for(i = 0; i < RouteListSize(); i++)
	{

		RouteListViewByIndex(pRouteListHead, i, &RtEntry);
		ShowRouteEntry(&RtEntry);
	}
}

/*
* Debug functions : Lists a single entry of the internal routing 
* table.
*/
void ShowRouteEntry(PRouteEntry pRouteEntry)
{
	ULONG i;

	printf("DestIPAddr %s \n", 
			inet_ntoa((struct in_addr) pRouteEntry->DestIPAddr));
	printf("DestIPAddrMask %s \n", 
			inet_ntoa((struct in_addr) pRouteEntry->DestIPAddrMask));
	printf("DestSeqNum %d \n", pRouteEntry->DestSeqNum);
	printf("ValidDestSeqNumFlag %d \n", pRouteEntry->ValidDestSeqNumFlag);
	printf("RouteStatusFlag %d \n", pRouteEntry->RouteStatusFlag);
	printf("pIfaceName %s \n", pRouteEntry->pIfaceName);
	printf("HopCount %d \n", pRouteEntry->HopCount);
	printf("NextHopIPAddr %s \n", 
			inet_ntoa((struct in_addr) pRouteEntry->NextHopIPAddr));
	for(i = 0; i < pRouteEntry->PrecursorListHead.PrecursorCount; i++)
	{
		PrecursorEntry PrecursEntry;

		PrecursorListViewByIndex(&pRouteEntry->PrecursorListHead, 
									i, 
									&PrecursEntry);
		printf("PrecursorEntry %d %s \n", i, 
					inet_ntoa((struct in_addr) PrecursEntry.DestIPAddr));
	}
	printf("ExpiryTime %d \n", pRouteEntry->ExpiryTime);
	printf("ActiveThreadType %d \n", pRouteEntry->ActiveThreadType); 
	printf("KernelRouteSet %d \n", pRouteEntry->KernelRouteSet);
	printf("NextHelloReceiveTime %d \n", pRouteEntry->NextHelloReceiveTime);
	printf("RouteType %d \n", pRouteEntry->RouteType);
}

/*
* Debug functions : Lists the contents of the internal routing 
* table (different format).
*/
void DumpInternalRouteTable()
{
	ULONG i;
	RouteEntry RtEntry;
	char PrintBuf[2048], Str[128];
	MILISECTIME CurrTime;
	int LifeTime;

	CurrTime = GetCurMiliSecTime();

	printf("\nIP Address = %s \n", inet_ntoa((struct in_addr) ParaIPAddress));
	printf("Interface = %s \n", ParaIfaceName);
	printf("Last RREQID = %d \n", CurrLocalRREQID);
	printf("Last Seq = %d \n", CurrLocalSeqNum);

	printf("Destination     Mask            Next Hop        Hops Type   Status   Seq   Lifetime HelloLife Precursors Thread\n"); 
	printf("-----------     ----            --------        ---- ----   ------   ---   -------- --------- ---------- ------\n");
//                                                                                                    1        1
//          1         2         3         4         5         6         7         8         9         0        1
//0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345679012345
//Destination     Mask            Next Hop        Hops Type   Status   Seq   Lifetime HelloLife Precursors Thread
//255.255.255.255 255.255.255.255 255.255.255.255 255  Static Invalid  99999 99999    99999     99999      Del

	for(i = 0; i < RouteListSize(); i++)
	{
		if(!RouteListViewByIndex(pRouteListHead, i, &RtEntry))
		{
			continue;
		}
		memset(PrintBuf, ' ', 115);
		PrintBuf[115] = '\0';

		// destination
		sprintf(Str, "%s", inet_ntoa((struct in_addr) RtEntry.DestIPAddr));
		memcpy(PrintBuf + 0, Str, strlen(Str));

		// mask
		sprintf(Str, "%s", inet_ntoa((struct in_addr) RtEntry.DestIPAddrMask));
		memcpy(PrintBuf + 16, Str, strlen(Str));

		// next hop
		sprintf(Str, "%s", inet_ntoa((struct in_addr) RtEntry.NextHopIPAddr));
		memcpy(PrintBuf + 32, Str, strlen(Str));

		// hops
		sprintf(Str, "%d", RtEntry.HopCount);
		memcpy(PrintBuf + 48, Str, strlen(Str));

		// route type
		sprintf(Str, "%s", (RtEntry.RouteType == ROUTE_TYPE_STATIC 
												? "Static" : "AODV"));
		memcpy(PrintBuf + 53, Str, strlen(Str));

		// status
		if(RtEntry.RouteType != ROUTE_TYPE_STATIC)
		{
			sprintf(Str, "%s", (RtEntry.RouteStatusFlag == ROUTE_STATUS_FLAG_VALID 
														? "Valid" : "Invalid"));
			memcpy(PrintBuf + 60, Str, strlen(Str));
		}

		// seq
		sprintf(Str, "%d", RtEntry.DestSeqNum);
		memcpy(PrintBuf + 69, Str, strlen(Str));

		// lifetime
		LifeTime = (int) (RtEntry.ExpiryTime == 0 
								? 0 : (RtEntry.ExpiryTime - CurrTime));
		LifeTime = (LifeTime < 0 ? 0 : LifeTime);
		sprintf(Str, "%d", LifeTime);
		memcpy(PrintBuf + 75, Str, strlen(Str));

		// hello lifetime
		LifeTime = (int) (RtEntry.NextHelloReceiveTime == 0 
						? 0 : (RtEntry.NextHelloReceiveTime - CurrTime));
		LifeTime = (LifeTime < 0 ? 0 : LifeTime);
		sprintf(Str, "%d", LifeTime);
		memcpy(PrintBuf + 84, Str, strlen(Str));

		// precursors
		sprintf(Str, "%d", RtEntry.PrecursorListHead.PrecursorCount);
		memcpy(PrintBuf + 94, Str, strlen(Str));

		// active thread type
		if(RtEntry.ActiveThreadType == ACTIVE_THREAD_DELETE_PERIOD_MINDER)
		{
			sprintf(Str, "Del");
			memcpy(PrintBuf + 105, Str, strlen(Str));
		} 
		else if(RtEntry.ActiveThreadType == ACTIVE_THREAD_ROUTE_LIFETIME_MINDER)
		{
			sprintf(Str, "Act");
			memcpy(PrintBuf + 105, Str, strlen(Str));
		}

		strcat(PrintBuf, "\n");

		printf(PrintBuf);
	}
	printf("\nTotal Routes = %d, Valid Routes = %d\n\n", 
									RouteListSize(), UnexpiredRouteSize()); 
}

void DumpCharArray(char *pCharArray, int ArrayLen)
{
	int i;

	printf("\n");

	for(i = 0; i < ArrayLen; i++) 
	{
		if((i % 8) == 0)
		{
			printf("\n");
		}
		printf("%02x ", (unsigned char) *(pCharArray + i));
	}

}

void DumpInternalRouteTableToGUI()
{
	ULONG i;
	RouteEntry RtEntry;
	char PrintBuf[2048], str[64];
	MILISECTIME CurrTime;
	HANDLE PipeHandle;
	LPTSTR PipeName = (wchar_t *)"\\\\.\\pipe\\UoBWinAODV"; 
	DWORD DataWritten;
	BOOL Success;
	int LifeTime;

	PipeHandle = CreateFile(PipeName,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE, 
                NULL,                      // no security 
                OPEN_EXISTING,             // existing file only 
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 
 
	if(PipeHandle == INVALID_HANDLE_VALUE) 
	{ 
        return; 
	}
	CurrTime = GetCurMiliSecTime();

	sprintf(PrintBuf, "H %s %s %d %d %d %d", 
					inet_ntoa((struct in_addr) ParaIPAddress),
					ParaIfaceName,
					CurrLocalRREQID,
					CurrLocalSeqNum,
					RouteListSize(), 
					UnexpiredRouteSize()); 


	Success = WriteFile( 
         PipeHandle,				// handle to pipe 
         PrintBuf,				// buffer to write from 
         (DWORD) (strlen(PrintBuf) + 1),	// number of bytes to write 
         &DataWritten,				// number of bytes written 
         NULL);						// not overlapped I/O 

	if(!Success) 
	{
		return;
	}

	for(i = 0; i < RouteListSize(); i++)
	{
		if(!RouteListViewByIndex(pRouteListHead, i, &RtEntry))
		{
			continue;
		}
		memset(PrintBuf, 0, sizeof(PrintBuf));

		sprintf(PrintBuf, "D %s ", 
				inet_ntoa((struct in_addr) RtEntry.DestIPAddr));

		sprintf(PrintBuf + strlen(PrintBuf), " %s ", 
				inet_ntoa((struct in_addr) RtEntry.DestIPAddrMask));

		sprintf(PrintBuf + strlen(PrintBuf), " %s %d ", 
				inet_ntoa((struct in_addr) RtEntry.NextHopIPAddr),
				RtEntry.HopCount);

		if(RtEntry.RouteType == ROUTE_TYPE_STATIC)
		{
			strcpy(str, "Static");
		}
		else
		{
			strcpy(str, "AODV");
		}
		sprintf(PrintBuf + strlen(PrintBuf), " %s ", str); 				

		if(RtEntry.RouteType != ROUTE_TYPE_STATIC 
			&& RtEntry.RouteStatusFlag == ROUTE_STATUS_FLAG_VALID)
		{
			strcpy(str, "Valid");
		} 
		if(RtEntry.RouteType != ROUTE_TYPE_STATIC 
			&& RtEntry.RouteStatusFlag == ROUTE_STATUS_FLAG_INVALID)
		{
			strcpy(str, "Invalid");
		} 
		else
		{
			strcpy(str, "-");
		}
		sprintf(PrintBuf + strlen(PrintBuf), " %s %d ", 
									str, 
									RtEntry.DestSeqNum);
		
		LifeTime = (int) (RtEntry.ExpiryTime == 0 
								? 0 : (RtEntry.ExpiryTime - CurrTime));
		LifeTime = (LifeTime < 0 ? 0 : LifeTime);
		sprintf(PrintBuf + strlen(PrintBuf), " %d ", LifeTime);

		LifeTime = (int) (RtEntry.NextHelloReceiveTime == 0 
						? 0 : (RtEntry.NextHelloReceiveTime - CurrTime));
		LifeTime = (LifeTime < 0 ? 0 : LifeTime);
		sprintf(PrintBuf + strlen(PrintBuf), " %d %d ", 
								LifeTime,
								RtEntry.PrecursorListHead.PrecursorCount);

		if(RtEntry.ActiveThreadType == ACTIVE_THREAD_DELETE_PERIOD_MINDER)
		{
			strcpy(str, "Delete");
		}
		else if(RtEntry.ActiveThreadType == ACTIVE_THREAD_ROUTE_LIFETIME_MINDER)
		{
			strcpy(str, "Active");
		}
		else
		{
			strcpy(str, "-");
		}
		sprintf(PrintBuf + strlen(PrintBuf), " %s ", str);

		Success = WriteFile( 
							PipeHandle,				// handle to pipe 
							PrintBuf,				// buffer to write from 
							(DWORD) (strlen(PrintBuf) + 1),	// number of bytes to write 
							&DataWritten,				// number of bytes written 
							NULL);						// not overlapped I/O 

		if(!Success) 
		{
			return;
		}
	}

	CloseHandle(PipeHandle);
}
