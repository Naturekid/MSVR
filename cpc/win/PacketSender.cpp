
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
* Functions in this file provide services for sending IP packets
* which include AODV messages as well as buffered IP packets.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "PacketSender.h"

SOCKET PacketSendSocket; // IP packet sending socket
WSADATA WinSockData;

/*
* Initialize packet sender 
*/
BOOL PacketSenderInit() 
{
	int RtnCode, OptVal;

	PacketSendSocket = INVALID_SOCKET;

	RtnCode = WSAStartup(MAKEWORD(2,2), &WinSockData);
    if (RtnCode != 0)
    {
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"PacketSender : Init (using WSAStartup()) failed");
		LeaveCriticalSection(pLoggingLock);
		return ( FALSE );
	}

	PacketSendSocket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if(PacketSendSocket == INVALID_SOCKET)
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"PacketSender : Init (using socket()) failed");
		LeaveCriticalSection(pLoggingLock);
		return ( FALSE );
	}

	// set socket options to control the IP packet header
	OptVal = 1;
	RtnCode = setsockopt(PacketSendSocket, IPPROTO_IP, IP_HDRINCL, 
						(char *)&OptVal, sizeof(OptVal));
	if (RtnCode == SOCKET_ERROR)
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"PacketSender : Init (using setsockopt()) failed");
		LeaveCriticalSection(pLoggingLock);
		return ( FALSE );
	}

	// set socket option to broadcast
	OptVal = 1;
	RtnCode = setsockopt(PacketSendSocket, SOL_SOCKET, SO_BROADCAST, 
						(char *)&OptVal, sizeof(OptVal));
	if (RtnCode == SOCKET_ERROR)
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"PacketSender : Init (using setsockopt()) failed");
		LeaveCriticalSection(pLoggingLock);
		return ( FALSE );
	}

	return ( TRUE );
}

/*
* Function to send a RREQ message given in its internal
* representation.
*/
BOOL SendRREQ(PRREQMessage pRREQMsg)
{
	int RtnCode, PktSize, xx;
	unsigned char *pPktBuffer;
	PRREQ_MSG pAODVMsg;

	PktSize = sizeof(IPV4_HDR) + sizeof(UDP_HDR) + AODV_RREQ_MSG_SIZE;
    pPktBuffer = (unsigned char *)malloc(PktSize);
	if(pPktBuffer == NULL) 
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"PacketSender : Could not allocate memory to send RREQ");
		LeaveCriticalSection(pLoggingLock);
		return ( FALSE );
	}
	memset(pPktBuffer, 0, PktSize);

	pAODVMsg = (PRREQ_MSG) (pPktBuffer + sizeof(IPV4_HDR) + sizeof(UDP_HDR));
	xx = sizeof(pAODVMsg->Flags);
	pAODVMsg->Type = AODV_RREQ_MSG_TYPE;
	pAODVMsg->Flags.JoinFlag = pRREQMsg->JoinFlag; 
	pAODVMsg->Flags.RepairFlag = pRREQMsg->RepairFlag; 
	pAODVMsg->Flags.GratRREPFlag = pRREQMsg->GratRREPFlag;
	pAODVMsg->Flags.DestOnlyFlag = pRREQMsg->DestOnlyFlag; 
	pAODVMsg->Flags.UnknownSeqNumFlag = pRREQMsg->UnknownSeqNumFlag;
	pAODVMsg->HopCount = pRREQMsg->HopCount;
	pAODVMsg->RREQID = htonl(pRREQMsg->RREQID);
	pAODVMsg->DestIPAddr.S_un.S_addr = pRREQMsg->DestIPAddr.S_un.S_addr;
	pAODVMsg->DestSeqNum = htonl(pRREQMsg->DestSeqNum);
	pAODVMsg->OrigIPAddr.S_un.S_addr = pRREQMsg->OrigIPAddr.S_un.S_addr;
	pAODVMsg->OrigSeqNum = htonl(pRREQMsg->OrigSeqNum);

	BuildIPHeader(pPktBuffer, pRREQMsg->FromIPAddr, pRREQMsg->ToIPAddr, 
					pRREQMsg->TTLValue, 
					sizeof(IPV4_HDR) + sizeof(UDP_HDR) + AODV_RREQ_MSG_SIZE);

	BuildUDPHeader(pPktBuffer + sizeof(IPV4_HDR), AODV_PORT, AODV_PORT, 
					sizeof(UDP_HDR) + AODV_RREQ_MSG_SIZE);

	RtnCode = SendPacketOut(pPktBuffer, 
				sizeof(IPV4_HDR) + sizeof(UDP_HDR) + AODV_RREQ_MSG_SIZE,
				pRREQMsg->ToIPAddr, AODV_PORT);

	if(RtnCode == SOCKET_ERROR) 
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"PacketSender : Sending RREQ (using sendto()) failed");
		LeaveCriticalSection(pLoggingLock);
		free(pPktBuffer); 

		return ( FALSE );
	}

	free(pPktBuffer); 
	return ( TRUE );
}

