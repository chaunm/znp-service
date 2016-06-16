/*
 * ZnpCommandState.c
 *
 *  Created on: Jun 11, 2016
 *      Author: ChauNM
 */
#include "znp.h"
#include "ZNP_AF/ZnpAf.h"
#include "ZnpCommandState.h"
static PZNPCMDSTATE pZnpCmdStateList = NULL;
static BOOL bZnpCmdListLock = FALSE;

void ZnpAddCmdState(WORD wCommand, BYTE nResult)
{
	while (bZnpCmdListLock == TRUE);
	bZnpCmdListLock = TRUE;
	PZNPCMDSTATE pLastState = pZnpCmdStateList;
	PZNPCMDSTATE pCmdState = (PZNPCMDSTATE)malloc(sizeof(ZNPCMDSTATE));
	pCmdState->wCommand = wCommand;
	pCmdState->nResult = nResult;
	pCmdState->next = NULL;
	pCmdState->ttl = ZNP_CMD_STATE_TTL;
	// since AF_DATA_REQUEST command doesn't need to wait so drop the state
	if ((wCommand == AF_DATA_REQ_RSP) || (wCommand == 0))
	{
		bZnpCmdListLock = FALSE;
		return;
	}
	if (pZnpCmdStateList == NULL)
	{
		pZnpCmdStateList = pCmdState;
		bZnpCmdListLock = FALSE;
		return;
	}
	while (pLastState->next != NULL)
		pLastState = pLastState->next;
	pLastState->next = pCmdState;
	bZnpCmdListLock = FALSE;
}

int ZnpGetCmdState(WORD wCmd)
{
	int nResult;
	while (bZnpCmdListLock == TRUE);
	bZnpCmdListLock = TRUE;
	PZNPCMDSTATE pCurrentState = pZnpCmdStateList;
	PZNPCMDSTATE pPrevState = pZnpCmdStateList;
	while (pCurrentState != NULL)
	{
		if (pCurrentState ->wCommand == wCmd)
		{
			nResult = pCurrentState->nResult;
			if (pCurrentState == pZnpCmdStateList)
			{
				pZnpCmdStateList = pZnpCmdStateList->next;
				free(pCurrentState);
				bZnpCmdListLock = FALSE;
				return nResult;
			}
			else
			{
				while(pPrevState->next != pCurrentState)
				{
					pPrevState = pPrevState->next;
				}
				pPrevState->next = pCurrentState->next;
				free(pCurrentState);
				bZnpCmdListLock = FALSE;
				return nResult;
			}
		}
		pCurrentState = pCurrentState->next;
	}
	bZnpCmdListLock = FALSE;
	return -1;
}

void ZnpCmdStateProcess()
{
	while (bZnpCmdListLock == TRUE);
	bZnpCmdListLock = TRUE;
	PZNPCMDSTATE pPrevState = pZnpCmdStateList;
	PZNPCMDSTATE pCurrentState = pZnpCmdStateList;
	// check and discard all the state after ttl expired
	while (pCurrentState != NULL)
	{
		pCurrentState->ttl--;
		if (pCurrentState->ttl == 0)
		{
			if (pCurrentState == pZnpCmdStateList)
			{
				pZnpCmdStateList = pZnpCmdStateList->next;
				free(pCurrentState);
				pCurrentState = pZnpCmdStateList;
			}
			else
			{
				while (pPrevState->next != pCurrentState)
				{
					pPrevState = pPrevState->next;
				}
				pPrevState->next = pCurrentState->next;
				free(pCurrentState);
				pCurrentState = pPrevState->next;
			}
		}
		else
		{
			pCurrentState = pCurrentState->next;
		}
	}
	bZnpCmdListLock = FALSE;
}
