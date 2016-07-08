#define VERSION L"v1.0a"

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

#define STRSAFE_NO_DEPRECATE
#include "opzoekingwg.h"

#pragma comment(lib,"shell32.lib")	//for ShellExecute
#ifndef _WINDOWS_H
#include <windows.h>
#endif
#undef SLIST_ENTRY		//there is a definition of SLIST_ENTRY in common.h (xlsxwriter)
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
wchar_t *xlsx_IN;

int help(void);
int do_exit(int exitcode);
char maaksjabloon(void);
char verwerkbestand(void);
char openxlsx(xlsxioreader *xlsxREAD, xlsxioreadersheet *xlsxREADSHEET);
char readyxlsx(lxw_workbook **xlsxWRITE, lxw_worksheet **xlsxWRITESHEET);
char *krijgcel(xlsxioreadersheet sheet);
void zetcel(lxw_worksheet *sheet, char value[]);

int wmain(int argc, wchar_t *argv[])	//returns 1 on succes
{
	if((argc == 3)&&(!wcscmp(argv[2], L"debug")))
		debug = 1;
	_textcolor(LIGHTGREEN);
	printf("\nZoekWG (via CLI) (%ls)\n", VERSION);
	_textcolor(GREEN);
	puts("maker: Sven Verlinden\nmaakt gebruik van: expat, zlib, libzip, xlsxio en xlsxwriter\n");
	if(debug)
	{
		_textcolor(DARKGRAY);
		printf("taalindeling (voor console): %s\n", setlocale(LC_ALL, ""));
	}
	if(debug)
	{
		_textcolor(DARKGRAY);
		printf("argv[0] = |%ls|\nargv[1] = |%ls|\ncd = |%ls|\n", argv[0], argv[1], _wgetcwd(NULL, 0));
	}
	if((argc < 2)||(argv[1][0] == L'-')||(argv[1][0] == L'/'))
		return do_exit(help());
	if(debug)
	{
		_textcolor(DARKGRAY);
		puts("Geen \'-\' of \'/\' als eerste teken van argument.");
	}
	wchar_t _IN[wcslen(argv[1])+1];
	wcscpy(_IN, argv[1]);
	xlsx_IN = _IN;
	if(debug)
	{
		_textcolor(DARKGRAY);
		printf("De tekenreeks van het argument is gekopieerd (%ls).\n", xlsx_IN);
	}
	return do_exit(verwerkbestand());
	/*switch(verwerkbestand())
	{
		case READ_FAILED:
			return do_exit(READ_FAILED);
			break;
		case WRITE_FAILED:
			return do_exit(WRITE_FAILED);
			break;
		case INCORRECT_XLSX:
			return do_exit(INCORRECT_XLSX);
			break;
		case 0:
			_wremove(xlsx_IN);
			_wrename(wPAD_TMP, xlsx_IN);
			return do_exit(0);
			break;
		default:
			return do_exit(1);
			break;
	}
	return 1;*/
}

