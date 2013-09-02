// $Id: Clipboard.cpp 10051 2012-03-31 04:23:44Z mrimer $

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
 * Portions created by the Initial Developer are Copyright (C) 1995, 1996,
 * 1997, 2000, 2001, 2002, 2005 Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 * Matt Schikore (Schik), Gerry JJ
 *
 * ***** END LICENSE BLOCK ***** */

#ifdef WIN32
#  include <windows.h> //Should be first include.
#  pragma warning(disable:4786)
#endif

#include "Assert.h"
#include "Clipboard.h"
#include "Dyn.h"

#if defined __APPLE__
#include <vector>
#include <Carbon/Carbon.h>
#endif

//******************************************************************************
#if defined __linux__ || defined __FreeBSD__

Display *CClipboard::display = NULL;
Window   CClipboard::window;
XAtoms   CClipboard::xa;
string   CClipboard::sClipBuf;

void (*CClipboard::LockDisplay)(void);
void (*CClipboard::UnlockDisplay)(void);

int CClipboard::ClipboardFilter (const SDL_Event *event)
{
	if (event->type == SDL_SYSWMEVENT) switch (event->syswm.msg->event.xevent.type)
	{
	  case SelectionRequest:
	  {
		XSelectionRequestEvent *req = &event->syswm.msg->event.xevent.xselectionrequest;
		XEvent sevent;
		int seln_format;
		unsigned long nbytes, overflow;
		unsigned char *seln_data;
		sevent.xselection.type = SelectionNotify;
		sevent.xselection.display = req->display;
		sevent.xselection.selection = req->selection;
		sevent.xselection.target = None;
		sevent.xselection.property = None;
		sevent.xselection.requestor = req->requestor;
		sevent.xselection.time = req->time;

		if (Dyn::XGetWindowProperty(display, DefaultRootWindow(display),
				xa.clipboard, 0, INT_MAX/4, False, req->target,
				&sevent.xselection.target, &seln_format,
				&nbytes, &overflow, &seln_data) == Success)
		{
			if (req->property == None)
				req->property = req->target;

			if (req->target == xa.targets)
			{
				//Atom is 64-bit on amd64, but we can only send 8/16/32-bit values =P
				Uint32 atoms[] = { (Uint32)xa.utf8_string, (Uint32)XA_STRING, (Uint32)xa.targets };
				Dyn::XChangeProperty(display, req->requestor, req->property,
					sevent.xselection.target = xa.targets, 32, PropModeReplace,
					(unsigned char*)&atoms[0], sizeof(atoms)/4);
				sevent.xselection.property = req->property;
			}
			else if (req->target == xa.utf8_string)
			{
				if (seln_data[nbytes - 1] == 0)
					--nbytes;
				Dyn::XChangeProperty(display, req->requestor, req->property,
					sevent.xselection.target = xa.utf8_string, 8, PropModeReplace,
					seln_data, nbytes);
				sevent.xselection.property = req->property;
			}
			else if (req->target == XA_STRING)
			{
				Dyn::XChangeProperty(display, req->requestor, req->property,
					sevent.xselection.target = XA_STRING, 8, PropModeReplace,
					(unsigned char*)sClipBuf.c_str(), sClipBuf.length());
				sevent.xselection.property = req->property;
			}

			Dyn::XSendEvent(display, req->requestor, False, 0, &sevent);
			Dyn::XSync(display, False);
			if (seln_data) Dyn::XFree(seln_data);
		}

		//We're done with this event
		return 0;
	  }

	  case SelectionNotify:
		break;

	  default:
		return 0;
	}

	//Post event
	return 1;
}

bool CClipboard::LostScrap ()
{
	LockDisplay();
	bool bLost = (Dyn::XGetSelectionOwner(display, xa.clipboard) != window);
	UnlockDisplay();
	return bLost;
}

bool CClipboard::Init ()
{
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (SDL_GetWMInfo(&info) && info.subsystem == SDL_SYSWM_X11) {
		display = info.info.x11.display;
		window = info.info.x11.window;
		LockDisplay = info.info.x11.lock_func;
		UnlockDisplay = info.info.x11.unlock_func;
		xa.utf8_string = Dyn::XInternAtom(display, "UTF8_STRING", False);
		xa.clipboard = Dyn::XInternAtom(display, "CLIPBOARD", False);
		//xa.clipboard = XA_PRIMARY;
		xa.targets = Dyn::XInternAtom(display, "TARGETS", False);

		//Enable SDL clipboard passing
		SDL_SetEventFilter(ClipboardFilter);
		SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
		return true;
	} else {
		display = NULL;   //No need to init the rest in this case
		return false;
	}
}


