// $Id: RoomScreen.cpp 10049 2012-03-30 17:31:23Z mrimer $

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

#include "RoomScreen.h"
#include "DrodFontManager.h"
#include "MapWidget.h"
#include "RoomWidget.h"
#include "SettingsScreen.h"
#include <FrontEndLib/LabelWidget.h>

#include <BackEndLib/Exception.h>
#include <BackEndLib/Files.h>
#include <BackEndLib/Wchar.h>

#define BG_SURFACE      (0)
#define PARTS_SURFACE   (1)

const string demoDefaultMusic = "Forest";

//*****************************************************************************
CRoomScreen::CRoomScreen(
//Base constructor.
//
//Params:
	const SCREENTYPE eSetType)
	: CDrodScreen(eSetType)
	, pMapWidget(NULL)
	, pScrollLabel(NULL)
	, bIsScrollVisible(false)
{
	this->imageFilenames.push_back(string("GameScreen"));
	this->imageFilenames.push_back(string("GameScreenParts"));

	this->signColor = Black;

	static const int X_MAP = 15;
	static const int Y_MAP = 578;
	static const UINT CX_MAP = 130;
	static const UINT CY_MAP = 138;
	static const int X_SCROLL_LABEL = 13;
#ifdef RUSSIAN_BUILD
	static const int Y_SCROLL_LABEL = 215;
#else
	static const int Y_SCROLL_LABEL = 211;
#endif
	static const UINT CX_SCROLL_LABEL = 132;
	static const UINT CY_SCROLL_LABEL = 340;

	//Add widgets.
	this->pMapWidget = new CMapWidget(TAG_MAP, X_MAP, Y_MAP, CX_MAP, CY_MAP, NULL);
	if (!this->pMapWidget) throw CException("CRoomScreen: Couldn't allocate resources");
	this->pMapWidget->Disable();
	AddWidget(this->pMapWidget);

	this->pScrollLabel = new CLabelWidget(0L, X_SCROLL_LABEL, Y_SCROLL_LABEL, 
			CX_SCROLL_LABEL, CY_SCROLL_LABEL, F_Scroll, wszEmpty);
	AddWidget(this->pScrollLabel);
	this->pScrollLabel->Hide();
}

//*****************************************************************************
void CRoomScreen::SetMusicStyle(
//Changes the music to match style.  If music is already matching style, nothing
//will happen.
//
//Params:
	WSTRING style, const UINT wMood, const UINT fadeDuration) //[default=3000ms]
{
	if (g_pTheDBM->IsStyleFrozen())
		return; //don't change music if style is not to change

	ASSERT(g_pTheDBM);
	g_pTheDBM->ConvertStyle(style);

	WSTRING wMoodText;
	switch (wMood)
	{
		case SONG_AMBIENT: case SONG_ATTACK: case SONG_PUZZLE: case SONG_EXIT:
			AsciiToUnicode(moodText[wMood], wMoodText);
		break;
		default: ASSERT(!"Invalid style mood"); break;
	}
	style += wMoodText;

	//Fade to next song in list and update play order.
	CFiles f;
	list<WSTRING> songlist;
	if (f.GetGameProfileString("Songs", style.c_str(), songlist))
	{
		ASSERT(g_pTheSound);
		g_pTheSound->CrossFadeSong(&songlist, fadeDuration);
		f.WriteGameProfileString("Songs", style.c_str(), songlist);
	} else {
		//Play default music when a graphics style is present, but music is not.
		static WSTRING wstrDemoDefaultMusic;
		AsciiToUnicode(demoDefaultMusic.c_str(), wstrDemoDefaultMusic);
		if (WCScmp(style.c_str(), wstrDemoDefaultMusic.c_str()) != 0)
			SetMusicStyle(wstrDemoDefaultMusic, wMood, fadeDuration);
	}
}

//*****************************************************************************
void CRoomScreen::SetSignText(
//Set text that appears on sign.
//
//Params:
	const WCHAR *pwczSetText)  //(in)   New text.  NULL will make the sign 
								//    disappear on next paint.
{
	this->wstrSignText = pwczSetText ? pwczSetText : wszEmpty;
}

