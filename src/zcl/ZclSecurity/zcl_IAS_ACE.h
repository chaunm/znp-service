/*
 * zcl_IAS_ACE.h
 *
 *  Created on: Apr 21, 2016
 *      Author: ChauNM
 */

#ifndef ZCL_IAS_ACE_H_
#define ZCL_IAS_ACE_H_

#define ZCL_IAS_ACE_ARM_RESPONSE				0x00
#define ZCL_IAS_ACE_GET_ZONE_ID_MAP_RESPONSE	0x01
#define ZCL_IAS_ACE_GET_ZONE_INFO_RESPONSE		0x02

#define ALL_ZONE_DISARMED						0x00
#define ONLY_DAY_ZONE_ARMED						0x01
#define ONLY_NIGHT_ZONE_ARMED					0x02
#define ALL_ZONE_ARMED							0x03

typedef struct tagZCLIASACEARMNOTI {
	BYTE nArmNotification;
	BYTE pData[];
} ZCLIASACEARMNOTI, *PZCLIASACEARMNOTI;

VOID ZclIasAceParseSpecificPackage(WORD nNwkAddr, BYTE nEndpoint, BYTE nCommandId,PBYTE pZclData, BYTE nDataSize);
BYTE ZclIasAceConfig(WORD nDstAddr, BYTE nEp);

#endif /* ZCL_IAS_ACE_H_ */
