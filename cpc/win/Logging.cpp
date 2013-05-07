
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
* Functions in this file provide services for logging activities
* when the UoBWinAODV is active.
*
* @author : Asanga Udugama
* @date : 15-jun-2004
* @email : adu@comnets.uni-bremen.de
*
* @modification history
*	@modification : Made log writing process more efficient
*	@author : Asanga Udugama
*	@date : 02-oct-2004
*	@email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
#include "Logging.h"

FILE	*LogFP;
int		ActualLoggingMode = LOGGING_MODE_INITIAL;

// Logging lock
CRITICAL_SECTION LoggingLock;
LPCRITICAL_SECTION pLoggingLock;

void LoggingInit()
{
	char PrintBuf[2048];

	if(ParaLoggingStatus == YES)
	{
		if(ParaLoggingMode == LOGGING_MODE_FILE)
		{
			LogFP = fopen(ParaLogFile,"a");
			if(LogFP == NULL)
			{
				ActualLoggingMode = LOGGING_MODE_CONSOLE;
				sprintf(PrintBuf, 
				"%ld :: Logging : Could not open log file %s. Logging to console \n", 
					GetCurMiliSecTime(), ParaLogFile);
				LogMessageInitial(PrintBuf);
			}
			else
			{
				fprintf(LogFP, "\n");
				ActualLoggingMode = LOGGING_MODE_FILE;
			}
		}
		else
		{
			printf("\n");
			ActualLoggingMode = LOGGING_MODE_CONSOLE;
		}
	}

	pLoggingLock = &LoggingLock;
	InitializeCriticalSection(pLoggingLock);
}

/*
* Log message considering the logging statuses
*/
void LogMessage(int LoggingLevel, char *pPrintStr)
{
	char TimeStr[512];

	//EnterCriticalSection(pLoggingLock);

	if(ParaLoggingStatus == YES)
	{
		if(ParaLoggingLevel >= LoggingLevel)
		{
			if(ActualLoggingMode == LOGGING_MODE_FILE)
			{
				GetCurTimeForLogging(TimeStr);
				fprintf(LogFP, TimeStr);
				fprintf(LogFP, " - ");
				fprintf(LogFP, pPrintStr);
				fprintf(LogFP, "\n");
				//fflush(LogFP);
			}
			// ActualLoggingMode == LOGGING_MODE_CONSOLE
			else 
			{
				GetCurTimeForLogging(TimeStr);
				printf(TimeStr);
				printf(" - ");
				printf(pPrintStr);
				printf("\n");
			}
		}
	}

	//LeaveCriticalSection(pLoggingLock);
}

void LoggingTerm()
{
	EnterCriticalSection(pLoggingLock);
	if(ParaLoggingStatus == YES && ActualLoggingMode == LOGGING_MODE_FILE)
	{
		fclose(LogFP);
		ParaLoggingStatus = NO;
	}
	LeaveCriticalSection(pLoggingLock);
}

/*
* Function to log messages that occur just befor the standard
* message logging is activated.
*/
void LogMessageInitial(char *pPrintStr)
{
	char TimeStr[64];

	sprintf(TimeStr, "%dl :: ", GetCurMiliSecTime());
	printf(TimeStr);
	printf(pPrintStr);
	printf("\n");
}
