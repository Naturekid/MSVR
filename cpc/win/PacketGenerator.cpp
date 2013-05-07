/*
* Functions in this file provide services for generating IP traffic. These
* are temporary functions.
*
* @author : Asanga Udugama
* @date : 18-jun-2004
* @email : adu@comnets.uni-bremen.de
*
*/

#include "stdafx.h"
#include "UoBWinAODVCommon.h"
//#include "FilterSetter.h"

FILE *pfp = NULL;

int GetAnIPPacket(char *PacketBuffer, int BufSize)
{
	char StrBuffer[4096], *ptr;
	int i;

	if(pfp == NULL) 
	{
		pfp = fopen(".\\PacketTrace.txt", "r");
		if(pfp == NULL) 
		{
			printf("Could not open packet trace \n");
			return 0;
		}
	}

	if(fgets(StrBuffer, 4096, pfp) == NULL)
	{
		printf("No lines to read in packet trace \n");
		return 0;
	}

	memset(PacketBuffer, 0, BufSize);

	i = 0;
	ptr = StrBuffer;
	while(sscanf(ptr, "%02x ", &PacketBuffer[i++]) != 0)
	{
		ptr += 3;
	}

	return (i - 1);

}
