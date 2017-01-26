//**************************************
//INCLUDE files for :Ascii<-->Unicode conversion
//**************************************
/*##############################################################################
CharsetConversion.h
Description: Ascii<->Unicode conversion
Author: Eugene Ciloci (ciloci@sympatico.ca) 
Last Modified: Sat Mar 30 02:07:54 2002
##############################################################################*/
#ifndef CHARSET_CONVERSION
#define CHARSET_CONVERSION
#include <stddef.h>

extern bool AsciiToUnicode(const char * szAscii, wchar_t * szUnicode);
extern bool UnicodeToAscii(const wchar_t * szUnicode, char * szAscii);
// return length, 0 = error
extern int Wide2UTF8( wchar_t * pFile, char * ps, int len );
extern void showConversionError(); // GetLastError();

#endif

// eof