void CClipboard::FixNewlines (string& sClip)
{
	//Windows newlines have already been fixed before we get here, but user
	//text (scrolls, etc) seems to have MAC newlines, so we replace \r by \n
	for (UINT i = 0; sClip[i]; ++i)
		if (sClip[i] == '\r')
			sClip[i] = '\n';
}

void CClipboard::UnfixNewlines (string& sClip)
{
	//To keep things consistent in the db, we replace \n by \r again.
	for (UINT i = 0; sClip[i]; ++i)
		if (sClip[i] == '\n')
			sClip[i] = '\r';
}


bool CClipboard::GetStringUTF8(
//Copies the system clipboard string into sClip
//Returns: true on success, false otherwise.
//
//Params:
	string& sClip) //(out)
{
	if (!display && !Init())
	{
		//Failed to load X, but at least we can have local clipboard
		sClip = sClipBuf;
		return true;
	}

	LockDisplay();
	Window owner = Dyn::XGetSelectionOwner(display, xa.clipboard);
	UnlockDisplay();

	Atom selection;

	if (owner == None || owner == window) {
		owner = DefaultRootWindow(display);
		selection = xa.clipboard;
	} else {
		//FIXME: Use TARGETS to check what formats are available, and handle XA_STRING
		SDL_Event event;

		owner = window;
		LockDisplay();
		selection = Dyn::XInternAtom(display, "DeadlySelectionOfDeath3", False);
		Dyn::XConvertSelection(display, xa.clipboard, xa.utf8_string, selection, owner, CurrentTime);
		UnlockDisplay();

		const Uint32 tick = SDL_GetTicks();
		do while (SDL_PollEvent(&event)) if (event.type == SDL_SYSWMEVENT)
		{
			XEvent xevent = event.syswm.msg->event.xevent;
			if ((xevent.type == SelectionNotify) &&
					(xevent.xselection.requestor == owner))
				goto Selection_ready;
		} while (SDL_GetTicks() - tick < 1000);

		//Timeout
		fprintf(stderr, "Clipboard timeout\n");
		return false;
	}

Selection_ready:
	unsigned char *src;
	Atom seln_type;
	int seln_format;
	unsigned long nbytes, overflow;
	bool bSuccess = false;

	LockDisplay();
	if (Dyn::XGetWindowProperty(display, owner, selection, 0, INT_MAX/4-1, False,
			xa.utf8_string, &seln_type, &seln_format, &nbytes, &overflow,
			&src) == Success)
	{
		if (seln_type == xa.utf8_string)
		{
			unsigned int ibytes = src[nbytes-1] ? nbytes : nbytes - 1;
			sClip.assign((const char*)src, ibytes);
			bSuccess = true;
			UnfixNewlines(sClip);
		}
		if (src) Dyn::XFree(src);
	}
	UnlockDisplay();
	return bSuccess;
}

bool CClipboard::SetStringUTF8(
//Copies the given string into the system clipboard
//Returns: true on success, false otherwise.
//
//Params:
	const char *szClip)  //(in)
{
	string sClip = szClip;

	if (!display && !Init())
	{
		//Failed to load X, but at least we can have local clipboard
		sClipBuf = sClip;
		return false;
	}

	FixNewlines(sClip);

	LockDisplay();
	Dyn::XChangeProperty(display, DefaultRootWindow(display), xa.clipboard,
		xa.utf8_string, 8, PropModeReplace, (unsigned char *)sClip.c_str(), sClip.length());
	if (LostScrap())
		Dyn::XSetSelectionOwner(display, xa.clipboard, window, CurrentTime);
	UnlockDisplay();
	UTF8ToAscii(sClip.c_str(), sClip.length(), sClipBuf);
	return true;
}

#endif //#ifdef __linux__ || defined __FreeBSD__

