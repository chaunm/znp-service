/*
 * zcl_IAS_ACE.c
 *
 *  Created on: Apr 21, 2016
 *      Author: ChauNM
 */
#include "zcl.h"
#include "zcl_IAS_ACE.h"
#include "DevicesManager.h"
#include "ZNP_ZDO/Znp_Zdo.h"
#include "log.h"
#include "ZnpActor.h"
#include "../fluent-logger/fluent-logger.h"
VOID ZclIasAceParseSpecificPackage(WORD nNwkAddr, BYTE nEndpoint, BYTE nCommandId,PBYTE pZclData, BYTE nDataSize)
{
	PZCLIASACEARMNOTI pArmNotification = (PZCLIASACEARMNOTI)pZclData;
	char* LogString;
	PZNPACTORDATA znpData;
	PDEVICEINFO deviceInfo = DeviceFind(nNwkAddr);
	if (deviceInfo == NULL) return;
	switch (nCommandId)
	{
	case ZCL_IAS_ACE_ARM_RESPONSE:
		switch (pArmNotification->nArmNotification)
		{
		case ALL_ZONE_DISARMED:
			znpData = ZnpActorMakeData("remote", ZNP_DATA_TYPE_STRING, (void*)"disarm", strlen("disarm"));
			ZnpActorPublishDeviceDataEvent(deviceInfo->IeeeAddr, nEndpoint, 1, &znpData);
			ZnpActorDestroyZnpData(znpData);
			LogString = (char*)malloc(255);
			printf("All Zone disarmed received from 0x%04X endpoint 0x%02X\n", nNwkAddr, nEndpoint);
			sprintf(LogString, "All Zone disarmed received from 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			FLUENT_LOGGER_INFO(LogString);
			LogWrite(LogString);
			free(LogString);
			break;
		case ONLY_DAY_ZONE_ARMED:
			znpData = ZnpActorMakeData("remote", ZNP_DATA_TYPE_STRING, (void*)"indoor", strlen("indoor"));
			ZnpActorPublishDeviceDataEvent(deviceInfo->IeeeAddr, nEndpoint, 1, &znpData);
			ZnpActorDestroyZnpData(znpData);
			LogString = (char*)malloc(255);
			printf("Only Day Zone armed received from 0x%04X endpoint 0x%02X\n", nNwkAddr, nEndpoint);
			sprintf(LogString, "Only Day Zone armed received from 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			FLUENT_LOGGER_INFO(LogString);
			LogWrite(LogString);
			free(LogString);
			break;
		case ONLY_NIGHT_ZONE_ARMED:
			znpData = ZnpActorMakeData("remote", ZNP_DATA_TYPE_STRING, (void*)"surpress", strlen("surpress"));
			ZnpActorPublishDeviceDataEvent(deviceInfo->IeeeAddr, nEndpoint, 1, &znpData);
			ZnpActorDestroyZnpData(znpData);
			LogString = (char*)malloc(255);
			printf("Only Night Zone armed received from 0x%04X endpoint 0x%02X\n", nNwkAddr, nEndpoint);
			sprintf(LogString, "Only Night Zone armed received from 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			FLUENT_LOGGER_INFO(LogString);
			LogWrite(LogString);
			free(LogString);
			break;
		case ALL_ZONE_ARMED:
			znpData = ZnpActorMakeData("remote", ZNP_DATA_TYPE_STRING, (void*)"arm", strlen("arm"));
			ZnpActorPublishDeviceDataEvent(deviceInfo->IeeeAddr, nEndpoint, 1, &znpData);
			ZnpActorDestroyZnpData(znpData);
			LogString = (char*)malloc(255);
			printf("All Zone armed received from 0x%04X endpoint 0x%02X\n", nNwkAddr, nEndpoint);
			sprintf(LogString, "All Zone armed received from 0x%04X endpoint 0x%02X", nNwkAddr, nEndpoint);
			FLUENT_LOGGER_INFO(LogString);
			LogWrite(LogString);
			free(LogString);
			break;
		default:
			break;
		}
		break;
	case ZCL_IAS_ACE_GET_ZONE_ID_MAP_RESPONSE:
		printf("Get zone ID response\n");
		FLUENT_LOGGER_INFO("Get zone ID response");
		break;
	case ZCL_IAS_ACE_GET_ZONE_INFO_RESPONSE:
		printf("Get zone info response\n");
		FLUENT_LOGGER_INFO("Get zone info response");
		break;
	default:
		break;
	}
}

BYTE ZclIasAceConfig(WORD nDstAddr, BYTE nEp)
{
	// make and
	PCLUSTERINFO pClusterInfo = DeviceFindClusterInfo(nDstAddr, nEp, ZCL_CLUSTER_ID_SS_IAS_ACE);
	if ( pClusterInfo == NULL)
		return 0;
	// request to bind
	ZnpZdoBindReq(nDstAddr, nEp, ZCL_CLUSTER_ID_SS_IAS_ACE);
	DeviceWaitInformed(nDstAddr, ZDO_BIND_RSP, 0xFF);
	return 0xFF;
}
