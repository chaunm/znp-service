/*
 * devices.c
 *
 *  Created on: Mar 2, 2016
 *      Author: ChauNM
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fluentlogger.h"
#include "DevicesManager.h"
#include "DeviceDesc.h"
#include "ZigbeeHaDeviceDesc.h"
#include "universal.h"
#include "ZNP_AF/ZnpAf.h"
#include "log.h"
#include "ZnpActor.h"
#include "../fluent-logger/fluent-logger.h"
#ifdef PI_RUNNING
static char FileName[] = "/home/pi/ZigbeeHost/data/devices.dat";
#endif

#define CLUSTER_GET_IDLE_TIME	100000

#ifdef PC_RUNNING
static char FileName[] = "/home/chaunm/ZigbeeHost/data/devices.dat";
#endif

static PDEVICEINFO pFirstDevice = NULL;
static WORD nDeviceCount = 0;
static DEVICEINFORM stDeviceInformance = {0};

static VOID DeviceListWriteToFile();
static DWORD DeviceReadInfoFromFile(FILE* pFile, PDEVICEINFO pDevice);
static DWORD DeviceReadEpInfoFromFile(FILE* pFile, PENDPOINTINFO pEndpoint);
static DWORD DeviceReadClusterDataFromFile(FILE* pFile, PCLUSTERINFO pCluster);
static VOID DeviceWriteInfoToFile(FILE* pFile, PDEVICEINFO pDevice);
static VOID DeviceWriteEpInfoToFile(FILE* pFile, PENDPOINTINFO pEndpoint);
static VOID DeviceWriteClusterDataToFile(FILE *pFile, PCLUSTERINFO pCluster);
static VOID DeviceListAssoDevice();
static VOID DeviceInitClusterData(PCLUSTERINFO pCluster);
static VOID DeviceReqClusterInfo(WORD nNwkAddr, BYTE nEndpoint, WORD nCluster);
static VOID DeviceGetInfoAndConfig(PWORD pNwkAddr);


/* Function: DeviceListInit()
 * Description: Initialize device list
 * Input: None.
 * Return: None.
 */
VOID DeviceListInit()
{
	DWORD nFileSize = 0;
	FILE* fDeviceList;
	PDEVICEINFO pDevice;
	PDEVICEINFO pLastDeviceRead = NULL;
	printf("Init Device List\n");
	/* Open device list file */
	fDeviceList = fopen(FileName, "r");
	// file not exits
	if (fDeviceList == NULL)
	{
		pFirstDevice = NULL;
		return;
	}
	fseek(fDeviceList, 0L, SEEK_END);
	nFileSize = ftell(fDeviceList);
	fseek(fDeviceList, 0L, SEEK_SET);
	if (nFileSize == 0)
	{
		pFirstDevice = NULL;
		fclose(fDeviceList);
		return;
	}
	while(nFileSize > 0)
	{
		// Read data from file
		pDevice = malloc(sizeof(DEVICEINFO));
		if (pLastDeviceRead != NULL)
			pLastDeviceRead->NextDevice = pDevice;
		nFileSize -= DeviceReadInfoFromFile(fDeviceList, pDevice);
		pDevice->nDeviceTimeOut = DEFAULT_DEVICE_TIMEOUT;
		if(pFirstDevice == NULL) pFirstDevice = pDevice;
		pLastDeviceRead = pDevice;
		nDeviceCount++;
	}
	fclose(fDeviceList);
	DeviceListAssoDevice();
	DeviceDescInit(pFirstDevice);
}

/* Function: DeviceListWriteToFile()
 * Description: Store devices list to file.
 * Input: None.
 * Return: None.
 */
static VOID DeviceListWriteToFile()
{
	FILE* fDeviceList;
	PDEVICEINFO pDevice = pFirstDevice;
	// check if old file already exists
	fDeviceList = fopen(FileName, "r");
	if (fDeviceList != NULL)
	{
		fclose(fDeviceList);
		unlink(FileName);
	}
	fDeviceList = fopen(FileName, "w");
	while (pDevice != NULL)
	{
		DeviceWriteInfoToFile(fDeviceList, pDevice);
		pDevice = pDevice->NextDevice;
	}
	fclose(fDeviceList);
	DevDesUpdateFromDeviceInfo(pFirstDevice);
}

/* Function: DeviceReadInfoFromFile(FILE *pFile, PDEVICEINFO pDevice)
 * Description: Read device's description from file
 * Input:
 *	- pFile: File stream to read data from.
 *	- pDevice: Pointer to device info data struct.
 * Return: None.
 */
static DWORD DeviceReadInfoFromFile(FILE* pFile, PDEVICEINFO pDevice)
{
	DWORD nReadSize = 0;
	BYTE nEpIndex = 0;
	//read device info
	nReadSize += fread((PVOID)pDevice, sizeof(DEVICEINFO), 1, pFile) * sizeof(DEVICEINFO);

	pDevice->pEndpointInfoList = (PENDPOINTINFO)malloc(sizeof(ENDPOINTINFO) * pDevice->nNumberActiveEndpoint);
	// read Endpoint info from file
	for (nEpIndex = 0; nEpIndex < pDevice->nNumberActiveEndpoint; nEpIndex++)
	{
		// Read EpInfo list from file)
		nReadSize += DeviceReadEpInfoFromFile(pFile, &pDevice->pEndpointInfoList[nEpIndex]);
	}
	pDevice->NextDevice = NULL;
	return nReadSize;
}

/* Function: DeviceReadEpInfoFromFile(FILE *pFile, PENDPOINTINFO pEndpoint)
 * Description: Read device description from file
 * Input:
 *	- pFile: File stream to read data from.
 *	- pEndpoint: Pointer to end point info data struct.
 * Return:
 * 	- Number of byte read
 */
static DWORD DeviceReadEpInfoFromFile(FILE* pFile, PENDPOINTINFO pEndpoint)
{
	DWORD nReadSize= 0;
	BYTE nClIndex = 0;
	nReadSize += fread((PVOID)pEndpoint, sizeof(ENDPOINTINFO), 1, pFile) * sizeof(ENDPOINTINFO);
	pEndpoint->pInClusterList = NULL;
	pEndpoint->pOutClusterList = NULL;
	pEndpoint->pInClusterList = (PCLUSTERINFO)malloc(sizeof(CLUSTERINFO) * (pEndpoint->nInCluster + pEndpoint->nOutCluster));
	if (pEndpoint->nOutCluster > 0)
		pEndpoint->pOutClusterList = &pEndpoint->pInClusterList[pEndpoint->nInCluster];
	for (nClIndex = 0; nClIndex < pEndpoint->nInCluster + pEndpoint->nOutCluster; nClIndex++)
	{
		//Read cluster data from file
		nReadSize += DeviceReadClusterDataFromFile(pFile, &pEndpoint->pInClusterList[nClIndex]);
	}
	return nReadSize;
}

/* Function: DeviceReadClusterDataFromFile(FILE *pFile, PCLUSTERINFO pCluster)
 * Description: Read end point description from file
 * Input:
 *	- pFile: File stream to read data from.
 *	- pCluster: Pointer to cluster info data struct.
 * Return:
 * 	- Number of byte read
 */
