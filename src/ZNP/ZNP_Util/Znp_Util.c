/*
 * Znp_Util.c
 *
 *  Created on: Mar 3, 2016
 *      Author: ChauNM
 */

#include <stddef.h>
#include <stdio.h>
#include "Znp_Util.h"
#include "queue.h"

int ZnpUtilGetDeviceInfo()
{
	PQUEUECONTENT pCommandContent = ZnpMakeSerialCommand(UTIL_GET_DEVICE_INFO, 0, NULL);
	printf("Get Znp device info \n");
	ZnpWriteSerialCommand(pCommandContent);
	ZnpSetState(ZNP_STATE_WAIT_RSP, UTIL_GET_DEVICE_INFO_RES);
	free(pCommandContent->pData);
	free(pCommandContent);
	while(ZnpGetState() != ZNP_STATE_ACTIVE);
	return 0;
}

VOID ZnpUtilProcDeviceInfoRsp(PUTILDEVICEINFO pData, BYTE nLength)
{
	BYTE nIndex;
	PWORD pIeeeAddress = (PWORD)(&(pData->IeeeAddr));
	ZnpSetIeeeAddr(pData->IeeeAddr);
	ZnpSetShortAddr(pData->nShortAddr);
	printf("Status: %d\n", pData->nStatus);
	printf("IEEE Address: 0x%04X%04X%04X%04X \n", pIeeeAddress[3], pIeeeAddress[2], pIeeeAddress[1], pIeeeAddress[0]);
	printf("Short Address: 0x%04X \n", pData->nShortAddr);
	printf("Device type: %d \n", pData->nDeviceType);
	printf("Device State: %d \n", pData->nDeviceState);
	printf("Number Associated devices: %d \n", pData->nAssocDevice);
	printf("Associated devices list: ");
	for (nIndex = 0; nIndex < pData->nAssocDevice; nIndex++)
	{
		printf("0x%04X, ", *((PWORD)(pData->pData + (nIndex * sizeof(WORD)))));
	}
	printf("\n");
}

VOID ZnpUtilProcessIncomingCommand(PZNPPACKAGE pBuffer, BYTE nLength)
{
	printf("ZNP Util command 0x%04X\n", pBuffer->nCommand);
	switch(pBuffer->nCommand)
	{
	case UTIL_GET_DEVICE_INFO_RES:
		ZnpUtilProcDeviceInfoRsp((PUTILDEVICEINFO)pBuffer->pData, pBuffer->nLength);
		break;
	default:
		break;
	}
}
