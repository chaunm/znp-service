/*
 * fluent-logger.c
 *
 *  Created on: Jul 7, 2016
 *      Author: ChauNM
 */

#include <unistd.h>
#include <pthread.h>
#include "fluent-logger.h"
#include "fluentlogger.h"
#include "jansson.h"
#include "universal.h"

LOGGER logger;

void FluentLoggerInit(PLOGGEROPTION loggerOpt)
{
	logger.context = NULL;
	logger.sender = StrDup(loggerOpt->sender);
	logger.sending = FALSE;
	if (loggerOpt->host != NULL)
		logger.host = StrDup(loggerOpt->host);
	else
		logger.host = StrDup("127.0.0.1");
}

static void FluentLoggerSend(PLOGGERMSG message)
{
	while (logger.sending == TRUE);
	logger.sending = TRUE;
	logger.context = fluent_connect(logger.host, FLUENT_LOGGER_PORT);
	if (logger.context == NULL)
	{
		logger.sending = FALSE;
		free(message);
		return;
	}
	char* logMessage;
	json_t* logJson = json_object();
	json_t* messageJson = json_string(message->message);
	json_t* fromJson = json_string(logger.sender);
	json_object_set(logJson, "from", fromJson);
	json_object_set(logJson, "message", messageJson);
	logMessage = json_dumps(logJson, JSON_INDENT(4));
	fluent_post_json(logger.context, message->level, logMessage);
	json_decref(messageJson);
	json_decref(fromJson);
	json_decref(logJson);
	free(logMessage);
	fluent_free(logger.context);
	logger.context = NULL;
	free(message);
	logger.sending = FALSE;
}

void FluentLoggerPost(char* level, char* message)
{
	//pthread_t sendThread;
	// try to connect to fluentd
	PLOGGERMSG loggerMsg = malloc(sizeof(LOGGERMSG));
	loggerMsg->level = level;
	loggerMsg->message = message;
	FluentLoggerSend(loggerMsg);
	//pthread_create(&sendThread, NULL,(void*)&FluentLoggerSend, (void*)loggerMsg);
	//pthread_detach(sendThread);
}

