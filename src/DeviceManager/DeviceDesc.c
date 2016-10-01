/*
 * DeviceDesc.c
 *
 *  Created on: Mar 16, 2016
 *      Author: ChauNM
 */

#include <stddef.h>
#include <stdio.h>
#include <jansson.h>
#include "DevicesManager.h"
#include "DeviceDesc.h"
#include "ZigbeeHaDeviceDesc.h"
#include "universal.h"
#include "zcl.h"


#ifdef PI_RUNNING
static char DescTxtFile[] = "/home/pi/ZigbeeHost/data/DeviceDesc.txt";
static char DescDatFile[] = "/home/pi/ZigbeeHost/data/DeviceDesc.dat";
#endif
#ifdef PC_RUNNING
static char DescTxtFile[] = "/home/chaunm/ZigbeeHost/data/DeviceDesc.txt";
static char DescDatFile[] = "/home/chaunm/ZigbeeHost/data/DeviceDesc.dat";
#endif
static PDEVICEDESC pFirstDevDesc = NULL;


VOID DeviceDescInit(PDEVICEINFO pFirstDeviceInfo)
{
	DWORD nFileSize = 0;
	FILE *fDataFile;
	FILE *fTextFile;
	PDEVICEDESC pLastDeviceRead, pDevDesc;
	PWORD pIeee;
	pLastDeviceRead = NULL;
	fDataFile = fopen(DescDatFile, "r");
	nFileSize = 0;
	if (fDataFile == NULL)
	{
		pFirstDevDesc = NULL;
		// write file from pDeviceInfoList
		DevDesUpdateFromDeviceInfo(pFirstDeviceInfo);
		return;
	}
	fseek(fDataFile, 0L, SEEK_END);
	nFileSize = ftell(fDataFile);
	fseek(fDataFile, 0L, SEEK_SET);

	if (nFileSize == 0)
	{
		pFirstDevDesc = NULL;
	}
	else
	{
		while(nFileSize > 0)
		{
			// Read data from file
			pDevDesc = (PDEVICEDESC)malloc(sizeof(DEVICEDESC));
			if (pLastDeviceRead != NULL)
				pLastDeviceRead->NextDevice = pDevDesc;
			nFileSize -= fread((PVOID)pDevDesc, sizeof(DEVICEDESC) , 1, fDataFile) * sizeof(DEVICEDESC);
			pDevDesc->NextDevice = NULL;
			if(pFirstDevDesc == NULL) pFirstDevDesc = pDevDesc;
			pLastDeviceRead = pDevDesc;
		}
	}
	fclose(fDataFile);
	nFileSize = 0;
	fTextFile = fopen(DescTxtFile, "r");
	if (fTextFile != NULL)
	{
		fseek(fTextFile, 0L, SEEK_END);
		nFileSize = ftell(fTextFile);
		fseek(fTextFile, 0L, SEEK_SET);
		fclose(fTextFile);
	}
	if (nFileSize > 0)
	{
		return;
	}
	else
	{
		//write description to Text file
		fTextFile = fopen(DescTxtFile, "w");
		pDevDesc = pFirstDevDesc;
		while (pDevDesc != NULL)
		{
			pIeee = (PWORD)(&(pDevDesc->IeeeAddress));
			fprintf(fTextFile, "ID: 0x%04X, Address: 0x%04X, Endpoint: 0x%02X, DeviceId: 0x%04X, DeviceType: 0x%04x, IEEE Address: 0x%04X%04X%04X%04X\n",
					pDevDesc->nId, pDevDesc->nNwkAddr, pDevDesc->nEp, pDevDesc->nDevId, pDevDesc->nDeviceType,
					pIeee[3], pIeee[2], pIeee[1], pIeee[0]);
			pDevDesc = pDevDesc->NextDevice;
		}
		fclose(fTextFile);
	}
}

