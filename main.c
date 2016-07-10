#define VERSION L"v1.0b"

#define UNICODE
#define _UNICODE

#include <stdio.h>
#include <stdlib.h>
/*for correct handling of UTF-8 data (tests with the CRT functions and with the correct locale set were not satisfactory)*/
#define wcstombs(dst, src, max) WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, max, NULL, NULL)
#define mbstowcs(dst, src, max) MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, max)
#include <wchar.h>
#include <string.h>
#include <direct.h>		//for _wgetcwd
#include <locale.h>		//for correct (console) character handling
#define _CONIO_RETRO_COLORS
#include <conio.h>		//for console functions (ex. color adaptations) [a console is typically 80 characters wide]
#include <process.h>	//for spawnlp (opening 'licenties.txt')

#include "opzoekingwg.h"
#include "resources.h"

#pragma comment(lib,"shell32.lib")	//for ShellExecute
#ifndef _SHELLAPI_H
#include <shellapi.h>	//for ShellExecute
#endif

#pragma comment(lib,"expat.lib")
#pragma comment(lib,"minizip.lib")
#pragma comment(lib,"libzip.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"xlsxio.lib")
#pragma comment(lib,"xlsxwriter.lib")
#define STATIC			//needed for correct use of xlsxio.lib via xlsxio_read.h
#include "xlsxio_read.h"
#undef STATIC
#undef SLIST_ENTRY		//there is a definition of SLIST_ENTRY in common.h (xlsxwriter)*/
#include "xlsxwriter.h"
#include "licenties.h"

#define SJAB_FAILED -1
#define READ_FAILED -2
#define WRITE_FAILED -3
#define INCORRECT_XLSX -4
#define PAD_SJABLOON "ZoekWG_sjabloon.xlsx"
#define PAD_TMP "ZoekWG_temp.xlsx"
#define wPAD_TMP L"ZoekWG_temp.xlsx"

char debug = 0;
wchar_t *xlsx_IN, *wcwd;

BOOL WINAPI SetConsoleIcon(HICON hIcon);
int help(void);
int do_exit(int exitcode);
char maaksjabloon(void);
char verwerkbestand(void);
char openxlsx(xlsxioreader *xlsxREAD, xlsxioreadersheet *xlsxREADSHEET);
char readyxlsx(lxw_workbook **xlsxWRITE, lxw_worksheet **xlsxWRITESHEET);
char *krijgcel(xlsxioreadersheet sheet);
void zetcel(lxw_worksheet *sheet, char value[]);

BOOL WINAPI SetConsoleIcon(HICON hIcon)
{
	typedef BOOL (WINAPI *PSetConsoleIcon)(HICON);
	static PSetConsoleIcon pSetConsoleIcon = NULL;
	if(pSetConsoleIcon == NULL)
		pSetConsoleIcon = (PSetConsoleIcon)GetProcAddress(GetModuleHandle(L"kernel32"), "SetConsoleIcon");
	if(pSetConsoleIcon == NULL)
		return FALSE;
	return pSetConsoleIcon(hIcon);
}

int wmain(int argc, wchar_t *argv[])	//returns 1 on succes
{
	SetConsoleIcon(LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(icoon)));
	if((argc == 3)&&(!wcscmp(argv[2], L"debug")))
		debug = 1;
	_textcolor(LIGHTGREEN);
	printf("\nZoekWG (via CLI) (%ls)\n", VERSION);
	_textcolor(GREEN);
	puts("maker: Sven Verlinden\nmaakt gebruik van: expat, zlib, libzip, xlsxio en xlsxwriter");
	wcwd = _wgetcwd(NULL, 0);
	if(debug)
	{
		_textcolor(DARKGRAY);
		printf("\ntaalindeling (voor console): %s\nargv[0] = |%ls|\nargv[1] = |%ls|\ncd = |%ls|\n", setlocale(LC_ALL, ""), argv[0], argv[1], wcwd);
	}
	if((argc < 2)||(argv[1][0] == L'-')||(argv[1][0] == L'/'))
		return do_exit(help());
	if(debug)
	{
		_textcolor(DARKGRAY);
		puts("Geen \'-\' of \'/\' als eerste teken van argument.");
	}
	wchar_t _IN[wcslen(argv[1])+1];		//kopieren van het argument om de tekenreeks 'veilig te stellen'
	wcscpy(_IN, argv[1]);
	xlsx_IN = _IN;
	if(debug)
	{
		_textcolor(DARKGRAY);
		printf("De tekenreeks van het argument is gekopieerd (%ls).\n", xlsx_IN);
	}
	return do_exit(verwerkbestand());
}

