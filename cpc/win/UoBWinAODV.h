
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
* UoBWinAODV.cpp
*
* @author : Asanga Udugama
* @date : 11-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#pragma once

#ifndef __UOBWINAODV__H
#define __UOBWINAODV__H


#include "UoBWinAODVCommon.h"

//#ifdef __cplusplus
//extern "C" {
//#endif

extern CRITICAL_SECTION LoggingLock;
extern LPCRITICAL_SECTION pLoggingLock;
extern void LoggingInit();
extern void LogMessage(int LoggingLevel, char *pPrintStr);
extern void LoggingTerm();
extern void LogMessageInitial(char *pPrintStr);
//
extern BOOL RouteListInit(PRouteListEntry pRouteListHead);
extern BOOL RouteListViewByValue(PRouteListEntry pRouteListHead, IN_ADDR *pIPAddr, 
								PRouteEntry pRouteEntry);
extern BOOL RouteListReplaceByValue(PRouteListEntry pRouteListHead, IN_ADDR *pIPAddr, 
								PRouteEntry pRouteEntry);
extern ULONG RouteListSize(); 
extern ULONG UnexpiredRouteSize();
extern BOOL RouteListViewByIndex(PRouteListEntry pRouteListHead, ULONG Index, 
								PRouteEntry pRouteEntry);
extern BOOL RouteListRemoveByValue(PRouteListEntry pRouteListHead, IN_ADDR *pIPAddr, 
								PRouteEntry pRouteEntry);
extern BOOL RouteListTerm(PRouteListEntry pRouteListHead);
//
extern BOOL RREQIDListInit(PRREQIDListEntry pRREQIDListHead);
extern BOOL RREQIDListRemoveByValue(PRREQIDListEntry pRREQIDListHead,
								IN_ADDR *pOrigIPAddr, ULONG *pRREQIDNum,
								PRREQIDEntry pRREQIDEntry);
extern BOOL RREQIDListViewByValue(PRREQIDListEntry pRREQIDListHead, IN_ADDR *pIPAddr, 
						   ULONG *RREQIDNum, PRREQIDEntry pRREQIDEntry);
extern BOOL RREQIDListTerm(PRREQIDListEntry pRREQIDListHead);
//
extern BOOL RREQIDExpiryListInit(PRREQIDExpiryListEntry pRREQIDExpiryListHead);
extern BOOL RREQIDListInsertAtHead(PRREQIDListEntry pRREQIDListHead, 
							PRREQIDEntry pRREQIDEntry);
extern BOOL RREQIDExpiryListInsertAtOrder(PRREQIDExpiryListEntry pRREQIDExpiryListHead, 
								 PRREQIDExpiryEntry pRREQIDExpiryEntry);
extern BOOL RREQIDExpiryListRemoveFromTail(PRREQIDExpiryListEntry pRREQIDExpiryListHead, 
									PRREQIDExpiryEntry pRREQIDExpiryEntry);
extern BOOL RREQIDExpiryListTerm(PRREQIDExpiryListEntry pRREQIDExpiryListHead);
//
extern BOOL RouteDiscoveryListInit(PRouteDiscoveryListEntry pRouteDiscoveryListHead);
extern BOOL RouteDiscoveryListViewByValue(PRouteDiscoveryListEntry pRouteDiscoveryListHead,
								IN_ADDR *pDestIPAddr,
								PRouteDiscoveryEntry pRouteDiscoveryEntry);
extern BOOL RouteDiscoveryListReplaceByValue(PRouteDiscoveryListEntry pRouteDiscoveryListHead,
								IN_ADDR *pDestIPAddr,
								PRouteDiscoveryEntry pRouteDiscoveryEntry);
extern BOOL RouteDiscoveryListRemoveByValue(PRouteDiscoveryListEntry pRouteDiscoveryListHead,
								IN_ADDR *pDestIPAddr,
								PRouteDiscoveryEntry pRouteDiscoveryEntry);
extern BOOL RouteDiscoveryListRemoveFromTail(PRouteDiscoveryListEntry pRouteDiscoveryListHead, 
									  PRouteDiscoveryEntry pRouteDiscoveryEntry);
extern BOOL RouteDiscoveryListTerm(PRouteDiscoveryListEntry pRouteDiscoveryListHead);
//
extern BOOL RouteDiscoveryTimerListInit(
						PRouteDiscoveryTimerListEntry pRouteDiscoveryTimerListHead);
extern BOOL RouteDiscoveryTimerListRemoveFromTail(
		PRouteDiscoveryTimerListEntry pRouteDiscoveryTimerListHead, 
		PRouteDiscoveryTimerEntry pRouteDiscoveryTimerEntry);
extern BOOL RouteDiscoveryTimerListInsertAtOrder(
			PRouteDiscoveryTimerListEntry pRouteDiscoveryTimerListHead, 
			PRouteDiscoveryTimerEntry pRouteDiscoveryTimerEntry);
