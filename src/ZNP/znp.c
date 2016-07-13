/*
 * znp.c
 *
 *  Created on: Jan 28, 2016
 *      Author: ChauNM
 */
#include <stdio.h>
#include "ZnpActor.h"
#include "serialcommunication.h"
#include "ZnpCommandState.h"
#include "queue.h"
#include "universal.h"
#include "log.h"
#include "znp.h"
#include "../fluent-logger/fluent-logger.h"
#include "ZNP_AF/ZnpAf.h"
#include "ZNP_SimpleAPI/Znp_SimpleApi.h"
#include "ZNP_System/Znp_System.h"
#include "ZNP_Util/Znp_Util.h"
#include "ZNP_ZDO/Znp_Zdo.h"


ZNPDEVICE stZnpDevice;
BYTE nZnpDefaultEp = ZNP_DEFAULT_ENDPOINT;
#define ZNP_MAX_TIMEOUT		10

PQUEUECONTENT ZnpMakeSerialCommand(WORD nCommand, BYTE nLength, PBYTE pData)
{
	PQUEUECONTENT pContent = (PQUEUECONTENT)malloc(sizeof(PQUEUECONTENT));
	PBYTE pBuffer = (PBYTE)malloc(nLength + 5);
	PZNPPACKAGE pPackage = _ZNPPACKAGE(pBuffer);
	BYTE nIndex;
	BYTE byCRC = 0;
	pPackage->nStart = 0xFE;
	pPackage->nCommand = nCommand;
	pPackage->nLength = nLength;
	if (pData != NULL)
		for (nIndex = 0; nIndex < nLength; nIndex++)
			pPackage->pData[nIndex] = pData[nIndex];

	for (nIndex = 1; nIndex < nLength + 4; nIndex++)
		byCRC ^= *(pBuffer + nIndex);
	*(pBuffer + nLength + 4) = byCRC;
	pContent->nSize = nLength + 5;
	pContent->pData = pBuffer;
	return pContent;
}

VOID ZnpWriteSerialCommand(PQUEUECONTENT pCommand)
{
	while(ZnpGetState() != ZNP_STATE_ACTIVE)
		usleep(100);
	SerialOutput(stZnpDevice.pSerialPort, pCommand->pData, pCommand->nSize);
}

VOID ZnpSetState(BYTE nState, WORD nActiveCommand)
{
	stZnpDevice.nZnpState = nState;
	stZnpDevice.nZnpActiveCommand = nActiveCommand;
	if (nState == ZNP_STATE_WAIT_RSP)
		stZnpDevice.nZnpTimeout = ZNP_WAIT_RESPONSE_TIME;
	else if (nState == ZNP_STATE_WAIT_IND)
		stZnpDevice.nZnpTimeout = ZNP_WAIT_IND_TIME;
	else
		stZnpDevice.nZnpTimeout = 0;
	/*
	if (nState != ZNP_STATE_ACTIVE)
		QueueSetState((stZnpDevice.pSerialPort)->pOutputQueue, QUEUE_WAIT);
	else
		QueueSetState((stZnpDevice.pSerialPort)->pOutputQueue, QUEUE_ACTIVE);
	*/
}

BYTE ZnpGetState()
{
	return stZnpDevice.nZnpState;
}

VOID ZnpSetZdoState(BYTE nState)
{
	stZnpDevice.nZdoState = nState;
}

BYTE ZnpGetZdoState()
{
	return stZnpDevice.nZdoState;
}

VOID ZnpSetIeeeAddr(IEEEADDRESS IeeeAddress)
{
	stZnpDevice.IeeeAddr = IeeeAddress;
}

IEEEADDRESS ZnpGetIeeeAddr()
{
	return stZnpDevice.IeeeAddr;
}

VOID ZnpSetShortAddr(WORD nShortAddr)
{
	stZnpDevice.nShortAddress = nShortAddr;
}

WORD ZnpGetShortAddr()
{
	return stZnpDevice.nShortAddress;
}

BYTE ZnpGetDefaultEp()
{
	return nZnpDefaultEp;
}

VOID ZnpStart()
{
	PQUEUECONTENT pStartCommand = ZnpMakeSerialCommand(ZB_START_REQ, 0, NULL);
	WORD nZigbeeStartTime = 600;
	ZnpWriteSerialCommand(pStartCommand);
	ZnpSetState(ZNP_STATE_WAIT_RSP, ZB_START_RES);
	free(pStartCommand->pData);
	free(pStartCommand);
	while (ZnpGetState() != ZNP_STATE_ACTIVE);
	ZnpGetCmdState(ZB_START_RES);
	while (ZnpGetZdoState() != ZNP_ZIGBEE_COOR_READY)
	{
		sleep(1);
		nZigbeeStartTime--;
		if (nZigbeeStartTime == 0)
		{
			printf("Timeout: ZNP start network fail \n");
			break;
		}
	}
}

