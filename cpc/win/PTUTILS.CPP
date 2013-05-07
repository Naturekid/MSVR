/////////////////////////////////////////////////////////////////////////////
//// INCLUDE FILES

#include "stdafx.h"
//#include "UoBWinAODV.h"
#include <ntddndis.h>
#include "IoCommon.h"

//
// Copyright (c) 2004 ComNets, University of Bremen. (for modifications)
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
//               PassThruEx Support Utilities - ptutils.cpp
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
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//// PtOpenControlChannel
//
// Purpose
// Open a "control channel" handle on the PassThru device.
//
// Parameters
//    None
//
// Return Value
//   The INVALIE_HANDLE_VALUE if unsuccessful. Otherwise, a valid handle
//   to the passthru device.
//
// Remarks
//   There are no parameters to this function because the PassThru filespec
//   name is already known. For this is "\\.\PassThru" or "\\.\Global\PassThru".
//
//   This call opens a "control channel". That is, a handle that can be
//   used for DeviceIoControl calls but is not associated with a specific
//   adapter.
//
//   Notice that the FILE_FLAG_OVERLAPPED attribute is not specified. The
//   returned handle is used for synchronous operations only.
//
//   A more sophisticated API would employ asynchronous I/O. However, a
//   sample of that complexity is beyond the scope of this article.
//

HANDLE
PtOpenControlChannel( void )
{
	DWORD	DesiredAccess;
	DWORD	ShareMode;
	LPSECURITY_ATTRIBUTES	lpSecurityAttributes = NULL;

	DWORD	CreationDistribution;
	DWORD	FlagsAndAttributes;
	HANDLE	TemplateFile;
	HANDLE	Handle;

	//
	// Use CreateFile to Open the Handle
	//
	DesiredAccess = GENERIC_READ|GENERIC_WRITE;
	ShareMode = 0;
	CreationDistribution = OPEN_EXISTING;
	FlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
	TemplateFile = (HANDLE)INVALID_HANDLE_VALUE;

	wchar_t *path = (wchar_t *)"\\\\.\\PassThru";

	Handle = CreateFile(
		path,
		DesiredAccess,
		ShareMode,
		lpSecurityAttributes,
		CreationDistribution,
		FlagsAndAttributes,
		TemplateFile
		);

	if( Handle == INVALID_HANDLE_VALUE )
	{
		//
		// Special Handling For Accessing Device On Windows 2000 Terminal Server
		// ---------------------------------------------------------------------
		// See Microsoft KB Article 259131
		//
		wchar_t *path = (wchar_t *)"\\\\.\\Globals\\PassThru";

		Handle = CreateFile(
			path,
			DesiredAccess,
			ShareMode,
			lpSecurityAttributes,
			CreationDistribution,
			FlagsAndAttributes,
			TemplateFile
			);
	}

	return (Handle);
}


/////////////////////////////////////////////////////////////////////////////
//// PtEnumerateBindings
//
// Purpose
// Use DeviceIoControl to query the PassThru device for a list of its
// current bindings.
//
// Parameters
//   PtHandle  - Handle returned from a successful call to PtOpenControlChannel.
//   Buf       - Pointer to a unsigned character array to be filled with
//               the PassThru driver's binding information.
//   BufLength - Pointer to a DWORD variable. On input, the caller must
//               initialize the variable with the size (in bytes) of the
//               buffer pointer to by NameBuffer, If the call is successful,
//               the variable will be filled with the number of bytes written
//               to NameBuffer.
//
// Return Value
//   Returns TRUE if the I/O operation was successful. In this case the
//   variable pointed to by BufLength is used to return the number of
//   bytes written to Buff.
//
//   Returns FALSE if the operation was unsuccessful. In this case assitional
//   error information can be fetched by calling GetLastError. The error
//   STATUS_BUFFER_TOO_SHORT indicated that the buffer passed to the function
//   was too short to contain the complete list of binding names. Making a second
//   call with a larger buffer may be successful.
//
// Remarks
//   If successful the buffer is filled with multiple wide-character strings
//   with the end of the buffer identified by an empty string.
//
//   Each binding made in the driver is represented by two strings (a tuple)
//   in the buffer:
//
//     Virtual Adapter Name - The name passed to NdisIMInitializeDeviceInstanceEx
//                            for the binding.
//     Lower Adapter Name   - The name passed to NdisOpenAdapter for the binding.
//

