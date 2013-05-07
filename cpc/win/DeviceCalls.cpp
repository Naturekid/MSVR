// TestIOCTL.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DeviceCalls.h"

// Copyright And Configuration Management ----------------------------------
//
//        PassThruEx IP Blocking Control Application - TestIOCTL.cpp
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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

static LPSTR MediumText[ ] = 
{
    "802.3",
    "802.5",
    "FDDI",
    "WAN",
    "LocalTalk",
    "DIX",              // defined for convenience, not a real medium
    "Arcnet Raw",
    "Arcnet 878.2",
    "ATM",
    "Wireless WAN",
    "Irda",
    "Bpc",
    "CoWan",
    "IEEE 1394 (Firewire)",
    "Unknown"
};

#define  MaxMediumText  (sizeof( MediumText )/sizeof( LPSTR ) )


/////////////////////////////////////////////////////////////////////////////
//// DisplayAdapterInfo
//
// Purpose
// Display some human-readable information about the specified adapter.
//
// Parameters
//
//   hAdapter - Handle returned from PtOpenAdapter.
//
// Return Value
//
// Remarks
//
//

DWORD
DisplayAdapterInfo( HANDLE hAdapter )
{
	UCHAR       OidData[4096];
	DWORD       nResult, ReturnedCount = 0;
	NDIS_MEDIUM NdisMedium;
	LPSTR       strMedium;
	WCHAR       VendorDesc[ 256 ];

	//
	// Query For Vendor Description
	//
	nResult = PtQueryInformation(
		hAdapter,
		OID_GEN_VENDOR_DESCRIPTION,
		VendorDesc,
		sizeof( VendorDesc ),
		&ReturnedCount
		);

	if( nResult != ERROR_SUCCESS )
	{
		printf( "Query For Vendor Description Failed; Error: 0x%8.8X\n", nResult );
		return( nResult );
	}

	wprintf( L"      Description: \042%*.*S\042\n",
		ReturnedCount, ReturnedCount, (LPWSTR )VendorDesc );

	//
	// Query For Medium In Use
	//
	nResult = PtQueryInformation(
		hAdapter,
		OID_GEN_MEDIA_IN_USE,
		OidData,
		sizeof( OidData ),
		&ReturnedCount
		);

	if( nResult != ERROR_SUCCESS )
	{
		printf( "Query For Media In Use Failed; Error: 0x%8.8X\n", nResult );
		return( nResult );
	}

	NdisMedium = *((PNDIS_MEDIUM )OidData);

	if( ReturnedCount >= sizeof( NDIS_MEDIUM ) )
	{
		if( NdisMedium < MaxMediumText )
		{
			strMedium = MediumText[ NdisMedium ];
		}
		else
		{
			strMedium = MediumText[ MaxMediumText - 1 ];
		}

		printf( "      Medium: %s; ", strMedium );
	}

	switch( NdisMedium )
	{
	case NdisMedium802_3:
		//
		// Query For 802.3 Current Address
		//
		nResult = PtQueryInformation(
			hAdapter,
			OID_802_3_CURRENT_ADDRESS,
			OidData,
			sizeof( OidData ),
			&ReturnedCount
			);

		if( nResult != ERROR_SUCCESS )
		{
			printf( "Query For Current Address Failed; Error: 0x%8.8X\n", nResult );
			return( nResult );
		}

		if (ReturnedCount == 6)
		{
			printf(
				"Mac address = %02.2X-%02.2X-%02.2X-%02.2X-%02.2X-%02.2X\n",
				OidData[0], OidData[1], OidData[2], OidData[3],
				OidData[4], OidData[5], OidData[6], OidData[7]
				);
		}
		else
		{
			printf(
				"DeviceIoControl returned an invalid count = %d\n",
				ReturnedCount
				);
		}
		break;

	case NdisMediumWan:
		//
		// Query For WAN Current Address
		//
		nResult = PtQueryInformation(
			hAdapter,
			OID_WAN_CURRENT_ADDRESS,
			OidData,
			sizeof( OidData ),
			&ReturnedCount
			);

		if( nResult != ERROR_SUCCESS )
		{
			printf( "Query For Current Address Failed; Error: 0x%8.8X\n", nResult );
			return( nResult );
		}

		if (ReturnedCount == 6)
		{
			printf(
				"Mac address = %02.2X-%02.2X-%02.2X-%02.2X-%02.2X-%02.2X\n",
				OidData[0], OidData[1], OidData[2], OidData[3],
				OidData[4], OidData[5], OidData[6], OidData[7]
				);
		}
		else
		{
			printf(
				"DeviceIoControl returned an invalid count = %d\n",
				ReturnedCount
				);
		}
		break;

	default:
		printf( "Mac address: Query Not Supported By Application...\n" );
		break;
	}

	printf( "\n" );

	return( nResult );
}


