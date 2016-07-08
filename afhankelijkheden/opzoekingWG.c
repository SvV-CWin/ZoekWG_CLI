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
*		TCHAR gemeente[MAX_GEMEENTE];
*		TCHAR straat[MAX_STRAAT];
*		TCHAR huisnr[MAX_HUISNUMMER];
*		TCHAR naam[MAX_NAAM];
*		TCHAR telefoonnr[MAX_TELEFOON];
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
*	- BUG_1: Een fout in de broncode.
*	- NOT_DOWNLOADED: De html-pagina kon niet gedownload worden.
*	- SERVER_UNAVAILABLE: wittegids.be gaf de boodschap: "503 Service Temporarily Unavailable".
*	- ADDRESS_NOT_IN_WG: Het adres is niet opgenomen in wittegids.be
*	- TELEPHONE_NOT_IN_WG: Er is van het adres geen telefoonnummer in wittegids.be
*	- MEMORY_ERROR: Er deed zich een fout voor bij het alloceren van geheugen.
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
*	opzoekingWG(NULL, NULL, "", NULL, NULL, NULL);
*
* MAKER
*	Sven Verlinden
*/

int opzoekingWG(HWND hW, HANDLE heap_van_proces, LPCTSTR gemeente, LPCTSTR straat, LPCTSTR huisnr, ADRES **adressen, CHAR debug)
{
	/*opkuis gevraagd?*/
	if(gemeente[0] == TEXT('\0'))
	{
		if(debug)
			fprintf(stderr, "-> ZWG: opkuisaanvraag ontvangen\n");
		DeleteFile(TEXT("wittegids.be.html"));
		return 0;
	}
	/*EINDE opkuis gevraagd?*/

	/* html-bestand van wittegids.be downloaden */	//normaal wordt op de URL '%'-encodering toegepast, maar na tests blijkt dit niet noodzakelijk te zijn	(getest op http://wittegids.be/q/name/address/where/Heist-op-den-Berg/street/Livinus Carr象at/nr/22)
	int aantal=0, i;
	StringCchLength(gemeente, STRSAFE_MAX_CCH, (unsigned int *)&i);
	aantal += i;
	StringCchLength(straat, STRSAFE_MAX_CCH, (unsigned int *)&i);
	aantal += i;
	StringCchLength(huisnr, STRSAFE_MAX_CCH, (unsigned int *)&i);
	aantal += i;
	TCHAR url[57+aantal+1];
	#if (defined UNICODE)||(defined _UNICODE)
	if(FAILED(StringCchPrintf(url, 57+aantal+1+1, L"http://www.wittegids.be/q/name/address/where/%ls/street/%ls/nr/%ls", gemeente, straat, huisnr)))	//URL samenstellen //++ om een onbekende reden mist StringCchPrintf het overzetten van het laatste karakter (zonder fout te geven), dit in tegenstelling tot sprintf_s onder dezelfde omstandigheden
	#else
	if(FAILED(StringCchPrintf(url, 57+aantal+1+1, "http://www.wittegids.be/q/name/address/where/%ls/street/%ls/nr/%ls", gemeente, straat, huisnr)))
	#endif
	{
		*adressen = NULL;
		return BUG_1;
	}

	HANDLE *hhtml;
	do
	{
		if((URLDownloadToFile(NULL, url, TEXT("wittegids.be.html"), 0, NULL) != S_OK)||(hhtml = CreateFile(TEXT("wittegids.be.html"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL), hhtml == INVALID_HANDLE_VALUE))
		{
			aantal = 0;
			StringCchLength(gemeente, STRSAFE_MAX_CCH, (unsigned int *)&i);
			aantal += i;
			StringCchLength(straat, STRSAFE_MAX_CCH, (unsigned int *)&i);
			aantal += i;
			StringCchLength(huisnr, STRSAFE_MAX_CCH, (unsigned int *)&i);
			aantal += i;
			StringCchLength(url, STRSAFE_MAX_CCH, (unsigned int *)&i);
			aantal += i;
			TCHAR temp[231+aantal+1];
			#if (defined UNICODE)||(defined _UNICODE)
			StringCchPrintf(temp, 231+aantal+1, L"De gegevens %ls, %ls en %ls werden omgevormd tot %ls.\n\nEr deed zich een probleem voor met het verkrijgen van deze pagina.\nMogelijke oorzaken zijn:\n- Geen internetverbinding\n- Fout gespeld adres\n\nOpnieuw proberen of dit adres overslaan?", gemeente, straat, huisnr, url);
			#else
			StringCchPrintf(temp, 231+aantal+1, "De gegevens %s, %s en %s werden omgevormd tot %s.\n\nEr deed zich een probleem voor met het verkrijgen van deze pagina.\nMogelijke oorzaken zijn:\n- Geen internetverbinding\n- Fout gespeld adres\n\nOpnieuw proberen of dit adres overslaan?", gemeente, straat, huisnr, url);
			#endif
			aantal = MessageBox(hW, temp, TEXT("opzoekingWG - Fout"), MB_RETRYCANCEL | MB_ICONWARNING | MB_SETFOREGROUND);
		}
		else
			break;
		if(aantal == IDCANCEL)
		{
			*adressen = NULL;
			if(hhtml != INVALID_HANDLE_VALUE)
				CloseHandle(hhtml);
			return NOT_DOWNLOADED;
		}
	} while(aantal == IDRETRY);
	/* EINDE download html-bestand */

	LARGE_INTEGER startdata, eindedata;
	eindedata.QuadPart = 150;
	/*bestaat adres in Witte Gids?*/
	if(!SetFPByStr(hhtml, TEXT("503 Service Temporarily Unavailable"), &eindedata, NULL))	//controleer op 503-serverfout
	{
		*adressen = NULL;
		CloseHandle(hhtml);
		return SERVER_UNAVAILABLE;
	}
	if(!SetFPByStr(hhtml, TEXT("Er werden geen resultaten gevonden voor "), NULL, NULL))	//controleer of resultaten zijn gevonden volgens de Witte Gids
	{
		*adressen = NULL;
		CloseHandle(hhtml);
		if(debug)
			fprintf(stderr, "-> ZWG: wittegids.be.html geeft \'Er werden geen resultaten gevonden voor \'.\n");
		return ADDRESS_NOT_IN_WG;
	}
	if(!SetFPByStr(hhtml, TEXT("Uw zoekopdracht voor het adres "), NULL, NULL))	//controleer of resultaten zijn gevonden volgens de Witte Gids
	{
		*adressen = NULL;
		CloseHandle(hhtml);
		if(debug)
			fprintf(stderr, "-> ZWG: wittegids.be.html geeft \'Uw zoekopdracht voor het adres \'.\n");
		return ADDRESS_NOT_IN_WG;
	}
	if(SetFPByStr(hhtml, TEXT("truvo.data['raw']="), NULL, &startdata))	//zet in startdata het begin van de data
	{
		if(debug)
			fprintf(stderr, "-> ZWG: \'truvo.data['raw']=\' niet gevonden in wittegids.be.html.\n");
		*adressen = NULL;
		CloseHandle(hhtml);
		return ADDRESS_NOT_IN_WG;
	}
	/*EINDE bestaanscontrole*/

	/*alle relevante gegevens opnemen in geheugen*/
	SetFPByStr(hhtml, TEXT(";truvo.data"), NULL, &eindedata);	//zet in eindedata waar de data stopt
	CHAR *data = HeapAlloc(heap_van_proces, 0, eindedata.QuadPart);
	CHAR *data_begin = data;
	if(data == NULL)
	{
		*adressen = NULL;
		return MEMORY_ERROR;
	}
	DWORD b;
	SetFilePointerEx(hhtml, startdata, NULL, FILE_BEGIN);
	ReadFile(hhtml, data, eindedata.QuadPart, &b, NULL);
	CloseHandle(hhtml);
	if(b == 0)
	{
		*adressen = NULL;
		HeapFree(heap_van_proces, 0, data_begin);
		return READ_ERROR;
	}
	data[b-1] = '\0';
	/*EINDE gegevens opnemen*/
	if(debug)
		fprintf(stderr, "-> ZWG: data = \'%s\'.\n", data);
	/*aantal telefoonnummers bepalen*/
	{
	CHAR *temp = data;
	for(aantal=0; temp = StrStrA(temp, "phone\":"), temp != NULL; ++aantal, ++temp);	//bepaal het aantal telefoonnummers
	if(aantal == 0)	//geen telefoonnummers
	{
		*adressen = NULL;
		HeapFree(heap_van_proces, 0, data_begin);
		return TELEPHONE_NOT_IN_WG;
	}
	}
	/*EINDE bepaal aantal telefoonnummers*/

	/*maak array van door te geven ADRES*/
	ADRES *Padressen = HeapAlloc(heap_van_proces, 0, aantal*sizeof(ADRES));
	if(Padressen == NULL)
	{
		*adressen = NULL;
		HeapFree(heap_van_proces, 0, data_begin);
		return MEMORY_ERROR;
	}
	/*EINDE array maken*/

	CHAR dummy[MAX_ALL];
	/*GEMEENTENAAM OPNEMEN*/
	if(StrStrA(data, "city\":\"") == NULL)
	{
		*adressen = NULL;
		HeapFree(heap_van_proces, 0, data_begin);
		HeapFree(heap_van_proces, 0, Padressen);
		return ADDRESS_NOT_IN_WG;
	}
	{
	CHAR *temp = StrStrA(data, "city\":\"") + sizeof("city\":\"") - 1;
	for(i=0; dummy[i] = temp[i], (dummy[i] != '\"')&&(i < MAX_GEMEENTE-1); ++i);
	}
	if(i == MAX_GEMEENTE-1)
		dummy[i-1] = '_';
	dummy[i] = '\0';
//	UnicodeStr4CodeToUTF8Str(dummy); //getest op http://www.wittegids.be/q/name/address/where/bierbeek/street/oude%20baan/nr/157
	#if (defined UNICODE)||(defined _UNICODE)
	MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, dummy, -1, Padressen[0].gemeente, MAX_GEMEENTE);
	#else
	StringCchCopy(Padressen[aantal].gemeente, MAX_GEMEENTE, dummy);
	#endif
	
	/*STRAATNAAM OPNEMEN*/
	if(StrStrA(data, "street\":\"") == NULL)
	{
		*adressen = NULL;
		HeapFree(heap_van_proces, 0, data_begin);
		HeapFree(heap_van_proces, 0, Padressen);
		return ADDRESS_NOT_IN_WG;
	}
	{
	CHAR *temp = StrStrA(data, "street\":\"") + sizeof("street\":\"") - 1;
	for(i=0; dummy[i] = temp[i], ((dummy[i] < '0')||(dummy[i] > '9'))&&(dummy[i] != '\"')&&(i < MAX_STRAAT-1); ++i);
	}
	if(i == MAX_STRAAT-1)
		dummy[i-2] = '_';
	dummy[i-1] = '\0';
//	UnicodeStr4CodeToUTF8Str(dummy); //getest op http://www.wittegids.be/q/name/address/where/bierbeek/street/oude%20baan/nr/157
	#if (defined UNICODE)||(defined _UNICODE)
	MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, dummy, -1, Padressen[0].straat, MAX_STRAAT);
	#else
	StringCchCopy(Padressen[aantal].straat, MAX_STRAAT, dummy);
	#endif

	for(aantal=0; data = StrStrA(data, "phone\":\"") + sizeof("phone\":\"") - 1, (unsigned int)data != sizeof("phone\":\"") - 1; ++aantal)
	{
		/*TELEFOONNUMMER OPNEMEN*/
		for(i=0; dummy[i] = data[i], (dummy[i] != '\"')&&(i < MAX_TELEFOON-1); ++i);
		if((i == 0)&&(StrStrA(data, "mobile\":\"") != NULL))	//als er geen telefoonnummer is: mobiel telefoonnummer
		{
			data = StrStrA(data, "mobile\":\"") + sizeof("mobile\":\"") - 1;
			for(i=0; dummy[i] = data[i], (dummy[i] != '\"')&&(i < MAX_TELEFOON-1); ++i);
		}
		if((i == 0)||((i == MAX_TELEFOON-1)&&(dummy[i] != '\"')))	//geen of ongeldig telefoonnummer
			goto NEGEREN;
		dummy[i] = '\0';
		#if (defined UNICODE)||(defined _UNICODE)
		MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, dummy, -1, Padressen[aantal].telefoonnr, MAX_TELEFOON);
		#else
		StringCchCopy(Padressen[aantal].telefoonnr, MAX_TELEFOON, dummy);
		#endif

		/*NAAM OPNEMEN*/
		if(StrStrA(data, "name\":\"") == NULL)
			goto NEGEREN;
		data = StrStrA(data, "name\":\"") + sizeof("name\":\"") - 1;
		for(i=0; dummy[i] = data[i], (dummy[i] != '\"')&&(i < MAX_NAAM-1); ++i);
		if((i == MAX_NAAM-1)&&(dummy[i] != '\"'))
			dummy[i-1] = '_';
		dummy[i] = '\0';
