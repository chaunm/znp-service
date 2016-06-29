/*
 ============================================================================
 Name        : ZigbeeHost.c
 Author      : ChauNM
 Version     :
 Copyright   :
 Description : C Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include "serialcommunication.h"
#include "znp.h"
#include "ZnpCommandState.h"
#include "zcl.h"
#include "DeviceManager/DevicesManager.h"
#include "log.h"
#include "universal.h"
#include "ZnpActor.h"

void PrintHelpMenu() {
	printf("program: ZigbeeHostAMA\n"
			"using ./ZigbeeHostAMA --port [] --id [] --token []\n"
			"--port: Serial port used to communicate with ZNP device (ex.: ttyUSB0, ttyAMA0..)\n"
			"--id: guid of the znp actor\n"
			"--token: pasword to the broker of the znp actor, this option can be omitted\n");
}

int main(int argc, char* argv[])
{

	pthread_t SerialProcessThread;
	pthread_t SerialOutputThread;
	pthread_t SerialHandleThread;
	//pthread_t DemoActorThread;
	PSERIAL	pSerialPort;
	BOOL bResult = FALSE;
	BYTE nRetry = 0;
	int SerialThreadErr;

	/* get option */

	int opt= 0;
	char *token = NULL;
	char *guid = NULL;
	char *SerialPort = NULL;

	// specific the expected option
	static struct option long_options[] = {
			{"id",      required_argument, 0, 'i' },
			{"token", 	required_argument, 0, 't' },
			{"port",    required_argument, 0, 'p' }
	};
	int long_index;
	/* Process option */
	while ((opt = getopt_long(argc, argv,":hi:t:p:",
			long_options, &long_index )) != -1) {
		switch (opt) {
		case 'h' :
			PrintHelpMenu();
			return EXIT_SUCCESS;
			break;
		case 'p' :
			SerialPort = StrDup(optarg);
			break;
		case 'i':
			guid = StrDup(optarg);
			break;
		case 't':
			token = StrDup(optarg);
			break;
		case ':':
			if ((optopt == 'i') || optopt == 'p')
			{
				printf("invalid option(s), using -h for help\n");
				return EXIT_FAILURE;
			}
			break;
		default:
			break;
		}
	}
	if ((SerialPort == NULL) || (guid == NULL))
	{
		printf("invalid options, using -h for help\n");
		return EXIT_FAILURE;
	}
	/* All option valid, start program */
	ZNPACTOROPTION option;
	option.guid = guid;
	option.psw = token;
	puts("Program start");
	LogWrite("Zigbee host start. start init ZNP");
	// Init device organization list
	DeviceListInit();
	/* Start Znp actor */
	//SerialHandleThread = pthread_create(&ZnpActorThread, NULL, (void*)&ZnpActorProcess, (void*)&option);
	printf("create znp actor\n");
	ZnpActorStart(&option);
	/* open serial port and init queue for serial communication */
	char* PortName = malloc(strlen("/dev/") + strlen(SerialPort) + 1);
	memset(PortName, 0, strlen("/dev/") + strlen(SerialPort) + 1);
	sprintf(PortName, "%s%s", "/dev/", SerialPort);
	printf("open port %s\n", PortName);
	while (bResult == FALSE)
	{
		pSerialPort = SerialOpen(PortName, B115200);
		if (pSerialPort == NULL)
		{
			printf("Can not open serial port %s, try another port\n", PortName);
			return EXIT_FAILURE;
		}
		free(PortName);
		// Initial Serial port handle process
		SerialThreadErr = pthread_create(&SerialProcessThread, NULL, (void*)&SerialProcessIncomingData, (void*)pSerialPort);
		SerialThreadErr = pthread_create(&SerialOutputThread, NULL, (void*)&SerialOutputDataProcess, (void*)pSerialPort);
		SerialThreadErr = pthread_create(&SerialHandleThread, NULL, (void*)&SerialInputDataProcess, (void*)pSerialPort);
		// init znp device
		bResult = ZnpInit(pSerialPort);
		if (bResult == FALSE)
		{
			// close serial port to retry
			pthread_cancel(SerialProcessThread);
			pthread_cancel(SerialOutputThread);
			pthread_cancel(SerialHandleThread);
			SerialClose(pSerialPort);
			nRetry++;
			if (nRetry == 5)
			{
				printf("can not start ZNP after 5 times, exit program\n");
				LogWrite("Can't not start ZNP apfter 5 times, exit program");
				exit(0);
			}
			printf("ZNP reset fail, retry\n");
			LogWrite("ZNP reset failed, retry");
		}
		printf("ZNP start success\n");
		LogWrite("ZNP start success");
	}

	while (1)
	{
		DeviceProcessTimeout();
		ZclMsgStatusProcess();
		ZnpCmdStateProcess();
		sleep(1);
	}
	SerialClose(pSerialPort);
	return EXIT_SUCCESS;
}