//******************************************************************************
bool CClipboard::SetString(
//Copies the given string into the system clipboard
//Returns: true on success, false otherwise.
//
//Params:
	const string& sClip)  //(in)
{
#ifdef WIN32
	if (!OpenClipboard(NULL))
		return false;
	EmptyClipboard();

	HGLOBAL global = GlobalAlloc(GMEM_ZEROINIT, (sClip.size()+1)*sizeof(char));

	if (global == NULL) {
		CloseClipboard();
		return false;
	}

	LPSTR data = (LPSTR)GlobalLock(global);

	strcpy(data, sClip.c_str());

	GlobalUnlock(global);
	SetClipboardData(CF_TEXT, global);
	CloseClipboard();

	return true;

#elif defined(__linux__) || defined(__FreeBSD__)
	bool bSuccess = false;
	BYTE *pbOutStr = NULL;
	if (to_utf8(sClip.c_str(), pbOutStr))
		bSuccess = SetStringUTF8((const char*)pbOutStr);
	delete[] pbOutStr;
	return bSuccess;

#elif defined(__APPLE__)
	PasteboardRef theClipboard;
    	OSStatus err = PasteboardCreate(kPasteboardClipboard, &theClipboard);
	if (err != noErr)
		return false;
	PasteboardClear(theClipboard);
	PasteboardSynchronize(theClipboard);
	CFDataRef data = CFDataCreate(kCFAllocatorDefault, (UInt8*)sClip.c_str(), sClip.size() + 1);
	PasteboardPutItemFlavor(theClipboard, (PasteboardItemID)1, CFSTR("public.utf8-plain-text"), data, 0);

	return true;

#else
#warning How do you set system clipboard data on this system?
   return false;
#endif
}

//******************************************************************************
bool CClipboard::GetString(
//Copies the system clipboard string into sClip
//Returns: true on success, false otherwise.
//
//Params:
	string& sClip)  //(out)
{
#ifdef WIN32
	if (!OpenClipboard(NULL))
		return false;
	HGLOBAL global = GetClipboardData(CF_TEXT);
	if (global == NULL) {
		CloseClipboard();
		return false;
	}
	LPSTR data = (LPSTR)GlobalLock(global);
	sClip = data;
	GlobalUnlock(global);
	CloseClipboard();

	return true;

#elif defined(__linux__) || defined(__FreeBSD__)
	string u8clip;
	bool bSuccess;
	if ((bSuccess = GetStringUTF8(u8clip)))
		UTF8ToAscii(u8clip.c_str(), u8clip.length(), sClip);
	return bSuccess;

#elif defined __APPLE__
	ItemCount  itemCount;

	PasteboardRef theClipboard;
    	OSStatus err = PasteboardCreate(kPasteboardClipboard, &theClipboard);
	if (err != noErr)
		return false;
	PasteboardSynchronize( theClipboard );
	PasteboardGetItemCount( theClipboard, &itemCount );
	UInt32 itemIndex = 1; // should be 1 or the itemCount?

	PasteboardItemID    itemID;
	CFArrayRef          flavorTypeArray;
	CFIndex             flavorCount;

	PasteboardGetItemIdentifier( theClipboard, itemIndex, &itemID );
	PasteboardCopyItemFlavors( theClipboard, itemID, &flavorTypeArray );

	flavorCount = CFArrayGetCount( flavorTypeArray );

	for(CFIndex flavorIndex = 0 ; flavorIndex < flavorCount; flavorIndex++ )
	{
		CFStringRef  flavorType = (CFStringRef)CFArrayGetValueAtIndex( flavorTypeArray, flavorIndex );

		if (UTTypeConformsTo(flavorType, CFSTR("public.utf8-plain-text")))
		{
			CFDataRef   flavorData;
			PasteboardCopyItemFlavorData( theClipboard, itemID,flavorType, &flavorData );

			CFIndex  flavorDataSize = CFDataGetLength( flavorData );
			sClip.resize(flavorDataSize);
			memcpy(&sClip[0], flavorData, flavorDataSize);
			CFRelease (flavorData);
			break;
		}
	}
	CFRelease (flavorTypeArray);

	return true;

#else
#warning How do you get system clipboard data on this system?
   return false;
#endif
}

