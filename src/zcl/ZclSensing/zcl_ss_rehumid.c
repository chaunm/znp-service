/*
 * zcl_ss_rehumid.c
 *
 *  Created on: Mar 23, 2016
 *      Author: ChauNM
 */


//#include "zcl.h"
#include "zcl_ss_rehumid.h"
#include "DevicesManager.h"
#include "log.h"
#include "ZNP_AF/ZnpAf.h"
#include "ZNP_ZDO/Znp_Zdo.h"
#include "ZnpActor.h"
#include "../fluent-logger/fluent-logger.h"

VOID ZclSsReHumidAttrInit(PZCLSSREHUMIDATTR pData)
{
	pData->nMeasuredValue = 0;
	pData->nMinValue = 0;
	pData->nMaxValue = 0;
	pData->nTolerance = 0;
}

BYTE ZclSsReHumidGetAttr(WORD nDstAddr, BYTE nEp)
{
	BYTE nDataSize;
	PZCLPACKAGE pPackage;
	PWORD pAttrId;
	BYTE nTransID = ZclGetTranSeq();
	nDataSize = sizeof(WORD) * ZCL_SS_REHUMID_ATTR_COUNT + ZCL_SHORT_HEADER;
	pPackage = (PZCLPACKAGE)malloc(nDataSize);
	pPackage->FrameControl.nDirection = 0;
	pPackage->FrameControl.nDisableDefaultRes = 0;
	pPackage->FrameControl.nFrameType = ZCL_FRAME_TYPE_PROFILE_CMD;
	pPackage->FrameControl.nManuSepcific = 0;
	pPackage->FrameControl.nReserved = 0;
	pPackage->ShortPackage.nCommandID = ZCL_CMD_READ;
	pPackage->ShortPackage.nTranSeqNumber = nTransID;
	pAttrId = (PWORD)pPackage->ShortPackage.pData;
	pAttrId[0] = ZCL_SS_REHUMID_MEASURED_ATTR;
	pAttrId[1] = ZCL_SS_REHUMID_MIN_ATTR;
	pAttrId[2] = ZCL_SS_REHUMID_MAX_ATTR;
	pAttrId[3] = ZCL_SS_REHUMID_TOLERANCE_ATTR;
	ZclAddMsgStatus(nDstAddr, nTransID);
	AfDataReq(nDstAddr, nEp, ZnpGetDefaultEp(), ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY, (PBYTE)pPackage, nDataSize, TRUE);
	//DeviceSetTimeoutTime(nDstAddr, DEFAULT_MESSAGE_TIMEOUT);
	free((PBYTE)pPackage);
	return nTransID;
}

VOID ZclSsReHumidUpdateReadAttr(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen)
{
	PCLUSTERINFO pCluster = DeviceFindClusterInfo(nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY);
	if (pCluster == NULL)
		return;
	PZCLSSREHUMIDATTR pClusterAttr = (PZCLSSREHUMIDATTR)pCluster->pData;
	if (pClusterAttr == NULL)
		return;
	PZCLATTRREADSTRUCT pReadAttr = (PZCLATTRREADSTRUCT)pData;
	WORD nAttrDataLength;

	PZNPACTORDATA znpData;
	printf("Update humidity attribute data\n");
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
			case ZCL_SS_REHUMID_MEASURED_ATTR:
				pClusterAttr->nMeasuredValue = *((PWORD)(pReadAttr->pData));
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				double znpValue = (double)pClusterAttr->nMeasuredValue / 100;
				znpData = ZnpActorMakeData("humidity", ZNP_DATA_TYPE_FLOAT, &znpValue, sizeof(double));
				ZnpActorPublishDeviceDataEvent(DeviceFind(nNwkAddr)->IeeeAddr, nEp, 1, &znpData);
				ZnpActorDestroyZnpData(znpData);
				break;
			case ZCL_SS_REHUMID_MIN_ATTR:
				pClusterAttr->nMinValue = *((PWORD)(pReadAttr->pData));
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_SS_REHUMID_MAX_ATTR:
				pClusterAttr->nMinValue = *((PWORD)(pReadAttr->pData));
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_SS_REHUMID_TOLERANCE_ATTR:
				pClusterAttr->nTolerance = *((PWORD)(pReadAttr->pData));
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
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
	printf("Update cluster data finished\n");
}

