
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
* PacketReceiver.cpp
*
* @author : Asanga Udugama
* @date : 11-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#ifndef __PACKETRECEIVER__H
#define __PACKETRECEIVER__H

#define  DEVICE_PREFIX_STR_A  "\\\\.\\"
#define  DEVICE_PREFIX_STR_W  L"\\\\.\\"

#define  DEVICE_STR_A         "\\Device\\"
#define  DEVICE_STR_W         L"\\Device\\"


HANDLE
PtOpenControlChannel( void );

BOOL
PtEnumerateBindings(
   HANDLE PtHandle,
   PCHAR  NameBuffer,
   DWORD *NameBufferLength
   );

HANDLE
PtOpenAdapterW( LPWSTR pszAdapterName );

HANDLE
PtOpenAdapterA( LPSTR pszAdapterName );

#ifdef UNICODE
#define PtOpenAdapter   PtOpenAdapterW
#else
#define PtOpenAdapter   PtOpenAdapterA
#endif

BOOL
PtCloseAdapter( HANDLE hAdapter );

DWORD
PtQueryInformation(
	HANDLE   hAdapter,
	ULONG    OidCode,
	PVOID    InformationBuffer,
	UINT     InformationBufferLength,
	PULONG   pBytesWritten
	);

BOOL
PtSetIPv4BlockingFilter(
   HANDLE               hAdapter,
   PIPv4BlockAddrArray  pIPv4BlockAddrArray
   );

//BOOL
//PtQueryIPv4Statistics(
//   HANDLE               hAdapter,
//   PIPv4AddrStats       pIPv4Stats
//   );

BOOL
PtGetStoredIPv4Packet(
   HANDLE               hAdapter,
   void *				pData,
   ULONG				BufferSize,
   ULONG *				ActualBufferSize
   );


// AODV parameters
extern int ParaExecutionMode;
extern int ParaIPVersion;
extern IN_ADDR ParaIPAddress;
extern char ParaIfaceName[MAX_IFACE_SIZE];
extern IN_ADDR ParaGatewayIPAddress;
extern int ParaLoggingStatus;
extern int ParaLoggingMode;
extern int ParaLoggingLevel;
extern char ParaLogFile[MAX_LOG_FILE_NAME_SIZE];
extern int ParaOnlyDestination;
extern int ParaGratuitousRREP;
extern int ParaRREPAckRequired;
extern IN_ADDR ParaIPAddressMulticast;
extern int ParaRERRSendingMode;
extern int ParaDeletePeriodMode;
extern int ParaRouteDiscoveryMode;
extern int ParaPacketBuffering;
extern int ParaActiveRouteTimeout;
extern int ParaAllowedHelloLoss;
extern int ParaHelloInterval;
extern int ParaLocalAddTTL;
extern int ParaNetDiameter;
extern int ParaNodeTraversalTime;
extern int ParaRERRRatelimit;
extern int ParaRREQRetries;
extern int ParaRREQRateLimit;
extern int ParaTimeoutBuffer;
extern int ParaTTLStart;
extern int ParaTTLIncrement;
extern int ParaTTLThreshold;
extern int ParaNetTraversalTime;
extern int ParaBlacklistTimeout;
extern int ParaMaxRepairTTL;
extern int ParaMyRouteTimeout;
extern int ParaNextHopWait;
extern int ParaPathDiscoveryTime;
extern int ParaDeletePeriod;


// PtDriver lock
extern CRITICAL_SECTION PtDriverLock;
extern LPCRITICAL_SECTION pPtDriverLock;

// WinAODV lock
extern CRITICAL_SECTION WinAODVLock;
extern LPCRITICAL_SECTION pWinAODVLock;

// Current info
extern ULONG CurrLocalSeqNum;
extern ULONG CurrLocalRREQID;

extern void ProcessRREQMsg(PRREQMessage pRREQMsg);
extern void ProcessRREPMsg(PRREPMessage pRREPMsg);
extern void ProcessRERRMsg(PRERRMessage pRERRMsg);
extern void ProcessRREPACKMsg(PRREPACKMessage pRREPACKMsg);
extern void ProcessHELLOMsg(PHELLOMessage pHELLOMsg);
extern void ProcessNormalPacket(PIPPacket pIPPkt);
extern void ProcessGatewayARP();
extern CRITICAL_SECTION LoggingLock;
extern LPCRITICAL_SECTION pLoggingLock;
extern void LogMessage(int LoggingLevel, char *pPrintStr);

extern int GetAnIPPacket(char *PacketBuffer, int BufSize);

DWORD WINAPI PacketReceiverThreadFunc( LPVOID pParam );
void GetAndProcessPackets(LPTSTR TmpAdapterName, char *pPacketData);
void ProcessEachPacket(char *pPacketData, ULONG PacketSize);
void BuildAODVMsg(char *pIPPktData);
void BuildNormalPacket(int PacketType, char *pIPPktData, ULONG PacketSize);
VOID PacketReceiverInit( VOID ); 
VOID PacketReceiverThreadTerm( VOID ); 

#endif // __PACKETRECEIVER__H
