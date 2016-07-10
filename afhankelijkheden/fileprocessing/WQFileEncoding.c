char QFileEncoding(HANDLE hFile)
{
	LARGE_INTEGER curr_pos, cursor;
	cursor.QuadPart = 0;
	SetFilePointerEx(hFile, cursor, &curr_pos, FILE_CURRENT);
	SetFilePointerEx(hFile, cursor, NULL, FILE_BEGIN);
	CHAR x[6];
	DWORD bread;
	ReadFile(hFile, x, sizeof(CHAR)*6, &bread, NULL);
	if(((unsigned char)x[0] == 255)&&((unsigned char)x[1] == 254)&&(x[2] == 0)&&(x[3] == 0))	//BOM of UTF-32 (LE)
	{
		if(curr_pos.QuadPart < 4)
			curr_pos.QuadPart = 4;
		SetFilePointerEx(hFile, curr_pos, NULL, FILE_BEGIN);
		return 32;
	}
	if(((unsigned char)x[0] == 255)&&((unsigned char)x[1] == 254))	//BOM of UTF-16 (LE)
	{
		if(curr_pos.QuadPart < 2)
			curr_pos.QuadPart = 2;
		SetFilePointerEx(hFile, curr_pos, NULL, FILE_BEGIN);
		return 16;
	}
	if(((unsigned char)x[0] == 239)&&((unsigned char)x[1] == 187)&&((unsigned char)x[2] == 191))	//BOM of UTF-8
	{
		if(curr_pos.QuadPart < 3)
			curr_pos.QuadPart = 3;
		SetFilePointerEx(hFile, curr_pos, NULL, FILE_BEGIN);
		return 8;
	}
	if((x[0] == 0)||(x[1] == 0)||(x[2] == 0)||(x[3] == 0)||(x[4] == 0)||(x[5] == 0))	//assume UTF-16 (without BOM)
	{
		SetFilePointerEx(hFile, curr_pos, NULL, FILE_BEGIN);
		return 16;
	}
	SetFilePointerEx(hFile, curr_pos, NULL, FILE_BEGIN);
	return 8;	//else: assume UTF-8 (without BOM)
}
