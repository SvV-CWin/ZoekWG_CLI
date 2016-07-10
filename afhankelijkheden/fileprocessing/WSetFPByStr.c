#define MIN_MEM_STR 1024*10+1

signed char SetFPByStrW(HANDLE hfile, LPCWSTR in_search, PLARGE_INTEGER pstopoffset, PLARGE_INTEGER pfileoffset)
{
	size_t length;
	if((FAILED(StringCchLengthW(in_search, STRSAFE_MAX_CCH, &length)) == TRUE)||(length == 0))
	{
		if(pfileoffset != NULL)
			(*pfileoffset).QuadPart = 0;
		return -2;
	}
	/*define 'search' and 'take' and make sure they are UTF-8 encoded*/
	int encoding = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, in_search, -1, NULL, 0, NULL, NULL);
	if((encoding == (int)L'\uFFFD')||(encoding == 0))
	{
		if(pfileoffset != NULL)
			(*pfileoffset).QuadPart = 0;
		return -2;
	}
	CHAR search[encoding], take[(encoding > MIN_MEM_STR)*encoding + (encoding <= MIN_MEM_STR)*MIN_MEM_STR], *occurrence, *duplicate;
	WCHAR wtake[(length+1 > MIN_MEM_STR)*(length+1) + (length+1 <= MIN_MEM_STR)*MIN_MEM_STR];
	WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, in_search, -1, search, encoding, NULL, NULL);
	/*end definition*/
	encoding = QFileEncoding(hfile);
	signed char returnvalue=1;
	LARGE_INTEGER fileoffset, startoffset;
	fileoffset.QuadPart = 0;
	DWORD b;
	size_t length2;
	startoffset.QuadPart = 0;
	SetFilePointerEx(hfile, startoffset, &startoffset, FILE_CURRENT);
	do
	{
		if(encoding == 8)
			ReadFile(hfile, take, sizeof(take)-sizeof(take[0]), &b, NULL);
		else
			ReadFile(hfile, wtake, sizeof(wtake)-sizeof(wtake[0]), &b, NULL);
		if(b == 0)
		{
			fileoffset.QuadPart = 0;
			returnvalue = -1;
			SetFilePointerEx(hfile, startoffset, NULL, FILE_BEGIN);
			break;
		}
		fileoffset.QuadPart += (LONGLONG)b;
		if(encoding == 8)
			take[b] = '\0';
		else
		{
			wtake[b/sizeof(wtake[0])] = L'\0';
			WideCharToMultiByte(CP_UTF8, 0, wtake, -1, take, sizeof(take), NULL, NULL);
		}
		while(returnvalue == 1)
		{
			if(occurrence = StrStrA(take, search), occurrence != NULL)	//check if the string has been found
			{
				if(encoding == 8)
					StringCchLengthA(occurrence, STRSAFE_MAX_CCH, &length);
				else
					length = MultiByteToWideChar(CP_UTF8, 0, occurrence, -1, NULL, 0);
				fileoffset.QuadPart -= (LONGLONG)length;
				if((pstopoffset != NULL)&&(fileoffset.QuadPart >= (*pstopoffset).QuadPart))	//check if the found string is located after StopOffset
				{
					SetFilePointerEx(hfile, startoffset, NULL, FILE_BEGIN);
					fileoffset.QuadPart = 0;
					returnvalue = -3;
					break;
				}
				returnvalue = 0;
				if(encoding == 8)
					StringCchLengthA(search, STRSAFE_MAX_CCH, &length2);
				else
				{
					StringCchLengthW(in_search, STRSAFE_MAX_CCH, &length2);
					length2 *= sizeof(wtake[0]);
				}
				startoffset.QuadPart = (LONGLONG)length2 - (LONGLONG)length;
				SetFilePointerEx(hfile, startoffset, NULL, FILE_CURRENT);
				break;
			}
			if(encoding == 8)
				StringCchLengthA(search, STRSAFE_MAX_CCH, &length);
			else
			{
				StringCchLengthW(in_search, STRSAFE_MAX_CCH, &length);
				length *= sizeof(wtake[0]);
			}
			if((pstopoffset != NULL)&&(fileoffset.QuadPart >= (*pstopoffset).QuadPart + (LONGLONG)length - 1))	//check if further reading would pass StopOffset
			{
				SetFilePointerEx(hfile, startoffset, NULL, FILE_BEGIN);
				fileoffset.QuadPart = 0;
				returnvalue = -3;
				break;
			}
			StringCchLengthA(search, STRSAFE_MAX_CCH, &length);
			StringCchLengthA(take, STRSAFE_MAX_CCH, &length2);
			if(occurrence = StrChrA(take+length2-length+1, *search), occurrence != NULL)	//shift part of string that begins with the first character of the search string and append next read
			{
				duplicate = StrDupA(occurrence);
				StringCchCopyA(take, length2, duplicate);
				LocalFree(duplicate);
				StringCchLengthA(take, STRSAFE_MAX_CCH, &length);
				if(encoding == 8)
					ReadFile(hfile, &take[length], sizeof(take)-length-1, &b, NULL);
				else
				{
					MultiByteToWideChar(CP_UTF8, 0, take, -1, wtake, sizeof(wtake)/sizeof(wtake[0]));
					StringCchLengthW(wtake, STRSAFE_MAX_CCH, &length);
					ReadFile(hfile, &wtake[length], sizeof(wtake)-length*sizeof(wtake[0])-sizeof(wtake[0]), &b, NULL);
				}
				if(b == 0)
				{
					fileoffset.QuadPart = 0;
					returnvalue = -1;
					SetFilePointerEx(hfile, startoffset, NULL, FILE_BEGIN);
					break;
				}
				fileoffset.QuadPart += (LONGLONG)b;
				if(encoding == 8)
					take[b+length] = '\0';
				else
				{
					wtake[(b+length*sizeof(wtake[0]))/sizeof(wtake[0])] = L'\0';
					WideCharToMultiByte(CP_UTF8, 0, wtake, -1, take, sizeof(take), NULL, NULL);
				}
			}
			else
				break;
		}
	} while(returnvalue == 1);
	if(pfileoffset != NULL)
		*pfileoffset = fileoffset;
	return returnvalue;
}

