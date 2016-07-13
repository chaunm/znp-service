/*
 * zcl_gen_onoff.c
 *
 *  Created on: Mar 15, 2016
 *      Author: ChauNM
 */

#include "zcl.h"
#include "DevicesManager.h"
#include "ZNP_AF/ZnpAf.h"
#include "log.h"
#include "ZnpActor.h"
#include "../fluent-logger/fluent-logger.h"
/********************************/
/* ZCL GEN ON OFF PROCESS 		*/
/********************************/
VOID ZclGenOnOffAttrInit(PZCLGENONOFFATTR pData)
{
	pData->nOnOff = 0;
	pData->nWriteOnOff = 0;
}

BYTE ZclGenOnOffGetAttr(WORD nDstAddr, BYTE nEp)
{
	BYTE nDataSize;
	PZCLPACKAGE pPackage;
	PWORD pAttrId;
	BYTE nTransID = ZclGetTranSeq();
	nDataSize = sizeof(WORD) * ZCL_GEN_ONOFF_ATTR_COUNT + ZCL_SHORT_HEADER;
	pPackage = (PZCLPACKAGE)malloc(nDataSize);
	pPackage->FrameControl.nDirection = 0;
	pPackage->FrameControl.nDisableDefaultRes = 0;
	pPackage->FrameControl.nFrameType = ZCL_FRAME_TYPE_PROFILE_CMD;
	pPackage->FrameControl.nManuSepcific = 0;
	pPackage->FrameControl.nReserved = 0;
	pPackage->ShortPackage.nCommandID = ZCL_CMD_READ;
	pPackage->ShortPackage.nTranSeqNumber = nTransID;
	pAttrId = (PWORD)pPackage->ShortPackage.pData;
	pAttrId[0] = ZCL_GEN_ONOFF_ONOFF_ATTR;
	ZclAddMsgStatus(nDstAddr, nTransID);
	AfDataReq(nDstAddr, nEp, ZnpGetDefaultEp(), ZCL_CLUSTER_ID_GEN_ON_OFF, (PBYTE)pPackage, nDataSize, TRUE);
	//DeviceSetTimeoutTime(nDstAddr, DEFAULT_MESSAGE_TIMEOUT);
	free((PBYTE)pPackage);
	return nTransID;
}

