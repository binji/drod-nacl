// $Id: Wchar.h 9791 2011-12-06 14:27:31Z trick $

/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Deadly Rooms of Death.
 *
 * The Initial Developer of the Original Code is
 * Caravel Software.
 * Portions created by the Initial Developer are Copyright (C) 2002, 2005
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 * Matt Schikore (schik)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef WCHAR_H
#define WCHAR_H

#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "PortsBase.h"
#include "Types.h"  //need BYTE, UINT

#if defined(__linux__) || defined __FreeBSD__ || defined(__APPLE__) || defined(__native_client__)

#	include "CharTraits.h"

#	define NEWLINE "\n"

#else  //WINDOWS

#	define WCv(x)  (x)
#	define pWCv(x) (*(x))
#	define W_t(x)  (x)
#	define We(x)   (x)

typedef wchar_t         WCHAR_t;
typedef WCHAR_t         WCHAR;    //wc, 16-bit UNICODE character

#	define NEWLINE "\r\n"

#endif

#include <string>
#include <functional>

typedef std::basic_string<WCHAR, std::char_traits<WCHAR>, std::allocator<WCHAR> > WSTRING;

//Some common small strings.  Larger strings are localizable and should be kept in database.
extern const WCHAR wszAmpersand[], wszAsterisk[], wszOpenAngle[], wszCloseAngle[], wszColon[],
	wszComma[], wszCRLF[], wszDollarSign[], wszElipsis[], wszEmpty[], wszEqual[],
	wszExclamation[], wszForwardSlash[], wszHyphen[], wszParentDir[],
	wszPercent[], wszPeriod[], wszPoundSign[], wszPlus[], wszQuestionMark[], wszQuote[],
	wszLeftBracket[], wszRightBracket[], wszLeftParen[], wszRightParen[],
	wszSemicolon[], wszSpace[], wszSlash[], wszUnderscore[], wszZero[];

//HTML formatting strings.
extern const WCHAR wszHtml[], wszEndHtml[], wszBody[], wszEndBody[], wszHeader[],
	wszEndHeader[], wszBTag[], wszEndBTag[], wszITag[], wszEndITag[], wszPTag[];

void AsciiToUnicode(const char *psz, WSTRING &wstr);
void CTextToUnicode(const char *psz, WSTRING &wstr);
bool UnicodeToAscii(const WSTRING& wstr, char *psz);
bool UnicodeToAscii(const WSTRING& wstr, std::string &str);
static inline void UnicodeToAscii(const WCHAR *wsz, char *psz)
		{ do { *(psz++) = (char)pWCv(wsz); } while (pWCv(wsz++)); }

void UnicodeToUTF8(const WCHAR *pwsz, std::string &str);
static inline void UnicodeToUTF8(const WSTRING& wstr, std::string &str)
		{ UnicodeToUTF8(wstr.c_str(), str); }

unsigned int UTF8ToUCS4Char(const char **ppsz);
bool UTF8ToUCS2(const char* s, const UINT len, WSTRING &wstr);
void UTF8ToAscii(const char* s, const UINT len, std::string &str);
UINT utf8len(const WCHAR* pwczText);
UINT utf8len(const char* pczText);
UINT to_utf8(const WCHAR* pwStr, BYTE* &pbOutStr);
UINT to_utf8(const char* pStr, BYTE* &pbOutStr);

bool UTF8ToUnicode(const char *psz, WSTRING &wstr);
static inline bool UTF8ToUnicode(const std::string &str, WSTRING &wstr)
		{ return UTF8ToUCS2(str.c_str(), str.length(), wstr); }


bool charFilenameSafe(const WCHAR wc);
WSTRING  filenameFilter(const WSTRING &wstr);
WSTRING  filterFirstLettersAndNumbers(const WSTRING &wstr);
WSTRING  filterUpperCase(const WSTRING &wstr);
WCHAR* getFilenameFromPath(const WCHAR *wstrFilepath);
bool isWInteger(const WCHAR* wcz);
bool isInteger(const char* pcz);

int      _Wtoi(const WCHAR* wsz);
WCHAR*   _itoW(int value, WCHAR* wcz, int radix);
WCHAR*   _ltoW(long value, WCHAR* wcz, int radix);

UINT     WCSlen(const WCHAR* wsz)  FUNCATTR_PURE;
int      WCScmp(const WCHAR* pwcz1, const WCHAR* pwcz2)  FUNCATTR_PURE;
int      WCSicmp(const WCHAR* pwcz1, const WCHAR* pwcz2)  FUNCATTR_PURE;
int      WCSncmp(const WCHAR* pwcz1, const WCHAR* pwcz2, const UINT n)  FUNCATTR_PURE;
WCHAR*   WCScpy(WCHAR* wszDest, const WCHAR* wszSrc);
WCHAR*   WCSncpy(WCHAR* wszDest, const WCHAR* wszSrc, UINT n);
WCHAR*   WCScat(WCHAR* pwcz1, const WCHAR* pwcz2);
WCHAR*   WCStok(WCHAR *wszStart, const WCHAR *wszDelim);

struct WSTRINGicmp : public std::binary_function<WSTRING, WSTRING, bool> {
	bool operator()(const WSTRING& x, const WSTRING& y) const { return WCSicmp(x.c_str(), y.c_str()) < 0;}
};

WCHAR*   fgetWs(WCHAR* wcz, int n, FILE* pFile);
void     fputWs(const WCHAR* wsz, FILE* pFile);

std::string strReplace(std::string const &source, std::string const &from, std::string const &to);
WSTRING WCSReplace(WSTRING const &source, WSTRING const &from, WSTRING const &to);

#endif
