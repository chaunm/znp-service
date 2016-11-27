/*
 * Znp_Zdo.c
 *
 *  Created on: Mar 3, 2016
 *      Author: ChauNM
 */

#include "Znp_Zdo.h"
#include "znp.h"
#include "ZnpCommandState.h"
#include "DevicesManager.h"
#include "log.h"
#include "string.h"
#include "ZnpActor.h"
#include "../fluent-logger/fluent-logger.h"
/* Function: ZnpZdoProcessZdoStateChangeInd(PZNPPACKAGE pBuffer, BYTE nLength)
 * Description:
 * 	- Process the ZDO_STATE_CHANGE_IND command from ZNP
 * Input:
 * 	- pBuffer: Pointer to the ZNP command.
 * 	- nLength: Size of the ZNP command
 */
VOID ZnpZdoProcessZdoStateChangeInd(PZNPPACKAGE pBuffer, BYTE nLength)
{
	ZnpSetZdoState(*(_ZNPCONTENT(pBuffer)));
	printf("ZDO State changed: %d \n", ZnpGetZdoState());
	if (ZnpGetZdoState() == ZNP_ZIGBEE_COOR_READY)
		printf("ZNP start success \n");
}

/* Function: ZnpZdoProcessPermitJoinRsp(PZNPPACKAGE pBuffer, BYTE nLength)
 * Description:
 * 	- Process the ZDO_PERMIT_JOIN_RSP command from ZNP
 * Input:
 * 	- pBuffer: Pointer to the ZNP command.
 * 	- nLength: Size of the ZNP command
 */
VOID ZnpZdoProcessPermitJoinRsp(PZNPPACKAGE pBuffer, BYTE nLength)
{
	PZNPZDOPERJOINDATA pPermitJoinData = (PZNPZDOPERJOINDATA)(_ZNPCONTENT(pBuffer));
	if (pPermitJoinData->nStatus == 0x00)
		printf("Address %d permit join success \n", pPermitJoinData->nSourceAddr);
	else
		printf("Address %d permit join fail \n", pPermitJoinData->nSourceAddr);
}

/* Function: ZnpZdoProcessEdAnnceRsp(PZNPPACKAGE pBuffer, BYTE nLength)
 * Description:
 * 	- Process the ZDO_ENDDEVICE_ANNOUNCE_RSP command from ZNP
 * Input:
 * 	- pBuffer: Pointer to the ZNP command.
 * 	- nLength: Size of the ZNP command
 */
VOID ZnpZdoProcessEdAnnceRsp(PZNPPACKAGE pBuffer, BYTE nLength)
{
	PZDOANNCEINFO pAnnceInfo;
	PWORD pIeeeAddress;
	printf("Device Announce:\n");
	pAnnceInfo = (PZDOANNCEINFO)(_ZNPCONTENT(pBuffer));
	printf("Source Address: 0x%04X \n", pAnnceInfo->nSrcAddr);
	printf("Nwk Address: 0x%04X \n", pAnnceInfo->nNwkAddr);
	pIeeeAddress = (PWORD)(&(pAnnceInfo->IeeeAddr));
	printf("IEEE Address: 0x%04x%04x%04x%04x \n", pIeeeAddress[3], pIeeeAddress[2], pIeeeAddress[1], pIeeeAddress[0]);
	printf("Capabilities: 0x%02x \n", pAnnceInfo->nCapabilities);
	DeviceSetTimeoutTime(pAnnceInfo->nNwkAddr, DEFAULT_DEVICE_TIMEOUT);
	char* pLogString = (char*)malloc(500);
	sprintf(pLogString, "Device Announce 0x%04X", pAnnceInfo->nNwkAddr);
	LogWrite(pLogString);
	free(pLogString);
	// Start device get info process
	DeviceAdd(pAnnceInfo);
}

VOID ZnpZdoProcessEdIeeeBroadcast(PZNPPACKAGE pBuffer, BYTE nLength)
{
	PZDOIEEEBRD pIeeeInfo = (PZDOIEEEBRD)(_ZNPCONTENT(pBuffer));
	char* LogString = malloc(250);
	sprintf(LogString, "Ieee Address broadcast, network address 0x%02X", pIeeeInfo->nNwkAddr);
	LogWrite(LogString);
	FLUENT_LOGGER_INFO(LogString);
	free(LogString);
	DeviceSetTimeoutTime(pIeeeInfo->nNwkAddr, DEFAULT_DEVICE_TIMEOUT);
}

