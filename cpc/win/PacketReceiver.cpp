
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
* Functions in this file is for the creation and running of the
* thread that manages the receipt of IP packets from the Driver.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "../cpc_prot.h"
#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "PacketReceiver.h"

DWORD PacketReceiverThreadID;
DWORD PacketReceiverThreadParam; 
HANDLE PacketReceiverThreadHandle; 


/*
* Thread function that manages the receipt of packets
*/
DWORD WINAPI PacketReceiverThreadFunc( LPVOID pParam ) 
{ 
	LPTSTR	TmpAdapterName;
	char	*pCommonPacketBuffer;

	// allocate space for common packet buffer
	if((pCommonPacketBuffer = (char *) malloc(MAX_PACKET_SIZE)) == NULL)
	{
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_FATAL,
					"PacketReceiverThread : Could not allocate initial memory");
		LeaveCriticalSection(pLoggingLock);
		return 0;
	}

	// convert ifc name to wide char string
	//TmpAdapterName = _tcsdup( ParaIfaceName );
	TmpAdapterName = _tcsdup( (wchar_t *)&ParaIfaceName );

	// endless loop that gets packets
	while(TRUE) 
	{
		GetAndProcessPackets(TmpAdapterName, pCommonPacketBuffer);
	}
}

void GetAndProcessPackets(LPTSTR TmpAdapterName, char *pPacketData)
{
	HANDLE	LowerHandle;
	ULONG	ActualBufferSize;
	BOOL	Success, PauseALittle;

	PauseALittle = FALSE;

	// hold PT Driver lock to read a packet
	EnterCriticalSection(pPtDriverLock);

	// Open A PassThru Handle Associated With The Lower Adapter
	LowerHandle = PtOpenAdapter(TmpAdapterName);

	if( LowerHandle == INVALID_HANDLE_VALUE )
	{
		PauseALittle = TRUE;

		// release lock
		LeaveCriticalSection(pPtDriverLock);
		
		goto exit_function;
	}

	// get one packet
	Success = PtGetStoredIPv4Packet(LowerHandle, pPacketData, 
									MAX_PACKET_SIZE, &ActualBufferSize);

	PtCloseAdapter( LowerHandle );

	// release lock
	LeaveCriticalSection(pPtDriverLock);

	if(Success)
	{
		ProcessEachPacket(pPacketData, ActualBufferSize);
	}
	else
	{
		PauseALittle = TRUE;
	}

exit_function:

	// wait for a while if no packets were processed
	// last time
	if(PauseALittle)
	{
		Sleep(500);
	}
}

void ProcessEachPacket(char *pPacketData, ULONG PacketSize)
{
	PIPV4_HDR pIPHdr;
	PUDP_HDR pUDPHdr;
	char *pIPPktData;
	int PacketType;
	ULONG IPPacketSize;
	PARP_HDR pARPHdr;

	PacketType = (int) *pPacketData;
	pIPPktData = pPacketData + 1;
	IPPacketSize = PacketSize - 1;

	// check if ARP to the gateway
	if(PacketType == PACKET_TYPE_ALLOWED_ARP)
	{
		pARPHdr = (PARP_HDR) pIPPktData;

		if(pARPHdr->target_ip_addr == ParaGatewayIPAddress.S_un.S_addr)
		{
			ProcessGatewayARP();
		}
		return;
	}

	// check whether IP
	if(IPPacketSize < sizeof(IPV4_HDR)) 
	{
		return;
	}

	pIPHdr = (PIPV4_HDR) pIPPktData;

	if(pIPHdr->ip_verlen != 0x45) 
	{
		return;
	}

	if(pIPHdr->ip_protocol != IPPROTOCOL_UDP)
	{
		// process non UDP packet (TCP, ICMP, etc)
		BuildNormalPacket(PacketType, pIPPktData, IPPacketSize);
		return;
	}

	if(IPPacketSize < (sizeof(IPV4_HDR) + sizeof(UDP_HDR))) 
	{
		return;
	}
	pUDPHdr = (PUDP_HDR) (pIPPktData + sizeof(IPV4_HDR));

	// process AODV packet
	if(ntohs(pUDPHdr->dst_portno) == ROUTING_PORT)
	{
		BuildAODVMsg(pIPPktData);
	} 
	else 
	{
		// process other UDP packet (non AODV)
		BuildNormalPacket(PacketType, pIPPktData, IPPacketSize);
	}
}

