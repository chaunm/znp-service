/*
 * zcl.h
 *
 *  Created on: Feb 26, 2016
 *      Author: ChauNM
 */

#include "zcl.h"
#include "stdio.h"
#include "ZNP_AF/ZnpAf.h"
#include "DevicesManager.h"
#include "log.h"
#include "universal.h"
#include "../fluent-logger/fluent-logger.h"

static BYTE nZclTranSeq = 1;
static PZCLMSGSTATUS pFirstAfMsgStatus = NULL;

VOID ZclAddMsgStatus(WORD nDstAddress, BYTE nTransID)
{
	PZCLMSGSTATUS pAfMsgStatus = (PZCLMSGSTATUS)malloc(sizeof(ZCLMSGSTATUS));
	PZCLMSGSTATUS pLastAfStatus;
	printf("\e[1;31m Waiting for Zcl response on device 0x%04X transID %d\n\e[1;32m", nDstAddress, nTransID);
	pAfMsgStatus->nDstAddress = nDstAddress;
	pAfMsgStatus->nTransID = nTransID;
	pAfMsgStatus->nTimeout = AF_MESSAGE_TIMEOUT;
	pAfMsgStatus->NextZclMsg = NULL;
	if (pFirstAfMsgStatus == NULL)
	{
		pFirstAfMsgStatus = pAfMsgStatus;
	}
	else
	{
		pLastAfStatus = pFirstAfMsgStatus;
		while(pLastAfStatus->NextZclMsg !=NULL)
		{
			pLastAfStatus = pLastAfStatus->NextZclMsg;
		}
		pLastAfStatus->NextZclMsg = pAfMsgStatus;
	}
}

VOID ZclDeleteMsgStatus(WORD nDstAddr, WORD nTransID)
{
	PZCLMSGSTATUS pCurrentMsgStatus = pFirstAfMsgStatus;
	PZCLMSGSTATUS pDeletedMsgStatus;
	printf("\e[1;31m Zcl receive from device 0x%04X transID %d\n\e[1;32m", nDstAddr, nTransID);
	if (pFirstAfMsgStatus == NULL) return;
	while ((pCurrentMsgStatus->nDstAddress == nDstAddr) && (pCurrentMsgStatus->nTransID == nTransID))
	{
		pFirstAfMsgStatus = pFirstAfMsgStatus->NextZclMsg;
		free(pCurrentMsgStatus);
		pCurrentMsgStatus = pFirstAfMsgStatus;
		if (pCurrentMsgStatus == NULL) return;
	}
	while (pCurrentMsgStatus->NextZclMsg != NULL)
	{
		pDeletedMsgStatus = pCurrentMsgStatus->NextZclMsg;
		if ((pDeletedMsgStatus->nDstAddress == nDstAddr) && (pDeletedMsgStatus->nTransID == nTransID))
		{
			pCurrentMsgStatus->NextZclMsg = pDeletedMsgStatus->NextZclMsg;
			free(pDeletedMsgStatus);
		}
		else
		{
			pCurrentMsgStatus = pDeletedMsgStatus;
		}
	}
}

VOID ZclMsgStatusProcess()
{
	PZCLMSGSTATUS pMsgStatus = pFirstAfMsgStatus;
	WORD nDeletedAddress;
	char* LogString;
	BYTE nDeletedTransId;
	while (pMsgStatus != NULL)
	{
		if (pMsgStatus->nTimeout > 0)
		{
			pMsgStatus->nTimeout--;
			pMsgStatus = pMsgStatus->NextZclMsg;
		}
		else
		{
			nDeletedAddress = pMsgStatus->nDstAddress;
			nDeletedTransId = pMsgStatus->nTransID;
			pMsgStatus = pMsgStatus->NextZclMsg;
			ZclDeleteMsgStatus(nDeletedAddress, nDeletedTransId);
			printf("\e[1;31m No response on device 0x%04X transID %d\n\e[1;33m", nDeletedAddress, nDeletedTransId);
			LogString = (char*)malloc(500);
			sprintf(LogString, "No response on device 0x%04X transID %d", nDeletedAddress, nDeletedTransId);
			LogWrite(LogString);
			FLUENT_LOGGER_WARN(LogString);
			free(LogString);
		}
	}
}

BYTE ZclGetTranSeq()
{
	nZclTranSeq++;
	if (nZclTranSeq == 0)
		nZclTranSeq = 1;
	return (nZclTranSeq - 1);
}

