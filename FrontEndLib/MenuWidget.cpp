// $Id: MenuWidget.cpp 9710 2011-10-20 18:24:39Z mrimer $

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

#include "MenuWidget.h"
#include "EventHandlerWidget.h"
#include "Sound.h"
#include <BackEndLib/Assert.h>
#include <BackEndLib/Ports.h> //for towlower

//
//Public methods.
//

//*****************************************************************************
CMenuWidget::CMenuWidget(
//Constructor.
//
//Params:
	const UINT dwSetTagNo,                //(in)   Required params for CWidget
	const int nSetX, const int nSetY,               //    constructor.
	const UINT wSetW, const UINT wSetH,             //
	const UINT eUnselectedFont,            //(in)   Font to use for text.
	const UINT eMouseOverFont, const UINT eSelectedFont)
	: CFocusWidget(WT_Menu, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
	, seid(SOUNDLIB::SEID_BUTTON)
{
	clear();
	this->eFontType[UNSELECTED] = eUnselectedFont;
	this->eFontType[SELECTED] = eSelectedFont;
	this->eFontType[MOUSEOVER] = eMouseOverFont;
}

//*****************************************************************************
CMenuWidget::~CMenuWidget()
//Destructor.
{
	clear();
}

//*****************************************************************************
void CMenuWidget::clear()
//Clear all menu options.
{
	for (UINT wIndex=this->options.size(); wIndex--;)
		delete this->options[wIndex];
	this->options.clear();

	this->wNextY = 0;
	this->nSelectedOption = this->nOnOption = -1;
}

//*****************************************************************************
void CMenuWidget::ResetSelection()
//Resets menu option selection/highlighting.
{
	this->nSelectedOption = this->nOnOption = -1;
}

//*****************************************************************************
void CMenuWidget::AddText(const WCHAR* wszText, const UINT dwTag)
//Adds another menu option.
{
	ASSERT(wszText);
	MenuOption *pOption = new MenuOption;
	pOption->text = wszText;
	pOption->hotkey = W_t(0);
	pOption->dwTag = dwTag;
	pOption->bEnabled = true;

	//Find hotkey in string.
	//Remove escape chars for correct size calculation.
	const UINT nLength = WCSlen(wszText);
	WSTRING textWithoutEscapeChars;
	for (UINT nIndex=0; nIndex<nLength; ++nIndex)
	{
		if (wszText[nIndex] == L'&')
		{
			ASSERT(nIndex+1 < nLength);
			pOption->hotkey = W_t(WCv(towlower(wszText[nIndex+1])));   //no uppercase mappings exist
			textWithoutEscapeChars += wszText + nIndex+1;
			break;
		} else {
			WCHAR ch[2] = {wszText[nIndex]};
			textWithoutEscapeChars += ch;
		}
	}

	//Calculate placement of text in menu.
	UINT wW, wH;
	g_pTheFM->GetTextWidthHeight(this->eFontType[UNSELECTED],
			textWithoutEscapeChars.c_str(), wW, wH);
	pOption->rect.x = wW >= this->w ? 0 : (this->w - wW) / 2;  //center text
	pOption->rect.y = this->wNextY;
	pOption->rect.w = wW;
	pOption->rect.h = wH;
	this->wNextY += wH;

	this->options.push_back(pOption);
}

//******************************************************************************
void CMenuWidget::Enable(const UINT dwTag, const bool bVal)
{
	UINT wIndex;
	for (wIndex=this->options.size(); wIndex--; )
		if (dwTag == this->options[wIndex]->dwTag)
		{
			this->options[wIndex]->bEnabled = bVal;
			if (wIndex == (UINT)this->nOnOption)
				this->nOnOption = -1;
			if (wIndex == (UINT)this->nSelectedOption)
				this->nSelectedOption = -1;
		}
}

//******************************************************************************
UINT CMenuWidget::GetOnOption()
//Returns: hotkey value corresponding to menu option that mouse is over
{
	if (this->nOnOption == -1) return (UINT)-1;
	ASSERT((UINT)this->nOnOption < this->options.size());
	return this->options[this->nOnOption]->dwTag;
}

//******************************************************************************
UINT CMenuWidget::GetSelectedOption()
//Returns: hotkey value corresponding to menu option that was selected
{
	if (this->nSelectedOption == -1) return (UINT)-1;
	ASSERT((UINT)this->nSelectedOption < this->options.size());
	const UINT dwTag = this->options[this->nSelectedOption]->dwTag;
	this->nSelectedOption = -1;   //reset
	return dwTag;
}

//******************************************************************************
void CMenuWidget::HandleKeyDown(
//Processes a keyboard event within the scope of the widget.
//
//Params:
	const SDL_KeyboardEvent &KeyboardEvent)   //(in) Event to handle.
{
	WCHAR wc = TranslateUnicodeKeysym(KeyboardEvent.keysym, false); //hotkeys are generally lower case
	wc = towlower(wc);
	if (!wc)
		return; //not a printable keypress

	UINT wIndex;
	for (wIndex=this->options.size(); wIndex--; )
		if (wc == this->options[wIndex]->hotkey && this->options[wIndex]->bEnabled)
		{
			SelectOption(wIndex);
			break;
		}

	if (wc > 255)
	{
		//However, Unicode hotkey lower case conversion might have been unsuccessful,
		//so a search for upper case hotkeys is required as well.
		TranslateUnicodeKeysym(wc, KeyboardEvent.keysym.sym, true);
		for (wIndex=this->options.size(); wIndex--; )
			if (wc == this->options[wIndex]->hotkey && this->options[wIndex]->bEnabled)
			{
				SelectOption(wIndex);
				return;
			}
	}
}

//*****************************************************************************
void CMenuWidget::HandleMouseMotion(
	const SDL_MouseMotionEvent &Motion)
{
	//const int nOldOnOption = this->nOnOption;
	this->nOnOption = -1;

	for (UINT wIndex=this->options.size(); wIndex--; )
	{
		if (InOption(wIndex, Motion.x, Motion.y))
		{
			ASSERT(this->nOnOption == -1);   //mouse can only be on one option at a time
			this->nOnOption = wIndex;
		}
	}

	//This repaint causes transparent (disabled) selections to become darker
	//as they are redrawn.
	//	if (nOldOnOption != this->nOnOption) RequestPaint();
}

//******************************************************************************
void CMenuWidget::HandleMouseUp(
//Handles mouse down event.
//
//Params:
	const SDL_MouseButtonEvent &Button) //(in) Event to handle.
{
	UINT wIndex;
	for (wIndex=this->options.size(); wIndex--; )
	{
		if (InOption(wIndex, Button.x, Button.y))
		{
			SelectOption(wIndex);
			break;
		}
	}
}

//*****************************************************************************
void CMenuWidget::Paint(
//Paint menu options inside of the widget area.
//
//Params:
	bool bUpdateRect)       //(in)   If true (default) and destination
								//    surface is the screen, the screen
								//    will be immediately updated in
								//    the widget's rect.
{
	if (!this->options.size()) return;

	int nOffsetX, nOffsetY;
	GetScrollOffset(nOffsetX, nOffsetY);

	SDL_Surface *pDestSurface = GetDestSurface();

	MenuFont eFont;
	bool bEnabled;
	for (UINT wIndex=0; wIndex<this->options.size(); ++wIndex)
	{
		bEnabled = this->options[wIndex]->bEnabled;
		eFont = bEnabled ? (wIndex == (UINT)this->nSelectedOption ? SELECTED :
				wIndex == (UINT)this->nOnOption ? MOUSEOVER : UNSELECTED) : UNSELECTED;

		g_pTheFM->DrawHotkeyTextToLine(this->eFontType[eFont],
				this->options[wIndex]->text.c_str(),
				this->x + nOffsetX + this->options[wIndex]->rect.x,
				this->y + nOffsetY + this->options[wIndex]->rect.y,
				this->w, pDestSurface,
				bEnabled ? 255 : 96, //disabled options are faded
				bEnabled ? this->eFontType[SELECTED] : this->eFontType[eFont]); //hotkey not shown on disabled options
	}

	if (bUpdateRect) UpdateRect();
}

//*****************************************************************************
bool CMenuWidget::InOption(const UINT wIndex, const UINT wX, const UINT wY) const
//Returns: true if pixel (wX,wY) is inside the wIndex'th menu option
{
	if (!this->options[wIndex]->bEnabled) return false;

	int nOffsetX, nOffsetY;
	GetScrollOffset(nOffsetX, nOffsetY);

	SDL_Rect& rect = this->options[wIndex]->rect;
	int nX = static_cast<int>(wX), nY = static_cast<int>(wY);
	return (this->y + nOffsetY + rect.y <= nY && nY < this->y + nOffsetY + rect.y + rect.h &&
			this->x + nOffsetX + rect.x <= nX && nX < this->x + nOffsetX + rect.x + rect.w);
}

//*****************************************************************************
void CMenuWidget::SelectOption(const UINT wIndex)
//Notify and display than a menu option has been selected.
{
	ASSERT(this->options[wIndex]->bEnabled);
	this->nSelectedOption = wIndex;

	g_pTheSound->PlaySoundEffect(this->seid);
	RequestPaint();

	CEventHandlerWidget *pEventHandler = GetEventHandlerWidget();
	if (pEventHandler) pEventHandler->OnSelectChange(GetTagNo());

	this->nSelectedOption = -1;
}
