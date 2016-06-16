/*
 * zcl_gen_onoff.h
 *
 *  Created on: Mar 15, 2016
 *      Author: ChauNM
 */

#ifndef ZCL_GEN_ONOFF_H_
#define ZCL_GEN_ONOFF_H_

#define ZCL_GEN_ONOFF_ATTR_COUNT	1
#define ZCL_GEN_ONOFF_ONOFF_ATTR	0x0000

#define ZCL_GEN_ONOFF_CMD_OFF		0x00
#define ZCL_GEN_ONOFF_CMD_ON		0x01
#define ZCL_GEN_ONOFF_CMD_TGGL		0x02

#pragma pack(1)
typedef struct tagZCLGENONOFFATTR {
	BOOL nOnOff;
	BOOL nWriteOnOff;
} ZCLGENONOFFATTR, *PZCLGENONOFFATTR;

VOID ZclGenOnOffAttrInit(PZCLGENONOFFATTR pData);
BYTE ZclGenOnOffGetAttr(WORD nNwkAddr, BYTE nEp);
VOID ZclGenOnOffUpdateReadAttr(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen);
VOID ZclGenOnOffParseWriteAttrRsp(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLength);
VOID ZclGenOnOffParseAttrReport(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen);
BYTE ZclGenOnOffConfig(WORD nDstAddr, BYTE nEp);
BYTE ZclGenOnOffSendCommand(WORD nDstAddr, BYTE nEp, BYTE nCommand);
#endif /* ZCL_GEN_ONOFF_H_ */
