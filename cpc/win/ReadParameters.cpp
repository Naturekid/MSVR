
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
* Functions in this file provide services for obtaining configuration
* information from the config file.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "ReadParameters.h"

/*
* Reads the configuration file and places the information in the
* set of variables starting with "Para"
*/
BOOL ReadConfigFile() 
{
	FILE *fp;
	char Buffer[1024], PrintBuf[1024], Tag[1024], Value[1024];
	int LineNum;

	SetDefaultValues();

	fp = fopen(pConfigFileName, "r");
	if(fp == NULL)
	{
		sprintf(PrintBuf, "ReadParameters : Could not open file %s",
					pConfigFileName);
		LogMessageInitial(PrintBuf);
		return (FALSE);
	}

	LineNum = 0;
	while(fgets(Buffer, 1024, fp) != NULL)
	{
		LineNum++;

		// get tag & value
		if(!GetTagAndValue(Buffer, 1024, Tag, Value))
		{
			sprintf(PrintBuf, "ReadParameters : Could not understand line %n",
				 	LineNum);
			LogMessageInitial(PrintBuf);
			fclose(fp);
			return ( FALSE );
		}

		// disregard comment lines
		if(*Tag == '#')
		{
			continue;
		}

		// place config value in the variable
		if(!DecodeAndStoreParameter(Tag, Value))
		{
			sprintf(PrintBuf, "ReadParameters : Could not understand Tag %s and/or Value %s",
				 	Tag, Value);
			LogMessageInitial(PrintBuf);
			fclose(fp);
			return ( FALSE );
		}
	}

	fclose(fp);

	// get the computed parameters
	UpdateComputedParameters();

	return (TRUE);
}

