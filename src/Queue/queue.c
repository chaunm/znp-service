/*
 * queue.c
 *
 *  Created on: Jan 22, 2016
 *      Author: ChauNM
 */

#include "queue.h"
#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "../fluent-logger/fluent-logger.h"
PQUEUECONTROL QueueCreate(BYTE nQueueSize, BYTE nBufferSize)
{
	PBYTE pQueueContent;
	PBYTE pBufferLength;
	BYTE nIndex;
	PQUEUECONTROL pQueue;
	if (nQueueSize <= 1) return NULL;
	pQueueContent = (PBYTE)malloc(nQueueSize * nBufferSize);
	if (pQueueContent == NULL) return NULL;
	pBufferLength = (PBYTE)malloc(nQueueSize);
	for (nIndex = 0; nIndex < nQueueSize; nIndex++)
		pBufferLength[nIndex] = 0;
	if (pBufferLength == NULL)
	{
		free(pQueueContent);
		return NULL;
	}
	pQueue = (PQUEUECONTROL)malloc(sizeof(QUEUECONTROL));
	if (pQueue == NULL)
	{
		free(pQueueContent);
		free(pBufferLength);
		return NULL;
	}
	pQueue->nBufferSize = nBufferSize;
	pQueue->nQueueSize = nQueueSize;
	pQueue->nCurrentPointer = nQueueSize - 1;
	pQueue->nFreePointer = 0;
	pQueue->nState = QUEUE_ACTIVE;
	pQueue->pContent = pQueueContent;
	pQueue->pContentLength = pBufferLength;
	return pQueue;
}

void QueueFreeMem(PQUEUECONTROL pQueue)
{
	if (pQueue == NULL) return;
	free((void *)(pQueue->pContent));
	free((void *)(pQueue->pContentLength));
	free((void *)pQueue);
}

BYTE QueuePush(void* pSource, BYTE nSize, PQUEUECONTROL pQueue)
{
	BYTE nIndex;
	PBYTE pDataContent;
	if (pQueue == NULL) return 1;
	if (pQueue->nFreePointer == pQueue->nCurrentPointer)
	{
		//printf("queue is full\n");
		FLUENT_LOGGER_WARN("Queue full");
		return 1;
	}
	if (nSize > pQueue->nBufferSize)
	{
		FLUENT_LOGGER_WARN("Input data is too large");
		//printf("input data is too large");
		return 1;
	}
	pQueue->pContentLength[pQueue->nFreePointer] = nSize;
	pDataContent = pQueue->pContent + (pQueue->nFreePointer * pQueue->nBufferSize);
	for (nIndex = 0; nIndex < nSize; nIndex++)
		*(pDataContent + nIndex) = *((PBYTE)(pSource + nIndex));
	pQueue->nFreePointer++;
	if (pQueue->nFreePointer == pQueue->nQueueSize)
		pQueue->nFreePointer = 0;
	return 0;
}

QUEUECONTENT QueueGetContent(PQUEUECONTROL pQueue)
{
	QUEUECONTENT stContent;
	stContent.nSize = 0;
	stContent.pData = NULL;
	if (pQueue == NULL) return stContent;
	BYTE nPosition = pQueue->nCurrentPointer + 1;
	if (nPosition == pQueue->nQueueSize) nPosition = 0;
	if (nPosition != pQueue->nFreePointer)
	{
		stContent.nSize = (pQueue->pContentLength)[nPosition];
		stContent.pData = (PBYTE)(pQueue->pContent + (pQueue->nBufferSize * nPosition));
	}
	return stContent;
}

VOID QueueFinishProcBuffer(PQUEUECONTROL pQueue)
{
	pQueue->nCurrentPointer++;
	if (pQueue->nCurrentPointer == pQueue->nQueueSize)
		pQueue->nCurrentPointer = 0;
}

VOID QueueSetState(PQUEUECONTROL pQueue, BYTE nState)
{
	if (pQueue != NULL)
	{
		pQueue->nState = nState;
	}
}

BYTE QueueGetState(PQUEUECONTROL pQueue)
{
	return pQueue->nState;
}