static DWORD DeviceReadClusterDataFromFile(FILE* pFile, PCLUSTERINFO pCluster)
{
	DWORD nReadSize = 0;
	nReadSize += fread((PVOID)pCluster, sizeof(CLUSTERINFO), 1, pFile) * sizeof(CLUSTERINFO);
	pCluster->pData = NULL;
	if (pCluster->nDataSize > 0)
	{
		pCluster->pData = (PBYTE)malloc(pCluster->nDataSize);
		nReadSize += fread((PVOID)pCluster->pData, sizeof(BYTE), pCluster->nDataSize, pFile);
	}
	return nReadSize;
}

/* Function: DeviceWriteInfoToFile(FILE *pFile, PDEVICEINFO pDevice)
 * Description: Write device description from file
 * Input:
 *	- pFile: File stream to read data from.
 *	- pDevice: Pointer to device info data struct.
 * Return:
 * 	- Number of byte read
 */
static VOID DeviceWriteInfoToFile(FILE* pFile, PDEVICEINFO pDevice)
{
	BYTE nEpIndex;
	fwrite((PVOID)pDevice, sizeof(DEVICEINFO), 1, pFile);
	for (nEpIndex = 0; nEpIndex < pDevice->nNumberActiveEndpoint; nEpIndex++)
	{
		DeviceWriteEpInfoToFile(pFile, &(pDevice->pEndpointInfoList[nEpIndex]));
	}
}

/* Function: DeviceWriteEpInfoToFile(FILE *pFile, PENDPOINTINFO pEndpoint)
 * Description: Write end point description from file
 * Input:
 *	- pFile: File stream to read data from.
 *	- pEndpoint: Pointer to end point info data struct.
 * Return: None.
 */
static VOID DeviceWriteEpInfoToFile(FILE* pFile, PENDPOINTINFO pEndpoint)
{
	BYTE nClIndex;
	fwrite((PVOID)pEndpoint, sizeof(ENDPOINTINFO), 1, pFile);
	for (nClIndex = 0; nClIndex < (pEndpoint->nInCluster + pEndpoint->nOutCluster); nClIndex++)
	{
		DeviceWriteClusterDataToFile(pFile, &pEndpoint->pInClusterList[nClIndex]);
	}
}

/* Function: DeviceWriteClusterInfoToFile(FILE *pFile, PCLUSTERINFO pCluster)
 * Description: Write cluster description from file
 * Input:
 *	- pFile: File stream to read data from.
 *	- pCluster: Pointer to cluster info data struct.
 * Return: None.
 */
static VOID DeviceWriteClusterDataToFile(FILE* pFile, PCLUSTERINFO pCluster)
{
	fwrite((PVOID)pCluster, sizeof(CLUSTERINFO), 1, pFile);
	if((pCluster->nDataSize > 0) && (pCluster->pData != NULL))
		fwrite((PVOID)pCluster->pData, sizeof(BYTE), pCluster->nDataSize, pFile);
}

/* Function: DeviceWaitInformed(WORK nNwkAddr, WORD nCommand)
 * Description: Waiting for a command from a network device.
 * Input:
 *	- nNwkAddr: Short address of the network device
 *	- nCommand: Command to wait.
 * Return:
 * 	- 0 if success
 * 	- 1 if time out
 */
BYTE DeviceWaitInformed(WORD nNwkAddr, WORD nCommand, BYTE nTransID)
{
	WORD nTimeout = 30000;
	PDEVICEINFO pDevice = DeviceFind(nNwkAddr);
	char* LogString;
	while((stDeviceInformance.nNwkAddr != nNwkAddr) || (stDeviceInformance.nCommand != nCommand)
		  || (stDeviceInformance.nTransID != nTransID))
	{
		usleep(1000);
		nTimeout--;
		if (nTimeout == 0)
		{
			LogString = (char*)malloc(500);
			sprintf(LogString, "No response from device 0x%04X for transID %d", pDevice->nNetworkAddress, nTransID);
			LogWrite(LogString);
			FLUENT_LOGGER_INFO(LogString);
			free(LogString);
			return 1;
		}
	}
	stDeviceInformance.nNwkAddr = 0;
	return 0;
}

/* Function: DeviceSetInformed(WORK nNwkAddr, WORD nCommand)
 * Description: Set the program to wait information from a network device
 * Input:
 *	- nNwkAddr: Short address of the network device
 *	- nCommand: Command to wait.
 * Return: None
 */
VOID DeviceSetInformed(WORD nNwkAddr, WORD nCommand, BYTE nTransID)
{
	stDeviceInformance.nNwkAddr = nNwkAddr;
	stDeviceInformance.nCommand = nCommand;
	stDeviceInformance.nTransID = nTransID;
}


/* Function: DeviceAdd(PZDOANNCEINFO pDeviceInfo)
 * Description: Add a device to list
 * Input:
 *	- pDeviceInfo: Pointer to device announcement data
 * Return: None
 */
VOID DeviceAdd(PZDOANNCEINFO pDeviceInfo)
{
	PDEVICEINFO pNewDevice = NULL;
	PDEVICEINFO pLastDevice = pFirstDevice;
	PWORD pNwkAddr = (PWORD)malloc(sizeof(WORD));
	*pNwkAddr = pDeviceInfo->nNwkAddr;
	pthread_t NewDvConfigThr;
	char* LogString = (char*)calloc(100, sizeof(BYTE));
	PWORD pIeee = (PWORD)&(pDeviceInfo->IeeeAddr);
	if (DeviceFind(pDeviceInfo->nNwkAddr) != NULL)
	{
		printf("Device already in list \n");
		// ra soat lai viec tao thread trong truong hop ho tro thiet bi Xiaomi
		pthread_create(&NewDvConfigThr, NULL, (void*)&DeviceGetInfoAndConfig, (void*)(pNwkAddr));
		pthread_detach(NewDvConfigThr);
		DeviceListAssoDevice();
		return;
	}
	printf("Add new device address 0x%04X\n", pDeviceInfo->nNwkAddr);
	pNewDevice = (PDEVICEINFO)calloc(1, sizeof(DEVICEINFO));
	pNewDevice->IeeeAddr = pDeviceInfo->IeeeAddr;
	pNewDevice->nNetworkAddress = pDeviceInfo->nNwkAddr;
	pNewDevice->nDeviceTimeOut = DEFAULT_DEVICE_TIMEOUT;
	pNewDevice->nNumberActiveEndpoint = 0;
	pNewDevice->pEndpointInfoList = NULL;
	pNewDevice->NextDevice = NULL;
	if (pFirstDevice == NULL)
	{
		pFirstDevice = pNewDevice;
		nDeviceCount++;
		pthread_create(&NewDvConfigThr, NULL, (void*)&DeviceGetInfoAndConfig, (void*)(pNwkAddr));
		pthread_detach(NewDvConfigThr);
		sprintf(LogString, "Device online 0x%04X, IEEE Address: 0x%04X%04X%04X%04X",
				pDeviceInfo->nNwkAddr, pIeee[3], pIeee[2], pIeee[1], pIeee[0]);
		LogWrite(LogString);
		FLUENT_LOGGER_INFO(LogString);
		DeviceListAssoDevice();
		free(LogString);
		return;
	}
	pLastDevice = pFirstDevice;
	while (pLastDevice->NextDevice != NULL)
	{
		pLastDevice = pLastDevice->NextDevice;
	}
	pLastDevice->NextDevice = pNewDevice;
	// new device added to list, get info
	nDeviceCount++;
	pthread_create(&NewDvConfigThr, NULL, (void*)&DeviceGetInfoAndConfig, (void*)(pNwkAddr));
	pthread_detach(NewDvConfigThr);
	sprintf(LogString, "Device online 0x%04X, IEEE Address: 0x%04X%04X%04X%04X",
			pDeviceInfo->nNwkAddr, pIeee[3], pIeee[2], pIeee[1], pIeee[0]);
	FLUENT_LOGGER_INFO(LogString);
	LogWrite(LogString);
	free(LogString);
	DeviceListAssoDevice();
}

