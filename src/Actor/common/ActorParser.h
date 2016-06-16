/*
 * ActorParser.h
 *
 *  Created on: Jun 10, 2016
 *      Author: ChauNM
 */

#ifndef ACTORPARSER_H_
#define ACTORPARSER_H_


#pragma pack(1)
typedef struct tagZNPACTORHEADER {
	double timeStamp;
	char* origin;
}ACTORHEADER, *PACTORHEADER;

char** 			ActorSplitStringByLim(char* inputString, const char input_delim);
char* 			ActorGetActFromTopic(char** topicNameSplit);
char* 			ActorCreateUuidString();
char** 			ActorSplitMessage(char* message);
void 			ActorFreeSplitMessage(char** splitMessage);
PACTORHEADER 	ActorParseHeader(char* headerMessage);
void 			ActorFreeHeaderStruct(PACTORHEADER header);
#endif /* ACTORPARSER_H_ */
