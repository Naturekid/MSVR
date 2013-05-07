/////////////////////////////////////////////////////////////////////////////
//// INCLUDE FILES

#include "precomp.h"
#pragma hdrstop
#include "iocommon.h"

//
// Copyright (c) 2004 ComNets, University of Bremen. 
// (retains the original copyright for modifications)
//
// Original code has been changed to accomodate functions required
// for UoB WinAODV
//
// Author of changes : Asanga Udugama (adu@comnets.uni-bremen.de)
// Date : 17-Aug-2004
//
//




//Original Copyright Message
//--------------------------
//
// Copyright And Configuration Management ----------------------------------
//
//         Implementation for PassThru Driver Filtering Module - filter.c
//
//                  Companion Sample Code for the Article
//
//        "Extending the Microsoft PassThru NDIS Intermediate Driver"
//
//    Copyright (c) 2003 Printing Communications Associates, Inc. (PCAUSA)
//                          http://www.pcausa.com
//
// The right to use this code in your own derivative works is granted so long
// as 1.) your own derivative works include significant modifications of your
// own, 2.) you retain the above copyright notices and this paragraph in its
// entirety within sources derived from this code.
// This product includes software developed by PCAUSA. The name of PCAUSA
// may not be used to endorse or promote products derived from this software
// without specific prior written permission.
// THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// End ---------------------------------------------------------------------
///
//
//
//
////////////////////////////////////////////////////////////////////////////
//                           Structure Definitions                        //
////////////////////////////////////////////////////////////////////////////

//
// FilterReserved Part of ADAPT Structure
// --------------------------------------
// This structure will be zero-initialized when the ADAPT structure is
// allocated.
//
// Hold Appropriate Spin Lock When Accessing FilterReserved Data
// -------------------------------------------------------------
// Acquiring a spin lock raises the IRQL to IRQL DISPATCH_LEVEL. Running
// at IRQL DISPATCH level insures that the filter data (contained
// somewhere in the FilterReserved area of the ADAPT structure) cannot
// be changed by one routine while it is being actively used by another.
//
// In this simple sample we can just use the Adapter spin lock for
// synchronization.
//
typedef struct _ADAPT_FILTER_RSVD
{
   BOOLEAN     bFilterInitDone;

   //
   // More Per-Adapter Filter-Specific Members
   // ----------------------------------------
   // Probably would contain the filter data or a pointer to it. Possibly
   // filter statistics, state variables, whatever...
   //
   IPv4AddrStats        IPv4Stats;

   PIPv4BlockAddrArray  pIPv4BlockAddrArray;

   PMACBlockAddrArray  pMACBlockAddrArray;

   // packet list variables
	BOOLEAN				PacketListActive;
	LIST_ENTRY			PacketListHead;
	NDIS_SPIN_LOCK		PacketListSpinLock;
	ULONG				PacketCount;
}
   ADAPT_FILTER_RSVD, *PADAPT_FILTER_RSVD;

C_ASSERT(sizeof(ADAPT_FILTER_RSVD) <= sizeof(((PADAPT)0)->FilterReserved));

int MySearch(PIPAddrInfo pCheckIPAddrInfo, 
			  PIPv4BlockAddrArray pAddressArray);
int MyMACSearch(PMACAddrInfo pCheckMACAddrInfo, 
				PMACBlockAddrArray pMACAddressArray);

//
// FilterReserved Part of OPEN_CONTEXT Structure
// ---------------------------------------------
// This structure will be zero-initialized when the OPEN_CONTEXT structure
// is allocated.
//
typedef struct _OPEN_CONTEXT_FILTER_RSVD
{
   BOOLEAN     bFilterInitDone;

   //
   // More Per-Open-Handle Filter-Specific Members
   // --------------------------------------------
   // Probably would contain the filter data or a pointer to it. Possibly
   // filter statistics, state variables, whatever...
   //
}
   OPEN_FILTER_RSVD, *POPEN_FILTER_RSVD;

C_ASSERT(sizeof(OPEN_FILTER_RSVD) <= sizeof(((POPEN_CONTEXT)0)->FilterReserved));


////////////////////////////////////////////////////////////////////////////
//                        Per-Open Filter Functions                       //
////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//// FltDevIoControl
//
// Purpose
//    This is the handler for filter-specific IOCTL function codes.
//
// Parameters
//    DeviceObject - pointer to a device object
//    pIrp - pointer to an I/O Request Packet
//
// Return Value
//    Status is returned.
//
// Remarks
//    Passthru call from main DevIoControl handler for IOCTL functions
//    that are not recognized there.
//