void BuildAODVMsg(char *pIPPktData)
{
	unsigned char *pMsgType;
	PIPV4_HDR	pIPHdr;
	PUDP_HDR	pUDPHdr;
	//int i;

	pIPHdr = (PIPV4_HDR) pIPPktData;
	pUDPHdr = (PUDP_HDR) (pIPPktData + sizeof(IPV4_HDR));
	pMsgType = (unsigned char *) pIPPktData + (sizeof(IPV4_HDR) + sizeof(UDP_HDR));

	// if these our my AODV packets, dont consider
	if(pIPHdr->ip_srcaddr == ParaIPAddress.S_un.S_addr)
	{
		return;
	}

	// TODO: Maybe need to redesign the code
	callback_set.prot_callback((char *)pMsgType, 0);

	// never run here
#if 0
	switch(*pMsgType) 
	{
		case AODV_RREQ_MSG_TYPE:
			{
				RREQMessage RREQMsgInternal;
				PRREQ_MSG pRREQMsg;

				pRREQMsg = (PRREQ_MSG) pMsgType;

				RREQMsgInternal.ToIPAddr.S_un.S_addr = pIPHdr->ip_destaddr ;
				RREQMsgInternal.FromIPAddr.S_un.S_addr = pIPHdr->ip_srcaddr ;
				RREQMsgInternal.TTLValue = pIPHdr->ip_ttl;
				RREQMsgInternal.MultiCast = (RREQMsgInternal.FromIPAddr.S_un.S_addr 
											== ParaIPAddressMulticast.S_un.S_addr 
											? TRUE : FALSE);
				strcpy(RREQMsgInternal.pIfaceName, ParaIfaceName);

				RREQMsgInternal.JoinFlag = pRREQMsg->Flags.JoinFlag;
				RREQMsgInternal.RepairFlag = pRREQMsg->Flags.RepairFlag;
				RREQMsgInternal.GratRREPFlag = pRREQMsg->Flags.GratRREPFlag;
				RREQMsgInternal.DestOnlyFlag = pRREQMsg->Flags.DestOnlyFlag;
				RREQMsgInternal.UnknownSeqNumFlag = pRREQMsg->Flags.UnknownSeqNumFlag;
				RREQMsgInternal.HopCount = pRREQMsg->HopCount;
				RREQMsgInternal.RREQID = ntohl(pRREQMsg->RREQID);
				RREQMsgInternal.DestIPAddr.S_un.S_addr = pRREQMsg->DestIPAddr.S_un.S_addr;
				RREQMsgInternal.DestSeqNum = ntohl(pRREQMsg->DestSeqNum);
				RREQMsgInternal.OrigIPAddr.S_un.S_addr = pRREQMsg->OrigIPAddr.S_un.S_addr;
				RREQMsgInternal.OrigSeqNum = ntohl(pRREQMsg->OrigSeqNum);

				// hold lock
				EnterCriticalSection(pWinAODVLock);

				ProcessRREQMsg(&RREQMsgInternal);

				// release lock
				LeaveCriticalSection(pWinAODVLock);

			}
			break;

		case AODV_RREP_MSG_TYPE:
			{
				RREPMessage RREPMsgInternal;
				HELLOMessage HELLOMsgInternal;
				PRREP_MSG pRREPMsg;

				pRREPMsg = (PRREP_MSG) pMsgType;

				// if dest IP and orig IP equal, then hello msg
				if(pRREPMsg->DestIPAddr.S_un.S_addr ==
													pRREPMsg->OrigIPAddr.S_un.S_addr)
				{
					HELLOMsgInternal.ToIPAddr.S_un.S_addr = pIPHdr->ip_destaddr ;
					HELLOMsgInternal.FromIPAddr.S_un.S_addr = pIPHdr->ip_srcaddr ;
					HELLOMsgInternal.TTLValue = pIPHdr->ip_ttl;
					HELLOMsgInternal.MultiCast = (HELLOMsgInternal.FromIPAddr.S_un.S_addr 
												== ParaIPAddressMulticast.S_un.S_addr 
													? TRUE : FALSE);
					strcpy(HELLOMsgInternal.pIfaceName, ParaIfaceName);

					HELLOMsgInternal.RepairFlag = pRREPMsg->Flags.RepairFlag;
					HELLOMsgInternal.AckFlag = pRREPMsg->Flags.AckFlag;
					HELLOMsgInternal.PrefixSize = pRREPMsg->Other.PrefixSize;
					HELLOMsgInternal.HopCount = pRREPMsg->HopCount;
					HELLOMsgInternal.DestIPAddr.S_un.S_addr 
								= pRREPMsg->DestIPAddr.S_un.S_addr;
					HELLOMsgInternal.DestSeqNum = ntohl(pRREPMsg->DestSeqNum);
					HELLOMsgInternal.OrigIPAddr.S_un.S_addr = pRREPMsg->OrigIPAddr.S_un.S_addr;
					HELLOMsgInternal.LifeTime = ntohl(pRREPMsg->LifeTime);

					// hold lock
					EnterCriticalSection(pWinAODVLock);

					ProcessHELLOMsg(&HELLOMsgInternal);

					// release lock
					LeaveCriticalSection(pWinAODVLock);

				}
				else
				{
					RREPMsgInternal.ToIPAddr.S_un.S_addr = pIPHdr->ip_destaddr ;
					RREPMsgInternal.FromIPAddr.S_un.S_addr = pIPHdr->ip_srcaddr ;
					RREPMsgInternal.TTLValue = pIPHdr->ip_ttl;
					RREPMsgInternal.MultiCast = (RREPMsgInternal.FromIPAddr.S_un.S_addr 
												== ParaIPAddressMulticast.S_un.S_addr 
													? TRUE : FALSE);
					strcpy(RREPMsgInternal.pIfaceName, ParaIfaceName);

					RREPMsgInternal.RepairFlag = pRREPMsg->Flags.RepairFlag;
					RREPMsgInternal.AckFlag = pRREPMsg->Flags.AckFlag;
					RREPMsgInternal.PrefixSize = pRREPMsg->Other.PrefixSize;
					RREPMsgInternal.HopCount = pRREPMsg->HopCount;
					RREPMsgInternal.DestIPAddr.S_un.S_addr 
								= pRREPMsg->DestIPAddr.S_un.S_addr;
					RREPMsgInternal.DestSeqNum = ntohl(pRREPMsg->DestSeqNum);
					RREPMsgInternal.OrigIPAddr.S_un.S_addr = pRREPMsg->OrigIPAddr.S_un.S_addr;
					RREPMsgInternal.LifeTime = ntohl(pRREPMsg->LifeTime);

					// hold lock
					EnterCriticalSection(pWinAODVLock);

					ProcessRREPMsg(&RREPMsgInternal);

					// release lock
					LeaveCriticalSection(pWinAODVLock);

				}
			}
			break;
		case AODV_RERR_MSG_TYPE:
			{
				RERRMessage RERRMsgInternal;
				PRERR_MSG pRERRMsg;

				pRERRMsg = (PRERR_MSG) pMsgType;

				RERRMsgInternal.ToIPAddr.S_un.S_addr = pIPHdr->ip_destaddr ;
				RERRMsgInternal.FromIPAddr.S_un.S_addr = pIPHdr->ip_srcaddr ;
				RERRMsgInternal.TTLValue = pIPHdr->ip_ttl;
				RERRMsgInternal.MultiCast = (RERRMsgInternal.FromIPAddr.S_un.S_addr 
												== ParaIPAddressMulticast.S_un.S_addr 
												? TRUE : FALSE);
				strcpy(RERRMsgInternal.pIfaceName, ParaIfaceName);

				RERRMsgInternal.NoDelFlag = pRERRMsg->Flags.NoDelFlag;
				RERRMsgInternal.DestCount = pRERRMsg->DestCount;
				for(i = 0; i < RERRMsgInternal.DestCount && i < MAX_RERR_DESTS; i++)
				{
					RERRMsgInternal.Unreachable[i].DestIPAddr.S_un.S_addr 
										= pRERRMsg->Unreachable[i].DestIPAddr.S_un.S_addr;
					RERRMsgInternal.Unreachable[i].DestSeqNum 
										= ntohl(pRERRMsg->Unreachable[i].DestSeqNum);
				}

				// hold lock
				EnterCriticalSection(pWinAODVLock);

				ProcessRERRMsg(&RERRMsgInternal);				

				// release lock
				LeaveCriticalSection(pWinAODVLock);

			}
			break;
		case AODV_RREPACK_MSG_TYPE:
			{
				RREPACKMessage RREPACKMsgInternal;
				PRREPACK_MSG pRREPACKMsg;


				pRREPACKMsg = (PRREPACK_MSG) pMsgType;

				RREPACKMsgInternal.ToIPAddr.S_un.S_addr = pIPHdr->ip_destaddr ;
				RREPACKMsgInternal.FromIPAddr.S_un.S_addr = pIPHdr->ip_srcaddr ;
				RREPACKMsgInternal.TTLValue = pIPHdr->ip_ttl;
				RREPACKMsgInternal.MultiCast 
						= (RREPACKMsgInternal.FromIPAddr.S_un.S_addr 
								== ParaIPAddressMulticast.S_un.S_addr 
									? TRUE : FALSE);
				strcpy(RREPACKMsgInternal.pIfaceName, ParaIfaceName);

				// hold lock
				EnterCriticalSection(pWinAODVLock);

				ProcessRREPACKMsg(&RREPACKMsgInternal);				

				// release lock
				LeaveCriticalSection(pWinAODVLock);
			}
			break;
		default:
			{
				EnterCriticalSection(pLoggingLock);
				LogMessage(LOGGING_SEVERAITY_ERROR,
						"PacketReceiver : Received a non-AODV message packet");
				LeaveCriticalSection(pLoggingLock);
			}
			break;
	}
#endif

    return; 
} 

