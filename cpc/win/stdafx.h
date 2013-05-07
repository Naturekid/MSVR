
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
* Header file that includes all the required system header files
*
* @author : Asanga Udugama
* @date : 11-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

//#include <stdio.h>
#include <iostream>
#include <tchar.h>

//#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

//#ifndef VC_EXTRALEAN
//#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
//#endif

//#include <afx.h>
//#include <afxwin.h>         // MFC core and standard components
//#include <afxext.h>         // MFC extensions
//#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
//#ifndef _AFX_NO_AFXCMN_SUPPORT
//#include <afxcmn.h>			// MFC support for Windows Common Controls
//#endif // _AFX_NO_AFXCMN_SUPPORT

//#include <iostream>

// TODO: reference additional headers your program requires here
//#include <winioctl.h>
//#include <ntddndis.h>
#include <stdlib.h>
#include <malloc.h>
#include <Winsock2.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <windows.h>
#include <iphlpapi.h>
#include <MSTcpIP.h>
#include <WS2TcpIP.h>
#include <conio.h>
#include <ctype.h>
#include <time.h>



