/*
 * Znp_Util.h
 *
 *  Created on: Mar 2, 2016
 *      Author: ChauNM
 */

#ifndef ZNP_UTIL_H_
#define ZNP_UTIL_H_

#include "znp.h"

/* Znp Util command */
#define UTIL_GET_DEVICE_INFO  		0x0027
#define UTIL_GET_DEVICE_INFO_RES	0x0067

/* Znp Util command struct definition */
#pragma pack (1)
typedef struct tagUTILDEVICEINFO {
	BYTE nStatus;
	IEEEADDRESS IeeeAddr;
	WORD nShortAddr;
	BYTE nDeviceType;
	BYTE nDeviceState;
	BYTE nAssocDevice;
	BYTE pData[];
} UTILDEVICEINFO, *PUTILDEVICEINFO;
/* exported function */
int ZnpUtilGetDeviceInfo();
VOID ZnpUtilProcessIncomingCommand(PZNPPACKAGE pBuffer, BYTE nLength);

#endif /* ZNP_UTIL_H_ */
