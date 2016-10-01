/*
 * devices.h
 *
 *  Created on: Mar 2, 2016
 *      Author: ChauNM
 */

#ifndef DEVICESMANAGER_H_
#define DEVICESMANAGER_H_

#include "ZNP_ZDO/Znp_Zdo.h"
#include <stddef.h>

#define DEFAULT_DEVICE_TIMEOUT 	86400
#define DEFAULT_MESSAGE_TIMEOUT	30

#define LQI_LIMIT	10

#pragma pack(1)
typedef struct tagCLUSTERINFO
{
	WORD nCluster;
	WORD nDataSize;
	PBYTE pData;
}CLUSTERINFO, *PCLUSTERINFO;

#pragma pack(1)
typedef struct tagENDPOINTINFO {
	BYTE nEndPoint;
	WORD nProfileID;
	WORD nDeviceID;
	BYTE nDeviceVersion;
	BYTE nInCluster;
	BYTE nOutCluster;
	WORD nDeviceType;
	PCLUSTERINFO pInClusterList;
	PCLUSTERINFO pOutClusterList;
} ENDPOINTINFO, *PENDPOINTINFO;

#pragma pack(1)
typedef struct tagDEVICEINFO {
	IEEEADDRESS IeeeAddr;
	WORD nNetworkAddress;
	BYTE nNumberActiveEndpoint;
	PENDPOINTINFO pEndpointInfoList;
	DWORD nDeviceTimeOut;
	struct tagDEVICEINFO* NextDevice;
} DEVICEINFO, *PDEVICEINFO;

#pragma pack(1)
typedef struct tagDEVICEINFORM {
	WORD nNwkAddr;
	WORD nCommand;
	BYTE nTransID;
} DEVICEINFORM, *PDEVICEINFORM;

#pragma pack(1)
typedef struct tagENDPOINTADDR{
	WORD nNwkAddr;
	BYTE nEp;
} ENDPOINTADDR, *PENDPOINTADDR;

#pragma pack(1)
typedef struct tagCLUSTERADDR {
	WORD nNwkAddr;
	BYTE nEp;
	WORD nClusterID;
} CLUSTERADDR, *PCLUSTERADDR;

VOID DeviceListInit();
VOID DeviceSetInformed(WORD nNwkAddr, WORD nCommand, BYTE nTransID);
VOID DeviceAdd(PZDOANNCEINFO pDeviceInfo);
VOID DeviceRemove(WORD nNwkAddr);
PDEVICEINFO DeviceFind(WORD nNwkAddr);
PENDPOINTINFO DeviceFindEpInfo(WORD nNwkAddr, BYTE nEndpoint);
PCLUSTERINFO DeviceFindClusterInfo(WORD nNwkAddr, BYTE nEndpoint, WORD nClusterId);
VOID DeviceUpdateIeeeAddr(WORD nAddress, IEEEADDRESS IeeeAddr);
VOID DeviceUpdateDeviceInfo(WORD nNwkAddr, BYTE nNumEp, PBYTE pEndpointList);
VOID DeviceUpdateEndpointDesc(PZDOSIMPLEDESCRSP pSimpleDescInfo);
VOID DeviceAddEndpoint(PENDPOINTADDR pEpAddr);
VOID DeviceAddCluster(PCLUSTERADDR pClusterAddr);
BYTE DeviceWaitInformed(WORD nNwkAddr, WORD nCommand, BYTE nTransID);
VOID DeviceProcessTimeout();
VOID DeviceSetTimeoutTime(WORD nAddress, DWORD nTime);

#endif /* DEVICESMANAGER_H_ */
