/*
* Een functie die van wittegids.be voor een gegeven adres de telefoonnummers en namen opvraagt.
*
* SYNTAX
*	int opzoekingWG(HWND hW, HANDLE heap_van_proces, LPCTSTR gemeente, LPCTSTR straat, LPCTSTR huisnr, ADRES **adressen, CHAR debug)
*
*	- hW: Een handle van een venster dat ouder moet zijn van vensterboodschappen die opzoekingWG kan tonen, mag NULL zijn.
*	- heap_van_proces: Een handle van een heap dat gebruikt moet worden om geheugen van te alloceren.
*	- gemeente: Een string van de gemeentenaam.
*	- straat: Een string van de straatnaam.
*	- huisnr: Een string van het huisnummer.
*	- adressen: Een pointer naar een pointer van een ADRES struct
*	- debug: 1 om meer informatie voor foutopsporing te geven, 0 om dit niet te doen
*
* ADRES struct
*	typedef struct _ADRES {
*		TCHAR gemeente[ZWG_MAX_GEMEENTE];
*		TCHAR straat[ZWG_MAX_STRAAT];
*		TCHAR huisnr[ZWG_MAX_HUISNUMMER];
*		TCHAR naam[ZWG_MAX_NAAM];
*		TCHAR telefoonnr[ZWG_MAX_TELEFOON];
*	} ADRES;
*
*	- element gemeente: Een string van de gemeentenaam, verkregen van wittegids.be.
*	- element straat: Een string van de straatnaam, verkregen van wittegids.be.
*	- element huisnr: Een string van het huisnummer, verkregen van wittegids.be.
*	- element naam: Een string van de naam, verkregen van wittegids.be.
*	- element telefoonnr: Een string van het telefoonnummer, verkregen van wittegids.be
*
* RETURNWAARDEN
*
*	Bij een fout, of wanneer er geen nuttige gegevens van wittegids.be werden gehaald, wordt van de pointer naar de pointer van een ADRES struct een NULL pointer gemaakt.
*	- ZWG_BUG_1: Een fout in de broncode.
*	- ZWG_NOT_DOWNLOADED: De html-pagina kon niet gedownload worden.
*	- ZWG_SERVER_UNAVAILABLE: wittegids.be gaf de boodschap: "503 Service Temporarily Unavailable".
*	- ZWG_ADDRESS_NOT_IN_WG: Het adres is niet opgenomen in wittegids.be
*	- ZWG_TELEPHONE_NOT_IN_WG: Er is van het adres geen telefoonnummer in wittegids.be
*	- ZWG_MEMORY_ERROR: Er deed zich een fout voor bij het alloceren van geheugen.
*	- ZWG_READ_ERROR: Er werden geen gegevens gelezen van wittegids.be.html
*	- 0: Returnwaarde na "opkuis" aanvraag.
*	In alle andere gevallen is de returnwaarde een int van het aantal unieke gevonden gegevens.
*
* OPMERKINGEN
*
*	In het geval van een probleem bij het downloaden van de webpagina krijgt de gebruiker een vensterboodschap met de opties "Opnieuw proberen" en "Annuleren" te zien.
*	In elk geval van een geslaagde oproep van de functie (de returnwaarde is positief of de pointer naar een ADRES struct is geen NULL pointer), is het aan te raden
*	de HeapFree-functie toe te passen op de pointer naar het ADRES struct. Dit om het door de functie gealloceerde geheugen terug vrij te geven en zo dus een geheugenlek
*	te voorkomen.
*	Na de laatste oproep van de functie is het aan te raden om de functie op te roepen met als string voor de gemeentenaam een lege string (""). Dit zorgt er voor dat
*	de functie een "opkuis" houdt.
*	Informatie voor foutopsporing wordt naar stderr geschreven.
*
* TYPISCH GEBRUIK
*
*	#include "opzoekingWG.h"
*	ADRES *adressen;
*	int ret = opzoekingWG(hW, GetProcessHeap(), "gemeente", "straat", "huisnr", &adressen, 0);
*	if(ret > 0)
*	{
*		int i;
*		for(i=0; i < ret; ++i)
*			printf("%s, %s, %s: %s, %s\n", adressen[i].gemeente, adressen[i].straat, adressen[i].huisnr, adressen[i].naam, adressen[i].telefoonnr);
*		HeapFree(GetProcessHeap(), 0, adressen);
*	}
*	else
*		printf("Er konden geen gegevens worden verkregen\n");
*	opzoekingWG(NULL, NULL, "", NULL, NULL, NULL, 0);
*
* MAKER
*	Sven Verlinden
*/

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
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>
#endif

#ifndef _SETFPBYSTR_H
#include "fileprocessing\setfpbystr.h"
#endif

/*#ifndef _UTOUTF8_H
#include "fileprocessing\utoutf8.h"
#endif*/

#define ZWG_MAX_HUISNUMMER 10	//[9][9][9][9][ ][D][C][1][2][\0]
#define ZWG_MAX_NAAM 66
#define ZWG_MAX_TELEFOON 15
#define ZWG_MAX_GEMEENTE 40
#define ZWG_MAX_STRAAT 50
#define ZWG_MAX_ALL ((ZWG_MAX_GEMEENTE > ZWG_MAX_STRAAT)&&(ZWG_MAX_GEMEENTE > ZWG_MAX_NAAM))*ZWG_MAX_GEMEENTE + ((ZWG_MAX_STRAAT > ZWG_MAX_GEMEENTE)&&(ZWG_MAX_STRAAT > ZWG_MAX_NAAM))*ZWG_MAX_STRAAT + ((ZWG_MAX_NAAM > ZWG_MAX_GEMEENTE)&&(ZWG_MAX_NAAM > ZWG_MAX_STRAAT))*ZWG_MAX_NAAM

#define ZWG_BUG_1 -1
#define ZWG_NOT_DOWNLOADED -2
#define ZWG_SERVER_UNAVAILABLE -3
#define ZWG_ADDRESS_NOT_IN_WG -4
#define ZWG_TELEPHONE_NOT_IN_WG -5
#define ZWG_MEMORY_ERROR -6
#define ZWG_READ_ERROR -7

typedef struct _ADRES {
	TCHAR gemeente[ZWG_MAX_GEMEENTE];
	TCHAR straat[ZWG_MAX_STRAAT];
	TCHAR huisnr[ZWG_MAX_HUISNUMMER];
	TCHAR naam[ZWG_MAX_NAAM];
	TCHAR telefoonnr[ZWG_MAX_TELEFOON];
} ADRES;

int opzoekingWG(HWND hW, HANDLE heap_van_proces, LPCTSTR gemeente, LPCTSTR straat, LPCTSTR huisnr, ADRES **adressen, CHAR debug);

#include "opzoekingWG.c"

#endif /*_OPZOEKINGWG_H*/