VOID ZclSsReHumidParseWriteAttrRsp(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLength)
{
	PCLUSTERINFO pClusterInfo = DeviceFindClusterInfo(nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY);
	if (pClusterInfo == NULL)
		return;
	PZCLSSREHUMIDATTR pAttrData = (PZCLSSREHUMIDATTR)pClusterInfo->pData;
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

VOID ZclSsReHumidParseAttrReport(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen)
{
	PCLUSTERINFO pZclInfo = DeviceFindClusterInfo(nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY);
	PZCLATTRREPORTSTRUCT pReportData = (PZCLATTRREPORTSTRUCT)pData;
	WORD nAttrDataLength;
	if (pZclInfo == NULL)
		return;
	PZCLSSREHUMIDATTR pClusterAttr = (PZCLSSREHUMIDATTR)pZclInfo->pData;
	if (pZclInfo->pData == NULL)
		return;
	char* pLogString;
	PZNPACTORDATA znpData;
	while (nLen > 0)
	{
		switch (pReportData->nAttrID)
		{
		case ZCL_SS_REHUMID_MEASURED_ATTR:
			pClusterAttr->nMeasuredValue = *((PWORD)(pReportData->pData));
			nLen -= sizeof(ZCLATTRREPORTSTRUCT) + sizeof(WORD);
			pData += sizeof(ZCLATTRREPORTSTRUCT) + sizeof(WORD);
			pLogString = (char*)malloc(200);
			sprintf(pLogString, "Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: %d",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY, pReportData->nAttrID, pClusterAttr->nMeasuredValue);
			LogWrite(pLogString);
			FLUENT_LOGGER_INFO(pLogString);
			printf("Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: %d\n",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY, pReportData->nAttrID, pClusterAttr->nMeasuredValue);
			free(pLogString);
			pReportData = (PZCLATTRREPORTSTRUCT)pData;
			double znpValue = (double)pClusterAttr->nMeasuredValue / 100;
			znpData = ZnpActorMakeData("humidity", ZNP_DATA_TYPE_FLOAT, &znpValue, sizeof(double));
			ZnpActorPublishDeviceDataEvent(DeviceFind(nNwkAddr)->IeeeAddr, nEp, 1, &znpData);
			ZnpActorDestroyZnpData(znpData);
			break;
		case ZCL_SS_REHUMID_MIN_ATTR:
			pClusterAttr->nMinValue = *((PWORD)(pReportData->pData));
			nLen -= sizeof(ZCLATTRREPORTSTRUCT) + sizeof(WORD);
			pData += sizeof(ZCLATTRREPORTSTRUCT) + sizeof(WORD);
			pLogString = (char*)malloc(200);
			sprintf(pLogString, "Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: %d",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY, pReportData->nAttrID, pClusterAttr->nMeasuredValue);
			LogWrite(pLogString);
			FLUENT_LOGGER_INFO(pLogString);
			printf("Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: %d\n",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY, pReportData->nAttrID, pClusterAttr->nMinValue);
			free(pLogString);
			pReportData = (PZCLATTRREPORTSTRUCT)pData;
			break;
		case ZCL_SS_REHUMID_MAX_ATTR:
			pClusterAttr->nMaxValue = *((PWORD)(pReportData->pData));
			nLen -= sizeof(ZCLATTRREPORTSTRUCT) + sizeof(WORD);
			pData += sizeof(ZCLATTRREPORTSTRUCT) + sizeof(WORD);
			pLogString = (char*)malloc(200);
			sprintf(pLogString, "Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: %d",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY, pReportData->nAttrID, pClusterAttr->nMeasuredValue);
			LogWrite(pLogString);
			FLUENT_LOGGER_INFO(pLogString);
			printf("Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: %d\n",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY, pReportData->nAttrID, pClusterAttr->nMaxValue);
			free(pLogString);
			pReportData = (PZCLATTRREPORTSTRUCT)pData;
			break;
		case ZCL_SS_REHUMID_TOLERANCE_ATTR:
			pClusterAttr->nTolerance = *((PWORD)(pReportData->pData));
			nLen -= sizeof(ZCLATTRREPORTSTRUCT) + sizeof(WORD);
			pData += sizeof(ZCLATTRREPORTSTRUCT) + sizeof(WORD);
			pLogString = (char*)malloc(200);
			sprintf(pLogString, "Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: %d",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY, pReportData->nAttrID, pClusterAttr->nTolerance);
			LogWrite(pLogString);
			FLUENT_LOGGER_INFO(pLogString);
			printf("Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: %d\n",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY, pReportData->nAttrID, pClusterAttr->nTolerance);
			free(pLogString);
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

BYTE ZclSsReHumidConfig(WORD nDstAddr, BYTE nEp)
{
	PZCLPACKAGE pPackage;
	BYTE nDataSize;
	BYTE nTransID = ZclGetTranSeq();
	// make and
	PCLUSTERINFO pClusterInfo = DeviceFindClusterInfo(nDstAddr, nEp, ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY);
	if ( pClusterInfo == NULL)
		return 0;
	// request to bind
	ZnpZdoBindReq(nDstAddr, nEp, ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY);
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
	pRptConfig->nAttrId = ZCL_SS_REHUMID_MEASURED_ATTR;
	pRptConfig->nDataType = ZCL_DATATYPE_UINT16;
	pRptConfig->nMinReport = 10;
	pRptConfig->nMaxReport = 3600;
	ZclAddMsgStatus(nDstAddr, nTransID);
	AfDataReq(nDstAddr, nEp, ZnpGetDefaultEp(), ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY, (PBYTE)pPackage, nDataSize, TRUE);
	//DeviceSetTimeoutTime(nDstAddr, DEFAULT_MESSAGE_TIMEOUT);
	free(pPackage);
	printf("Configuration finish\n");
	return nTransID;
}