extern BOOL RouteDiscoveryTimerListTerm(
					PRouteDiscoveryTimerListEntry pRouteDiscoveryTimerListHead);
//
extern BOOL RouteExpiryListInit(PRouteExpiryListEntry pRouteExpiryListHead);
extern BOOL RouteExpiryListInsertAtOrder(PRouteExpiryListEntry pRouteExpiryListHead, 
								 PRouteExpiryEntry pRouteExpiryEntry);
extern BOOL RouteExpiryListRemoveByValue(PRouteExpiryListEntry pRouteExpiryListHead, 
								  IN_ADDR *pIPAddr, 
								  PRouteExpiryEntry pRouteExpiryEntry);
extern BOOL RouteExpiryListRemoveFromTail(PRouteExpiryListEntry pRouteExpiryListHead, 
										PRouteExpiryEntry pRouteExpiryEntry);
extern BOOL RouteExpiryListTerm(PRouteExpiryListEntry pRouteExpiryListHead);
//
extern BOOL DeleteExpiryListInit(PDeleteExpiryListEntry pDeleteExpiryListHead);
extern BOOL DeleteExpiryListInsertAtOrder(PDeleteExpiryListEntry pDeleteExpiryListHead, 
								 PDeleteExpiryEntry pDeleteExpiryEntry);
extern BOOL DeleteExpiryListRemoveFromTail(PDeleteExpiryListEntry pDeleteExpiryListHead, 
										PDeleteExpiryEntry pDeleteExpiryEntry);
BOOL DeleteExpiryListRemoveByValue(PDeleteExpiryListEntry pDeleteExpiryListHead, 
								  IN_ADDR *pIPAddr, 
								  PDeleteExpiryEntry pDeleteExpiryEntry);
extern BOOL DeleteExpiryListTerm(PDeleteExpiryListEntry pDeleteExpiryListHead);
//
extern BOOL HelloExpiryListInit(PHelloExpiryListEntry pHelloExpiryListHead);
extern BOOL HelloExpiryListInsertAtOrder(PHelloExpiryListEntry pHelloExpiryListHead, 
								 PHelloExpiryEntry pHelloExpiryEntry);
extern BOOL HelloExpiryListRemoveFromTail(PHelloExpiryListEntry pHelloExpiryListHead, 
										PHelloExpiryEntry pHelloExpiryEntry);
extern BOOL HelloExpiryListTerm(PHelloExpiryListEntry pHelloExpiryListHead);
//
extern BOOL PacketSenderInit();
extern BOOL SendRREQ(PRREQMessage pRREQMsg);
extern BOOL SendRREP(PRREPMessage pRREPMsg);
extern BOOL SendRERR(PRERRMessage pRERRMsg);
extern BOOL SendRREPACK(PRREPACKMessage pRREPACKMsg);
extern BOOL SendHELLO(PHELLOMessage pHELLOMsg);
extern BOOL SendIPPacket(PIPPacket pIPPacket);
extern BOOL PacketSenderTerm(); 

extern VOID PacketReceiverInit( VOID ); 
extern VOID PacketReceiverTerm( VOID ); 

extern VOID HelloSendThreadInit( VOID ); 
extern VOID HelloSendThreadTerm( VOID ); 

extern VOID RREQIDExpiryThreadInit(VOID); 
extern VOID RREQIDExpiryThreadTerm(VOID); 

extern VOID DeleteExpiryThreadInit(VOID); 
extern VOID DeleteExpiryThreadTerm(VOID); 

extern VOID RouteExpiryThreadInit( VOID ); 
extern VOID RouteExpiryThreadTerm( VOID ); 

extern VOID HelloExpiryThreadInit( VOID ); 
extern VOID HelloExpiryThreadTerm( VOID ); 

extern VOID RouteDiscoveryTimerThreadInit( VOID ); 
extern VOID RouteDiscoveryTimerThreadTerm( VOID ); 

extern VOID StatusPrinterThreadInit( VOID ); 
extern VOID StatusPrinterThreadTerm( VOID ); 

extern char *BuildRREQAsString(PRREQMessage pRREQMsgInternal, char *BuildStr);
extern char *BuildRREPAsString(PRREPMessage pRREPMsgInternal, char *BuildStr);
extern char *BuildRERRAsString(PRERRMessage pRERRMsgInternal, char *BuildStr);
extern char *BuildRREPACKAsString(PRREPACKMessage pRREPACKMsgInternal, char *BuildStr);
extern char *BuildHELLOAsString(PHELLOMessage pHELLOMsgInternal, char *BuildStr);
extern char *BuildRouteEntryAsString(PRouteEntry pRouteEntry, char *BuildStr);
extern MILISECTIME GetCurMiliSecTime(); 
extern ULONG IncrementSeqNum(ULONG *SeqNum);
extern ULONG IncrementRREQID(ULONG *RREQID);
extern void ShowRouteList();
extern void DumpInternalRouteTable();
extern void DumpInternalRouteTableToGUI();