NTSTATUS
FltDevIoControl(
   IN PDEVICE_OBJECT    pDeviceObject,
   IN PIRP              pIrp
   )
{
   PIO_STACK_LOCATION  pIrpSp;
   NTSTATUS            NtStatus = STATUS_NOT_SUPPORTED;
   ULONG               BytesReturned = 0;
   ULONG               FunctionCode;
   PUCHAR              ioBuffer = NULL;
   ULONG               inputBufferLength;
   ULONG               outputBufferLength;
   PADAPT              pAdapt = NULL;
   PADAPT_FILTER_RSVD  pFilterContext = NULL;
   POPEN_CONTEXT       pOpenContext;

   UNREFERENCED_PARAMETER(pDeviceObject);

   pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

   DBGPRINT(("==>FLT DevIoControl: FileObject %p\n", pIrpSp->FileObject));

   FunctionCode = pIrpSp->Parameters.DeviceIoControl.IoControlCode;

   ioBuffer = pIrp->AssociatedIrp.SystemBuffer;
   inputBufferLength  = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
   outputBufferLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

   pOpenContext = pIrpSp->FileObject->FsContext;

   if( !pOpenContext )
   {
      DBGPRINT(( "      Invalid Handle\n" ));
      
      NtStatus = STATUS_INVALID_HANDLE;
      goto CompleteTheIRP;
   }

   DBGPRINT(( "      Found Open Context\n" ));

   pAdapt = pOpenContext->pAdapt;

   if( !pAdapt )
   {
      DBGPRINT(( "      Adapter Not Found\n" ));
      
      NtStatus = STATUS_INVALID_HANDLE;
      goto CompleteTheIRP;
   }

   pFilterContext = (PADAPT_FILTER_RSVD )&pAdapt->FilterReserved;

   //
   // Fail IOCTL If Unbind Is In Progress
   //
   NdisAcquireSpinLock(&pAdapt->Lock);

   if( pAdapt->UnbindingInProcess )
   {
      NdisReleaseSpinLock(&pAdapt->Lock);
      DBGPRINT(( "      Unbind In Process\n" ));

      NtStatus = STATUS_INVALID_DEVICE_STATE;
      goto CompleteTheIRP;
   }

   //
   // Fail IOCTL If Adapter Is Powering Down
   // 
   if (pAdapt->StandingBy == TRUE)
   {
      NdisReleaseSpinLock(&pAdapt->Lock);
      DBGPRINT(( "      Miniport Powering Down\n" ));

      NtStatus = STATUS_INVALID_DEVICE_STATE;
      goto CompleteTheIRP;
   }

   //
   // Hold Appropriate Spin Lock When Changing Filter Data
   // ----------------------------------------------------
   // This is just a reminder. Code for changing filter is not yet
   // implemented. It would be in one (or more) of the IOCTL function
   // handlers.
   //
   // See note at ADAPT_FILTER_RSVD structure declaration.
   //

   //
   // Now (Finally) Handle The IOCTL
   //
   switch (FunctionCode)
   {
        case IOCTL_PTUSERIO_QUERY_IPv4_BLOCK_STATISTICS:
            {
               BytesReturned = sizeof( IPv4AddrStats );

               if( outputBufferLength < BytesReturned )
               {
                  NtStatus = STATUS_BUFFER_TOO_SMALL;
                  break;
               }

               NdisMoveMemory(
                  ioBuffer,
                  &pFilterContext->IPv4Stats,
                  BytesReturned
                  );
               
               NtStatus = STATUS_SUCCESS;
            }
            break;

        case IOCTL_PTUSERIO_RESET_IPv4_BLOCK_STATISTICS:
            NdisZeroMemory(
               &pFilterContext->IPv4Stats,
               sizeof( IPv4AddrStats )
               );

            NtStatus = STATUS_SUCCESS;
            break;

        case IOCTL_PTUSERIO_SET_IPv4_BLOCK_FILTER:
            {
               ULONG                nExpectedBufferSize = 0;
               PIPv4BlockAddrArray  pNewIPv4BlockAddrArray = NULL;
               PIPv4BlockAddrArray  pOldIPv4BlockAddrArray;

               pOldIPv4BlockAddrArray = pFilterContext->pIPv4BlockAddrArray;

               if( ioBuffer && inputBufferLength
                  && inputBufferLength >= sizeof( IPv4BlockAddrArray )
                  )
               {
                  pNewIPv4BlockAddrArray = (PIPv4BlockAddrArray )ioBuffer;

                  //nExpectedBufferSize = sizeof( ULONG ) * pNewIPv4BlockAddrArray->NumberElements;
                  nExpectedBufferSize = sizeof( IPAddrInfo ) * pNewIPv4BlockAddrArray->NumberElements;
                  nExpectedBufferSize += sizeof( ULONG );

                  if( nExpectedBufferSize > inputBufferLength )
                  {
                     NtStatus = STATUS_INVALID_PARAMETER;
                     break;
                  }
               
                  pNewIPv4BlockAddrArray = NULL;
               }

               //
               // Allocate And Initialize The New IP Block Address Array
               //
               if( nExpectedBufferSize )
               {
                  NDIS_STATUS       nNdisStatus;

                  nNdisStatus = NdisAllocateMemoryWithTag(
                                    &pNewIPv4BlockAddrArray,
                                    nExpectedBufferSize,
                                    TAG
                                    );

                  if( nNdisStatus == NDIS_STATUS_SUCCESS )
                  {
                     //
                     // Copy The IP Block Address Array
                     //
                     NdisMoveMemory(
                        pNewIPv4BlockAddrArray, ioBuffer, nExpectedBufferSize );

                     NtStatus = STATUS_SUCCESS;
                  }
                  else
                  {
                     NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                  }
               }

               //
               // Set The New IP Block Address Array
               //
               pFilterContext->pIPv4BlockAddrArray = pNewIPv4BlockAddrArray;

               //
               // Free The Old IP Block Address Array
               //
               if( pOldIPv4BlockAddrArray )
               {
                  NdisFreeMemory( pOldIPv4BlockAddrArray, 0, 0);
               }

			   // activate packet list
			   if(pFilterContext->PacketListActive != TRUE)
			   {
					pFilterContext->PacketListActive = TRUE;
					InitializeListHead(&(pFilterContext->PacketListHead));
					NdisAllocateSpinLock(&(pFilterContext->PacketListSpinLock));
					pFilterContext->PacketCount = 0;
			   }
            }
            break;

        case IOCTL_PTUSERIO_SET_MAC_BLOCK_FILTER:
			{
               ULONG                nExpectedBufferSize = 0;
               PMACBlockAddrArray  pNewMACBlockAddrArray = NULL;
               PMACBlockAddrArray  pOldMACBlockAddrArray;

               pOldMACBlockAddrArray = pFilterContext->pMACBlockAddrArray;

               if( ioBuffer && inputBufferLength
                  && inputBufferLength >= sizeof( MACBlockAddrArray )
                  )
               {
                  pNewMACBlockAddrArray = (PMACBlockAddrArray )ioBuffer;

                  nExpectedBufferSize = sizeof( MACAddrInfo ) * pNewMACBlockAddrArray->NumberElements;
                  nExpectedBufferSize += sizeof( ULONG );

                  if( nExpectedBufferSize > inputBufferLength )
                  {
                     NtStatus = STATUS_INVALID_PARAMETER;
                     break;
                  }
               
                  pNewMACBlockAddrArray = NULL;
               }

               //
               // Allocate And Initialize The New MAC Block Address Array
               //
               if( nExpectedBufferSize )
               {
                  NDIS_STATUS       nNdisStatus;

                  nNdisStatus = NdisAllocateMemoryWithTag(
                                    &pNewMACBlockAddrArray,
                                    nExpectedBufferSize,
                                    TAG
                                    );

                  if( nNdisStatus == NDIS_STATUS_SUCCESS )
                  {
                     //
                     // Copy The MAC Block Address Array
                     //
                     NdisMoveMemory(
                        pNewMACBlockAddrArray, ioBuffer, nExpectedBufferSize );

                     NtStatus = STATUS_SUCCESS;
                  }
                  else
                  {
                     NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                  }
               }

               //
               // Set The New MAC Block Address Array
               //
               pFilterContext->pMACBlockAddrArray = pNewMACBlockAddrArray;

               //
               // Free The Old MAC Block Address Array
               //
               if( pOldMACBlockAddrArray )
               {
                  NdisFreeMemory( pOldMACBlockAddrArray, 0, 0);
               }

			   // activate packet list
			   if(pFilterContext->PacketListActive != TRUE)
			   {
					pFilterContext->PacketListActive = TRUE;
					InitializeListHead(&(pFilterContext->PacketListHead));
					NdisAllocateSpinLock(&(pFilterContext->PacketListSpinLock));
					pFilterContext->PacketCount = 0;
			   }
            }

			break;

        case IOCTL_PTUSERIO_GET_BUFFERED_IPv4_PACKET:
			{
				PLIST_ENTRY			pLinkField;
				PPacketDataEntry	pPacketDataEntry;

				if(pFilterContext->PacketListActive != TRUE) {
					NtStatus = STATUS_NO_MORE_ENTRIES;
					break;
				}
				// acquire lock to add to list
				NdisAcquireSpinLock(&(pFilterContext->PacketListSpinLock));

				if(pFilterContext->PacketCount > 0
					&& !IsListEmpty(&(pFilterContext->PacketListHead))) 
				{
					// get entry from tail
					pLinkField = RemoveHeadList(&(pFilterContext->PacketListHead));
					pFilterContext->PacketCount--;
					--pFilterContext->IPv4Stats.PTBufferedPkts;  // SpinLock Already Held
					
					// release the lock
					NdisReleaseSpinLock(&(pFilterContext->PacketListSpinLock));

					// get packet entry
					pPacketDataEntry = CONTAINING_RECORD(pLinkField, PacketDataEntry, LinkField);

					BytesReturned = (outputBufferLength < (pPacketDataEntry->PacketSize + 1) ? 
										outputBufferLength : (pPacketDataEntry->PacketSize + 1));
					NdisMoveMemory((ioBuffer + 1), pPacketDataEntry->pPacketData, (BytesReturned - 1));
					(*ioBuffer) = pPacketDataEntry->PacketType;

					// free up memory
					NdisFreeMemory( pPacketDataEntry->pPacketData, 0, 0);
					NdisFreeMemory( pPacketDataEntry, 0, 0);

					NtStatus = STATUS_SUCCESS;
				} 
				else
				{
					NtStatus = STATUS_NO_MORE_ENTRIES;

					// release the lock
					NdisReleaseSpinLock(&(pFilterContext->PacketListSpinLock));
				}
			}

			break;

		case IOCTL_PTUSERIO_QUERY_IPv4_BLOCK_FILTERS:
            {
				if(!pFilterContext->pIPv4BlockAddrArray)
				{
					NtStatus = STATUS_NO_MORE_ENTRIES;
					break;
				}

				BytesReturned = sizeof(ULONG) + 
				   (pFilterContext->pIPv4BlockAddrArray->NumberElements * sizeof(IPAddrInfo));

				if( outputBufferLength < BytesReturned )
				{
					NtStatus = STATUS_BUFFER_TOO_SMALL;
					break;
				}

				NdisMoveMemory(
                  ioBuffer,
                  pFilterContext->pIPv4BlockAddrArray,
                  BytesReturned
                  );
               
				NtStatus = STATUS_SUCCESS;
			}
            break;

		case IOCTL_PTUSERIO_QUERY_MAC_BLOCK_FILTERS:
            {
				if(!pFilterContext->pMACBlockAddrArray)
				{
					NtStatus = STATUS_NO_MORE_ENTRIES;
					break;
				}

				BytesReturned = sizeof(ULONG) + 
				   (pFilterContext->pMACBlockAddrArray->NumberElements * sizeof(MACAddrInfo));

				if( outputBufferLength < BytesReturned )
				{
					NtStatus = STATUS_BUFFER_TOO_SMALL;
					break;
				}

				NdisMoveMemory(
                  ioBuffer,
                  pFilterContext->pMACBlockAddrArray,
                  BytesReturned
                  );
               
				NtStatus = STATUS_SUCCESS;
			}
            break;


        default:
            // ...Fail with STATUS_NOT_SUPPORTED for now... // TEMPORARY!!!
            NtStatus = STATUS_NOT_SUPPORTED;                // TEMPORARY!!!
            break;
   }

   NdisReleaseSpinLock(&pAdapt->Lock);

   //
   // Complete The IRP
   //
CompleteTheIRP:

   if (NtStatus != STATUS_PENDING)
   {
      pIrp->IoStatus.Information = BytesReturned;
      pIrp->IoStatus.Status = NtStatus;
      IoCompleteRequest(pIrp, IO_NO_INCREMENT);
   }
    
   DBGPRINT(("<== FLT DevIoControl\n"));
   
   return NtStatus;
}


