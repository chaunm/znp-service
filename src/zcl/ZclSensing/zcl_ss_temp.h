/*
 * zcl_ss_temp.h
 *
 *  Created on: Mar 23, 2016
 *      Author: ChauNM
 */

#ifndef ZCL_SS_TEMP_H_
#define ZCL_SS_TEMP_H_

#define ZCL_SS_TEMP_ATTR_COUNT		4

#define ZCL_SS_TEMP_MEASURED_ATTR	0x0000
#define ZCL_SS_TEMP_MIN_ATTR		0x0001
#define ZCL_SS_TEMP_MAX_ATTR		0x0002
#define ZCL_SS_TEMP_TOLERANCE_ATTR	0x0003

#pragma pack(1)
typedef struct tagZCLSSTEMPATTR {
	SWORD nMeasuredValue;
	SWORD nMinValue;
	SWORD nMaxValue;
	WORD nTolerance;
} ZCLSSTEMPATTR, *PZCLSSTEMPATTR;

VOID ZclSsTempAttrInit(PZCLSSTEMPATTR pData);
BYTE ZclSsTempGetAttr(WORD nNwkAddr, BYTE nEp);
VOID ZclSsTempUpdateReadAttr(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen);
VOID ZclSsTempParseWriteAttrRsp(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLength);
VOID ZclSsTempParseAttrReport(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen);
BYTE ZclSsTempConfig(WORD nDstAddr, BYTE nEp);

#endif /* ZCL_SS_TEMP_H_ */