int help(void)	//returns exit return code
{
	_textcolor(YELLOW);
	puts("\nZoekWG vult een Microsoft Excel 2007/2010/2013 XML (*.xlsx)-bestand aan\n\
met telefoonnummers, afkomstig van wittegids.be\n\
Het (*.xlsx)-adressenbestand moet aan een bepaald sjabloon voldoen.\n\n\
Wil je de licenties van de gebruikte onderdelen bekijken? [Y/N]");
	switch(_getch())
	{
		case 'y':
		case 'Y':
			{
			FILE *f = fopen("licenties.txt", "w");
			if(f != NULL)
			{
				fputs(LICENTIES, f);
				fclose(f);
				_textcolor(GREEN);
				puts("Het aangemaakte bestand \'licenties.txt\' wordt geopend.\nPas als Notepad wordt gesloten gaat het programma verder.");
				if(_spawnlp(_P_WAIT, "C:\\Windows\\notepad", "C:\\Windows\\notepad", "licenties.txt", NULL) == -1)		//ondanks spawnl->p<- wordt notepad niet gevonden, tenzij op deze manier
				{
					_textcolor(RED);
					perror("Het aangemaakte bestand \'licenties.txt\' kon niet geopend worden");
				}
				remove("licenties.txt");
			}
			else
			{
				_textcolor(RED);
				puts("In de huidige werkmap kon \'licenties.txt\' niet aangemaakt worden.");
			}
			}
			break;
		case 0:			//pijltjes, functietoetsen, ... zijn steeds twee opeenvolgende 'karakters' met een enkele toetsaanslag
		case 224:
			_getch();
			break;
	}
	_textcolor(BROWN);
	puts("\nGebruik (opdrachtensyntax):\n\
ZoekWG_CLI.exe <pad naar aan te vullen bestand> [debug]\n\n\
<pad ...>: pad naar het aan te vullen (*.xlsx)-bestand\n\
[debug]: meer uitvoer (optioneel)\n");
	_textcolor(YELLOW);
	puts("\
-> Je kan het programma het makkelijkst gebruiken via \'slepen-en-neerzetten\':\n\
In een verkenner klik en sleep je het aan te vullen (*.xlsx)-bestand\n\
op het programma (ZoekWG(.exe)) en laat je het bestand los.\n");
	if(maaksjabloon())
		return SJAB_FAILED;
	_textcolor(WHITE);
	printf("Een in te vullen sjabloon werd gemaakt in\n\
 \'%ls\', met als naam\n\
 \'%s\'.\nDit bestand nu openen? [Y/N]\n", wcwd, PAD_SJABLOON);
	switch(_getch())
	{
		case 'y':
		case 'Y':
			{
			int ret = (int)ShellExecuteA(NULL, "open", PAD_SJABLOON, NULL, NULL, SW_SHOWNORMAL);
			if(ret <= 32)
			{
				_textcolor(LIGHTRED);
				puts("\nHet bestand met het sjabloon kon niet geopend worden!");
				if(debug)
				{
					_textcolor(RED);
					switch(ret)
					{
						case 0:
							puts("(The operating system is out of memory or resources)");
							break;
						case ERROR_FILE_NOT_FOUND:
							puts("(The environment variable COMSPEC was not found)");
							break;
						case ERROR_PATH_NOT_FOUND:
							puts("(The path for the environment variable COMSPEC was not found)");
							break;
						case ERROR_BAD_FORMAT:
							puts("(The environment variable COMSPEC is invalid (non-Win32 .exe or error in .exe image))");
							break;
						case SE_ERR_ACCESSDENIED:
							puts("(The operating system denied access to the environment variable COMSPEC)");
							break;
						case SE_ERR_ASSOCINCOMPLETE:
							puts("(The file name association for the environment variable COMSPEC is incomplete or invalid)");
							break;
						case SE_ERR_DDEBUSY:
							puts("(The DDE transaction could not be completed because other DDE transactions were being processed)");
							break;
						case SE_ERR_DDEFAIL:
							puts("(The DDE transaction failed)");
							break;
						case SE_ERR_DDETIMEOUT:
							puts("(The DDE transaction could not be completed because the request timed out)");
							break;
						case SE_ERR_DLLNOTFOUND:
							puts("(The specified DLL was not found)");
							break;
						case SE_ERR_NOASSOC:
							puts("(There is no application associated with the the environment variable COMSPEC. This error will also be returned if you attempt to print a file that is not printable)");
							break;
						case SE_ERR_OOM:
							puts("(There was not enough memory to complete the operation)");
							break;
						case SE_ERR_SHARE:
							puts("(A sharing violation occurred)");
							break;
					}
				}
				return 1;
			}
			}
			break;
		case 0:
		case 224:
			_getch();
			break;
	}
	_textcolor(WHITE);
	return 0;
}