VOID
FltOnInitOpenContext(
    IN POPEN_CONTEXT pOpenContext
    )
{
   POPEN_FILTER_RSVD   pFilterContext;

   //
   // Initialize FilterReserved Area In OPEN_CONTEXT Structure
   //
   pFilterContext = (POPEN_FILTER_RSVD )&pOpenContext->FilterReserved;
}


VOID
FltOnDeinitOpenContext(
    IN POPEN_CONTEXT pOpenContext
    )
{
   POPEN_FILTER_RSVD   pFilterContext;

   //
   // Deinitialize FilterReserved Area In OPEN_CONTEXT Structure
   //
   pFilterContext = (POPEN_FILTER_RSVD )&pOpenContext->FilterReserved;
}


////////////////////////////////////////////////////////////////////////////
//                      Per-Adapter Filter Functions                      //
////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//// FltOnInitAdapter
//
// Purpose
//    Called to initialize the FilterReserved area in a newly-allocated
//    ADAPT structure.
//
// Parameters
//    pAdapt - pointer to ADAPT structure being initialized.
//
// Return Value
//    Nothing.
//
// Remarks
//    Called from PtBindAdapter just prior to making the call to
//    NdisOpenAdapter.
//

VOID
FltOnInitAdapter(
    IN PADAPT  pAdapt
    )
{
   PADAPT_FILTER_RSVD   pFilterContext;

   //
   // Initialize FilterReserved Area In ADAPT Structure
   //
   pFilterContext = (PADAPT_FILTER_RSVD )&pAdapt->FilterReserved;
}


/////////////////////////////////////////////////////////////////////////////
//// FltOnDeinitAdapter
//
// Purpose
//    Called to free resources associated with the FilterReserved area in
//    the ADAPT structure.
//
// Parameters
//    pAdapt - pointer to ADAPT structure to be freed.
//
// Return Value
//    Nothing.
//
// Remarks
//    Called from protocol.c and miniport.c immediately after adapter is
//    closed and just prior to freeing the ADAPT structure.
//

VOID
FltOnDeinitAdapter(
    IN PADAPT  pAdapt
    )
{
   PADAPT_FILTER_RSVD   pFilterContext;

   //
   // Deinitialize FilterReserved Area In ADAPT Structure
   //
   pFilterContext = (PADAPT_FILTER_RSVD )&pAdapt->FilterReserved;

   if( pFilterContext->pIPv4BlockAddrArray )
   {
      NdisFreeMemory( pFilterContext->pIPv4BlockAddrArray, 0, 0);
   }

   pFilterContext->pIPv4BlockAddrArray = NULL;
}

////////////////////////////////////////////////////////////////////////////
//                        Send Packet Filter Functions                    //
////////////////////////////////////////////////////////////////////////////

int IPv4AddrCompare( const void *pKey, const void *pElement )
{
   //ULONG a1 = *(PULONG )pKey;
   //ULONG a2 = *(PULONG )pElement;
   PIPAddrInfo pIPAddrInfoKey = (PIPAddrInfo ) pKey;
   PIPAddrInfo pIPAddrInfoElement = (PIPAddrInfo ) pElement;

   ULONG keyIP = pIPAddrInfoKey->IPAddr;
   ULONG elemIP = pIPAddrInfoElement->IPAddr;
   ULONG maskIP = pIPAddrInfoElement->MaskIPAddr;
   ULONG a1 = keyIP & maskIP;
   ULONG a2 = elemIP & maskIP;

   if( a1 == a2 )
   {
      return( 0 );
   }

   if( a1 < a2 )
   {
      return( -1 );
   }

   return( 1 );
}

