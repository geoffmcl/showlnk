//**************************************
// Name: Ascii<-->Unicode conversion
// Description:Two small and simple functions for converting from Ascii to Unicode and vice-versa
// By: Eugene Ciloci
//
// Inputs:An Ascii or Unicode string
//
// Returns:On Success: true 
// On Error: false if any of the strings are NULL
//
// Assumes:Target string must be large enough to hold the converted string
//
// Side Effects:Modifies the target string
//
// This code is copyrighted and has
// limited warranties.Please see http://www.Planet-Source-Code.com/vb/scripts/ShowCode.asp?txtCodeId=3437&lngWId=3
//for details.
//**************************************

/*##############################################################################
CharsetConversion.cpp
Description: Ascii<->Unicode conversion functions
Author: Eugene Ciloci (ciloci@sympatico.ca)
Last Modified: Sat Mar 30 02:07:54 2002
##############################################################################*/
#include <Windows.h>
#include <string.h>
#include "conversions.hxx"
#include "sprtf.hxx"

/*------------------------------------------------------------------------------
Purpose: Convert an Ascii string to Unicode
Parameters: [in] szAscii - Ascii string to be converted
		[out] szUnicode - Unicode string to receive conversion
On Success: szUnicode contains converted string.
		Returns true
On Error: false if either string is NULL
------------------------------------------------------------------------------*/
bool AsciiToUnicode(const char * szAscii, wchar_t * szUnicode)
{
	int len, i;
	if((szUnicode == NULL) || (szAscii == NULL))
		return false;
	len = strlen(szAscii);
	for(i=0;i<len+1;i++)
		*szUnicode++ = static_cast<wchar_t>(*szAscii++);
	return true;
}

/*------------------------------------------------------------------------------
Purpose: Convert a Unicode string to Ascii
Parameters: [in] szUnicode - Unicode string to be converted
			[out] szAscii - Ascii string to receive conversion
On Success: szAscii contains converted string
		Returns true
On Error: false if either string is NULL
------------------------------------------------------------------------------*/
bool UnicodeToAscii(const wchar_t * szUnicode, char * szAscii)
{
	int len, i;
	if((szUnicode == NULL) || (szAscii == NULL))
		return false;
	len = wcslen(szUnicode);
	for(i=0;i<len+1;i++)
		*szAscii++ = static_cast<char>(*szUnicode++);
	return true;
}

// converts Unicode widechar string to UTF8 string
// return length, retrun 0 = error
//#define  DEF_CP   CP_ACP
#define  DEF_CP   CP_UTF8
int Wide2UTF8( wchar_t * pFile, char * ps, int len )
{
   //int wlen = lstrlen(pFile);
   int wlen = wcslen(pFile);
   // The number includes the byte for the null terminator
   int ret = WideCharToMultiByte( DEF_CP, // UINT CodePage,            // code page
      0, // DWORD dwFlags,            // performance and mapping flags
      pFile,   // LPCWSTR lpWideCharStr,    // wide-character string
      wlen,     // int cchWideChar,          // number of chars in string.
      ps,      // LPSTR lpMultiByteStr,     // buffer for new string
      len,    // int cbMultiByte,          // size of buffer
      NULL,    // LPCSTR lpDefaultChar,     // default for unmappable chars
      NULL );  // LPBOOL lpUsedDefaultChar  // set when default char used
   ps[ret] = 0;
   //if(ret)
   //   ps[ret-1] = 0;
   return ret;
}

void showConversionError()
{
    int res = GetLastError();
    sprtf("Error: %d: %s\n",
        ((res == ERROR_INSUFFICIENT_BUFFER) ?
        "ERROR_INSUFFICIENT_BUFFER" : // A supplied buffer size was not large enough, or it was incorrectly set to NULL.
            (res == ERROR_INVALID_FLAGS) ?
        "ERROR_INVALID_FLAGS" : // The values supplied for flags were not valid.
            (res == ERROR_INVALID_PARAMETER) ?
        "ERROR_INVALID_PARAMETER" : // Any of the parameter values was invalid.
            (res == ERROR_NO_UNICODE_TRANSLATION) ?
        "ERROR_NO_UNICODE_TRANSLATION" : // Invalid Unicode was found in a string.
            "UNLISTED_ERROR" ));
}

// eof - conversions.cxx
