/*
 * zcl_IAS_zone.c
 *
 *  Created on: Mar 5, 2016
 *      Author: ChauNM
 */

#include "znp.h"
#include "ZnpActor.h"
#include "DevicesManager.h"
#include "ZNP_AF/ZnpAf.h"
#include "ZclSecurity/zcl_IAS_zone.h"
#include "zcl.h"
#include "universal.h"
#include "log.h"
#include "../fluent-logger/fluent-logger.h"

/********************************/
/* ZCL IAS ZONE CLUSTER PROCESS */
/********************************/
VOID ZclIasZoneAttrInit(PZCLIASZONEATTR pData)
{
	pData->nZoneState = 1;
	pData->nZoneSetting = 0x0807060504030201;
	pData->nWriteZoneSetting = pData->nZoneSetting;
	pData->nZoneType = 0x0504;
	pData->nZoneStatus = 0x0706;
}

BYTE ZclIasZoneGetAttr(WORD nDstAddr, BYTE nEp)
{
	BYTE nDataSize;
	PZCLPACKAGE pPackage;
	PWORD pAttrId;
	BYTE nTransID = ZclGetTranSeq();
	nDataSize = sizeof(WORD) * ZCL_IAS_ZONE_ATTR_COUNT + ZCL_SHORT_HEADER;
	pPackage = (PZCLPACKAGE)malloc(nDataSize);
	pPackage->FrameControl.nDirection = 0;
	pPackage->FrameControl.nDisableDefaultRes = 0;
	pPackage->FrameControl.nFrameType = ZCL_FRAME_TYPE_PROFILE_CMD;
	pPackage->FrameControl.nManuSepcific = 0;
	pPackage->FrameControl.nReserved = 0;
	pPackage->ShortPackage.nCommandID = ZCL_CMD_READ;
	pPackage->ShortPackage.nTranSeqNumber = nTransID;
	pAttrId = (PWORD)pPackage->ShortPackage.pData;
	pAttrId[0] = ZCL_IAS_ZONE_STATE_ATTR;
	pAttrId[1] = ZCL_IAS_ZONE_TYPE_ATTR;
	pAttrId[2] = ZCL_IAS_ZONE_STATUS_ATTR;
	pAttrId[3] = ZCL_IAS_ZONE_SETTING_ATTR;
	ZclAddMsgStatus(nDstAddr, nTransID);
	AfDataReq(nDstAddr, nEp, ZnpGetDefaultEp(), ZCL_CLUSTER_ID_SS_IAS_ZONE, (PBYTE)pPackage, nDataSize, TRUE);
	//DeviceSetTimeoutTime(nDstAddr, DEFAULT_MESSAGE_TIMEOUT);
	free((PBYTE)pPackage);
	return nTransID;
}

VOID ZclIasZoneUpdateReadAttr(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen)
{
	PCLUSTERINFO pCluster = DeviceFindClusterInfo(nNwkAddr, nEp, ZCL_CLUSTER_ID_SS_IAS_ZONE);
	if (pCluster == NULL)
		return;
	PZCLIASZONEATTR pClusterAttr = (PZCLIASZONEATTR)pCluster->pData;
	if (pClusterAttr == NULL)
		return;
	PZCLATTRREADSTRUCT pReadAttr = (PZCLATTRREADSTRUCT)pData;
	WORD nAttrDataLength;
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
			case ZCL_IAS_ZONE_SETTING_ATTR:
				CopyMemory((PBYTE)&(pClusterAttr->nZoneSetting), (pReadAttr->pData), sizeof(IEEEADDRESS));
				PWORD pIeeeAddress = (PWORD)(&pClusterAttr->nZoneSetting);
				printf("ICE IEEE Address: 0x%04x%04x%04x%04x \n", pIeeeAddress[3], pIeeeAddress[2], pIeeeAddress[1], pIeeeAddress[0]);
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(IEEEADDRESS);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(IEEEADDRESS);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_IAS_ZONE_STATE_ATTR:
				pClusterAttr->nZoneState = pReadAttr->pData[0];
				printf("Zone state: 0x%02X\n", pClusterAttr->nZoneState);
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(BYTE);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				break;
			case ZCL_IAS_ZONE_TYPE_ATTR:
				pClusterAttr->nZoneType = *((PWORD)(pReadAttr->pData));
				printf("Zone type: 0x%04X\n", pClusterAttr->nZoneType);
				nLen -= sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pData += sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD);
				pReadAttr = (PZCLATTRREADSTRUCT)pData;
				/* Comment get Endpoint device type
				PENDPOINTINFO pEpInfo = DeviceFindEpInfo(nNwkAddr, nEp);
				if (pEpInfo != NULL)
					if (pEpInfo->nDeviceID == 0x0402)
						pEpInfo->nDeviceType = pClusterAttr->nZoneType;
				*/
				break;
			case ZCL_IAS_ZONE_STATUS_ATTR:
				pClusterAttr->nZoneStatus = *((PWORD)(pReadAttr->pData));
				printf("Zone status: 0x%04X\n", pClusterAttr->nZoneStatus);
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
}

