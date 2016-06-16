/*
 * ZnpAf.h
 *
 *  Created on: Mar 2, 2016
 *      Author: ChauNM
 */

#ifndef ZNPAF_H_
#define ZNPAF_H_
#include "znp.h"
#include "zcl.h"

/* Znp AF command */
#define AF_REGISTER_REQ				0x0024
#define AF_REGISTER_RSP				0x0064
#define AF_DATA_REQ					0x0124
#define AF_DATA_REQ_RSP				0x0164
#define AF_DATA_CFM					0x8044
#define AF_INCOMING_MSG				0x8144

/* AF transmit option */
#define AF_ACK_REQUEST				0x10
#define AF_DISCV_ROUTE				0x20
#define AF_EN_SECURITY				0x40
#define AF_DEFAULT_RADIUS			10

#define AF_MESSAGE_TIMEOUT			30
/* Af command struct definition */
#pragma pack(1)
typedef struct tagAFENDPOINTREGISTER{
	BYTE nEndPoint;
	WORD nProfileID;
	WORD nDeviceID;
	BYTE nDevVer;
	BYTE nLatencyReq;
	BYTE pData[];
} AFENDPOINTREGISTER, *PAFENDPOINTREGISTER;

#pragma pack(1)
typedef struct tagAFDATAREQ {
	WORD nDstAddr;
	BYTE nDstEndPoint;
	BYTE nSrcEndPoint;
	WORD nClusterID;
	BYTE nTransID;
	BYTE nOptions;
	BYTE nRadius;
	BYTE nLen;
	BYTE pData[];
} AFDATAREQ, *PAFDATAREQ;

#pragma pack(1)
typedef struct tagAFINCOMINGMSG {
	WORD nGroupId;
	WORD nClusterId;
	WORD nSrcAddr;
	BYTE nSrcEndpoint;
	BYTE nDstEndpoint;
	BYTE nWasBroadcast;
	BYTE nLinkQua;
	BYTE nSecuUse;
	DWORD nTimestamp;
	BYTE nTransId;
	BYTE nLen;
	BYTE pData[];
} AFINCOMINGMSG, *PAFINCOMINGMSG;

int AfRegisterEndpoint(BYTE nEndPoint, WORD nProfileID, WORD nDeviceID, BYTE nNumberInCluster,
		BYTE nNumberOutCluster, PWORD pInCluster, PWORD pOutCluster, BYTE nDeviceVer, BYTE nLatencyReq);
VOID AfDataReq(WORD nDstAddr, BYTE nDstEp, BYTE nSrcEp, WORD nClusterID, PBYTE pData, BYTE nLen, BOOL bResponseRequire);
VOID AfProcessIncomingCommand(PZNPPACKAGE pZnpPackage, BYTE nLength);
#endif /* ZNPAF_H_ */