signed char SetFPByStrIW(HANDLE hfile, LPCWSTR in_search, PLARGE_INTEGER pstopoffset, PLARGE_INTEGER pfileoffset)
{
	size_t length;
	if((FAILED(StringCchLengthW(in_search, STRSAFE_MAX_CCH, &length)) == TRUE)||(length == 0))
	{
		if(pfileoffset != NULL)
			(*pfileoffset).QuadPart = 0;
		return -2;
	}
	/*define 'search' and 'take' and make sure they are UTF-8 encoded*/
	int encoding = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, in_search, -1, NULL, 0, NULL, NULL);
	if((encoding == (int)L'\uFFFD')||(encoding == 0))
	{
		if(pfileoffset != NULL)
			(*pfileoffset).QuadPart = 0;
		return -2;
	}
	CHAR search[encoding], take[(encoding > MIN_MEM_STR)*encoding + (encoding <= MIN_MEM_STR)*MIN_MEM_STR], *occurrence, *duplicate;
	WCHAR wtake[(length+1 > MIN_MEM_STR)*(length+1) + (length+1 <= MIN_MEM_STR)*MIN_MEM_STR];
	WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, in_search, -1, search, encoding, NULL, NULL);
	/*end definition*/
	encoding = QFileEncoding(hfile);
	signed char returnvalue=1;
	LARGE_INTEGER fileoffset, startoffset;
	fileoffset.QuadPart = 0;
	DWORD b;
	size_t length2;
	startoffset.QuadPart = 0;
	SetFilePointerEx(hfile, startoffset, &startoffset, FILE_CURRENT);
	do
	{
		if(encoding == 8)
			ReadFile(hfile, take, sizeof(take)-sizeof(take[0]), &b, NULL);
		else
			ReadFile(hfile, wtake, sizeof(wtake)-sizeof(wtake[0]), &b, NULL);
		if(b == 0)
		{
			fileoffset.QuadPart = 0;
			returnvalue = -1;
			SetFilePointerEx(hfile, startoffset, NULL, FILE_BEGIN);
			break;
		}
		fileoffset.QuadPart += (LONGLONG)b;
		if(encoding == 8)
			take[b] = '\0';
		else
		{
			wtake[b/sizeof(wtake[0])] = L'\0';
			WideCharToMultiByte(CP_UTF8, 0, wtake, -1, take, sizeof(take), NULL, NULL);
		}
		while(returnvalue == 1)
		{
			if(occurrence = StrStrIA(take, search), occurrence != NULL)	//check if the string has been found
			{
				if(encoding == 8)
					StringCchLengthA(occurrence, STRSAFE_MAX_CCH, &length);
				else
					length = MultiByteToWideChar(CP_UTF8, 0, occurrence, -1, NULL, 0);
				fileoffset.QuadPart -= (LONGLONG)length;
				if((pstopoffset != NULL)&&(fileoffset.QuadPart >= (*pstopoffset).QuadPart))	//check if the found string is located after StopOffset
				{
					SetFilePointerEx(hfile, startoffset, NULL, FILE_BEGIN);
					fileoffset.QuadPart = 0;
					returnvalue = -3;
					break;
				}
				returnvalue = 0;
				if(encoding == 8)
					StringCchLengthA(search, STRSAFE_MAX_CCH, &length2);
				else
				{
					StringCchLengthW(in_search, STRSAFE_MAX_CCH, &length2);
					length2 *= sizeof(wtake[0]);
				}
				startoffset.QuadPart = (LONGLONG)length2 - (LONGLONG)length;
				SetFilePointerEx(hfile, startoffset, NULL, FILE_CURRENT);
				break;
			}
			if(encoding == 8)
				StringCchLengthA(search, STRSAFE_MAX_CCH, &length);
			else
			{
				StringCchLengthW(in_search, STRSAFE_MAX_CCH, &length);
				length *= sizeof(wtake[0]);
			}
			if((pstopoffset != NULL)&&(fileoffset.QuadPart >= (*pstopoffset).QuadPart + (LONGLONG)length - 1))	//check if further reading would pass StopOffset
			{
				SetFilePointerEx(hfile, startoffset, NULL, FILE_BEGIN);
				fileoffset.QuadPart = 0;
				returnvalue = -3;
				break;
			}
			StringCchLengthA(search, STRSAFE_MAX_CCH, &length);
			StringCchLengthA(take, STRSAFE_MAX_CCH, &length2);
			if(occurrence = StrChrIA(take+length2-length+1, *search), occurrence != NULL)	//shift part of string that begins with the first character of the search string and append next read
			{
				duplicate = StrDupA(occurrence);
				StringCchCopyA(take, length2, duplicate);
				LocalFree(duplicate);
				StringCchLengthA(take, STRSAFE_MAX_CCH, &length);
				if(encoding == 8)
					ReadFile(hfile, &take[length], sizeof(take)-length-1, &b, NULL);
				else
				{
					MultiByteToWideChar(CP_UTF8, 0, take, -1, wtake, sizeof(wtake)/sizeof(wtake[0]));
					StringCchLengthW(wtake, STRSAFE_MAX_CCH, &length);
					ReadFile(hfile, &wtake[length], sizeof(wtake)-length*sizeof(wtake[0])-sizeof(wtake[0]), &b, NULL);
				}
				if(b == 0)
				{
					fileoffset.QuadPart = 0;
					returnvalue = -1;
					SetFilePointerEx(hfile, startoffset, NULL, FILE_BEGIN);
					break;
				}
				fileoffset.QuadPart += (LONGLONG)b;
				if(encoding == 8)
					take[b+length] = '\0';
				else
				{
					wtake[(b+length*sizeof(wtake[0]))/sizeof(wtake[0])] = L'\0';
					WideCharToMultiByte(CP_UTF8, 0, wtake, -1, take, sizeof(take), NULL, NULL);
				}
			}
			else
				break;
		}
	} while(returnvalue == 1);
	if(pfileoffset != NULL)
		*pfileoffset = fileoffset;
	return returnvalue;
}