/////////////////////////////////////////////////////////////////////////////
//// FltFilterSendPacket
//
// Purpose
//   Called to filter each packet packet being sent.
//
// Parameters
//    pAdapt - pointer to ADAPT structure that the send is on.
//    pSendPacket - pointer to send packet to be filtered.
//    DispatchLevel - TRUE if caller is running at IRQL DISPATCH_LEVEL.
//                    FALSE of caller is running at IRQL <= DISPATCH_LEVEL.
//
// Return Value
//    A ULONG containing the send filter bitmap defined in filter.h
//
// Remarks
//   Called from MPSendPackets for each individual packet in the send
//   packet array and from MPSend.
//
//   Runs at IRQL <= DISPATCH_LEVEL if called from MPSendPackets or
//   at IRQL == DISPATCH_LEVEL if called from MPSend.
//

ULONG
FltFilterSendPacket(
	IN PADAPT         pAdapt,
	IN	PNDIS_PACKET   pSendPacket,
   IN BOOLEAN        DispatchLevel  // TRUE -> IRQL == DISPATCH_LEVEL
	)
{
   PADAPT_FILTER_RSVD   pFilterContext;
   ULONG                SndFltAction = SND_FLT_SIMPLE_PASSTHRU;
   USHORT               EtherType;
   ULONG                NumberOfBytesRead;
   struct ip            IPHeader;
   IPAddrInfo currIPAddrInfo;
   unsigned char PacketType = ALLOWED_PACKET;

   //
   // Hold Adapter Spin Lock When Using Filter Data
   // ---------------------------------------------
   // See note at ADAPT_FILTER_RSVD structure declaration.
   //
   if( DispatchLevel )
      NdisDprAcquireSpinLock(&pAdapt->Lock);
   else
      NdisAcquireSpinLock(&pAdapt->Lock);

   //
   // Find FilterReserved Area In ADAPT Structure
   //
   pFilterContext = (PADAPT_FILTER_RSVD )&pAdapt->FilterReserved;

   ++pFilterContext->IPv4Stats.MPSendPktsCt; // SpinLock Already Held

   //
   // Pass All Packets If No Filter Is Set
   //
   if( !pFilterContext->pIPv4BlockAddrArray )
   {
      goto ExitTheFilter;
   }

   //
   // Filter On EtherType
   // -------------------
   // We only filter IP at this point. The switch below is provided simply
   // to illustrate how other EtherTypes could be identified and handled
   // differently.
   //
   FltReadOnPacket(
      pSendPacket,
      &EtherType,
      sizeof( EtherType ),
      FIELD_OFFSET( struct ether_header, ether_type ),
      &NumberOfBytesRead
      );

   if( NumberOfBytesRead != sizeof( EtherType ) )
   {
      goto ExitTheFilter;
   }

   switch( ntohs( EtherType ) )
   {
      // See ..\B2Winet/ethernet.h for ETHERTYPE Definitions
      case ETHERTYPE_IP:
         break;

      case ETHERTYPE_ARP:
			//++pFilterContext->IPv4Stats.PTARPPktsSent;  // SpinLock Already Held
		  goto SaveARPPacket;
      case ETHERTYPE_REVARP:
      case ETHERTYPE_NETBEUI:
      default:
         goto ExitTheFilter;
   }

   //
   // Fetch The IP Header
   // -------------------
   // The logic of the send packet filter and the recceive packet filter
   // is identical - except for whether the IP destination address of the
   // IP source address should be checked.
   //
   // However. for illustrative purposes there is a difference in the
   // filter implementation.
   //
   // The send packet filter uses FltReadOnPacket to fetch the entire
   // IP header and then accesses information using fields in struct ip.
   //
   // The receive packet filter uses a different technique. It uses
   // FltReadOnPacket to fetch only specific fields of interest.
   //
   FltReadOnPacket(
      pSendPacket,
      &IPHeader,
      sizeof( IPHeader ),
      sizeof( struct ether_header ),
      &NumberOfBytesRead
      );

   if( NumberOfBytesRead != sizeof( IPHeader ) )
   {
      goto ExitTheFilter;
   }

   //
   // Only Filter IPv4 For Now...
   //
   switch( IPHeader.ip_v )
   {
      case 4:        // IPv4
         break;

      case 6:        // IPv6
      default:
         goto ExitTheFilter;
   }

   //
   // Do Binary Search On Sorted List Of IP Addresses To Block
   //
   //if( bsearch(
   //      &IPHeader.ip_dst.s_addr,                              // Key To Search For
   //      (pFilterContext->pIPv4BlockAddrArray)->IPAddrArray,   // Array Base
   //      (pFilterContext->pIPv4BlockAddrArray)->NumberElements,// Number Of Elements In Array
   //      sizeof( ULONG ),                                      // Bytes Per Element
   //      IPv4AddrCompare                                       // Comparison Function
   //      )
   //   )
   //
   // Asanga 
   // Do Binary Search On Sorted List Of IP Addresses To Block
   // If the IP address is not in the list
   //

   currIPAddrInfo.IPAddr = IPHeader.ip_dst.s_addr;
   currIPAddrInfo.MaskIPAddr = 0;
   //if( !bsearch(
   //      &currIPAddrInfo,                              // Key To Search For
   //      (pFilterContext->pIPv4BlockAddrArray)->IPAddrInfoArray,   // Array Base
   //      (pFilterContext->pIPv4BlockAddrArray)->NumberElements,// Number Of Elements In Array
   //      sizeof( IPAddrInfo ),                                      // Bytes Per Element
   //      IPv4AddrCompare                                       // Comparison Function
   //      )
   //   )
   if( !MySearch(&currIPAddrInfo, (pFilterContext->pIPv4BlockAddrArray)) )
   {
      //
      // Reject Packets That Have Matching IP Source Address
      //
      ++pFilterContext->IPv4Stats.MPSendPktsDropped;  // SpinLock Already Held
      SndFltAction = SND_FLT_BLOCK_PACKET;
	  PacketType = DISALLOWED_PACKET;
   }
   else 
   {
		if(IPHeader.ip_dst.s_addr == (ULONG) 0xffffffff)
		{
			++pFilterContext->IPv4Stats.McastPktsSent;  // SpinLock Already Held
		}
   }
   

	// place packet in list
	{
		PPacketDataEntry	pPacketDataEntry;
		NDIS_STATUS			nNdisStatus;
		ULONG				PacketSize, NumberOfBytesRead;

			// get memory for the list item
		nNdisStatus = NdisAllocateMemoryWithTag(&pPacketDataEntry,
											sizeof(PacketDataEntry),
											TAG);
		if( nNdisStatus != NDIS_STATUS_SUCCESS )
		{
			goto ExitSavePacket;
		}

		// get memory for the IP packet
		pPacketDataEntry->pPacketData = NULL;
		pPacketDataEntry->PacketSize = 0;
		PacketSize = ntohs(IPHeader.ip_len);
		nNdisStatus = NdisAllocateMemoryWithTag(&(pPacketDataEntry->pPacketData),
											PacketSize,
											TAG);
		if( nNdisStatus != NDIS_STATUS_SUCCESS )
		{
			NdisFreeMemory( pPacketDataEntry, 0, 0);
			goto ExitSavePacket;
		}

		NumberOfBytesRead = 0;
		// copy packet contents
		FltReadOnPacket(pSendPacket,
							pPacketDataEntry->pPacketData,
							PacketSize,
							sizeof( struct ether_header ),
							&NumberOfBytesRead);

		if( NumberOfBytesRead != PacketSize )
		{
			NdisFreeMemory( pPacketDataEntry->pPacketData, 0, 0);
			NdisFreeMemory( pPacketDataEntry, 0, 0);
			goto ExitSavePacket;
		}
		pPacketDataEntry->PacketSize = PacketSize;
		pPacketDataEntry->PacketType = PacketType;

		// acquire lock to add to list
		NdisAcquireSpinLock(&(pFilterContext->PacketListSpinLock));

		if(pFilterContext->PacketCount > PACKET_LIMIT) 
		{
			NdisFreeMemory( pPacketDataEntry->pPacketData, 0, 0);
			NdisFreeMemory( pPacketDataEntry, 0, 0);

			// release the lock
			NdisReleaseSpinLock(&(pFilterContext->PacketListSpinLock));
			goto ExitSavePacket;
		}

		// insert list item
		InsertTailList(&(pFilterContext->PacketListHead), &(pPacketDataEntry->LinkField));
		(pFilterContext->PacketCount)++;
		++pFilterContext->IPv4Stats.PTBufferedPkts;  // SpinLock Already Held

		// release the lock
		NdisReleaseSpinLock(&(pFilterContext->PacketListSpinLock));

	ExitSavePacket:			
		goto ExitTheFilter	;
	}

SaveARPPacket:
	// place ARP packet in list
	{
		PPacketDataEntry	pPacketDataEntry;
		NDIS_STATUS			nNdisStatus;
		ULONG				PacketSize, NumberOfBytesRead;

			// get memory for the list item
		nNdisStatus = NdisAllocateMemoryWithTag(&pPacketDataEntry,
											sizeof(PacketDataEntry),
											TAG);
		if( nNdisStatus != NDIS_STATUS_SUCCESS )
		{
			goto ExitSaveARPPacket;
		}

		// get memory for the IP packet
		pPacketDataEntry->pPacketData = NULL;
		pPacketDataEntry->PacketSize = 0;
		PacketSize = ARP_PACKET_SIZE;
		nNdisStatus = NdisAllocateMemoryWithTag(&(pPacketDataEntry->pPacketData),
											PacketSize,
											TAG);
		if( nNdisStatus != NDIS_STATUS_SUCCESS )
		{
			NdisFreeMemory( pPacketDataEntry, 0, 0);
			goto ExitSaveARPPacket;
		}

		NumberOfBytesRead = 0;
		// copy packet contents
		FltReadOnPacket(pSendPacket,
							pPacketDataEntry->pPacketData,
							PacketSize,
							sizeof( struct ether_header ),
							&NumberOfBytesRead);

		if( NumberOfBytesRead != PacketSize )
		{
			NdisFreeMemory( pPacketDataEntry->pPacketData, 0, 0);
			NdisFreeMemory( pPacketDataEntry, 0, 0);
			goto ExitSaveARPPacket;
		}
		pPacketDataEntry->PacketSize = PacketSize;
		pPacketDataEntry->PacketType = ALLOWED_ARP_PACKET;

		// acquire lock to add to list
		NdisAcquireSpinLock(&(pFilterContext->PacketListSpinLock));

		if(pFilterContext->PacketCount > PACKET_LIMIT) 
		{
			NdisFreeMemory( pPacketDataEntry->pPacketData, 0, 0);
			NdisFreeMemory( pPacketDataEntry, 0, 0);

			// release the lock
			NdisReleaseSpinLock(&(pFilterContext->PacketListSpinLock));
			goto ExitSaveARPPacket;
		}

		// insert list item
		InsertTailList(&(pFilterContext->PacketListHead), &(pPacketDataEntry->LinkField));
		(pFilterContext->PacketCount)++;
		++pFilterContext->IPv4Stats.PTBufferedPkts;  // SpinLock Already Held

		// release the lock
		NdisReleaseSpinLock(&(pFilterContext->PacketListSpinLock));

	ExitSaveARPPacket:			
		goto ExitTheFilter	;
	}

ExitTheFilter:
   //
   // Release Adapter Spin Lock After Filtering
   //
   if( DispatchLevel )
      NdisDprReleaseSpinLock(&pAdapt->Lock);
   else
      NdisReleaseSpinLock(&pAdapt->Lock);

   return( SndFltAction );
}