/*
* Function to send a RREP message given in its internal
* representation.
*/
BOOL SendRREP(PRREPMessage pRREPMsg)
{
	int RtnCode, PktSize;
	unsigned char *pPktBuffer;
	PRREP_MSG pAODVMsg;

	PktSize = sizeof(IPV4_HDR) + sizeof(UDP_HDR) + AODV_RREP_MSG_SIZE;
    pPktBuffer = (unsigned char *)malloc(PktSize);
	if(pPktBuffer == NULL) 
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"PacketSender : Could not allocate memory to send RREP");
		LeaveCriticalSection(pLoggingLock);
		return ( FALSE );
	}
	memset(pPktBuffer, 0, PktSize);

	pAODVMsg = (PRREP_MSG) (pPktBuffer + sizeof(IPV4_HDR) + sizeof(UDP_HDR));
	pAODVMsg->Type = AODV_RREP_MSG_TYPE;
	pAODVMsg->Flags.RepairFlag = pRREPMsg->RepairFlag; 
	pAODVMsg->Flags.AckFlag = pRREPMsg->AckFlag;
	pAODVMsg->Other.PrefixSize = pRREPMsg->PrefixSize;
	pAODVMsg->HopCount = pRREPMsg->HopCount;

	pAODVMsg->DestIPAddr.S_un.S_addr = pRREPMsg->DestIPAddr.S_un.S_addr;
	pAODVMsg->DestSeqNum = htonl(pRREPMsg->DestSeqNum);
	pAODVMsg->OrigIPAddr.S_un.S_addr = pRREPMsg->OrigIPAddr.S_un.S_addr;
	pAODVMsg->LifeTime = htonl(pRREPMsg->LifeTime);

	BuildIPHeader(pPktBuffer, pRREPMsg->FromIPAddr, pRREPMsg->ToIPAddr, 
					pRREPMsg->TTLValue, 
					sizeof(IPV4_HDR) + sizeof(UDP_HDR) + AODV_RREP_MSG_SIZE);

	BuildUDPHeader(pPktBuffer + sizeof(IPV4_HDR), AODV_PORT, AODV_PORT, 
					sizeof(UDP_HDR) + AODV_RREP_MSG_SIZE);

	RtnCode = SendPacketOut(pPktBuffer, 
				sizeof(IPV4_HDR) + sizeof(UDP_HDR) + AODV_RREP_MSG_SIZE,
				pRREPMsg->ToIPAddr, AODV_PORT);

	if(RtnCode == SOCKET_ERROR) 
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"PacketSender : Sending RREP (using sendto()) failed");
		LeaveCriticalSection(pLoggingLock);
		free(pPktBuffer); 

		return ( FALSE );
	}

	free(pPktBuffer); 
	return ( TRUE );
}

