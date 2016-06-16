/*
 * typesdef.h
 *
 *  Created on: Jan 28, 2016
 *      Author: ChauNM
 */

#ifndef TYPESDEF_H_
#define TYPESDEF_H_

#include <stddef.h>

typedef unsigned char 			BYTE;
typedef unsigned char* 			PBYTE;
typedef unsigned short 			WORD;
typedef unsigned short* 		PWORD;
typedef signed short			SWORD;
typedef signed short*			PSWORD;
#ifdef PI_RUNNING
typedef unsigned long			DWORD;
typedef unsigned long*			PDWORD;
typedef unsigned long long int	LONG;
typedef unsigned long long int*	PLONG;
#endif
#ifdef PC_RUNNING
typedef unsigned int			DWORD;
typedef unsigned int*			PDWORD;
typedef unsigned long int		LONG;
typedef unsigned long int*		PLONG;
#endif
typedef unsigned char			BOOL;
typedef unsigned char*			PBOOL;
typedef LONG					IEEEADDRESS;
typedef PLONG					PIEEEADDRESS;
typedef void					VOID;
typedef void* 					PVOID;

#define TRUE					1
#define FALSE					0

#endif /* TYPESDEF_H_ */