////////////////////////////////////////////////////////////////////////////
//                      Receive Packet Filter Functions                   //
////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//// FltFilterReceivePacket
//
// Purpose
//   Called to filter each received packet.
//
// Parameters
//    pAdapt - pointer to ADAPT structure that the send is on.
//    pReceivedPacket - pointer to receive packet to be filtered.
//
// Return Value
//    A ULONG containing the receive filter action bitmap defined
//    in filter.h
//
// Remarks
//   Called from PtReceivePacket for each received packet.
//
//   Runs at IRQL DISPATCH_LEVEL.
//

ULONG
FltFilterReceivePacket(
	IN PADAPT         pAdapt,
	IN	PNDIS_PACKET   pReceivedPacket
	)
{
   PADAPT_FILTER_RSVD   pFilterContext;
   ULONG                RcvFltAction = RCV_FLT_SIMPLE_PASSTHRU;
   USHORT               EtherType;
   ULONG                NumberOfBytesRead;
   ULONG                PktSrcAddr;
   UCHAR                IPVersion;

   //
   // Hold Adapter Spin Lock When Using Filter Data
   // ---------------------------------------------
   // See note at ADAPT_FILTER_RSVD structure declaration.
   //
   NdisDprAcquireSpinLock(&pAdapt->Lock);

   //
   // Find FilterReserved Area In ADAPT Structure
   //
   pFilterContext = (PADAPT_FILTER_RSVD )&pAdapt->FilterReserved;

   ++pFilterContext->IPv4Stats.PTRcvPktCt;   // SpinLock Already Held


   //
   // Filter On EtherType
   // -------------------
   // We only filter IP at this point. The switch below is provided simply
   // to illustrate how other EtherTypes could be identified and handled
   // differently.
   //
   FltReadOnPacket(
      pReceivedPacket,
      &EtherType,
      sizeof( EtherType ),
      FIELD_OFFSET( struct ether_header, ether_type ),
      &NumberOfBytesRead
      );

   if( NumberOfBytesRead != sizeof( EtherType ) )
   {
      goto ExitTheFilter;
   }

   switch( ntohs( EtherType ) )
   {
      // See ..\B2Winet/ethernet.h for ETHERTYPE Definitions
      case ETHERTYPE_IP:
         break;

      case ETHERTYPE_ARP:
      case ETHERTYPE_REVARP:
      case ETHERTYPE_NETBEUI:
      default:
         goto ExitTheFilter;
   }

   // check MAC filter (blocked or not)
   {
		MACAddrInfo currMACAddrInfo;
		struct ether_header EthHdr;      // See ../B2Winet/ethernet.h
		ULONG i;

		//
		// Pass All Packets If No MAC Filter Is Set
		//
		if( !pFilterContext->pMACBlockAddrArray )
		{
			goto ExitMACFiltering;
		}

		FltReadOnPacket(pReceivedPacket,
						&EthHdr,
						sizeof( struct ether_header ),
						0,
						&NumberOfBytesRead );
		for(i = 0; i < MAC_ADDRESS_SIZE; i++)
		{
			currMACAddrInfo.MACAddr[i] = EthHdr.ether_shost[i];
		}

		if( MyMACSearch(&currMACAddrInfo, (pFilterContext->pMACBlockAddrArray)) )
		{
			RcvFltAction = RCV_FLT_BLOCK_PACKET;
			goto ExitTheFilter;
		}

	ExitMACFiltering:
			;
   }



   //
   // Pass All Packets If No Filter Is Set
   //
   if( !pFilterContext->pIPv4BlockAddrArray )
   {
      goto ExitTheFilter;
   }


   //
   // The logic of the send packet filter and the recceive packet filter
   // is identical - except for whether the IP destination address of the
   // IP source address should be checked.
   //
   // However. for illustrative purposes there is a difference in the
   // filter implementation.
   //
   // The receive packet filter uses FltReadOnPacket to fetch only specific
   // fields of interest.
   //
   // The send packet filter uses FltReadOnPacket to fetch the entire
   // IP header and then accesses information using fields in struct ip.
   //

   //
   // Only Filter IPv4 For Now...
   //
   FltReadOnPacket(
      pReceivedPacket,
      &IPVersion,
      sizeof( IPVersion ),
      sizeof( struct ether_header ),
      &NumberOfBytesRead
      );

   if( NumberOfBytesRead != sizeof( IPVersion ) )
   {
      goto ExitTheFilter;
   }

   if( NumberOfBytesRead != sizeof( IPVersion ) )
   {
      goto ExitTheFilter;
   }

   switch( IPVersion >> 4 )
   {
      case 4:        // IPv4
         break;

      case 6:        // IPv6
      default:
         goto ExitTheFilter;
   }

   //
   // Fetch IPv4 Source Address
   //
   FltReadOnPacket(
      pReceivedPacket,
      &PktSrcAddr,
      sizeof( PktSrcAddr ),
      sizeof( struct ether_header ) + FIELD_OFFSET( struct ip, ip_src.s_addr ),
      &NumberOfBytesRead
      );

   if( NumberOfBytesRead != sizeof( PktSrcAddr ) )
   {
      goto ExitTheFilter;
   }

   //
   // No blocking of incomming packets - Asanga
   //
   // Do Binary Search On Sorted List Of IP Addresses To Block
   //
   //if( bsearch(
   //      &PktSrcAddr,                                          // Key To Search For
   //      (pFilterContext->pIPv4BlockAddrArray)->IPAddrArray,     // Array Base
   //      (pFilterContext->pIPv4BlockAddrArray)->NumberElements,  // Number Of Elements In Array
   //      sizeof( ULONG ),                                      // Bytes Per Element
   //      IPv4AddrCompare                                      // Comparison Function
   //      )
   //   )
   //{
      //
      // Reject Packets That Have Matching IP Source Address
      //
   //   ++pFilterContext->IPv4Stats.PTRcvPktDropped; // SpinLock Already Held
   //   RcvFltAction = RCV_FLT_BLOCK_PACKET;
   //}



	// place packet in list
	{
		PPacketDataEntry	pPacketDataEntry;
		NDIS_STATUS			nNdisStatus;
		ULONG				PacketSize, NumberOfBytesRead;
		struct ip			IPHeader;


		FltReadOnPacket(
			pReceivedPacket,
			&IPHeader,
			sizeof( IPHeader ),
			sizeof( struct ether_header ),
			&NumberOfBytesRead
			);

		if( NumberOfBytesRead != sizeof( IPHeader ) )
		{
			goto ExitTheFilter;
		}
		
		
		// get memory for the list item
		nNdisStatus = NdisAllocateMemoryWithTag(&pPacketDataEntry,
											sizeof(PacketDataEntry),
											TAG);
		if( nNdisStatus != NDIS_STATUS_SUCCESS )
		{
			goto ExitSavePacket;
		}

		// get memory for the IP packet
		pPacketDataEntry->pPacketData = NULL;
		pPacketDataEntry->PacketSize = 0;
		PacketSize = ntohs(IPHeader.ip_len);
		nNdisStatus = NdisAllocateMemoryWithTag(&(pPacketDataEntry->pPacketData),
											PacketSize,
											TAG);
		if( nNdisStatus != NDIS_STATUS_SUCCESS )
		{
			NdisFreeMemory( pPacketDataEntry, 0, 0);
			goto ExitSavePacket;
		}

		NumberOfBytesRead = 0;
		// copy packet contents
		FltReadOnPacket(pReceivedPacket,
							pPacketDataEntry->pPacketData,
							PacketSize,
							sizeof( struct ether_header ),
							&NumberOfBytesRead);

		if( NumberOfBytesRead != PacketSize )
		{
			NdisFreeMemory( pPacketDataEntry->pPacketData, 0, 0);
			NdisFreeMemory( pPacketDataEntry, 0, 0);
			goto ExitSavePacket;
		}
		pPacketDataEntry->PacketSize = PacketSize;
		pPacketDataEntry->PacketType = ALLOWED_PACKET;

		// acquire lock to add to list
		NdisAcquireSpinLock(&(pFilterContext->PacketListSpinLock));

		if(pFilterContext->PacketCount >= PACKET_LIMIT) 
		{
			NdisFreeMemory( pPacketDataEntry->pPacketData, 0, 0);
			NdisFreeMemory( pPacketDataEntry, 0, 0);
			// release the lock
			NdisReleaseSpinLock(&(pFilterContext->PacketListSpinLock));
			goto ExitSavePacket;
		}

		// insert list item
		InsertTailList(&(pFilterContext->PacketListHead), &(pPacketDataEntry->LinkField));
		(pFilterContext->PacketCount)++;
		++pFilterContext->IPv4Stats.PTBufferedPkts;  // SpinLock Already Held

		// release the lock
		NdisReleaseSpinLock(&(pFilterContext->PacketListSpinLock));

	ExitSavePacket:			
			;
	}



ExitTheFilter:
   //
   // Release Adapter Spin Lock After Filtering
   //
   NdisDprReleaseSpinLock(&pAdapt->Lock);

   return( RcvFltAction );
}

