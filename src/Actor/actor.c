/*
 * actor.c
 *
 *  Created on: May 26, 2016
 *      Author: ChauNM
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "jansson.h"
#include <mosquitto.h>
#include <uuid/uuid.h>
#include "actor.h"
#include "universal.h"
#include "common/ActorParser.h"
#include "unistd.h"

int ActorConnect(PACTOR pACtor, char* guid, char* psw, char* inPost, WORD inPort);
void ActorOnMessage(struct mosquitto* client, void* context, const struct mosquitto_message* message);
void ActorOnOffline(struct mosquitto* client, void * context, int cause);
void ActorOnConnect(struct mosquitto* client, void* context, int result);
void ActorOnDelivered(struct mosquitto* client, void* context, int dt);

static void ActorOnRequestStop(PVOID pParam)
{
	PACTOR actor = (PACTOR)pParam;
	json_t* responseJson = json_object();
	json_t* statusJson = json_string("status.offline");
	json_object_set(responseJson, "status", statusJson);
	char* responseMessage = json_dumps(responseJson, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
	ActorSend(actor, "event/service/world/manifest", responseMessage, NULL, FALSE, NULL);
	free(responseMessage);
	json_decref(statusJson);
	json_decref(responseJson);
	sleep(5);
	exit(EXIT_SUCCESS);
}

char* ActorMakeGuid(char* prefix)
{
	char* znpGuid;
	char* uuid = ActorCreateUuidString();
	znpGuid = malloc(strlen(uuid) + strlen(prefix) + 1);
	memset(znpGuid, 0, strlen(uuid) + strlen("ZNP-") + 2);
	sprintf(znpGuid, "%s-%s", prefix, uuid);
	return znpGuid;
}

char* ActorMakeTopicName(const char* messageType, const char* guid, char* topic)
{
	char* topicName = malloc(strlen(messageType) + strlen(guid) + strlen(topic) + 1);
	memset(topicName, 0, strlen(messageType) + strlen(guid) + strlen(topic) + 1);
	sprintf(topicName, "%s%s%s", messageType, guid, topic);
	return topicName;
}

void ActorRegisterCallback(PACTOR pActor, const char* event, ACTORCALLBACKFN callback, char callbackType)
{
	if ((callback == NULL) || (event == NULL)) return;
	PACTORCALLBACK pCallback = (PACTORCALLBACK)malloc(sizeof(ACTORCALLBACK));
	PACTORCALLBACK pProcessingCallback = pActor->pActorCallback;
	pCallback->event = StrDup(event);
	pCallback->callbackFn = callback;
	pCallback->callbackType = callbackType;
	pCallback->nextCallback = NULL;
	if (pActor->pActorCallback == NULL)
		pActor->pActorCallback = pCallback;
	else
	{
		while(pProcessingCallback->nextCallback != NULL)
			pProcessingCallback = pProcessingCallback->nextCallback;
		pProcessingCallback->nextCallback = pCallback;
	}
}

void ActorEmitEvent(PACTOR pActor,const char* event, void* pParam)
{
	PACTOREVENT pEvent = (PACTOREVENT)malloc(sizeof(ACTOREVENT));
	PACTOREVENT pProcessingEvent = pActor->pEvent;
	if (event == NULL)
	{
		free(pEvent);
		return;
	}
	pEvent->event = StrDup(event);
	pEvent->callbackParam = pParam;
	pEvent->NextEvent = NULL;
	if (pActor->pEvent == NULL)
	{
		pActor->pEvent = pEvent;
	}
	else
	{
		while (pProcessingEvent->NextEvent != NULL)
			pProcessingEvent = pProcessingEvent->NextEvent;
		pProcessingEvent->NextEvent = pEvent;
	}
}

void ActorProcessEvent(PACTOR pActor)
{
	PACTOREVENT pProcessingEvent;
	PACTORCALLBACK pProcessingCallback;
	PACTORCALLBACK pSearchCallback;
	// process for awaiting event in actor
	while (pActor->pEvent != NULL)
	{
		pProcessingEvent = pActor->pEvent;
		//Search for callback to server event
		pProcessingCallback = pActor->pActorCallback;
		while (pProcessingCallback != NULL)
		{
			if (strcmp(pProcessingCallback->event, pProcessingEvent->event) == 0)
			{
				pProcessingCallback->callbackFn(pProcessingEvent->callbackParam);
				// check if callback need to be deleted
				if (pProcessingCallback->callbackType == CALLBACK_ONCE)
				{
					if (pActor->pActorCallback == pProcessingCallback)
					{
						pActor->pActorCallback = pProcessingCallback->nextCallback;
						free(pProcessingCallback->event);
						free(pProcessingCallback);
					}
					else
					{
						pSearchCallback = pActor->pActorCallback;
						while (pSearchCallback->nextCallback != pProcessingCallback)
							pSearchCallback = pSearchCallback->nextCallback;
						pSearchCallback->nextCallback = pProcessingCallback->nextCallback;
						free(pProcessingCallback->event);
						free(pProcessingCallback);
					}
				}
				break;
			}
			pProcessingCallback = pProcessingCallback->nextCallback;
		}
		pActor->pEvent = pProcessingEvent->NextEvent;
		// free event memory
		free(pProcessingEvent->event);
		if (pProcessingEvent->callbackParam != NULL)
			free(pProcessingEvent->callbackParam);
		free(pProcessingEvent);
	}
}

PACTOR ActorCreate(char* guid, char* psw, char* host, WORD port)
{
	if ((guid == NULL))
		return NULL;
	PACTOR pActor = (PACTOR)malloc(sizeof(ACTOR));
	memset(pActor, 0, sizeof(ACTOR));
	pActor->guid = StrDup(guid);
	pActor->psw = StrDup(psw);
	if (host != NULL)
		pActor->host = StrDup(host);
	else
		pActor->host = StrDup(HOST);
	if (port != 0)
		pActor->port = port;
	else
		pActor->port = PORT;
	pActor->connected = FALSE;
	while (pActor->client == NULL)
	{
		ActorConnect(pActor, pActor->guid, pActor->psw, pActor->host, pActor->port);
		sleep(5);
	}
	pActor->pEvent = NULL;
	pActor->pActorCallback = NULL;

	if (pActor->client != NULL)
	{
		return pActor;
	}
	else
	{
		ActorDelete(pActor);
		return NULL;
	}

}

void ActorDelete(PACTOR pActor)
{

	PACTOREVENT pDeleteEvent = pActor->pEvent;
	PACTORCALLBACK pDeleteCallback = pActor-> pActorCallback;

	mosquitto_disconnect(pActor->client);
	mosquitto_destroy(pActor->client);
	while (pActor->pEvent != NULL)
	{
		pDeleteEvent = pActor->pEvent;
		pActor->pEvent = pDeleteEvent->NextEvent;
		free(pDeleteEvent->event);
		free(pDeleteEvent->callbackParam);
		free(pDeleteEvent);
	}
	while(pActor->pActorCallback != NULL)
	{
		pDeleteCallback = pActor->pActorCallback;
		pActor->pActorCallback = pDeleteCallback->nextCallback;
		free(pDeleteCallback->event);
		free(pDeleteCallback);
	}
	free(pActor->guid);
	free(pActor->psw);
	free(pActor);
}

int ActorConnect(PACTOR pActor, char* guid, char* psw, char* inHost, WORD inPort)
{

    int rc;
    struct mosquitto* client;
    WORD port;
    char* host;
    if (inPort != 0)
    	port = inPort;
    else
    	port = PORT;
    if (inHost != NULL)
    	host = StrDup(inHost);
    else
    	host = StrDup(HOST);
    if (pActor->client == NULL)
    {
    	client = mosquitto_new(guid, TRUE, (void*)pActor);
    	pActor->client = client;
    	// Setting callback for connection
    	mosquitto_connect_callback_set(client, ActorOnConnect);
    	mosquitto_disconnect_callback_set(client, ActorOnOffline);
    	mosquitto_message_callback_set(client, ActorOnMessage);
    	mosquitto_publish_callback_set(client, ActorOnDelivered);
    	// set user and password if needed
    	if ((guid != NULL ) && (psw != NULL))
    		mosquitto_username_pw_set(client, guid, psw);
    }
    else
    	client = pActor->client;
    if (client == NULL)
    	return -1;

    //connect to broker
    //printf("%s connected to %s at port %d\n", pActor->guid, host, port);
    //printf("id: %s, password: %s\n", guid, psw);
    pActor->connected = 0;
    rc = mosquitto_connect(client, host, port, 60);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        mosquitto_destroy(client);
        pActor->client = NULL;
        //printf("%s Failed to connect, return code %d\n", guid, rc);
    }
    free(host);
    return rc;
}

void ActorSend(PACTOR pActor, char* topicName, char* message, ACTORCALLBACKFN callback, char bIdGen, char* type)
{
	json_t* jsonMessage;
	json_t* jsonHeader;
	json_t* jsonId = NULL;
	json_t* typeJson = NULL;
	char* sendBuffer;
	if (pActor->connected == FALSE) return;
	if ((topicName == NULL) || (message == NULL))
		return;
	jsonMessage = json_loads(message, JSON_DECODE_ANY, NULL);
	if (type != NULL)
	{
		typeJson = json_string(type);
		json_object_set(jsonMessage, "type", typeJson);
		json_decref(typeJson);
	}
	jsonHeader = json_object_get(jsonMessage, "header");
	if (jsonHeader != NULL)
	{
		jsonId = json_object_get(jsonHeader, "id");
	}
	if ((jsonId == NULL) && (bIdGen == TRUE))
	{
		char* uuidString = ActorCreateUuidString();
		json_t* uuidJson = json_string(uuidString);
		if (jsonHeader == NULL)
			jsonHeader = json_object();
		json_object_set(jsonHeader, "id", uuidJson);
		json_object_set(jsonMessage, "header", jsonHeader);
		json_decref(uuidJson);
		json_decref(jsonHeader);
		free(uuidString);

	}
	else
	{
		if (jsonId != NULL)
			json_decref(jsonId);
		if (jsonHeader != NULL)
			json_decref(jsonHeader);
	}
	sendBuffer = json_dumps(jsonMessage, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
	mosquitto_publish(pActor->client, &pActor->DeliveredToken, topicName, strlen(sendBuffer),
			(void*)sendBuffer, QOS, 0);

	if (callback != NULL)
	{
		jsonHeader = json_object_get(jsonMessage, "header");
		if (jsonHeader != NULL)
		{
			jsonId = json_object_get(jsonHeader, "id");
			printf("Set callback on id %s\n", json_string_value(jsonId));
			ActorRegisterCallback(pActor, json_string_value(jsonId), callback, CALLBACK_ONCE);
			json_decref(jsonId);
			json_decref(jsonHeader);
		}
	}
	json_decref(jsonMessage);
	free(sendBuffer);
}

void ActorReceive(PACTOR pActor, char* topicName, char* payload)
{
	int i;
	char* TopicNameAct;
	char** TopicNameSplit;
	char** messageSplit;
	char* pParamMessage;
	char* messageContent;
	json_t* receiveJsonMessage;
	json_t* responseJsonReq;
	TopicNameSplit = ActorSplitStringByLim(topicName, '/');
	TopicNameAct = ActorGetActFromTopic(TopicNameSplit);
	if (TopicNameAct == NULL)
	{
		if (*TopicNameSplit)
		{
			for (i = 0; *(TopicNameSplit + i); i++)
			{
				free(*(TopicNameSplit + i));
			}
			free(TopicNameSplit);
		}
		return;
	}
	// Request processing
	//if (strcmp(TopicNameAct, ":request") == 0)
	if (strcmp(TopicNameAct, "action") == 0)
	{
		pParamMessage = StrDup(payload);
		ActorEmitEvent(pActor, topicName, pParamMessage);
	}
	// Event processing
	if (strcmp(TopicNameAct, "event") == 0)
	{
		printf("parsing event");
	}
	// Primary topic processing
	if (strcmp(topicName, pActor->guid) == 0)
	{
		//split message into header and content;
		messageSplit = ActorSplitMessage(payload);
		if (messageSplit == NULL)
		{
			if (*TopicNameSplit)
			{
				for (i = 0; *(TopicNameSplit + i); i++)
				{
					free(*(TopicNameSplit + i));
				}
				free(TopicNameSplit);
			}
			return;
		}
		// assume that message content is the 2nd json message
		messageContent = messageSplit[1];
		receiveJsonMessage = json_loads(messageContent, JSON_DECODE_ANY, NULL);
		if (receiveJsonMessage == NULL)
		{
			if (*TopicNameSplit)
			{
				for (i = 0; *(TopicNameSplit + i); i++)
				{
					free(*(TopicNameSplit + i));
				}
				free(TopicNameSplit);
			}
			ActorFreeSplitMessage(messageSplit);
			return;
		}
		json_t* messageTypeJson = json_object_get(receiveJsonMessage, "type");
		if (messageTypeJson == NULL)
		{
			json_decref(receiveJsonMessage);
			if (*TopicNameSplit)
			{
				for (i = 0; *(TopicNameSplit + i); i++)
				{
					free(*(TopicNameSplit + i));
				}
				free(TopicNameSplit);
			}
			ActorFreeSplitMessage(messageSplit);
			return;
		}
		// parsing response message
		if (strcmp(json_string_value(messageTypeJson), "response") == 0)
		{
			responseJsonReq = json_object_get(receiveJsonMessage, "request");
			if (responseJsonReq == NULL)
			{
				json_decref(receiveJsonMessage);
				json_decref(messageTypeJson);
				if (*TopicNameSplit)
				{
					for (i = 0; *(TopicNameSplit + i); i++)
					{
						free(*(TopicNameSplit + i));
					}
					free(TopicNameSplit);
				}
				ActorFreeSplitMessage(messageSplit);
				return;
			}
			// should check id before get
			json_t* headerJson = json_object_get(responseJsonReq, "header");
			if (headerJson == NULL)
			{
				json_decref(receiveJsonMessage);
				json_decref(messageTypeJson);
				json_decref(responseJsonReq);
				if (*TopicNameSplit)
				{
					for (i = 0; *(TopicNameSplit + i); i++)
					{
						free(*(TopicNameSplit + i));
					}
					free(TopicNameSplit);
				}
				ActorFreeSplitMessage(messageSplit);
				return;
			}
			json_t* requestIdJson = json_object_get(headerJson, "id");
			if (requestIdJson == NULL)
			{
				json_decref(receiveJsonMessage);
				json_decref(messageTypeJson);
				json_decref(responseJsonReq);
				json_decref(headerJson);
				if (*TopicNameSplit)
				{
					for (i = 0; *(TopicNameSplit + i); i++)
					{
						free(*(TopicNameSplit + i));
					}
					free(TopicNameSplit);
				}
				ActorFreeSplitMessage(messageSplit);
				return;
			}
			pParamMessage = StrDup(payload);
			// creat event
			if (json_is_string(requestIdJson))
				ActorEmitEvent(pActor, json_string_value(requestIdJson), pParamMessage);
			json_decref(requestIdJson);
			json_decref(headerJson);
			json_decref(responseJsonReq);
		}
		// parsing action, stop message
		if (strcmp(json_string_value(messageTypeJson), "action/stop") == 0)
		{
			ActorOnRequestStop((PVOID)pActor);
		}
		json_decref(messageTypeJson);
		json_decref(receiveJsonMessage);
		ActorFreeSplitMessage(messageSplit);
	}

	// free allocated memory
	if (*TopicNameSplit)
	{
		//free memory
		for (i = 0; *(TopicNameSplit + i); i++)
		{
			free(*(TopicNameSplit + i));
		}
		free(TopicNameSplit);
	}
}

char* ActorGetGuid(PACTOR pActor)
{
	char* guid = StrDup(pActor->guid);
	return guid;
}
// callback function for MQTTClient event
void ActorOnMessage(struct mosquitto* client, void* context, const struct mosquitto_message* message)
{
	printf("%s received data on topic %s\n", ((PACTOR)context)->guid, message->topic);
	char *messageContent = malloc(message->payloadlen + 1);
	memset(messageContent, 0, message->payloadlen + 1);
	memcpy(messageContent, message->payload, message->payloadlen);
	printf("%s\n", messageContent);
	//Process on message
	ActorReceive((PACTOR)context, message->topic, messageContent);
	free(messageContent);

}

void ActorOnOffline(struct mosquitto* client, void * context, int cause)
{
	printf("\n*** %s Connection lost***\n", ((PACTOR)context)->guid);
	printf("     cause: %d\n", cause);
	//retry connect
	int rc = -1;
	PACTOR pActor = (PACTOR)context;
	mosquitto_destroy(pActor->client);
	pActor->client = NULL;
	pActor->connected = FALSE;
	while (rc != MOSQ_ERR_SUCCESS)
	{
		rc = ActorConnect(pActor, pActor->guid, pActor->psw, pActor->host, pActor->port);
		sleep(5);
	}
}

void ActorOnConnect(struct mosquitto* client, void* context, int result)
{
	PACTOR pActor = (PACTOR)context;
	char* topicName;
	//char* guid = pActor->guid;
	printf("%s actor connected %d\n", pActor->guid, result);
	if (result == 0)
	{
		pActor->connected = 1;
		//no more request stop - chaunm 10/10/2016
		//ActorRegisterCallback(pActor, ":request/stop", ActorOnRequestStop, CALLBACK_RETAIN);

		// listen on action
		//topicName = ActorMakeTopicName(guid, "/:request/#");
		topicName = ActorMakeTopicName("action/", pActor->guid, "/#");
		printf("subscribe to topic %s\n", topicName);
		mosquitto_subscribe(client, &pActor->DeliveredToken, topicName, QOS);
		free(topicName);
		// listen on its primary topic for response and common request
		//topicName = ActorMakeTopicName(guid, "/:response");
		topicName = StrDup(pActor->guid);
		printf("subscribe to topic %s\n", topicName);
		mosquitto_subscribe(client, &pActor->DeliveredToken, topicName, QOS);
		free(topicName);

		//listen on its own event topic to test
		//topicName = ActorMakeTopicName(guid, "/:event/#");
		topicName = ActorMakeTopicName("event/", pActor->guid, "/#");
		printf("subscribe to topic %s\n", topicName);
		mosquitto_subscribe(client, &pActor->DeliveredToken, topicName, QOS);
		free(topicName);

		//publish to the system about online status
		json_t* startJson = json_object();
		json_t* statusJson = json_string("status.online");
		json_object_set(startJson, "status", statusJson);
		char* startMessage = json_dumps(startJson, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
		ActorSend(pActor, "event/service/world/manifest", startMessage, NULL, FALSE, NULL);
		free(startMessage);
		json_decref(statusJson);
		json_decref(startJson);
	}
	else
		pActor->connected = 0;
}

void ActorOnDelivered(struct mosquitto* client, void* context, int dt)
{
	PACTOR pActor = (PACTOR)context;
	printf("%s send message with token value %d delivery confirmed\n", pActor->guid, dt);
}