VOID DevDesUpdateFromDeviceInfo (PDEVICEINFO pFirstDev)
{
	PDEVICEINFO pDeviceInfo = pFirstDev;
	PDEVICEDESC pDeviceDesc = pFirstDevDesc;
	PDEVICEDESC pRemoveDesc;
	WORD nId = 1;
	BYTE nEpIndex;
	FILE* fDatFile;
	FILE* fTextFile;
	PWORD pIeee;
	while (pDeviceDesc != NULL)
	{
		pRemoveDesc = pDeviceDesc;
		pDeviceDesc = pRemoveDesc->NextDevice;
		free(pRemoveDesc);
	}
	pFirstDevDesc = NULL;
	fDatFile = fopen(DescDatFile, "w+");
	fTextFile = fopen(DescTxtFile, "w+");
	while (pDeviceInfo != NULL)
	{
		for (nEpIndex = 0; nEpIndex < pDeviceInfo->nNumberActiveEndpoint; nEpIndex++)
		{
			pDeviceDesc = (PDEVICEDESC)calloc(1, sizeof(DEVICEDESC));
			//pDeviceDesc = (PDEVICEDESC)malloc(sizeof(DEVICEDESC));
			//memset(pDeviceDesc, 0, sizeof(DEVICEDESC))
			pDeviceDesc->nId = nId;
			nId++;
			pDeviceDesc->nNwkAddr = pDeviceInfo->nNetworkAddress;
			pDeviceDesc->nEp = pDeviceInfo->pEndpointInfoList[nEpIndex].nEndPoint;
			pDeviceDesc->nDevId = pDeviceInfo->pEndpointInfoList[nEpIndex].nDeviceID;
			pDeviceDesc->IeeeAddress = pDeviceInfo->IeeeAddr;
			if (pDeviceDesc->nDevId != DEVICE_ID_INVALID)
				pDeviceDesc->nDeviceType = pDeviceInfo->pEndpointInfoList[nEpIndex].nDeviceType;
			else
				if (pDeviceInfo->pEndpointInfoList[nEpIndex].pInClusterList != NULL)
					pDeviceDesc->nDeviceType = pDeviceInfo->pEndpointInfoList[nEpIndex].pInClusterList[0].nCluster;
			pDeviceDesc->NextDevice = NULL;
			if (pFirstDevDesc == NULL)
				pFirstDevDesc = pDeviceDesc;
			fwrite((PVOID)pDeviceDesc, sizeof(DEVICEDESC), 1, fDatFile);
			pIeee = (PWORD)(&(pDeviceDesc->IeeeAddress));
			fprintf(fTextFile, "ID: 0x%04X, Address: 0x%04X, Endpoint: 0x%02X, DeviceId: 0x%04X, DeviceType: 0x%04x, IEEE Address: 0x%04X%04X%04X%04X\n",
					pDeviceDesc->nId, pDeviceDesc->nNwkAddr, pDeviceDesc->nEp, pDeviceDesc->nDevId, pDeviceDesc->nDeviceType,
					pIeee[3], pIeee[2], pIeee[1], pIeee[0]);
		}
		pDeviceInfo = pDeviceInfo->NextDevice;
	}
	fclose(fDatFile);
	fclose(fTextFile);
}

VOID DevDesUpdateFiles()
{
	PDEVICEDESC pDevDesc;
	FILE* fDatFile;
	FILE* fTextFile;
	PWORD pIeee;
	fDatFile = fopen(DescDatFile, "r");
	fTextFile = fopen(DescTxtFile, "r");
	if (fDatFile != NULL)
	{
		fclose(fDatFile);
		unlink(DescDatFile);
	}
	if (fTextFile != NULL)
	{
		fclose(fTextFile);
		unlink(DescTxtFile);
	}
	fDatFile = fopen(DescDatFile, "w");
	fTextFile = fopen(DescTxtFile, "w");
	pDevDesc = pFirstDevDesc;
	while(pDevDesc != NULL)
	{
		fwrite((PVOID)pDevDesc, sizeof(PDEVICEDESC), 1, fDatFile);
		pIeee = (PWORD)(&(pDevDesc->IeeeAddress));
		fprintf(fTextFile, "ID: 0x%04X, Address: 0x%04X, Endpoint: 0x%02X, DeviceId: 0x%04X, DeviceType: 0x%04x, IEEE Address: 0x%04X%04X%04X%04X\n",
				pDevDesc->nId, pDevDesc->nNwkAddr, pDevDesc->nEp, pDevDesc->nDevId, pDevDesc->nDeviceType,
				pIeee[3], pIeee[2], pIeee[1], pIeee[0]);
		pDevDesc = pDevDesc->NextDevice;
	}
}