//*****************************************************************************
void CRoomScreen::PaintBackground()
//Paint background.
{
	ASSERT(this->images[BG_SURFACE]);
	SDL_BlitSurface(this->images[BG_SURFACE], NULL, GetDestSurface(), NULL);
}

//*****************************************************************************
void CRoomScreen::PaintScroll()
//Paint the scroll.
{
	static const int X_SCROLL = 6;
	static const int Y_SCROLL = 187;
	static const UINT CX_SCROLL = 154;
	static const UINT CY_SCROLL = 380;
	static const int X_SRC_SCROLL = 2;
	static const int Y_SRC_SCROLL = 2;
	static SDL_Rect ScreenRect = {X_SCROLL, Y_SCROLL, CX_SCROLL, CY_SCROLL};
	static SDL_Rect ScrollRect = {X_SRC_SCROLL, Y_SRC_SCROLL, CX_SCROLL, CY_SCROLL};

	ASSERT(this->images[BG_SURFACE]);
	ASSERT(this->images[PARTS_SURFACE]);

	SDL_Surface *pDestSurface = GetDestSurface();

	if (this->bIsScrollVisible)
	{
		ASSERT(this->images[PARTS_SURFACE]);
		SDL_BlitSurface(this->images[PARTS_SURFACE], &ScrollRect, 
				pDestSurface, &ScreenRect);
		this->pScrollLabel->Show();
		this->pScrollLabel->Paint();
	}
	else
	{
		ASSERT(this->images[BG_SURFACE]);
		SDL_BlitSurface(this->images[BG_SURFACE], &ScreenRect, 
				pDestSurface, &ScreenRect);
		this->pScrollLabel->Hide();
	}
	
	UpdateRect(ScreenRect);
}

