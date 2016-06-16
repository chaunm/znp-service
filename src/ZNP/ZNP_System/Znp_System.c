/*
 * Znp_System.c
 *
 *  Created on: Mar 3, 2016
 *      Author: ChauNM
 */

#include "znp.h"
#include "Znp_System.h"

VOID ZnpSysProccessIncomingCommand(PZNPPACKAGE pBuffer, BYTE nLength)
{
	printf("Znp system command 0x%04X\n", pBuffer->nCommand);
	switch (pBuffer->nCommand)
	{
	case SYS_RESET_IND:
		printf("ZNP reset \n");
		PZNPSYSRESETIND pResetInd= (PZNPSYSRESETIND)_ZNPCONTENT(pBuffer);
		printf("Resetting reason: %d \n", pResetInd->nReason);
		printf("Transport revision: %d \n", pResetInd->nTransRev);
		printf("ProductID: %d \n", pResetInd->nProductID);
		printf("Major Rel: %d \n", pResetInd->nMajorRel);
		printf("Minor Rel: %d \n", pResetInd->nMinorRel);
		printf("Hw Rev: %d \n", pResetInd->nHwRev);
		break;
	default:
		break;
	}
}