/* Function: DeviceRemove(WORD nNwkAddr)
 * Description: remove a device from list
 * Input:
 *	- nNwkAddr: short address of network device
 * Return: None
 */
VOID DeviceRemove(WORD nNwkAddr)
{
	PDEVICEINFO pCurrentDevice = pFirstDevice;
	PDEVICEINFO pRemovedDevice;
	BYTE nEpIndex;
	BYTE nClusterIndex;
	char* LogString = (char*)malloc(100);
	sprintf(LogString, "Device 0x%04X removed", nNwkAddr);
	LogWrite(LogString);
	FLUENT_LOGGER_INFO(LogString);
	//MqttClientPublishMessage(LogString);
	free(LogString);
	if (pCurrentDevice == NULL) return;
	if (pCurrentDevice->nNetworkAddress == nNwkAddr)
	{
		pRemovedDevice = pCurrentDevice;
		pFirstDevice = pCurrentDevice->NextDevice;
		for (nEpIndex = 0; nEpIndex < pRemovedDevice->nNumberActiveEndpoint; nEpIndex++)
		{
			for (nClusterIndex = 0; nClusterIndex < (pRemovedDevice->pEndpointInfoList[nEpIndex].nInCluster + pRemovedDevice->pEndpointInfoList[nEpIndex].nOutCluster); nClusterIndex++)
			{
				if (pRemovedDevice->pEndpointInfoList[nEpIndex].pInClusterList[nClusterIndex].pData != NULL)
					free(pRemovedDevice->pEndpointInfoList[nEpIndex].pInClusterList[nClusterIndex].pData);
			}
			if (pRemovedDevice->pEndpointInfoList[nEpIndex].pInClusterList != NULL)
				free(pRemovedDevice->pEndpointInfoList[nEpIndex].pInClusterList);
			//DevDescRemove(pRemovedDevice->nNetworkAddress, pRemovedDevice->pEndpointInfoList[nEpIndex].nEndPoint);
		}
		if (pRemovedDevice->pEndpointInfoList != NULL)
			free(pRemovedDevice->pEndpointInfoList);
		free(pRemovedDevice);
		nDeviceCount--;
		DeviceListWriteToFile();
		DeviceListAssoDevice();
		return;
	}
	while (pCurrentDevice->NextDevice != NULL)
	{
		pRemovedDevice = pCurrentDevice->NextDevice;
		if (pRemovedDevice->nNetworkAddress == nNwkAddr)
		{
			pCurrentDevice->NextDevice = pRemovedDevice->NextDevice;
			for (nEpIndex = 0; nEpIndex < pRemovedDevice->nNumberActiveEndpoint; nEpIndex++)
			{
				for (nClusterIndex = 0; nClusterIndex < (pRemovedDevice->pEndpointInfoList[nEpIndex].nInCluster + pRemovedDevice->pEndpointInfoList[nEpIndex].nOutCluster); nClusterIndex++)
				{
					if (pRemovedDevice->pEndpointInfoList[nEpIndex].pInClusterList[nClusterIndex].pData != NULL)
						free(pRemovedDevice->pEndpointInfoList[nEpIndex].pInClusterList[nClusterIndex].pData);
				}
				if (pRemovedDevice->pEndpointInfoList[nEpIndex].pInClusterList != NULL)
					free(pRemovedDevice->pEndpointInfoList[nEpIndex].pInClusterList);
			}
			if(pRemovedDevice->pEndpointInfoList != NULL)
				free(pRemovedDevice->pEndpointInfoList);
			free(pRemovedDevice);
			nDeviceCount--;
			DeviceListWriteToFile();
			DeviceListAssoDevice();
			return;
		}
		pCurrentDevice = pRemovedDevice;
	}
}

/* Function: DeviceFind(WORD nNwkAddr)
 * Description: Find a device description in list
 * Input:
 *	- nNwkAddr: short address of network device
 * Return:
 * 	- Pointer to device description. NULL if nNwkAddr not valid
 */
PDEVICEINFO DeviceFind(WORD nNwkAddr)
{
	PDEVICEINFO pDevice = pFirstDevice;
	while (pDevice != NULL)
	{
		if (pDevice->nNetworkAddress == nNwkAddr)
		{
			return pDevice;
		}
		pDevice = pDevice->NextDevice;
	}
	return NULL;
}

/* Function: DeviceListAssoDevice()
 * Description: List all associated devices.
 * Input: None.
 * Return: None.
 */
static VOID DeviceListAssoDevice()
{
	PDEVICEINFO pDevice = pFirstDevice;
	if (pDevice == NULL)
	{
		printf("no device in list\n");
		return;
	}
	printf("Number associated device: %d\n", nDeviceCount);
	printf("Associated devices: ");
	while(pDevice != NULL)
	{
		printf("0x%04X ", pDevice->nNetworkAddress);
		pDevice = pDevice->NextDevice;
	}
	printf ("\n");
}

VOID DeviceUpdateIeeeAddr(WORD nAddress, IEEEADDRESS IeeeAddr)
{
	PDEVICEINFO pDevice = DeviceFind(nAddress);
	if (pDevice == NULL)
		return;
	else
	{
		printf("Update Ieee for device 0x%04X\n", nAddress);
		pDevice->IeeeAddr = IeeeAddr;
	}
}

/* Function: DeviceUpdateDeviceInfo(WORD nNwkAddr, BYTE nNumEp, PBYTE pEndpointList)
 * Description: Update Endpoint info
 * Input:
 *	- nNwkAddr: Short address of network device.
 *	- nNumEp: Number of active end point.
 *	- pEndpointList: Pointer to end point data
 * Return:
 * 	- Pointer to device description. NULL if nNwkAddr not valid
 */
