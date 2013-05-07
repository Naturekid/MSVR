
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
* FilterSetter.cpp
*
* @author : Asanga Udugama
* @date : 11-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/
#ifndef __FILTERSETTER__H
#define __FILTERSETTER__H

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

// PtDriver lock
extern CRITICAL_SECTION PtDriverLock;
extern LPCRITICAL_SECTION pPtDriverLock;

// AODV parameters
extern char ParaIfaceName[MAX_IFACE_SIZE];

extern CRITICAL_SECTION LoggingLock;
extern LPCRITICAL_SECTION pLoggingLock;
extern void LogMessage(int LoggingLevel, char *pPrintStr);

void SetFilter(PIPv4BlockAddrArray  pARFilter);
void ClearFilter();

#endif // __FILTERSETTER__H