VOID ZclIasZoneParseWriteAttrRsp(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLength)
{
	PCLUSTERINFO pClusterInfo = DeviceFindClusterInfo(nNwkAddr, nEp, ZCL_CLUSTER_ID_SS_IAS_ZONE);
	if (pClusterInfo == NULL)
		return;
	PZCLIASZONEATTR pAttrData = (PZCLIASZONEATTR)pClusterInfo->pData;
	if (pAttrData == NULL)
		return;
	PZCLATTRWRITERSPSTRUCT pWriteRsp = (PZCLATTRWRITERSPSTRUCT)pData;
	if (nLength == 1)
	{
		printf("Attributes write success\n");
		pAttrData->nZoneSetting = pAttrData->nWriteZoneSetting;
		nLength = 0;
	}
	while (nLength > 0)
	{
		printf("Write attribute 0x%04X ", pWriteRsp->nAttrId);
		if (pWriteRsp->nStatus == 0)
		{
			if(pWriteRsp->nAttrId == ZCL_IAS_ZONE_SETTING_ATTR)
				pAttrData->nZoneSetting = pAttrData->nWriteZoneSetting;
			printf("success\n");
		}
		else
		{
			if(pWriteRsp->nAttrId == ZCL_IAS_ZONE_SETTING_ATTR)
				pAttrData->nWriteZoneSetting = pAttrData->nZoneSetting;
			printf("fail\n");
		}
		nLength -= sizeof(ZCLATTRWRITERSPSTRUCT);
		pWriteRsp++;
	}
}