/////////////////////////////////////////////////////////////////////////////
//// FltFilterReceive
//
// Purpose
//   Called to filter each received packet indication.
//
// Parameters
//    pAdapt - pointer to ADAPT structure that the send is on.
//    Others - See DDK ProtocolReceive documentation.
//
// Return Value
//    A ULONG containing the receive filter action bitmap defined
//    in filter.h
//
// Remarks
//   Called from PtReceive for each received packet indication.
//
//   We are using the structures and other definitions adapted from
//   FreeBSD 5.0 for some of the Internet information. See the files
//   in ../B2Winet.
//
//   Runs at IRQL DISPATCH_LEVEL.
//

ULONG
FltFilterReceive(
   IN PADAPT         pAdapt,
   IN NDIS_HANDLE    MacReceiveContext,
   IN PVOID          HeaderBuffer,
   IN UINT           HeaderBufferSize,
   IN PVOID          LookAheadBuffer,
   IN UINT           LookAheadBufferSize,
   IN UINT           PacketSize
   )
{
   PADAPT_FILTER_RSVD   pFilterContext;
   ULONG                RcvFltAction = RCV_FLT_SIMPLE_PASSTHRU;
   struct ether_header  *pEthHdr;      // See ../B2Winet/ethernet.h
   struct ip            *pIPHeader;    // See ../B2Winet/ip.h
   ULONG                PktSrcAddr;

   //
   // Find FilterReserved Area In ADAPT Structure
   //
   pFilterContext = (PADAPT_FILTER_RSVD )&pAdapt->FilterReserved;

   //
   // Hold Adapter Spin Lock When Using Filter Data
   // ---------------------------------------------
   // See note at ADAPT_FILTER_RSVD structure declaration.
   //
   NdisDprAcquireSpinLock(&pAdapt->Lock);


   ++pFilterContext->IPv4Stats.PTRcvCt;  // SpinLock Already Held

   //
   // Locate Ethernet Header
   //
   pEthHdr = (struct ether_header * )HeaderBuffer;

   //
   // Filter On EtherType
   //
   switch( ntohs( pEthHdr->ether_type ) )
   {
      // See ..\B2Winet/ethernet.h for ETHERTYPE Definitions
      case ETHERTYPE_IP:
         break;

      case ETHERTYPE_ARP:
      case ETHERTYPE_REVARP:
      case ETHERTYPE_NETBEUI:
      default:
         goto ExitTheFilter;
   }

   // check MAC filter (blocked or not)
   {
		MACAddrInfo currMACAddrInfo;
		ULONG i;

		//
		// Pass All Packets If No MAC Filter Is Set
		//
		if( !pFilterContext->pMACBlockAddrArray )
		{
			goto ExitMACFiltering;
		}

		for(i = 0; i < MAC_ADDRESS_SIZE; i++)
		{
			currMACAddrInfo.MACAddr[i] = pEthHdr->ether_shost[i];
		}

		if( MyMACSearch(&currMACAddrInfo, (pFilterContext->pMACBlockAddrArray)) )
		{
			RcvFltAction = RCV_FLT_BLOCK_PACKET;
			goto ExitTheFilter;
		}

	ExitMACFiltering:
			;
   }


   //
   // Pass All Packets If No Filter Is Set
   //
   if( !pFilterContext->pIPv4BlockAddrArray )
   {
      goto ExitTheFilter;
   }

   //
   // Locate IP Header
   //
   pIPHeader = (struct ip * )LookAheadBuffer;

   //
   // Only Filter IPv4 For Now...
   //
   switch( pIPHeader->ip_v )
   {
      case 4:        // IPv4
         break;

      case 6:        // IPv6
      default:
         goto ExitTheFilter;
   }

   PktSrcAddr = pIPHeader->ip_src.s_addr;

   //
   // No blocking of incomming packets - Asanga
   //
   // Do Binary Search On Sorted List Of IP Addresses To Block
   //
   //if( bsearch(
   //      &PktSrcAddr,                                          // Key To Search For
   //      (pFilterContext->pIPv4BlockAddrArray)->IPAddrArray,     // Array Base
   //      (pFilterContext->pIPv4BlockAddrArray)->NumberElements,  // Number Of Elements In Array
   //      sizeof( ULONG ),                                      // Bytes Per Element
   //      IPv4AddrCompare                                      // Comparison Function
   //      )
   //   )
   //{
      //
      // Reject Packets That Have Matching IP Source Address
      //
   //   ++pFilterContext->IPv4Stats.PTRcvDropped; // SpinLock Already Held
   //   RcvFltAction = RCV_FLT_BLOCK_PACKET;
   //}





	// place packet in list
	{
		PPacketDataEntry	pPacketDataEntry;
		NDIS_STATUS			nNdisStatus;
		ULONG				PacketSize, NumberOfBytesRead, i;
		

		if(LookAheadBufferSize < ntohs(pIPHeader->ip_len))
		{
			goto ExitSavePacket;
		}

		// get memory for the list item
		nNdisStatus = NdisAllocateMemoryWithTag(&pPacketDataEntry,
											sizeof(PacketDataEntry),
											TAG);
		if( nNdisStatus != NDIS_STATUS_SUCCESS )
		{
			goto ExitSavePacket;
		}

		// get memory for the IP packet
		pPacketDataEntry->pPacketData = NULL;
		pPacketDataEntry->PacketSize = 0;
		PacketSize = ntohs(pIPHeader->ip_len);
		nNdisStatus = NdisAllocateMemoryWithTag(&(pPacketDataEntry->pPacketData),
											PacketSize,
											TAG);
		if( nNdisStatus != NDIS_STATUS_SUCCESS )
		{
			NdisFreeMemory( pPacketDataEntry, 0, 0);
			goto ExitSavePacket;
		}

		NumberOfBytesRead = 0;

		// copy packet contents
		for(i = 0; i < PacketSize; i++)
		{
			 *(((UCHAR *) pPacketDataEntry->pPacketData) + i) 
					= *(((UCHAR *) LookAheadBuffer) + i);
		}

		pPacketDataEntry->PacketSize = PacketSize;
		pPacketDataEntry->PacketType = ALLOWED_PACKET;

		// acquire lock to add to list
		NdisAcquireSpinLock(&(pFilterContext->PacketListSpinLock));

		if(pFilterContext->PacketCount >= PACKET_LIMIT) 
		{
			NdisFreeMemory( pPacketDataEntry->pPacketData, 0, 0);
			NdisFreeMemory( pPacketDataEntry, 0, 0);
			// release the lock
			NdisReleaseSpinLock(&(pFilterContext->PacketListSpinLock));
			goto ExitSavePacket;
		}

		// insert list item
		InsertTailList(&(pFilterContext->PacketListHead), &(pPacketDataEntry->LinkField));
		(pFilterContext->PacketCount)++;
		++pFilterContext->IPv4Stats.PTBufferedPkts;  // SpinLock Already Held

		// release the lock
		NdisReleaseSpinLock(&(pFilterContext->PacketListSpinLock));

	ExitSavePacket:			
			;
	}







ExitTheFilter:
   //
   // Release Adapter Spin Lock After Filtering
   //
   NdisDprReleaseSpinLock(&pAdapt->Lock);

   return( RcvFltAction );
}

