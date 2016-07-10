LPSTR UnicodeStr4CodeToUTF8Str(LPSTR in)
{
	LPSTR temp;
	unsigned int code, help;
	CHAR utf8[5];
	size_t length;
	while(temp = StrChrA(in, '\\'), temp != NULL)
	{
		if((temp[1] == 'u')||(temp[1] == 'U'))
		{
			temp[0] = '\0';
			StringCchLengthA(in, STRSAFE_MAX_CCH, &length);
			CHAR A[length+1];
			StringCchCopyA(A, length+1, in);
			StringCchLengthA(temp+6, STRSAFE_MAX_CCH, &length);
			CHAR B[length+1];
			StringCchCopyA(B, length+1, temp+6);
			temp[6] = '\0';
			CHAR hex_code[7];
			StringCchCopyA(hex_code, 7, "0x");
			CharLowerA(temp+2);
			StringCchCatA(hex_code, 7, temp+2);
			if(StrToIntExA(hex_code, STIF_SUPPORT_HEX, (int *)&code) == TRUE)
			{
				help = code;
				if(code <= 0x007F)
				{
					utf8[0] = code;
					utf8[1] = '\0';
				}
				else if(code <= 0x07FF)
				{
					utf8[0] = 0xC0+(help >> 6);
					utf8[1] = 0x80+(code&0x3F);
					utf8[2] = '\0';
				}
				else if(code <= 0xFFFF)
				{
					utf8[0] = 0xE0+(help >> (6+6));
					help = code;
					utf8[1] = 0x80+((help&0xFFF) >> 6);
					utf8[2] = 0x80+(code&0x3F);
					utf8[3] = '\0';
				}
				else
				{
					utf8[0] = 0xF0+(help >> (6+6+6));
					help = code;
					utf8[1] = 0x80+((help&0x3FFFF) >> (6+6));
					help = code;
					utf8[2] = 0x80+((help&0xFFF) >> 6);
					utf8[3] = 0x80+(code&0x3F);
					utf8[4] = '\0';
				}
				StringCchCopyA(in, STRSAFE_MAX_CCH, A);	//the following copy and catenate requests are surely safe, since the inputstring must be longer than the converted string ("\uXXXX" to a maximum of 4 characters UTF-8 encoding)
				StringCchCatA(in, STRSAFE_MAX_CCH, utf8);
				StringCchCatA(in, STRSAFE_MAX_CCH, B);
			}
			else
			{
				StringCchCopyA(in, STRSAFE_MAX_CCH, A);
				StringCchCatA(in, STRSAFE_MAX_CCH, "\\");
				StringCchCatA(in, STRSAFE_MAX_CCH, hex_code+2);
				StringCchCatA(in, STRSAFE_MAX_CCH, B);
			}
		}
		else
			temp[0] = 1;
	}
	for(code=0; in[code] != '\0'; ++code)
	{
		if(in[code] == 1)
			in[code] = '\\';
	}
	return in;
}
