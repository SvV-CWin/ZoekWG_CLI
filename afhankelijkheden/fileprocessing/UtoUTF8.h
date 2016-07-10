/*
* Replaces a null-terminated (non-wide) character string that has Unicode codes in it (in the form "\uXXXX" or "\UXXXX", for example from a html-page) with the same string wherein the codes are replaced with the corresponding UTF-8 characters
* -> LPSTR UnicodeStr4CodeToUTF8Str(LPSTR in)
*
* maker: Sven Verlinden
*/

#ifndef _UTOUTF8_H
#define _UTOUTF8_H

LPSTR UnicodeStr4CodeToUTF8Str(LPSTR in);
#include "fileprocessing\wutoutf8.c"

#endif  /*?_WINDOWS_H*/

#endif /*_UTOUTF8_H*/
