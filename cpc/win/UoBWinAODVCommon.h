
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
* Header file that provides the common definitions 
* used by all source files.
*
* @author : Asanga Udugama
* @date : 11-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#ifndef __UOBWINAODVCOMMON__H
#define __UOBWINAODVCOMMON__H

#pragma pack(push,1)


#define CR_CHAR							0x0D
#define LF_CHAR							0x0A

#define EXECUTION_MODE_CONSOLE			0
#define EXECUTION_MODE_GUI				1

#define IP_VERSION_4					4
#define IP_VERSION_6					6

#define YES								1
#define NO								0

// loging mode
#define LOGGING_MODE_INITIAL			0
#define LOGGING_MODE_CONSOLE			1
#define LOGGING_MODE_FILE				2

// logging level literals
#define LOGGING_SEVERAITY_NONE			0
#define LOGGING_SEVERAITY_FATAL			1
#define LOGGING_SEVERAITY_ERROR			2  // includes 1
#define LOGGING_SEVERAITY_INFO			3  // includes 1, 2
#define LOGGING_SEVERAITY_MORE_INFO		4  // includes 1, 2, 3
#define LOGGING_SEVERAITY_MIN			1
#define LOGGING_SEVERAITY_MAX			4



#define RERR_SEND_MODE_UNICAST			0
#define RERR_SEND_MODE_MULTICAST		1

#define DELETE_PERIOD_MODE_HELLO		0
#define DELETE_PERIOD_MODE_LINKLAYER	1

#define ROUTE_DISCOVERY_MODE_NON_ERS	0
#define ROUTE_DISCOVERY_MODE_ERS		1


#define MAX_IFACE_SIZE					512

#define MAX_RERR_DESTS					10
#define MAX_IFACE_NAME_LENGTH			512
#define MAX_LOG_FILE_NAME_SIZE			512

#define UPDATE_TYPE_NOMAL				1
#define UPDATE_TYPE_DELETE				2

#define HOST_MASK						((ULONG) 0xffffffff)

#define MAX_PACKET_SIZE					4096

typedef ULONG MILISECTIME;

// pack to the 1st byte
#include <pshpack1.h>

typedef
struct _RREQ_MSG
{
	unsigned char Type; // always equal to 1
	struct _flags {
		unsigned char JoinFlag:				1;
		unsigned char RepairFlag:			1;
		unsigned char GratRREPFlag:			1;
		unsigned char DestOnlyFlag:			1;
		unsigned char UnknownSeqNumFlag:	1;
		unsigned char Unused1:				3;
	} Flags;
	unsigned char Unused2;
	unsigned char HopCount;
	ULONG RREQID;
	IN_ADDR DestIPAddr;
	ULONG DestSeqNum;
	IN_ADDR OrigIPAddr;
	ULONG OrigSeqNum;
}
	RREQ_MSG, *PRREQ_MSG;

typedef
struct _RREP_MSG
{
	unsigned char Type; // always 2
	struct _flags {
		unsigned char RepairFlag:	1;
		unsigned char AckFlag:		1;
		unsigned char Unused2:		6;
	} Flags;
	struct _other {
		unsigned char Unused2:		3;
		unsigned char PrefixSize:	5;
	} Other;
	unsigned char HopCount;
	IN_ADDR DestIPAddr;
	ULONG DestSeqNum;
	IN_ADDR OrigIPAddr;
	ULONG LifeTime;
}
	RREP_MSG, *PRREP_MSG;

typedef
struct _RERR_MSG
{
	unsigned char Type; // always 3
	struct _flags {
		unsigned char NoDelFlag:	1;
		unsigned char Unused2:		7;
	} Flags;
	unsigned char Unused2;
	unsigned char DestCount;
	struct _unreachable {
		IN_ADDR DestIPAddr;
		ULONG DestSeqNum;
	} Unreachable[MAX_RERR_DESTS];
}
	RERR_MSG, *PRERR_MSG;

typedef
struct _RREPACK_MSG
{
	unsigned char Type; // always 4
}
	RREPACK_MSG, *PRREPACK_MSG;

//
// IPv4 Header (without any IP options)
//
typedef 
struct ip_hdr
{
    unsigned char  ip_verlen;        // 4-bit IPv4 version
                                     // 4-bit header length (in 32-bit words)
    unsigned char  ip_tos;           // IP type of service
    unsigned short ip_totallength;   // Total length
    unsigned short ip_id;            // Unique identifier 
    unsigned short ip_offset;        // Fragment offset field
    unsigned char  ip_ttl;           // Time to live
    unsigned char  ip_protocol;      // Protocol(TCP,UDP etc)
    unsigned short ip_checksum;      // IP checksum
    unsigned int   ip_srcaddr;       // Source address
    unsigned int   ip_destaddr;      // Source address
} IPV4_HDR, *PIPV4_HDR, FAR * LPIPV4_HDR;

