/*
 * zcl_gen_powerconfig.c
 *
 *  Created on: Mar 15, 2016
 *      Author: ChauNM
 */

#include <stdio.h>
#include <string.h>
#include "zcl.h"
#include "DevicesManager.h"
#include "log.h"
#include "ZNP_AF/ZnpAf.h"
#include "ZnpActor.h"
#include "../fluent-logger/fluent-logger.h"

void ZclGenPwrCfgAttrInit(PZCLPWRCFGATTR pData)
{
	memset((void*)pData, 0, sizeof(ZCLPWRCFGATTR));
	/*
	pData->wMainVoltage = 0;
	pData->byMainFreq = 0;
	pData->byMainAlarm = 0;
	pData->wMainMinThres = 0;
	pData->wMainMaxThres = 0;
	pData->wMainTripPoint = 0;
	*/
}

BYTE ZclGenPwrCfgGetAttr(WORD nDstAddr, BYTE nEp)
{
	BYTE nDataSize;
	PZCLPACKAGE pPackage;
	PWORD pAttrId;
	BYTE nTransID = ZclGetTranSeq();
	nDataSize = sizeof(WORD) * ZCL_GEN_PWR_CFG_ATTR_CNT + ZCL_SHORT_HEADER;
	pPackage = (PZCLPACKAGE)malloc(nDataSize);
	pPackage->FrameControl.nDirection = 0;
	pPackage->FrameControl.nDisableDefaultRes = 0;
	pPackage->FrameControl.nFrameType = ZCL_FRAME_TYPE_PROFILE_CMD;
	pPackage->FrameControl.nManuSepcific = 0;
	pPackage->FrameControl.nReserved = 0;
	pPackage->ShortPackage.nCommandID = ZCL_CMD_READ;
	pPackage->ShortPackage.nTranSeqNumber = nTransID;
	pAttrId = (PWORD)pPackage->ShortPackage.pData;
	pAttrId[0] = ZCL_GEN_PWR_CFG_MAIN_VOLT_ATTR;
	pAttrId[1] = ZCL_GEN_PWR_CFG_MAIN_FREQ_ATTR;
	pAttrId[2] = ZCL_GEN_PWR_CFG_MAIN_ALARM_MASK_ATTR;
	pAttrId[3] = ZCL_GEN_PWR_CFG_MAIN_VOLT_MIN_THRES_ATTR;
	pAttrId[4] = ZCL_GEN_PWR_CFG_MAIN_VOLT_MAX_THRES_ATTR;
	pAttrId[5] = ZCL_GEN_PWR_CFG_MAIN_VOLT_TRIP_ATTR;
	pAttrId[6] = ZCL_GEN_PWR_CFG_BAT_VOLT_ATTR;
	pAttrId[7] = ZCL_GEN_PWR_CFG_BAT_MANU_ATTR;
	pAttrId[8] = ZCL_GEN_PWR_CFG_BAT_SIZE_ATTR;
	pAttrId[9] = ZCL_GEN_PWR_CFG_BAT_AH_ATTR;
	pAttrId[10] = ZCL_GEN_PWR_CFG_BAT_QUANTITY_ATTR;
	pAttrId[11] = ZCL_GEN_PWR_CFG_BAT_RATE_VOLT_ATTR;
	pAttrId[12] = ZCL_GEN_PWR_CFG_BAT_ALARM_MASK_ATTR;
	pAttrId[13] = ZCL_GEN_PWR_CFG_BAT_LOW_VOLT_THRES_ATTR;
	FLUENT_LOGGER_INFO("Read attribute(s) of gen_pwr_config cluster");
	ZclAddMsgStatus(nDstAddr, nTransID);
	AfDataReq(nDstAddr, nEp, ZnpGetDefaultEp(), ZCL_CLUSTER_ID_GEN_POWER_CFG, (PBYTE)pPackage, nDataSize, TRUE);
	free((PBYTE)pPackage);
	return nTransID;
}

