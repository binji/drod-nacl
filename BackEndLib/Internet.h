// $Id: Internet.h 9604 2011-03-09 14:38:17Z TFMurphy $

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
*
* ***** END LICENSE BLOCK ***** */

#ifndef NO_CURL
#include <curl/curl.h>  //must come first
#endif
#include "StretchyBuffer.h"
#include "IDSet.h"

#include <map>
#include <string>
#include <vector>
#include <SDL_thread.h>
using std::string;

#ifndef INTERNET_H
#define INTERNET_H

//****************************************************************************
class CInternet_Thread_Info
{
public:
#ifdef NO_CURL
	CInternet_Thread_Info() {}
#else
	CStretchyBuffer* pBuffer;
	string strUrl;
	SDL_Thread* pThread;
	CURLcode eRetVal;
	long responseCode;
	UINT bytesComplete;
	bool bIgnoreResponse;
	char errorBuffer[CURL_ERROR_SIZE];

	CInternet_Thread_Info()
		: pBuffer(NULL)
		, pThread(NULL)
		, eRetVal((CURLcode)-1),
		, responseCode(0)
		, bytesComplete(0)
		, bIgnoreResponse(false)
	{ errorBuffer[0] = 0; }
#endif
};

//****************************************************************************
class CInternet
{
public:
#ifdef NO_CURL
	static int GetStatus(const UINT handle) { return 0; }
	static CStretchyBuffer* GetResults(const UINT handle, const bool bSkipClean=false) { return NULL; }
	static bool HttpGet(const string& strUrl, UINT* handle=NULL) { return false; }
	static bool Init(const char* pUserAgent, const bool bSSL=false) { return true; }
	static void Deinit() {}
	static bool CancelRequest(const UINT handle) { return false; }
	static UINT GetBytesCompleted(const UINT handle) { return 0; }
#else
	static void AddCookie(const string& strName, const string& value);
	static void AddPostData(const string& strName, const CStretchyBuffer& value);
	static void AddPostData(const string& strName, const string& value);
	static void AddPostData(const string& strName, const std::vector<string>& values);
	static void ClearCookies() { CInternet::strCookies = ""; }
	static void Deinit();
	static UINT GetBytesCompleted(const UINT handle);
	static string GetCookies() { return CInternet::strCookies; }
	static curl_httppost* GetFormPost();
	static curl_httppost* GetFormEnd();
	static CStretchyBuffer* GetResults(const UINT handle, const bool bSkipClean=false);
	static int GetStatus(const UINT handle);
	static bool HttpGet(const string& strUrl, UINT* handle=NULL);
	static bool Init(const char* pUserAgent, const bool bSSL=false);
	static void OutputError(const UINT handle);
	static bool CancelRequest(const UINT handle);

	static curl_httppost* pFormPostBegin;
	static curl_httppost* pFormPostEnd;
	static string userAgent;
	static std::vector<CURL*> usedHandles;
private:
	static void CleanCanceled();

	static CURLM* pHandle;
	static string strCookies;
	static UINT nHandles;
	static bool bInit;

	static std::map<UINT, CInternet_Thread_Info*> threadInfo;
	static std::vector<CInternet_Thread_Info*> ignoredThreads;
	static CIDSet canceledHandles;
#endif
};

#endif // INTERNET_H
