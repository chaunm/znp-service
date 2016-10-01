/*
 * DeviceDesc.h
 *
 *  Created on: Mar 16, 2016
 *      Author: ChauNM
 */

#ifndef DEVICEDESC_H_
#define DEVICEDESC_H_
#include <jansson.h>

#pragma pack(1)
typedef struct tagDEVICEDESC {
	WORD nId;
	WORD nNwkAddr;
	IEEEADDRESS IeeeAddress;
	BYTE nEp;
	WORD nDevId;
	WORD nDeviceType;
	struct tagDEVICEDESC* NextDevice;
} DEVICEDESC, *PDEVICEDESC;


VOID DeviceDescInit(PDEVICEINFO pDevInfo);
VOID DevDesUpdateFromDeviceInfo(PDEVICEINFO pFirstDev);
json_t* DevDesMakeDeviceClassJson(WORD deviceId, WORD deviceType);
//VOID DevDescAdd(PDEVICEDESC pDeviceDesc);
//VOID DevDescRemove(WORD nNwkAddr, BYTE nEp);
#endif /* DEVICEDESC_H_ */