////////////////////////////////////////////////////////////////////////////
//                            Utility Functions                           //
////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//// FltReadOnPacket
//
// Purpose
// Logical read on the packet data in a NDIS_PACKET.
//
// Parameters
//
// Return Value
//
// Remarks
// The purpose of this function is to provide a convienient mechanism to
// read packet data from an NDIS_PACKET that may have multiple chained
// NDIS_BUFFERs.
//

VOID
FltReadOnPacket(
   IN PNDIS_PACKET Packet,
   IN PVOID lpBufferIn,
   IN ULONG nNumberOfBytesToRead,
   IN ULONG nOffset,                // Byte Offset, Starting With MAC Header
   OUT PULONG lpNumberOfBytesRead
   )
{
   PNDIS_BUFFER    CurrentBuffer;
   UINT            nBufferCount, TotalPacketLength;
   PUCHAR          VirtualAddress = NULL;
   PUCHAR          lpBuffer = (PUCHAR )lpBufferIn;
   UINT            CurrentLength, CurrentOffset;
   UINT            AmountToMove;

   //
   // Sanity Check
   //
   if( !Packet || !lpBuffer || !lpNumberOfBytesRead )
   {
      if( lpNumberOfBytesRead )
      {
         *lpNumberOfBytesRead = 0;
      }
      return;
   }

   *lpNumberOfBytesRead = 0;
   if (!nNumberOfBytesToRead)
      return;

   //
   // Query Packet
   //
   NdisQueryPacket(
      (PNDIS_PACKET )Packet,
      (PUINT )NULL,           // Physical Buffer Count
      (PUINT )&nBufferCount,  // Buffer Count
      &CurrentBuffer,         // First Buffer
      &TotalPacketLength      // TotalPacketLength
      );

   //
   // Query The First Buffer
   //
#if (defined(NDIS50) || defined(NDIS51))
   NdisQueryBufferSafe(
      CurrentBuffer,
      &VirtualAddress,
      &CurrentLength,
      NormalPagePriority
      );
#else
   NdisQueryBuffer(
      CurrentBuffer,
      &VirtualAddress,
      &CurrentLength
      );
#endif

   //
   // Handle Possible Low-Resource Failure Of NdisQueryBufferSafe
   //
   if( !VirtualAddress )
   {
      return;
   }

   __try
   {
      CurrentOffset = 0;

      while( nOffset || nNumberOfBytesToRead )
      {
         while( !CurrentLength )
         {
            NdisGetNextBuffer(
               CurrentBuffer,
               &CurrentBuffer
               );

            // If we've reached the end of the packet.  We return with what
            // we've done so far (which must be shorter than requested).
            if (!CurrentBuffer)
               __leave; // Leave __try and eventually return...

#if (defined(NDIS50) || defined(NDIS51))
            NdisQueryBufferSafe(
               CurrentBuffer,
               &VirtualAddress,
               &CurrentLength,
               NormalPagePriority
               );
#else
            NdisQueryBuffer(
               CurrentBuffer,
               &VirtualAddress,
               &CurrentLength
               );
#endif

            //
            // Handle Possible Low-Resource Failure Of NdisQueryBufferSafe
            //
            if( !VirtualAddress )
            {
               __leave; // Leave __try and eventually return...
            }

            CurrentOffset = 0;
         }

         if( nOffset )
         {
            // Compute how much data to move from this fragment
            if( CurrentLength > nOffset )
               CurrentOffset = nOffset;
            else
               CurrentOffset = CurrentLength;

            nOffset -= CurrentOffset;
            CurrentLength -= CurrentOffset;
         }

         if( nOffset )
         {
            CurrentLength = 0;
            continue;
         }

         if( !CurrentLength )
         {
            continue;
         }

         // Compute how much data to move from this fragment
         if (CurrentLength > nNumberOfBytesToRead)
            AmountToMove = nNumberOfBytesToRead;
         else
            AmountToMove = CurrentLength;

         // Copy the data.
         NdisMoveMemory(
            lpBuffer,
            &VirtualAddress[ CurrentOffset ],
            AmountToMove
            );

         // Update destination pointer
         lpBuffer += AmountToMove;

         // Update counters
         *lpNumberOfBytesRead +=AmountToMove;
         nNumberOfBytesToRead -=AmountToMove;
         CurrentLength = 0;
      }
   }
   __finally
   {
      //
      // lpNumberOfBytesRead may be less then specified if exception
      // occured...
      //
   }
}