VOID DeviceUpdateDeviceInfo(WORD nNwkAddr, BYTE nNumEp, PBYTE pEndpointList)
{
	// cho nay nen xay dung co che de check lai xem thong tin cu da ok chua, neu thong tin cu da ok thi khong can remove
	// bug: 10/07/2016
	PDEVICEINFO pDevice = DeviceFind(nNwkAddr);
	PENDPOINTINFO pEpInfo = NULL;
	BYTE nEpIndex, nClusterIndex;
	// fix 11/07/2016, only free if there is different in number of endpoint
	if (pDevice->nNumberActiveEndpoint == nNumEp)
	{
		return;
	}
	// check if end point information has been in list than free to update new end point information
	if (pDevice != NULL)
	{
		if (pDevice->pEndpointInfoList != NULL)
		{
			printf("Remove EP Data\n");

			//free all cluster list info
			for (nEpIndex = 0; nEpIndex < pDevice->nNumberActiveEndpoint; nEpIndex++)
			{
				// free all the cluster data in end point
				for (nClusterIndex = 0; nClusterIndex < (pDevice->pEndpointInfoList[nEpIndex].nInCluster + pDevice->pEndpointInfoList[nEpIndex].nOutCluster); nClusterIndex++)
				{
					if (pDevice->pEndpointInfoList[nEpIndex].pInClusterList[nClusterIndex].pData != NULL)
						free(pDevice->pEndpointInfoList[nEpIndex].pInClusterList[nClusterIndex].pData);
				}
				if (pDevice->pEndpointInfoList[nEpIndex].pInClusterList != NULL)
					free(pDevice->pEndpointInfoList[nEpIndex].pInClusterList);
			}
			//free old end point info list
			if (pDevice->pEndpointInfoList != NULL)
				free(pDevice->pEndpointInfoList);
			pDevice->pEndpointInfoList = NULL;
			printf("EP data removed\n");
		}
	}
	if (nNumEp > 0)
	{
		printf("Adding end point list\n");
		//create end point information list
		pEpInfo = (PENDPOINTINFO)malloc(sizeof(ENDPOINTINFO) * nNumEp);
		for (nEpIndex = 0; nEpIndex < nNumEp; nEpIndex++)
		{
			pEpInfo[nEpIndex].nEndPoint = pEndpointList[nEpIndex];
			pEpInfo[nEpIndex].nDeviceID = 0xFFFF;
			pEpInfo[nEpIndex].nProfileID = 0xFFFF;
			pEpInfo[nEpIndex].nInCluster = 0;
			pEpInfo[nEpIndex].nOutCluster = 0;
			pEpInfo[nEpIndex].pInClusterList = NULL;
			pEpInfo[nEpIndex].pOutClusterList = NULL;
			pEpInfo[nEpIndex].nDeviceType = 0xFFFF;
			pEpInfo[nEpIndex].nDeviceVersion = 0;
		}
	}
	pDevice->nNumberActiveEndpoint = nNumEp;
	pDevice->pEndpointInfoList = pEpInfo;
	printf("update endpoint finished\n");
}

/* Function: DeviceFindEpInfo(WORD nNwkAddr, BYTE nEndpoint)
 * Description: Find an end point
 * Input:
 *	- nNwkAddr: Short address of network device.
 *	- nEndpoint: End point to find.
 * Return:
 * 	- Pointer to end point description. NULL if not valid
 */
PENDPOINTINFO DeviceFindEpInfo(WORD nNwkAddr, BYTE nEndpoint)
{
	PDEVICEINFO pDevice = DeviceFind(nNwkAddr);
	if (pDevice == NULL) return NULL;
	PENDPOINTINFO pEndpoint = pDevice->pEndpointInfoList;
	BYTE nEpIndex;
	if ((pDevice == NULL) || (pEndpoint == NULL))
		return NULL;
	for (nEpIndex = 0; nEpIndex < pDevice->nNumberActiveEndpoint; nEpIndex++)
	{
		if (pDevice->pEndpointInfoList[nEpIndex].nEndPoint == nEndpoint)
			return (&(pDevice->pEndpointInfoList[nEpIndex]));
	}
	return NULL;
}


/* Function: DeviceUpdateEndpointDesc(PZDOSIMPLEDESCRSP pSimpleDescInfo)
 * Description: Update end point info to list
 * Input:
 *	- pSimpleDescInfo: Pointer to simple description.
 * Return: None
 */
VOID DeviceUpdateEndpointDesc(PZDOSIMPLEDESCRSP pSimpleDescInfo)
{
	PDEVICEINFO pDeviceInfo = DeviceFind(pSimpleDescInfo->nNwkAddr);
	BYTE nEpIndex;
	BYTE nIndex;
	BYTE nNumberInCluster;
	BYTE nNumberOutCluster;
	PWORD pClusterList;
	if (pDeviceInfo == NULL) return;
	if (pDeviceInfo->nNumberActiveEndpoint == 0) return;
	nEpIndex = 0;
	while((pDeviceInfo->pEndpointInfoList[nEpIndex].nEndPoint != pSimpleDescInfo->nEndpoint)
		&& (nEpIndex < pDeviceInfo->nNumberActiveEndpoint))
	{
		nEpIndex++;
	}
	if (nEpIndex == pDeviceInfo->nNumberActiveEndpoint)
	{
		printf("Endpoint not found");
		return;
	}
	pDeviceInfo->pEndpointInfoList[nEpIndex].nDeviceID = pSimpleDescInfo->nDeviceId;
	pDeviceInfo->pEndpointInfoList[nEpIndex].nDeviceVersion = pSimpleDescInfo->nDevVer;
	pDeviceInfo->pEndpointInfoList[nEpIndex].nProfileID = pSimpleDescInfo->nProfileId;
	nNumberInCluster = pSimpleDescInfo->pData[0];
	nNumberOutCluster = pSimpleDescInfo->pData[1 + nNumberInCluster * sizeof(WORD)];
	pDeviceInfo->pEndpointInfoList[nEpIndex].nInCluster = nNumberInCluster;
	pDeviceInfo->pEndpointInfoList[nEpIndex].nOutCluster = nNumberOutCluster;
	pDeviceInfo->pEndpointInfoList[nEpIndex].pInClusterList = (PCLUSTERINFO)(malloc(sizeof(CLUSTERINFO) * (nNumberInCluster + nNumberOutCluster)));
	if (nNumberOutCluster > 0)
	{
		pDeviceInfo->pEndpointInfoList[nEpIndex].pOutClusterList = &(pDeviceInfo->pEndpointInfoList[nEpIndex].pInClusterList[nNumberInCluster]);
	}
	pClusterList = (PWORD)(pSimpleDescInfo->pData + 1);
	for (nIndex = 0; nIndex < nNumberInCluster; nIndex++)
	{
		pDeviceInfo->pEndpointInfoList[nEpIndex].pInClusterList[nIndex].nCluster = pClusterList[nIndex];
		DeviceInitClusterData(&(pDeviceInfo->pEndpointInfoList[nEpIndex].pInClusterList[nIndex]));
	}
	pClusterList = (PWORD)(pSimpleDescInfo->pData + 2 + nNumberInCluster * sizeof(WORD));
	for (nIndex = 0; nIndex < nNumberOutCluster; nIndex++)
	{
		pDeviceInfo->pEndpointInfoList[nEpIndex].pOutClusterList[nIndex].nCluster = pClusterList[nIndex];
		DeviceInitClusterData(&(pDeviceInfo->pEndpointInfoList[nEpIndex].pOutClusterList[nIndex]));
	}
	/* Add description to file */
	//PDEVICEDESC pDevDesc = (PDEVICEDESC)malloc(sizeof(DEVICEDESC));
	//pDevDesc->nNwkAddr = pSimpleDescInfo->nNwkAddr;
	//pDevDesc->nDevId = pSimpleDescInfo->nDeviceId;
	//pDevDesc->nEp = pSimpleDescInfo->nEndpoint;
	//pDevDesc->nDeviceType = 0;
	//DevDescAdd(pDevDesc);
	//free(pDevDesc);
	printf("Simple Desc update finish \n");
}


/* Function: DeviceInitClusterData(PCLUSTERINFO pCluster)
 * Description: Init Cluster data
 * Input:
 *	- pCluster: Pointer to cluster description.
 * Return: None
 */