BOOL
PtEnumerateBindings(
   HANDLE PtHandle,
   PCHAR  NameBuffer,
   DWORD *NameBufferLength
   )
{
	BOOL	 bResult;

	//
	// Use DeviceIoControl to Call The Device
	//
	bResult = DeviceIoControl(
		PtHandle,
		IOCTL_PTUSERIO_ENUMERATE,
		NULL,
		0,
		NameBuffer,
		*NameBufferLength,
		NameBufferLength,
		NULL
		);

	return( bResult );
}


/////////////////////////////////////////////////////////////////////////////
//// PtOpenAdapterW
//
// Purpose
// To open a handle on the PassThru device and associate it with the specificed
// lower aadapter.
//
// Parameters
//   pszAdapterName - Wide character NULL terminated lower adapter name string.
//
// Return Value
// If the function succeeds, the return value is an open handle on the PassThru device that
// has been associated with the specified lower adapter.
//
// If the function fails, the return value is INVALID_HANDLE_VALUE.
//
// Remarks
// The adapter name must be a string returned from a prion PtEnumerateBindings call.
// This is because the string comparison call within the driver is case-sensitive using
// the kernel equivalent of memcmp.
//
// This design enforces exclusive access on a per-binding basis. Only one
// handle to a specific binding can be opened at any given time.
//
// The handle opened by this function must eventually be closed by calling the
// PtCloseAdapter function.
//
HANDLE
PtOpenAdapterW( LPWSTR pszAdapterName )
{
	HANDLE            hAdapter;
	BOOLEAN           bRc;
	ULONG             bytesReturned, nBufferLength, NdisStatus = 0;

	hAdapter = INVALID_HANDLE_VALUE;

	//
	// Open A Control Channel Handle On The PassThru Device
	// ----------------------------------------------------
	//   This call opens a "control channel". That is, a handle that can be
	//   used for DeviceIoControl calls but is not yet associated with a specifc
	//   adapter.
	//
	hAdapter = PtOpenControlChannel();

	if( hAdapter == INVALID_HANDLE_VALUE )
	{
		return( INVALID_HANDLE_VALUE );
	}

	//
	// Determine Length (Bytes) Of The Adapter Name
	// --------------------------------------------
	// Exclude terminating NULL.
	//
	nBufferLength = (ULONG )wcslen( (PWSTR )pszAdapterName ) * sizeof( WCHAR );

	//
	// Call Driver To Make Open The Adapter Context
	//
	bRc = DeviceIoControl(
		hAdapter, 
		(DWORD)IOCTL_PTUSERIO_OPEN_ADAPTER, 
		pszAdapterName, 
		nBufferLength,
		&NdisStatus,
		sizeof( NdisStatus ),
		&bytesReturned,
		NULL 
		);

	//
	// Check Results
	//
	if ( !bRc )
	{
		CloseHandle( hAdapter );

		return( INVALID_HANDLE_VALUE );
	}

	//
	// I/O Successful. Now Check NDIS Status
	//
	//   if( NdisStatus != NDIS_STATUS_SUCCESS )
	if( NdisStatus != 0 )
	{
		SetLastError( NdisStatus );

		CloseHandle( hAdapter );

		return( INVALID_HANDLE_VALUE );
	}

	return( hAdapter );     // Success
}

HANDLE
PtOpenAdapterA( LPSTR pszAdapterName )
{
	WCHAR AdapterName_W[ _MAX_PATH ];
	int   nBytesWritten;

	//
	// Convert Multi-Byte (ANSI) NDIS Name To Wide-Character
	//
	nBytesWritten = MultiByteToWideChar(
		CP_ACP,
		0,
		pszAdapterName,
		-1,
		AdapterName_W,
		sizeof( AdapterName_W )
		);

	if( !nBytesWritten )
	{
		return( INVALID_HANDLE_VALUE );
	}

	//
	// Call Wide-Character Vserion Of PtOpenAdapter
	//
	return( PtOpenAdapterW( AdapterName_W ) );
}

BOOL
PtCloseAdapter( HANDLE hAdapter )
{
	//
	// Close The Handle
	// ----------------
	// Future versions may perform additional work in this routine...
	//
	return( CloseHandle( hAdapter ) );
}


