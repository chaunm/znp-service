/*
 * zcl_gen_alarm.h
 *
 *  Created on: Jun 14, 2016
 *      Author: ChauNM
 */

#ifndef ZCL_GEN_ALARM_H_
#define ZCL_GEN_ALARM_H_

#define ZCL_ALARM_BATTERY_LOW_CODE 	0x10

#define ZCL_GEN_ALARM_ATTR_COUNT 	0x01
#define ZCL_GEN_ALARM_COUNT_ATTR	0x00

#define ZCL_GEN_ALARM_ALARM_CMD				0x00
#define ZCL_GEN_ALARM_GET_ALARM_RSP_CMD		0x01


#pragma pack(1)
typedef struct tagZCLGENALARMATTR {
	WORD wAlarmCount;
}ZLCGENALARMATTR, *PZCLGENALARMATTR;

#pragma pack(1)
typedef struct tagZCLGENALARMALRMCMD {
	BYTE nAlarmCode;
	WORD nClusterID;
}ZCLGENALARMALRMCMD, *PZCLGENALARMALRMCMD;

VOID ZclGenAlarmAttrInit(PZCLGENALARMATTR pData);
BYTE ZclGenAlarmGetAttr(WORD nDstAddr, BYTE nEp);
VOID ZclGenAlarmUpdateReadAttr(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen);
BYTE ZclGenAlarmConfig(WORD nDstAddr, BYTE nEp);
VOID ZclGenAlarmParseSpecificPackage(WORD nNwkAddr, BYTE nEndpoint, BYTE nCommandId, PBYTE pZclData, BYTE nDataSize);
#endif /* ZCL_GEN_ALARM_H_ */