int do_exit(int exitcode)	//returns exit return code
{
	_textcolor(RED);
	switch(exitcode)
	{
		case SJAB_FAILED:
			puts("\nEr kon geen sjabloon gemaakt worden.");
			break;
		case READ_FAILED:
			puts("\nEr is een probleem met het inlezen van het bestand.");
			break;
		case WRITE_FAILED:
			puts("\nEr is een probleem met het aanmaken van een (*.xlsx)-bestand.");
			break;
		case INCORRECT_XLSX:
			puts("\nHet ingevoerde (*.xlsx)-bestand voldoet niet aan de verwachtingen.");
			break;
		case 0:
			_textcolor(LIGHTGREEN);
			puts("\nDe verwerking is succesvol voltooid!");
			break;
		default:
			puts("\nEr is een probleem opgetreden.");
			break;
	}
	_textcolor(WHITE);
	puts("\nDruk op een toets om het programma af te sluiten...");
	switch(_getch())
	{
		case 0:
		case 224:
			_getch();
			break;
	}
	SetConsoleIcon(NULL);
	free(wcwd);
	return exitcode;
}

char maaksjabloon(void)	//returns 0 on succes
{
	lxw_workbook *SJAB_wb = workbook_new(PAD_SJABLOON);
	if(SJAB_wb == NULL)
	{
		if(debug)
		{
			_textcolor(LIGHTRED);
			puts("Er is onvoldoende geheugen om een nieuw xlsx-werkboek te maken (1)!");
		}
		return 1;
	}
	lxw_worksheet *SJAB_ws = workbook_add_worksheet(SJAB_wb, "afwezigen");
	if(SJAB_ws == NULL)
	{
		if(debug)
		{
			_textcolor(LIGHTRED);
			puts("Er is onvoldoende geheugen om een nieuw xlsx-werkboek te maken (2)!");
		}
		workbook_close(SJAB_wb);
		remove(PAD_SJABLOON);
		return 1;
	}
	worksheet_set_column(SJAB_ws, 0, 0, 29.75, NULL);
	worksheet_set_column(SJAB_ws, 1, 1, 12.4, NULL);
	worksheet_set_column(SJAB_ws, 2, 2, 39.7, NULL);
	worksheet_write_string(SJAB_ws, 0, 0, "gebiedsnummer:", NULL);
	worksheet_write_string(SJAB_ws, 0, 1, "1", NULL);
	worksheet_write_string(SJAB_ws, 1, 0, "gemeente:", NULL);
	worksheet_write_string(SJAB_ws, 1, 1, "Duffel", NULL);
	worksheet_write_string(SJAB_ws, 3, 0, "straatnaam", NULL);
	worksheet_write_string(SJAB_ws, 3, 1, "huisnummer", NULL);
	worksheet_write_string(SJAB_ws, 4, 0, "Stationsstraat", NULL);
	worksheet_write_string(SJAB_ws, 4, 1, "20", NULL);
	worksheet_write_string(SJAB_ws, 5, 1, "22", NULL);
	worksheet_write_string(SJAB_ws, 7, 0, "Rietlei", NULL);
	worksheet_write_string(SJAB_ws, 7, 1, "7", NULL);
	worksheet_write_string(SJAB_ws, 9, 0, "A. Stocletlaan", NULL);
	worksheet_write_string(SJAB_ws, 9, 1, "10", NULL);
	worksheet_write_string(SJAB_ws, 10, 1, "12", NULL);
	if(workbook_close(SJAB_wb) != LXW_NO_ERROR)//++ test op bestaan in huidige werkmap
	{
		if(debug)
		{
			_textcolor(LIGHTRED);
			puts("Probleem bij het aanmaken van het xlsx-bestand!");
		}
		return 2;
	}
	return 0;	
	
	/*xlsxiowrite_set_row_height(SJAB_wb, 1);
	xlsxiowrite_set_detection_rows(SJAB_wb, 11);
	xlsxiowrite_add_column(SJAB_wb, "gebiedsnummer:", 20);
	xlsxiowrite_add_column(SJAB_wb, "1", 20);
	xlsxiowrite_next_row(SJAB_wb);
	xlsxiowrite_add_cell_string(SJAB_wb, "gemeente:");
	xlsxiowrite_add_cell_string(SJAB_wb, "Duffel");
	xlsxiowrite_next_row(SJAB_wb);
	xlsxiowrite_next_row(SJAB_wb);
	xlsxiowrite_add_cell_string(SJAB_wb, "straatnaam");
	xlsxiowrite_add_cell_string(SJAB_wb, "huisnummer");
	xlsxiowrite_next_row(SJAB_wb);
	xlsxiowrite_add_cell_string(SJAB_wb, "Stationsstraat");
	xlsxiowrite_add_cell_string(SJAB_wb, "20");
	xlsxiowrite_next_row(SJAB_wb);
	xlsxiowrite_add_cell_string(SJAB_wb, "");
	xlsxiowrite_add_cell_string(SJAB_wb, "22");
	xlsxiowrite_next_row(SJAB_wb);
	xlsxiowrite_next_row(SJAB_wb);
	xlsxiowrite_add_cell_string(SJAB_wb, "Rietlei");
	xlsxiowrite_add_cell_string(SJAB_wb, "7");
	xlsxiowrite_next_row(SJAB_wb);
	xlsxiowrite_next_row(SJAB_wb);
	xlsxiowrite_add_cell_string(SJAB_wb, "A. Stocletlaan");
	xlsxiowrite_add_cell_string(SJAB_wb, "10");
	xlsxiowrite_next_row(SJAB_wb);
	xlsxiowrite_add_cell_string(SJAB_wb, "");
	xlsxiowrite_add_cell_string(SJAB_wb, "12");
	if(xlsxiowrite_close(SJAB_wb))
	{
		_textcolor(LIGHTRED);
		puts("Probleem bij het aanmaken van het xlsx-bestand!");
		return 2;
	}
	return 0;*/
}