signed char SetFPByStrA(HANDLE hfile, LPCSTR search, PLARGE_INTEGER pstopoffset, PLARGE_INTEGER pfileoffset)
{
	size_t length;
	if((FAILED(StringCchLengthA(search, STRSAFE_MAX_CCH, &length)) == TRUE)||(length == 0))
	{
		if(pfileoffset != NULL)
			(*pfileoffset).QuadPart = 0;
		return -2;
	}
	CHAR take[(length+1 > MIN_MEM_STR)*(length+1) + (length+1 <= MIN_MEM_STR)*MIN_MEM_STR], *occurrence, *duplicate;
	length = MultiByteToWideChar(CP_UTF8, 0, search, -1, NULL, 0);
	WCHAR wtake[(length > MIN_MEM_STR)*(length) + (length <= MIN_MEM_STR)*MIN_MEM_STR];
	int encoding = QFileEncoding(hfile);
	signed char returnvalue=1;
	LARGE_INTEGER fileoffset, startoffset;
	fileoffset.QuadPart = 0;
	DWORD b;
	size_t length2;
	startoffset.QuadPart = 0;
	SetFilePointerEx(hfile, startoffset, &startoffset, FILE_CURRENT);
	do
	{
		if(encoding == 8)
			ReadFile(hfile, take, sizeof(take)-sizeof(take[0]), &b, NULL);
		else
			ReadFile(hfile, wtake, sizeof(wtake)-sizeof(wtake[0]), &b, NULL);
		if(b == 0)
		{
			fileoffset.QuadPart = 0;
			returnvalue = -1;
			SetFilePointerEx(hfile, startoffset, NULL, FILE_BEGIN);
			break;
		}
		fileoffset.QuadPart += (LONGLONG)b;
		if(encoding == 8)
			take[b] = '\0';
		else
		{
			wtake[b/sizeof(wtake[0])] = L'\0';
			WideCharToMultiByte(CP_UTF8, 0, wtake, -1, take, sizeof(take), NULL, NULL);
		}
		while(returnvalue == 1)
		{
			if(occurrence = StrStrA(take, search), occurrence != NULL)	//check if the string has been found
			{
				if(encoding == 8)
					StringCchLengthA(occurrence, STRSAFE_MAX_CCH, &length);
				else
					length = MultiByteToWideChar(CP_UTF8, 0, occurrence, -1, NULL, 0);
				fileoffset.QuadPart -= (LONGLONG)length;
				if((pstopoffset != NULL)&&(fileoffset.QuadPart >= (*pstopoffset).QuadPart))	//check if the found string is located after StopOffset
				{
					SetFilePointerEx(hfile, startoffset, NULL, FILE_BEGIN);
					fileoffset.QuadPart = 0;
					returnvalue = -3;
					break;
				}
				returnvalue = 0;
				if(encoding == 8)
					StringCchLengthA(search, STRSAFE_MAX_CCH, &length2);
				else
					length2 = MultiByteToWideChar(CP_UTF8, 0, search, -1, NULL, 0)*sizeof(WCHAR);
				startoffset.QuadPart = (LONGLONG)length2 - (LONGLONG)length;
				SetFilePointerEx(hfile, startoffset, NULL, FILE_CURRENT);
				break;
			}
			if(encoding == 8)
				StringCchLengthA(search, STRSAFE_MAX_CCH, &length);
			else
				length = MultiByteToWideChar(CP_UTF8, 0, search, -1, NULL, 0)*sizeof(WCHAR);
			if((pstopoffset != NULL)&&(fileoffset.QuadPart >= (*pstopoffset).QuadPart + (LONGLONG)length - 1))	//check if further reading would pass StopOffset
			{
				SetFilePointerEx(hfile, startoffset, NULL, FILE_BEGIN);
				fileoffset.QuadPart = 0;
				returnvalue = -3;
				break;
			}
			StringCchLengthA(search, STRSAFE_MAX_CCH, &length);
			StringCchLengthA(take, STRSAFE_MAX_CCH, &length2);
			if(occurrence = StrChrA(take+length2-length+1, *search), occurrence != NULL)	//shift part of string that begins with the first character of the search string and append next read
			{
				duplicate = StrDupA(occurrence);
				StringCchCopyA(take, length2, duplicate);
				LocalFree(duplicate);
				StringCchLengthA(take, STRSAFE_MAX_CCH, &length);
				if(encoding == 8)
					ReadFile(hfile, &take[length], sizeof(take)-length-1, &b, NULL);
				else
				{
					MultiByteToWideChar(CP_UTF8, 0, take, -1, wtake, sizeof(wtake)/sizeof(wtake[0]));
					StringCchLengthW(wtake, STRSAFE_MAX_CCH, &length);
					ReadFile(hfile, &wtake[length], sizeof(wtake)-length*sizeof(wtake[0])-sizeof(wtake[0]), &b, NULL);
				}
				if(b == 0)
				{
					fileoffset.QuadPart = 0;
					returnvalue = -1;
					SetFilePointerEx(hfile, startoffset, NULL, FILE_BEGIN);
					break;
				}
				fileoffset.QuadPart += (LONGLONG)b;
				if(encoding == 8)
					take[b+length] = '\0';
				else
				{
					wtake[(b+length*sizeof(wtake[0]))/sizeof(wtake[0])] = L'\0';
					WideCharToMultiByte(CP_UTF8, 0, wtake, -1, take, sizeof(take), NULL, NULL);
				}
			}
			else
				break;
		}
	} while(returnvalue == 1);
	if(pfileoffset != NULL)
		*pfileoffset = fileoffset;
	return returnvalue;
}