//
// Define the UDP header 
//
typedef 
struct udp_hdr
{
    unsigned short src_portno;       // Source port no.
    unsigned short dst_portno;       // Dest. port no.
    unsigned short udp_length;       // Udp packet length
    unsigned short udp_checksum;     // Udp checksum
} UDP_HDR, *PUDP_HDR;

//
// Define the ARP header 
//
typedef 
struct arp_hdr
{
    unsigned short hw_type;
    unsigned short proto_type;
	unsigned char hw_size;
	unsigned char proto_size;
    unsigned short request_type;
	unsigned char sender_mac_addr[6];
	unsigned int sender_ip_addr;
	unsigned char target_mac_addr[6];
	unsigned int target_ip_addr;
} ARP_HDR, *PARP_HDR;

#include <poppack.h>


//-------------------------------------------------
// structure for a single buffered IP Packet

#define PACKET_TYPE_DISALLOWED	1
#define PACKET_TYPE_ALLOWED		2
#define PACKET_TYPE_ALLOWED_ARP	3

typedef
struct _IPPacket 
{
	ULONG TotalPacketSize;
	void *pIPPacket;
	PIPV4_HDR pIPHeader;
	ULONG HeaderSize;
	void *pIPData;
	ULONG DataSize;
	IN_ADDR FromIPAddr;
	IN_ADDR ToIPAddr;
	unsigned char Protocol;
	ULONG PacketTypeFlag;
} 
	IPPacket, *PIPPacket;


// structure for a single buffered IP packet List entry
typedef
struct _IPPacketListEntry 
{
	PIPPacket pIPPacketValue;
	ULONG IPPacketCount;
	struct _IPPacketListEntry *pNext;
	struct _IPPacketListEntry *pPrev;
} 
	IPPacketListEntry, *PIPPacketListEntry;



//-------------------------------------------------
// messages
// RREQ Message
typedef
struct _RREQMessage
{
	IN_ADDR ToIPAddr;
	IN_ADDR FromIPAddr;
	unsigned char TTLValue;
	BOOL MultiCast;
	char pIfaceName[MAX_IFACE_NAME_LENGTH];

	BOOL JoinFlag;
	BOOL RepairFlag;
	BOOL GratRREPFlag;
	BOOL DestOnlyFlag;
	BOOL UnknownSeqNumFlag;
	unsigned char HopCount;
	ULONG RREQID;
	IN_ADDR DestIPAddr;
	ULONG DestSeqNum;
	IN_ADDR OrigIPAddr;
	ULONG OrigSeqNum;
}
	RREQMessage, *PRREQMessage;

// RREP Message
typedef
struct _RREPMessage
{
	IN_ADDR ToIPAddr;
	IN_ADDR FromIPAddr;
	unsigned char TTLValue;
	BOOL MultiCast;
	char pIfaceName[MAX_IFACE_NAME_LENGTH];

	BOOL RepairFlag;
	BOOL AckFlag;
	unsigned char PrefixSize;
	unsigned char HopCount;
	IN_ADDR DestIPAddr;
	ULONG DestSeqNum;
	IN_ADDR OrigIPAddr;
	ULONG LifeTime;
}
	RREPMessage, *PRREPMessage;

// RERR Message
typedef
struct _RERRMessage
{
	IN_ADDR ToIPAddr;
	IN_ADDR FromIPAddr;
	unsigned char TTLValue;
	BOOL MultiCast;
	char pIfaceName[MAX_IFACE_NAME_LENGTH];

	BOOL NoDelFlag;
	unsigned char DestCount;
	struct _unreachable {
		IN_ADDR DestIPAddr;
		ULONG DestSeqNum;
	} Unreachable[MAX_RERR_DESTS];
}
	RERRMessage, *PRERRMessage;

// RREPAck Message
typedef
struct _RREPACKMessage
{
	IN_ADDR ToIPAddr;
	IN_ADDR FromIPAddr;
	unsigned char TTLValue;
	BOOL MultiCast;
	char pIfaceName[MAX_IFACE_NAME_LENGTH];
}
	RREPACKMessage, *PRREPACKMessage;

