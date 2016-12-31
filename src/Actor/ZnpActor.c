/*
 * ZnpActor.c
 *
 *  Created on: Jun 7, 2016
 *      Author: ChauNM
 */
#include <string.h>
#include <jansson.h>
#include <pthread.h>
#include "actor.h"
#include "znp.h"
#include "DevicesManager.h"
#include "DeviceDesc.h"
#include "ZnpActor.h"
#include "universal.h"
#include "common/ActorParser.h"
#include "ZNP_SimpleAPI/Znp_SimpleApi.h"
#include "ZigbeeHaDeviceDesc.h"
#include "zcl.h"
#include "ZclSecurity/zcl_IAS_zone.h"
#include "ZclSecurity/zcl_IAS_ACE.h"

static PACTOR pZnpActor;
static pthread_t znpActorThread;

static void ZnpActorOnRequestAddDevice(PVOID pParam)
{
	char* message = (char*)pParam;
	char **znpSplitMessage;
	if (pZnpActor == NULL) return;
	json_t* payloadJson = NULL;
	json_t* paramsJson = NULL;
	json_t* durationJson = NULL;
	json_t* responseJson = NULL;
	json_t* statusJson = NULL;
	PACTORHEADER header;
	char* responseTopic;
	int duration = 0;
	char* responseMessage;
	BYTE nResult;
	znpSplitMessage = ActorSplitMessage(message);
	if (znpSplitMessage == NULL)
		return;
	// parse header to get origin of message
	header = ActorParseHeader(znpSplitMessage[0]);
	if (header == NULL)
	{
		ActorFreeSplitMessage(znpSplitMessage);
		return;
	}
	//parse payload
	payloadJson = json_loads(znpSplitMessage[1], JSON_DECODE_ANY, NULL);
	if (payloadJson == NULL)
	{
		ActorFreeSplitMessage(znpSplitMessage);
		ActorFreeHeaderStruct(header);
		return;
	}
	paramsJson = json_object_get(payloadJson, "params");
	if (paramsJson == NULL)
	{
		json_decref(payloadJson);
		ActorFreeSplitMessage(znpSplitMessage);
		ActorFreeHeaderStruct(header);
		return;
	}
	durationJson = json_object_get(paramsJson, "duration");
	if (durationJson == NULL)
	{
		json_decref(paramsJson);
		json_decref(payloadJson);
		ActorFreeSplitMessage(znpSplitMessage);
		ActorFreeHeaderStruct(header);
		return;
	}
	duration = json_integer_value(durationJson);
	json_decref(durationJson);
	json_decref(paramsJson);
	json_decref(payloadJson);
	// send add item command to znp
	nResult = ZnpZbPermitJoiningReq(0xFFFF, duration);
	//make response package
	responseJson = json_object();
	statusJson = json_object();
	json_t* requestJson = json_loads(znpSplitMessage[1], JSON_DECODE_ANY, NULL);
	json_object_set(responseJson, "request", requestJson);
	json_decref(requestJson);
	json_t* resultJson;
	if (nResult == 0)
	{
		resultJson = json_string("status.success");
	}
	else
	{
		resultJson = json_string("status.failure");
	}
	json_object_set(statusJson, "status", resultJson);
	json_decref(resultJson);
	json_object_set(responseJson, "response", statusJson);
	json_decref(statusJson);
	responseMessage = json_dumps(responseJson, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
	responseTopic = StrDup(header->origin);
	ActorFreeHeaderStruct(header);
	json_decref(responseJson);
	ActorFreeSplitMessage(znpSplitMessage);
//	ActorSend(pZnpActor, responseTopic, responseMessage, NULL, FALSE, responseTopic);
	ActorSend(pZnpActor, responseTopic, responseMessage, NULL, FALSE, "response");
	free(responseMessage);
	free(responseTopic);
}

static void ZnpActorOnRequestRemoveDevice(PVOID pParam)
{
	char* message = (char*)pParam;
	char **znpSplitMessage;
	if (pZnpActor == NULL) return;
	json_t* payloadJson = NULL;
	json_t* paramsJson = NULL;
	json_t* deviceIdJson = NULL;
	json_t* responseJson = NULL;
	json_t* statusJson = NULL;
	char* responseTopic;
	char* deviceId = 0;
	char* responseMessage;
	BYTE nResult;
	PACTORHEADER header;
	znpSplitMessage = ActorSplitMessage(message);
	if (znpSplitMessage == NULL)
		return;
	// parse header to get origin of message
	header = ActorParseHeader(znpSplitMessage[0]);
	if (header == NULL)
	{
		ActorFreeSplitMessage(znpSplitMessage);
		return;
	}
	//parse payload
	payloadJson = json_loads(znpSplitMessage[1], JSON_DECODE_ANY, NULL);
	if (payloadJson == NULL)
	{
		ActorFreeSplitMessage(znpSplitMessage);
		ActorFreeHeaderStruct(header);
		return;
	}
	paramsJson = json_object_get(payloadJson, "params");
	if (paramsJson == NULL)
	{
		json_decref(payloadJson);
		ActorFreeSplitMessage(znpSplitMessage);
		ActorFreeHeaderStruct(header);
		return;
	}
	deviceIdJson = json_object_get(paramsJson, "deviceId");
	if (deviceIdJson == NULL)
	{
		json_decref(paramsJson);
		json_decref(payloadJson);
		ActorFreeSplitMessage(znpSplitMessage);
		ActorFreeHeaderStruct(header);
		return;
	}
	deviceId = StrDup(json_string_value(deviceIdJson));
	json_decref(deviceIdJson);
	json_decref(paramsJson);
	json_decref(payloadJson);
	// assume that result always success
	nResult = 0;
	// do something here due to remove command
	printf("Device %s has been removed\n", deviceId);
	//make response package
	responseJson = json_object();
	statusJson = json_object();
	json_t* requestJson = json_loads(znpSplitMessage[1], JSON_DECODE_ANY, NULL);
	json_object_set(responseJson, "request", requestJson);
	json_decref(requestJson);
	json_t* resultJson;
	if (nResult == 0)
	{
		resultJson = json_string("status.success");
	}
	else
	{
		resultJson = json_string("status.failure");
	}
	json_object_set(statusJson, "status", resultJson);
	json_decref(resultJson);
	json_object_set(responseJson, "response", statusJson);
	json_decref(statusJson);
	responseMessage = json_dumps(responseJson, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
	json_decref(responseJson);
	ActorFreeSplitMessage(znpSplitMessage);
	//responseTopic = ActorMakeTopicName(NULL, header->origin, NULL);
	responseTopic = StrDup(header->origin);
	ActorFreeHeaderStruct(header);
//	ActorSend(pZnpActor, responseTopic, responseMessage, NULL, FALSE, responseTopic);
	ActorSend(pZnpActor, responseTopic, responseMessage, NULL, FALSE, "response");
	free(responseMessage);
	free(responseTopic);
}

static void ZnpActorCreat(char* guid, char* psw, char* host, WORD port)
{
	pZnpActor = ActorCreate(guid, psw, host, port);
	//Register callback to handle request package
	if (pZnpActor == NULL)
	{
		printf("Couldn't create actor\n");
		return;
	}
	char* actionTopic;
	actionTopic = ActorMakeTopicName("action/", guid, "/add_devices");
	ActorRegisterCallback(pZnpActor, actionTopic, ZnpActorOnRequestAddDevice, CALLBACK_RETAIN);
	free(actionTopic);
	actionTopic = ActorMakeTopicName("action/", guid, "/remove_device");
	ActorRegisterCallback(pZnpActor, actionTopic, ZnpActorOnRequestRemoveDevice, CALLBACK_RETAIN);
	free(actionTopic);
}

void ZnpActorPublishEndpointAddedEvent(IEEEADDRESS macId, BYTE endpoint, WORD deviceId, WORD deviceType)
{
	if (pZnpActor == NULL) return;
	json_t* eventJson = json_object();
	json_t* paramsJson = json_object();
	char* eventMessage;
	char* topicName;
	char* macIdString = IeeeToString(macId);
	json_t* macJson = json_string(macIdString);
	free(macIdString);
	json_object_set(paramsJson, "macId", macJson);
	json_decref(macJson);
	char* endpointString = malloc(20);
	sprintf(endpointString, "%d", endpoint);
	json_t* endpointJson = json_string(endpointString);
	free(endpointString);
	json_object_set(paramsJson, "endpoint", endpointJson);
	json_decref(endpointJson);
	json_t* deviceClassJson;
	deviceClassJson = DevDesMakeDeviceClassJson(deviceId, deviceType);
	json_object_set(paramsJson, "class", deviceClassJson);
	json_decref(deviceClassJson);
	json_t* protocolJson = json_string("zigbee");
	json_object_set(paramsJson, "protocol", protocolJson);
	json_decref(protocolJson);
	json_object_set(eventJson, "params", paramsJson);
	json_decref(paramsJson);
	eventMessage = json_dumps(eventJson, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
	json_decref(eventJson);
	//topicName = ActorMakeTopicName(pZnpActor->guid, "/:event/endpoint_added");
	topicName = ActorMakeTopicName("event/", pZnpActor->guid, "/endpoint_added");
	ActorSend(pZnpActor, topicName, eventMessage, NULL, FALSE, topicName);
	free(eventMessage);
	free(topicName);
}

void ZnpActorPublishDeviceAddedEvent(WORD nAddress)
{
	json_t* eventJson = json_object();
	json_t* paramsJson = json_object();
	json_t* macIdJson = NULL;
	json_t* protocolJson = NULL;
	json_t* endpointsJson = NULL;
	json_t* epJson = NULL;
	json_t* epIndexJson = NULL;
	json_t* deviceClassJson = NULL;
	char* epString = malloc(20);
	char* macIDString = NULL;
	char* eventMessage = NULL;
	char* topicName = NULL;
	PDEVICEINFO pDevice = DeviceFind(nAddress);
	BYTE epIndex;
	if (pDevice == NULL) return;
	macIDString = IeeeToString(pDevice->IeeeAddr);
	macIdJson = json_string(macIDString);
	json_object_set(paramsJson, "macId", macIdJson);
	json_decref(macIdJson);
	free(macIDString);
	protocolJson = json_string("zigbee");
	json_object_set(paramsJson, "protocol", protocolJson);
	json_decref(protocolJson);
	endpointsJson = json_array();
	for (epIndex = 0; epIndex < pDevice->nNumberActiveEndpoint; epIndex++)
	{
		epJson = json_object();
		sprintf(epString, "%d", pDevice->pEndpointInfoList[epIndex].nEndPoint);
		epIndexJson = json_string(epString);
		json_object_set(epJson, "endpoint", epIndexJson);
		json_decref(epIndexJson);
		deviceClassJson = DevDesMakeDeviceClassJson(pDevice->pEndpointInfoList[epIndex].nDeviceID,
				pDevice->pEndpointInfoList[epIndex].nDeviceType);
		json_object_set(epJson, "class", deviceClassJson);
		json_decref(deviceClassJson);
		json_array_append(endpointsJson, epJson);
		json_decref(epJson);
	}
	json_object_set(paramsJson, "endpoints", endpointsJson);
	json_decref(endpointsJson);
	json_object_set(eventJson, "params", paramsJson);
	json_decref(paramsJson);
	free(epString);
	eventMessage = json_dumps(eventJson, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
	json_decref(eventJson);
	//topicName = ActorMakeTopicName(pZnpActor->guid, "/:event/device_added");
	topicName = ActorMakeTopicName("event/", pZnpActor->guid, "/device_added");
	ActorSend(pZnpActor, topicName, eventMessage, NULL, FALSE, topicName);
	free(eventMessage);
	free(topicName);
}

void ZnpActorPublishDeviceRemovedEvent(IEEEADDRESS macId)
{
	if(pZnpActor == NULL) return;
	json_t* eventJson = json_object();
	json_t* paramsJson = json_object();
	char* macIdString = IeeeToString(macId);
	json_t* macIdJson = json_string(macIdString);
	free(macIdString);
	json_t* protocolJson = json_string("zigbee");
	json_object_set(paramsJson, "macId", macIdJson);
	json_object_set(paramsJson, "protocol", protocolJson);
	json_object_set(eventJson, "params", paramsJson);
	char* eventMessage = json_dumps(eventJson, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
	//char* topicName = ActorMakeTopicName(pZnpActor->guid, "/:event/device_removed");
	char* topicName = ActorMakeTopicName("event/", pZnpActor->guid, "/device_removed");
	ActorSend(pZnpActor, topicName, eventMessage, NULL, FALSE, topicName);
	json_decref(protocolJson);
	json_decref(macIdJson);
	json_decref(paramsJson);
	json_decref(eventJson);
	free(topicName);
	free(eventMessage);
}

void ZnpActorPublishDeviceOfflineEvent(IEEEADDRESS macId)
{
	if (pZnpActor == NULL) return;
	json_t* eventJson = json_object();
	json_t* paramsJson = json_object();
	char* macIdString = IeeeToString(macId);
	json_t* macIdJson = json_string(macIdString);
	free(macIdString);
	json_t* protocolJson = json_string("zigbee");
	json_object_set(paramsJson, "macId", macIdJson);
	json_object_set(paramsJson, "protocol", protocolJson);
	json_object_set(eventJson, "params", paramsJson);
	char* eventMessage = json_dumps(eventJson, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
	//char* topicName = ActorMakeTopicName(pZnpActor->guid, "/:event/device_offline");
	char* topicName = ActorMakeTopicName("event/", pZnpActor->guid, "/device_offline");
	ActorSend(pZnpActor, topicName, eventMessage, NULL, FALSE, topicName);
	json_decref(protocolJson);
	json_decref(macIdJson);
	json_decref(paramsJson);
	json_decref(eventJson);
	free(topicName);
	free(eventMessage);
}

void ZnpActorPublishDeviceOnlineEvent(IEEEADDRESS macId)
{
	if (pZnpActor == NULL) return;
	json_t* eventJson = json_object();
	json_t* paramsJson = json_object();
	char* macIdString = IeeeToString(macId);
	json_t* macIdJson = json_string(macIdString);
	free(macIdString);
	json_t* protocolJson = json_string("zigbee");
	json_object_set(paramsJson, "macId", macIdJson);
	json_object_set(paramsJson, "protocol", protocolJson);
	json_object_set(eventJson, "params", paramsJson);
	char* eventMessage = json_dumps(eventJson, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
	//char* topicName = ActorMakeTopicName(pZnpActor->guid, "/:event/device_online");
	char* topicName = ActorMakeTopicName("event/", pZnpActor->guid, "/device_online");
	ActorSend(pZnpActor, topicName, eventMessage, NULL, FALSE, topicName);
	json_decref(protocolJson);
	json_decref(macIdJson);
	json_decref(paramsJson);
	json_decref(eventJson);
	free(topicName);
	free(eventMessage);
}

void ZnpActorPublishDeviceErrorEvent(IEEEADDRESS macId, WORD error)
{
	if (pZnpActor == NULL) return;
	json_t* eventJson = json_object();
	json_t* paramsJson = json_object();
	char* macIdString = IeeeToString(macId);
	json_t* macIdJson = json_string(macIdString);
	free(macIdString);
	json_t* protocolJson = json_string("zigbee");
	char* errorMessage = malloc(50);
	sprintf(errorMessage, "%s%d","error.actor.", error);
	json_t* errorJson = json_string(errorMessage);
	free(errorMessage);
	json_object_set(paramsJson, "macId", macIdJson);
	json_object_set(paramsJson, "protocol", protocolJson);
	json_object_set(paramsJson, "error", errorJson);
	json_object_set(eventJson, "params", paramsJson);
	char* eventMessage = json_dumps(eventJson, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
	//char* topicName = ActorMakeTopicName(pZnpActor->guid, "/:event/device_error");
	char* topicName = ActorMakeTopicName("event/", pZnpActor->guid, "/device_error");
	ActorSend(pZnpActor, topicName, eventMessage, NULL, FALSE, topicName);
	json_decref(errorJson);
	json_decref(protocolJson);
	json_decref(macIdJson);
	json_decref(paramsJson);
	json_decref(eventJson);
	free(topicName);
	free(eventMessage);
}

void ZnpActorPublishDeviceLqi(WORD nAddress, BYTE nLqi)
{
	PDEVICEINFO pDevice;
	if (pZnpActor == NULL) return;
	pDevice = DeviceFind(nAddress);
	if (pDevice == NULL) return;
	json_t* eventJson = json_object();
	json_t* paramsJson = json_object();
	char* macIdString = IeeeToString(pDevice->IeeeAddr);
	json_t* macIdJson = json_string(macIdString);
	json_t* lqiJson = NULL;
	free(macIdString);
	json_t* protocolJson = json_string("zigbee");
	if (nLqi > LQI_LIMIT)
		lqiJson = json_integer(1);
	else
		lqiJson = json_integer(0);
	json_object_set(paramsJson, "macId", macIdJson);
	json_object_set(paramsJson, "protocol", protocolJson);
	json_object_set(paramsJson, "signal_strength", lqiJson);
	json_object_set(eventJson, "params", paramsJson);
	char* eventMessage = json_dumps(eventJson, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
	//char* topicName = ActorMakeTopicName(pZnpActor->guid, "/:event/device_signal");
	char* topicName = ActorMakeTopicName("event/", pZnpActor->guid, "/device_signal");
	ActorSend(pZnpActor, topicName, eventMessage, NULL, FALSE, topicName);
	json_decref(lqiJson);
	json_decref(protocolJson);
	json_decref(macIdJson);
	json_decref(paramsJson);
	json_decref(eventJson);
	free(topicName);
	free(eventMessage);
}

void ZnpActorPublishZnpStatus(char* status)
{
	if (pZnpActor == NULL) return;
	json_t* eventJson = json_object();
	//json_t* paramsJson = json_object();
	json_t* statusJson = json_string(status);
	json_object_set(eventJson, "status", statusJson);
	//json_object_set(eventJson, "params", paramsJson);
	char* eventMessage = json_dumps(eventJson, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
	//char* topicName = ActorMakeTopicName(pZnpActor->guid, "/:event/status");
	char* topicName = ActorMakeTopicName("event/", "service/world", "/manifest");
	ActorSend(pZnpActor, topicName, eventMessage, NULL, FALSE, topicName);
	json_decref(statusJson);
	//json_decref(paramsJson);
	json_decref(eventJson);
	free(topicName);
	free(eventMessage);
}

PZNPACTORDATA ZnpActorMakeData(char* dataName, BYTE nDataType, void* data, BYTE dataLen)
{
	if (dataName == NULL)
		return NULL;
	PZNPACTORDATA pData = malloc(sizeof(ZNPACTORDATA));
	pData->dataName = StrDup(dataName);
	pData->nDataType = nDataType;
	switch (nDataType)
	{
	case ZNP_DATA_TYPE_STRING:
		pData->value = (void*)StrDup((char*)data);
		break;
	case ZNP_DATA_TYPE_INTERGER:
		pData->value = malloc(sizeof(int));
		memset(pData->value, 0, sizeof(int));
		CopyMemory(pData->value, data, dataLen);
		break;
	case ZNP_DATA_TYPE_FLOAT:
		pData->value = malloc(sizeof(double));
		memset(pData->value, 0, sizeof(double));
		CopyMemory(pData->value, data, dataLen);
		break;
	default:
		free(pData->dataName);
		free(pData);
		return NULL;
	}
	return pData;
}

void ZnpActorDestroyZnpData(PZNPACTORDATA pData)
{
	if (pData != NULL)
	{
		if (pData->dataName != NULL) free(pData->dataName);
		if (pData->value != NULL) free(pData->value);
		free(pData);
	}
}

void ZnpActorPublishDeviceDataEvent(IEEEADDRESS macId, BYTE endPoint, BYTE nDataCount, PZNPACTORDATA *pData)
{
	if (pZnpActor == NULL) return;
	json_t* eventJson = json_object();
	json_t* paramsJson = json_object();
	char* macIdString = IeeeToString(macId);
	json_t* macIdJson = json_string(macIdString);
	free(macIdString);
	char* endpointString = malloc(20);
	sprintf(endpointString, "%d", endPoint);
	json_t* endpointJson = json_string(endpointString);
	free(endpointString);
	json_t* protocolJson = json_string("zigbee");
	json_object_set(paramsJson, "macId", macIdJson);
	json_object_set(paramsJson, "endpoint", endpointJson);
	json_object_set(paramsJson, "protocol", protocolJson);
	json_decref(macIdJson);
	json_decref(endpointJson);
	json_decref(protocolJson);
	json_t* dataJson;
	json_t* valueJson;
	dataJson = json_object();
	BYTE nIndex = 0;
	while (nDataCount > 0)
	{
		switch (pData[nIndex]->nDataType)
		{
		case ZNP_DATA_TYPE_INTERGER:
			valueJson = json_integer((int)(*((int*)(pData[nIndex]->value))));
			break;
		case ZNP_DATA_TYPE_FLOAT:
			valueJson = json_real((double)(*((double*)(pData[nIndex]->value))));
			break;
		case ZNP_DATA_TYPE_STRING:
			valueJson = json_string(pData[nIndex]->value);
			break;
		default:
			json_decref(dataJson);
			json_decref(paramsJson);
			json_decref(eventJson);
			return;
			break;
		}
		json_object_set(dataJson, pData[nIndex]->dataName, valueJson);
		json_decref(valueJson);
		nDataCount--;
		nIndex++;
	}
	json_object_set(paramsJson, "data", dataJson);
	json_decref(dataJson);
	json_object_set(eventJson, "params", paramsJson);
	json_decref(paramsJson);
	char* eventMessage = json_dumps(eventJson, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
	json_decref(eventJson);
	//char* topicName = ActorMakeTopicName(pZnpActor->guid, "/:event/device_data");
	char* topicName = ActorMakeTopicName("event/", pZnpActor->guid, "/device_data");
	ActorSend(pZnpActor, topicName, eventMessage, NULL, FALSE, topicName);
	free(topicName);
	free(eventMessage);
}

void ZnpActorProcess(PACTOROPTION option)
{
	mosquitto_lib_init();
	ZnpActorCreat(option->guid, option->psw, option->host, option->port);
	if (pZnpActor == NULL)
	{
		mosquitto_lib_cleanup();
		return;
	}
	while(1)
	{
		ActorProcessEvent(pZnpActor);
		mosquitto_loop(pZnpActor->client, 0, 1);
		usleep(10000);
	}
	mosquitto_disconnect(pZnpActor->client);
	mosquitto_destroy(pZnpActor->client);
	mosquitto_lib_cleanup();
}

void ZnpActorStart(PACTOROPTION option)
{
	pthread_create(&znpActorThread, NULL, (void*)&ZnpActorProcess, (void*)option);
	pthread_detach(znpActorThread);
}
