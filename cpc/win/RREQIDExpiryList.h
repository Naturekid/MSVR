
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
* RREQIDExpiryList.cpp
*
* @author : Asanga Udugama
* @date : 11-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#ifndef __RREQIDEXPIRYLIST__H
#define __RREQIDEXPIRYLIST__H

BOOL RREQIDExpiryListInit(PRREQIDExpiryListEntry pRREQIDExpiryListHead);
BOOL RREQIDExpiryListInsertAtOrder(PRREQIDExpiryListEntry pRREQIDExpiryListHead, 
								 PRREQIDExpiryEntry pRREQIDExpiryEntry);
BOOL RREQIDExpiryListRemoveFromTail(PRREQIDExpiryListEntry pRREQIDExpiryListHead, 
									PRREQIDExpiryEntry pRREQIDExpiryEntry);
ULONG RREQIDExpiryListSize(); 
BOOL RREQIDExpiryListTerm(PRREQIDExpiryListEntry pRREQIDExpiryListHead);


#endif // __RREQIDEXPIRYLIST__H
