/*
 * zcl_ss_temp.c
 *
 *  Created on: Mar 23, 2016
 *      Author: ChauNM
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zcl_ss_temp.h"
#include "DevicesManager.h"
#include "log.h"
#include "ZNP_AF/ZnpAf.h"
#include "ZNP_ZDO/Znp_Zdo.h"
#include "ZnpActor.h"
#include "../fluent-logger/fluent-logger.h"
//#include "mqtt_client.h"

VOID ZclSsTempAttrInit(PZCLSSTEMPATTR pData)
{
	pData->nMeasuredValue = 0;
	pData->nMinValue = 0;
	pData->nMaxValue = 0;
	pData->nTolerance = 0;
}

BYTE ZclSsTempGetAttr(WORD nDstAddr, BYTE nEp)
{
	BYTE nDataSize;
	PZCLPACKAGE pPackage;
	PWORD pAttrId;
	BYTE nTransID = ZclGetTranSeq();
	nDataSize = sizeof(WORD) * ZCL_SS_TEMP_ATTR_COUNT + ZCL_SHORT_HEADER;
	pPackage = (PZCLPACKAGE)malloc(nDataSize);
	pPackage->FrameControl.nDirection = 0;
	pPackage->FrameControl.nDisableDefaultRes = 0;
	pPackage->FrameControl.nFrameType = ZCL_FRAME_TYPE_PROFILE_CMD;
	pPackage->FrameControl.nManuSepcific = 0;
	pPackage->FrameControl.nReserved = 0;
	pPackage->ShortPackage.nCommandID = ZCL_CMD_READ;
	pPackage->ShortPackage.nTranSeqNumber = nTransID;
	pAttrId = (PWORD)pPackage->ShortPackage.pData;
	pAttrId[0] = ZCL_SS_TEMP_MEASURED_ATTR;
	pAttrId[1] = ZCL_SS_TEMP_MIN_ATTR;
	pAttrId[2] = ZCL_SS_TEMP_MAX_ATTR;
	pAttrId[3] = ZCL_SS_TEMP_TOLERANCE_ATTR;
	ZclAddMsgStatus(nDstAddr, nTransID);
	AfDataReq(nDstAddr, nEp, ZnpGetDefaultEp(), ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, (PBYTE)pPackage, nDataSize, TRUE);
	free((PBYTE)pPackage);
	return nTransID;
}

VOID ZclSsTempUpdateReadAttr(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen)
{
	PCLUSTERINFO pCluster = DeviceFindClusterInfo(nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT);
	if (pCluster == NULL)
		return;
	PZCLSSTEMPATTR pClusterAttr = (PZCLSSTEMPATTR)pCluster->pData;
	if (pClusterAttr == NULL)
		return;
	PZCLATTRREADSTRUCT pReadAttr = (PZCLATTRREADSTRUCT)pData;
	WORD nAttrDataLength;

	PZNPACTORDATA znpData;
	printf("Update cluster data\n");
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
			case ZCL_SS_TEMP_MEASURED_ATTR:
				pClusterAttr->nMeasuredValue = *((PSWORD)(pReadAttr->pData));
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(SWORD);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(SWORD);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				double znpValue = (double)pClusterAttr->nMeasuredValue / 100;
				znpData = ZnpActorMakeData("temperature", ZNP_DATA_TYPE_FLOAT, &znpValue, sizeof(znpValue));
				ZnpActorPublishDeviceDataEvent(DeviceFind(nNwkAddr)->IeeeAddr, nEp, 1, &znpData);
				ZnpActorDestroyZnpData(znpData);
				break;
			case ZCL_SS_TEMP_MIN_ATTR:
				pClusterAttr->nMinValue = *((PSWORD)(pReadAttr->pData));
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(SWORD);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(SWORD);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_SS_TEMP_MAX_ATTR:
				pClusterAttr->nMinValue = *((PSWORD)(pReadAttr->pData));
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(SWORD);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(SWORD);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_SS_TEMP_TOLERANCE_ATTR:
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

VOID ZclSsTempParseWriteAttrRsp(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLength)
{
	PCLUSTERINFO pClusterInfo = DeviceFindClusterInfo(nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT);
	if (pClusterInfo == NULL)
		return;
	PZCLSSTEMPATTR pAttrData = (PZCLSSTEMPATTR)pClusterInfo->pData;
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
		if (pWriteRsp->nStatus == 0)
			printf("success\n");
		else
			printf("fail\n");
		nLength -= sizeof(ZCLATTRWRITERSPSTRUCT);
		pWriteRsp++;
	}
}

VOID ZclSsTempParseAttrReport(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen)
{
	PCLUSTERINFO pZclInfo = DeviceFindClusterInfo(nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT);
	PZCLATTRREPORTSTRUCT pReportData = (PZCLATTRREPORTSTRUCT)pData;
	WORD nAttrDataLength;
	if (pZclInfo == NULL)
		return;
	PZCLSSTEMPATTR pClusterAttr = (PZCLSSTEMPATTR)pZclInfo->pData;
	if (pZclInfo->pData == NULL)
		return;
	char* pLogString;
	PZNPACTORDATA znpData;
	while (nLen > 0)
	{
		switch (pReportData->nAttrID)
		{
		case ZCL_SS_TEMP_MEASURED_ATTR:
			pClusterAttr->nMeasuredValue = *((PSWORD)(pReportData->pData));
			nLen -= sizeof(ZCLATTRREPORTSTRUCT) + sizeof(SWORD);
			pData += sizeof(ZCLATTRREPORTSTRUCT) + sizeof(SWORD);
			pLogString = (char*)malloc(200);
			sprintf(pLogString, "Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: %d",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, pReportData->nAttrID, pClusterAttr->nMeasuredValue);
			LogWrite(pLogString);
			printf("Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value:%d\n",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, pReportData->nAttrID, pClusterAttr->nMeasuredValue);
			FLUENT_LOGGER_INFO(pLogString);
			free(pLogString);
			double znpValue = ((double)(pClusterAttr->nMeasuredValue)) / 100;
			znpData = ZnpActorMakeData("temperature", ZNP_DATA_TYPE_FLOAT, &znpValue, sizeof(znpValue));
			ZnpActorPublishDeviceDataEvent(DeviceFind(nNwkAddr)->IeeeAddr, nEp, 1, &znpData);
			ZnpActorDestroyZnpData(znpData);
			pReportData = (PZCLATTRREPORTSTRUCT)pData;
			break;
		case ZCL_SS_TEMP_MIN_ATTR:
			pClusterAttr->nMinValue = *((PSWORD)(pReportData->pData));
			nLen -= sizeof(ZCLATTRREPORTSTRUCT) + sizeof(SWORD);
			pData += sizeof(ZCLATTRREPORTSTRUCT) + sizeof(SWORD);
			pLogString = (char*)malloc(200);
			sprintf(pLogString, "Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: %d",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, pReportData->nAttrID, pClusterAttr->nMeasuredValue);
			LogWrite(pLogString);
			FLUENT_LOGGER_INFO(pLogString);
			printf("Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: %d\n",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, pReportData->nAttrID, pClusterAttr->nMinValue);
			free(pLogString);
			pReportData = (PZCLATTRREPORTSTRUCT)pData;
			break;
		case ZCL_SS_TEMP_MAX_ATTR:
			pClusterAttr->nMaxValue = *((PSWORD)(pReportData->pData));
			nLen -= sizeof(ZCLATTRREPORTSTRUCT) + sizeof(SWORD);
			pData += sizeof(ZCLATTRREPORTSTRUCT) + sizeof(SWORD);
			pLogString = (char*)malloc(200);
			sprintf(pLogString, "Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: 0x%d",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, pReportData->nAttrID, pClusterAttr->nMeasuredValue);
			LogWrite(pLogString);
			FLUENT_LOGGER_INFO(pLogString);
			printf("Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: %d\n",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, pReportData->nAttrID, pClusterAttr->nMaxValue);
			free(pLogString);
			pReportData = (PZCLATTRREPORTSTRUCT)pData;
			break;
		case ZCL_SS_TEMP_TOLERANCE_ATTR:
			pClusterAttr->nTolerance = *((PWORD)(pReportData->pData));
			nLen -= sizeof(ZCLATTRREPORTSTRUCT) + sizeof(WORD);
			pData += sizeof(ZCLATTRREPORTSTRUCT) + sizeof(WORD);
			pLogString = (char*)malloc(200);
			sprintf(pLogString, "Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: %d",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, pReportData->nAttrID, pClusterAttr->nTolerance);
			LogWrite(pLogString);
			FLUENT_LOGGER_INFO(pLogString);
			printf("Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: %d\n",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, pReportData->nAttrID, pClusterAttr->nTolerance);
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

BYTE ZclSsTempConfig(WORD nDstAddr, BYTE nEp)
{
	PZCLPACKAGE pPackage;
	BYTE nDataSize;
	BYTE nTransID = ZclGetTranSeq();
	// make and
	PCLUSTERINFO pClusterInfo = DeviceFindClusterInfo(nDstAddr, nEp, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT);
	if ( pClusterInfo == NULL)
		return 0;
	// request to bind
	ZnpZdoBindReq(nDstAddr, nEp, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT);
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
	pRptConfig->nAttrId = ZCL_SS_TEMP_MEASURED_ATTR;
	pRptConfig->nDataType = ZCL_DATATYPE_INT16;
	pRptConfig->nMinReport = 10;
	pRptConfig->nMaxReport = 3600;
	ZclAddMsgStatus(nDstAddr, nTransID);
	AfDataReq(nDstAddr, nEp, ZnpGetDefaultEp(), ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, (PBYTE)pPackage, nDataSize, TRUE);
	//DeviceSetTimeoutTime(nDstAddr, DEFAULT_MESSAGE_TIMEOUT);
	free(pPackage);
	return nTransID;
}
