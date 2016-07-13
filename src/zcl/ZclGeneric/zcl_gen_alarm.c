/*
 * zcl_gen_alarm.c
 *
 *  Created on: Jun 14, 2016
 *      Author: ChauNM
 */

#include "zcl.h"
#include "DevicesManager.h"
#include "log.h"
#include "ZNP_AF/ZnpAf.h"
#include "ZnpActor.h"
#include "../fluent-logger/fluent-logger.h"
VOID ZclGenAlarmAttrInit(PZCLGENALARMATTR pData)
{
	pData->wAlarmCount = 0;
}

BYTE ZclGenAlarmGetAttr(WORD nDstAddr, BYTE nEp)
{
	BYTE nDataSize;
	PZCLPACKAGE pPackage;
	PWORD pAttrId;
	BYTE nTransID = ZclGetTranSeq();
	nDataSize = sizeof(WORD) * ZCL_GEN_ALARM_ATTR_COUNT + ZCL_SHORT_HEADER;
	pPackage = (PZCLPACKAGE)malloc(nDataSize);
	pPackage->FrameControl.nDirection = 0;
	pPackage->FrameControl.nDisableDefaultRes = 0;
	pPackage->FrameControl.nFrameType = ZCL_FRAME_TYPE_PROFILE_CMD;
	pPackage->FrameControl.nManuSepcific = 0;
	pPackage->FrameControl.nReserved = 0;
	pPackage->ShortPackage.nCommandID = ZCL_CMD_READ;
	pPackage->ShortPackage.nTranSeqNumber = nTransID;
	pAttrId = (PWORD)pPackage->ShortPackage.pData;
	pAttrId[0] = ZCL_GEN_ALARM_COUNT_ATTR;
	ZclAddMsgStatus(nDstAddr, nTransID);
	AfDataReq(nDstAddr, nEp, ZnpGetDefaultEp(), ZCL_CLUSTER_ID_GEN_ALARMS, (PBYTE)pPackage, nDataSize, TRUE);
	free((PBYTE)pPackage);
	return nTransID;
}

VOID ZclGenAlarmUpdateReadAttr(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen)
{
	PCLUSTERINFO pCluster = DeviceFindClusterInfo(nNwkAddr, nEp, ZCL_CLUSTER_ID_GEN_ALARMS);
	if (pCluster == NULL)
		return;
	PZCLGENALARMATTR pClusterAttr = (PZCLGENALARMATTR)pCluster->pData;
	if (pClusterAttr == NULL)
		return;
	PZCLATTRREADSTRUCT pReadAttr = (PZCLATTRREADSTRUCT)pData;
	WORD nAttrDataLength;
	while (nLen > 0)
	{
		if (pReadAttr->nState != 0)
		{
			nLen -= sizeof(pReadAttr->nAttrID) + sizeof(pReadAttr->nState);
			FLUENT_LOGGER_DEBUG("zcl general alarm attribute is not valid");
			printf("Attribute 0x%04X not valid\n", pReadAttr->nAttrID);

		}
		else
		{
			switch (pReadAttr->nAttrID)
			{
			case ZCL_GEN_PWR_CFG_MAIN_VOLT_ATTR:
				pClusterAttr->wAlarmCount = (WORD)(*pReadAttr->pData);
				//printf("Alarm count %d\n", pClusterAttr->wAlarmCount);
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			default:
				nAttrDataLength = ZclGetDataLength(pReadAttr->nDataType, pReadAttr->pData);
				if (nAttrDataLength == 0xFFFF)
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
}

BYTE ZclGenAlarmConfig(WORD nDstAddr, BYTE nEp)
{
	// make and
	PCLUSTERINFO pClusterInfo = DeviceFindClusterInfo(nDstAddr, nEp, ZCL_CLUSTER_ID_GEN_ALARMS);
	if ( pClusterInfo == NULL)
		return 0;
	// request to bind
	ZnpZdoBindReq(nDstAddr, nEp, ZCL_CLUSTER_ID_GEN_ALARMS);
	DeviceWaitInformed(nDstAddr, ZDO_BIND_RSP, 0xFF);
	return 0xFF;
}

VOID ZclAlarmParseAlarmCommand(WORD nNwkAddr, BYTE nEndpoint, PZCLGENALARMALRMCMD pAlarmInfo)
{
	PDEVICEINFO pDevice = DeviceFind(nNwkAddr);
	if (pDevice == NULL) return;
	PZNPACTORDATA znpData;
	char value = 0;
	char* loggerMessage;
	switch (pAlarmInfo->nClusterID)
	{
	case ZCL_CLUSTER_ID_GEN_POWER_CFG:
		switch(pAlarmInfo->nAlarmCode)
		{
		case 0x10:
			loggerMessage = malloc(300);
			sprintf(loggerMessage, "battery low: Device 0x%04X endpoint %d", nNwkAddr, nEndpoint);
			FLUENT_LOGGER_WARN(loggerMessage);
			free(loggerMessage);
			//printf("battery low alarm\n");
			value = 1;
			znpData = ZnpActorMakeData("battery", ZNP_DATA_TYPE_INTERGER, (void*)&value, sizeof(value));
			ZnpActorPublishDeviceDataEvent(pDevice->IeeeAddr, nEndpoint, 1, &znpData);
			ZnpActorDestroyZnpData(znpData);
			break;
		case 0x00:
			loggerMessage = malloc(300);
			sprintf(loggerMessage, "main power low: Device 0x%04X endpoint %d", nNwkAddr, nEndpoint);
			FLUENT_LOGGER_WARN(loggerMessage);
			free(loggerMessage);
			//printf("Main power low alarm\n");
			break;
		case 0x01:
			loggerMessage = malloc(300);
			sprintf(loggerMessage, "main power high: Device 0x%04X endpoint %d", nNwkAddr, nEndpoint);
			FLUENT_LOGGER_WARN(loggerMessage);
			free(loggerMessage);
			//printf("Main power high alarm\n");
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

VOID ZclGenAlarmParseSpecificPackage(WORD nNwkAddr, BYTE nEndpoint, BYTE nCommandId, PBYTE pZclData, BYTE nDataSize)
{
	switch (nCommandId)
	{
	case ZCL_GEN_ALARM_ALARM_CMD:
		ZclAlarmParseAlarmCommand(nNwkAddr, nEndpoint , (PZCLGENALARMALRMCMD)pZclData);
		break;
	default:
		break;
	}
}