static VOID DeviceInitClusterData(PCLUSTERINFO pCluster)
{
	switch (pCluster->nCluster)
	{
	case ZCL_CLUSTER_ID_GEN_POWER_CFG:
		pCluster->pData = malloc(sizeof(ZCLPWRCFGATTR));
		pCluster->nDataSize = sizeof(ZCLPWRCFGATTR);
		ZclGenPwrCfgAttrInit((PZCLPWRCFGATTR)pCluster->pData);
		break;
	case ZCL_CLUSTER_ID_GEN_ON_OFF:
		pCluster->pData = malloc(sizeof(ZCLGENONOFFATTR));
		pCluster->nDataSize = sizeof(ZCLGENONOFFATTR);
		ZclGenOnOffAttrInit((PZCLGENONOFFATTR)pCluster->pData);
		break;
	case ZCL_CLUSTER_ID_GEN_ALARMS:
		pCluster->pData = malloc(sizeof(ZLCGENALARMATTR));
		pCluster->nDataSize = sizeof(ZLCGENALARMATTR);
		ZclGenAlarmAttrInit((PZCLGENALARMATTR)pCluster->pData);
		break;
	case ZCL_CLUSTER_ID_SS_IAS_ZONE:
		pCluster->pData = malloc(sizeof(ZCLIASZONEATTR));;
		pCluster->nDataSize = sizeof(ZCLIASZONEATTR);
		ZclIasZoneAttrInit((PZCLIASZONEATTR)pCluster->pData);
		break;
	case ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT:
		pCluster->pData = malloc(sizeof(ZCLSSTEMPATTR));
		pCluster->nDataSize = sizeof(ZCLSSTEMPATTR);
		ZclSsTempAttrInit((PZCLSSTEMPATTR)pCluster->pData);
		break;
	case ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY:
		pCluster->pData = malloc(sizeof(ZCLSSREHUMIDATTR));
		pCluster->nDataSize = sizeof(ZCLSSREHUMIDATTR);
		ZclSsReHumidAttrInit((PZCLSSREHUMIDATTR)pCluster->pData);
		break;
	case ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING:
		pCluster->pData = malloc(sizeof(ZCLSSOCCUPANCYATTR));
		pCluster->nDataSize = sizeof(ZCLSSOCCUPANCYATTR);
		ZclSsOccuAttrInit((PZCLSSOCCUPANCYATTR)pCluster->pData);
		break;
	default:
		pCluster->nDataSize = 0;
		pCluster->pData = NULL;
		break;
	}
}

/* Function: DeviceReqClusterInfo(WORD nNwkAddr, BYTE nEndpoint, WORD nCluster)
 * Description: Request for Cluster data
 * Input:
 *	- nNwkAddr: Short address of device to request.
 *	- nEndpoint: End pont to request.
 *	- nCluster: Cluster ID to request
 * Return: None
 */
static VOID DeviceReqClusterInfo(WORD nNwkAddr, BYTE nEndpoint, WORD nCluster)
{
	BYTE nTransID = ZclGetClusterAttr(nNwkAddr, nEndpoint, nCluster);
	if ( nTransID != 0xFF)
	{
		DeviceWaitInformed(nNwkAddr, AF_INCOMING_MSG, nTransID);
	}
}

/* Function: DeviceFindClusterInfo(WORD nNwkAddr, BYTE nEndpoint, WORD nCluster)
 * Description: Request for Cluster data
 * Input:
 *	- nNwkAddr: Short address of device to request.
 *	- nEndpoint: End pont to request.
 *	- nClusterID: Cluster ID to request
 * Return:
 * 	- Pointer to cluster data
 */
PCLUSTERINFO DeviceFindClusterInfo(WORD nNwkAddr, BYTE nEndpoint, WORD nClusterId)
{
	PENDPOINTINFO pEp = DeviceFindEpInfo(nNwkAddr, nEndpoint);
	PCLUSTERINFO pCluster = NULL;
	BYTE nClusterIndex;
	if ((pEp == NULL) || (pEp->pInClusterList == NULL))
		return NULL;
	for (nClusterIndex = 0; nClusterIndex < (pEp->nInCluster + pEp->nOutCluster); nClusterIndex++)
	{
		if ((pEp->pInClusterList[nClusterIndex]).nCluster == nClusterId)
		{
			pCluster = &(pEp->pInClusterList[nClusterIndex]);
			return pCluster;
		}
	}
	return pCluster;
}


/* Function: DeviceConfigCluster(WORD nNwkAddr, BYTE nEndpoint, WORD nCluster)
 * Description: Configure a cluster to work probably
 * Input:
 *	- nNwkAddr: Short address of device to request.
 *	- nEndpoint: End pont to request.
 *	- nClusterID: Cluster ID to request
 * Return: None
 */
static VOID DeviceConfigCluster(WORD nNwkAddr, BYTE nEndpoint, WORD nCluster)
{
	BYTE nTransID = ZclConfigCluster(nNwkAddr, nEndpoint, nCluster);
	if (nTransID != 0xFF)
	{
		DeviceWaitInformed(nNwkAddr, AF_INCOMING_MSG, nTransID);
		// can nhac xu ly khi timeout
	}
}

/* Function: DeviceConfigDevicetype(WORD nNwkAddr, BYTE nEndpoint)
 * Description:
 * 	- Get device type of an endpoint
 * Input:
 *	- nNwkAddr: Network address of device
 *	- nEndpoint: Endpoint to be configured
 * Return: None.
 */
