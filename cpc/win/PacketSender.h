
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
* PacketSender.cpp
*
* @author : Asanga Udugama
* @date : 11-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#ifndef __PACKETSENDER__H
#define __PACKETSENDER__H


extern int ParaExecutionMode;
extern int ParaIPVersion;
extern IN_ADDR ParaIPAddress;
extern char ParaIfaceName[MAX_IFACE_SIZE];
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

extern CRITICAL_SECTION LoggingLock;
extern LPCRITICAL_SECTION pLoggingLock;
extern void LogMessage(int LoggingLevel, char *pPrintStr);
extern void DumpCharArray(char *pCharArray, int ArrayLen);

BOOL PacketSenderInit();
BOOL SendRREQ(PRREQMessage pRREQMsg);
BOOL SendRREP(PRREPMessage pRREPMsg);
BOOL SendRERR(PRERRMessage pRERRMsg);
BOOL SendRREPACK(PRREPACKMessage pRREPACKMsg);
BOOL SendHELLO(PHELLOMessage pHELLOMsg);
BOOL SendIPPacket(PIPPacket pIPPacket);
BOOL BuildIPHeader(void *pIPBuffer, IN_ADDR FromIPAddr, IN_ADDR ToIPAddr, 
					unsigned short int TTLValue, 
					int IPPktLen);
BOOL BuildUDPHeader(void *pUDPBuffer, unsigned short int SrcPortNum,
					unsigned short int DestPortNum, 
					int UDPPktLen);
USHORT GetIPChecksum(USHORT *pBuffer, int Size);
int	SendPacketOut(void *pPktBuffer, int PktLen, IN_ADDR ToIPAddr, 
						unsigned short int PortNum);
BOOL PacketSenderTerm(); 


#endif // __PACKETSENDER__H