/*
* Function to send a RERR message given in its internal
* representation.
*/
BOOL SendRERR(PRERRMessage pRERRMsg)
{
	int RtnCode;
	unsigned char *pPktBuffer;
	PRERR_MSG pAODVMsg;
	int TotalPacketLen, UDPPacketLen, i;

	TotalPacketLen = sizeof(IPV4_HDR) + sizeof(UDP_HDR) + AODV_RERR_MSG_SIZE 
						+ (pRERRMsg->DestCount * sizeof(pRERRMsg->Unreachable[0])); 
	UDPPacketLen = sizeof(UDP_HDR) + AODV_RERR_MSG_SIZE 
						+ (pRERRMsg->DestCount * sizeof(pRERRMsg->Unreachable[0]));
    pPktBuffer = (unsigned char *) malloc(TotalPacketLen);
	if(pPktBuffer == NULL) 
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"PacketSender : Could not allocate memory to send RERR");
		LeaveCriticalSection(pLoggingLock);
		return ( FALSE );
	}
	memset(pPktBuffer, 0, TotalPacketLen);

	pAODVMsg = (PRERR_MSG) (pPktBuffer + sizeof(IPV4_HDR) + sizeof(UDP_HDR));
	pAODVMsg->Type = AODV_RERR_MSG_TYPE;
	pAODVMsg->Flags.NoDelFlag = pRERRMsg->NoDelFlag; 
	pAODVMsg->DestCount = pRERRMsg->DestCount;

	for(i = 0; i < pRERRMsg->DestCount; i++)
	{
		pAODVMsg->Unreachable[i].DestIPAddr.S_un.S_addr =
				pRERRMsg->Unreachable[i].DestIPAddr.S_un.S_addr;
		pAODVMsg->Unreachable[i].DestSeqNum =
				htonl(pRERRMsg->Unreachable[i].DestSeqNum);
	}

	BuildIPHeader(pPktBuffer, pRERRMsg->FromIPAddr, pRERRMsg->ToIPAddr, 
					pRERRMsg->TTLValue, TotalPacketLen);

	BuildUDPHeader(pPktBuffer + sizeof(IPV4_HDR), AODV_PORT, AODV_PORT, 
					UDPPacketLen);

	RtnCode = SendPacketOut(pPktBuffer, TotalPacketLen,
							pRERRMsg->ToIPAddr, AODV_PORT);

	if(RtnCode == SOCKET_ERROR) 
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"PacketSender : Sending RERR (using sendto()) failed");
		LeaveCriticalSection(pLoggingLock);
		free(pPktBuffer); 

		return ( FALSE );
	}

	free(pPktBuffer); 

	return ( TRUE );
}

/*
* Function to send a RREPACK message given in its internal
* representation.
*/
BOOL SendRREPACK(PRREPACKMessage pRREPACKMsg)
{
	int RtnCode, PktSize;
	unsigned char *pPktBuffer;
	PRREPACK_MSG pAODVMsg;

	PktSize = sizeof(IPV4_HDR) + sizeof(UDP_HDR) + AODV_RREPACK_MSG_SIZE;
    pPktBuffer = (unsigned char *)malloc(PktSize);
	if(pPktBuffer == NULL) 
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"PacketSender : Could not allocate memory to send RREPACK");
		LeaveCriticalSection(pLoggingLock);
		return ( FALSE );
	}
	memset(pPktBuffer, 0, PktSize);

	pAODVMsg = (PRREPACK_MSG) (pPktBuffer + sizeof(IPV4_HDR) + sizeof(UDP_HDR));
	pAODVMsg->Type = AODV_RREPACK_MSG_TYPE;

	BuildIPHeader(pPktBuffer, pRREPACKMsg->FromIPAddr, pRREPACKMsg->ToIPAddr, 
					pRREPACKMsg->TTLValue, 
					sizeof(IPV4_HDR) + sizeof(UDP_HDR) + AODV_RREPACK_MSG_SIZE);

	BuildUDPHeader(pPktBuffer + sizeof(IPV4_HDR), AODV_PORT, AODV_PORT, 
					sizeof(UDP_HDR) + AODV_RREPACK_MSG_SIZE);

	RtnCode = SendPacketOut(pPktBuffer, 
				sizeof(IPV4_HDR) + sizeof(UDP_HDR) + AODV_RREPACK_MSG_SIZE,
				pRREPACKMsg->ToIPAddr, AODV_PORT);

	if(RtnCode == SOCKET_ERROR) 
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"PacketSender : Sending RREPACK (using sendto()) failed");
		LeaveCriticalSection(pLoggingLock);
		free(pPktBuffer); 

		return ( FALSE );
	}

	free(pPktBuffer); 

	return ( TRUE );
}