//		UnicodeStr4CodeToUTF8Str(dummy); //getest op http://www.wittegids.be/q/name/address/where/bierbeek/street/oude%20baan/nr/157
		#if (defined UNICODE)||(defined _UNICODE)
		MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, dummy, -1, Padressen[aantal].naam, MAX_NAAM);
		#else
		StringCchCopy(Padressen[aantal].naam, MAX_NAAM, dummy);
		#endif

		/*STRAATNAAM OVERSLAAN*/
		if(StrStrA(data, "street\":\"") == NULL)
			goto NEGEREN;
		data = StrStrA(data, "street\":\"") + sizeof("street\":\"") - 1;
		for(++data; ((data[0] < '0')||(data[0] > '9'))&&(data[0] != '\"'); ++data);
		if(data[0] == '\"')	//adres zonder huisnummer
			goto NEGEREN;

		/*HUISNUMMER OPNEMEN*/
		for(i=0; dummy[i] = data[i], (dummy[i] != '\"')&&(i < MAX_HUISNUMMER-1); ++i);
		dummy[i] = '\0';
		#if (defined UNICODE)||(defined _UNICODE)
		MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, dummy, -1, Padressen[aantal].huisnr, MAX_HUISNUMMER);
		#else
		StringCchCopy(Padressen[aantal].huisnr, MAX_HUISNUMMER, dummy);
		#endif

		/*CONTROLE of huisnummer van het adres overeenkomt met dat in de Witte Gids*/
		for(i=0; (Padressen[aantal].huisnr[i] != TEXT(' '))&&(Padressen[aantal].huisnr[i] != TEXT('\0')); ++i)
		{
			if(Padressen[aantal].huisnr[i] != huisnr[i])
				break;
		}
		if(((Padressen[aantal].huisnr[i] != TEXT(' '))&&(Padressen[aantal].huisnr[i] != TEXT('\0')))||(huisnr[i] != TEXT('\0')))
			goto NEGEREN;

		/*CONTROLE of combinatie telefoonnummer en naam al gevonden is*/
		for(i=0; i < aantal; ++i)
		{
			if((!StrCmp(Padressen[i].telefoonnr, Padressen[aantal].telefoonnr))&&(!StrCmp(Padressen[i].naam, Padressen[aantal].naam)))
				goto NEGEREN;
		}
		StringCchCopy(Padressen[aantal].gemeente, MAX_GEMEENTE, Padressen[0].gemeente);
		StringCchCopy(Padressen[aantal].straat, MAX_STRAAT, Padressen[0].straat);
		goto AANNEMEN;

		NEGEREN:
		aantal--;
		AANNEMEN:;
	}
	HeapFree(heap_van_proces, 0, data_begin);
	if(aantal == 0)
	{
		*adressen = NULL;
		HeapFree(heap_van_proces, 0, Padressen);
		return TELEPHONE_NOT_IN_WG;
	}
	Padressen = HeapReAlloc(heap_van_proces, 0, Padressen, aantal*sizeof(ADRES));	//verkleint het nodige geheugen in geval van niet-opgenomen elementen
	*adressen = Padressen;
	return aantal;
}