//*****************************************************************************
void CRoomScreen::PaintSign()
//Paint the sign.
{
	static const UINT CX_LEFT_SIGN = 65;
	static const UINT CX_MIDDLE_SIGN = 36;
	static const UINT CX_RIGHT_SIGN = 68;
	static const UINT CX_SIGN = 821;
	static const UINT CY_SIGN = 36;
	static const int X_LEFT_SIGN_SRC = 1;
	static const int X_MIDDLE_SIGN_SRC = 69;
	static const int X_RIGHT_SIGN_SRC = 118;
	static const int Y_SIGN_SRC = 384;
	static const int X_SIGN = 163;
	static const int Y_SIGN = 2;
	static SDL_Rect LeftSignSource = {X_LEFT_SIGN_SRC, Y_SIGN_SRC, CX_LEFT_SIGN, CY_SIGN};
	static SDL_Rect MiddleSignSource = {X_MIDDLE_SIGN_SRC, Y_SIGN_SRC, CX_MIDDLE_SIGN, CY_SIGN};
	static SDL_Rect RightSignSource = {X_RIGHT_SIGN_SRC, Y_SIGN_SRC, CX_RIGHT_SIGN, CY_SIGN};
	static SDL_Rect EntireSign = {X_SIGN, Y_SIGN, CX_SIGN, CY_SIGN};

	ASSERT(this->images[BG_SURFACE]);
	ASSERT(this->images[PARTS_SURFACE]);

	SDL_Surface *pDestSurface = GetDestSurface();

	//Blit background over the entire possible area the sign could cover.
	SDL_BlitSurface(this->images[BG_SURFACE], &EntireSign, pDestSurface, &EntireSign);

	//Is there text to display?
	if (this->wstrSignText.size()) //Yes.
	{
		UINT wMiddleCount, wTextWidth, wTextHeight;

		//Figure out how wide it will be.
		g_pTheFM->GetTextWidthHeight(F_Sign, this->wstrSignText.c_str(), wTextWidth, wTextHeight);
		ASSERT(wTextWidth > 0);
			
		//Figure how many middle sign parts will be needed to display the text.
		wMiddleCount = (wTextWidth / CX_MIDDLE_SIGN);
		if (wTextWidth % CX_MIDDLE_SIGN != 0) ++wMiddleCount; //Round up.

		//Is the text too large to fit?
		UINT wSignWidth = CX_LEFT_SIGN + (wMiddleCount * CX_MIDDLE_SIGN) +
				CX_RIGHT_SIGN;
		if (wSignWidth > CX_SIGN)
		{
			//Sign width too large -- truncate sign text to fit.
			wSignWidth = CX_SIGN;
			wMiddleCount = (CX_SIGN - CX_LEFT_SIGN - CX_RIGHT_SIGN) / CX_MIDDLE_SIGN;
		}

		//Blit left part of sign.
		SDL_Rect Dest = {X_SIGN + ((CX_SIGN - wSignWidth) / 2), Y_SIGN, 
				CX_LEFT_SIGN, CY_SIGN};
		Uint32 TransparentColor = SDL_MapRGB(this->images[PARTS_SURFACE]->format, 226, 0, 0);
		SDL_SetColorKey(this->images[PARTS_SURFACE], SDL_SRCCOLORKEY, TransparentColor);
		SDL_BlitSurface(this->images[PARTS_SURFACE], &LeftSignSource, pDestSurface, &Dest);
		SDL_SetColorKey(this->images[PARTS_SURFACE], 0, TransparentColor);

		//Blit middle parts of sign.
		Dest.x += CX_LEFT_SIGN;
		Dest.w = CX_MIDDLE_SIGN;
		for (UINT wI = 0; wI < wMiddleCount; ++wI)
		{
			SDL_BlitSurface(this->images[PARTS_SURFACE], &MiddleSignSource, pDestSurface, &Dest);
			Dest.x += CX_MIDDLE_SIGN;
		}

		//Blit right part of sign.
		Dest.w = CX_RIGHT_SIGN;
		SDL_SetColorKey(this->images[PARTS_SURFACE], SDL_SRCCOLORKEY, TransparentColor);
		SDL_BlitSurface(this->images[PARTS_SURFACE], &RightSignSource, pDestSurface, &Dest);

		//Draw text on sign.
		int xText = X_SIGN + (int(CX_SIGN - wTextWidth) / 2);
		if (xText < X_SIGN + (int)CX_LEFT_SIGN/2)
			xText = X_SIGN + CX_LEFT_SIGN/2;
		int yText = Y_SIGN + ((CY_SIGN - wTextHeight) / 2);
		if (yText < Y_SIGN)
			yText = Y_SIGN;
		yText -= 3;     //kludge -- this font text is normally drawn too low

		//Set color.
		const SDL_Color origColor = g_pTheFM->GetFontColor(F_Sign);
		g_pTheFM->SetFontColor(F_Sign, this->signColor);

		g_pTheFM->DrawTextXY(F_Sign, this->wstrSignText.c_str(), pDestSurface, xText, yText,
				CX_SIGN - CX_LEFT_SIGN);

		g_pTheFM->SetFontColor(F_Sign, origColor);
		SDL_SetColorKey(this->images[PARTS_SURFACE], 0, TransparentColor);
	}

	UpdateRect(EntireSign);
}

//*****************************************************************************
SDLKey CRoomScreen::GetKeysymForCommand(const UINT wCommand) const
//Returns: keysym currently set for indicated command
{
	for (UINT wIndex=0; wIndex<SDLK_LAST; ++wIndex)
		if (static_cast<UINT>(this->KeysymToCommandMap[wIndex]) == wCommand)
			return SDLKey(wIndex);

	ASSERT(!"Command not assigned");
	return SDLK_UNKNOWN;
}