/////////////////////////////////////////////////////////////////////////////
//// bsearch
//
// Purpose
// Does a binary search of a sorted array for a key.
//
// Parameters
//  pSearchKey       - key to search for
//  pArrayBase       - base of sorted array to search
//  nNumElements     - number of elements in array
//  nBytesPerElement - number of bytes per element
//  int (*compare)()   - pointer to function that compares two array
//          elements, returning neg when #1 < #2, pos when #1 > #2, and
//          0 when they are equal. Function is passed pointers to two
//          array elements.
//
// Return Value
//  if key is found:
//          returns pointer to occurrence of key in array
//  if key is not found:
//          returns NULL
//
// Remarks
//

PVOID bsearch(
   const PVOID pSearchKey,
   const PVOID pArrayBase,
   ULONG nNumElements,
   ULONG nBytesPerElement,
   BSEARCH_CMP_FCN compare
   )
{
   char *lo = (char *)pArrayBase;
   char *hi = (char *)pArrayBase + (nNumElements - 1) * nBytesPerElement;
   char *mid;
   unsigned int half;
   int result;

   while( lo <= hi )
   {
      if( half = nNumElements / 2 )
      {
         mid = lo + (nNumElements & 1 ? half : (half - 1)) * nBytesPerElement;
         if (!(result = (*compare)(pSearchKey,mid)))
         {
            return(mid);
         }
         else if (result < 0)
         {
            hi = mid - nBytesPerElement;
            nNumElements = nNumElements & 1 ? half : half-1;
         }
         else
         {
            lo = mid + nBytesPerElement;
            nNumElements = half;
         }
      }
      else if (nNumElements)
      {
         return((*compare)(pSearchKey,lo) ? NULL : lo);
      }
      else
      {
         break;
      }
   }

   return(NULL);
}

int MySearch(PIPAddrInfo pCheckIPAddrInfo, PIPv4BlockAddrArray pAddressArray)
{
	ULONG i;

	for(i = 0; i < pAddressArray->NumberElements; i++)
	{
		if((pCheckIPAddrInfo->IPAddr & pAddressArray->IPAddrInfoArray[i].MaskIPAddr)
			== (pAddressArray->IPAddrInfoArray[i].IPAddr &
							pAddressArray->IPAddrInfoArray[i].MaskIPAddr) )
		{
			return TRUE;
		}
	}
	return FALSE;
}

int MyMACSearch(PMACAddrInfo pCheckMACAddrInfo, PMACBlockAddrArray pMACAddressArray)
{
	ULONG i, j, gotcha;

	for(i = 0; i < pMACAddressArray->NumberElements; i++)
	{
		gotcha = TRUE;
		for(j = 0; j < MAC_ADDRESS_SIZE; j++)
		{
			if(pCheckMACAddrInfo->MACAddr[j] !=
					pMACAddressArray->MACAddrInfoArray[i].MACAddr[j])
			{
				gotcha = FALSE;
				break;
			}
		}
		if(gotcha)
		{
			return TRUE;
		}
	}
	return FALSE;
}


////////////////////////////////////////////////////////////////////////////
//                              Debug Functions                           //
////////////////////////////////////////////////////////////////////////////

#if DBG

#endif // DBG


