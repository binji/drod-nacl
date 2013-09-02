// $Id: RoomScreen.h 9742 2011-10-22 16:12:27Z mrimer $

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
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef ROOMSCREEN_H
#define ROOMSCREEN_H

#include "DrodBitmapManager.h"
#include "DrodScreen.h"
#include "DrodScreenManager.h"
#include "DrodSound.h"
#include "../DRODLib/DbPackedVars.h"

//Base class for displaying a room,
//including basically everything that goes on "gamescreen.png"
//(i.e. RoomWidget, Scroll, Map, and Sign).

static const UINT TAG_ROOM = 1000;
static const UINT TAG_MAP = 1001;

class CMapWidget;
class CRoomWidget;
class CLabelWidget;
class CCurrentGame;
class CRoomScreen : public CDrodScreen
{
public:
	static void    SetMusicStyle(WSTRING style, const UINT wMood, const UINT fadeDuration=3000);

protected:
	friend class CScreenManager;

	CRoomScreen(const SCREENTYPE eSetType);
	virtual ~CRoomScreen() { }

	int      FindKey(const int nCommand) const;
	SDLKey   GetKeysymForCommand(const UINT wCommand) const;
	void     HideScroll() {this->bIsScrollVisible = false; PaintScroll();}
	void     InitKeysymToCommandMap(CDbPackedVars &PlayerSettings);
	void     PaintBackground();
	void     PaintScroll();
	void     PaintSign();
	void     SetSignText(const WCHAR *pwczSetText);
	void     ShowScroll() {this->bIsScrollVisible = true; PaintScroll();}

	//These are accessed by CDemoScreen.
	CMapWidget *      pMapWidget;
	CLabelWidget *    pScrollLabel;

	WSTRING           wstrSignText;
	SDL_Color         signColor;    //color of text on sign

	bool              bIsScrollVisible;

	int               KeysymToCommandMap[SDLK_LAST];
};

#endif //...#ifndef ROOMSCREEN_H