/////////////////////////////////////////////////////////////////////////////
//// ShowBindings
//
// Purpose
// Enumerate all PassThru bindings and show some information about each.
//
// Parameters
//
// Return Value
//
// Remarks
//
//

void ShowBindings()
{
	//
	// Open A Handle On The PassThru Device
	//
	HANDLE PtHandle = PtOpenControlChannel();

	if( PtHandle == INVALID_HANDLE_VALUE )
	{
		_tprintf( _T("Unable to open handle on PassThru device\n") );

		return;
	}

	//
	// Enumerate The PassThru Bindings
	//
	WCHAR  BindingList[ 2048 ];
	DWORD  BufLength = sizeof( BindingList );

	if( PtEnumerateBindings( PtHandle, (PCHAR )BindingList, &BufLength ) )
	{
		PWCHAR   pWStr = BindingList;
		UINT     nWCHARsRead;
		INT      nBytesUnread = BufLength;

		if( !BufLength )
		{
			_tprintf( _T("Enumeration is empty\n") );
		}
		else
		{
			cout << endl << "Driver Bindings:" << endl;

			while( pWStr && *pWStr && nBytesUnread > 0 )
			{
				//
				// Display Virtual Adapter Name
				// ----------------------------
				// This is the name passed to NdisIMInitializeDeviceInstanceEx.
				// We can call this our "virtual adapter name".
				//
				printf( "   \042%ws\042\n", pWStr );

				//
				// Advance In Buffer
				//
				nWCHARsRead = (ULONG )wcslen( pWStr ) + 1;
				nBytesUnread -= nWCHARsRead * (ULONG )sizeof( WCHAR );

				if( nBytesUnread <= 0 )
				{
					pWStr = NULL;
				}
				else
				{
					pWStr += nWCHARsRead;
				}

				if( !( pWStr && *pWStr && nBytesUnread > 0 ) )
				{
					_tprintf( _T("Unexpected enumeration termination\n") );

					break;
				}

				//
				// Display Lower Adapter Name
				// --------------------------
				// This is the name passed to NdisOpenAdapter. We call this the
				// "lower adapte name".
				//
				printf( "      \042%ws\042\n", pWStr );

				//
				// Open A PassThru Handle Associated With The Lower Adapter
				//
				HANDLE hLowerAdapter;

				hLowerAdapter = PtOpenAdapterW( pWStr );

				if( hLowerAdapter != INVALID_HANDLE_VALUE )
				{
					//
					// Display Human-Readable Information
					// ----------------------------------
					// This function uses PtQueryInformation to fetch information from the
					// lower miniport.
					//
					DisplayAdapterInfo( hLowerAdapter );

					PtCloseAdapter( hLowerAdapter );
				}

				//
				// Advance In Buffer
				//
				nWCHARsRead = (ULONG )wcslen( pWStr ) + 1;
				nBytesUnread -= nWCHARsRead * (ULONG )sizeof( WCHAR );

				if( nBytesUnread <= 0 )
				{
					pWStr = NULL;
				}
				else
				{
					pWStr += nWCHARsRead;
				}
			}
		}
	}
	else
	{
		cout << endl << "PtEnumerateBindings Failed" << endl;
	}

	CloseHandle( PtHandle );
}