BYTE ZclParseReadRspData(WORD nNwkAddr, BYTE nEndpoint, WORD nClusterId, PBYTE pData, BYTE nLength)
{
	char* LogString = (char*)malloc(255);
	printf("Attribute read:\n");
	sprintf(LogString, "Attribute read from 0x%04X endpoint 0x%02X ClusterID 0x%04X", nNwkAddr, nEndpoint, nClusterId);
	FLUENT_LOGGER_INFO(LogString);
	LogWrite(LogString);
	free(LogString);
	BYTE nIndex = 0;
	while (nIndex < nLength)
	{
		printf("0x%02X ", pData[nIndex]);
		nIndex++;
	}
	printf("\n");
	switch (nClusterId)
	{
	case ZCL_CLUSTER_ID_SS_IAS_ZONE:
		ZclIasZoneUpdateReadAttr(nNwkAddr, nEndpoint, pData, nLength);
		break;
	case ZCL_CLUSTER_ID_GEN_POWER_CFG:
		ZclGenPwrCfgUpdateReadAttr(nNwkAddr, nEndpoint, pData, nLength);
		//return 1;
		break;
	case ZCL_CLUSTER_ID_GEN_ON_OFF:
		ZclGenOnOffUpdateReadAttr(nNwkAddr, nEndpoint, pData, nLength);
		break;
	case ZCL_CLUSTER_ID_GEN_ALARMS:
		ZclGenAlarmUpdateReadAttr(nNwkAddr, nEndpoint, pData, nLength);
		break;
	case ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT:
		ZclSsTempUpdateReadAttr(nNwkAddr, nEndpoint, pData, nLength);
		break;
	case ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY:
		ZclSsReHumidUpdateReadAttr(nNwkAddr, nEndpoint, pData, nLength);
		break;
	case ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING:
		ZclSsOccuUpdateReadAttr(nNwkAddr, nEndpoint, pData, nLength);
		break;
	default:
		break;
	}
	return 0;
}

BYTE ZclGetClusterAttr(WORD nDstAddr, BYTE nEndpoint, WORD nCluster)
{
	switch (nCluster)
	{
	case ZCL_CLUSTER_ID_GEN_POWER_CFG:
		return ZclGenPwrCfgGetAttr(nDstAddr, nEndpoint);
		break;
	case ZCL_CLUSTER_ID_GEN_ALARMS:
		//return ZclGenAlarmGetAttr(nDstAddr, nEndpoint);
		break;
	case ZCL_CLUSTER_ID_SS_IAS_ZONE:
		return ZclIasZoneGetAttr(nDstAddr, nEndpoint);
		break;
	case ZCL_CLUSTER_ID_GEN_ON_OFF:
		return ZclGenOnOffGetAttr(nDstAddr, nEndpoint);
		break;
	case ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT:
		return ZclSsTempGetAttr(nDstAddr, nEndpoint);
		break;
	case ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY:
		return ZclSsReHumidGetAttr(nDstAddr, nEndpoint);
		break;
	case ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING:
		return ZclSsOccuGetAttr(nDstAddr, nEndpoint);
		break;
	default:
		break;
	}
	return 0xFF;
}

BYTE ZclParseReportData(WORD nNwkAddr, BYTE nEp, WORD nClusterId, PBYTE pData, BYTE nLength)
{
	char* LogString;
	BYTE nIndex = 0;
	printf("Attribute reported:\n");
	while (nIndex < nLength)
	{
		printf("0x%02X ", pData[nIndex]);
		nIndex++;
	}
	printf("\n");
	switch (nClusterId)
	{
	case ZCL_CLUSTER_ID_GEN_ON_OFF:
		ZclGenOnOffParseAttrReport(nNwkAddr, nEp, pData, nLength);
		break;
	case ZCL_CLUSTER_ID_SS_IAS_ZONE:
		ZclIasZoneParseAttrReport(nNwkAddr, nEp, pData, nLength);
		break;
	case ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT:
		ZclSsTempParseAttrReport(nNwkAddr, nEp, pData, nLength);
		break;
	case ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY:
		ZclSsReHumidParseAttrReport(nNwkAddr, nEp, pData, nLength);
		break;
	case ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING:
		ZclSsOccuParseAttrReport(nNwkAddr, nEp, pData, nLength);
		break;
	default:
		LogString = (char*)malloc(255);
		sprintf(LogString, "Attribute reported from 0x%04X endpoint 0x%02X cluster 0x%04X", nNwkAddr, nEp, nClusterId);
		FLUENT_LOGGER_INFO(LogString);
		LogWrite(LogString);
		free(LogString);
		break;
	}
	return 0;
}

