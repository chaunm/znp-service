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

#ifdef PI_RUNNING
#include "unistd.h"
#endif

int ActorConnect(PACTOR pACtor, char* guid, char* psw);
void ActorOnMessage(struct mosquitto* client, void* context, const struct mosquitto_message* message);
void ActorOnOffline(struct mosquitto* client, void * context, int cause);
void ActorOnConnect(struct mosquitto* client, void* context, int result);
void ActorOnDelivered(struct mosquitto* client, void* context, int dt);


char* ActorMakeGuid(char* prefix)
{
	char* znpGuid;
	char* uuid = ActorCreateUuidString();
	znpGuid = malloc(strlen(uuid) + strlen(prefix) + 1);
	memset(znpGuid, 0, strlen(uuid) + strlen("ZNP-") + 2);
	sprintf(znpGuid, "%s-%s", prefix, uuid);
	return znpGuid;
}

char* ActorMakeTopicName(const char* guid, char* topic)
{
	char* topicName = malloc(strlen(guid) + strlen(topic) + 1);
	memset(topicName, 0, strlen(guid) + strlen(topic) + 1);
	sprintf(topicName, "%s%s", guid, topic);
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
	if (event == NULL) return;
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
					pSearchCallback = pActor->pActorCallback;
					if (pSearchCallback == pProcessingCallback)
					{
						pActor->pActorCallback = pProcessingCallback->nextCallback;
						free(pProcessingCallback->event);
						free(pProcessingCallback);
					}
					else
					{
						while (pSearchCallback->nextCallback != pProcessingCallback)
							pSearchCallback = pSearchCallback->nextCallback;
						if (pSearchCallback == pProcessingCallback)
						{
							pSearchCallback->nextCallback = pProcessingCallback->nextCallback;
							free(pProcessingCallback->event);
							free(pProcessingCallback);
						}
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

PACTOR ActorCreate(char* guid, char* psw)
{
	if ((guid == NULL))
		return NULL;
	PACTOR pActor = (PACTOR)malloc(sizeof(ACTOR));
	memset(pActor, 0, sizeof(ACTOR));
	pActor->guid = StrDup(guid);
	pActor->psw = StrDup(psw);
	ActorConnect(pActor, pActor->guid, pActor->psw);
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

int ActorConnect(PACTOR pActor, char* guid, char* psw)
{

    int rc;
    char* topicName;
    struct mosquitto* client;
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
    if (client == NULL) return -1;

    //connect to broker
    printf("%s connected to %s at port %d\n", pActor->guid, HOST, PORT);
    printf("id: %s, password: %s\n", guid, psw);
    pActor->connected = 0;
    rc = mosquitto_connect(client, HOST, PORT, 60);
    while (rc != MOSQ_ERR_SUCCESS)
    {
        //mosquitto_destroy(client);
        printf("%s Failed to connect, return code %d\n", guid, rc);
        //pActor->client = NULL;
        rc = mosquitto_connect(client, HOST, PORT, 60);
        sleep(5);
    }

    topicName = ActorMakeTopicName(guid, "/:request/#");
    printf("subscribe to topic %s\n", topicName);
    mosquitto_subscribe(client, &pActor->DeliveredToken, topicName, QOS);
    free(topicName);

    topicName = ActorMakeTopicName(guid, "/:response");
    printf("subscribe to topic %s\n", topicName);
    mosquitto_subscribe(client, &pActor->DeliveredToken, topicName, QOS);
    free(topicName);

    //listen on its own event topic to test
    topicName = ActorMakeTopicName(guid, "/:event/#");
    printf("subscribe to topic %s\n", topicName);
    mosquitto_subscribe(client, &pActor->DeliveredToken, topicName, QOS);
    free(topicName);
    return 0;
}

void ActorSend(PACTOR pActor, char* topicName, char* message, ACTORCALLBACKFN callback, char bIdGen)
{
	json_t* jsonMesage;
	char* sendBuffer;
	if ((topicName == NULL) || (message == NULL))
		return;
	jsonMesage = json_loads(message, JSON_DECODE_ANY, NULL);
	if ((!json_object_get(jsonMesage, "id")) && (bIdGen == TRUE))
	{
		char* uuidString = ActorCreateUuidString();
		json_t* uuidJson = json_string(uuidString);
		json_object_set(jsonMesage, "id", uuidJson);
		json_decref(uuidJson);
		free(uuidString);
		sendBuffer = json_dumps(jsonMesage, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
	}
	else
		sendBuffer = StrDup(message);
	mosquitto_publish(pActor->client, &pActor->DeliveredToken, topicName, strlen(sendBuffer),
			(void*)sendBuffer, QOS, 0);

	if (callback != NULL)
	{
		ActorRegisterCallback(pActor, json_string_value(json_object_get(jsonMesage, "id")), callback, CALLBACK_ONCE);
	}
	json_decref(jsonMesage);
	free(sendBuffer);
}

void ActorReceive(PACTOR pActor, char* topicName, char* payload)
{
	int i;
	char* TopicNameAct;
	char* relTopic;
	char relTopicPosition;
	int	relTopicSize = 0;
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
	if (strcmp(TopicNameAct, ":request") == 0)
	{
		// get position of topic act in topic name
		relTopicPosition = 0;
		while (strcmp(*(TopicNameSplit + relTopicPosition), ":request") != 0)
		{
			relTopicPosition++;
		}
		// get the size of relative topic
		i = relTopicPosition;
		while (*(TopicNameSplit + i))
		{
			relTopicSize += strlen(*(TopicNameSplit + i));
			i++;
		}
		relTopicSize++; // to ensure that the final byte of the relative topic name is 0, so the string format is valid
		relTopic = malloc(relTopicSize);
		memset(relTopic, 0, relTopicSize);
		sprintf(relTopic,"%s", *(TopicNameSplit + relTopicPosition));
		i = relTopicPosition + 1;
		while (*(TopicNameSplit + i))
		{
			sprintf(relTopic,"%s%s%s", relTopic, "/", *(TopicNameSplit + i));
			i++;
		}
		pParamMessage = StrDup(payload);
		ActorEmitEvent(pActor, relTopic, pParamMessage);
		free(relTopic);
	}
	// Response processing
	if (strcmp(TopicNameAct, ":response") == 0)
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
		puts("Response received");
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
		responseJsonReq = json_object_get(receiveJsonMessage, "request");
		if (responseJsonReq == NULL)
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

		// should check id before get
		json_t* requestIdJson = json_object_get(responseJsonReq, "id");
		if (requestIdJson == NULL)
		{
			json_decref(receiveJsonMessage);
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
		pParamMessage = StrDup(payload);
		// creat event
		ActorEmitEvent(pActor, json_string_value(requestIdJson), pParamMessage);
		json_decref(requestIdJson);
		json_decref(responseJsonReq);
		json_decref(receiveJsonMessage);
		ActorFreeSplitMessage(messageSplit);
	}
	// Event processing
	if (strcmp(TopicNameAct, ":event"))
	{

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
	while (rc != 0)
	{
		rc = ActorConnect(pActor, pActor->guid, pActor->psw);
		if (rc != 0)
			printf("reconnected fail\n");
		else
			printf("reconnect success\n");
		sleep(5);
	}
}

void ActorOnConnect(struct mosquitto* client, void* context, int result)
{
	PACTOR pActor = (PACTOR)context;
	printf("%s actor connected %d\n", pActor->guid, result);
	if (result == 0)
		pActor->connected = 1;
	else
		pActor->connected = 0;
}

void ActorOnDelivered(struct mosquitto* client, void* context, int dt)
{
	PACTOR pActor = (PACTOR)context;
	printf("%s send message with token value %d delivery confirmed\n", pActor->guid, dt);
}