static VOID DeviceConfigDeviceType(WORD nNwkAddr, BYTE nEndpoint)
{
	BYTE nClusterIndex;;
	PENDPOINTINFO pEndpoint = DeviceFindEpInfo(nNwkAddr, nEndpoint);
	PCLUSTERINFO pCluster;
	if (pEndpoint == NULL)
		return;
	switch (pEndpoint->nDeviceID)
	{
	case DEVICE_ID_ON_OFF_SWITCH:
		pEndpoint->nDeviceType = DEVICE_ID_ON_OFF_SWITCH;
		break;
	case DEVICE_ID_LEVEL_CTR_SWITCH:
		pEndpoint->nDeviceType = DEVICE_ID_LEVEL_CTR_SWITCH;
		break;
	case DEVICE_ID_ON_OFF_OUTPUT:
		pEndpoint->nDeviceType = DEVICE_ID_ON_OFF_OUTPUT;
		break;
	case DEVICE_ID_LEVEL_CONTROLLABLE_OUTPUT:
		pEndpoint->nDeviceType = DEVICE_ID_LEVEL_CONTROLLABLE_OUTPUT;
		break;
	case DEVICE_ID_SCENE_SELECCTOR:
		pEndpoint->nDeviceType = DEVICE_ID_SCENE_SELECCTOR;
		break;
	case DEVICE_ID_CONFIG_TOOL:
		pEndpoint->nDeviceType = DEVICE_ID_CONFIG_TOOL;
		break;
	case DEVICE_ID_REMOTE_CONTROL:
		pEndpoint->nDeviceType = DEVICE_ID_REMOTE_CONTROL;
		break;
	case DEVICE_ID_COMBINED_INTERFACE:
		pEndpoint->nDeviceType = DEVICE_ID_COMBINED_INTERFACE;
		break;
	case DEVICE_ID_RANGE_EXTENDER:
		pEndpoint->nDeviceType = DEVICE_ID_RANGE_EXTENDER;
		break;
	case DEVICE_ID_POWER_OUTLET:
		pEndpoint->nDeviceType = DEVICE_ID_POWER_OUTLET;
		break;
	case DEVICE_ID_DOOR_LOCK:
		pEndpoint->nDeviceType = DEVICE_ID_DOOR_LOCK;
		break;
	case DEVICE_ID_DOOR_LOCK_CONTROLLER:
		pEndpoint->nDeviceType = DEVICE_ID_DOOR_LOCK_CONTROLLER;
		break;
	case DEVICE_ID_SIMPLE_SENSOR:
		pEndpoint->nDeviceType = DEVICE_ID_SIMPLE_SENSOR;
		break;
	case DEVICE_ID_ON_OFF_LIGHT:
		pEndpoint->nDeviceType = DEVICE_ID_ON_OFF_LIGHT;
		break;
	case DEVICE_ID_DIMMABLE_LIGHT:
		pEndpoint->nDeviceType = DEVICE_ID_DIMMABLE_LIGHT;
		break;
	case DEVICE_ID_COLOR_DIMMABLE_LIGHT:
		pEndpoint->nDeviceType = DEVICE_ID_COLOR_DIMMABLE_LIGHT;
		break;
	case DEVICE_ID_ON_OFF_LIGHT_SW:
		pEndpoint->nDeviceType = DEVICE_ID_ON_OFF_LIGHT_SW;
		break;
	case DEVICE_ID_DIMMER_SWITCH:
		pEndpoint->nDeviceType = DEVICE_ID_DIMMER_SWITCH;
		break;
	case DEVICE_ID_COLOR_DIMMER_SWITCH:
		pEndpoint->nDeviceType = DEVICE_ID_COLOR_DIMMER_SWITCH;
		break;
	case DEVICE_ID_LIGHT_SENSOR:
		pEndpoint->nDeviceType = DEVICE_ID_LIGHT_SENSOR;
		break;
	case DEVICE_ID_OCCUPANCY_SENSOR:
		pEndpoint->nDeviceType = DEVICE_ID_OCCUPANCY_SENSOR;
		break;
	case DEVICE_ID_SHADE:
		pEndpoint->nDeviceType = DEVICE_ID_SHADE;
		break;
	case DEVICE_ID_SHADE_CONTROLLER:
		pEndpoint->nDeviceType = DEVICE_ID_SHADE_CONTROLLER;
		break;
	case DEVICE_ID_WINDOW_COVERING_DEVICE:
		pEndpoint->nDeviceType = DEVICE_ID_WINDOW_COVERING_DEVICE;
		break;
	case DEVICE_ID_WINDOW_COVERING_CONTROLLER:
		pEndpoint->nDeviceType = DEVICE_ID_WINDOW_COVERING_CONTROLLER;
		break;
	case DEVICE_ID_HEATING_COOLING_UNIT:
		pEndpoint->nDeviceType = DEVICE_ID_HEATING_COOLING_UNIT;
		break;
	case DEVICE_ID_THERMOSTAT:
		pEndpoint->nDeviceType = DEVICE_ID_THERMOSTAT;
		break;
	case DEVICE_ID_TEMPERATURE_SENSOR:
		for (nClusterIndex = 0; nClusterIndex < pEndpoint->nInCluster + pEndpoint->nOutCluster; nClusterIndex++)
		{
			if ((pEndpoint->pInClusterList[nClusterIndex].nCluster & 0x0400) == 0x0400)
			{
				pEndpoint->nDeviceType = pEndpoint->pInClusterList[nClusterIndex].nCluster;
				break;
			}
		}
		break;
	case DEVICE_ID_PUMP:
		pEndpoint->nDeviceType = DEVICE_ID_PUMP;
		break;
	case DEVICE_ID_PUMP_CONTROLLER:
		pEndpoint->nDeviceType = DEVICE_ID_PUMP_CONTROLLER;
		break;
	case DEVICE_ID_PRESSURE_SENSOR:
		pEndpoint->nDeviceType = DEVICE_ID_PRESSURE_SENSOR;
		break;
	case DEVICE_ID_FLOW_SENSOR:
		pEndpoint->nDeviceType = DEVICE_ID_FLOW_SENSOR;
		break;
	case DEVICE_ID_IAS_CIE:
		pEndpoint->nDeviceType = DEVICE_ID_IAS_CIE;
		break;
	case DEVICE_ID_IAS_ACE:
		pEndpoint->nDeviceType = DEVICE_ID_IAS_ACE;
		break;
	case DEVICE_ID_IAS_ZONE:
		pCluster = DeviceFindClusterInfo(nNwkAddr, nEndpoint, ZCL_CLUSTER_ID_SS_IAS_ZONE);
		if (pCluster != NULL)
		{
			pEndpoint->nDeviceType = ((PZCLIASZONEATTR)(pCluster->pData))->nZoneType;
		}
		break;
	case DEVICE_ID_IAS_WARNING_DEVICE:
		pEndpoint->nDeviceType = DEVICE_ID_IAS_WARNING_DEVICE;
		break;
	default:
		break;
	}
}

/* Function: DeviceGetInfoAndConfig(PWORD pNwkAddr)
 * Description:
 * 	- Get info and configure a device to work probably
 * 	- This function must not be called from serial received thread
 * Input:
 *	- pNwkAddr: Pointer to device address.
 * Return: None.
 */