/*
* Based on the given Tag, places the Value in the parameter
*/
BOOL DecodeAndStoreParameter(char *pTag, char *pValue)
{
	BOOL Rtn;

	Rtn = FALSE;
	if(_stricmp(pTag, "executionmode") == 0)
	{
		if(_stricmp(pValue, "gui") == 0) 
		{
			ParaExecutionMode = EXECUTION_MODE_GUI;
			Rtn = TRUE;
		}
		else if(_stricmp(pValue, "console") == 0)
		{
			ParaExecutionMode = EXECUTION_MODE_CONSOLE;
			Rtn = TRUE;
		}
	}
	else if(_stricmp(pTag, "ipversion") == 0)
	{
		if(_stricmp(pValue, "ipv4") == 0)
		{
			ParaIPVersion = IP_VERSION_4;
			Rtn = TRUE;
		}
		else if(_stricmp(pValue, "ipv6") == 0)
		{
			ParaIPVersion = IP_VERSION_6;
			Rtn = TRUE;
		}
	}
	else if(_stricmp(pTag, "ipaddress") == 0)
	{
		IN_ADDR TmpIPAddr;

		TmpIPAddr.S_un.S_addr = inet_addr(pValue);
		if(TmpIPAddr.S_un.S_addr != 0)
		{
			ParaIPAddress.S_un.S_addr = TmpIPAddr.S_un.S_addr;
			Rtn = TRUE;
		}
	}
	else if(_stricmp(pTag, "ifacename") == 0)
	{
		strcpy(ParaIfaceName, pValue);
		Rtn = TRUE;
	}
	else if(_stricmp(pTag, "gatewayipaddress") == 0)
	{
		IN_ADDR TmpIPAddr;

		TmpIPAddr.S_un.S_addr = inet_addr(pValue);
		if(TmpIPAddr.S_un.S_addr != 0)
		{
			ParaGatewayIPAddress.S_un.S_addr = TmpIPAddr.S_un.S_addr;
			Rtn = TRUE;
		}
	}
	else if(_stricmp(pTag, "loggingstatus") == 0)
	{
		if(_stricmp(pValue, "yes") == 0)
		{
			ParaLoggingStatus = YES;
			Rtn = TRUE;
		}
		else if(_stricmp(pValue, "no") == 0)
		{
			ParaLoggingStatus = NO;
			Rtn = TRUE;
		}
	}
	else if(_stricmp(pTag, "loggingmode") == 0)
	{
		if(ParaLoggingStatus == YES && _stricmp(pValue, "console") == 0)
		{
			ParaLoggingMode = LOGGING_MODE_CONSOLE;
			Rtn = TRUE;
		}
		else if(ParaLoggingStatus == YES && _stricmp(pValue, "file") == 0)
		{
			ParaLoggingMode = LOGGING_MODE_FILE;
			Rtn = TRUE;
		}
		else 
		{
			ParaLoggingMode = LOGGING_MODE_INITIAL;
			Rtn = TRUE;
		}
	}
	else if(_stricmp(pTag, "logginglevel") == 0)
	{
		int Level;

		Level = 0;
		Level = atoi(pValue);
		if(ParaLoggingStatus == YES &&
			Level >= LOGGING_SEVERAITY_MIN && Level <= LOGGING_SEVERAITY_MAX)
		{
			ParaLoggingLevel = Level;
			Rtn = TRUE;
		} 
		else 
		{
			ParaLoggingLevel = LOGGING_SEVERAITY_NONE;
			Rtn = TRUE;
		}
	}
	else if(_stricmp(pTag, "logfile") == 0)
	{
		if(ParaLoggingStatus == YES && ParaLoggingMode == LOGGING_MODE_FILE)
		{
			FILE *TmpFP;

			if((TmpFP = fopen(pValue, "a")) != NULL)
			{
				fclose(TmpFP);
				strcpy(ParaLogFile, pValue);
				Rtn = TRUE;

			}
		}
		else
		{
			strcpy(ParaLogFile, "");
			Rtn = TRUE;
		}
	}
	else if(_stricmp(pTag, "onlydestination") == 0)
	{
		if(_stricmp(pValue, "yes") == 0)
		{
			ParaOnlyDestination = YES;
			Rtn = TRUE;
		}
		else if(_stricmp(pValue, "no") == 0)
		{
			ParaOnlyDestination = NO;
			Rtn = TRUE;
		}
	}
	else if(_stricmp(pTag, "gratuitousrrep") == 0)
	{
		if(_stricmp(pValue, "yes") == 0)
		{
			ParaGratuitousRREP = YES;
			Rtn = TRUE;
		}
		else if(_stricmp(pValue, "no") == 0)
		{
			ParaGratuitousRREP = NO;
			Rtn = TRUE;
		}
	}
	else if(_stricmp(pTag, "rrepackrequired") == 0)
	{
		if(_stricmp(pValue, "yes") == 0)
		{
			ParaRREPAckRequired = YES;
			Rtn = TRUE;
		}
		else if(_stricmp(pValue, "no") == 0)
		{
			ParaRREPAckRequired = NO;
			Rtn = TRUE;
		}
	}
	else if(_stricmp(pTag, "ipaddressmulticast") == 0)
	{
		IN_ADDR TmpIPAddr;

		TmpIPAddr.S_un.S_addr = inet_addr(pValue);
		if(TmpIPAddr.S_un.S_addr != 0)
		{
			ParaIPAddressMulticast.S_un.S_addr = TmpIPAddr.S_un.S_addr;
			Rtn = TRUE;
		}
	}
	else if(_stricmp(pTag, "rerrsendingmode") == 0)
	{
		if(_stricmp(pValue, "unicast") == 0)
		{
			ParaRERRSendingMode = RERR_SEND_MODE_UNICAST;
			Rtn = TRUE;
		}
		else if(_stricmp(pValue, "multicast") == 0)
		{
			ParaRERRSendingMode = RERR_SEND_MODE_MULTICAST;
			Rtn = TRUE;
		}
	}
	else if(_stricmp(pTag, "deleteperiodmode") == 0)
	{
		if(_stricmp(pValue, "hello") == 0)
		{
			ParaDeletePeriodMode = DELETE_PERIOD_MODE_HELLO;
			Rtn = TRUE;
		}
		else if(_stricmp(pValue, "linklayer") == 0)
		{
			ParaDeletePeriodMode = DELETE_PERIOD_MODE_LINKLAYER;
			Rtn = TRUE;
		}
	}
	else if(_stricmp(pTag, "routediscoverymode") == 0)
	{
		if(_stricmp(pValue, "ers") == 0)
		{
			ParaRouteDiscoveryMode = ROUTE_DISCOVERY_MODE_ERS;
			Rtn = TRUE;
		}
		else if(_stricmp(pValue, "non-ers") == 0)
		{
			ParaRouteDiscoveryMode = ROUTE_DISCOVERY_MODE_NON_ERS;
			Rtn = TRUE;
		}
	}
	else if(_stricmp(pTag, "packetbuffering") == 0)
	{
		if(_stricmp(pValue, "yes") == 0)
		{
			ParaPacketBuffering = YES;
			Rtn = TRUE;
		}
		else if(_stricmp(pValue, "no") == 0)
		{
			ParaPacketBuffering = NO;
			Rtn = TRUE;
		}
	}
	else if(_stricmp(pTag, "printfrequency") == 0)
	{
		int Num;

		Num = atoi(pValue);
		ParaPrintFrequency = Num;

		Rtn = TRUE;
	}
	else if(_stricmp(pTag, "activeroutetimeout") == 0)
	{
		int Num;

		Num = atoi(pValue);
		ParaActiveRouteTimeout = Num;

		Rtn = TRUE;
	}
	else if(_stricmp(pTag, "allowedhelloloss") == 0)
	{
		int Num;

		Num = atoi(pValue);
		ParaAllowedHelloLoss = Num;

		Rtn = TRUE;
	}
	else if(_stricmp(pTag, "hellointerval") == 0)
	{
		int Num;

		Num = atoi(pValue);
		ParaHelloInterval = Num;

		Rtn = TRUE;
	}
	else if(_stricmp(pTag, "localaddttl") == 0)
	{
		int Num;

		Num = atoi(pValue);
		ParaLocalAddTTL = Num;

		Rtn = TRUE;
	}
	else if(_stricmp(pTag, "netdiameter") == 0)
	{
		int Num;

		Num = atoi(pValue);
		ParaNetDiameter = Num;

		Rtn = TRUE;
	}
	else if(_stricmp(pTag, "nodetraversaltime") == 0)
	{
		int Num;

		Num = atoi(pValue);
		ParaNodeTraversalTime = Num;

		Rtn = TRUE;
	}
	else if(_stricmp(pTag, "rerrratelimit") == 0)
	{
		int Num;

		Num = atoi(pValue);
		ParaRERRRatelimit = Num;

		Rtn = TRUE;
	}
	else if(_stricmp(pTag, "rreqretries") == 0)
	{
		int Num;

		Num = atoi(pValue);
		ParaRREQRetries = Num;

		Rtn = TRUE;
	}
	else if(_stricmp(pTag, "rreqratelimit") == 0)
	{
		int Num;

		Num = atoi(pValue);
		ParaRREQRateLimit = Num;

		Rtn = TRUE;
	}
	else if(_stricmp(pTag, "timeoutbuffer") == 0)
	{
		int Num;

		Num = atoi(pValue);
		ParaTimeoutBuffer = Num;

		Rtn = TRUE;
	}
	else if(_stricmp(pTag, "ttlstart") == 0)
	{
		int Num;

		Num = atoi(pValue);
		ParaTTLStart = Num;

		Rtn = TRUE;
	}
	else if(_stricmp(pTag, "ttlincrement") == 0)
	{
		int Num;

		Num = atoi(pValue);
		ParaTTLIncrement = Num;

		Rtn = TRUE;
	}
	else if(_stricmp(pTag, "ttlthreshold") == 0)
	{
		int Num;

		Num = atoi(pValue);
		ParaTTLThreshold = Num;

		Rtn = TRUE;
	}

	return (Rtn);
}

