/*
 * Znp_SimpleApi.c
 *
 *  Created on: Mar 2, 2016
 *      Author: ChauNM
 */

#include "znp.h"
#include "ZnpCommandState.h"
#include "Znp_SimpleApi.h"

int ZnpZbWriteConfig(BYTE nConfigID, BYTE nLength, PBYTE pData)
{
	int nResult;
	PZNPZBCONFIG pZnpConfig;
	PBYTE pBuffer = malloc(nLength + 2);
	BYTE nIndex;
	pZnpConfig = (PZNPZBCONFIG)pBuffer;
	pZnpConfig->nConfigID = nConfigID;
	pZnpConfig->nLength = nLength;
	for (nIndex = 0; nIndex < nLength; nIndex++)
		pZnpConfig->pData[nIndex] = pData[nIndex];
	PQUEUECONTENT pCommandContent = ZnpMakeSerialCommand(ZB_WRITE_CONFIGURATION_REQ, pZnpConfig->nLength + 2, (PBYTE)pZnpConfig);
	free(pBuffer);
	ZnpWriteSerialCommand(pCommandContent);
	ZnpSetState(ZNP_STATE_WAIT_RSP, ZB_WRITE_CONFIGURATION_RES);
	free(pCommandContent->pData);
	free(pCommandContent);
	while(ZnpGetState() != ZNP_STATE_ACTIVE);
	nResult = ZnpGetCmdState(ZB_WRITE_CONFIGURATION_RES);
	printf("Write configuration result %d\n", nResult);
	return nResult;
}

int ZnpZbPermitJoiningReq(WORD nAddress, BYTE nTimeout)
{
	int nResult;
	PZNPZBPERMITJOIN pPermitJoinData = malloc(sizeof(ZNPZBPERMITJOIN));
	pPermitJoinData->nAddress = nAddress;
	pPermitJoinData->nTimeout = nTimeout;
	PQUEUECONTENT pPermitJoinCommand = ZnpMakeSerialCommand(ZB_PERMIT_JOIN_REQ, sizeof(ZNPZBPERMITJOIN), (PBYTE)pPermitJoinData);
	free(pPermitJoinData);
	ZnpWriteSerialCommand(pPermitJoinCommand);
	ZnpSetState(ZNP_STATE_WAIT_RSP, ZB_PERMIT_JOIN_RES);
	free(pPermitJoinCommand->pData);
	free(pPermitJoinCommand);
	while(ZnpGetState() != ZNP_STATE_ACTIVE)
	{
		usleep(100000);
		nTimeout--;
		if (nTimeout == 0)
			return 1;
	}
	nResult = ZnpGetCmdState(ZB_PERMIT_JOIN_RES);
	printf("Znp Permit Join Requet result %d\n", nResult);
	return nResult;
}

VOID ZnpZbProcessIncomingCommand(PZNPPACKAGE pZnpPackage, BYTE nLength)
{
	printf("ZNP simple api command 0x%04X\n", pZnpPackage->nCommand);
	switch (pZnpPackage->nCommand)
	{
	case ZB_WRITE_CONFIGURATION_RES:
		ZnpAddCmdState(ZB_WRITE_CONFIGURATION_RES, *(_ZNPCONTENT(pZnpPackage)));
		break;
	case ZB_START_RES:
		ZnpAddCmdState(ZB_START_RES, 0);
		break;
	case ZB_START_CFM:
		// Process start request confirm package
		break;
	case ZB_PERMIT_JOIN_RES:
		ZnpAddCmdState(ZB_PERMIT_JOIN_RES, *(_ZNPCONTENT(pZnpPackage)));
		break;
	default:
		break;
	}
}
