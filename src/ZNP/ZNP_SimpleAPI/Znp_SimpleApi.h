/*
 * Znp_SimpleApi.h
 *
 *  Created on: Mar 2, 2016
 *      Author: ChauNM
 */

#ifndef ZNP_SIMPLEAPI_H_
#define ZNP_SIMPLEAPI_H_

#include "znp.h"

/* Znp ZB Command */
#define ZB_READ_CONFIGURATRION_REQ	0x0426
#define ZB_READ_CONFIGURATRION_RES	0x0466
#define ZB_WRITE_CONFIGURATION_REQ	0x0526
#define ZB_WRITE_CONFIGURATION_RES	0x0566
#define ZB_START_REQ				0x0026
#define ZB_START_RES				0x0066
#define ZB_START_CFM				0x8046
#define ZB_PERMIT_JOIN_REQ			0x0826
#define ZB_PERMIT_JOIN_RES			0x0866


/* ZNP configuration information */
#define ZCD_NV_STARTUP_OPTION			0x0003
#define ZCD_NV_LOGICAL_TYPE				0x0087
#define ZCD_NV_ZDO_DIRECT_CB			0x008F
#define ZCD_NV_POLL_RATE				0x0024
#define ZCD_NV_QUEUED_POLL_RATE			0x0025
#define ZCD_NV_RESPONSE_POLL_RATE		0x0026
#define ZCD_NV_POLL_FAILURE_RETRIES		0x0029
#define ZCD_NV_INDIRECT_MSG_TIMEOUT		0x002B
#define ZCD_NV_APS_FRAME_RETRIES		0x0043
#define ZCD_NV_APS_ACK_WAIT_DURATION	0x0044
#define ZCD_NV_BIDING_TIME				0x0046
#define ZCD_NV_USERDESC					0x0081
#define ZCD_NV_PANID					0x0083
#define ZCD_NV_CHANLIST					0x0084
#define ZCD_NV_PRECFGKEY				0x0062
#define ZCD_NV_PRECFGKEY_ENABLE			0x0063
#define ZCD_NV_SECURITY_MODE			0x0064
#define ZCD_NV_USE_DEFAULT_TCLK			0x006D
#define ZCD_NV_BCAST_RETRIES			0x002E
#define ZCD_NV_PASSIVE_ACK_TIMEOUT		0x002F
#define ZCD_NV_BCAST_DELIVERY_TIME		0x0030
#define ZCD_NV_ROUTE_EXPIRY_TIME		0x002C
#define ZCD_NV_RF_TEST_PARAMS			0x0F07

/* Znp ZB command struct definition */
#pragma pack(1)
typedef struct tagZNPZBPERMITJOIN
{
	WORD nAddress;
	BYTE nTimeout;
} ZNPZBPERMITJOIN, *PZNPZBPERMITJOIN;

#pragma pack(1)
typedef struct tagZNPZBCONFIG {
	BYTE nConfigID;
	BYTE nLength;
	BYTE pData[];
} ZNPZBCONFIG, *PZNPZBCONFIG;

int ZnpZbWriteConfig(BYTE nConfigID, BYTE nLength, PBYTE pData);
int ZnpZbPermitJoiningReq(WORD nAddress, BYTE nTimeout);
VOID ZnpZbProcessIncomingCommand(PZNPPACKAGE pZnpPackage, BYTE nLength);

#endif /* ZNP_SIMPLEAPI_H_ */
