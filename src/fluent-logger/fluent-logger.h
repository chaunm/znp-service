/*
 * fluent-logger.h
 *
 *  Created on: Jul 7, 2016
 *      Author: ChauNM
 */

#ifndef FLUENT_LOGGER_H_
#define FLUENT_LOGGER_H_

#include "fluentlogger.h"
#define FLUENT_LOGGER_PORT 24224

typedef struct tagLOGGER {
	fluent_context_t *context;
	char* sender;
	char* host;
	BOOL sending;
} LOGGER, *PLOGGER;

typedef struct tagLOGGEROPTION {
	char* sender;
	char* host;
} LOGGEROPTION, *PLOGGEROPTION;

typedef struct tagLOGGERMSG {
	char* level;
	char* message;
} LOGGERMSG, *PLOGGERMSG;

void FluentLoggerInit(PLOGGEROPTION loggerOpt);
void FluentLoggerPost(char* level, char* message);

#define FLUENT_LOGGER_INFO(message)		FluentLoggerPost("mqtt.log.info", message);
#define FLUENT_LOGGER_WARN(message) 	FluentLoggerPost("mqtt.log.warn", message);
#define FLUENT_LOGGER_ERROR(message) 	FluentLoggerPost("mqtt.log.error", message);
#define FLUENT_LOGGER_FATAL(message) 	FluentLoggerPost("mqtt.log.fatal", message);
#define FLUENT_LOGGER_DEBUG(message)	FluentLoggerPost("mqtt.log.debug", message);

#endif /* FLUENT_LOGGER_H_ */
