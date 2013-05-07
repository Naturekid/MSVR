#pragma once

#include "resource.h"

#define  DEVICE_PREFIX_STR_A  "\\\\.\\"
#define  DEVICE_PREFIX_STR_W  L"\\\\.\\"

#define  DEVICE_STR_A         "\\Device\\"
#define  DEVICE_STR_W         L"\\Device\\"

#ifdef __cplusplus
extern "C" {
#endif

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

BOOL
PtQueryIPv4Statistics(
   HANDLE               hAdapter,
   PIPv4AddrStats       pIPv4Stats
   );

BOOL
PtGetStoredIPv4Packet(
   HANDLE               hAdapter,
   void *				pData,
   ULONG				BufferSize,
   ULONG *				ActualBufferSize
   );

#ifdef __cplusplus
}
#endif
