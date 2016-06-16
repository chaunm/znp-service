/*
 * ZnpCommandState.h
 *
 *  Created on: Jun 11, 2016
 *      Author: ChauNM
 */

#ifndef ZNPCOMMANSTATE_H_
#define ZNPCOMMANDSTATUS_H_

typedef struct tagZNPCMDSTATE {
	WORD wCommand;
	BYTE nResult;
	BYTE ttl;
	struct tagZNPCMDSTATE* next;
} ZNPCMDSTATE, *PZNPCMDSTATE;;

#define ZNP_CMD_STATE_TTL 	3

void ZnpAddCmdState(WORD wCmd, BYTE nResult);
int  ZnpGetCmdState(WORD wCmd);
void ZnpCmdStateProcess();
#endif /* ZNPCOMMANSTATE_H_ */