/* Function: ZnpZdoProcessEdLeaveRsp(PZNPPACKAGE pBuffer, BYTE nLength)
 * Description:
 * 	- Process the ZDO_ENDDEVICE_LEAVE_RSP command from ZNP
 * Input:
 * 	- pBuffer: Pointer to the ZNP command.
 * 	- nLength: Size of the ZNP command
 */
VOID ZnpZdoProcessEdLeaveRsp(PZNPPACKAGE pBuffer, BYTE nLength)
{
	PZDOEDLEAVE pZdoLeaveInfo = (PZDOEDLEAVE)(_ZNPCONTENT(pBuffer));
	printf("Device leaved: 0x%04X\n", pZdoLeaveInfo->nNwkAddress);
	DeviceRemove(pZdoLeaveInfo->nNwkAddress);
	ZnpActorPublishDeviceRemovedEvent(pZdoLeaveInfo->IeeeAddress);
}

/* Function: ZnpZdoActiveEpRequest(WORD nNwkAddr)
 * Description:
 * 	- Request for active endpoint list of a device in network
 * Input:
 * 	- nNwkAddr: Network address of the device to be requested
 */
BYTE ZnpZdoActiveEpRequest(WORD nNwkAddr)
{
	// ham co cho while, khong duoc goi khi xu ly goi tin nhan ma phai goi tu luong khac
	BYTE nTimeOutTime = 30;
	PZDOATVEPREQ pActiveEpRqt = (PZDOATVEPREQ)malloc(sizeof(ZDOATVEPREQ));
	pActiveEpRqt->nDstAddr = nNwkAddr;
	pActiveEpRqt->nNwkAddrOfInterest = nNwkAddr;
	PQUEUECONTENT pCommand = ZnpMakeSerialCommand(ZDO_ACTIVE_EP_REQ, sizeof(ZDOATVEPREQ), (PBYTE)pActiveEpRqt);
	free(pActiveEpRqt);
	ZnpWriteSerialCommand(pCommand);
	ZnpSetState(ZNP_STATE_WAIT_RSP, ZDO_ACTIVE_EP_REQ_RSP);
	free(pCommand->pData);
	free(pCommand);
	while((ZnpGetState() != ZNP_STATE_ACTIVE) && (nTimeOutTime > 0))
	{
		sleep(1);
		nTimeOutTime--;
		if (nTimeOutTime == 0)
		{
			return 1;
			printf("Request EP timeout");
		}
	}
	return 0;
}

/* Function: ZnpZdoProcessActEpRsp(PZNPPACKAGE pBuffer, BYTE nLength)
 * Description:
 * 	- Process the ZDO_ACTIVE_EP_REQ_RSP command from ZNP
 * Input:
 * 	- pBuffer: Pointer to the ZNP command.
 * 	- nLength: Size of the ZNP command
 */
VOID ZnpZdoProcessActEpRsp(PZNPPACKAGE pBuffer, BYTE nLength)
{
	PZDOATVEPRES pActEpRsp = (PZDOATVEPRES)(_ZNPCONTENT(pBuffer));
	BYTE nIndex;
	printf("Device active EP response\n");
	printf("Source Address: 0x%04X\n", pActEpRsp->nSrcAddr);
	printf("Nwk Address: 0x%04X\n", pActEpRsp->nNwkAddr);
	printf("Status: %d \n", pActEpRsp->nStatus);
	printf("Number active EP: %d\n", pActEpRsp->nNumAtvEp);
	printf("EP List:");
	for (nIndex = 0; nIndex < pActEpRsp->nNumAtvEp; nIndex++)
		printf("0x%02X, ", pActEpRsp->pEpList[nIndex]);
	printf("\n");
	//Update Endpoint info
	DeviceUpdateDeviceInfo(pActEpRsp->nNwkAddr, pActEpRsp->nNumAtvEp, pActEpRsp->pEpList);
	DeviceSetInformed(pActEpRsp->nNwkAddr, pBuffer->nCommand, 0xFF);
	DeviceSetTimeoutTime(pActEpRsp->nNwkAddr, DEFAULT_DEVICE_TIMEOUT);
}