extern BOOL PrecursorListInit(PPrecursorListEntry pPrecursorListHead);
extern BOOL PrecursorListViewByIndex(PPrecursorListEntry pPrecursorListHead, 
							  ULONG Index, 
							  PPrecursorEntry pPrecursorEntry);
extern BOOL PrecursorListReplaceByValue(PPrecursorListEntry pPrecursorListHead, 
								 IN_ADDR *IPAddr, 
								 PPrecursorEntry pPrecursorEntry);
extern BOOL PrecursorListTerm(PPrecursorListEntry pPrecursorListHead);

extern BOOL IPPacketListInit(PIPPacketListEntry pIPPacketListHead);
extern BOOL IPPacketListInsertAtHead(PIPPacketListEntry pIPPacketListHead, 
							  PIPPacket pIPPacket);
extern BOOL IPPacketListRemoveFromTail(PIPPacketListEntry pIPPacketListHead, 
								PIPPacket pIPPacket);
extern ULONG IPPacketListSize(PIPPacketListEntry pIPPacketListHead) ;
extern BOOL IPPacketListTerm(PIPPacketListEntry pIPPacketListHead);
//
extern void SetFilter(PIPv4BlockAddrArray  pARFilter);
extern void ClearFilter();
//
extern void OSOpsInit();
extern void OSOpsSetStaticRoutes(PRouteListEntry pRtListHead);
extern void OSOpsAddRoute(PRouteEntry pRouteEntry);
extern void OSOpsDeleteRoute(PRouteEntry pRouteEntry);
extern void OSOpsSetGatewayMAC();
extern void OSOpsTerm();
//
extern BOOL ReadConfigFile(); 
//
void BeginAODVOperations();
void EndAODVOperations();
int CheckActiveRouteLifetime(IN_ADDR *pDestIPAddr);
int GetNextExpiringActiveRoute(IN_ADDR *pDestIPAddr);
int CheckDeleteRouteLifetime(IN_ADDR *pDestIPAddr);
int GetNextExpiringDeleteRoute(IN_ADDR *pDestIPAddr);
int CheckHelloReceived(IN_ADDR *pDestIPAddr);
int GetNextExpiringHelloReceived(IN_ADDR *pDestIPAddr);
void InvalidateDestinations(PRouteEntry pDestRouteEntry);
void InvalidateDestinationsUnicast(PRouteEntry pDestRouteEntry);
int RemoveRREQIDAtExpiry(IN_ADDR *pOrigIPAddr, ULONG *pOrigRREQID);
int GetNextExpiringRREQID(IN_ADDR *pOrigIPAddr, ULONG *pOrigRREQID);
int SendNextHello(PHELLOMessage pLastHELLOMsg);
void ProcessRREQMsg(PRREQMessage pRREQMsg);
void ProcessRREPMsg(PRREPMessage pRREPMsg);
void ProcessRERRMsg(PRERRMessage pRERRMsg);
void ProcessRERRMsgUnicast(PRERRMessage pRERRMsg);
void ProcessRREPACKMsg(PRREPACKMessage pRREPACKMsg);
void ProcessHELLOMsg(PHELLOMessage pHELLOMsg);
void ProcessNormalPacket(PIPPacket pIPPkt);
void ProcessInprogressRouteDiscovery(PIPPacket pIPPkt, 
									 PRouteDiscoveryEntry pRouteDiscEntry);
void ProcessExistingRouteUse(PIPPacket pIPPkt);
void ProcessGatewayARP();
void ProcessRouteDiscovery(PIPPacket pIPPkt);
int ProcessRouteDiscoveryFurther(IN_ADDR *pDestIPAddr);
int GetNextExpiringRouteDiscovery(IN_ADDR *pDestIPAddr);
void UpdateRoute(PRouteEntry pRouteEntry, ULONG UpdateType);
void RemoveRoute(PRouteEntry pRouteEntry);
void UpdateRREQID(IN_ADDR *pOrigIPAddr, ULONG *pRREQIDNum);
void UpdatePrecursorList(PPrecursorListEntry pPrecursorListHead, IN_ADDR *IPAddr);
void UpdateRouteDiscovery(PRouteDiscoveryEntry pNewRtDiscEntry, IN_ADDR *pDestIPAddr);
void UpdatePTDriverWithNewValues();
void UpdatePTDriverWithNulValues();
void UpdateSystemRoutingTable(PRouteEntry pRouteEntry);
void UpdateStaticRoutes();
void ShowUsage( void );
void UpdateStaticRoutes();
void InactivateRoutes();
int PrintStatus();

//int _tmain(int argc, TCHAR* argv[], TCHAR* envp[]);

//#ifdef __cplusplus
//}
//#endif

#endif // __UOBWINAODV__H

