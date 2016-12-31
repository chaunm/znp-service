/*
 * ActorParser.c
 *
 *  Created on: Jun 10, 2016
 *      Author: chaunm
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <uuid/uuid.h>
#include "jansson.h"
#include "ActorParser.h"
#include "universal.h"

char** ActorSplitStringByLim(char* inputString, const char input_delim)
{
	char* input = StrDup(inputString);
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = input;
    char* last_comma = 0;
    char delim[2];
    delim[0] = input_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (input_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (input + strlen(input) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(input, delim);

        while (token)
        {
            //assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        //assert(idx == count - 1);
        *(result + idx) = 0;
    }
    free(input);
    return result;
}

char* ActorGetActFromTopic(char** topicNameSplit)
{
	/*
	int i = 0;
	while (*(topicNameSplit + i) != NULL)
	{
		if ((strcmp(*(topicNameSplit + i), ":request") == 0) ||
			(strcmp(*(topicNameSplit + i), ":response") == 0) ||
			(strcmp(*(topicNameSplit + i), ":event") == 0))
		{
			return *(topicNameSplit + i);
		}
		else
			i++;
	}
	*/
	if (*(topicNameSplit) != NULL)
		return *(topicNameSplit);
	return NULL;
}

char* ActorCreateUuidString()
{
	uuid_t uuid;
	char* uuidString = (char*)malloc(50);
	memset(uuidString, 0, 50);
	uuid_generate(uuid);
	sprintf(uuidString, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",	uuid[0], uuid[1],uuid[2],uuid[3],
			uuid[4],uuid[5],uuid[6],uuid[7],uuid[8],uuid[9],uuid[10],uuid[11],uuid[12],uuid[13],uuid[14],uuid[15]);
	char* resultString = StrDup(uuidString);
	free(uuidString);
	return resultString;
}

static char ActorValidateMessage(char* message)
{
	int i;
	int count = 0;
	if (message[0] != '{')
		return 0;
	else
	{
		for(i = 0; i < strlen(message); i++)
		{
			if (message[i] == '{') count++;
			if (message[i] == '}') count--;
		}
		if (count != 0)
			return 0;
	}
	return 1;
}

// split received message into list of json message
char** ActorSplitMessage(char* message)
{
	char** result = NULL;
	int messageCount = 0;
	int messageIndex = 0;
	int i;
	int limCount = 0;
	int messageStart;
	int messageEnd;

	if (ActorValidateMessage(message) == 0)
		return NULL;
	for (i = 0; i < strlen(message); i++)
	{
		if (message[i] == '{')
		{
			limCount++;
		}
		if (message[i] == '}')
		{
			limCount--;
			if (limCount == 0)
				messageCount++;
		}

	}
	// assume that all the message always content 2 json packages
	if (messageCount != 2)
		return NULL;
	result = (char**)malloc(sizeof(char*) * (messageCount + 1)); // +1 to dedicate that the end of list is 0
	*(result + messageCount) = NULL;
	//copy JsonMessage to list
	limCount = 0;
	messageIndex = 0;
	messageStart = 0;
	for (i = 0; i < strlen(message); i++)
	{
		if (message[i] == '{')
		{
			if (limCount == 0)
				messageStart = i;
			limCount++;
		}
		if (message[i] == '}')
		{
			limCount--;
			if (limCount == 0)
			{
				messageEnd = i;
				*(result + messageIndex) = malloc(messageEnd - messageStart + 2);
				memset(*(result + messageIndex), 0, messageEnd - messageStart + 2);
				memcpy(*(result + messageIndex), (message + messageStart), messageEnd - messageStart + 1);
				messageIndex++;
				if (messageIndex == messageCount) break;
			}
		}
	}
	return result;
}

void ActorFreeSplitMessage(char** splitMessage)
{
	if (splitMessage == NULL) return;
	int count = 0;
	while (*(splitMessage + count) != NULL)
	{
		free(*(splitMessage + count));
		count++;
	}
	free(splitMessage);
}

PACTORHEADER ActorParseHeader(char* headerMessage)
{
	json_t* fromJson;
	json_t* timeStampJson;
	json_t* headerJson = json_loads(headerMessage, JSON_DECODE_ANY, NULL);
	PACTORHEADER pZnpActorHeader;
	if (headerJson == NULL)
	{
		return NULL;
	}
	fromJson = json_object_get(headerJson, "from");
	if (fromJson == NULL)
	{
		json_decref(headerJson);
		return NULL;
	}
	pZnpActorHeader = malloc(sizeof(PACTORHEADER));
	pZnpActorHeader->origin = StrDup(json_string_value(fromJson));
	timeStampJson = json_object_get(headerJson, "timestamp");
	if (timeStampJson == NULL)
		pZnpActorHeader->timeStamp = 0;
	else
	{
		pZnpActorHeader->timeStamp = json_number_value(timeStampJson);
		json_decref(timeStampJson);
	}
	json_decref(fromJson);
	json_decref(headerJson);
	return pZnpActorHeader;
}

void ActorFreeHeaderStruct(PACTORHEADER header)
{
	if (header != NULL)
	{
		if (header->origin != NULL)
			free(header->origin);
		free(header);
	}
}