signed char SetFPByStrIA(HANDLE hfile, LPCSTR search, PLARGE_INTEGER pstopoffset, PLARGE_INTEGER pfileoffset)
{
	size_t length;
	if((FAILED(StringCchLengthA(search, STRSAFE_MAX_CCH, &length)) == TRUE)||(length == 0))
	{
		if(pfileoffset != NULL)
			(*pfileoffset).QuadPart = 0;
		return -2;
	}
	int encoding = QFileEncoding(hfile);
	CHAR take[(length+1 > MIN_MEM_STR)*(length+1) + (length+1 <= MIN_MEM_STR)*MIN_MEM_STR], *occurrence, *duplicate;
	length = MultiByteToWideChar(CP_UTF8, 0, search, -1, NULL, 0);
	WCHAR wtake[(length > MIN_MEM_STR)*(length) + (length <= MIN_MEM_STR)*MIN_MEM_STR];
	signed char returnvalue=1;
	LARGE_INTEGER fileoffset, startoffset;
	fileoffset.QuadPart = 0;
	DWORD b;
	size_t length2;
	startoffset.QuadPart = 0;
	SetFilePointerEx(hfile, startoffset, &startoffset, FILE_CURRENT);
	do
	{
		if(encoding == 8)
			ReadFile(hfile, take, sizeof(take)-sizeof(take[0]), &b, NULL);
		else
			ReadFile(hfile, wtake, sizeof(wtake)-sizeof(wtake[0]), &b, NULL);
		if(b == 0)
		{
			fileoffset.QuadPart = 0;
			returnvalue = -1;
			SetFilePointerEx(hfile, startoffset, NULL, FILE_BEGIN);
			break;
		}
		fileoffset.QuadPart += (LONGLONG)b;
		if(encoding == 8)
			take[b] = '\0';
		else
		{
			wtake[b/sizeof(wtake[0])] = L'\0';
			WideCharToMultiByte(CP_UTF8, 0, wtake, -1, take, sizeof(take), NULL, NULL);
		}
		while(returnvalue == 1)
		{
			if(occurrence = StrStrIA(take, search), occurrence != NULL)	//check if the string has been found
			{
				if(encoding == 8)
					StringCchLengthA(occurrence, STRSAFE_MAX_CCH, &length);
				else
					length = MultiByteToWideChar(CP_UTF8, 0, occurrence, -1, NULL, 0);
				fileoffset.QuadPart -= (LONGLONG)length;
				if((pstopoffset != NULL)&&(fileoffset.QuadPart >= (*pstopoffset).QuadPart))	//check if the found string is located after StopOffset
				{
					SetFilePointerEx(hfile, startoffset, NULL, FILE_BEGIN);
					fileoffset.QuadPart = 0;
					returnvalue = -3;
					break;
				}
				returnvalue = 0;
				if(encoding == 8)
					StringCchLengthA(search, STRSAFE_MAX_CCH, &length2);
				else
					length2 = MultiByteToWideChar(CP_UTF8, 0, search, -1, NULL, 0)*sizeof(WCHAR);
				startoffset.QuadPart = (LONGLONG)length2 - (LONGLONG)length;
				SetFilePointerEx(hfile, startoffset, NULL, FILE_CURRENT);
				break;
			}
			if(encoding == 8)
				StringCchLengthA(search, STRSAFE_MAX_CCH, &length);
			else
				length = MultiByteToWideChar(CP_UTF8, 0, search, -1, NULL, 0)*sizeof(WCHAR);
			if((pstopoffset != NULL)&&(fileoffset.QuadPart >= (*pstopoffset).QuadPart + (LONGLONG)length - 1))	//check if further reading would pass StopOffset
			{
				SetFilePointerEx(hfile, startoffset, NULL, FILE_BEGIN);
				fileoffset.QuadPart = 0;
				returnvalue = -3;
				break;
			}
			StringCchLengthA(search, STRSAFE_MAX_CCH, &length);
			StringCchLengthA(take, STRSAFE_MAX_CCH, &length2);
			if(occurrence = StrChrIA(take+length2-length+1, *search), occurrence != NULL)	//shift part of string that begins with the first character of the search string and append next read
			{
				duplicate = StrDupA(occurrence);
				StringCchCopyA(take, length2, duplicate);
				LocalFree(duplicate);
				StringCchLengthA(take, STRSAFE_MAX_CCH, &length);
				if(encoding == 8)
					ReadFile(hfile, &take[length], sizeof(take)-length-1, &b, NULL);
				else
				{
					MultiByteToWideChar(CP_UTF8, 0, take, -1, wtake, sizeof(wtake)/sizeof(wtake[0]));
					StringCchLengthW(wtake, STRSAFE_MAX_CCH, &length);
					ReadFile(hfile, &wtake[length], sizeof(wtake)-length*sizeof(wtake[0])-sizeof(wtake[0]), &b, NULL);
				}
				if(b == 0)
				{
					fileoffset.QuadPart = 0;
					returnvalue = -1;
					SetFilePointerEx(hfile, startoffset, NULL, FILE_BEGIN);
					break;
				}
				fileoffset.QuadPart += (LONGLONG)b;
				if(encoding == 8)
					take[b+length] = '\0';
				else
				{
					wtake[(b+length*sizeof(wtake[0]))/sizeof(wtake[0])] = L'\0';
					WideCharToMultiByte(CP_UTF8, 0, wtake, -1, take, sizeof(take), NULL, NULL);
				}
			}
			else
				break;
		}
	} while(returnvalue == 1);
	if(pfileoffset != NULL)
		*pfileoffset = fileoffset;
	return returnvalue;
}
