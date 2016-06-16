/*
 * ZnpActor.h
 *
 *  Created on: Jun 8, 2016
 *      Author: ChauNM
 */

#ifndef ZNPACTOR_H_
#define ZNPACTOR_H_


#pragma pack(1)
typedef struct tagZNPDEVICEADDDATA {
	IEEEADDRESS MacId;
	BYTE nEndpoint;
	WORD nDeviceClass;
	BYTE nProtocol;
} ZNPDEVICEADDDATA, *PZNPDEVICEADDDATA;

#pragma pack(1)
typedef struct tagZNPACTOROPTION {
	char* guid;
	char* psw;
}ZNPACTOROPTION, *PZNPACTOROPTION;

#pragma pack(1)
typedef struct tagZNPACTORDATA {
	char* dataName;
	BYTE nDataType;
	void* value;
} ZNPACTORDATA,*PZNPACTORDATA;

#define DEVICE_SENSOR_DOOR		0x01
#define DEVICE_SENSOR_MOTION	0x02
#define DEVICE_SENSOR_TEMP		0x03
#define DEVICE_SENSOR_HUMI		0x04
#define DEVICE_SENSOR_SMOKE		0x05

#define ZNP_DATA_TYPE_INTERGER	0
#define ZNP_DATA_TYPE_FLOAT		1
#define ZNP_DATA_TYPE_STRING	2

void ZnpActorCreate(PZNPACTOROPTION option);
void ZnpActorPublishEvent(char* event, void* eventParam);
void ZnpActorPublishDeviceAddedEvent(IEEEADDRESS macId, BYTE endpoint, WORD deviceId, WORD deviceType);
void ZnpActorPublishDeviceRemovedEvent(IEEEADDRESS macId);
void ZnpActorPublishDeviceOfflineEvent(IEEEADDRESS macId);
void ZnpActorPublishDeviceOnlineEvent(IEEEADDRESS macId);
void ZnpActorPublishDeviceErrorEvent(IEEEADDRESS macId, WORD error);
PZNPACTORDATA ZnpActorMakeData(char* dataName, BYTE nDataType, void* data, BYTE dataLen);
void ZnpActorDestroyZnpData(PZNPACTORDATA pData);
void ZnpActorPublishDeviceDataEvent(IEEEADDRESS macId, BYTE endPoint, BYTE nDataCount, PZNPACTORDATA *pData);
#endif /* ZNPACTOR_H_ */