BYTE ZclParseWriteAttrRsp(WORD nNwkAddr, BYTE nEp, WORD nClusterId, PBYTE pData, BYTE nLength)
{
	switch (nClusterId)
	{
	case ZCL_CLUSTER_ID_SS_IAS_ZONE:
		ZclIasZoneParseWriteAttrRsp(nNwkAddr, nEp, pData, nLength);
		return 1;
		break;
	case ZCL_CLUSTER_ID_GEN_POWER_CFG:
		ZclGenPwrCfgParseWriteAttrRsp(nNwkAddr, nEp, pData, nLength);
		return 1;
	case ZCL_CLUSTER_ID_GEN_ON_OFF:
		ZclGenOnOffParseWriteAttrRsp(nNwkAddr, nEp, pData, nLength);
		return 1;
		break;
	case ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT:
		ZclSsTempParseWriteAttrRsp(nNwkAddr, nEp, pData, nLength);
		return 1;
		break;
	case ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY:
		ZclSsReHumidParseWriteAttrRsp(nNwkAddr, nEp, pData, nLength);
		return 1;
		break;
	case ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING:
		ZclSsOccuParseWriteAttrRsp(nNwkAddr, nEp, pData, nLength);
		return 1;
		break;
	default:
		break;
	}
	return 0;
}

BYTE ZclConfigCluster(WORD nDstAddr, BYTE nEndpoint, WORD nCluster)
{
	switch (nCluster)
	{
	case ZCL_CLUSTER_ID_GEN_POWER_CFG:
		return ZclGenPwrCfgConfig(nDstAddr, nEndpoint);
		break;
	case ZCL_CLUSTER_ID_GEN_ON_OFF:
		return ZclGenOnOffConfig(nDstAddr, nEndpoint);
		break;
	case ZCL_CLUSTER_ID_GEN_ALARMS:
		return ZclGenAlarmConfig(nDstAddr, nEndpoint);
		break;
	case ZCL_CLUSTER_ID_SS_IAS_ZONE:
		return ZclIasZoneConfig(nDstAddr, nEndpoint);
		break;
	case ZCL_CLUSTER_ID_SS_IAS_ACE:
		return ZclIasAceConfig(nDstAddr, nEndpoint);
		break;
	case ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT:
		return ZclSsTempConfig(nDstAddr, nEndpoint);
		break;
	case ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY:
		return ZclSsReHumidConfig(nDstAddr, nEndpoint);
		break;
	case ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING:
		return ZclSsOccuConfig(nDstAddr, nEndpoint);
		break;
	default:
		break;
	}
	return 0xFF;
}

VOID ZclParseGeneralCommand(WORD nNwkAddr, BYTE nEndpoint, WORD nClusterId, BYTE nCommandId, PBYTE pZclData, BYTE nDataSize)
{
	switch(nCommandId)
	{
	case ZCL_CMD_READ:
		break;
	case ZCL_CMD_READ_RSP:
		ZclParseReadRspData(nNwkAddr, nEndpoint, nClusterId, pZclData, nDataSize);
		break;
	case ZCL_CMD_WRITE:
		break;
	case ZCL_CMD_WRITE_UNDIVIDED:
		break;
	case ZCL_CMD_WRITE_RSP:
		ZclParseWriteAttrRsp(nNwkAddr, nEndpoint, nClusterId, pZclData, nDataSize);
		break;
	case ZCL_CMD_WRITE_NO_RSP:
		break;
	case ZCL_CMD_CONFIG_REPORT:
		break;
	case ZCL_CMD_CONFIG_REPORT_RSP:
		break;
	case ZCL_CMD_READ_REPORT_CFG:
		break;
	case ZCL_CMD_READ_REPORT_CFG_RSP:
		break;
	case ZCL_CMD_REPORT:
		ZclParseReportData(nNwkAddr, nEndpoint, nClusterId, pZclData, nDataSize);
		break;
	case ZCL_CMD_DEFAULT_RSP:
		break;
	case ZCL_CMD_DISCOVER:
		break;
	case ZCL_CMD_DISCOVER_RSP:
		break;
	default:
		break;
	}
}

