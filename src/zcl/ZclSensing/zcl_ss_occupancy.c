/*
 * zcl_ss_occupancy.c
 *
 *  Created on: Apr 6, 2016
 *      Author: ChauNM
 */

#include "zcl_ss_occupancy.h"
#include "DevicesManager.h"
#include "log.h"
#include "ZNP_AF/ZnpAf.h"
#include "ZNP_ZDO/Znp_Zdo.h"
#include "ZnpActor.h"
#include "../fluent-logger/fluent-logger.h"
VOID ZclSsOccuAttrInit(PZCLSSOCCUPANCYATTR pData)
{
	pData->nState = 0;
	pData->nSensorType = 0;
}

BYTE ZclSsOccuGetAttr(WORD nDstAddr, BYTE nEp)
{
	BYTE nDataSize;
	PZCLPACKAGE pPackage;
	PWORD pAttrId;
	BYTE nTransID = ZclGetTranSeq();
	nDataSize = sizeof(WORD) * ZCL_SS_OCCUPANCY_ATTR_COUNT + ZCL_SHORT_HEADER;
	pPackage = (PZCLPACKAGE)malloc(nDataSize);
	pPackage->FrameControl.nDirection = 0;
	pPackage->FrameControl.nDisableDefaultRes = 0;
	pPackage->FrameControl.nFrameType = ZCL_FRAME_TYPE_PROFILE_CMD;
	pPackage->FrameControl.nManuSepcific = 0;
	pPackage->FrameControl.nReserved = 0;
	pPackage->ShortPackage.nCommandID = ZCL_CMD_READ;
	pPackage->ShortPackage.nTranSeqNumber = nTransID;
	pAttrId = (PWORD)pPackage->ShortPackage.pData;
	pAttrId[0] = ZCL_SS_OCCUPANCY_STATE_ATTR;
	pAttrId[1] = ZCL_SS_OCCUPANCY_SS_TYPE_ATTR;
	ZclAddMsgStatus(nDstAddr, nTransID);
	AfDataReq(nDstAddr, nEp, ZnpGetDefaultEp(), ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING, (PBYTE)pPackage, nDataSize, TRUE);
	free((PBYTE)pPackage);
	return nTransID;
}

VOID ZclSsOccuUpdateReadAttr(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen)
{
	PCLUSTERINFO pCluster = DeviceFindClusterInfo(nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING);
	if (pCluster == NULL)
		return;
	PZCLSSOCCUPANCYATTR pClusterAttr = (PZCLSSOCCUPANCYATTR)pCluster->pData;
	if (pClusterAttr == NULL)
		return;
	PZCLATTRREADSTRUCT pReadAttr = (PZCLATTRREADSTRUCT)pData;
	WORD nAttrDataLength;
	PZNPACTORDATA znpData;
	/* comment phan get Endpoint device type
	PENDPOINTINFO pEpInfo = DeviceFindEpInfo(nNwkAddr, nEp);
	if (pEpInfo != NULL)
		pEpInfo->nDeviceType = ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING;
	*/
	char* pLogString = malloc(250);
	while (nLen > 0)
	{
		if (pReadAttr->nState != 0)
		{
			nLen -= sizeof(pReadAttr->nAttrID) + sizeof(pReadAttr->nState);
		}
		else
		{
			switch (pReadAttr->nAttrID)
			{
			case ZCL_SS_OCCUPANCY_STATE_ATTR:
				pClusterAttr->nState = *((PBYTE)(pReadAttr->pData));
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				int znpValue = pClusterAttr->nState;
				znpData = ZnpActorMakeData("occupancy", ZNP_DATA_TYPE_INTERGER, &znpValue, sizeof(int));
				sprintf(pLogString, "occuoancy data update from 0x%04x endpoint %d value %d", nNwkAddr, nEp, znpValue);
				FLUENT_LOGGER_INFO(pLogString);
				ZnpActorPublishDeviceDataEvent(DeviceFind(nNwkAddr)->IeeeAddr, nEp, 1, &znpData);
				ZnpActorDestroyZnpData(znpData);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_SS_OCCUPANCY_SS_TYPE_ATTR:
				pClusterAttr->nSensorType = *((PBYTE)(pReadAttr->pData));
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			default:
				nAttrDataLength = ZclGetDataLength(pReadAttr->nDataType, pReadAttr->pData);
				if (nAttrDataLength == 0xFFFF)
					// cac kieu data chua duoc ho tro array, struct, set..
					return;
				else
				{
					nLen -= sizeof(ZCLATTRREADSTRUCT) + nAttrDataLength;
					pData += sizeof(ZCLATTRREADSTRUCT) + nAttrDataLength;
					pReadAttr = (PZCLATTRREADSTRUCT)pData;
				}
				break;
			}
		}
	}
	free(pLogString);
}

