/*
* This function locates a string in a (file) stream.
*
* SYNTAX
* Windows function
*	signed char SetFPByStr(HANDLE hFile, LPCTSTR lpSearch, PLARGE_INTEGER pStopOffset, PLARGE_INTEGER pFileOffset)
*
*	hFile		A handle to the (file) stream to perform the search on. Typically this handle is returned by a call to CreateFile.
*	lpSearch	A null-terminated string that contains the search string.
*	pStopOffset	An optional pointer to the variable that holds the offset off the current position of the file pointer of the (file) stream at which to stop the search. See further. This paramater can be NULL.
*	pFileOffset	An optional pointer to the variable that receives the offset (in bytes) from the beginning of the search to the (starting) location of the search string on the (file) stream. This paramater can be NULL.
*
* CRT function	!! UNAVAILABLE !!
*	signed char SetFPByStr(FILE *file, _TCHAR *Search, long long *StopOffset, long long *FileOffset)
*
* REMARKS
* The search will begin at the current position of the file pointer of the (file) stream.
* SetFPByStrI is the same function, except for the string comparison, which is not case sensitive.
*
* The function will end when either of the following possibillities occurs first:
* - the search string has zero length, its length exceeds STRSAFE_MAX_CCH or contains an invalid character, in which case the file pointer of the (file) stream will not be changed. Also, the function returns -2 and, if pFileOffset is not NULL, the function sets *pFileOffset to 0.
* - the first occurrence of the search string has been found, in which case the file pointer of the (file) stream will be set right after this occurrence. Also, the function returns 0.
* - end-of-file is reached, in which case the file pointer of the (file) stream will not be changed. Also, the function returns -1 and, if pFileOffset is not NULL, the function sets *pFileOffset to 0.
* - during the search, the file pointer of the (file) stream passed the offset located in pStopOffset, in which case the file pointer of the (file) stream will not be changed. Also, the function returns -3 and, if pFileOffset is not NULL, the function sets *pFileOffset to 0.
*
* MAKER: Sven Verlinden
*/

#ifndef _SETFPBYSTR_H
#define _SETFPBYSTR_H

#if (defined UNICODE)||(defined _UNICODE)
#define SetFPByStr SetFPByStrW
#define SetFPByStrI SetFPByStrIW
#else /*!UNICODE*/
#define SetFPByStr _SetFPByStr
#define SetFPByStrI _SetFPByStrI
#endif /*?UNICODE*/

#ifdef _WINDOWS_H
signed char SetFPByStrW(HANDLE hfile, LPCWSTR in_search, PLARGE_INTEGER pstopoffset, PLARGE_INTEGER pfileoffset);
signed char SetFPByStrIW(HANDLE hfile, LPCWSTR in_search, PLARGE_INTEGER pstopoffset, PLARGE_INTEGER pfileoffset);
//signed char _SetFPByStr(HANDLE hfile, LPCSTR in_search, PLARGE_INTEGER pstopoffset, PLARGE_INTEGER pfileoffset);
//signed char _SetFPByStrI(HANDLE hfile, LPCSTR in_search, PLARGE_INTEGER pstopoffset, PLARGE_INTEGER pfileoffset);
//#pragma comment(lib,"SetFPByStr_W.lib")
#include "fileprocessing\qfileencoding.h"
#include "fileprocessing\wsetfpbystr.c"

//#else /*!_WINDOWS_H*/

//#pragma comment(lib,"SetFPByStr_CRT.lib")
#endif /*?_WINDOWS_H*/

#endif /*_SETFPBYSTR_H*/