// HELLO Message
typedef
struct _HELLOMessage
{
	IN_ADDR ToIPAddr;
	IN_ADDR FromIPAddr;
	unsigned char TTLValue;
	BOOL MultiCast;
	char pIfaceName[MAX_IFACE_NAME_LENGTH];

	BOOL RepairFlag;
	BOOL AckFlag;
	unsigned char PrefixSize;
	unsigned char HopCount;
	IN_ADDR DestIPAddr;
	ULONG DestSeqNum;
	IN_ADDR OrigIPAddr;
	ULONG LifeTime;
}
	HELLOMessage, *PHELLOMessage;

//-------------------------------------------------

// AODV messaging literals
#define AODV_PORT				654

#define AODV_RREQ_MSG_TYPE		1
#define AODV_RREQ_MSG_SIZE		24

#define AODV_RREP_MSG_TYPE		2
#define AODV_RREP_MSG_SIZE		20

#define AODV_RERR_MSG_TYPE		3
#define AODV_RERR_MSG_SIZE		4	// the balance length to be computed

#define AODV_RREPACK_MSG_TYPE	4
#define AODV_RREPACK_MSG_SIZE	2


// possible status values of ValidDestSeqNumFlag
#define DEST_SEQ_FLAG_INVALID					0
#define DEST_SEQ_FLAG_VALID						1

// possible status values of RouteStatusFlag
#define ROUTE_STATUS_FLAG_INVALID				0
#define ROUTE_STATUS_FLAG_VALID					1
#define ROUTE_STATUS_FLAG_REPAIRABLE			2
#define ROUTE_STATUS_FLAG_BEING_REPAIRED		3

// values for ActiveThreadTypeFlag
#define ACTIVE_THREAD_NO_MINDER					0
#define ACTIVE_THREAD_ROUTE_LIFETIME_MINDER		1
#define ACTIVE_THREAD_DELETE_PERIOD_MINDER		2

// values for RouteType
#define ROUTE_TYPE_AODV							0
#define ROUTE_TYPE_STATIC						99

//-------------------------------------------------
// structure for a single Precursor entry
typedef
struct _PrecursorEntry
{
	IN_ADDR DestIPAddr;
} 
	PrecursorEntry, *PPrecursorEntry;
	
// structure for a single Precursor List entry
typedef
struct _PrecursorListEntry
{
	PPrecursorEntry pPrecursorValue;
	ULONG PrecursorCount; // only valid for the head entry
	struct _PrecursorListEntry *pNext;
	struct _PrecursorListEntry *pPrev;
} 
	PrecursorListEntry, *PPrecursorListEntry;

// structure for a single route entry
typedef
struct _RouteEntry 
{
	IN_ADDR DestIPAddr;
	IN_ADDR DestIPAddrMask;
	ULONG DestSeqNum;
	ULONG ValidDestSeqNumFlag;
	ULONG RouteStatusFlag;
	char pIfaceName[MAX_IFACE_NAME_LENGTH];
	unsigned char HopCount;
	IN_ADDR NextHopIPAddr;
	PrecursorListEntry PrecursorListHead;
	MILISECTIME ExpiryTime;
	//HANDLE ActiveThreadHandle;
	ULONG ActiveThreadType; 
	BOOL KernelRouteSet; // true if route entry set in the kernel, else false

	// values used to manage HELLO receipts
	DWORD HelloReceiptThreadHandle;
	MILISECTIME NextHelloReceiveTime;

	// used to status whether AODV route or static route
	ULONG RouteType;
} 
	RouteEntry, *PRouteEntry;



// structure for a single Precursor List entry
typedef
struct _RouteListEntry
{
	PRouteEntry pRouteValue;
	struct _RouteListEntry *pNext;
	struct _RouteListEntry *pPrev;
} 
	RouteListEntry, *PRouteListEntry;


//-------------------------------------------------
// structure for a single route expiry entry
typedef
struct _RouteExpiryEntry
{
	IN_ADDR DestIPAddr;
	MILISECTIME ExpiryTime;
} 
	RouteExpiryEntry, *PRouteExpiryEntry;

// structure for a single route expiry List entry
typedef
struct _RouteExpiryListEntry
{
	PRouteExpiryEntry pRouteExpiryValue;
	struct _RouteExpiryListEntry *pNext;
	struct _RouteExpiryListEntry *pPrev;
} 
	RouteExpiryListEntry, *PRouteExpiryListEntry;

//-------------------------------------------------
// structure for a single delete expiry entry
typedef
struct _DeleteExpiryEntry
{
	IN_ADDR DestIPAddr;
	MILISECTIME ExpiryTime;
} 
	DeleteExpiryEntry, *PDeleteExpiryEntry;