VOID ZclParseSpecificCommand(WORD nNwkAddr, BYTE nEndpoint, WORD nClusterId, BYTE nCommandId, PBYTE pZclData, BYTE nDataSize)
{
	switch (nClusterId)
	{
	case ZCL_CLUSTER_ID_SS_IAS_ACE:
		ZclIasAceParseSpecificPackage(nNwkAddr, nEndpoint, nCommandId, pZclData, nDataSize);
		break;
	case ZCL_CLUSTER_ID_SS_IAS_ZONE:
		ZclIasZoneParseSpecificPackage(nNwkAddr, nEndpoint, nCommandId, pZclData, nDataSize);
		break;
	case ZCL_CLUSTER_ID_GEN_ALARMS:
		ZclGenAlarmParseSpecificPackage(nNwkAddr, nEndpoint, nCommandId, pZclData, nDataSize);
		break;
	default:
		break;
	}
}

WORD ZclGetDataLength(BYTE nDataType, PBYTE pData)
{
	switch (nDataType)
	{
	case ZCL_DATATYPE_NO_DATA:
	case ZCL_DATATYPE_UNKNOWN:
		return 0;
		break;
	case ZCL_DATATYPE_BOOLEAN:
	case ZCL_DATATYPE_DATA8:
	case ZCL_DATATYPE_BITMAP8:
	case ZCL_DATATYPE_UINT8:
	case ZCL_DATATYPE_INT8:
	case ZCL_DATATYPE_ENUM8:
		return 1;
		break;
	case ZCL_DATATYPE_DATA16:
	case ZCL_DATATYPE_BITMAP16:
	case ZCL_DATATYPE_UINT16:
	case ZCL_DATATYPE_INT16:
	case ZCL_DATATYPE_ENUM16:
	case ZCL_DATATYPE_SEMI_PREC:
	case ZCL_DATATYPE_CLUSTER_ID:
	case ZCL_DATATYPE_ATTR_ID:
		return 2;
		break;
	case ZCL_DATATYPE_DATA24:
	case ZCL_DATATYPE_BITMAP24:
	case ZCL_DATATYPE_UINT24:
	case ZCL_DATATYPE_INT24:
		return 3;
		break;
	case ZCL_DATATYPE_DATA32:
	case ZCL_DATATYPE_BITMAP32:
	case ZCL_DATATYPE_UINT32:
	case ZCL_DATATYPE_INT32:
	case ZCL_DATATYPE_SINGLE_PREC:
	case ZCL_DATATYPE_TOD:
	case ZCL_DATATYPE_DATE:
	case ZCL_DATATYPE_UTC:
	case ZCL_DATATYPE_BAC_OID:
		return 4;
		break;
	case ZCL_DATATYPE_DATA40:
	case ZCL_DATATYPE_BITMAP40:
	case ZCL_DATATYPE_UINT40:
	case ZCL_DATATYPE_INT40:
		return 5;
		break;
	case ZCL_DATATYPE_DATA48:
	case ZCL_DATATYPE_BITMAP48:
	case ZCL_DATATYPE_UINT48:
	case ZCL_DATATYPE_INT48:
		return 6;
		break;
	case ZCL_DATATYPE_DATA56:
	case ZCL_DATATYPE_BITMAP56:
	case ZCL_DATATYPE_UINT56:
	case ZCL_DATATYPE_INT56:
		return 7;
		break;
	case ZCL_DATATYPE_DATA64:
	case ZCL_DATATYPE_BITMAP64:
	case ZCL_DATATYPE_UINT64:
	case ZCL_DATATYPE_INT64:
	case ZCL_DATATYPE_DOUBLE_PREC:
	case ZCL_DATATYPE_IEEE_ADDR:
		return 8;
		break;
	case ZCL_DATATYPE_OCTET_STR:
	case ZCL_DATATYPE_CHAR_STR:
		if (pData[1] == 0xFF)
			return 1;
		else
			return 1 + pData[1];
		break;
	case ZCL_DATATYPE_LONG_OCTET_STR:
	case ZCL_DATATYPE_LONG_CHAR_STR:
		if (((PWORD)pData)[1] == 0xFFFF)
			return 2;
		else
			return 2 + ((PWORD)pData)[1];
		break;
	default:
		return 0xFFFF;
		break;
	}
	return 0xFFFF;
}