/*
* Break the string into its tag and value components
*/
BOOL GetTagAndValue(char *pBuffer, int BufferSize, char *pTag, char *pValue)
{
	int i;
	char *pTagStart, *pValueStart;

	// remove begining spaces
	i = 0;
	while((*(pBuffer+i) != '\0') && (*(pBuffer+i) == ' ') && (i < BufferSize))
	{
		i++;
	}

	// check if remark line or blank line
	if((i >= BufferSize) || (*(pBuffer+i) == '\0') || (*(pBuffer+i) == '#'))
	{
		strcpy(pTag, "#");
		return TRUE;
	}

	// start of the tag
	pTagStart = pBuffer + i;

	// find the end of the tag
	i++;
	while((*(pBuffer+i) != '\0') && (*(pBuffer+i) != '=') && (*(pBuffer+i) != ';')
		&& (*(pBuffer+i) != ' ') && (*(pBuffer+i) != '#') && (i < BufferSize) )
	{
		i++;
	}

	// check if the end of tag is valid
	if((i >= BufferSize) || (*(pBuffer+i) == '\0') || (*(pBuffer+i) == '#')
		|| (*(pBuffer+i) == ';'))
	{
		*pTag = '\0';
		*pValue = '\0';
		return FALSE;
	}

	if(*(pBuffer+i) == ' ')
	{
		*(pBuffer+i) = '\0';
		i++;
		while((*(pBuffer+i) != '\0') && (*(pBuffer+i) != '=') && (i < BufferSize))
		{
			i++;
		}

		if((i >= BufferSize) || (*(pBuffer+i) == '\0'))
		{
			*pTag = '\0';
			*pValue = '\0';
			return FALSE;
		}
		*(pBuffer+i) = '\0';
	}
	else
	{
		*(pBuffer+i) = '\0';
	}


	// find the start of the value
	i++;
	while((*(pBuffer+i) != '\0') && (*(pBuffer+i) == ' ') && (i < BufferSize))
	{
		i++;
	}

	if((i >= BufferSize) || (*(pBuffer+i) == '\0'))
	{
		*pTag = '\0';
		*pValue = '\0';
		return FALSE;
	}

	// start of value
	pValueStart = pBuffer + i;

	// find the end of the tag
	i++;
	while((*(pBuffer+i) != '\0') && (*(pBuffer+i) != ';')
		&& (*(pBuffer+i) != ' ') && (*(pBuffer+i) != '#') 
		&& (*(pBuffer+i) != LF_CHAR) && (i < BufferSize) )
	{
		i++;
	}

	if((i >= BufferSize))
	{
		*pTag = '\0';
		*pValue = '\0';
		return FALSE;
	}

	*(pBuffer+i) = '\0';

	strcpy(pTag, pTagStart);
	strcpy(pValue, pValueStart);

	return TRUE;
}

