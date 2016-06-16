/*
 * zcl_IAS_zone.h
 *
 *  Created on: Mar 5, 2016
 *      Author: ChauNM
 */

#ifndef ZCL_IAS_ZONE_H_
#define ZCL_IAS_ZONE_H_
#include "typesdef.h"

#define ZCL_SECU_DEFAULT_ZONE		0x00

/* IAS Zone definition */
#define ZCL_IAS_ZONE_ATTR_COUNT		4
#define ZCL_IAS_ZONE_STATE_ATTR		0x0000
#define ZCL_IAS_ZONE_TYPE_ATTR		0x0001
#define ZCL_IAS_ZONE_STATUS_ATTR	0x0002
#define ZCL_IAS_ZONE_SETTING_ATTR	0x0010

/* ZCL security command */
#define ZCL_IAS_ZONE_ENROLL_RSP		0x00
#define ZCL_IAS_ZONE_STATUS_CHNG	0x00
#define ZCL_IAS_ZONE_ENROLL_RQT		0x01

/* ZCL enroll response code */
#define ZCL_IAS_ZONE_ENROLL_SUCCESS		0x00
#define ZCL_IAS_ZONE_ENROLL_NOT_SPT		0x01
#define ZCL_IAS_ZONE_ENROLL_NOT_PER		0x02
#define ZCL_IAS_ZONE_ENROLL_MANY_ZONE	0x03

#define ZCL_IAS_ZONE_STD_CIE			0x0000
#define ZCL_IAS_ZONE_MOTION_SENSOR		0x000D
#define ZCL_IAS_ZONE_CONTACT_SWITCH		0x0015
#define ZCL_IAS_ZONE_FIRE_SENSOR		0x0028
#define ZCL_IAS_ZONE_WATER_SENSOR		0x002A
#define ZCL_IAS_ZONE_GAS_SENSOR			0x002B
#define ZCL_IAS_ZONE_PERSONAL_EMER		0x002C
#define ZCL_IAS_ZONE_VIBRATION_SENSOR	0x002D
#define ZCL_IAS_ZONE_RM_CONTROL			0x010F
#define ZCL_IAS_ZONE_KEY_FOB			0x0115
#define ZCL_IAS_ZONE_KEY_PAD			0x021D
#define ZCL_IAS_ZONE_STANDARD_WARNING	0x0225

#pragma pack(1)
typedef struct tagZCLIASZONEATTR {
	BYTE nZoneState;
	WORD nZoneType;
	WORD nZoneStatus;
	IEEEADDRESS nZoneSetting;
	IEEEADDRESS nWriteZoneSetting;
}ZCLIASZONEATTR, *PZCLIASZONEATTR;

#pragma pack(1)
typedef struct tagZCLIASZONEENROLLRQT {
	WORD nZoneType;
	WORD nManuCode;
} ZCLIASZONEENROLLRQT, *PZCLIASZONEENROLLRQT;

#pragma pack(1)
typedef struct tagZCLIASZONEENROLLRSP {
	BYTE nRspCode;
	BYTE nZoneId;
} ZCLIASZONEENROLLRSP, *PZCLIASZONEENROLLRSP;

#pragma pack(1)
typedef struct tagZCLIASZONESTATUSCHG {
	WORD nZoneStatus;
	BYTE nExt;
	BYTE pData[];
} ZCLIASZONESTATUSCHG, *PZCLIASZONESTATUSCHG;

VOID ZclIasZoneAttrInit(PZCLIASZONEATTR pData);
BYTE ZclIasZoneGetAttr(WORD nDstAddr, BYTE nEp);
BYTE ZclIasZoneConfig(WORD nDstAddr, BYTE nEp);
VOID ZclIasZoneParseSpecificPackage(WORD nNwkAddr, BYTE nEndpoint, BYTE nCommandId, PBYTE pZclData, BYTE nDataSize);
VOID ZclIasZoneUpdateReadAttr(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen);
VOID ZclIasZoneParseWriteAttrRsp(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLength);
VOID ZclIasZoneParseAttrReport(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLength);

#endif /* ZCL_IAS_ZONE_H_ */