/*
* Function to send a HELLO message given in its internal
* representation.
*/
BOOL SendHELLO(PHELLOMessage pHELLOMsg)
{
	int RtnCode, PktSize;
	unsigned char *pPktBuffer;
	PRREP_MSG pAODVMsg;

	PktSize = sizeof(IPV4_HDR) + sizeof(UDP_HDR) + AODV_RREP_MSG_SIZE;
    pPktBuffer = (unsigned char *) malloc(PktSize);
	if(pPktBuffer == NULL) 
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"PacketSender : Could not allocate memory to send RREP");
		LeaveCriticalSection(pLoggingLock);
		return ( FALSE );
	}
	memset(pPktBuffer, 0, PktSize);

	pAODVMsg = (PRREP_MSG) (pPktBuffer + sizeof(IPV4_HDR) + sizeof(UDP_HDR));
	pAODVMsg->Type = AODV_RREP_MSG_TYPE;
	pAODVMsg->Flags.RepairFlag = pHELLOMsg->RepairFlag; 
	pAODVMsg->Flags.AckFlag = pHELLOMsg->AckFlag;
	pAODVMsg->Other.PrefixSize = pHELLOMsg->PrefixSize;
	pAODVMsg->HopCount = pHELLOMsg->HopCount;

	pAODVMsg->DestIPAddr.S_un.S_addr = pHELLOMsg->DestIPAddr.S_un.S_addr;
	pAODVMsg->DestSeqNum = htonl(pHELLOMsg->DestSeqNum);
	pAODVMsg->OrigIPAddr.S_un.S_addr = pHELLOMsg->OrigIPAddr.S_un.S_addr;
	pAODVMsg->LifeTime = htonl(pHELLOMsg->LifeTime);

	BuildIPHeader(pPktBuffer, pHELLOMsg->FromIPAddr, pHELLOMsg->ToIPAddr, 
					pHELLOMsg->TTLValue, 
					sizeof(IPV4_HDR) + sizeof(UDP_HDR) + AODV_RREP_MSG_SIZE);

	BuildUDPHeader(pPktBuffer + sizeof(IPV4_HDR), AODV_PORT, AODV_PORT, 
					sizeof(UDP_HDR) + AODV_RREP_MSG_SIZE);

	RtnCode = SendPacketOut(pPktBuffer, 
				sizeof(IPV4_HDR) + sizeof(UDP_HDR) + AODV_RREP_MSG_SIZE,
				pHELLOMsg->ToIPAddr, AODV_PORT);

	if(RtnCode == SOCKET_ERROR) 
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"PacketSender : Sending RREP (using sendto()) failed");
		LeaveCriticalSection(pLoggingLock);
		free(pPktBuffer); 

		return ( FALSE );
	}

	free(pPktBuffer); 

	return ( TRUE );
}

/*
* Function to send a IP packet that was buffered when a route
* discovery was in progress.
*/
BOOL SendIPPacket(PIPPacket pIPPkt)
{
	int RtnCode;

	RtnCode = SendPacketOut((void *) pIPPkt->pIPPacket, pIPPkt->TotalPacketSize,
				pIPPkt->ToIPAddr, 0);

	if(RtnCode == SOCKET_ERROR) 
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_ERROR,
					"PacketSender : Sending a buffered packet (using sendto()) failed");
		LeaveCriticalSection(pLoggingLock);
		free(pIPPkt->pIPPacket);
		return ( FALSE );
	}

	free(pIPPkt->pIPPacket);
	return ( TRUE );
}