/////////////////////////////////////////////////////////////////////////////
//// PtQueryInformation
//
// Purpose
// Call the PassThu driver to make a NdisRequest to query information on
// the specified adapter.
//
// Parameters
//    hAdapter
//
//    Oidcode
//
//    InformationBuffer
//
//    InformationBufferLength
//
//    pBytesWritten
//
// Return Value
// Returns the value ERROR_SUCCESS if successful. Otherwise an error code.
//
// Remarks
// This function and the associated driver implementation only supports
// synchronous queries.
//

DWORD
PtQueryInformation(
	HANDLE   hAdapter,
	ULONG    OidCode,
	PVOID    InformationBuffer,
	UINT     InformationBufferLength,
	PULONG   pBytesWritten
	)
{
	DWORD       nResult = ERROR_SUCCESS;

	*pBytesWritten = 0;

	//
	// Make The DeviceIoControl Call
	//
	if( !DeviceIoControl(
		hAdapter,
		IOCTL_PTUSERIO_QUERY_INFORMATION,
		&OidCode,
		sizeof(OidCode),
		InformationBuffer,
		InformationBufferLength,
		pBytesWritten,
		NULL
		)
		)
	{
		//
		// DeviceIoControl returned an error
		//
		nResult = GetLastError();
	}

	return( nResult );
}


/////////////////////////////////////////////////////////////////////////////
//// PtSetIPv4BlockingFilter
//
// Purpose
// Call the PassThu driver to make a NdisRequest to set an IPv4 IP address
// blocking list on the specified adapter.
//
// Parameters
//    hAdapter
//
//    pIPv4BlockAddrArray
//
// Return Value
// Returns the value ERROR_SUCCESS if successful. Otherwise an error code.
//
// Remarks
// This function and the associated driver implementation only supports
// synchronous queries.
//

BOOL
PtSetIPv4BlockingFilter(
   HANDLE               hAdapter,
   PIPv4BlockAddrArray  pIPv4BlockAddrArray
   )
{
	BOOL	bResult;
	ULONG BlockingFilterLength = 0;

	if( pIPv4BlockAddrArray )
	{
		//BlockingFilterLength = sizeof( ULONG ) * ( pIPv4BlockAddrArray->NumberElements + 1 );
		BlockingFilterLength = sizeof( ULONG ) 
								+ (sizeof(IPAddrInfo) * pIPv4BlockAddrArray->NumberElements);
	}

	//
	// Use DeviceIoControl to Call The Device
	//
	bResult = DeviceIoControl(
		         hAdapter, 
		         IOCTL_PTUSERIO_SET_IPv4_BLOCK_FILTER,
		         pIPv4BlockAddrArray,
		         BlockingFilterLength,
		         NULL,
		         0,
		         &BlockingFilterLength,
		         NULL
		         );

	return( bResult );
}

BOOL
PtQueryIPv4Statistics(
   HANDLE               hAdapter,
   PIPv4AddrStats       pIPv4Stats
   )
{
	BOOL	bResult;
	ULONG StatsArrayLength = 0;

	if( pIPv4Stats )
	{
		StatsArrayLength = sizeof( IPv4AddrStats );
	}

	//
	// Use DeviceIoControl to Call The Device
	//
	bResult = DeviceIoControl(
		         hAdapter, 
		         IOCTL_PTUSERIO_QUERY_IPv4_BLOCK_STATISTICS,
		         NULL,
		         0,
		         pIPv4Stats,
		         StatsArrayLength,
		         &StatsArrayLength,
		         NULL
		         );

	return( bResult );
}

BOOL
PtGetStoredIPv4Packet(
   HANDLE               hAdapter,
   void *				pData,
   ULONG				BufferSize,
   ULONG *				ActualBufferSize
   )
{
	BOOL	bResult;
	ULONG	ArrayLength = 0;


	if( pData )
	{
		ArrayLength = BufferSize;
	}

	//
	// Use DeviceIoControl to Call The Device
	//
	bResult = DeviceIoControl(
		         hAdapter, 
		         IOCTL_PTUSERIO_GET_BUFFERED_IPv4_PACKET,
		         NULL,
		         0,
		         pData,
		         ArrayLength,
		         &ArrayLength,
		         NULL
		         );

	*ActualBufferSize = ArrayLength;

	return( bResult );
}