BOOL ZnpInit(PSERIAL pSerialPort, WORD nStatusUpdateTime)
{
	BYTE nTimeout = 0;
	PQUEUECONTENT pCommandContent;
	stZnpDevice.pSerialPort = pSerialPort;
	stZnpDevice.nZnpState = ZNP_STATE_ACTIVE;
	stZnpDevice.nZdoState = ZNP_ZIGBEE_STATE_HOLD;
	stZnpDevice.nZnpActiveCommand = 0;
	if (nStatusUpdateTime != 0)
		stZnpDevice.nStatusUpdateTime = nStatusUpdateTime;
	else
		stZnpDevice.nStatusUpdateTime = ZNP_DEFAULT_STATUS_UPDATE_TIME;
	stZnpDevice.nZnpTimeout = 0;
	//Send reset command
	PBYTE pCommandData = malloc(sizeof(BYTE));
	//Reset ZNP if reset fail return
	*pCommandData = 0;
	pCommandContent = ZnpMakeSerialCommand(SYS_RESET_REQ, 1, pCommandData);
	ZnpWriteSerialCommand(pCommandContent);
	free(pCommandContent->pData);
	free(pCommandContent);
	free(pCommandData);
	ZnpSetState(ZNP_STATE_WAIT_IND, SYS_RESET_IND);
	while(ZnpGetState() != ZNP_STATE_ACTIVE);
	{
		nTimeout++;
		sleep(1);
		//timeout reset fail try to reset again
		if (nTimeout == 10)
			return FALSE;
	}
#ifdef CLEAR_NETWORK_STATE
	//reset ZNP network info if needed
	pCommandData = malloc(sizeof(BYTE));
	*(PWORD)pCommandData = 0x02;
	ZnpZbWriteConfig(ZCD_NV_STARTUP_OPTION, sizeof(BYTE), pCommandData);
	free(pCommandData);
	ZnpSetState(ZNP_STATE_WAIT_IND, SYS_RESET_IND);
	while(ZnpGetState() != ZNP_STATE_ACTIVE);
	{
		nTimeout++;
		sleep(1);
		//timeout reset fail try to reset again
		if (nTimeout == 10)
			return FALSE;
	}
#endif
	//Write ZDO_DIRECT_CB_CONFIG
	pCommandData = malloc(sizeof(BYTE));
	*pCommandData = 1;
	ZnpZbWriteConfig(ZCD_NV_ZDO_DIRECT_CB, 1, pCommandData);
	free(pCommandData);
	//write ZCD_NV_LOGICAL_TYPE
	pCommandData = malloc(sizeof(BYTE));
	*pCommandData = ZNP_DEVICE_TYPE_COORDINATOR;
	ZnpZbWriteConfig(ZCD_NV_LOGICAL_TYPE, 1, pCommandData);
	free(pCommandData);
	//write ZCD_NV_PANID
	pCommandData = malloc(sizeof(WORD));
	*(PWORD)pCommandData = 0xFFFF;
	ZnpZbWriteConfig(ZCD_NV_PANID, sizeof(WORD), pCommandData);
	free(pCommandData);
	//write ZCD_NV_CHANLIST - ALL Channel
	pCommandData = malloc(sizeof(DWORD));
	*(PDWORD)pCommandData = 0x00000800;
	ZnpZbWriteConfig(ZCD_NV_CHANLIST, sizeof(DWORD), pCommandData);
	free(pCommandData);
	while (ZnpGetState() != ZNP_STATE_ACTIVE);
	//Register AF Endpoint
	printf("Register AF Endpoint\n");
	AfRegisterEndpoint(nZnpDefaultEp, 0x0104, 0x0402, 0, 0, NULL, NULL, 1, 0);
	//Start Application
	ZnpStart();
	// Get device info
	ZnpUtilGetDeviceInfo();
	//Permit join request
	ZnpZbPermitJoiningReq(0xFFFC, 255);
	return TRUE;
}

VOID ZnpHandleCommand(PBYTE pBuffer, BYTE nLength)
{
	PZNPPACKAGE pZnpData = (PZNPPACKAGE)pBuffer;
	BYTE nCommandSubSystem;
	nCommandSubSystem = (BYTE)(pZnpData->nCommand & 0x001F);
	switch (nCommandSubSystem)
	{
	case ZNP_SYS_INTERFACE:
		ZnpSysProccessIncomingCommand(pZnpData, nLength);
		break;
	case ZNP_AF_INTERFACE:
		AfProcessIncomingCommand(pZnpData, nLength);
		break;
	case ZNP_ZDO_INTERFACE:
		ZnpZdoProcessIncomingCommand(pZnpData, nLength);
		break;
	case ZNP_SIMPLE_API_INTERFACE:
		ZnpZbProcessIncomingCommand(pZnpData, nLength);
		break;
	case ZNP_UTIL_INTERFACE:
		ZnpUtilProcessIncomingCommand(pZnpData, nLength);
		break;
	default:
		break;
	}
	if ((stZnpDevice.nZnpState != ZNP_STATE_ACTIVE) &&
			(stZnpDevice.nZnpActiveCommand == pZnpData->nCommand))
		ZnpSetState(ZNP_STATE_ACTIVE, 0);
}

VOID ZnpStateProcess()
{
	static BYTE nTimeCount = 0;
	if (stZnpDevice.nZnpState != ZNP_STATE_ACTIVE)
	{
		if (stZnpDevice.nZnpTimeout > 0)
		{
			stZnpDevice.nZnpTimeout--;
			if (stZnpDevice.nZnpTimeout == 0)
			{
				ZnpActorPublishZnpStatus("status.offline.znp_comm_fail");
				FLUENT_LOGGER_INFO("Publish status offline");
			}
		}
	}
	else
	{
		nTimeCount++;
		if (nTimeCount == stZnpDevice.nStatusUpdateTime)
		{
			nTimeCount = 0;
			ZnpActorPublishZnpStatus("status.online");
			FLUENT_LOGGER_INFO("Publish status online");
		}
	}
}