/* Function: ZnpZdoSimpleDescReq(PZNPPACKAGE pBuffer, BYTE nLength)
 * Description:
 * 	- Send ZDO_SIMPLE_EP_REQ to a device in network
 * Input:
 * 	- nNwkAddr: Short address of the device to be requested.
 * 	- nEndPoint: Enpoint to be requested.
 */
BYTE ZnpZdoSimpleDescReq(WORD nNwkAddr, BYTE nEndPoint)
{
	PZDOSIMPLEDESCREQ pSimpleDescReq = (PZDOSIMPLEDESCREQ)malloc(sizeof(ZDOSIMPLEDESCREQ));
	BYTE nTimeOutTime = 60;
	pSimpleDescReq->nDstAddr = nNwkAddr;
	pSimpleDescReq->nNwkAddrOfInterest = nNwkAddr;
	pSimpleDescReq->nEndPoint = nEndPoint;
	PQUEUECONTENT pCommand = ZnpMakeSerialCommand(ZDO_SIMPLE_DESC_REQ, sizeof(ZDOSIMPLEDESCREQ), (PBYTE)pSimpleDescReq);
	free(pSimpleDescReq);
	ZnpWriteSerialCommand(pCommand);
	ZnpSetState(ZNP_STATE_WAIT_RSP, ZDO_SIMPLE_DESC_REQ_RSP);
	free(pCommand->pData);
	free(pCommand);
	while((ZnpGetState() != ZNP_STATE_ACTIVE) && (nTimeOutTime > 0))
	{
		sleep(1);
		nTimeOutTime--;
		if (nTimeOutTime == 0)
		{
			return 1;
			printf("Request simple description timeout");
		}
	}
	//return nNwkAddr;
	return 0;
}

/* Function: ZnpZdoProcessSimpleDescRsp(PZNPPACKAGE pBuffer, BYTE nLength)
 * Description:
 * 	- Process the ZDO_SIMPLE_DESCRIPTION_REQ_RSP command from ZNP
 * Input:
 * 	- pBuffer: Pointer to the ZNP command.
 * 	- nLength: Size of the ZNP command
 */
VOID ZnpZdoProcessSimpleDescRsp(PZNPPACKAGE pBuffer, BYTE nLength)
{
	BYTE nNumberInCluster;
	BYTE nNumberOutCluster;
	PWORD pClusterList;
	BYTE nIndex;
	PZDOSIMPLEDESCRSP pSimpleDesc = (PZDOSIMPLEDESCRSP)(_ZNPCONTENT(pBuffer));
	printf("Source Address: 0x%04X\n", pSimpleDesc->nSrcAddr);
	printf("Nwk Address: 0x%04X\n", pSimpleDesc->nNwkAddr);
	printf("Status: %d\n", pSimpleDesc->nStatus);
	printf("Endpoint: %d\n", pSimpleDesc->nEndpoint);
	printf("Profile ID: 0x%04X\n", pSimpleDesc->nProfileId);
	printf("Device ID: 0x%04X\n", pSimpleDesc->nDeviceId);
	printf("Device version: %d\n", pSimpleDesc->nDevVer);
	nNumberInCluster = pSimpleDesc->pData[0];
	if (nNumberInCluster > 0)
	{
		pClusterList = (PWORD)(pSimpleDesc->pData + 1);
		printf("Number In cluster: %d\n", nNumberInCluster);
		printf("In Cluster List: ");
		for (nIndex = 0; nIndex < nNumberInCluster; nIndex++)
			printf("0x%04X, ", pClusterList[nIndex]);
		printf("\n");
	}
	nNumberOutCluster = pSimpleDesc->pData[1 + nNumberInCluster * sizeof(WORD)];
	if (nNumberOutCluster > 0)
	{
		printf("Number Out cluster: %d\n", nNumberOutCluster);
		pClusterList = (PWORD)(pSimpleDesc->pData + 2 + nNumberInCluster * sizeof(WORD));
		printf("Out Cluster List: ");
		for (nIndex = 0; nIndex < nNumberOutCluster; nIndex++)
			printf("0x%04X, ", pClusterList[nIndex]);
		printf("\n");
	}
	// update enpoint description to device manager
	DeviceUpdateEndpointDesc(pSimpleDesc);
	DeviceSetInformed(pSimpleDesc->nNwkAddr, pBuffer->nCommand, 0xFF);
}

