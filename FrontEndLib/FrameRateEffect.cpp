// $Id: FrameRateEffect.cpp 8019 2007-07-14 22:30:11Z trick $

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

#include "FrameRateEffect.h"
#include "FontManager.h"
#include "Sound.h"
#include <BackEndLib/Assert.h>

//
//Public methods.
//

//*****************************************************************************
CFrameRateEffect::CFrameRateEffect(CWidget *pSetWidget)
	: CEffect(pSetWidget,EFFECTLIB::EFRAMERATE)
	, x(pOwnerWidget->GetX())
	, y(pOwnerWidget->GetY())
	, dwLastDrawTime(SDL_GetTicks())
	, wFrameCount(0)
	, fLastFrameCount(0.0)
//Constructor.
{
	//Even when other effects are cleared, it's generally okay to leave this one.
	RequestRetainOnClear();

	SDL_Rect rect = {this->x, this->y, 0, 0};
	this->dirtyRects.push_back(rect);
}

//*****************************************************************************
bool CFrameRateEffect::Draw(SDL_Surface* pDestSurface)
//Calc the frame rate and put it on the screen.
//
//Returns:
//True.
{
	//Calc the frame rate based on the amount of times this method is called
	//over the duration of one second.
	++this->wFrameCount;

	const UINT dwNow = SDL_GetTicks();
	if (dwNow >= this->dwLastDrawTime + 1000)
	{
		this->fLastFrameCount = this->wFrameCount * (1000.f / (float)(dwNow-this->dwLastDrawTime));
		this->fLastFrameCount *= 10.f;
		this->dwLastDrawTime = dwNow;
		this->wFrameCount = 0;
	}

	WSTRING wStr;
	WCHAR wczNum[10];
	wStr += _itoW(((int)this->fLastFrameCount) / 10, wczNum, 10);
	wStr += wszPeriod;
	wStr += _itoW(((int)this->fLastFrameCount) % 10, wczNum, 10);

	//Show some other stats too.
	if (g_pTheSound)
	{
		wStr += wszSpace;
		wStr += _itoW(g_pTheSound->GetMemoryUsage(), wczNum, 10);  //sound lib memory usage
	}

	if (!pDestSurface)
		pDestSurface = GetDestSurface();

	//Display frame rate in top-left corner of widget.
	g_pTheFM->DrawTextXY(FONTLIB::F_FrameRate, wStr.c_str(), pDestSurface,
			this->x, this->y);

	//Get area of effect.
	UINT cxDraw, cyDraw;
	g_pTheFM->GetTextWidthHeight(FONTLIB::F_FrameRate, wStr.c_str(), cxDraw, cyDraw);
	ASSERT(this->dirtyRects.size() == 1);
	this->dirtyRects[0].w = cxDraw;
	this->dirtyRects[0].h = cyDraw;

	//Effect will last forever.
	return true;
}