int help(void)	//returns exit return code
{
	_textcolor(YELLOW);
	puts("ZoekWG vult een Microsoft Excel 2007/2010/2013 XML (*.xlsx)-bestand aan\n\
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
		case 0:
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
	printf("Een in te vullen sjabloon werd gemaakt in\n\'%ls\', met als naam\n\
\'%s\'.\nDit bestand nu openen? [Y/N]\n", _wgetcwd(NULL, 0), PAD_SJABLOON);
	switch(_getch())
	{
		case 'y':
		case 'Y':
			{
			int ret = (int)ShellExecuteA(NULL, "open", PAD_SJABLOON, NULL, NULL, SW_SHOWNORMAL);
			if(ret <= 32)
			{
				_textcolor(LIGHTRED);
				puts("\nHet bestand kon niet geopend worden!");
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
	putchar('\n');
	_textcolor(WHITE);
	return 0;
}

int do_exit(int exitcode)
{
	_textcolor(RED);
	switch(exitcode)
	{
		case SJAB_FAILED:
			puts("Er kon geen sjabloon gemaakt worden.");
			break;
		case READ_FAILED:
			puts("Er is een probleem met het inlezen van het bestand.");
			break;
		case WRITE_FAILED:
			puts("Er is een probleem met het aanmaken van een (*.xlsx)-bestand.");
			break;
		case INCORRECT_XLSX:
			puts("Het ingevoerde (*.xlsx)-bestand voldoet niet aan de verwachtingen.");
			break;
		case 0:
			_textcolor(LIGHTGREEN);
			puts("De verwerking is succesvol voltooid!");
			break;
		default:
			puts("Er is een probleem opgetreden.");
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
	putchar('\n');
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
			puts("Er is onvoldoende geheugen om een nieuw xlsx-werkboek te maken!");
		}
		return 1;
	}
	lxw_worksheet *SJAB_ws = workbook_add_worksheet(SJAB_wb, "afwezigen");
	if(SJAB_ws == NULL)
	{
		if(debug)
		{
			_textcolor(LIGHTRED);
			puts("Er is onvoldoende geheugen om een nieuw xlsx-werkboek te maken!");
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
	if(workbook_close(SJAB_wb) != LXW_NO_ERROR)
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
	wchar_t gemeente[MAX_GEMEENTE];
	gemeente[0] = L'0';
	while(celIN = krijgcel(xlsxREADSHEET), (celIN != NULL)&&(_stristr(celIN, "straat") == NULL))
	{
		if((gemeente[0] == L'1')&&(mbstowcs(gemeente, celIN, MAX_GEMEENTE) < 2))
		{
			if(debug)
			{
				_textcolor(LIGHTRED);
				printf("Probleem bij het omvormen van\n\'%s\' naar een \'wide character\'-string.\n", celIN);
			}
			free(celIN);
			goto ABORT;
		}
		if(_stristr(celIN, "gemeente") != NULL)
		{
			zetcel(xlsxWRITESHEET, "");		zetcel(xlsxWRITESHEET, "");
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
	if((celIN == NULL)||(free(celIN), celIN = krijgcel(xlsxREADSHEET), celIN == NULL))
	{
		if(debug)
		{
			_textcolor(LIGHTRED);
			puts("Er zijn geen gegevens (meer) ingelezen voor of bij het woord \'straat\'.");
		}
		goto ABORT;
	}
	free(celIN);
	int i;
	for(i = 0; i != 6; ++i)
		zetcel(xlsxWRITESHEET, "");
	zetcel(xlsxWRITESHEET, "Straatnaam");	zetcel(xlsxWRITESHEET, "Huisnummer");	zetcel(xlsxWRITESHEET, "Naam");	zetcel(xlsxWRITESHEET, "Telefoonnummer");

	wchar_t straat[MAX_STRAAT], huisnr[MAX_HUISNUMMER];
	straat[0] = L'0';
	HANDLE hheap = GetProcessHeap();
	ADRES *adres;
	char nieuwestraat, mbs[MAX_ALL];
	int ret;
	while(celIN = krijgcel(xlsxREADSHEET), celIN != NULL)
	{
		if(celIN[0] != '\0')
		{
			nieuwestraat = 1;
			if(straat[0] != L'0')
			{
				for(i = 0; i != 4; ++i)
					zetcel(xlsxWRITESHEET, "");
			}
			if(mbstowcs(straat, celIN, MAX_STRAAT) < 2)
			{
				if(debug)
				{
					_textcolor(LIGHTRED);
					printf("Probleem bij het omvormen van\n\'%s\' naar een \'wide character\'-string.\n", celIN);
				}
				free(celIN);
				goto ABORT;
			}
		}
		else
			nieuwestraat = 0;
		free(celIN);
		celIN = krijgcel(xlsxREADSHEET);
		if(!mbstowcs(huisnr, celIN, MAX_HUISNUMMER))
		{
			if(debug)
			{
				_textcolor(LIGHTRED);
				printf("Probleem bij het omvormen van\n\'%s\' naar een \'wide character\'-string.\n", celIN);
			}
			free(celIN);
			goto ABORT;
		}
		free(celIN);
		if(huisnr[0] == L'\0')
		{
			if(nieuwestraat)
			{
				if(!wcstombs(mbs, straat, MAX_ALL))
					zetcel(xlsxWRITESHEET, "?");
				else
					zetcel(xlsxWRITESHEET, mbs);
				for(i=0; i != 3; ++i)
					zetcel(xlsxWRITESHEET, "");
			}
			goto LOOPEND;
		}
		if(debug)
			_textcolor(BROWN);
		ret = opzoekingWG(NULL, hheap, gemeente, straat, huisnr, &adres, debug);
		if(ret < 1)
		{
			if(nieuwestraat)
			{
				if(!wcstombs(mbs, straat, MAX_ALL))
					zetcel(xlsxWRITESHEET, "?");
				else
					zetcel(xlsxWRITESHEET, mbs);
			}
			else
				zetcel(xlsxWRITESHEET, "");
			if(!wcstombs(mbs, huisnr, MAX_ALL))
				zetcel(xlsxWRITESHEET, "?");
			else
				zetcel(xlsxWRITESHEET, mbs);
			zetcel(xlsxWRITESHEET, "");
			_textcolor(RED);
			printf("%ls %ls -> ", straat, huisnr);
		}
		switch(ret)
		{
			case BUG_1:
				zetcel(xlsxWRITESHEET, "Fout in programma");
				puts("Fout in programma");
				break;
			case NOT_DOWNLOADED:
				zetcel(xlsxWRITESHEET, "Downloadprobleem");
				puts("Downloadprobleem");
				break;
			case SERVER_UNAVAILABLE:
				zetcel(xlsxWRITESHEET, "wittegids.be onbereikbaar");
				puts("wittegids.be onbereikbaar");
				break;
			case ADDRESS_NOT_IN_WG:
				zetcel(xlsxWRITESHEET, "Adres onbeschikbaar");
				puts("Adres onbeschikbaar");
				break;
			case TELEPHONE_NOT_IN_WG:
				zetcel(xlsxWRITESHEET, "Telefoonnr onbeschikbaar");
				puts("Telefoonnr onbeschikbaar");
				break;
			case MEMORY_ERROR:
				zetcel(xlsxWRITESHEET, "Geheugenprobleem");
				puts("Geheugenprobleem");
				break;
			default:
				for(i = 0; i != ret; ++i)
				{
					if(nieuwestraat)
					{
						if(!wcstombs(mbs, adres[i].straat, MAX_ALL))
							zetcel(xlsxWRITESHEET, "?");
						else
							zetcel(xlsxWRITESHEET, mbs);
						nieuwestraat = 0;
					}
					else
						zetcel(xlsxWRITESHEET, "");
					if(!wcstombs(mbs, adres[i].huisnr, MAX_ALL))
						zetcel(xlsxWRITESHEET, "?");
					else
						zetcel(xlsxWRITESHEET, mbs);
					if(!wcstombs(mbs, adres[i].naam, MAX_ALL))
						zetcel(xlsxWRITESHEET, "?");
					else
						zetcel(xlsxWRITESHEET, mbs);
					if(!wcstombs(mbs, adres[i].telefoonnr, MAX_ALL))
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
	opzoekingWG(NULL, NULL, L"", NULL, NULL, NULL, debug);
	workbook_close(xlsxWRITE);
	xlsxioread_sheet_close(xlsxREADSHEET);
	xlsxioread_close(xlsxREAD);
	while(_wremove(xlsx_IN))
	{
		_textcolor(RED);
		puts("Het bestaande (*.xlsx)-bestand kon niet vervangen worden.\n\
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
				printf("Het bestand is opgeslagen als \'%s\' in \'%ls\'.\n", PAD_TMP, _wgetcwd(NULL, 0));
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
	char *value = xlsxioread_sheet_next_cell(sheet);
	if(value != NULL)
		return value;
	if(!xlsxioread_sheet_next_row(sheet))
		return NULL;
	return xlsxioread_sheet_next_cell(sheet);
}

void zetcel(lxw_worksheet *sheet, char value[])
{
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
