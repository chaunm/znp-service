/*
 * Znp_Zdo.h
 *
 *  Created on: Mar 2, 2016
 *      Author: ChauNM
 */

#ifndef ZNP_ZDO_H_
#define ZNP_ZDO_H_

#include "znp.h"

/* Znp ZDO command */
#define ZDO_STATE_CHANGE_IND		0xC045
#define ZDO_SIMPLE_DESC_REQ			0X0425
#define ZDO_SIMPLE_DESC_REQ_RSP		0X0465
#define ZDO_ACTIVE_EP_REQ			0x0525
#define ZDO_ACTIVE_EP_REQ_RSP		0x0565
#define ZDO_SIMPLE_DESC_RSP			0x8445
#define ZDO_ACTIVE_EP_RSP			0x8545
#define ZDO_MGMT_PERMIT_JOIN_RSP	0xB645
#define ZDO_END_DEVICE_IEEE_IND		0xCA45
#define ZDO_END_DEVICE_ANNCE_IND	0xC145
#define ZDO_END_DEVICE_LEAVE		0xC945
#define ZDO_BIND_REQ				0x2125
#define ZDO_BIND_REQ_RSP			0x2165
#define ZDO_BIND_RSP				0xA145
#define ZDO_IEEE_ADDR_REQ			0x0125
#define ZDO_IEEE_ADDR_REQ_RSP		0x0165
#define ZDO_IEEE_ADDR_RSP			0x8145
#define ZDO_PERMIT_JOIN_SESSION_END	0xCB45
#define ZDO_MGMT_RTG_REQ			0x3225
#define ZDO_MGMT_RTG_REQ_RSP		0x3265
#define ZDO_MGMT_RTG_RSP			0xB245
/* ZDO command struct definition */
#pragma pack(1)
typedef struct tagZNPZDOPERJOINDATA {
	WORD nSourceAddr;
	BYTE nStatus;
} ZNPZDOPERJOINDATA, *PZNPZDOPERJOINDATA;

#pragma pack(1)
typedef struct tagZDOANNCEINFO {
	WORD nSrcAddr;
	WORD nNwkAddr;
	IEEEADDRESS IeeeAddr;
	BYTE nCapabilities;
} ZDOANNCEINFO, *PZDOANNCEINFO;

typedef struct tagZDOIEEEBRD {
	WORD nNwkAddr;
	IEEEADDRESS IeeeAddr;
	WORD footer;
} ZDOIEEEBRD, *PZDOIEEEBRD;

#pragma pack(1)
typedef struct tagZDOATVEPRQT {
	WORD nDstAddr;
	WORD nNwkAddrOfInterest;
} ZDOATVEPREQ, *PZDOATVEPREQ;

#pragma pack(1)
typedef struct tagZDOATVEPRES {
	WORD nSrcAddr;
	BYTE nStatus;
	WORD nNwkAddr;
	BYTE nNumAtvEp;
	BYTE pEpList[];
} ZDOATVEPRES, *PZDOATVEPRES;

#pragma pack(1)
typedef struct tagZDOSIMPLEDESCREQ {
	WORD nDstAddr;
	WORD nNwkAddrOfInterest;
	BYTE nEndPoint;
} ZDOSIMPLEDESCREQ, *PZDOSIMPLEDESCREQ;

#pragma pack(1)
typedef struct tagZDOSIMPLEDESCRSP {
	WORD nSrcAddr;
	BYTE nStatus;
	WORD nNwkAddr;
	BYTE nLength;
	BYTE nEndpoint;
	WORD nProfileId;
	WORD nDeviceId;
	BYTE nDevVer;
	BYTE pData[];
} ZDOSIMPLEDESCRSP, *PZDOSIMPLEDESCRSP;

#pragma pack(1)
typedef struct tagZDOEDLEAVE {
	WORD nNwkAddress;
	IEEEADDRESS IeeeAddress;
	BYTE pData[3];
}ZDOEDLEAVE, *PZDOEDLEAVE;

#pragma pack(1)
typedef struct tagZDOBINDREQ {
	WORD nDstAddr;
	IEEEADDRESS SrcAddr;
	BYTE nSrcEp;
	WORD nClusterID;
	BYTE nAddrMode;
	IEEEADDRESS DstAddr;
	BYTE nDstEp;
} ZDOBINDREQ, *PZDOBINDREQ;

typedef struct tagZDOBINDRSP {
	WORD nSrcAddr;
	BYTE nStatus;
} ZDOBINDRSP, *PZDOBINDRSP;

typedef struct tagZDOIEEEADDRREQ {
	WORD nDstAddr;
	BYTE nRequestType;
	BYTE nStartIndex;
} ZDOIEEEADDRREQ, *PZDOIEEEADDRREQ;

typedef struct tagZDOIEEEADDRRSP {
	BYTE nStatus;
	IEEEADDRESS IeeeAddr;
	WORD nNetworkAddr;
	BYTE nStartIndex;
	BYTE nAssoDevices;
	WORD nAssoDeviceList[];
}ZDOIEEEADDRRSP, *PZDOIEEEADDRRSP;

typedef struct tagZDOMGMTRTGREQ {
	WORD nAddress;
	BYTE nStartIndex;
} ZDOMGMTRTGREQ, *PZDOMGMTRTGREQ;

typedef struct tagRTGENTRY {
	WORD nAddress;
	BYTE nStatus;
	WORD nextHop;
} RTGENTRY, *PRTGENTRY;

typedef struct tagZDOMGMTRTGRSP {
	WORD nSourceAddr;
	BYTE nStatus;
	BYTE nTotalEntries;
	BYTE nStartIndex;
	BYTE nEntriesCount;
	RTGENTRY entriesList[];
} ZDOMGMTRTGRSP, *PZDOMGMTRTGRSP;
/* exported function */
VOID ZnpZdoProcessIncomingCommand(PZNPPACKAGE pBuffer, BYTE nLength);
BYTE ZnpZdoActiveEpRequest(WORD nNwkAddress);
BYTE ZnpZdoSimpleDescReq(WORD nNwkAddr, BYTE nEndPoint);
VOID ZnpZdoBindReq(WORD nDstAddr, BYTE nSrcEp, WORD nClusterId);
BYTE ZnpZdoIeeeAddrReq(WORD nNetworkAddr, BYTE nRequestType, BYTE nStartIndex);
#endif /* ZNP_ZDO_H_ */
