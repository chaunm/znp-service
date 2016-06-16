/*
 * zcl_gen_powerconfig.h
 *
 *  Created on: Mar 15, 2016
 *      Author: ChauNM
 */

#ifndef ZCL_GEN_POWERCONFIG_H_
#define ZCL_GEN_POWERCONFIG_H_


#define ZCL_GEN_PWR_CFG_ATTR_CNT					14
#define ZCL_GEN_PWR_CFG_MAIN_VOLT_ATTR				0x0000
#define ZCL_GEN_PWR_CFG_MAIN_FREQ_ATTR				0x0001
#define ZCL_GEN_PWR_CFG_MAIN_ALARM_MASK_ATTR		0x0010
#define ZCL_GEN_PWR_CFG_MAIN_VOLT_MIN_THRES_ATTR	0x0011
#define ZCL_GEN_PWR_CFG_MAIN_VOLT_MAX_THRES_ATTR	0x0012
#define ZCL_GEN_PWR_CFG_MAIN_VOLT_TRIP_ATTR			0x0013
#define ZCL_GEN_PWR_CFG_BAT_VOLT_ATTR				0x0020
#define ZCL_GEN_PWR_CFG_BAT_MANU_ATTR				0x0030
#define ZCL_GEN_PWR_CFG_BAT_SIZE_ATTR				0x0031
#define ZCL_GEN_PWR_CFG_BAT_AH_ATTR					0x0032
#define ZCL_GEN_PWR_CFG_BAT_QUANTITY_ATTR			0x0033
#define ZCL_GEN_PWR_CFG_BAT_RATE_VOLT_ATTR			0x0034
#define ZCL_GEN_PWR_CFG_BAT_ALARM_MASK_ATTR			0x0035
#define ZCL_GEN_PWR_CFG_BAT_LOW_VOLT_THRES_ATTR		0x0036

#define BATTERY_LOW_LIMIT			22

#pragma pack(1)
typedef struct tagZCLPWRCFGATTR {
	WORD wMainVoltage;
	BYTE byMainFreq;
	BYTE byMainAlarm;
	WORD wMainMinThres;
	WORD wMainMaxThres;
	WORD wMainTripPoint;
	BYTE byBatVoltage;
	BYTE byBatManu;
	BYTE byBatSize;
	WORD wBatAhRating;
	BYTE byBatQuantity;
	BYTE byBatRatedVoltage;
	BYTE byBatAlarm;
	BYTE byBatLowThresHold;
} ZCLPWRCFGATTR, *PZCLPWRCFGATTR;

void ZclGenPwrCfgAttrInit(PZCLPWRCFGATTR pData);
BYTE ZclGenPwrCfgGetAttr(WORD nDstAddr, BYTE nEp);
VOID ZclGenPwrCfgUpdateReadAttr(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen);
VOID ZclGenPwrCfgParseWriteAttrRsp(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLength);
BYTE ZclGenPwrCfgConfig(WORD nDstAddr, BYTE nEp);
#endif /* ZCL_GEN_POWERCONFIG_H_ */
