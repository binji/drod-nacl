// $Id: FlashMessageEffect.cpp 9784 2011-11-30 23:35:42Z mrimer $

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
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "FlashMessageEffect.h"
#include "FontManager.h"
#include "BitmapManager.h"
#include "Widget.h"

#include <BackEndLib/Assert.h>

#include <math.h>

//********************************************************************************
CFlashMessageEffect::CFlashMessageEffect(
//Constructor.
//
//
//Params:
	CWidget *pSetWidget,    //(in)   Should be a room widget.
	const WCHAR *text,      //(in)  Text to display
	const int yOffset,		//(in)  Offset from center of parent, in pixels [default=0]
	const Uint32 wDuration, //(in)  How long to display (in milliseconds) [default=3000]
	const Uint32 fadeTime)  //fade out time at end of duration [default=1000]
	: CEffect(pSetWidget)
	, pTextSurface(NULL)
	, yOffset(yOffset)
	, wDuration(wDuration)
	, fadeTime(fadeTime)
{
	ASSERT(pSetWidget->GetType() == WT_Room);
	ASSERT(text);
	ASSERT(wDuration);

	pSetWidget->GetRect(this->screenRect);
	this->dirtyRects.push_back(this->screenRect);

	this->wstrText = text;

	RenderText();
}

//********************************************************************************
CFlashMessageEffect::~CFlashMessageEffect()
{
	SDL_FreeSurface(this->pTextSurface);
}

//********************************************************************************
bool CFlashMessageEffect::Draw(SDL_Surface* pDestSurface)
//Draws a pulsing message in the middle of the parent widget.
{
	//End after duration has elapsed.
	const Uint32 elapsed = TimeElapsed();
	if (elapsed >= this->wDuration)
		return false;

	if (!pDestSurface) pDestSurface = GetDestSurface();

	//Pulse text size.
	static const float cycle = 600.0f; //ms
	static const float max_scale_factor = 0.1f;
	const float size_delta = 1.0f + (max_scale_factor * float(sin(TWOPI * elapsed / cycle)));
	const UINT scaled_w = ROUND(size_delta * this->base_size.w);
	const UINT scaled_h = ROUND(size_delta * this->base_size.h);

	//Scale.
	Uint8 *pSrcPixel = (Uint8*)this->pTextSurface->pixels;
	SDL_Surface *pScaledSurface = g_pTheBM->ScaleSurface(this->pTextSurface, pSrcPixel,
			this->base_size.w, this->base_size.h,
			scaled_w, scaled_h);
	if (!pScaledSurface)
		return false;

	//Center text in widget.
	const UINT xDraw = (this->screenRect.w - scaled_w) / 2;
	const UINT yDraw = this->yOffset + (this->screenRect.h - scaled_h) / 2;

	//Specify area of effect.
	ASSERT(this->dirtyRects.size() == 1);
	SDL_Rect rect = {this->screenRect.x + xDraw, this->screenRect.y + yDraw,
			scaled_w, scaled_h};
	this->dirtyRects[0] = rect;

	if (g_pTheBM->bAlpha) {
		const Uint32 time_left = this->wDuration - elapsed;
		if (time_left < this->fadeTime) {
			static const Uint8 start_opacity = 255;
			const float fFadePerMS = start_opacity / float(this->fadeTime);
			const Uint8 opacity = (Uint8)(time_left * fFadePerMS);
			SDL_SetAlpha(pScaledSurface, SDL_SRCALPHA, opacity);
		}
	}

	SDL_BlitSurface(pScaledSurface, NULL, pDestSurface, &rect);

	SDL_FreeSurface(pScaledSurface);

	return true;
}

//********************************************************************************
void CFlashMessageEffect::RenderText()
//Creates a surface with the text to display, so it doesn't have to be rerendered each frame.
{
	ASSERT(!this->pTextSurface);

	static const UINT eDrawFont = FONTLIB::F_FlashMessage;
	UINT w, h;
	g_pTheFM->GetTextWidthHeight(eDrawFont, this->wstrText.c_str(), w, h);
	this->dirtyRects[0].w = w;
	this->dirtyRects[0].h = h;
	this->base_size = this->dirtyRects[0];

	ASSERT(this->dirtyRects.size() == 1);
	this->pTextSurface = CBitmapManager::ConvertSurface(SDL_CreateRGBSurface(
			SDL_SWSURFACE, w, h, g_pTheBM->BITS_PER_PIXEL, 0, 0, 0, 0));
	ASSERT(this->pTextSurface);

	SDL_Color BG = g_pTheFM->GetFontBackColor(eDrawFont);
	const Uint32 TransparentColor = SDL_MapRGB(this->pTextSurface->format,
		BG.r, BG.g, BG.b);
	SDL_FillRect(this->pTextSurface, NULL, TransparentColor);   //make entire bg transparent
	g_pTheFM->DrawTextXY(eDrawFont, this->wstrText.c_str(), this->pTextSurface, 0, 0);
	SDL_SetColorKey(this->pTextSurface, SDL_SRCCOLORKEY, TransparentColor);
}