char verwerkbestand(void)	//returns 0 on succes
{
	xlsxioreader xlsxREAD;
	xlsxioreadersheet xlsxREADSHEET;
	if(openxlsx(&xlsxREAD, &xlsxREADSHEET))
		return READ_FAILED;
	if(debug)
	{
		_textcolor(DARKGRAY);
		puts("Het (*.xlsx)-bestand werd geopend.");
	}
	lxw_workbook *xlsxWRITE;
	lxw_worksheet *xlsxWRITESHEET;
	if(readyxlsx(&xlsxWRITE, &xlsxWRITESHEET))
	{
		xlsxioread_sheet_close(xlsxREADSHEET);
		xlsxioread_close(xlsxREAD);
		return WRITE_FAILED;
	}
	if(debug)
	{
		_textcolor(DARKGRAY);
		puts("Een geheugenlocatie voor een nieuw (*.xlsx)-bestand is toegewezen.");
	}

	char *celIN;
	wchar_t gemeente[ZWG_MAX_GEMEENTE];
	gemeente[0] = L'0';
	/*Neem tot de cel met een tekenreeks met 'straat' in, de gegevens over in de nieuwe (*.xlsx)*/
	while(celIN = krijgcel(xlsxREADSHEET), (celIN != NULL)&&(_stristr(celIN, "straat") == NULL))
	{
		if((gemeente[0] == L'1')&&(mbstowcs(gemeente, celIN, ZWG_MAX_GEMEENTE) < 2))	//gemeentenaam bewaren
		{
			if(debug)
			{
				_textcolor(LIGHTRED);
				printf("Probleem bij het omvormen van\n \'%s\' naar een \'wide character\'-string (1).\n", celIN);
			}
			free(celIN);
			goto ABORT;
		}
		if(_stristr(celIN, "gemeente") != NULL)
		{
			zetcel(xlsxWRITESHEET, "");		zetcel(xlsxWRITESHEET, "");		//zetcel gaat uit van vier gebruikte kolommen per rij
			gemeente[0] = L'1';
		}
		zetcel(xlsxWRITESHEET, celIN);
		free(celIN);
	}
	if(debug)
	{
		_textcolor(DARKGRAY);
		printf("Gemeente: \'%ls\'.\n", gemeente);
	}
	if((celIN == NULL)||(free(celIN), celIN = krijgcel(xlsxREADSHEET), celIN == NULL))	//neem de cel met 'huisnummer' op
	{
		if(debug)
		{
			_textcolor(LIGHTRED);
			puts("Er zijn geen gegevens (meer) ingelezen voor of na het woord \'straat\'.");
		}
		goto ABORT;
	}
	free(celIN);
	int i;
	for(i = 0; i != 6; ++i)
		zetcel(xlsxWRITESHEET, "");		//zetcel gaat uit van vier gebruikte kolommen per rij
	zetcel(xlsxWRITESHEET, "Straatnaam");	zetcel(xlsxWRITESHEET, "Huisnummer");	zetcel(xlsxWRITESHEET, "Naam");	zetcel(xlsxWRITESHEET, "Telefoonnummer");

	wchar_t straat[ZWG_MAX_STRAAT], huisnr[ZWG_MAX_HUISNUMMER];
	straat[0] = L'0';
	HANDLE hheap = GetProcessHeap();
	ADRES *adres;
	char nieuwestraat, mbs[ZWG_MAX_ALL];
	int ret;
	while(celIN = krijgcel(xlsxREADSHEET), celIN != NULL)	//neem gegevens op tot einde van het blad
	{
		/*celIN bevat het gegeven uit de kolom met de straatnamen*/
		if(celIN[0] != '\0')
		{
			nieuwestraat = 1;
			if(straat[0] != L'0')	//het is niet de eerste straat van de gegevens
			{
				for(i = 0; i != 4; ++i)
					zetcel(xlsxWRITESHEET, "");		//schrijf een blanco rij
			}
			if(mbstowcs(straat, celIN, ZWG_MAX_STRAAT) < 2)
			{
				if(debug)
				{
					_textcolor(LIGHTRED);
					printf("Probleem bij het omvormen van\n \'%s\' naar een \'wide character\'-string (2).\n", celIN);
				}
				free(celIN);
				goto ABORT;
			}
		}
		else
			nieuwestraat = 0;
		free(celIN);
		celIN = krijgcel(xlsxREADSHEET);
		/*celIN bevat het gegeven uit de kolom met de huisnummers*/
		if(celIN[0] == '\0')		//een rij waarbij de cel in de kolom met de huisnummers geen gegevens bevat, wordt genegeerd (dit kan zich voordoen als bij het aanmaken van het (*.xlsx)-bestand gegevens uit de laatste rij(en) verwijderd werden)
		{
			if(nieuwestraat)
			{
				if(!wcstombs(mbs, straat, ZWG_MAX_ALL))
					zetcel(xlsxWRITESHEET, "?");
				else
					zetcel(xlsxWRITESHEET, mbs);
				for(i=0; i != 3; ++i)
					zetcel(xlsxWRITESHEET, "");
			}
			free(celIN);
			goto LOOPEND;
		}
		/*eventuele toevoegingen aan het huisnummer verwijderen*/
		for(i=0; isdigit(celIN[i]); ++i);
		if(!i)
		{
			if(debug)
			{
				_textcolor(LIGHTRED);
				printf("\'%s\' werd verwacht een huisnummer te zijn,\n maar begint niet met een cijfer.\n", celIN);
			}
			free(celIN);
			goto ABORT;
		}
		celIN[i] = '\0';
		if(!mbstowcs(huisnr, celIN, ZWG_MAX_HUISNUMMER))
		{
			if(debug)
			{
				_textcolor(LIGHTRED);
				printf("Probleem bij het omvormen van\n \'%s\' naar een \'wide character\'-string (3).\nEr werd een huisnummer verwacht.\n", celIN);
			}
			free(celIN);
			goto ABORT;
		}
		free(celIN);
		if(debug)
			_textcolor(BROWN);
		/*OPZOEKING obv de gegevens*/
		ret = opzoekingWG(NULL, hheap, gemeente, straat, huisnr, &adres, debug);
		if(ret < 1)		//'voorbereidingen' voor een gefaalde opzoeking
		{
			if(nieuwestraat)
			{
				if(!wcstombs(mbs, straat, ZWG_MAX_ALL))
					zetcel(xlsxWRITESHEET, "?");
				else
					zetcel(xlsxWRITESHEET, mbs);
			}
			else
				zetcel(xlsxWRITESHEET, "");
			if(!wcstombs(mbs, huisnr, ZWG_MAX_ALL))
				zetcel(xlsxWRITESHEET, "?");
			else
				zetcel(xlsxWRITESHEET, mbs);
			zetcel(xlsxWRITESHEET, "");
			_textcolor(RED);
			printf("%ls %ls -> ", straat, huisnr);
		}
		switch(ret)
		{
			case ZWG_BUG_1:
				zetcel(xlsxWRITESHEET, "Fout in programma");
				puts("Fout in programma");
				break;
			case ZWG_NOT_DOWNLOADED:
				zetcel(xlsxWRITESHEET, "Downloadprobleem");
				puts("Downloadprobleem");
				break;
			case ZWG_SERVER_UNAVAILABLE:
				zetcel(xlsxWRITESHEET, "wittegids.be onbereikbaar");
				puts("wittegids.be onbereikbaar");
				break;
			case ZWG_ADDRESS_NOT_IN_WG:
				zetcel(xlsxWRITESHEET, "Adres onbeschikbaar");
				puts("Adres onbeschikbaar");
				break;
			case ZWG_TELEPHONE_NOT_IN_WG:
				zetcel(xlsxWRITESHEET, "Telefoonnr onbeschikbaar");
				puts("Telefoonnr onbeschikbaar");
				break;
			case ZWG_MEMORY_ERROR:
				zetcel(xlsxWRITESHEET, "Geheugenprobleem");
				puts("Geheugenprobleem");
				break;
			case ZWG_READ_ERROR:
				zetcel(xlsxWRITESHEET, "Inleesfout");
				puts("Inleesfout");
				break;
			default:
				for(i = 0; i != ret; ++i)	//voor elk gevonden gegeven...
				{
					if(nieuwestraat)
					{
						if(!wcstombs(mbs, adres[i].straat, ZWG_MAX_ALL))
							zetcel(xlsxWRITESHEET, "?");
						else
							zetcel(xlsxWRITESHEET, mbs);
						nieuwestraat = 0;
					}
					else
						zetcel(xlsxWRITESHEET, "");
					if(!wcstombs(mbs, adres[i].huisnr, ZWG_MAX_ALL))
						zetcel(xlsxWRITESHEET, "?");
					else
						zetcel(xlsxWRITESHEET, mbs);
					if(!wcstombs(mbs, adres[i].naam, ZWG_MAX_ALL))
						zetcel(xlsxWRITESHEET, "?");
					else
						zetcel(xlsxWRITESHEET, mbs);
					if(!wcstombs(mbs, adres[i].telefoonnr, ZWG_MAX_ALL))
						zetcel(xlsxWRITESHEET, "?");
					else
						zetcel(xlsxWRITESHEET, mbs);
				}
				HeapFree(hheap, 0, adres);
				break;
		}
		LOOPEND:;
	}

	if(debug)
		_textcolor(BROWN);
	/*'opkuis' aanvragen aan opzoekingWG*/
	opzoekingWG(NULL, NULL, L"", NULL, NULL, NULL, debug);
	workbook_close(xlsxWRITE);
	xlsxioread_sheet_close(xlsxREADSHEET);
	xlsxioread_close(xlsxREAD);
	while(_wremove(xlsx_IN))
	{
		_textcolor(RED);
		puts("\nHet bestaande (*.xlsx)-bestand kon niet vervangen worden.\n\
Mogelijks is het bestand nog geopend. Opnieuw proberen? [Y/N]");
		switch(_getch())
		{
			case 'y':
			case 'Y':
				break;
			case 0:
			case 224:
				_getch();
			default:
				_textcolor(GREEN);
				printf("Het bestand is opgeslagen als \'%s\' in\n \'%ls\'.\n", PAD_TMP, wcwd);
				return 0;
				break;
		}
	}
	_wrename(wPAD_TMP, xlsx_IN);
	return 0;

ABORT:
	xlsxioread_sheet_close(xlsxREADSHEET);
	xlsxioread_close(xlsxREAD);
	workbook_close(xlsxWRITE);
	/*'opkuis' aanvragen aan opzoekingWG*/
	opzoekingWG(NULL, NULL, L"", NULL, NULL, NULL, debug);
	return INCORRECT_XLSX;
}

