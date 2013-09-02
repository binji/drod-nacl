// $Id: VarMonitorEffect.cpp 9742 2011-10-22 16:12:27Z mrimer $

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
 * Portions created by the Initial Developer are Copyright (C) 2006
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "VarMonitorEffect.h"
#include "DrodFontManager.h"
#include "RoomWidget.h"
#include "../DRODLib/DbHolds.h"
#include <BackEndLib/Assert.h>

//
//Public methods.
//

//*****************************************************************************
CVarMonitorEffect::CVarMonitorEffect(CWidget *pSetWidget)
	: CEffect(pSetWidget, EVARMONITOR)
	, x(pOwnerWidget->GetX())
	, y(pOwnerWidget->GetY())
	, lastTurn((UINT)-1)
	, lastUpdate(SDL_GetTicks())
//Constructor.
{
	//Even when other effects are cleared, it's generally okay to leave this one.
	RequestRetainOnClear();

	SDL_Rect rect = {this->x, this->y, 0, 0};
	this->dirtyRects.push_back(rect);

	this->pRoomWidget = DYN_CAST(CRoomWidget*, CWidget*, pSetWidget);

	//Get current var state.
	CCurrentGame *pGame = (CCurrentGame*)pRoomWidget->GetCurrentGame();
	if (pGame)
		pGame->GetVarValues(this->lastVars);
}

//*****************************************************************************
bool CVarMonitorEffect::Draw(SDL_Surface* pDestSurface)
//Calc the frame rate and put it on the screen.
//
//Returns: True (display indefinitely)
{
	if (!pDestSurface)
		pDestSurface = GetDestSurface();

	const UINT turn = this->pRoomWidget->GetLastTurn();
	if (turn != this->lastTurn)
	{
		//Check vars for updated state.
		CCurrentGame *pGame = (CCurrentGame*)pRoomWidget->GetCurrentGame();
		VARMAP curVars;
		set<VarNameType> diff;
		if (pGame)
		{
			pGame->GetVarValues(curVars);
			pGame->DiffVarValues(this->lastVars,curVars,diff);
		}
		if (!diff.empty())
		{
			//Vars have changed.  Update display.
			this->lastUpdate = SDL_GetTicks();
			this->text.resize(0);
			for (set<VarNameType>::const_iterator var = diff.begin(); var != diff.end(); ++var)
			{
				//Print changed var names and values.
				WSTRING temp;
				const char *pVarName = var->c_str();
				AsciiToUnicode(pVarName, temp);
				char *varID = pGame->pHold->getVarAccessToken(temp.c_str());
				this->text += temp;
				this->text += wszSpace;
				this->text += wszEqual;
				this->text += wszSpace;
				const UNPACKEDVARTYPE vType = pGame->stats.GetVarType(varID);
				const bool bValidInt = vType == UVT_int || vType == UVT_unknown;
				if (bValidInt)
				{
					WCHAR temp[16];
					const int val = pGame->stats.GetVar(varID, int(0));
					this->text += _itoW(val, temp, 10);
				} else {
					this->text += pGame->stats.GetVar(varID, wszEmpty);
				}
				this->text += wszCRLF;
			}
			this->lastVars = curVars;
		}

		this->lastTurn = turn;
	}

	//Fade out after a while.
	static const UINT timeToFadeBegin = 2000, timeToFadeEnd = 3000; //ms
	const Uint32 timeSinceLastUpdate = SDL_GetTicks() - this->lastUpdate;
	Uint8 opacity = timeSinceLastUpdate <= timeToFadeBegin ? 255 :
			timeSinceLastUpdate >= timeToFadeEnd ? 0 :
			(timeToFadeEnd - timeSinceLastUpdate)*255 / (timeToFadeEnd - timeToFadeBegin);

	UINT cxDraw=0, cyDraw=0;
	if (opacity > 0)
	{
		//Display in top-left corner of parent widget.
		g_pTheFM->DrawTextToRect(F_FrameRate, this->text.c_str(), this->x, this->y,
			this->pRoomWidget->GetW(), this->pRoomWidget->GetH(), pDestSurface, 0, opacity);

		//Dirty area of effect.
		g_pTheFM->GetTextRectHeight(F_FrameRate, this->text.c_str(), this->pRoomWidget->GetW(), cxDraw, cyDraw);
	}
	ASSERT(this->dirtyRects.size() == 1);
	this->dirtyRects[0].w = cxDraw;
	this->dirtyRects[0].h = cyDraw;

	//Effect will last forever.
	return true;
}

//*****************************************************************************
void CVarMonitorEffect::SetText(const WCHAR* pText)
//Sets text being displayed.
{
	this->text = pText ? pText : wszEmpty;
}