VOID ZclIasZoneParseAttrReport(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen)
{
	PCLUSTERINFO pZclInfo = DeviceFindClusterInfo(nNwkAddr, nEp, ZCL_CLUSTER_ID_SS_IAS_ZONE);
	PZCLATTRREPORTSTRUCT pReportData = (PZCLATTRREPORTSTRUCT)pData;
	WORD nAttrDataLength;
	if (pZclInfo == NULL)
		return;
	PZCLIASZONEATTR pClusterAttr = (PZCLIASZONEATTR)pZclInfo->pData;
	if (pZclInfo->pData == NULL)
		return;

	while (nLen > 0)
	{
		switch (pReportData->nAttrID)
		{
		case ZCL_IAS_ZONE_SETTING_ATTR:
			CopyMemory((PBYTE)&(pClusterAttr->nZoneSetting), (pReportData->pData), sizeof(IEEEADDRESS));
			PWORD pIeeeAddress = (PWORD)(&pClusterAttr->nZoneSetting);
			printf("ICE IEEE Address: 0x%04x%04x%04x%04x \n", pIeeeAddress[3], pIeeeAddress[2], pIeeeAddress[1], pIeeeAddress[0]);
			nLen -= sizeof(ZCLATTRREPORTSTRUCT) + sizeof(IEEEADDRESS);
			pData += sizeof(ZCLATTRREPORTSTRUCT) + sizeof(IEEEADDRESS);
			pReportData = (PZCLATTRREPORTSTRUCT)pData;
			break;
		case ZCL_IAS_ZONE_STATE_ATTR:
			pClusterAttr->nZoneState = pReportData->pData[0];
			printf("Zone state: 0x%02X\n", pClusterAttr->nZoneState);
			nLen -= sizeof(ZCLATTRREPORTSTRUCT) + sizeof(BYTE);
			pData += sizeof(ZCLATTRREPORTSTRUCT) + sizeof(BYTE);
			pReportData = (PZCLATTRREPORTSTRUCT)pData;
			break;
		case ZCL_IAS_ZONE_TYPE_ATTR:
			pClusterAttr->nZoneType = *((PWORD)(pReportData->pData));
			printf("Zone type: 0x%04X\n", pClusterAttr->nZoneType);
			nLen -= sizeof(ZCLATTRREPORTSTRUCT) + sizeof(WORD);
			pData += sizeof(ZCLATTRREPORTSTRUCT) + sizeof(WORD);
			pReportData = (PZCLATTRREPORTSTRUCT)pData;
			break;
		case ZCL_IAS_ZONE_STATUS_ATTR:
			pClusterAttr->nZoneStatus = *((PWORD)(pReportData->pData));
			printf("Zone status: 0x%04X\n", pClusterAttr->nZoneStatus);
			nLen -= sizeof(ZCLATTRREPORTSTRUCT) + sizeof(WORD);
			pData += sizeof(ZCLATTRREPORTSTRUCT) + sizeof(WORD);
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

BYTE ZclIasZoneConfig(WORD nDstAddr, BYTE nEp)
{
	PIEEEADDRESS pIceIeee;
	PZCLPACKAGE pPackage;
	BYTE nDataSize;
	PZCLATTRWRITESTRUCT pZoneSetting;
	BYTE nTransID = ZclGetTranSeq();
	PCLUSTERINFO pClusterInfo = DeviceFindClusterInfo(nDstAddr, nEp, ZCL_CLUSTER_ID_SS_IAS_ZONE);
	if ( pClusterInfo == NULL)
		return 0;
	// Enroll process
	pIceIeee = &((PZCLIASZONEATTR)pClusterInfo->pData)->nZoneSetting;
	if (*pIceIeee == ZnpGetIeeeAddr())
		return 0;
	nDataSize = sizeof(ZCLATTRWRITESTRUCT) + sizeof(IEEEADDRESS) + ZCL_SHORT_HEADER;
	pPackage = (PZCLPACKAGE)malloc(nDataSize);
	pPackage->FrameControl.nDirection = 0;
	pPackage->FrameControl.nDisableDefaultRes = 0;
	pPackage->FrameControl.nFrameType = ZCL_FRAME_TYPE_PROFILE_CMD;
	pPackage->FrameControl.nManuSepcific = 0;
	pPackage->FrameControl.nReserved = 0;
	pPackage->ShortPackage.nCommandID = ZCL_CMD_WRITE;
	pPackage->ShortPackage.nTranSeqNumber = nTransID;
	pZoneSetting = (PZCLATTRWRITESTRUCT)pPackage->ShortPackage.pData;
	pZoneSetting->nAttrID = ZCL_IAS_ZONE_SETTING_ATTR;
	pZoneSetting->nDataType = ZCL_DATATYPE_IEEE_ADDR;
	*((PIEEEADDRESS)pZoneSetting->pData) = ZnpGetIeeeAddr();
	ZclAddMsgStatus(nDstAddr, nTransID);
	AfDataReq(nDstAddr, nEp, ZnpGetDefaultEp(), ZCL_CLUSTER_ID_SS_IAS_ZONE, (PBYTE)pPackage, nDataSize, TRUE);
	//DeviceSetTimeoutTime(nDstAddr, DEFAULT_MESSAGE_TIMEOUT);
	free(pPackage);
	((PZCLIASZONEATTR)pClusterInfo->pData)->nWriteZoneSetting = ZnpGetIeeeAddr();
	return nTransID;
}

BYTE ZclIasZoneAcceptEnroll(WORD nNwkAddr, BYTE nEndpoint)
{
	PZCLPACKAGE pPackage;
	PZCLIASZONEENROLLRSP pZoneEnrollRsp;
	BYTE nTransID = ZclGetTranSeq();
	BYTE nDataSize = sizeof(ZCLIASZONEENROLLRSP) + ZCL_SHORT_HEADER;
	pPackage = (PZCLPACKAGE)malloc(nDataSize);
	pPackage->FrameControl.nDirection = 0;
	pPackage->FrameControl.nDisableDefaultRes = 0;
	pPackage->FrameControl.nFrameType = ZCL_FRAME_TYPE_SPECIFIC_CMD;
	pPackage->FrameControl.nManuSepcific = 0;
	pPackage->FrameControl.nReserved = 0;
	pPackage->ShortPackage.nCommandID = ZCL_IAS_ZONE_ENROLL_RSP;
	pPackage->ShortPackage.nTranSeqNumber = nTransID;
	pZoneEnrollRsp = (PZCLIASZONEENROLLRSP)pPackage->ShortPackage.pData;
	pZoneEnrollRsp->nRspCode = ZCL_IAS_ZONE_ENROLL_SUCCESS;
	// trong ung dung that co the phai tinh toan Zone ID de dua vao
	pZoneEnrollRsp->nZoneId = ZCL_SECU_DEFAULT_ZONE;
	// Ham Af chua while nen luong xu ly khong duoc goi ham Af
	// Dang gap loi nay do viec accept dang duoc goi tu luong xu ly
	AfDataReq(nNwkAddr, nEndpoint, ZnpGetDefaultEp(), ZCL_CLUSTER_ID_SS_IAS_ZONE, (PBYTE)pPackage, nDataSize, FALSE);
	free(pPackage);
	return nTransID;
}

VOID ZclIasZoneParseZoneStatusChange(WORD nNwkAddr, BYTE nEndpoint, PZCLIASZONESTATUSCHG pStatus)
{
	PZCLATTRREADSTRUCT pReadStruct = (PZCLATTRREADSTRUCT)malloc(sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD));
	PCLUSTERINFO pCluster = DeviceFindClusterInfo(nNwkAddr, nEndpoint, ZCL_CLUSTER_ID_SS_IAS_ZONE);
	PDEVICEINFO deviceInfo = DeviceFind(nNwkAddr);
	PZNPACTORDATA pZnpData[3] = {NULL};
	BYTE value;
	char* LogString;
	if (pCluster == NULL)
		return;
	if ((pStatus->nZoneStatus & 0x0008) != 0) // check for battery update
	{
		value = 1;
		printf("Battery low\n");
		LogString = (char*)malloc(255);
		sprintf(LogString, "Battery low on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
		FLUENT_LOGGER_WARN(LogString);
		LogWrite(LogString);
		free(LogString);
	}
	else
	{
		value = 0;
	}
	pZnpData[0] = ZnpActorMakeData("battery", ZNP_DATA_TYPE_INTERGER, (void*)&value, sizeof(value));
	switch (((PZCLIASZONEATTR)pCluster->pData)->nZoneType)
	{
	case ZCL_IAS_ZONE_STD_CIE:
		if ((pStatus->nZoneStatus & 0x0001) != 0)
		{
			printf("System alarm activated\n");
			LogWrite("System alarm activated");
			value = 1;
		}
		else
		{
			printf("System alarm deactivated\n");
			LogWrite("System alarm deactivated");
			value = 0;
		}
		pZnpData[1] = ZnpActorMakeData("alarm", ZNP_DATA_TYPE_INTERGER, (void*)&value, sizeof(value));
		break;
	case ZCL_IAS_ZONE_MOTION_SENSOR:
		if ((pStatus->nZoneStatus & 0x0001) != 0)
		{
			printf("Motion activated\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Motion activated on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		else
		{
			printf("Motion deactivated\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Motion deactivated on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		if ((pStatus->nZoneStatus & 0x0002) != 0)
		{
			printf("Human detected\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Human detected on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		else
		{
			printf("Human undetected\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Human undetected on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		if ((pStatus->nZoneStatus & 0x0003) != 0)
			value = 1;
		else
			value = 0;
		pZnpData[1] = ZnpActorMakeData("motion", ZNP_DATA_TYPE_INTERGER, (void*)&value, sizeof(value));
		break;
	case ZCL_IAS_ZONE_CONTACT_SWITCH:
		if ((pStatus->nZoneStatus & 0x0001) != 0)
		{
			printf("Door 1 opened\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Door 1 opened on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		else
		{
			printf("Door 1 closed\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Door 1 closed on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		if ((pStatus->nZoneStatus & 0x0002) != 0)
		{
			printf("Door 2 opened\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Door 2 opened on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		else
		{
			printf("Door 2 closed\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Door 2 closed on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		if ((pStatus->nZoneStatus & 0x0003) != 0)
			value = 1;
		else
			value = 0;
		pZnpData[1] = ZnpActorMakeData("open", ZNP_DATA_TYPE_INTERGER, (void*)&value, sizeof(value));
		break;
	case ZCL_IAS_ZONE_FIRE_SENSOR:
		if ((pStatus->nZoneStatus & 0x0001) != 0)
		{
			value = 1;
			printf("Fire detected\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "fire detected on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		else
		{
			value = 0;
			printf("Fire undetected\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Fire undetected on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		pZnpData[1] = ZnpActorMakeData("fire", ZNP_DATA_TYPE_INTERGER, (void*)&value, sizeof(value));
		break;
	case ZCL_IAS_ZONE_WATER_SENSOR:
		if ((pStatus->nZoneStatus & 0x0001) != 0)
		{
			value = 1;
			printf("Water leakage detected\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Water leakage detected on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		else
		{
			value = 0;
			printf("Water leakage undetected\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Water leakage undetected on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		pZnpData[1] = ZnpActorMakeData("water", ZNP_DATA_TYPE_INTERGER, (void*)&value, sizeof(value));
		break;
	case ZCL_IAS_ZONE_GAS_SENSOR:
		if ((pStatus->nZoneStatus & 0x0001) != 0)
		{
			value = 1;
			printf("CO detected\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "CO detected on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		else
		{
			value = 0;
			printf("CO undetected\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "CO undetected on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		pZnpData[1] = ZnpActorMakeData("carbon monoxide", ZNP_DATA_TYPE_INTERGER, (void*)&value, sizeof(value));
		if ((pStatus->nZoneStatus & 0x0002) != 0)
		{
			value = 1;
			printf("Gas leakage detected\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Gas leakage detected on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		else
		{
			value = 0;
			printf("Gas leakage undetected\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Gas leakage undetected on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		pZnpData[2] = ZnpActorMakeData("gas", ZNP_DATA_TYPE_INTERGER, (void*)&value, sizeof(value));
		break;
	case ZCL_IAS_ZONE_PERSONAL_EMER:
		if ((pStatus->nZoneStatus & 0x0001) != 0)
		{
			printf("Device fall detected\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Device fall detected on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		else
		{
			printf("Device fall undetected\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Device fall undetected on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		if ((pStatus->nZoneStatus & 0x0002) != 0)
		{
			printf("Emergency button detected\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Emergency button detected on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		else
		{
			printf("Emergency button undetected\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Emergency button undetected on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		if ((pStatus->nZoneStatus & 0x0003) != 0)
			value = 1;
		else
			value = 0;
		pZnpData[1] = ZnpActorMakeData("emergency", ZNP_DATA_TYPE_INTERGER, (void*)&value, sizeof(value));
		break;
	case ZCL_IAS_ZONE_VIBRATION_SENSOR:
		if ((pStatus->nZoneStatus & 0x0001) != 0)
		{
			printf("Device movement detected\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Device movement detected on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		else
		{
			printf("Device movement undetected\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Device movement undetected on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		if ((pStatus->nZoneStatus & 0x0002) != 0)
		{
			printf("Device vibration detected\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Device vibration detected on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		else
		{
			printf("Device vibration undetected\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Device vibration undetected on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		if ((pStatus->nZoneStatus & 0x0003) != 0)
			value = 1;
		else
			value = 0;
		pZnpData[1] = ZnpActorMakeData("vibration", ZNP_DATA_TYPE_INTERGER, (void*)&value, sizeof(value));
		break;
	case ZCL_IAS_ZONE_KEY_FOB:
		if ((pStatus->nZoneStatus & 0x0001) != 0)
		{
			value = 1;
			printf("Key pressed\n");
			LogString = (char*)malloc(255);
			sprintf(LogString, "Key pressed on device 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			LogWrite(LogString);
			free(LogString);
		}
		else
			value = 0;
		pZnpData[1] = ZnpActorMakeData("keyfob", ZNP_DATA_TYPE_INTERGER, (void*)&value, sizeof(value));
		break;
	default:
		break;
	}
	pReadStruct->nAttrID = ZCL_IAS_ZONE_STATUS_ATTR;
	pReadStruct->nDataType = ZCL_DATATYPE_BITMAP16;
	pReadStruct->nState = 0;
	*((PWORD)pReadStruct->pData) = pStatus->nZoneStatus;
	ZclIasZoneUpdateReadAttr(nNwkAddr, nEndpoint, (PBYTE)pReadStruct, sizeof(ZCLATTRREADSTRUCT) + sizeof(WORD));
	//publish data to topic
	deviceInfo = DeviceFind(nNwkAddr);
	if (deviceInfo == NULL)
		return;
	if (pZnpData[2] != NULL)
	{
		ZnpActorPublishDeviceDataEvent(deviceInfo->IeeeAddr, nEndpoint, 3, pZnpData);
		ZnpActorDestroyZnpData(pZnpData[0]);
		ZnpActorDestroyZnpData(pZnpData[1]);
		ZnpActorDestroyZnpData(pZnpData[2]);
	}
	else
	{
		ZnpActorPublishDeviceDataEvent(deviceInfo->IeeeAddr, nEndpoint, 2, pZnpData);
		ZnpActorDestroyZnpData(pZnpData[0]);
		ZnpActorDestroyZnpData(pZnpData[1]);
	}
	free(pReadStruct);
}

VOID ZclIasZoneParseSpecificPackage(WORD nNwkAddr, BYTE nEndpoint, BYTE nCommandId, PBYTE pZclData, BYTE nDataSize)
{
	switch (nCommandId)
	{
	char* LogString;
	case ZCL_IAS_ZONE_ENROLL_RQT:
		printf("Ias Zone device enroll request:\n");
		printf("Zone type: 0x%04X\n", ((PZCLIASZONEENROLLRQT)pZclData)->nZoneType);
		printf("Manufacturer code: 0x%04X\n", ((PZCLIASZONEENROLLRQT)pZclData)->nManuCode);
		LogString = (char*)malloc(200);
		sprintf(LogString, "IAS Zone Enrolled request. Address: 0x%04X - Endpoint - 0x%02X:",
				nNwkAddr, nEndpoint);
		LogWrite(LogString);
		FLUENT_LOGGER_INFO(LogString);
		free(LogString);
		printf("Accept enroll\n");
		ZclIasZoneAcceptEnroll(nNwkAddr, nEndpoint);
		break;
	case ZCL_IAS_ZONE_STATUS_CHNG:
		printf("Ias Zone status change - Device: 0x%04X - Endpoint: 0x%02X - ClusterID 0x%04X - Status: 0x%04X\n",
				nNwkAddr, nEndpoint, ZCL_CLUSTER_ID_SS_IAS_ZONE, (((PZCLIASZONESTATUSCHG)pZclData)->nZoneStatus));
		ZclIasZoneParseZoneStatusChange(nNwkAddr, nEndpoint, (PZCLIASZONESTATUSCHG)pZclData);
		LogString = (char*)malloc(200);
		sprintf(LogString, "Ias Zone status change - Device: 0x%04X - Endpoint: 0x%02X - ClusterID 0x%04X - Status: 0x%04X",
				nNwkAddr, nEndpoint, ZCL_CLUSTER_ID_SS_IAS_ZONE, (((PZCLIASZONESTATUSCHG)pZclData)->nZoneStatus));
		LogWrite(LogString);
		FLUENT_LOGGER_INFO(LogString);
		free(LogString);
		break;
	default:
		break;
	}
}
