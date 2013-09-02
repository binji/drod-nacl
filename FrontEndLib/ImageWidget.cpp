// $Id: ImageWidget.cpp 8019 2007-07-14 22:30:11Z trick $

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

#include "ImageWidget.h"

#include "BitmapManager.h"
#include <BackEndLib/Assert.h>

//
//Public methods.
//

//*****************************************************************************
CImageWidget::CImageWidget(
//Constructor.
//
//Params:
	const UINT dwSetTagNo,                //(in)   Required params for CWidget
	const int nSetX, const int nSetY,               //    constructor.
	SDL_Surface *pSurface)          //(in)   Image surface to display.
	: CWidget(WT_Image, dwSetTagNo, nSetX, nSetY, 0, 0)
	, pImageSurface(NULL)
	, alpha(255)
{
	if (pSurface)
		SetImage(pSurface);
	Load();
}

//*****************************************************************************
CImageWidget::CImageWidget(
//Constructor.
//
//Params:
	const UINT dwSetTagNo,                //(in)   Required params for CWidget
	const int nSetX, const int nSetY,               //    constructor.
	const WCHAR *pwczFilename)          //(in)   Image file to display.
	: CWidget(WT_Image, dwSetTagNo, nSetX, nSetY, 0, 0)
	, pImageSurface(NULL)
	, alpha(255)
{
	if (pwczFilename)
		SetImage(pwczFilename);
	Load();
}

//*****************************************************************************
CImageWidget::~CImageWidget()
{
	if (this->pImageSurface)
		SDL_FreeSurface(this->pImageSurface);

	if (IsLoaded())
		Unload();
}

//*****************************************************************************
void CImageWidget::Paint(bool /*bUpdateRect*/)
{
	if (!this->pImageSurface) return;
	ASSERT(this->w > 0);
	ASSERT(this->h > 0);
	ASSERT(this->w <= (UINT)this->pImageSurface->w);
	ASSERT(this->h <= (UINT)this->pImageSurface->h);

	int nOffsetX, nOffsetY;
	GetScrollOffset(nOffsetX, nOffsetY);

	SDL_Rect src = {0, 0, this->w, this->h};
	SDL_Rect dest;
	GetRect(dest);
	dest.x += nOffsetX;
	dest.y += nOffsetY;
	SDL_BlitSurface(this->pImageSurface, &src, GetDestSurface(), &dest);
}

//*****************************************************************************
void CImageWidget::SetAlpha(const Uint8 setAlpha)
{
	ASSERT(setAlpha > 0);
	this->alpha = setAlpha;
	if (this->pImageSurface)
	{
		if (this->alpha < 255 || this->pImageSurface->format->Amask)
			SDL_SetAlpha(this->pImageSurface, SDL_SRCALPHA, this->alpha);
		else
			SDL_SetAlpha(this->pImageSurface, 0, 0);	//don't need any alpha on this surface
	}
}

//*****************************************************************************
void CImageWidget::SetImage(SDL_Surface *pSurface)
//Sets the image to surface (may be NULL).
{
	if (this->pImageSurface) SDL_FreeSurface(this->pImageSurface);
	this->pImageSurface = NULL;

	this->pImageSurface = pSurface;
	if (this->pImageSurface)
	{
		this->w = this->pImageSurface->w;
		this->h = this->pImageSurface->h;
		SetAlpha(this->alpha);
	} else {
		this->w = this->h = 0;
	}

	this->wFilename = wszEmpty;	//don't know image name
}

//*****************************************************************************
void CImageWidget::SetImage(const WCHAR *pwczFilename)
{
	ASSERT(pwczFilename);

	if (this->wFilename == pwczFilename) return;	//don't reload same image

	if (this->pImageSurface) SDL_FreeSurface(this->pImageSurface);
	this->pImageSurface = NULL;

	ASSERT(g_pTheBM);
	const UINT wFormat = g_pTheBM->GetImageExtensionType(pwczFilename);
	if (wFormat)
		this->pImageSurface = g_pTheBM->LoadImageSurface(pwczFilename, wFormat);
	if (!this->pImageSurface)  //try to determine path if load didn't work
		this->pImageSurface = g_pTheBM->LoadImageSurface(pwczFilename);
	if (this->pImageSurface)
	{
		this->w = this->pImageSurface->w;
		this->h = this->pImageSurface->h;
		SetAlpha(this->alpha);
	} else {
		this->w = this->h = 0;
	}

	this->wFilename = pwczFilename;	//save image filename (reload optimization)
}
