#include "universal.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

VOID CopyMemory (PBYTE pDest, PBYTE pSource, BYTE nLength)
{
	BYTE nIndex;
	if ((pDest == NULL) || (pSource == NULL))
			return;
	for (nIndex = 0; nIndex < nLength; nIndex++)
	{
		pDest[nIndex] = pSource[nIndex];
	}
}


char* StrDup(const char* string)
{
	if (string == NULL) return NULL;
	char* pDest = malloc(strlen(string) + 1);
	memset(pDest, 0, strlen(string) + 1);
	memcpy(pDest, string, strlen(string));
	return pDest;
}

char* IeeeToString(IEEEADDRESS macId)
{
	char* macIdString = malloc(17);
	memset(macIdString, 0, 17);
	PWORD macIdArray = (PWORD)(&macId);
	sprintf(macIdString, "%04X%04X%04X%04X", macIdArray[3], macIdArray[2], macIdArray[1], macIdArray[0]);
	return macIdString;
}
