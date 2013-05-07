
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
* Utils.cpp
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#ifndef __UTILS__H
#define __UTILS__H

extern ULONG CurrLocalSeqNum;
extern ULONG CurrLocalRREQID;
extern IN_ADDR ParaIPAddress;
extern char ParaIfaceName[MAX_IFACE_SIZE];

extern ULONG RouteListSize(); 
extern ULONG UnexpiredRouteSize(); 
extern BOOL RouteListViewByIndex(PRouteListEntry pRouteListHead, ULONG Index, 
								PRouteEntry pRouteEntry);
extern BOOL PrecursorListViewByIndex(PPrecursorListEntry pPrecursorListHead, 
							  ULONG Index, 
							  PPrecursorEntry pPrecursorEntry);
extern RouteListEntry RouteListHead;
extern PRouteListEntry pRouteListHead;

char *BuildRREQAsString(PRREQMessage pRREQMsgInternal, char *BuildStr);
char *BuildRREPAsString(PRREPMessage pRREPMsgInternal, char *BuildStr);
char *BuildRERRAsString(PRERRMessage pRERRMsgInternal, char *BuildStr);
char *BuildRREPACKAsString(PRREPACKMessage pRREPACKMsgInternal, char *BuildStr);
char *BuildHELLOAsString(PHELLOMessage pHELLOMsgInternal, char *BuildStr);
char *BuildRouteEntryAsString(PRouteEntry pRouteEntry, char *BuildStr);
MILISECTIME GetCurMiliSecTime(); 
char *GetCurTimeForLogging(char *TimeStr);
ULONG IncrementSeqNum(ULONG *SeqNum);
ULONG IncrementRREQID(ULONG *RREQID);
void ShowRouteList();
void ShowRouteEntry(PRouteEntry pRouteEntry);
void DumpInternalRouteTable();
void DumpCharArray(char *pCharArray, int ArrayLen);
void DumpInternalRouteTableToGUI();

#endif // __UTILS__H