void BuildNormalPacket(int PacketType, char *pIPPktData, ULONG PacketSize)
{
	IPPacket IPPkt;
	PIPV4_HDR	pIPHdr;
	char *pNewIPPktData;

	// allocate space for packet & copy contents
	if((pNewIPPktData = (char *) malloc(PacketSize)) == NULL)
	{
		return;
	}
	memcpy(pNewIPPktData, pIPPktData, PacketSize);

	pIPHdr = (PIPV4_HDR) pNewIPPktData;

	IPPkt.TotalPacketSize = PacketSize;
	IPPkt.pIPPacket = pNewIPPktData;
	IPPkt.pIPHeader = pIPHdr;
	IPPkt.HeaderSize = sizeof(IPV4_HDR);
	IPPkt.pIPData = (pIPHdr + sizeof(IPV4_HDR));
	IPPkt.DataSize = PacketSize - sizeof(IPV4_HDR);
	IPPkt.FromIPAddr.S_un.S_addr = pIPHdr->ip_srcaddr;
	IPPkt.ToIPAddr.S_un.S_addr = pIPHdr->ip_destaddr;
	IPPkt.Protocol = pIPHdr->ip_protocol;
	IPPkt.PacketTypeFlag = PacketType;

	// hold lock
	EnterCriticalSection(pWinAODVLock);

	ProcessNormalPacket(&IPPkt);

	// release lock
	LeaveCriticalSection(pWinAODVLock);

	return;
}

/*
* Initializes the packet receive thread environment and starts the 
* thread
*/
VOID PacketReceiverInit( VOID ) 
{ 

	PacketReceiverThreadParam = 1;
    PacketReceiverThreadHandle = CreateThread(NULL, 0, PacketReceiverThreadFunc,
							&PacketReceiverThreadParam, 0,
							&PacketReceiverThreadID);
 
   // Check the return value for success. 
   if (PacketReceiverThreadHandle == NULL) 
   {
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_FATAL,
							"PacketReceiverThread : Could not start the thread");
		LeaveCriticalSection(pLoggingLock);
   }
   else 
   {
		EnterCriticalSection(pLoggingLock);
		LogMessage(LOGGING_SEVERAITY_INFO,
							"PacketReceiverThread : Thread started successfully");
		LeaveCriticalSection(pLoggingLock);
   }
}

/*
* Terminates the packet receive thread
*/
VOID PacketReceiverTerm( VOID ) 
{
	EnterCriticalSection(pLoggingLock);
	LogMessage(LOGGING_SEVERAITY_INFO,
						"PacketReceiverThread : Thread being terminated");
	LeaveCriticalSection(pLoggingLock);

	CloseHandle(PacketReceiverThreadHandle);
}