VOID ZclGenOnOffUpdateReadAttr(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen)
{
	PCLUSTERINFO pCluster = DeviceFindClusterInfo(nNwkAddr, nEp, ZCL_CLUSTER_ID_GEN_ON_OFF);
	if (pCluster == NULL)
		return;
	PZCLGENONOFFATTR pClusterAttr = (PZCLGENONOFFATTR)pCluster->pData;
	if (pClusterAttr == NULL)
		return;
	PZCLATTRREADSTRUCT pReadAttr = (PZCLATTRREADSTRUCT)pData;
	WORD nAttrDataLength;
	PZNPACTORDATA znpData;
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
			case ZCL_GEN_ONOFF_ONOFF_ATTR:
				pClusterAttr->nOnOff = (BOOL)(*pReadAttr->pData);
				pClusterAttr->nWriteOnOff = pClusterAttr->nOnOff;
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(BOOL);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(BOOL);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				int znpValue = pClusterAttr->nOnOff;
				znpData = ZnpActorMakeData("onoff", ZNP_DATA_TYPE_INTERGER, &znpValue, sizeof(int));
				ZnpActorPublishDeviceDataEvent(DeviceFind(nNwkAddr)->IeeeAddr, nEp, 1, &znpData);
				ZnpActorDestroyZnpData(znpData);
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

VOID ZclGenOnOffParseWriteAttrRsp(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLength)
{
	PCLUSTERINFO pClusterInfo = DeviceFindClusterInfo(nNwkAddr, nEp, ZCL_CLUSTER_ID_GEN_ON_OFF);
	if (pClusterInfo == NULL)
		return;
	PZCLGENONOFFATTR pAttrData = (PZCLGENONOFFATTR)pClusterInfo->pData;
	if (pAttrData == NULL)
		return;
	PZCLATTRWRITERSPSTRUCT pWriteRsp = (PZCLATTRWRITERSPSTRUCT)pData;
	if (nLength == 1)
	{
		printf("Attributes write success\n");
		pAttrData->nOnOff = pAttrData->nWriteOnOff;
		nLength = 0;
	}
	while (nLength > 0)
	{
		printf( "Write attribute 0x%04x, device 0x%04X, endpoint %d", pWriteRsp->nAttrId, nNwkAddr, nEp);
		if (pWriteRsp->nStatus == 0)
		{
			if (pWriteRsp->nAttrId == ZCL_IAS_ZONE_SETTING_ATTR)
				pAttrData->nOnOff = pAttrData->nWriteOnOff;
			printf("success\n");
		}
		else
		{
			if (pWriteRsp->nAttrId == ZCL_IAS_ZONE_SETTING_ATTR)
				pAttrData->nWriteOnOff = pAttrData->nOnOff;
			printf("fail\n");
		}
		nLength -= sizeof(ZCLATTRWRITERSPSTRUCT);
		pWriteRsp++;
	}
}

VOID ZclGenOnOffParseAttrReport(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen)
{
	PCLUSTERINFO pZclInfo = DeviceFindClusterInfo(nNwkAddr, nEp, ZCL_CLUSTER_ID_GEN_ON_OFF);
	PZCLATTRREPORTSTRUCT pReportData = (PZCLATTRREPORTSTRUCT)pData;
	WORD nAttrDataLength;
	if (pZclInfo == NULL)
		return;
	PZCLGENONOFFATTR pClusterAttr = (PZCLGENONOFFATTR)pZclInfo->pData;
	if (pZclInfo->pData == NULL)
		return;
	PZNPACTORDATA znpData;
	while (nLen > 0)
	{
		switch (pReportData->nAttrID)
		{
		case ZCL_GEN_ONOFF_ONOFF_ATTR:
			pClusterAttr->nOnOff = pReportData->pData[0];
			pClusterAttr->nWriteOnOff = pClusterAttr->nOnOff;
			nLen -= sizeof(ZCLATTRREPORTSTRUCT) + sizeof(BOOL);
			pData += sizeof(ZCLATTRREPORTSTRUCT) + sizeof(BOOL);
			pReportData = (PZCLATTRREPORTSTRUCT)pData;
			char* pLogString = (char*)malloc(200);
			sprintf(pLogString, "Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: 0x%02X",
					nNwkAddr, nEp, ZCL_CLUSTER_ID_GEN_ON_OFF, ZCL_GEN_ONOFF_ONOFF_ATTR, pClusterAttr->nOnOff);
			LogWrite(pLogString);
			FLUENT_LOGGER_INFO(pLogString);
			//MqttClientPublishMessage(pLogString);
			//printf("Attribute Report - Address: 0x%04X, Endpoint: 0x%02X, Cluster ID: 0x%04X, Attr ID: 0x%04X, Value: 0x%02X\n",
							//nNwkAddr, nEp, ZCL_CLUSTER_ID_GEN_ON_OFF, ZCL_GEN_ONOFF_ONOFF_ATTR, pClusterAttr->nOnOff);
			int znpValue = pClusterAttr->nOnOff;
			znpData = ZnpActorMakeData("occupancy", ZNP_DATA_TYPE_INTERGER, &znpValue, sizeof(int));
			ZnpActorPublishDeviceDataEvent(DeviceFind(nNwkAddr)->IeeeAddr, nEp, 1, &znpData);
			ZnpActorDestroyZnpData(znpData);
			free(pLogString);
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

BYTE ZclGenOnOffConfig(WORD nDstAddr, BYTE nEp)
{
	PZCLPACKAGE pPackage;
	BYTE nDataSize;
	BYTE nTransID = ZclGetTranSeq();
	// make and
	PCLUSTERINFO pClusterInfo = DeviceFindClusterInfo(nDstAddr, nEp, ZCL_CLUSTER_ID_GEN_ON_OFF);
	if ( pClusterInfo == NULL)
		return 0;
	// make config report package
	nDataSize = sizeof(ZCLSRVRPTCFG) + ZCL_SHORT_HEADER + sizeof(BYTE);
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
	pRptConfig->nAttrId = ZCL_GEN_ONOFF_ONOFF_ATTR;
	pRptConfig->nDataType = ZCL_DATATYPE_BOOLEAN;
	pRptConfig->nMinReport = 60;
	pRptConfig->nMaxReport = 120;
	pRptConfig->pData[0] = 1;
	ZclAddMsgStatus(nDstAddr, nTransID);
	AfDataReq(nDstAddr, nEp, ZnpGetDefaultEp(), ZCL_CLUSTER_ID_GEN_ON_OFF, (PBYTE)pPackage, nDataSize, TRUE);
	//DeviceSetTimeoutTime(nDstAddr, DEFAULT_MESSAGE_TIMEOUT);
	free(pPackage);
	return nTransID;
}

BYTE ZclGenOnOffSendCommand(WORD nDstAddr, BYTE nEp, BYTE nCommand)
{
	BYTE nTransID = ZclGetTranSeq();
	PZCLPACKAGE pPackage = (PZCLPACKAGE)malloc(ZCL_SHORT_HEADER);
	pPackage->FrameControl.nDirection = 0;
	pPackage->FrameControl.nDisableDefaultRes = 0;
	pPackage->FrameControl.nFrameType = ZCL_FRAME_TYPE_SPECIFIC_CMD;
	pPackage->FrameControl.nManuSepcific = 0;
	pPackage->FrameControl.nReserved = 0;
	pPackage->ShortPackage.nCommandID = nCommand;
	pPackage->ShortPackage.nTranSeqNumber = nTransID;
	ZclAddMsgStatus(nDstAddr, nTransID);
	AfDataReq(nDstAddr, nEp, ZnpGetDefaultEp(), ZCL_CLUSTER_ID_GEN_ON_OFF, (PBYTE)pPackage, ZCL_SHORT_HEADER, FALSE);
	free(pPackage);
	return nTransID;
}
