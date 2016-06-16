/*
 * log.c
 *
 *  Created on: Mar 17, 2016
 *      Author: ChauNM
 */

#include <stdio.h>
#include <string.h>
#include <time.h>


VOID LogWrite(char string[])
{
	char LogFileName[80];
	FILE* fLogFile = NULL;
	time_t CurrentTime = time(NULL);
	char Time[10];
	struct tm timest = *localtime(&CurrentTime);
#ifdef PI_RUNNING
	strftime(LogFileName, 80, "/home/pi/ZigbeeHost/log/%F.txt", &timest);
#endif
#ifdef PC_RUNNING
	strftime(LogFileName, 80, "/home/chaunm/ZigbeeHost/log/%F.txt", &timest);
#endif
	strftime(Time, 20, "%T", &timest);
	fLogFile = fopen(LogFileName, "a");
	fprintf(fLogFile, "%s ", Time);
	fprintf(fLogFile, " ");
	fprintf(fLogFile, "%s", string);
	fprintf(fLogFile, "\n");
	fclose(fLogFile);
}

