/*
 * Znp_System.h
 *
 *  Created on: Mar 2, 2016
 *      Author: ChauNM
 */

#ifndef ZNP_SYSTEM_H_
#define ZNP_SYSTEM_H_

#include "znp.h"

/* define ZNP System command set */
#define SYS_RESET_REQ				0x0041
#define SYS_RESET_IND				0x8041

/* ZNP System command struct definition */
#pragma pack(1)
typedef struct tagZNPSYSRESETIND {
	BYTE nReason;
	BYTE nTransRev;
	BYTE nProductID;
	BYTE nMajorRel;
	BYTE nMinorRel;
	BYTE nHwRev;
} ZNPSYSRESETIND, *PZNPSYSRESETIND;

/* exported fucntion */
VOID ZnpSysProccessIncomingCommand(PZNPPACKAGE pBuffer, BYTE nLength);
#endif /* ZNP_SYSTEM_H_ */