VOID ZnpZdoBindReq(WORD nDstAddr, BYTE nSrcEp, WORD nClusterId)
{
	PDEVICEINFO pDevInfo = DeviceFind(nDstAddr);
	PZDOBINDREQ pZdoBindReq = (PZDOBINDREQ)malloc(sizeof(ZDOBINDREQ));
	if (pDevInfo == NULL)
		return;
	printf("Binding request to Address 0x%04X - Endpoint 0x%02X\n", nDstAddr, nSrcEp);
	pZdoBindReq->nDstAddr = nDstAddr;
	pZdoBindReq->SrcAddr =pDevInfo->IeeeAddr;
	pZdoBindReq->nSrcEp = nSrcEp;
	pZdoBindReq->nClusterID = nClusterId;
	pZdoBindReq->nAddrMode = 0x03;
	pZdoBindReq->DstAddr =  ZnpGetIeeeAddr();
	pZdoBindReq->nDstEp = ZNP_DEFAULT_ENDPOINT;

	PQUEUECONTENT pCommand = ZnpMakeSerialCommand(ZDO_BIND_REQ, sizeof(ZDOBINDREQ), (PBYTE)pZdoBindReq);
	free(pZdoBindReq);
	ZnpWriteSerialCommand(pCommand);
	ZnpSetState(ZNP_STATE_WAIT_RSP, ZDO_BIND_REQ_RSP);
	free(pCommand->pData);
	free(pCommand);
}

VOID ZnpZdoProcessBindRsp(PZNPPACKAGE pBuffer, BYTE nLength)
{
		PZDOBINDRSP pBindRsp = (PZDOBINDRSP)(_ZNPCONTENT(pBuffer));
		printf("Device 0x%04X bind status: 0x%02X\n", pBindRsp->nSrcAddr, pBindRsp->nStatus);
		//Update Endpoint info
		DeviceSetInformed(pBindRsp->nSrcAddr, pBuffer->nCommand, 0xFF);
}

BYTE ZnpZdoIeeeAddrReq(WORD nNetworkAddr, BYTE nRequestType, BYTE nStartIndex)
{
	BYTE nTimeOutTime = 60;
	PDEVICEINFO pDevInfo = DeviceFind(nNetworkAddr);
	PZDOIEEEADDRREQ pZdoIeeeReq = (PZDOIEEEADDRREQ)malloc(sizeof(ZDOIEEEADDRREQ));
	if (pDevInfo == NULL)
		return 1;
	printf("\e[1;31mGet IEEE address of device 0x%04X\n\e[1;33m", nNetworkAddr);
	pZdoIeeeReq->nDstAddr = nNetworkAddr;
	pZdoIeeeReq->nRequestType = nRequestType;
	pZdoIeeeReq->nStartIndex = nStartIndex;

	PQUEUECONTENT pCommand = ZnpMakeSerialCommand(ZDO_IEEE_ADDR_REQ, sizeof(ZDOIEEEADDRREQ), (PBYTE)pZdoIeeeReq);
	free(pZdoIeeeReq);
	ZnpWriteSerialCommand(pCommand);
	ZnpSetState(ZNP_STATE_WAIT_RSP, ZDO_IEEE_ADDR_REQ_RSP);
	free(pCommand->pData);
	free(pCommand);
	while((ZnpGetState() != ZNP_STATE_ACTIVE) && (nTimeOutTime > 0))
	{
		sleep(1);
		nTimeOutTime--;
		if (nTimeOutTime == 0)
		{
			return 1;
			printf("Request Ieee timeout\n");
		}
	}
	return 0;
}

