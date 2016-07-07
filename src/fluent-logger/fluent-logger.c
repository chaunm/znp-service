/*
 * fluent-logger.c
 *
 *  Created on: Jul 7, 2016
 *      Author: ChauNM
 */

#include <unistd.h>
#include "fluent-logger.h"
#include "fluentlogger.h"
#include "jansson.h"
#include "universal.h"

LOGGER logger;

void FluentLoggerInit(PLOGGEROPTION loggerOpt)
{
	logger.context = NULL;
	logger.sender = StrDup(loggerOpt->sender);
	logger.host = StrDup(loggerOpt->host);
	while (logger.context == NULL)
	{
		printf("fluentd sender: %s, host: %s\n", logger.sender, logger.host);
		if (logger.host != NULL)
			logger.context = fluent_connect(logger.host, FLUENT_LOGGER_PORT);
		else
			logger.context = fluent_connect("127.0.0.1", FLUENT_LOGGER_PORT);
		if (logger.context == NULL)
			sleep(10);
	}
}

void FluentLoggerPost(char* level, char* message)
{
	if (logger.context == NULL)
		return;
	char* logMessage;
	json_t* logJson = json_object();
	json_t* messageJson = json_string(message);
	json_t* fromJson = json_string(logger.sender);
	json_object_set(logJson, "from", fromJson);
	json_object_set(logJson, "message", messageJson);
	logMessage = json_dumps(logJson, JSON_INDENT(4));
	fluent_post_json(logger.context, level, logMessage);
	json_decref(messageJson);
	json_decref(fromJson);
	json_decref(logJson);
	free(logMessage);
}

