/*
 * znp.h
 *
 *  Created on: Jan 28, 2016
 *      Author: ChauNM
 */

#ifndef ZNP_H_
#define ZNP_H_

#include "typesdef.h"
#include "serialcommunication.h"



#define ZNP_DEFAULT_ENDPOINT	1

/* define ZNP State */
#define ZNP_STATE_ACTIVE	0x00
#define ZNP_STATE_WAIT_RSP	0x01
#define ZNP_STATE_WAIT_IND	0x02

/* define ZNP Zigbee State */
#define ZNP_ZIGBEE_STATE_HOLD 	0x00
#define ZNP_ZIGBEE_COOR_READY	0x09

/* ZNP Device type */
#define ZNP_DEVICE_TYPE_COORDINATOR		0x00
#define ZNP_DEVICE_TYPE_ROUTER			0x01
#define ZNP_DEVICE_TYPE_ENDDEVICE		0x02
/* time for waiting response from ZNP */
#define ZNP_WAIT_RESPONSE_TIME			10
#define ZNP_WAIT_IND_TIME				10
#define ZNP_DEFAULT_STATUS_UPDATE_TIME	20

/* define ZNP Command */
#pragma pack(1)
typedef struct tagZNPDEVICE {
	PSERIAL pSerialPort;
	IEEEADDRESS IeeeAddr;
	WORD nShortAddress;
	BYTE nZnpState;
	BYTE nZdoState;
	BYTE nZnpTimeout;
	BYTE nStatusUpdateTime;
	WORD nZnpActiveCommand;
} ZNPDEVICE, *PZNPDEVICE;


#pragma pack(1)
typedef struct tagZNPPACKAGE {
	BYTE nStart;
	BYTE nLength;
	WORD nCommand;
	BYTE pData[];
} ZNPPACKAGE, *PZNPPACKAGE;

#pragma pack(1)
typedef struct tagZNPCLUSTERLIST {
	BYTE nNumberCluster;
	WORD arClusterList[];
} ZNPCLUSTERLIST, *PZNPCLUSTERLIST;




#define _ZNPPACKAGE(pBuffer)	((PZNPPACKAGE)pBuffer)
#define _ZNPCONTENT(pBuffer)	((PBYTE)(pBuffer) + 4)

/* define ZNP command interface */
#define ZNP_RPC_ERR_INTERFACE		0
#define ZNP_SYS_INTERFACE			1
#define ZNP_AF_INTERFACE			4
#define ZNP_ZDO_INTERFACE			5
#define ZNP_SIMPLE_API_INTERFACE	6
#define ZNP_UTIL_INTERFACE			7

/* Exported function */
VOID ZnpHandleCommand(PBYTE pBuffer, BYTE nLength);
PQUEUECONTENT ZnpMakeSerialCommand(WORD nCommand, BYTE nLength, PBYTE pData);
VOID ZnpWriteSerialCommand(PQUEUECONTENT pCommand);
VOID ZnpSetState(BYTE nState, WORD nActiveCommand);
BYTE ZnpGetState();
VOID ZnpSetZdoState(BYTE nState);
BYTE ZnpGetZdoState();
VOID ZnpSetIeeeAddr(IEEEADDRESS IeeeAddr);
IEEEADDRESS ZnpGetIeeeAddr();
VOID ZnpSetShortAddr(WORD nShortAddr);
WORD ZnpGetShortAddr();
BYTE ZnpGetDefaultEp();
BOOL ZnpInit(PSERIAL pSerialPort, WORD nStatusUpdateTime);
VOID ZnpStateProcess();
#endif /* ZNP_H_ */
