/*
 * ZnpAf.c
 *
 *  Created on: Mar 2, 2016
 *      Author: ChauNM
 */

#include <pthread.h>
#include "ZnpAf.h"
#include "znp.h"
#include "ZnpCommandState.h"
#include "queue.h"
#include "universal.h"
#include "zcl.h"
#include "DevicesManager.h"
#include "ZNP_ZDO/Znp_Zdo.h"
#include "ZnpActor.h"


static BYTE nAfTransId = 0;
static BYTE nAfDefaultOption = AF_ACK_REQUEST | AF_DISCV_ROUTE;
static BYTE nAfDefaultRadius = AF_DEFAULT_RADIUS;

int AfRegisterEndpoint(BYTE nEndPoint, WORD nProfileID, WORD nDeviceID, BYTE nNumberInCluster,
		BYTE nNumberOutCluster, PWORD pInCluster, PWORD pOutCluster, BYTE nDeviceVer, BYTE nLatencyReq)
{
	int nResult;
	// Calculate data size for ZNP package;
	BYTE nZnpDataSize = (nNumberInCluster + nNumberOutCluster) * sizeof(WORD);
	nZnpDataSize += 9;
	// Make data buffer
	PBYTE pData = (PBYTE)malloc(nZnpDataSize);
	PZNPCLUSTERLIST pInClusterList;
	PZNPCLUSTERLIST pOutClusterList;
	PAFENDPOINTREGISTER pAfEndPointPackage = (PAFENDPOINTREGISTER)pData;
	pAfEndPointPackage->nEndPoint = nEndPoint;
	pAfEndPointPackage->nProfileID = nProfileID;
	pAfEndPointPackage->nDeviceID = nDeviceID;
	pInClusterList = (PZNPCLUSTERLIST)pAfEndPointPackage->pData;
	pInClusterList->nNumberCluster = nNumberInCluster;
	if ((nNumberInCluster != 0) && (pInCluster != NULL))
		CopyMemory((PBYTE)pInClusterList->arClusterList, (PBYTE)pInCluster, nNumberInCluster * 2);
	pOutClusterList = (PZNPCLUSTERLIST)(pAfEndPointPackage->pData + nNumberInCluster * 2 + 1);
	pOutClusterList->nNumberCluster = nNumberOutCluster;
	if ((nNumberOutCluster != 0) && (pOutCluster != NULL))
		CopyMemory((PBYTE)pOutClusterList->arClusterList, (PBYTE)pOutCluster, nNumberOutCluster * 2);
	PQUEUECONTENT pZnpCommand = ZnpMakeSerialCommand(AF_REGISTER_REQ, nZnpDataSize, pData);
	free(pData);
	ZnpWriteSerialCommand(pZnpCommand);
	ZnpSetState(ZNP_STATE_WAIT_RSP, AF_REGISTER_RSP);
	free(pZnpCommand->pData);
	free(pZnpCommand);
	while(ZnpGetState() != ZNP_STATE_ACTIVE);
	nResult = ZnpGetCmdState(AF_REGISTER_RSP);
	printf("AF register result %d\n", nResult);
	return nResult;
}

VOID AfDataReq(WORD nDstAddr, BYTE nDstEp, BYTE nSrcEp, WORD nClusterID, PBYTE pData, BYTE nLen, BOOL bResponseRequire)
{
	PAFDATAREQ pAfDataReq = (PAFDATAREQ)malloc(sizeof(AFDATAREQ) + nLen);
	PQUEUECONTENT pZnpCommand;
	pAfDataReq->nDstAddr = nDstAddr;
	pAfDataReq->nDstEndPoint = nDstEp;
	pAfDataReq->nSrcEndPoint = nSrcEp;
	pAfDataReq->nClusterID = nClusterID;
	pAfDataReq->nTransID = nAfTransId;
	nAfTransId++;
	pAfDataReq->nOptions = nAfDefaultOption;
	pAfDataReq->nRadius = nAfDefaultRadius;
	pAfDataReq->nLen = nLen;
	CopyMemory(pAfDataReq->pData, pData, nLen);
	pZnpCommand = ZnpMakeSerialCommand(AF_DATA_REQ, sizeof(AFDATAREQ) + nLen, (PBYTE)pAfDataReq);
	free((PBYTE)pAfDataReq);
	ZnpWriteSerialCommand(pZnpCommand);
	ZnpSetState(ZNP_STATE_WAIT_RSP, AF_DATA_REQ_RSP);
	free(pZnpCommand->pData);
	free(pZnpCommand);
	if ((bResponseRequire == TRUE) && (DeviceFind(nDstAddr) != NULL))
	{
		DeviceSetTimeoutTime(nDstAddr, DEFAULT_MESSAGE_TIMEOUT);
	}
	//while(ZnpGetState() != ZNP_STATE_ACTIVE);
}