VOID ZclGenPwrCfgUpdateReadAttr(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen)
{
	PCLUSTERINFO pCluster = DeviceFindClusterInfo(nNwkAddr, nEp, ZCL_CLUSTER_ID_GEN_POWER_CFG);
	if (pCluster == NULL)
		return;
	PZCLPWRCFGATTR pClusterAttr = (PZCLPWRCFGATTR)pCluster->pData;
	if (pClusterAttr == NULL)
		return;
	PZCLATTRREADSTRUCT pReadAttr = (PZCLATTRREADSTRUCT)pData;
	WORD nAttrDataLength;
	while (nLen > 0)
	{
		if (pReadAttr->nState != 0)
		{
			nLen -= sizeof(pReadAttr->nAttrID) + sizeof(pReadAttr->nState);
			printf("Attribute 0x%04X not valid\n", pReadAttr->nAttrID);
		}
		else
		{
			switch (pReadAttr->nAttrID)
			{
			case ZCL_GEN_PWR_CFG_MAIN_VOLT_ATTR:
				pClusterAttr->wMainVoltage = (WORD)(*pReadAttr->pData);
				printf("Main voltage: %d\n", pClusterAttr->wMainVoltage);
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_GEN_PWR_CFG_MAIN_FREQ_ATTR:
				pClusterAttr->byMainFreq = (BYTE)(*pReadAttr->pData);
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_GEN_PWR_CFG_MAIN_ALARM_MASK_ATTR:
				pClusterAttr->byMainAlarm = (BYTE)(*pReadAttr->pData);
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_GEN_PWR_CFG_MAIN_VOLT_MIN_THRES_ATTR:
				pClusterAttr->wMainMinThres = (WORD)(*pReadAttr->pData);
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_GEN_PWR_CFG_MAIN_VOLT_MAX_THRES_ATTR:
				pClusterAttr->wMainMaxThres = (WORD)(*pReadAttr->pData);
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_GEN_PWR_CFG_MAIN_VOLT_TRIP_ATTR:
				pClusterAttr->wMainTripPoint = (WORD)(*pReadAttr->pData);
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_GEN_PWR_CFG_BAT_VOLT_ATTR:
				pClusterAttr->byBatVoltage = (BYTE)(*pReadAttr->pData);
				printf("Battery voltage: 0x%02X\n", pClusterAttr->byBatVoltage);
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_GEN_PWR_CFG_BAT_MANU_ATTR:
				pClusterAttr->byBatManu = (BYTE)(*pReadAttr->pData);
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_GEN_PWR_CFG_BAT_SIZE_ATTR:
				pClusterAttr->byBatSize = (BYTE)(*pReadAttr->pData);
				printf("Battery size: 0x%02X\n", pClusterAttr->byBatSize);
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_GEN_PWR_CFG_BAT_AH_ATTR:
				pClusterAttr->wBatAhRating = (WORD)(*pReadAttr->pData);
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_GEN_PWR_CFG_BAT_QUANTITY_ATTR:
				pClusterAttr->byBatQuantity = (BYTE)(*pReadAttr->pData);
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_GEN_PWR_CFG_BAT_RATE_VOLT_ATTR:
				pClusterAttr->byBatRatedVoltage = (BYTE)(*pReadAttr->pData);
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_GEN_PWR_CFG_BAT_ALARM_MASK_ATTR:
				pClusterAttr->byBatAlarm = (BYTE)(*pReadAttr->pData);
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_GEN_PWR_CFG_BAT_LOW_VOLT_THRES_ATTR:
				pClusterAttr->byBatLowThresHold = (BYTE)(*pReadAttr->pData);
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
}

VOID ZclGenPwrCfgParseWriteAttrRsp(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLength)
{
	PCLUSTERINFO pClusterInfo = DeviceFindClusterInfo(nNwkAddr, nEp, ZCL_CLUSTER_ID_GEN_POWER_CFG);
	if (pClusterInfo == NULL)
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
		{
			printf("success\n");
		}
		else
		{
			printf("fail\n");
		}
		break;
		nLength -= sizeof(ZCLATTRWRITERSPSTRUCT);
		pWriteRsp++;
	}
}

BYTE ZclGenPwrCfgConfig(WORD nDstAddr, BYTE nEp)
{
	PZCLPACKAGE pPackage;
	BYTE nDataSize;
	BYTE nTransID = ZclGetTranSeq();
	PZCLATTRWRITESTRUCT pWriteData;
	// make and
	PCLUSTERINFO pClusterInfo = DeviceFindClusterInfo(nDstAddr, nEp, ZCL_CLUSTER_ID_GEN_POWER_CFG);
	if ( pClusterInfo == NULL)
		return 0;
	PZCLPWRCFGATTR pAttrData = (PZCLPWRCFGATTR)pClusterInfo->pData;
	if (pAttrData->byBatSize == 0)
		return 0xFF; // no battery use
	// write battery alamr mask and battery limit
	nDataSize = 2 * (sizeof(ZCLATTRWRITESTRUCT) + sizeof(BYTE)) + ZCL_SHORT_HEADER;
	pPackage = (PZCLPACKAGE)malloc(nDataSize);
	pPackage->FrameControl.nDirection = 0;
	pPackage->FrameControl.nDisableDefaultRes = 0;
	pPackage->FrameControl.nFrameType = ZCL_FRAME_TYPE_PROFILE_CMD;
	pPackage->FrameControl.nManuSepcific = 0;
	pPackage->FrameControl.nReserved = 0;
	pPackage->ShortPackage.nCommandID = ZCL_CMD_WRITE;
	pPackage->ShortPackage.nTranSeqNumber = nTransID;

	pWriteData = (PZCLATTRWRITESTRUCT)pPackage->ShortPackage.pData;
	pWriteData->nAttrID = ZCL_GEN_PWR_CFG_BAT_ALARM_MASK_ATTR;
	pWriteData->nDataType = ZCL_DATATYPE_BITMAP8;
	*((PBYTE)(pWriteData->pData)) = 0x01;

	pWriteData = (PZCLATTRWRITESTRUCT)(pPackage->ShortPackage.pData + sizeof(ZCLATTRWRITESTRUCT) + sizeof(BYTE));
	pWriteData->nAttrID = ZCL_GEN_PWR_CFG_BAT_LOW_VOLT_THRES_ATTR;
	pWriteData->nDataType = ZCL_DATATYPE_UINT8;
	*((PBYTE)(pWriteData->pData)) = BATTERY_LOW_LIMIT;

	ZclAddMsgStatus(nDstAddr, nTransID);
	AfDataReq(nDstAddr, nEp, ZnpGetDefaultEp(), ZCL_CLUSTER_ID_GEN_POWER_CFG, (PBYTE)pPackage, nDataSize, TRUE);
	free(pPackage);
	return nTransID;
}
