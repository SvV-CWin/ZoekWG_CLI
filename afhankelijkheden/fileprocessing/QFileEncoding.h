/*
* Returns 32, 16 or 8, according to the encoding of the file: UTF-32, UTF-16 or UTF-8 respectively.
* The function assumes LE encoding and uses the BOM (or the presence of null-bytes in the first 6 bytes of the file) to determine the encoding.
* -> char QFileEncoding(HANDLE hFile)
*
* maker: Sven Verlinden
*/

#ifndef _QFILEENCODING_H
#define _QFILEENCODING_H

char QFileEncoding(HANDLE hFile);
#include "fileprocessing\wqfileencoding.c"

#endif /*_QFILEENCODING_H*/
