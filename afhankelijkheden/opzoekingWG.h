#ifndef _OPZOEKINGWG_H
#define _OPZOEKINGWG_H

#ifndef _WINDOWS_H
#include <windows.h>
#endif
#ifndef _URLMON_H
#include <urlmon.h>
#endif
#pragma comment(lib,"urlmon.lib")
#ifndef _SHLWAPI_H
#include <shlwapi.h>
#endif
#pragma comment(lib,"shlwapi.lib")
#ifndef _STRSAFE_H
#include <strsafe.h>
#endif

#ifndef _FILEPROCESSING_H
//++#include "fileprocessing.h"
#include "fileprocessing\setfpbystr.h"
//#include "fileprocessing\utoutf8.h"	niet meer nodig sinds de aanpassing van wittegids.be
#endif

#define MAX_HUISNUMMER 10	//[9][9][9][9][ ][D][C][1][2][\0]
#define MAX_NAAM 66
#define MAX_TELEFOON 15
#define MAX_GEMEENTE 40
#define MAX_STRAAT 50
#define MAX_ALL ((MAX_GEMEENTE > MAX_STRAAT)&&(MAX_GEMEENTE > MAX_NAAM))*MAX_GEMEENTE + ((MAX_STRAAT > MAX_GEMEENTE)&&(MAX_STRAAT > MAX_NAAM))*MAX_STRAAT + ((MAX_NAAM > MAX_GEMEENTE)&&(MAX_NAAM > MAX_STRAAT))*MAX_NAAM

#define BUG_1 -1
#define NOT_DOWNLOADED -2
#define SERVER_UNAVAILABLE -3
#define ADDRESS_NOT_IN_WG -4
#define TELEPHONE_NOT_IN_WG -5
#define MEMORY_ERROR -6
#define READ_ERROR -7

typedef struct _ADRES {
	TCHAR gemeente[MAX_GEMEENTE];
	TCHAR straat[MAX_STRAAT];
	TCHAR huisnr[MAX_HUISNUMMER];
	TCHAR naam[MAX_NAAM];
	TCHAR telefoonnr[MAX_TELEFOON];
} ADRES;

int opzoekingWG(HWND hW, HANDLE heap_van_proces, LPCTSTR gemeente, LPCTSTR straat, LPCTSTR huisnr, ADRES **adressen, CHAR debug);

#include "opzoekingWG.c"

#endif /*_OPZOEKINGWG_H*/