int IPAddressCompare( const void *pKey, const void *pElement )
{
	//ULONG a1 = *(PULONG )pKey;
	//ULONG a2 = *(PULONG )pElement;

	PIPAddrInfo pIPAddrInfoKey = (PIPAddrInfo ) pKey;
	PIPAddrInfo pIPAddrInfoElement = (PIPAddrInfo ) pElement;
	ULONG a3 = pIPAddrInfoKey->IPAddr & pIPAddrInfoKey->MaskIPAddr;
	ULONG a4 = pIPAddrInfoElement->IPAddr & pIPAddrInfoElement->MaskIPAddr;
	ULONG a1 = pIPAddrInfoKey->IPAddr & a3;
	ULONG a2 = pIPAddrInfoElement->IPAddr & a4;


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
//// ClearAllIPv4BlockingFilters
//
// Purpose
// Clear all IPv4 IP address blocking lists on all adapters.
//
// Parameters
//
// Return Value
//
// Remarks
//
//

void ClearAllIPv4BlockingFilters()
{
	//
	// Open A Handle On The PassThru Device
	//
	HANDLE PtHandle = PtOpenControlChannel();

	if( PtHandle == INVALID_HANDLE_VALUE )
	{
		_tprintf( _T("Unable to open handle on PassThru device\n") );

		return;
	}

	//
	// Enumerate The PassThru Bindings
	//
	WCHAR  BindingList[ 2048 ];
	DWORD  BufLength = sizeof( BindingList );

	if( PtEnumerateBindings( PtHandle, (PCHAR )BindingList, &BufLength ) )
	{
		LPWSTR   pWStr = BindingList;
		UINT     nWCHARsRead;
		INT      nBytesUnread = BufLength;

		if( !BufLength )
		{
			_tprintf( _T("Enumeration is empty\n") );
		}
		else
		{
			cout << endl << "Driver Bindings:" << endl;

			while( pWStr && *pWStr && nBytesUnread > 0 )
			{
				//
				// Display Virtual Adapter Name
				// ----------------------------
				// This is the name passed to NdisIMInitializeDeviceInstanceEx.
				// We can call this our "virtual adapter name".
				//
				printf( "   \042%ws\042\n", pWStr );

				//
				// Advance In Buffer
				//
				nWCHARsRead = (ULONG )wcslen( pWStr ) + 1;
				nBytesUnread -= nWCHARsRead * (ULONG )sizeof( WCHAR );

				if( nBytesUnread <= 0 )
				{
					pWStr = NULL;
				}
				else
				{
					pWStr += nWCHARsRead;
				}

				if( !( pWStr && *pWStr && nBytesUnread > 0 ) )
				{
					_tprintf( _T("Unexpected enumeration termination\n") );

					break;
				}

				//
				// Display Lower Adapter Name
				// --------------------------
				// This is the name passed to NdisOpenAdapter. We call this the
				// "lower adapte name".
				//
				printf( "      \042%ws\042\n", pWStr );

				//
				// Open A PassThru Handle Associated With The Lower Adapter
				//
				HANDLE hLowerAdapter;

				hLowerAdapter = PtOpenAdapterW( pWStr );

				if( hLowerAdapter != INVALID_HANDLE_VALUE )
				{
					PtSetIPv4BlockingFilter( hLowerAdapter, NULL );

					PtCloseAdapter( hLowerAdapter );
				}

				//
				// Advance In Buffer
				//
				nWCHARsRead = (ULONG )wcslen( pWStr ) + 1;
				nBytesUnread -= nWCHARsRead * sizeof( WCHAR );

				if( nBytesUnread <= 0 )
				{
					pWStr = NULL;
				}
				else
				{
					pWStr += nWCHARsRead;
				}
			}
		}
	}
	else
	{
		cout << endl << "PtEnumerateBindings Failed" << endl;
	}

	CloseHandle( PtHandle );
}

void DisplayIPV4Statistics()
{
	//
	// Open A Handle On The PassThru Device
	//
	HANDLE PtHandle = PtOpenControlChannel();

	if( PtHandle == INVALID_HANDLE_VALUE )
	{
		_tprintf( _T("Unable to open handle on PassThru device\n") );

		return;
	}

	//
	// Enumerate The PassThru Bindings
	//
	WCHAR  BindingList[ 2048 ];
	DWORD  BufLength = sizeof( BindingList );

	if( PtEnumerateBindings( PtHandle, (PCHAR )BindingList, &BufLength ) )
	{
		LPWSTR   pWStr = BindingList;
		UINT     nWCHARsRead;
		INT      nBytesUnread = BufLength;

		if( !BufLength )
		{
			_tprintf( _T("Enumeration is empty\n") );
		}
		else
		{
			cout << endl << "Driver Bindings:" << endl;

			while( pWStr && *pWStr && nBytesUnread > 0 )
			{
				//
				// Display Virtual Adapter Name
				// ----------------------------
				// This is the name passed to NdisIMInitializeDeviceInstanceEx.
				// We can call this our "virtual adapter name".
				//
				printf( "   \042%ws\042\n", pWStr );

				//
				// Advance In Buffer
				//
				nWCHARsRead = (ULONG )wcslen( pWStr ) + 1;
				nBytesUnread -= nWCHARsRead * (ULONG )sizeof( WCHAR );

				if( nBytesUnread <= 0 )
				{
					pWStr = NULL;
				}
				else
				{
					pWStr += nWCHARsRead;
				}

				if( !( pWStr && *pWStr && nBytesUnread > 0 ) )
				{
					_tprintf( _T("Unexpected enumeration termination\n") );

					break;
				}

				//
				// Display Lower Adapter Name
				// --------------------------
				// This is the name passed to NdisOpenAdapter. We call this the
				// "lower adapte name".
				//
				printf( "      \042%ws\042\n", pWStr );

				//
				// Open A PassThru Handle Associated With The Lower Adapter
				//
				HANDLE hLowerAdapter;

				hLowerAdapter = PtOpenAdapterW( pWStr );

				if( hLowerAdapter != INVALID_HANDLE_VALUE )
				{
               BOOL  bResult;
               IPv4AddrStats IPv4Stats;

               bResult = PtQueryIPv4Statistics(
                           hLowerAdapter,
                           &IPv4Stats
                           );

               if( bResult )
               {
                  printf( "\n" );
                  printf( "Total Packets Sent        : %d\n", IPv4Stats.MPSendPktsCt );
                  printf( "   Send Packets Blocked   : %d\n", IPv4Stats.MPSendPktsDropped );
                  printf( "   Send Packets Buffered  : %d\n", IPv4Stats.PTBufferedPkts );
                  printf( "Total Packets Received    : %d\n",
                     IPv4Stats.PTRcvCt + IPv4Stats.PTRcvPktCt );
                  printf( "   Receive Packets Blocked: %d\n",
                     IPv4Stats.PTRcvDropped + IPv4Stats.PTRcvPktDropped );

                  printf( "\n" );
               }

					PtCloseAdapter( hLowerAdapter );
				}

				//
				// Advance In Buffer
				//
				nWCHARsRead = (ULONG )wcslen( pWStr ) + 1;
				nBytesUnread -= nWCHARsRead * sizeof( WCHAR );

				if( nBytesUnread <= 0 )
				{
					pWStr = NULL;
				}
				else
				{
					pWStr += nWCHARsRead;
				}
			}
		}
	}
	else
	{
		cout << endl << "PtEnumerateBindings Failed" << endl;
	}

	CloseHandle( PtHandle );
}





void DisplayStoredIPV4Packet()
{
	//
	// Open A Handle On The PassThru Device
	//
	HANDLE PtHandle = PtOpenControlChannel();

	if( PtHandle == INVALID_HANDLE_VALUE )
	{
		_tprintf( _T("Unable to open handle on PassThru device\n") );

		return;
	}

	//
	// Enumerate The PassThru Bindings
	//
	WCHAR  BindingList[ 2048 ];
	DWORD  BufLength = sizeof( BindingList );

	if( PtEnumerateBindings( PtHandle, (PCHAR )BindingList, &BufLength ) )
	{
		LPWSTR   pWStr = BindingList;
		UINT     nWCHARsRead;
		INT      nBytesUnread = BufLength;

		printf( "\n" );
		if( !BufLength )
		{
			_tprintf( _T("Enumeration is empty\n") );
		}
		else
		{
			cout << endl << "Driver Bindings:" << endl;

			while( pWStr && *pWStr && nBytesUnread > 0 )
			{
				//
				// Display Virtual Adapter Name
				// ----------------------------
				// This is the name passed to NdisIMInitializeDeviceInstanceEx.
				// We can call this our "virtual adapter name".
				//
				printf( "   \042%ws\042\n", pWStr );

				//
				// Advance In Buffer
				//
				nWCHARsRead = (ULONG )wcslen( pWStr ) + 1;
				nBytesUnread -= nWCHARsRead * (ULONG )sizeof( WCHAR );

				if( nBytesUnread <= 0 )
				{
					pWStr = NULL;
				}
				else
				{
					pWStr += nWCHARsRead;
				}

				if( !( pWStr && *pWStr && nBytesUnread > 0 ) )
				{
					_tprintf( _T("Unexpected enumeration termination\n") );

					break;
				}

				//
				// Display Lower Adapter Name
				// --------------------------
				// This is the name passed to NdisOpenAdapter. We call this the
				// "lower adapte name".
				//
				printf( "      \042%ws\042\n", pWStr );

				//
				// Open A PassThru Handle Associated With The Lower Adapter
				//
				HANDLE hLowerAdapter;

				hLowerAdapter = PtOpenAdapterW( pWStr );

				if( hLowerAdapter != INVALID_HANDLE_VALUE )
				{
					BOOL			bResult;
					char			PacketData[4096];
					ULONG			ActualBufferSize;
					ULONG			i, CharCount;
					IPv4AddrStats IPv4Stats;

					bResult = PtGetStoredIPv4Packet(hLowerAdapter,
														PacketData,
														4096,
														&ActualBufferSize);
					// get one packet
					if( bResult )
					{
						CharCount = 0;
						printf( "\n" );
						printf( "Packet Contents" );
						printf( "\n" );
						for(i = 0; i < ActualBufferSize; i++) 
						{
							printf("%02x ", (unsigned char) PacketData[i]);
							CharCount++;
							if(CharCount >= 16) 
							{
								printf( "\n" );
								CharCount = 0;
							}

						}
						printf( "\n" );

					} 
					else
					{
						printf( "\n" );
						printf( "No more packets left" );
						printf( "\n" );
					}



					bResult = PtQueryIPv4Statistics(
											hLowerAdapter,
											&IPv4Stats
											);

					if( bResult )
					{
						printf( "\n" );
						printf( "Total Packets Sent        : %d\n", IPv4Stats.MPSendPktsCt );
						printf( "   Send Packets Blocked   : %d\n", IPv4Stats.MPSendPktsDropped );
						printf( "   Send Packets Buffered  : %d\n", IPv4Stats.PTBufferedPkts );
						printf( "Total Packets Received    : %d\n",
									IPv4Stats.PTRcvCt + IPv4Stats.PTRcvPktCt );
						printf( "   Receive Packets Blocked: %d\n",
									IPv4Stats.PTRcvDropped + IPv4Stats.PTRcvPktDropped );

						printf( "\n" );
					}

					PtCloseAdapter( hLowerAdapter );
				}

				//
				// Advance In Buffer
				//
				nWCHARsRead = (ULONG )wcslen( pWStr ) + 1;
				nBytesUnread -= nWCHARsRead * sizeof( WCHAR );

				if( nBytesUnread <= 0 )
				{
					pWStr = NULL;
				}
				else
				{
					pWStr += nWCHARsRead;
				}
			}
		}
	}
	else
	{
		cout << endl << "PtEnumerateBindings Failed" << endl;
	}

	CloseHandle( PtHandle );
}



/////////////////////////////////////////////////////////////////////////////
//// SetIPv4BlockList
//
// Purpose
// Set an IP address blocking list on the named adapter.
//
// Parameters
//
// Return Value
//
// Remarks
//
//

BOOL
SetIPv4BlockList(
   LPTSTR pszAdapterName,
   PIPv4BlockAddrArray pIPv4BlockAddrArray
   )
{
	if( !pszAdapterName )
	{
		return( FALSE );
	}

	if( !pIPv4BlockAddrArray )
	{
		return( FALSE );
	}

	HANDLE hLowerAdapter = PtOpenAdapter( pszAdapterName );

	if( hLowerAdapter != INVALID_HANDLE_VALUE )
	{
		PtSetIPv4BlockingFilter( hLowerAdapter, pIPv4BlockAddrArray );

		PtCloseAdapter( hLowerAdapter );

		return( TRUE );
	}

	return( FALSE );
}

BOOL
ReadIPv4BlockList(
   LPTSTR pARFileName,
   LPTSTR *hAdapterName,
   HIPv4BlockAddrArray hIPv4BlockAddrArray
   )
{
	FILE                 *pARFile;
	CHAR                 *s, *t;
	CHAR                 line[ 128 ], str1[ 64 ], str2[ 64 ];
	ULONG                nIPAddress, nMaskIPAddress;
	TCHAR                *pszAdapterName = NULL;
	ULONG                nFilterSize, nFreeAddrSlot;
	PIPv4BlockAddrArray  pARFilter = NULL;

	if( !hAdapterName )
	{
		return( FALSE );
	}

	*hAdapterName = NULL;

	if( !hIPv4BlockAddrArray )
	{
		return( FALSE );
	}

	*hIPv4BlockAddrArray = NULL;

	//
	// Open The Accept/Reject Filter Database
	//
	pARFile = _tfopen( pARFileName, _T("r") );

	if( !pARFile )
	{
		printf( "Couldn't open AR File %s\n", pARFileName );
		return( FALSE );
	}

	//
	// Read Lines On The Accept/Reject Filter Database
	//
	while( fgets( line, 128, pARFile ) != NULL )
	{
		s = line;

		//
		// Strip Leading Whitespace
		//
		while( *s != '\0' && isspace( *s ) )
		{
			++s;
		}

		//
		// Strip Trailing Whitespace
		//
		t = s;
		t += strlen( s );
		--t;

		while( t > s )
		{
			if( isspace( *t ) )
			{
				*t-- = '\0';
			}
			else
			{
				break;
			}
		}

		//
		// Skip Empty Lines
		//
		if( *s == '\0' )
		{
			continue;
		}

		//
		// Skip Comment Lines
		//
		if( *s == '#' )
		{
			continue;
		}

		if(sscanf(s, "%s %s ", str1, str2) == 2 
			&& (nIPAddress = inet_addr( str1 )) != INADDR_NONE)
		{
			nMaskIPAddress = inet_addr( str2 );
		//nIPAddress = inet_addr( s );  // ATTENTION!!! TCHAR Problem!!!

		//if( nIPAddress != INADDR_NONE )
		//{
			//
			// Handle IP Address Lines
			//
			if( !pARFilter )
			{
				//
				// Make First Buffer Allocation
				//
				nFilterSize = sizeof( IPv4BlockAddrArray );
				pARFilter = (PIPv4BlockAddrArray )malloc( nFilterSize );

				if( pARFilter )
				{
					pARFilter->NumberElements = 0;
				}

				nFreeAddrSlot = 0;
			}

			if( !pARFilter )
			{
				printf( "Alloc Failure\n" );
				return( FALSE );
			}

			if( !nFreeAddrSlot )
			{
				//nFilterSize += sizeof( ULONG ) * 16;
				nFilterSize += sizeof( IPAddrInfo ) * 16;
				nFreeAddrSlot = 16;

				pARFilter = (PIPv4BlockAddrArray )realloc( pARFilter, nFilterSize );
			}

			//
			// Add An IP Address To The Buffer
			//
			//pARFilter->IPAddrArray[ pARFilter->NumberElements++ ] =
			//	nIPAddress;
			pARFilter->IPAddrInfoArray[ pARFilter->NumberElements ].IPAddr =
				nIPAddress;
			pARFilter->IPAddrInfoArray[ pARFilter->NumberElements ].MaskIPAddr =
				nMaskIPAddress;
			pARFilter->NumberElements++;
			--nFreeAddrSlot;
		}
		else
		{
			//
			// Handle Non-IP Address Lines
			// ---------------------------
			// The only non-IP address line should be the first non-comment
			// line. It must specify the adapter.
			//
			if( pszAdapterName )
			{
				printf( "Malformed Data File\n" );

				if( pARFilter )
				{
					free( pARFilter );
				}

				return( FALSE );
			}

			//
			// Save A Copy Of The Adapter Name String
			//
			pszAdapterName = _tcsdup( s );
		}
	}

	if( !pARFilter || !pszAdapterName )
	{
		//
		// Nothing To Do!!!
		//
		if( pszAdapterName )
		{
			free( pszAdapterName );
		}

		if( pARFilter )
		{
			free( pARFilter );
		}

		return( TRUE );   // Success (Did nothing...)
	}

	if( pszAdapterName )
	{
		*hAdapterName = pszAdapterName;
	}

	if( hIPv4BlockAddrArray )
	{
		*hIPv4BlockAddrArray = pARFilter;
	}

	//
	// Sort The IP Address Array
	// -------------------------
	// This is necessary because the driver will use a binary search on
	// the array.
	//
	//qsort( pARFilter->IPAddrArray, pARFilter->NumberElements, sizeof( ULONG ), IPAddressCompare );
	qsort( pARFilter->IPAddrInfoArray, pARFilter->NumberElements, sizeof( IPAddrInfo ), IPAddressCompare );

	//
	// Display The Sorted Filter List
	//
	printf( "Sorted IP Address List       :\n" );
	for(unsigned int i = 0; i < pARFilter->NumberElements; i++) {
			//struct in_addr in;
			ULONG ipval, maskval, maskedval;

			ipval = pARFilter->IPAddrInfoArray[i].IPAddr;
			//printf(" %s ", inet_ntoa(struct in);
			maskval = pARFilter->IPAddrInfoArray[i].MaskIPAddr;
			maskedval = ipval & maskval;  
			printf(" ");
	}

	fclose( pARFile );

	return( TRUE );
}