//******************************************************************************
bool CClipboard::GetString(
//Copies the system clipboard string into sClip
//Returns: true on success, false otherwise.
//
//Params:
	WSTRING& sClip) //(out)
{
#ifdef WIN32
	if (!OpenClipboard(NULL))
		return false;
	HGLOBAL global = GetClipboardData(CF_UNICODETEXT);
	if (global == NULL) {
		CloseClipboard();
		return false;
	}
	LPWSTR data = (LPWSTR)GlobalLock(global);
	sClip = data;
	GlobalUnlock(global);
	CloseClipboard();

	return true;

#elif defined __APPLE__
	PasteboardRef theClipboard;
    	OSStatus err = PasteboardCreate(kPasteboardClipboard, &theClipboard);
	if (err != noErr)
		return false;

	ItemCount itemCount;
	PasteboardSynchronize(theClipboard);
	PasteboardGetItemCount(theClipboard, &itemCount);
	UInt32 itemIndex = 1;

	PasteboardItemID itemID;
	PasteboardGetItemIdentifier(theClipboard, itemIndex, &itemID);

	CFArrayRef flavorTypeArray;
	PasteboardCopyItemFlavors(theClipboard, itemID, &flavorTypeArray);

	CFIndex flavorCount = CFArrayGetCount(flavorTypeArray);

	for (CFIndex flavorIndex = 0; flavorIndex < flavorCount; flavorIndex++)
	{
		CFStringRef flavorType = (CFStringRef)CFArrayGetValueAtIndex(flavorTypeArray, flavorIndex);

		if (UTTypeConformsTo(flavorType, CFSTR("public.utf8-plain-text")))
		{
			CFDataRef flavorData;
			PasteboardCopyItemFlavorData(theClipboard, itemID, flavorType, &flavorData);

			//CFIndex flavorDataSize = CFDataGetLength(flavorData);
 			const string str = (char*)CFDataGetBytePtr(flavorData);
			UTF8ToUCS2(str.c_str(), str.size(), sClip);
			CFRelease(flavorData);
			break;
		}
	}
	CFRelease (flavorTypeArray);

	return true;

#elif defined(__linux__) || defined(__FreeBSD__)
	string u8clip;
	bool bSuccess;
	if ((bSuccess = GetStringUTF8(u8clip)))
		UTF8ToUCS2(u8clip.c_str(), u8clip.length(), sClip);
	return bSuccess;

#else
#error CClipboard::GetString -- Unicode not implemented
#endif
}

//******************************************************************************
bool CClipboard::SetString(
//Copies the given string into the system clipboard
//Returns: true on success, false otherwise.
//
//Params:
	const WSTRING& sClip)  //(in)
{
#ifdef WIN32
	if (!OpenClipboard(NULL))
		return false;
	EmptyClipboard();

	HGLOBAL global = GlobalAlloc(GMEM_ZEROINIT, (sClip.size()+1)*sizeof(WCHAR));

	if (global == NULL) {
		CloseClipboard();
		return false;
	}

	LPWSTR data = (LPWSTR)GlobalLock(global);

	WCScpy(data, sClip.c_str());

	GlobalUnlock(global);
	SetClipboardData(CF_UNICODETEXT, global);
	CloseClipboard();

	return true;
#elif defined(__APPLE__)
	PasteboardRef theClipboard;
    	OSStatus err = PasteboardCreate(kPasteboardClipboard, &theClipboard);
	if (err != noErr)
		return false;
	PasteboardClear(theClipboard);
	PasteboardSynchronize(theClipboard);
	BYTE *pbOutStr = NULL;
	if (to_utf8(sClip.c_str(), pbOutStr)) {
		CFDataRef data = CFDataCreate(kCFAllocatorDefault, (UInt8*)pbOutStr, sClip.size() + 1);
		PasteboardPutItemFlavor(theClipboard, (PasteboardItemID)1, CFSTR("public.utf8-plain-text"), data, 0);
	}
	delete[] pbOutStr;
	return true;
#elif defined(__linux__) || defined(__FreeBSD__)
	bool bSuccess = false;
	BYTE *pbOutStr = NULL;
	if (to_utf8(sClip.c_str(), pbOutStr))
		bSuccess = SetStringUTF8((const char*)pbOutStr);
	delete[] pbOutStr;
	return bSuccess;

#else
#error CClipboard::SetString -- Unicode not implemented
#endif
}