/*
* Set default values
*/
void SetDefaultValues()
{
	ParaExecutionMode = EXECUTION_MODE_CONSOLE;
	ParaIPVersion = IP_VERSION_4;
	ParaIPAddress.S_un.S_addr = inet_addr("134.102.158.91");
	strcpy(ParaIfaceName, "\\Device\\Packet_{3E2A6273-9FF1-408A-8F16-EC3F4BD9F7DB}");
	ParaGatewayIPAddress.S_un.S_addr = inet_addr("134.102.158.89");
	ParaLoggingStatus = YES;
	ParaLoggingMode = LOGGING_MODE_FILE;
	ParaLoggingLevel = LOGGING_SEVERAITY_MORE_INFO;
	strcpy(ParaLogFile, ".\\winaodv.log");
	ParaOnlyDestination = YES;
	ParaGratuitousRREP = NO;
	ParaRREPAckRequired = NO;
	ParaIPAddressMulticast.S_un.S_addr = inet_addr("255.255.255.255");
	ParaRERRSendingMode = RERR_SEND_MODE_UNICAST;
	ParaDeletePeriodMode = DELETE_PERIOD_MODE_HELLO;
	ParaRouteDiscoveryMode = ROUTE_DISCOVERY_MODE_NON_ERS;
	ParaPacketBuffering = YES;
	ParaPrintFrequency = 1000;
	ParaActiveRouteTimeout = 6000;
	ParaAllowedHelloLoss = 3;
	ParaHelloInterval = 1000;
	ParaLocalAddTTL = 2;
	ParaNetDiameter = 20;
	ParaNodeTraversalTime = 40;
	ParaRERRRatelimit = 10;
	ParaRREQRetries = 5;
	ParaRREQRateLimit = 10;
	ParaTimeoutBuffer = 2;
	ParaTTLStart = 5;
	ParaTTLIncrement = 2;
	ParaTTLThreshold = 15;
}

/*
* Re-computes the computed parameters 
*/
void UpdateComputedParameters()
{
	ParaNetTraversalTime = 2 * ParaNodeTraversalTime * ParaNetDiameter;
	ParaBlacklistTimeout = ParaRREQRetries * ParaNetTraversalTime;
	ParaMaxRepairTTL = (int) (0.3 * ParaNetDiameter);
	ParaMyRouteTimeout = 2 * ParaActiveRouteTimeout;
	ParaNextHopWait = ParaNodeTraversalTime + 10;
	ParaPathDiscoveryTime = 2 * ParaNetTraversalTime;
	if(ParaDeletePeriodMode == DELETE_PERIOD_MODE_HELLO) 
	{
		ParaDeletePeriod = ParaAllowedHelloLoss * ParaHelloInterval;
	} 
	else 
	{
		ParaDeletePeriod = ParaActiveRouteTimeout;
	}
}