VOID ZnpZdoProcessIeeeRsp(PZNPPACKAGE pBuffer, BYTE nLength)
{
	PZDOIEEEADDRRSP pIeeeRsp = (PZDOIEEEADDRRSP)(_ZNPCONTENT(pBuffer));
	printf("Check Zdo ieee status %d\n", pIeeeRsp->nStatus);
	if (pIeeeRsp->nStatus != 0x00) return;
	printf("Ieee Address get from 0x%04X\n", pIeeeRsp->nNetworkAddr);
	DeviceUpdateIeeeAddr(pIeeeRsp->nNetworkAddr, pIeeeRsp->IeeeAddr);
	DeviceSetInformed(pIeeeRsp->nNetworkAddr, pBuffer->nCommand, 0xFF);
}

VOID ZnpZdoProcessMgmtRtgRsp(PZNPPACKAGE pBuffer, BYTE nLength)
{
	BYTE count;
	PZDOMGMTRTGRSP pRtgMgmtRsp = (PZDOMGMTRTGRSP)(_ZNPCONTENT(pBuffer));
	printf("Routing entry response\n");
	printf("Total entries: %d\n", pRtgMgmtRsp->nTotalEntries);
	printf("Number of entry: %d\n", pRtgMgmtRsp->nEntriesCount);
	for (count = 0; count < pRtgMgmtRsp->nEntriesCount; count++)
	{
		printf("Entry %d, address 0x%04X, status %d:", pRtgMgmtRsp->nStartIndex + count,
				pRtgMgmtRsp->entriesList[count].nAddress, pRtgMgmtRsp->entriesList[count].nStatus);
		if (pRtgMgmtRsp->entriesList[count].nStatus > 1)
			DeviceSetTimeoutTime(pRtgMgmtRsp->entriesList[count].nAddress, 1);
		else
			DeviceSetTimeoutTime(pRtgMgmtRsp->entriesList[count].nAddress, 1000);
	}

}
/* Function: ZnpZdoProcessIncomingCommand(PZNPPACKAGE pBuffer, BYTE nLength)
 * Description:
 * 	- Process the Zdo command from ZNP
 * Input:
 * 	- pBuffer: Pointer to the ZNP command.
 * 	- nLength: Size of the ZNP command
 */
VOID ZnpZdoProcessIncomingCommand(PZNPPACKAGE pBuffer, BYTE nLength)
{
	printf("ZNP ZDO command 0x%04X\n", pBuffer->nCommand);
	switch(pBuffer->nCommand)
	{
	case ZDO_STATE_CHANGE_IND:
		ZnpZdoProcessZdoStateChangeInd(pBuffer, nLength);
		break;
	case ZDO_MGMT_PERMIT_JOIN_RSP:
		ZnpZdoProcessPermitJoinRsp(pBuffer, nLength);
		break;
	case ZDO_END_DEVICE_IEEE_IND:
		ZnpZdoProcessEdIeeeBroadcast(pBuffer, nLength);
		break;
	case ZDO_END_DEVICE_ANNCE_IND:
		ZnpZdoProcessEdAnnceRsp(pBuffer, nLength);
		break;
	case ZDO_END_DEVICE_LEAVE:
		ZnpZdoProcessEdLeaveRsp(pBuffer, nLength);
		break;
	case ZDO_ACTIVE_EP_RSP:
		ZnpZdoProcessActEpRsp(pBuffer, nLength);
		break;
	case ZDO_SIMPLE_DESC_RSP:
		ZnpZdoProcessSimpleDescRsp(pBuffer, nLength);
		break;
	case ZDO_BIND_RSP:
		ZnpZdoProcessBindRsp(pBuffer, nLength);
		break;
	case ZDO_IEEE_ADDR_RSP:
		ZnpZdoProcessIeeeRsp(pBuffer, nLength);
		break;
	case ZDO_PERMIT_JOIN_SESSION_END:
		printf("Permit join session ended\n");
		FLUENT_LOGGER_INFO("Permit joint session end");
		break;
	case ZDO_MGMT_RTG_RSP:
		ZnpZdoProcessMgmtRtgRsp(pBuffer, nLength);
		break;
	default:
		break;
	}
}