BYTE ZclParsePackage(WORD nNwkAddr, BYTE nEndpoint, WORD nClusterId, PBYTE pPackage, BYTE nLength)
{
	PZCLPACKAGE pZclPackage = (PZCLPACKAGE)pPackage;
	PBYTE pZclData;
	BYTE nDataSize = 0;
	BYTE nCommandID;
	BYTE nTransID;
	if (pZclPackage->FrameControl.nManuSepcific == 0)
	{
		if (nLength < (sizeof(pZclPackage->FrameControl) + sizeof(pZclPackage->ShortPackage)))
		{
			printf("Data length is not valid\n");
			return 0xFF;
		}
		nDataSize = nLength - sizeof(pZclPackage->FrameControl) - sizeof(pZclPackage->ShortPackage);
		pZclData = pZclPackage->ShortPackage.pData;
		nCommandID = pZclPackage->ShortPackage.nCommandID;
		nTransID = pZclPackage->ShortPackage.nTranSeqNumber;
	}
	else
	{
		if (nLength < (sizeof(pZclPackage->FrameControl) + sizeof(pZclPackage->FullPackage)))
		{
			printf("Data length is not valid\n");
			return 0xFF;
		}
		nDataSize = nLength - sizeof(pZclPackage->FrameControl) - sizeof(pZclPackage->FullPackage);
		pZclData = pZclPackage->FullPackage.pData;
		nCommandID = pZclPackage->FullPackage.nCommandID;
		nTransID = pZclPackage->FullPackage.nTranSeqNumber;
	}
	ZclDeleteMsgStatus(nNwkAddr, nTransID);
	switch (pZclPackage->FrameControl.nFrameType)
	{
	case ZCL_FRAME_TYPE_PROFILE_CMD:
		ZclParseGeneralCommand(nNwkAddr, nEndpoint, nClusterId, nCommandID, pZclData, nDataSize);
		break;
	case ZCL_FRAME_TYPE_SPECIFIC_CMD:
		ZclParseSpecificCommand(nNwkAddr, nEndpoint, nClusterId, nCommandID, pZclData, nDataSize);
		break;
	default:
		break;
	}
	return nTransID;
}

BYTE ZclReadAttribute(WORD nNwkAddr, BYTE nEndpoint, WORD nClusterId, WORD nAttributeId)
{
	BYTE nDataSize;
	PZCLPACKAGE pPackage;
	PWORD pAttrId;
	BYTE nTransID = ZclGetTranSeq();
	nDataSize = sizeof(WORD) + ZCL_SHORT_HEADER;
	pPackage = (PZCLPACKAGE)malloc(nDataSize);
	pPackage->FrameControl.nDirection = 0;
	pPackage->FrameControl.nDisableDefaultRes = 0;
	pPackage->FrameControl.nFrameType = ZCL_FRAME_TYPE_PROFILE_CMD;
	pPackage->FrameControl.nManuSepcific = 0;
	pPackage->FrameControl.nReserved = 0;
	pPackage->ShortPackage.nCommandID = ZCL_CMD_READ;
	pPackage->ShortPackage.nTranSeqNumber = nTransID;
	pAttrId = (PWORD)pPackage->ShortPackage.pData;
	*pAttrId = nAttributeId;
	ZclAddMsgStatus(nNwkAddr, nTransID);
	AfDataReq(nNwkAddr, nEndpoint, ZnpGetDefaultEp(), ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING, (PBYTE)pPackage, nDataSize, TRUE);
//	/DeviceSetTimeoutTime(nNwkAddr, DEFAULT_MESSAGE_TIMEOUT);
	free((PBYTE)pPackage);
	return nTransID;
}

BYTE ZclReadAtrributeList(WORD nNwkAddr, BYTE nEndpoint, WORD nClusterId, PWORD pAttributeList, BYTE nNumberOfAttribute)
{
	BYTE nDataSize;
	PZCLPACKAGE pPackage;
	BYTE nTransID = ZclGetTranSeq();
	nDataSize = sizeof(WORD) * nNumberOfAttribute + ZCL_SHORT_HEADER;
	pPackage = (PZCLPACKAGE)malloc(nDataSize);
	pPackage->FrameControl.nDirection = 0;
	pPackage->FrameControl.nDisableDefaultRes = 0;
	pPackage->FrameControl.nFrameType = ZCL_FRAME_TYPE_PROFILE_CMD;
	pPackage->FrameControl.nManuSepcific = 0;
	pPackage->FrameControl.nReserved = 0;
	pPackage->ShortPackage.nCommandID = ZCL_CMD_READ;
	pPackage->ShortPackage.nTranSeqNumber = nTransID;
	CopyMemory(pPackage->ShortPackage.pData, (PBYTE)pAttributeList, sizeof(WORD) * nNumberOfAttribute);
	ZclAddMsgStatus(nNwkAddr, nTransID);
	AfDataReq(nNwkAddr, nEndpoint, ZnpGetDefaultEp(), ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING, (PBYTE)pPackage, nDataSize, TRUE);
	//DeviceSetTimeoutTime(nNwkAddr, DEFAULT_MESSAGE_TIMEOUT);
	free((PBYTE)pPackage);
	return nTransID;
}