VOID ZclSsOccuParseWriteAttrRsp(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLength)
{
	PCLUSTERINFO pClusterInfo = DeviceFindClusterInfo(nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING);
	if (pClusterInfo == NULL)
		return;
	PZCLSSOCCUPANCYATTR pAttrData = (PZCLSSOCCUPANCYATTR)pClusterInfo->pData;
	if (pAttrData == NULL)
		return;
	PZCLATTRWRITERSPSTRUCT pWriteRsp = (PZCLATTRWRITERSPSTRUCT)pData;
	if (nLength == 1)
	{
		printf("Attributes write success\n");
		nLength = 0;
	}
	while (nLength > 0)
	{
		printf("Write attribute 0x%04X ", pWriteRsp->nAttrId);
		if (pWriteRsp->nStatus == 0)
			printf("success\n");
		else
			printf("fail\n");
		nLength -= sizeof(ZCLATTRWRITERSPSTRUCT);
		pWriteRsp++;
	}
}

VOID ZclSsOccuParseAttrReport(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen)
{
	PCLUSTERINFO pZclInfo = DeviceFindClusterInfo(nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING);
	PZCLATTRREPORTSTRUCT pReportData = (PZCLATTRREPORTSTRUCT)pData;
	WORD nAttrDataLength;
	if (pZclInfo == NULL)
		return;
	PZCLSSOCCUPANCYATTR pClusterAttr = (PZCLSSOCCUPANCYATTR)pZclInfo->pData;
	if (pZclInfo->pData == NULL)
		return;
	char* pLogString;
	PZNPACTORDATA znpData;
	while (nLen > 0)
	{
		switch (pReportData->nAttrID)
		{
		case ZCL_SS_OCCUPANCY_STATE_ATTR:
			pClusterAttr->nState = *((PBYTE)(pReportData->pData));
			nLen -= sizeof(ZCLATTRREPORTSTRUCT) + sizeof(BYTE);
			pData += sizeof(ZCLATTRREPORTSTRUCT) + sizeof(BYTE);
			pLogString = (char*)malloc(200);
			sprintf(pLogString, "Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: %d",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING, pReportData->nAttrID, pClusterAttr->nState);
			LogWrite(pLogString);
			FLUENT_LOGGER_INFO(pLogString);
			//MqttClientPublishMessage(pLogString);
			printf("Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: %d\n",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING, pReportData->nAttrID, pClusterAttr->nState);
			free(pLogString);
			int znpValue = pClusterAttr->nState;
			znpData = ZnpActorMakeData("occupancy", ZNP_DATA_TYPE_INTERGER, &znpValue, sizeof(int));
			ZnpActorPublishDeviceDataEvent(DeviceFind(nNwkAddr)->IeeeAddr, nEp, 1, &znpData);
			ZnpActorDestroyZnpData(znpData);
			pReportData = (PZCLATTRREPORTSTRUCT)pData;
			break;
		default:
			nAttrDataLength = ZclGetDataLength(pReportData->nDataType, pReportData->pData);
			if (nAttrDataLength == 0xFFFF)
				return;
			else
			{
				nLen -= sizeof(ZCLATTRREPORTSTRUCT) + nAttrDataLength;
				pData += sizeof(ZCLATTRREPORTSTRUCT) + nAttrDataLength;
				pReportData = (PZCLATTRREPORTSTRUCT)pData;
			}
			break;
		}
	}
}

BYTE ZclSsOccuConfig(WORD nDstAddr, BYTE nEp)
{
	PZCLPACKAGE pPackage;
	BYTE nDataSize;
	BYTE nTransID = ZclGetTranSeq();
	// make and
	PCLUSTERINFO pClusterInfo = DeviceFindClusterInfo(nDstAddr, nEp, ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING);
	if ( pClusterInfo == NULL)
		return 0;
	// request to bind
	ZnpZdoBindReq(nDstAddr, nEp, ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING);
	DeviceWaitInformed(nDstAddr, ZDO_BIND_RSP, 0xFF);
	// make config report package
	//nDataSize = 2 * (sizeof(ZCLSRVRPTCFG) + sizeof(BYTE)) + ZCL_SHORT_HEADER;
	nDataSize = sizeof(ZCLSRVRPTCFG) + ZCL_SHORT_HEADER;
	pPackage = (PZCLPACKAGE)malloc(nDataSize);
	pPackage->FrameControl.nDirection = 0;
	pPackage->FrameControl.nDisableDefaultRes = 0;
	pPackage->FrameControl.nFrameType = ZCL_FRAME_TYPE_PROFILE_CMD;
	pPackage->FrameControl.nManuSepcific = 0;
	pPackage->FrameControl.nReserved = 0;
	pPackage->ShortPackage.nCommandID = ZCL_CMD_CONFIG_REPORT;
	pPackage->ShortPackage.nTranSeqNumber = nTransID;
	PZCLSRVRPTCFG pRptConfig = (PZCLSRVRPTCFG)pPackage->ShortPackage.pData;
	pRptConfig->nDir = 0x00;
	pRptConfig->nAttrId = ZCL_SS_OCCUPANCY_STATE_ATTR;
	pRptConfig->nDataType = ZCL_DATATYPE_BITMAP8;
	pRptConfig->nMinReport = 10;
	pRptConfig->nMaxReport = 300;
	//pRptConfig->pData[0] = 1;
	ZclAddMsgStatus(nDstAddr, nTransID);
	AfDataReq(nDstAddr, nEp, ZnpGetDefaultEp(), ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING, (PBYTE)pPackage, nDataSize, TRUE);
	//DeviceSetTimeoutTime(nDstAddr, DEFAULT_MESSAGE_TIMEOUT);
	free(pPackage);
	return nTransID;
}