char openxlsx(xlsxioreader *xlsxREAD, xlsxioreadersheet *xlsxREADSHEET)	//returns 0 on success
{
	char xlsxIN[wcslen(xlsx_IN)+5];
	if(wcstombs(xlsxIN, xlsx_IN, wcslen(xlsx_IN)+5) < 4)
	{
		if(debug)
		{
			_textcolor(LIGHTRED);
			printf("Probleem bij het omvormen van\n\'%ls\' naar een multibytestring.\n", xlsx_IN);
		}
		return 1;
	}
	*xlsxREAD = xlsxioread_open(xlsxIN);
	if(*xlsxREAD == NULL)
	{
		if(debug)
		{
			_textcolor(LIGHTRED);
			printf("xlsxioread_open(%s) faalde.\n", xlsxIN);
		}
		return 1;
	}
	*xlsxREADSHEET = xlsxioread_sheet_open(*xlsxREAD, NULL, XLSXIOREAD_SKIP_EMPTY_ROWS);
	if(*xlsxREADSHEET == NULL)
	{
		if(debug)
		{
			_textcolor(LIGHTRED);
			puts("xlsxioread_sheet_open(...) faalde.");
		}
		xlsxioread_close(*xlsxREAD);
		return 1;
	}
	if(!xlsxioread_sheet_next_row(*xlsxREADSHEET))		//needed for correct initialisation
		return 1;
	return 0;
}

