/*
 * zcl_ss_rehumid.h
 *
 *  Created on: Mar 23, 2016
 *      Author: ChauNM
 */

#ifndef ZCL_SS_REHUMID_H_
#define ZCL_SS_REHUMID_H_

#define ZCL_SS_REHUMID_ATTR_COUNT		4

#define ZCL_SS_REHUMID_MEASURED_ATTR	0x0000
#define ZCL_SS_REHUMID_MIN_ATTR			0x0001
#define ZCL_SS_REHUMID_MAX_ATTR			0x0002
#define ZCL_SS_REHUMID_TOLERANCE_ATTR	0x0003

#pragma pack(1)
typedef struct tagZCLSSREHUMIDATTR {
	WORD nMeasuredValue;
	WORD nMinValue;
	WORD nMaxValue;
	WORD nTolerance;
} ZCLSSREHUMIDATTR, *PZCLSSREHUMIDATTR;

VOID ZclSsReHumidAttrInit(PZCLSSREHUMIDATTR pData);
BYTE ZclSsReHumidGetAttr(WORD nNwkAddr, BYTE nEp);
VOID ZclSsReHumidUpdateReadAttr(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen);
VOID ZclSsReHumidParseWriteAttrRsp(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLength);
VOID ZclSsReHumidParseAttrReport(WORD nNwkAddr, BYTE nEp, PBYTE pData, BYTE nLen);
BYTE ZclSsReHumidConfig(WORD nDstAddr, BYTE nEp);


#endif /* ZCL_SS_REHUMID_H_ */