/*
* Function to build the IP header of a packet
*/
BOOL BuildIPHeader(void *pIPBuffer, IN_ADDR FromIPAddr, IN_ADDR ToIPAddr, 
					unsigned short int TTLValue, 
					int IPPktLen)
{
    PIPV4_HDR    pIPHeader;

    pIPHeader = (PIPV4_HDR) pIPBuffer;

    pIPHeader->ip_verlen      = (4 << 4) | (sizeof(IPV4_HDR) / sizeof(unsigned long));
    pIPHeader->ip_tos         = 0;
    pIPHeader->ip_totallength = htons((u_short) IPPktLen);
    pIPHeader->ip_id          = 0;
    pIPHeader->ip_offset      = htons((USHORT) 0x4000);
    pIPHeader->ip_ttl         = (unsigned char) TTLValue;
    pIPHeader->ip_protocol    = IPPROTO_UDP;
    pIPHeader->ip_checksum    = 0;
	pIPHeader->ip_srcaddr     = FromIPAddr.S_un.S_addr;
	pIPHeader->ip_destaddr    = ToIPAddr.S_un.S_addr;

    pIPHeader->ip_checksum    = GetIPChecksum((USHORT *)pIPHeader, 
												sizeof(IPV4_HDR));

	return (TRUE);
}

/*
* Function to build the UDP header of a packet
*/
BOOL BuildUDPHeader(void *pUDPBuffer, unsigned short int SrcPortNum,
					unsigned short int DestPortNum, 
					int UDPPktLen)
{
    PUDP_HDR pUDPHeader;
	
	pUDPHeader = (PUDP_HDR) pUDPBuffer;
	
	pUDPHeader->src_portno = htons(SrcPortNum);
	pUDPHeader->dst_portno = htons(DestPortNum);
    pUDPHeader->udp_length = htons((u_short) UDPPktLen);
    pUDPHeader->udp_checksum = 0; // checksum 0 might not work ???? 

	return (TRUE);
}

/*
* Function to compute the checksum used in IP header.
*/
USHORT GetIPChecksum(USHORT *pBuffer, int Size)
{
    unsigned long ChkSum;
	
	ChkSum = 0;
    while (Size > 1)
    {
        ChkSum += (*pBuffer);
		pBuffer++;
        Size  -= sizeof(USHORT);   
    }
    // If the buffer was not a multiple of 16-bits, add the last byte
    if (Size)
    {
        ChkSum += *((UCHAR *) pBuffer);   
    }
    // Add the low order 16-bits to the high order 16-bits
    ChkSum = (ChkSum >> 16) + (ChkSum & 0xffff);
    ChkSum += (ChkSum >>16); 

    // Take the 1's complement
    return (USHORT)(~ChkSum); 
}

/*
* Function to send an IP packet out
*/
int	SendPacketOut(void *pPktBuffer, int PktLen, IN_ADDR ToIPAddr, 
						unsigned short int PortNum)
{
	int RtnCode, xx;
	struct sockaddr_in DestIPSockAddr;

	//DumpCharArray((char *)pPktBuffer, PktLen);

	memset(&DestIPSockAddr, 0, sizeof(DestIPSockAddr));
	DestIPSockAddr.sin_family=AF_INET;
	memcpy(&DestIPSockAddr.sin_addr, &ToIPAddr, sizeof(DestIPSockAddr.sin_addr));
	DestIPSockAddr.sin_port = PortNum;
	RtnCode = sendto(PacketSendSocket, (char *) pPktBuffer, PktLen, 0,
							(struct sockaddr *)&DestIPSockAddr,
							sizeof(DestIPSockAddr));
	if(RtnCode == SOCKET_ERROR) 
	{
		xx = WSAGetLastError();
	}
	return (RtnCode);
}

/*
* Terminate packet sender 
*/
BOOL PacketSenderTerm() 
{
	int RtnCode;

	RtnCode = closesocket(PacketSendSocket);
	if(RtnCode == SOCKET_ERROR)
	{
		return ( FALSE );
	}

	WSACleanup();

	return ( TRUE );
}