char readyxlsx(lxw_workbook **xlsxWRITE, lxw_worksheet **xlsxWRITESHEET)	//returns 0 on success
{
	*xlsxWRITE = workbook_new(PAD_TMP);
	if(*xlsxWRITE == NULL)
	{
		if(debug)
		{
			_textcolor(LIGHTRED);
			puts("Er is onvoldoende geheugen om een nieuw xlsx-werkboek te maken!");
		}
		return 1;
	}
	*xlsxWRITESHEET = workbook_add_worksheet(*xlsxWRITE, "afwezigen");
	if(*xlsxWRITESHEET == NULL)
	{
		if(debug)
		{
			_textcolor(LIGHTRED);
			puts("Er is onvoldoende geheugen om een nieuw xlsx-werkboek te maken!");
		}
		workbook_close(*xlsxWRITE);
		remove(PAD_TMP);
		return 1;
	}
	worksheet_set_column(*xlsxWRITESHEET, 0, 0, 29.75, NULL);
	worksheet_set_column(*xlsxWRITESHEET, 1, 1, 12.4, NULL);
	worksheet_set_column(*xlsxWRITESHEET, 2, 2, 39.7, NULL);
	worksheet_set_column(*xlsxWRITESHEET, 3, 3, 17.0, NULL);
	return 0;
}

char *krijgcel(xlsxioreadersheet sheet)
{
	//xlsxioread slaat automatisch lege rijen over
	char *value = xlsxioread_sheet_next_cell(sheet);
	if(value != NULL)
		return value;
	if(!xlsxioread_sheet_next_row(sheet))
		return NULL;
	return xlsxioread_sheet_next_cell(sheet);
}

void zetcel(lxw_worksheet *sheet, char value[])
{
	//er wordt van uitgegaan dat de gegevens in de rijen STEEDS vier kolommen vullen
	static lxw_row_t rij = 0;
	static lxw_col_t kolom = 0;
	if(kolom == 4)
	{
		kolom = 0;
		++rij;
	}
	worksheet_write_string(sheet, rij, kolom, value, NULL);
	++kolom;
}