static VOID DeviceGetInfoAndConfig(PWORD pNwkAddr)
{
	BYTE nStatus = 0;
	BYTE nEpIndex, nClusterIndex;
	PDEVICEINFO pDevInfo = DeviceFind(*pNwkAddr);
	BOOL addDeviceEvent = FALSE;
	if (pDevInfo->IeeeAddr == 0)
	{
		nStatus = ZnpZdoIeeeAddrReq(*pNwkAddr, 0, 0);
		nStatus = DeviceWaitInformed(pDevInfo->nNetworkAddress, ZDO_IEEE_ADDR_RSP, 0xFF);
		if (nStatus == 1)
			return;
	}

	// request for active endpoint list
	//if (pDevInfo->pEndpointInfoList == NULL)
	{
		printf("\e[1;31mGet number of active end point:\n\e[1;33m");
		nStatus = ZnpZdoActiveEpRequest(*pNwkAddr);
		nStatus = DeviceWaitInformed(pDevInfo->nNetworkAddress, ZDO_ACTIVE_EP_RSP, 0xFF);
	}
	if (nStatus == 1)
	{
		//DeviceRemove(*pNwkAddr);
		return;
	}
	// request for end point description
	printf("Get information of %d endpoint(s)\n", pDevInfo->nNumberActiveEndpoint);
	for (nEpIndex = 0; nEpIndex < pDevInfo->nNumberActiveEndpoint; nEpIndex++)
	{
		addDeviceEvent = FALSE;
		printf("\e[1;31mGetting information enpoint 0x%02X\n\e[1;32m", pDevInfo->pEndpointInfoList[nEpIndex].nEndPoint);
		// fix 11/07/2016: only get endpoint info again if needed
		if (pDevInfo->pEndpointInfoList[nEpIndex].pInClusterList == NULL)
		{
			addDeviceEvent = TRUE;
			nStatus = ZnpZdoSimpleDescReq(pDevInfo->nNetworkAddress, pDevInfo->pEndpointInfoList[nEpIndex].nEndPoint);
			nStatus = DeviceWaitInformed(pDevInfo->nNetworkAddress, ZDO_SIMPLE_DESC_RSP, 0xFF);
		}
		if (nStatus == 1)
		{
			char* loggerMessage = malloc(250);
			sprintf(loggerMessage, "No response from device 0x%04X, endpoint %d", *pNwkAddr, pDevInfo->pEndpointInfoList[nEpIndex].nEndPoint);
			FLUENT_LOGGER_WARN(loggerMessage);
			LogWrite(loggerMessage);
			free(loggerMessage);
			printf("\e[1;31mNo response, next\n\e[1;33m");
		}
		else
		{
			// Get cluster information
			for (nClusterIndex = 0; nClusterIndex < (pDevInfo->pEndpointInfoList[nEpIndex].nInCluster + pDevInfo->pEndpointInfoList[nEpIndex].nOutCluster); nClusterIndex++)
			{
				printf("Get cluster info 0x%04X\n", pDevInfo->pEndpointInfoList[nEpIndex].pInClusterList[nClusterIndex].nCluster);
				//Get cluster info
				DeviceReqClusterInfo(pDevInfo->nNetworkAddress, pDevInfo->pEndpointInfoList[nEpIndex].nEndPoint, pDevInfo->pEndpointInfoList[nEpIndex].pInClusterList[nClusterIndex].nCluster);
				//Config cluster
				printf("Main configure cluster\n");
				DeviceConfigCluster(pDevInfo->nNetworkAddress, pDevInfo->pEndpointInfoList[nEpIndex].nEndPoint, pDevInfo->pEndpointInfoList[nEpIndex].pInClusterList[nClusterIndex].nCluster);
				usleep(CLUSTER_GET_IDLE_TIME);
			}
			// Get Device type
			printf("\e[1;31mNumber of endpoint %d\n\e[1;32m", pDevInfo->nNumberActiveEndpoint);
			// viet them mot ham xu ly get device type
			DeviceConfigDeviceType(*pNwkAddr, pDevInfo->pEndpointInfoList[nEpIndex].nEndPoint);
			// publish device_added event
			// bug 11/07/2016: trong truong hop thay pi co the phat sinh goi tin add device khong can thiet
			/*
			if (addDeviceEvent == TRUE)
			{
				ZnpActorPublishDeviceAddedEvent(pDevInfo->IeeeAddr, pDevInfo->pEndpointInfoList[nEpIndex].nEndPoint,
						pDevInfo->pEndpointInfoList[nEpIndex].nDeviceID, pDevInfo->pEndpointInfoList[nEpIndex].nDeviceType);
			}
			ZnpActorPublishDeviceOnlineEvent(pDevInfo->IeeeAddr);
			*/
		}
	}
	/* 01/10/2016: publish all device info in one message */
	if (addDeviceEvent == TRUE)
	{
		ZnpActorPublishDeviceAddedEvent(pDevInfo->nNetworkAddress);
	}
	DeviceListWriteToFile();
	DeviceListAssoDevice();
	free(pNwkAddr);
}

/* Function: DeviceAddEndpoint(PENDPOINTADDR pEpAddr)
 * Description:
 * 	- Adding an endpoint to device
 * 	- This function must not be called from serial received thread
 * Input:
 *	- nNwkAddr: device's short address
 *	- nEp: endpoint to be added
 * Return: None.
 */
VOID DeviceAddEndpoint(PENDPOINTADDR pEpAddr)
{
	PDEVICEINFO pDevice = DeviceFind(pEpAddr->nNwkAddr);
	PENDPOINTINFO pEp = DeviceFindEpInfo(pEpAddr->nNwkAddr, pEpAddr->nEp);
	PENDPOINTINFO pNewEpList;
	BYTE nStatus;
	BYTE nClusterIndex;
	BYTE nEpIndex = 0;
	printf("Add endpoint 0x%02X to device 0x%04X\n", pEpAddr->nEp, pEpAddr->nNwkAddr);
	if (pDevice == NULL) return;
	if (pEp != NULL) return;
	nEpIndex = pDevice->nNumberActiveEndpoint;
	// adding end point to device info struct
	//pNewEpList = (PENDPOINTINFO)malloc(sizeof(ENDPOINTINFO) * (nEpIndex + 1));
	pNewEpList = (PENDPOINTINFO)calloc(nEpIndex + 1, sizeof(ENDPOINTINFO));
	// Copy old data to new endpoint info list
	CopyMemory((PBYTE)pNewEpList, (PBYTE)pDevice->pEndpointInfoList, sizeof(ENDPOINTINFO) * nEpIndex);
	pNewEpList[nEpIndex].nEndPoint = pEpAddr->nEp;
	pNewEpList[nEpIndex].nDeviceVersion = 0;
	pNewEpList[nEpIndex].nDeviceID = 0xFFFF;
	pNewEpList[nEpIndex].nProfileID = 0xFFFF;
	pNewEpList[nEpIndex].nInCluster = 0;
	pNewEpList[nEpIndex].nOutCluster = 0;
	pNewEpList[nEpIndex].nDeviceType = 0xFFFF;
	pNewEpList[nEpIndex].pInClusterList = NULL;
	pNewEpList[nEpIndex].pOutClusterList = NULL;
	pDevice->nNumberActiveEndpoint++;
	if (pDevice->pEndpointInfoList != NULL)
	{
		pEp = DeviceFindEpInfo(pEpAddr->nNwkAddr, pEpAddr->nEp);
		if (pEp == NULL)
			free(pDevice->pEndpointInfoList);
		else
		{
			free(pNewEpList);
			return;
		}
	}
	pDevice->pEndpointInfoList = pNewEpList;
	nStatus = ZnpZdoSimpleDescReq(pDevice->nNetworkAddress, pEpAddr->nEp);
	nStatus = DeviceWaitInformed(pDevice->nNetworkAddress, ZDO_SIMPLE_DESC_RSP, 0xFF);
	if (nStatus == 1)
	{
		return;
	}
	// Get cluster information
	for (nClusterIndex = 0; nClusterIndex < (pDevice->pEndpointInfoList[nEpIndex].nInCluster + pDevice->pEndpointInfoList[nEpIndex].nOutCluster); nClusterIndex++)
	{
		printf("Get and config cluster 0x%04x\n", pDevice->pEndpointInfoList[nEpIndex].pInClusterList[nClusterIndex].nCluster);
		//Get cluster info
		DeviceReqClusterInfo(pDevice->nNetworkAddress, pDevice->pEndpointInfoList[nEpIndex].nEndPoint, pDevice->pEndpointInfoList[nEpIndex].pInClusterList[nClusterIndex].nCluster);
		//Config cluster
		printf("Configure cluster %p\n", pDevice->pEndpointInfoList[nEpIndex].pInClusterList);
		if ((pDevice->pEndpointInfoList[nEpIndex].pInClusterList) != NULL)
		{
			DeviceConfigCluster(pDevice->nNetworkAddress, pDevice->pEndpointInfoList[nEpIndex].nEndPoint, pDevice->pEndpointInfoList[nEpIndex].pInClusterList[nClusterIndex].nCluster);
		}
		else
			return;
		usleep(CLUSTER_GET_IDLE_TIME);
	}
	DeviceConfigDeviceType(pEpAddr->nNwkAddr, pEpAddr->nEp);
	// publish device_added event
	pEp = DeviceFindEpInfo(pEpAddr->nNwkAddr, pEpAddr->nEp);
	ZnpActorPublishEndpointAddedEvent(pDevice->IeeeAddr, pEpAddr->nEp, pEp->nDeviceID, pEp->nDeviceType);
	ZnpActorPublishDeviceOnlineEvent(pDevice->IeeeAddr);
	DeviceListWriteToFile();
	DeviceListAssoDevice();
	printf("\e[1;31mAdd endpoint 0x%02X finished, total ep %d \n\e[1;32m", pEpAddr->nEp, pDevice->nNumberActiveEndpoint);
	free(pEpAddr);
}