VOID AfProcessIncomingMsg(PZNPPACKAGE pZnpPackage, BYTE nLength)
{
	PAFINCOMINGMSG pIncomingData = (PAFINCOMINGMSG)(_ZNPCONTENT((PBYTE)pZnpPackage));
	pthread_t AddClusterThr;//, AddEpThr;
	BYTE nZclTransID;
	BYTE nIndex;
	printf("AF incoming data \n");
	printf("Group ID: 0x%04X\n", pIncomingData->nGroupId);
	printf("Cluster ID: 0x%04X\n", pIncomingData->nClusterId);
	printf("Source Address: 0x%04X\n", pIncomingData->nSrcAddr);
	printf("Source Endpoint: %d\n", pIncomingData->nSrcEndpoint);
	printf("Destination Endpoint: %d\n", pIncomingData->nDstEndpoint);
	printf("Was broadcast: %d\n", pIncomingData->nWasBroadcast);
	printf("Link quality: 0x%02X\n", pIncomingData->nLinkQua);
	printf("Security use: %d\n", pIncomingData->nSecuUse);
	PWORD pTime = (PWORD)&pIncomingData->nTimestamp;
	printf("Time stamp: 0x%04X%04X\n", pTime[1], pTime[0]);
	printf("Transmission ID: %d\n", pIncomingData->nTransId);
	printf("Data Length: %d\n", pIncomingData->nLen);
	// if device does not exists in list than return;
	DeviceSetTimeoutTime(pIncomingData->nSrcAddr, DEFAULT_DEVICE_TIMEOUT);
	ZnpActorPublishDeviceLqi(pIncomingData->nSrcAddr, pIncomingData->nLinkQua);
	if (DeviceFind(pIncomingData->nSrcAddr) != NULL)
	{
		/*
		if (DeviceFindEpInfo(pIncomingData->nSrcAddr, pIncomingData->nSrcEndpoint) == NULL)
		{
			printf("1 Add endpoint\n");
			PENDPOINTADDR pEpAddr = (PENDPOINTADDR)malloc(sizeof(ENDPOINTADDR));
			pEpAddr->nNwkAddr = pIncomingData->nSrcAddr;
			pEpAddr->nEp = pIncomingData->nSrcEndpoint;
			pthread_create(&AddEpThr, NULL, (PVOID)DeviceAddEndpoint, (PVOID)pEpAddr);
		}
		*/
		if (DeviceFindClusterInfo(pIncomingData->nSrcAddr, pIncomingData->nSrcEndpoint, pIncomingData->nClusterId) == NULL)
		{
			printf("Cluster's not in list, add cluster 0x%0X to device 0x%04X\n", pIncomingData->nClusterId, pIncomingData->nSrcAddr);
			PCLUSTERADDR pClusterAddr = (PCLUSTERADDR)malloc(sizeof(CLUSTERADDR));
			pClusterAddr->nEp = pIncomingData->nSrcEndpoint;
			pClusterAddr->nNwkAddr = pIncomingData->nSrcAddr;
			pClusterAddr->nClusterID = pIncomingData->nClusterId;
			pthread_create(&AddClusterThr, NULL, (PVOID)DeviceAddCluster, (PVOID)pClusterAddr);
			// add pthread_detach to tell that resources of this thread should be freed after return
			pthread_detach(AddClusterThr);
		}
		// print ZCL data for debugging
		printf("ZCL Data:\n");
		for (nIndex = 0; nIndex < pIncomingData->nLen; nIndex++)
			printf("0x%02X ", pIncomingData->pData[nIndex]);
		printf("\n");

		nZclTransID = ZclParsePackage(pIncomingData->nSrcAddr, pIncomingData->nSrcEndpoint, pIncomingData->nClusterId, pIncomingData->pData, pIncomingData->nLen);
		DeviceSetInformed(pIncomingData->nSrcAddr, pZnpPackage->nCommand, nZclTransID);
	}
	else
	{
		ZDOANNCEINFO stZdoAnnce;
		stZdoAnnce.nNwkAddr = pIncomingData->nSrcAddr;
		stZdoAnnce.IeeeAddr = 0;
		stZdoAnnce.nCapabilities = 0;
		stZdoAnnce.nSrcAddr = pIncomingData->nSrcAddr;
		DeviceAdd(&stZdoAnnce);
	}
}

VOID AfProcessIncomingCommand(PZNPPACKAGE pZnpPackage, BYTE nLength)
{
	printf("ZNP Af command 0x%04X\n", pZnpPackage->nCommand);
	switch (pZnpPackage->nCommand)
	{
	case AF_REGISTER_RSP:
		ZnpAddCmdState(AF_REGISTER_RSP, *((PBYTE)(_ZNPCONTENT(pZnpPackage))));
		break;

	case AF_DATA_REQ_RSP:
		ZnpAddCmdState(AF_DATA_REQ_RSP, *((PBYTE)(_ZNPCONTENT(pZnpPackage))));
		if (*((PBYTE)(_ZNPCONTENT(pZnpPackage))) == 0)
			printf("Af data request success\n");
		else
			printf("Error: %d\n", *((PBYTE)(_ZNPCONTENT(pZnpPackage))));
		break;
	case AF_INCOMING_MSG:
		AfProcessIncomingMsg(pZnpPackage, nLength);
		break;
	default:
		break;
	}
}