// structure for a single delete expiry List entry
typedef
struct _DeleteExpiryListEntry
{
	PDeleteExpiryEntry pDeleteExpiryValue;
	struct _DeleteExpiryListEntry *pNext;
	struct _DeleteExpiryListEntry *pPrev;
} 
	DeleteExpiryListEntry, *PDeleteExpiryListEntry;

//-------------------------------------------------
// structure for a single hello expiry entry
typedef
struct _HelloExpiryEntry
{
	IN_ADDR DestIPAddr;
	MILISECTIME ExpiryTime;
} 
	HelloExpiryEntry, *PHelloExpiryEntry;

// structure for a single hello expiry List entry
typedef
struct _HelloExpiryListEntry
{
	PHelloExpiryEntry pHelloExpiryValue;
	struct _HelloExpiryListEntry *pNext;
	struct _HelloExpiryListEntry *pPrev;
} 
	HelloExpiryListEntry, *PHelloExpiryListEntry;

//-------------------------------------------------
// structure for a single RREQ ID entry
typedef
struct _RREQIDEntry 
{
	IN_ADDR OrigIPAddr;
	ULONG RREQIDNum;
	MILISECTIME ExpiryTime;
} 
	RREQIDEntry, *PRREQIDEntry;

// structure for a single RREQ ID List entry
typedef
struct _RREQIDListEntry 
{
	PRREQIDEntry pRREQIDValue;
	struct _RREQIDListEntry *pNext;
	struct _RREQIDListEntry *pPrev;
} 
	RREQIDListEntry, *PRREQIDListEntry;

//-------------------------------------------------
// structure for a single RREQ ID expiry entry
typedef
struct _RREQIDExpiryEntry 
{
	IN_ADDR OrigIPAddr;
	ULONG RREQIDNum;
	MILISECTIME ExpiryTime;
} 
	RREQIDExpiryEntry, *PRREQIDExpiryEntry;

// structure for a single RREQ ID expiry List entry
typedef
struct _RREQIDExpiryListEntry 
{
	PRREQIDExpiryEntry pRREQIDExpiryValue;
	struct _RREQIDExpiryListEntry *pNext;
	struct _RREQIDExpiryListEntry *pPrev;
} 
	RREQIDExpiryListEntry, *PRREQIDExpiryListEntry;

//-------------------------------------------------
// structure for Route Discovery thread

typedef
struct _RouteDiscoveryTimerEntry 
{
	IN_ADDR DestIPAddr;
	MILISECTIME ExpiryTime;
}
	RouteDiscoveryTimerEntry, *PRouteDiscoveryTimerEntry;

// structure for a single Route Discovery List entry
typedef
struct _RouteDiscoveryTimerListEntry 
{
	PRouteDiscoveryTimerEntry pRouteDiscoveryTimerValue;
	struct _RouteDiscoveryTimerListEntry *pNext;
	struct _RouteDiscoveryTimerListEntry *pPrev;
} 
	RouteDiscoveryTimerListEntry, *PRouteDiscoveryTimerListEntry;

//-------------------------------------------------
// structure for a single in-progress Route Discovery
typedef
struct _RouteDiscoveryEntry 
{
	IN_ADDR DestIPAddr;
	RREQMessage LastRREQMsg;
	IPPacketListEntry IPPacketListHead;
	int RREQRetries;
	int WaitDuration;
} 
	RouteDiscoveryEntry, *PRouteDiscoveryEntry;

// structure for a single Route Discovery List entry
typedef
struct _RouteDiscoveryListEntry 
{
	PRouteDiscoveryEntry pRouteDiscoveryValue;
	struct _RouteDiscoveryListEntry *pNext;
	struct _RouteDiscoveryListEntry *pPrev;
} 
	RouteDiscoveryListEntry, *PRouteDiscoveryListEntry;



#define		IPPROTOCOL_UDP			17

#define		MAX_POSSIBLE_ROUTES		512

#pragma pack(pop)

#pragma pack(push,4)

typedef
struct _IPAddrInfo {
		// The arrays. (IP addr & Mask IP addr)
	ULONG IPAddr;
	ULONG MaskIPAddr;
} 
 IPAddrInfo, *PIPAddrInfo;

typedef
struct _IPv4BlockAddrArray
{
    // Number of array elements.
    ULONG NumberElements;
	IPAddrInfo IPAddrInfoArray[1];
}
   IPv4BlockAddrArray, *PIPv4BlockAddrArray, **HIPv4BlockAddrArray;

#pragma pack(pop)

#endif // __UOBWINAODVCOMMON__H