/* Function: DeviceAddCluster(PCLUSTERADDR pClusterAddr)
 * Description:
 * 	- Adding a cluster to device
 * 	- This function must not be called from serial received thread
 * Input:
 *	- nNwkAddr: device short address
 *	- nEp: endpoint.
 *	- nCluster: ClusterID to be add.
 * Return: None.
 */
VOID DeviceAddCluster(PCLUSTERADDR pClusterAddr)
{
	PENDPOINTINFO pEp;
	PCLUSTERINFO pCluster;
	BYTE nClusterIndex = 0;
	PCLUSTERINFO pNewClusterInfoList;
	PENDPOINTADDR pEpAddr = (PENDPOINTADDR)pClusterAddr;
	printf("\e[1;31mStart adding cluster\n\e[1;33m");
	pEp = DeviceFindEpInfo(pClusterAddr->nNwkAddr, pClusterAddr->nEp);
	if (pEp == NULL)
	{
		DeviceAddEndpoint(pEpAddr);
	}
	printf("Endpoint exists, add cluster\n");
	pCluster = DeviceFindClusterInfo(pClusterAddr->nNwkAddr, pClusterAddr->nEp, pClusterAddr->nClusterID);
	if (pCluster != NULL)
	{
		printf("Cluster exist, return\n");
		return;
	}
	printf("Cluster does not exist, add\n");
	pEp = DeviceFindEpInfo(pClusterAddr->nNwkAddr, pClusterAddr->nEp);
	pCluster = DeviceFindClusterInfo(pClusterAddr->nNwkAddr, pClusterAddr->nEp, pClusterAddr->nClusterID);
	if (pCluster != NULL) return;
	if (pEp != DeviceFindEpInfo(pClusterAddr->nNwkAddr, pClusterAddr->nEp) || (pEp == NULL)) return;
	nClusterIndex = pEp->nInCluster + pEp->nOutCluster;
	pEp->nOutCluster++;
	//pNewClusterInfoList = (PCLUSTERINFO)malloc(sizeof(CLUSTERINFO) * (nClusterIndex + 1));
	pNewClusterInfoList = (PCLUSTERINFO)calloc(nClusterIndex + 1, sizeof(CLUSTERINFO));
	if (pEp != DeviceFindEpInfo(pClusterAddr->nNwkAddr, pClusterAddr->nEp))
	{
		free(pNewClusterInfoList);
		return;
	}

	CopyMemory((PBYTE)pNewClusterInfoList, (PBYTE)pEp->pInClusterList, sizeof(CLUSTERINFO) * nClusterIndex);
	pNewClusterInfoList[nClusterIndex].nCluster = pClusterAddr->nClusterID;
	pNewClusterInfoList[nClusterIndex].nDataSize = 0;
	pNewClusterInfoList[nClusterIndex].pData = NULL;
	if (pEp != DeviceFindEpInfo(pClusterAddr->nNwkAddr, pClusterAddr->nEp))
	{
		free(pNewClusterInfoList);
		return;
	}
	//free old list
	if (pEp->pInClusterList != NULL)
	{
		pCluster = DeviceFindClusterInfo(pClusterAddr->nNwkAddr, pClusterAddr->nEp, pClusterAddr->nClusterID);
		if (pCluster == NULL)
			free(pEp->pInClusterList);
		else
			return;
	}
	pEp->pInClusterList = pNewClusterInfoList;
	pEp->pOutClusterList = &(pNewClusterInfoList[pEp->nInCluster]);
	DeviceInitClusterData(&pNewClusterInfoList[nClusterIndex]);
	DeviceReqClusterInfo(pClusterAddr->nNwkAddr, pClusterAddr->nEp, pClusterAddr->nClusterID);
	printf("Configure added cluster\n");
	DeviceConfigCluster(pClusterAddr->nNwkAddr, pClusterAddr->nEp, pClusterAddr->nClusterID);
	DeviceConfigDeviceType(pClusterAddr->nNwkAddr, pClusterAddr->nEp);
	// publish device_added event
	PDEVICEINFO pDevice = DeviceFind(pClusterAddr->nNwkAddr);
	ZnpActorPublishEndpointAddedEvent(pDevice->IeeeAddr, pEp->nEndPoint, pEp->nDeviceID, pEp->nDeviceType);
	ZnpActorPublishDeviceOnlineEvent(pDevice->IeeeAddr);
	printf("Add cluster 0x%04X finished\n", pClusterAddr->nClusterID);
	DeviceListWriteToFile();
	DeviceListAssoDevice();
	free(pClusterAddr);
}

/* Function: DeviceProcessTimeout()
 * Description:
 * 	- Check timeout of a device, check if there's a device without communication for a long time or message timeout
 * Input:
 * Return: None.
 */
VOID DeviceProcessTimeout()
{
	//static WORD routingCycleCheck = 0;
	PDEVICEINFO pDevice = pFirstDevice;
	char* LogString;
	while (pDevice != NULL)
	{
		if (pDevice->nDeviceTimeOut == 120)
			ZnpZdoActiveEpRequest(pDevice->nNetworkAddress);
		if (pDevice->nDeviceTimeOut > 0)
		{
			pDevice->nDeviceTimeOut--;
			if (pDevice->nDeviceTimeOut == 0)
			{
				LogString = (char*)malloc(500);
				sprintf(LogString, "Device timeout 0x%04X:", pDevice->nNetworkAddress);
				ZnpActorPublishDeviceOfflineEvent(pDevice->IeeeAddr);
				FLUENT_LOGGER_INFO(LogString);
				LogWrite(LogString);
				free(LogString);
			}
		}
		pDevice = pDevice->NextDevice;
	}

	// get routing table and reset timeout if device active / set time out to 0 if device inactive
	/*
	if (routingCycleCheck < ROUTING_TABLE_CYCLE)
		routingCycleCheck++;
	else
	{
		routingCycleCheck = 0;
		//ZnpGetRtgTable();
		//ZnpZdoActiveEpRequest()
	}
	*/
}

/* Function: DeviceProcessTimeout()
 * Description:
 * 	- Set timeout time of a device to check if device response is valid
 * Input:
 * Return: None.
 */
VOID DeviceSetTimeoutTime(WORD nAddress, DWORD nTime)
{
	PDEVICEINFO pDevice = DeviceFind(nAddress);
	if (pDevice == NULL)
		return;
	if (pDevice->nDeviceTimeOut == 0)
		ZnpActorPublishDeviceOnlineEvent(pDevice->IeeeAddr);
	pDevice->nDeviceTimeOut = nTime;
}
/* Y tuong xet time out 1 goi tin
 * su dung mot danh sach lien ket
 * cac gia tri can luu la network id, va zcl transId , thoi gian timeout
 * thoi gian timeout tinh theo giaya.
 * khi nhan goi tin zcl thi su dung cac network id va transid de thuc hien xoa khoi danh sach
 * dung mot vong quet check timeout voi chu ky 1000ms
 */