//*****************************************************************************
void CRoomScreen::InitKeysymToCommandMap(
//Set the keysym-to-command map with values from player settings that will determine
//which commands correspond to which keys.
//
//Params:
	CDbPackedVars &PlayerSettings)   //(in)   Player settings to load from.
{
	//Clear the map.
	memset(this->KeysymToCommandMap, CMD_UNSPECIFIED, 
			sizeof(this->KeysymToCommandMap));

	//Check whether default is for keyboard or laptop.
	CFiles Files;
	string strKeyboard;
	UINT wKeyboard = 0;	//default to numpad
	if (Files.GetGameProfileString("Localization", "Keyboard", strKeyboard))
	{
		wKeyboard = atoi(strKeyboard.c_str());
		if (wKeyboard > 1) wKeyboard = 0;	//invalid setting
	}

	//Get values from current player settings.
	UINT wIndex = 0;
	int nKey = PlayerSettings.GetVar(COMMANDNAME_ARRAY[wIndex], COMMANDKEY_ARRAY[wKeyboard][wIndex]);
	this->KeysymToCommandMap[nKey] = CMD_NW;
	++wIndex;
	nKey = PlayerSettings.GetVar(COMMANDNAME_ARRAY[wIndex], COMMANDKEY_ARRAY[wKeyboard][wIndex]);
	this->KeysymToCommandMap[nKey] = CMD_N;
	++wIndex;
	nKey = PlayerSettings.GetVar(COMMANDNAME_ARRAY[wIndex], COMMANDKEY_ARRAY[wKeyboard][wIndex]);
	this->KeysymToCommandMap[nKey] = CMD_NE;
	++wIndex;
	nKey = PlayerSettings.GetVar(COMMANDNAME_ARRAY[wIndex], COMMANDKEY_ARRAY[wKeyboard][wIndex]);
	this->KeysymToCommandMap[nKey] = CMD_W;
	++wIndex;
	nKey = PlayerSettings.GetVar(COMMANDNAME_ARRAY[wIndex], COMMANDKEY_ARRAY[wKeyboard][wIndex]);
	this->KeysymToCommandMap[nKey] = CMD_WAIT;
	++wIndex;
	nKey = PlayerSettings.GetVar(COMMANDNAME_ARRAY[wIndex], COMMANDKEY_ARRAY[wKeyboard][wIndex]);
	this->KeysymToCommandMap[nKey] = CMD_E;
	++wIndex;
	nKey = PlayerSettings.GetVar(COMMANDNAME_ARRAY[wIndex], COMMANDKEY_ARRAY[wKeyboard][wIndex]);
	this->KeysymToCommandMap[nKey] = CMD_SW;
	++wIndex;
	nKey = PlayerSettings.GetVar(COMMANDNAME_ARRAY[wIndex], COMMANDKEY_ARRAY[wKeyboard][wIndex]);
	this->KeysymToCommandMap[nKey] = CMD_S;
	++wIndex;
	nKey = PlayerSettings.GetVar(COMMANDNAME_ARRAY[wIndex], COMMANDKEY_ARRAY[wKeyboard][wIndex]);
	this->KeysymToCommandMap[nKey] = CMD_SE;
	++wIndex;
	nKey = PlayerSettings.GetVar(COMMANDNAME_ARRAY[wIndex], COMMANDKEY_ARRAY[wKeyboard][wIndex]);
	this->KeysymToCommandMap[nKey] = CMD_C;
	++wIndex;
	nKey = PlayerSettings.GetVar(COMMANDNAME_ARRAY[wIndex], COMMANDKEY_ARRAY[wKeyboard][wIndex]);
	this->KeysymToCommandMap[nKey] = CMD_CC;
	++wIndex;
	nKey = PlayerSettings.GetVar(COMMANDNAME_ARRAY[wIndex], COMMANDKEY_ARRAY[wKeyboard][wIndex]);
	this->KeysymToCommandMap[nKey] = CMD_RESTART;
	++wIndex;
	nKey = PlayerSettings.GetVar(COMMANDNAME_ARRAY[wIndex], COMMANDKEY_ARRAY[wKeyboard][wIndex]);
	this->KeysymToCommandMap[nKey] = CMD_UNDO;
	++wIndex;
	nKey = PlayerSettings.GetVar(COMMANDNAME_ARRAY[wIndex], COMMANDKEY_ARRAY[wKeyboard][wIndex]);
	this->KeysymToCommandMap[nKey] = CMD_BATTLE_KEY;
	++wIndex;
	nKey = PlayerSettings.GetVar(COMMANDNAME_ARRAY[wIndex], COMMANDKEY_ARRAY[wKeyboard][wIndex]);
	this->KeysymToCommandMap[nKey] = CMD_EXEC_COMMAND;
}

//*****************************************************************************
int CRoomScreen::FindKey(
//Returns:
//Key index mapped to command.
	const int nCommand) const  //(in) Command to find mapping for.
{
	for (int nIndex=0; nIndex<SDLK_LAST; ++nIndex)
		if (this->KeysymToCommandMap[nIndex] == nCommand)
			return nIndex;
	ASSERTP(false, "Failed to find key mapping.");
	return -1;
}