json_t* DevDesMakeDeviceClassJson(WORD deviceId, WORD deviceType)
{
	json_t* deviceClassJson;
	switch (deviceId)
	{
	case DEVICE_ID_OCCUPANCY_SENSOR:
		deviceClassJson = json_string("class.device.sensor.occupancy");
		break;
	case DEVICE_ID_TEMPERATURE_SENSOR:
		switch(deviceType)
		{
		case ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT:
			deviceClassJson = json_string("class.device.sensor.temperature");
			break;
		case ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY:
			deviceClassJson = json_string("class.device.sensor.humidity");
			break;
		default:
			deviceClassJson = json_string("class.device.sensor.unknown");
			break;
		}
		break;
		case DEVICE_ID_IAS_ZONE:
			switch(deviceType)
			{
			case ZCL_IAS_ZONE_MOTION_SENSOR:
				deviceClassJson = json_string("class.device.sensor.motion");
				break;
			case ZCL_IAS_ZONE_CONTACT_SWITCH:
				deviceClassJson = json_string("class.device.sensor.door");
				break;
			case ZCL_IAS_ZONE_FIRE_SENSOR:
				deviceClassJson = json_string("class.device.sensor.fire");
				break;
			case ZCL_IAS_ZONE_WATER_SENSOR:
				deviceClassJson = json_string("class.device.sensor.water");
				break;
			case ZCL_IAS_ZONE_GAS_SENSOR:
				deviceClassJson = json_string("class.device.sensor.gas");
				break;
			case ZCL_IAS_ZONE_VIBRATION_SENSOR:
				deviceClassJson = json_string("class.device.sensor.vibration");
				break;
			case ZCL_IAS_ZONE_RM_CONTROL:
				deviceClassJson = json_string("class.device.keyfob.remote");
				break;
			case ZCL_IAS_ZONE_KEY_FOB:
				deviceClassJson = json_string("class.device.keyfob.panic");
				break;
			default:
				deviceClassJson = json_string("class.device.security.unknown");
				break;
			}
			break;
			case DEVICE_ID_IAS_ACE:
				deviceClassJson = json_string("class.device.keyfob.remote");
				break;
			default:
				deviceClassJson = json_string("class.device.unknown");
				break;
	}
	return deviceClassJson;
}

/*
PDEVICEDESC DevDescFind(WORD nNwkAddr, BYTE nEp)
{
	PDEVICEDESC pDevDesc;
	pDevDesc = pFirstDevDesc;
	while (pDevDesc != NULL)
	{
		if ((pDevDesc->nNwkAddr == nNwkAddr) && (pDevDesc->nEp == nEp))
			return pDevDesc;
	}
	return NULL;
}

VOID DevDescRemove(WORD nNwkAddr, BYTE nEp)
{
	PDEVICEDESC pRemovedDevice;
	PDEVICEDESC pCurrentDevice;
	PDEVICEDESC pPreviousDevice;
	pPreviousDevice = pFirstDevDesc;
	pCurrentDevice = pFirstDevDesc;
	while (pCurrentDevice != NULL)
	{
		pRemovedDevice = NULL;
		if ((pCurrentDevice->nNwkAddr == nNwkAddr) && (pCurrentDevice->nEp == nEp))
		{
			pRemovedDevice = pCurrentDevice;
			if (pCurrentDevice == pFirstDevDesc)
			{
				pFirstDevDesc = pCurrentDevice->NextDevice;
			}
			else
			{
				pPreviousDevice->NextDevice = pRemovedDevice->NextDevice;
			}
		}
		else
			pPreviousDevice = pCurrentDevice;
		pCurrentDevice = pCurrentDevice->NextDevice;
		if (pRemovedDevice != NULL)
			free(pRemovedDevice);
	}
	DevDesUpdateFiles();
}

VOID DevDescAdd(PDEVICEDESC pDeviceDesc)
{
	PDEVICEDESC pDevDesc;
	PDEVICEDESC pLastDevice;
	pDevDesc = (PDEVICEDESC)malloc(sizeof(DEVICEDESC));
	CopyMemory((PBYTE)pDevDesc, (PBYTE)pDeviceDesc, sizeof(DEVICEDESC));
	pDevDesc->NextDevice = NULL;
	pLastDevice = pFirstDevDesc;
	//check if device desc already in list
	pDevDesc = DevDescFind(pDeviceDesc->nNwkAddr, pDeviceDesc->nEp);
	if (pDevDesc != NULL)
		DevDescRemove(pDeviceDesc->nNwkAddr, pDeviceDesc->nEp);
	if (pLastDevice == NULL)
	{
		pFirstDevDesc = pDevDesc;
	}
	else
	{
		while (pLastDevice->NextDevice != NULL)
			pLastDevice = pLastDevice->NextDevice;
		pLastDevice->NextDevice = pDevDesc;
	}
	DevDesUpdateFiles();
}
*/


