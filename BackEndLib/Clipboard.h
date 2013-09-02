// $Id: Clipboard.h 8019 2007-07-14 22:30:11Z trick $

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
 * Matt Schikore (Schik)
 *
 * ***** END LICENSE BLOCK ***** */

//CClipboard
//Declarations for CClipboard.
//Class for accessing and modifying the system clipboard

//The linux code is based on scrap (PD, http://www.libsdl.org/projects/scrap/)

#ifndef CLIPBOARD_H
#define CLIPBOARD_H
#ifdef WIN32
#  pragma warning(disable:4786)
#endif

#include "Wchar.h"

#if defined __linux__ || defined __FreeBSD__
#include "SDL.h"
#include "SDL_syswm.h"
#endif

#include <string>
#include <map>
using std::string;
using std::map;

//***************************************************************************
#if defined __linux__ || defined __FreeBSD__
typedef struct {
	Atom utf8_string;
	Atom clipboard;
	Atom targets;
} XAtoms;
#endif

class CClipboard
{
#if defined __linux__ || defined __FreeBSD__
	static Display *display;
	static Window   window;
	static void (*LockDisplay)(void);
	static void (*UnlockDisplay)(void);
	static XAtoms xa;

	static int  ClipboardFilter (const SDL_Event *event);
	static bool LostScrap ();
	static bool Init ();

	static void FixNewlines (string& sClip);
	static void UnfixNewlines (string& sClip);
	static bool GetStringUTF8(string& sClip);
	static bool SetStringUTF8(const char *sClip);

	static string sClipBuf;
#endif

public:
	CClipboard() {}

	static bool GetString(string& sClip);
	static bool SetString(const string& sClip);
	static bool GetString(WSTRING& sClip);
	static bool SetString(const WSTRING& sClip);
};

#endif //...#ifndef CLIPBOARD_H
