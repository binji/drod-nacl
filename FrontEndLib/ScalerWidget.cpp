// $Id: ScalerWidget.cpp 10030 2012-03-29 07:00:37Z mrimer $

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
 * mrimer
 *
 * ***** END LICENSE BLOCK ***** */

#include "ScalerWidget.h"
#include "BitmapManager.h"

#include <BackEndLib/Assert.h>
#include <BackEndLib/Wchar.h>

//
//Public methods.
//

//***************************************************************************************
CScalerWidget::CScalerWidget(
//Constructor.
//
//Params:
	const UINT dwSetTagNo, const int nSetX, const int nSetY, const UINT wSetW,
	const UINT wSetH, //(in) Passed to CWidget.
	const bool bShowQuick)  //[true]
	: CWidget(WT_Scaler, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)
	, bNewScaleDimensions(false)
	, pTrueScaleSurface(NULL)
	, ppTrueToDestScaleMap(NULL)
	, bShowQuick(bShowQuick)
	, nAntiAliasY(0)
	, bAntiAliasInProgress(false)
{
	this->pTrueScaleContainer = new CFrameWidget(TAG_UNSPECIFIED, 0, 0, 0, 0, wszEmpty);
}

//***************************************************************************************
CScalerWidget::~CScalerWidget()
//Destructor.
{
	delete this->pTrueScaleContainer;
}

//***************************************************************************************
int CScalerWidget::GetScaledX(const int nTrueX)
//Given an X coordinate on the true-scale (source) surface, what is the corresponding
//X coordinate on the scaled (destination) surface.
const
{
	SDL_Rect TrueRect;
	this->pTrueScaleContainer->GetRect(TrueRect);
	
	const double dblScaledXPixelsPerTrue = (double) this->w / (double) TrueRect.w;

	return this->x + static_cast<int>( ((double) nTrueX * dblScaledXPixelsPerTrue) );
}

//***************************************************************************************
int CScalerWidget::GetScaledY(const int nTrueY)
//Given a Y coordinate on the true-scale (source) surface, what is the corresponding
//Y coordinate on the scaled (destination) surface.
const
{
	SDL_Rect TrueRect;
	this->pTrueScaleContainer->GetRect(TrueRect);
	
	const double dblScaledYPixelsPerTrue = (double) this->h / (double) TrueRect.h;

	return this->y + static_cast<int>( ((double) nTrueY * dblScaledYPixelsPerTrue) );
}

//***************************************************************************************
int CScalerWidget::GetScaledW(const int nTrueW)
//Given a width on the true-scale (source) surface, what is the corresponding
//width on the scaled (destination) surface.
const
{
	SDL_Rect TrueRect;
	this->pTrueScaleContainer->GetRect(TrueRect);
	
	const double dblScaledXPixelsPerTrue = (double) this->w / (double) TrueRect.w;

	return static_cast<int>( ((double) nTrueW * dblScaledXPixelsPerTrue) );
}

//***************************************************************************************
int CScalerWidget::GetScaledH(const int nTrueH)
//Given a height on the true-scale (source) surface, what is the corresponding
//height on the scaled (destination) surface.
const
{
	SDL_Rect TrueRect;
	this->pTrueScaleContainer->GetRect(TrueRect);
	
	const double dblScaledYPixelsPerTrue = (double) this->h / (double) TrueRect.h;

	return static_cast<int>( ((double) nTrueH * dblScaledYPixelsPerTrue) );
}

//***************************************************************************************
bool CScalerWidget::Load()
//Load resources for the widget.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(!this->bIsLoaded);
	ASSERT(!this->pTrueScaleSurface);
	ASSERT(!this->ppTrueToDestScaleMap);

	SDL_Rect ContainerRect;
	this->pTrueScaleContainer->GetRect(ContainerRect);
	if (ContainerRect.w && ContainerRect.h)
	{
		if (!CreateNewTrueScaleSurface() || !CalcScaleInstructions()) 
			return false;
		this->pTrueScaleContainer->SetDestSurface(this->pTrueScaleSurface);
		this->bNewScaleDimensions = false;
	}

	//Load scaled and normal children.
	this->bIsLoaded = this->pTrueScaleContainer->LoadChildren() && LoadChildren();
	return this->bIsLoaded ;
}

//***************************************************************************************
void CScalerWidget::Unload()
//Unloads resources for the widget.
{
	ASSERT(this->bIsLoaded);

	//Unload any children widgets.
	UnloadChildren();
	this->pTrueScaleContainer->UnloadChildren();

	if (this->pTrueScaleSurface) 
	{
		SDL_FreeSurface(this->pTrueScaleSurface);
		this->pTrueScaleSurface = NULL;
	}
	
	if (this->ppTrueToDestScaleMap)
	{
		delete [] this->ppTrueToDestScaleMap;
		this->ppTrueToDestScaleMap = NULL;
	}
	
	//In case widget is reloaded, force reinits on everything.
	this->bNewScaleDimensions = true;
	this->bAntiAliasInProgress = false;

	this->bIsLoaded = false;
}

//***************************************************************************************
CWidget * CScalerWidget::AddScaledWidget(
//Adds the widget to a parentless container widget used to render widgets to
//destination.
//
//Params:
	CWidget *pNewWidget, //(in) Widget to add.
	bool bLoad)        //(in) Load the widget?
//
//Returns:
//The pNewWidget param or NULL if widget load failed.
{
	//Add the widget.
	if (!this->pTrueScaleContainer->AddWidget(pNewWidget, bLoad)) 
		return NULL; //Load failed.

	//Check for increase in child container rect.
	SDL_Rect ContainerRect, ChildrenRect;
	pTrueScaleContainer->GetRect(ContainerRect);
	pTrueScaleContainer->GetRectContainingChildren(ChildrenRect);
	if (!ARE_RECTS_EQUAL(ContainerRect, ChildrenRect))
	{
		//There must be at least one scaled widget at 0,0 or things aren't
		//going to work.  When adding widgets to CScalerWidget, make sure this 
		//is true.
		ASSERT(ChildrenRect.x == 0 && ChildrenRect.y == 0);

		//The container rect should only grow since you can only add scaled 
		//widgets, and can't remove them.
		ASSERT(ChildrenRect.w >= ContainerRect.w);
		ASSERT(ChildrenRect.h >= ContainerRect.h);

		pTrueScaleContainer->Resize(ChildrenRect.w, ChildrenRect.h);
		this->bNewScaleDimensions = true;
		this->bAntiAliasInProgress = false;
	}

	//If widget was added after CScalerWidget loaded, then set dest surface.
	if (this->pTrueScaleSurface)
		pNewWidget->SetDestSurface(this->pTrueScaleSurface);

	return pNewWidget;
}

//***************************************************************************************
void CScalerWidget::Resize(UINT wSetW, UINT wSetH)
//Resizes plus flags for new scaling dimensions.
{
	CWidget::Resize(wSetW, wSetH);
	this->bNewScaleDimensions = true;
	this->bAntiAliasInProgress = false;
}

//***************************************************************************************
void CScalerWidget::PaintClipped(
	const int /*nX*/, const int /*nY*/, const UINT /*wW*/, const UINT /*wH*/, const bool /*bUpdateRect*/)
{
	//Paint() uses direct access to pixels, so it can't be clipped with 
	//SDL_SetClipRect().  Either you need to write PaintClipped() or change
	//the situation which causes this widget to be clipped.
	ASSERTP(false, "ScalerWidget: Can't paint clipped.");
}

//***************************************************************************************
void CScalerWidget::Paint(
//Draws children to a hidden true-scale surface, then draws a scaled version to the
//widget area of the destination surface.
//
//Params:
	bool bUpdateRect)             //(in)   If true (default) and destination
										//    surface is the screen, the screen
										//    will be immediately updated in
										//    the widget's rect.
{
	//Drawing code below needs to be modified to accept offsets.  Until then,
	//this widget can't be offset.
	ASSERT(!IsScrollOffset());

	SDL_Rect ContainerRect;
	this->pTrueScaleContainer->GetRect(ContainerRect);
	if (ContainerRect.w == 0) 
	{
		//No children to paint yet.
		ASSERT(ContainerRect.x == 0);
		ASSERT(ContainerRect.y == 0);
		ASSERT(ContainerRect.h == 0);
		return;
	}

	//If scaling dimensions have changed, update true-scale surface and the 
	//instructions for scaling from true-scale to widget areas.
	if (this->bNewScaleDimensions)
	{
		if (!this->pTrueScaleSurface || 
				this->pTrueScaleSurface->w != ContainerRect.w ||
				this->pTrueScaleSurface->h != ContainerRect.h)
		{
			if (!CreateNewTrueScaleSurface()) {ASSERTP(false, "True scale surface alloc failed."); return;}
			this->pTrueScaleContainer->SetDestSurface(this->pTrueScaleSurface);
		}
		if (!CalcScaleInstructions()) {ASSERTP(false, "Calc scale instr. failed."); return;}
		this->bNewScaleDimensions = false;
		this->bAntiAliasInProgress = false;
	}

	//Paint scaled children to true-scale surface.
	this->pTrueScaleContainer->PaintChildren(false);

	if (this->bShowQuick)
	{
		//Draw quick scale to widget area.
		DrawScaledQuick();
		//Set up members to later draw anti-aliased during animate.
		//Try to anti-alias in about one second.
		this->nAntiAliasY = this->y;
		this->bAntiAliasInProgress = true;
	} else {
		//Draw anti-aliased now.
		DrawAntiAliasedLines(this->y);
	}

	//Paint children (non-scaled).
	PaintChildren();

	//Put it up on the screen.
	if (bUpdateRect) UpdateRect();
}

//
//Protected methods.
//

//***************************************************************************************
void CScalerWidget::HandleAnimate()
//Handles animation occurring between events.
{
	ASSERT(IsVisible());

	if (this->bAntiAliasInProgress)
	{  
		//I'm going to spend 25ms working on the anti-aliased image each time
		//HandleAnimate() is called.  HandleAnimate() probably won't be called 
		//more often than 33ms (30 fps) so this gives the event-handler a little
		//time to work on other things.  A higher value might result in choppiness.
		//A lower value would make the anti-aliasing take longer.  If it seems
		//like the value should be different on different screens, then we can
		//add a method to set the maximum time to spend per animate.
		const UINT MAX_TIME_PER_ANIMATE = 25;

		//Anti-alias the lines.
		UINT wAntiAliasLineCount =
				DrawAntiAliasedLines(this->nAntiAliasY, MAX_TIME_PER_ANIMATE);

		//Update anti-aliasing status.
		this->nAntiAliasY += wAntiAliasLineCount;
		this->bAntiAliasInProgress = (this->nAntiAliasY < static_cast<int>(this->y + this->h));

		//Paint children (non-scaled).
		PaintChildren();

		//Show finished anti-aliasing when done.
		if (!this->bAntiAliasInProgress) UpdateRect();
	}
}

//
//Private methods.
//

//***************************************************************************************
void CScalerWidget::DrawScaledQuick()
//Do a quick scaled draw from true-scale surface to widget area on screen surface.
{
	ASSERT(!this->bNewScaleDimensions);
	ASSERT(this->pTrueScaleSurface);
	ASSERT(this->ppTrueToDestScaleMap);

	SDL_Surface *pDestSurface = LockDestSurface();
	const UINT wBPP = pDestSurface->format->BytesPerPixel;
	ASSERT(wBPP >= 3);
#if (GAME_BYTEORDER == GAME_BYTEORDER_BIG)
	ASSERT(!"needs to be tested");
	ASSERT(wBPP == 4);
	ASSERT(pDestSurface->format->Rmask == 0xff0000);
	ASSERT(pDestSurface->format->Gmask == 0x00ff00);
	ASSERT(pDestSurface->format->Bmask == 0x0000ff);
	ASSERT(pTrueScaleSurface->format->BytesPerPixel == 4);
	ASSERT(pTrueScaleSurface->format->Rmask == 0xff0000);
	ASSERT(pTrueScaleSurface->format->Gmask == 0x00ff00);
	ASSERT(pTrueScaleSurface->format->Bmask == 0x0000ff);
#endif

	//Set pointer to first element of map.  There is one element for each
	//destination pixel that points to a pixel in the true-scale surface to be copied.
	Uint8 * *ppSrc = this->ppTrueToDestScaleMap;
	
	//Calc offset between end of a row and beginning of next.
	UINT dwRowOffset = (pDestSurface->w - this->w) * wBPP;

	//Calc location of topleft pixel.
	Uint8 *pSeek = (Uint8 *)(pDestSurface->pixels) + 
			( (this->y * pDestSurface->w) + this->x) * wBPP + PIXEL_FUDGE_FACTOR;

	//Calc location of bottomleft pixel plus one row too far.
	Uint8 *pEndOfSeek = pSeek + (this->h * pDestSurface->w * wBPP);

	//Each iteration fills one row of pixels.
	Uint8 *pEndOfRow, *pSrc;
	while (pSeek != pEndOfSeek)
	{
		ASSERT(pSeek < pEndOfSeek);
		pEndOfRow = pSeek + (this->w * wBPP);

		//Each iteration sets 3 bytes (1 pixel) from the scale map.
		while (pSeek != pEndOfRow)
		{
			pSrc = *(ppSrc + PIXEL_FUDGE_FACTOR);
			pSeek[0] = pSrc[0];
			pSeek[1] = pSrc[1];
			pSeek[2] = pSrc[2];
			pSeek += wBPP;

			++ppSrc;
		}

		//Advance to beginning of next row.
		pSeek += dwRowOffset;
	}

	UnlockDestSurface();
}

//***************************************************************************************
UINT CScalerWidget::DrawAntiAliasedLines(
//Draw one or more destination lines using the true-scale surface for source.
//Pixels will be anti-aliased by averaging their neighbors.
//
//Params:
	const int nStartY,   //(in)   Destination line at which to begin drawing.
	const UINT dwMaxTime)  //(in)   Maximum amount of time, in milliseconds to spend
						//    in routine.  Actually, routine is likely to go
						//    a tiny bit longer.  [default=0: no maximum]
//
//Returns:
//Number of lines that were drawn.
{
	const UINT dwStopTime = SDL_GetTicks() + dwMaxTime;

	//Number of source pixels to move for each dest pixel.
	SDL_Rect ContainerRect;
	this->pTrueScaleContainer->GetRect(ContainerRect);
	const double dblIncSrcX = (double) ContainerRect.w / (double) this->w;
	const double dblIncSrcY = (double) ContainerRect.h / (double) this->h;
	
	//Algorithm is incorrect if dest is less than half the size of source.
	//It only uses a maximum of four source pixels to average into one dest pixel.
	ASSERT(dblIncSrcX <= 2 && dblIncSrcY <= 2);
	
	//Lock dest surface for pixel copying.
	SDL_Surface *pDestSurface = LockDestSurface();
	const UINT wBPP = pDestSurface->format->BytesPerPixel;
	ASSERT(wBPP >= 3);
#if (GAME_BYTEORDER == GAME_BYTEORDER_BIG)
	ASSERT(wBPP == 4);
	ASSERT(pDestSurface->format->Rmask == 0xff0000);
	ASSERT(pDestSurface->format->Gmask == 0x00ff00);
	ASSERT(pDestSurface->format->Bmask == 0x0000ff);
	ASSERT(pTrueScaleSurface->format->BytesPerPixel == 4);
	ASSERT(pTrueScaleSurface->format->Rmask == 0xff0000);
	ASSERT(pTrueScaleSurface->format->Gmask == 0x00ff00);
	ASSERT(pTrueScaleSurface->format->Bmask == 0x0000ff);
#endif

	//Copy pixels over.
	Uint8 *pSrc, *pSrcTop, *pDest, *pSrcEast, *pSrcSouth, *pSrcSoutheast;
	UINT xSrc, ySrc;
	double dblSouthOffsetPercent, dblEastOffsetPercent, dblSoutheastOffsetPercent;
	double dblSrcPercent, dblSrcX;
	double dblSrcY = dblIncSrcY * (double) (nStartY - this->y);

	//shorthand vars for speed optimization
	const UINT yEnd = this->y + this->h;
	const UINT xEnd = this->x + this->w;
	const UINT yEndMinusOne = yEnd - 1;
	const UINT xEndMinusOne = xEnd - 1;
	const UINT thisx = this->x;
	const UINT wPitch =  this->pTrueScaleSurface->pitch;

	//Loop vars.
	double yPercent;
	double xPercent;
	double OneMinusYPercent;

	//Each iteration draws one anti-aliased line.
	UINT yDest;
	for (yDest = nStartY; yDest < yEndMinusOne; ++yDest)
	{
		dblSrcX = 0;
		ySrc = (UINT) dblSrcY;
		yPercent = dblSrcY - ySrc; //shorthand
		OneMinusYPercent = 1.0 - yPercent;

		//init
		pDest = (Uint8 *)(pDestSurface->pixels) +
					(yDest * pDestSurface->pitch) + (this->x * wBPP) + PIXEL_FUDGE_FACTOR;
		pSrcTop = (Uint8 *)(this->pTrueScaleSurface->pixels) +
					(ySrc * wPitch) + PIXEL_FUDGE_FACTOR;

		//Each iteration draws one anti-aliased pixel.
		for (UINT xDest = thisx; xDest < xEndMinusOne; ++xDest)
		{
			xSrc = (UINT) dblSrcX;
			xPercent = dblSrcX - xSrc; //shorthand

			dblSouthOffsetPercent = yPercent * (1.0 - xPercent);
			dblEastOffsetPercent = xPercent * OneMinusYPercent;
			dblSoutheastOffsetPercent = yPercent * xPercent;
			ASSERT(dblEastOffsetPercent + dblSouthOffsetPercent +
					dblSoutheastOffsetPercent <= 1.0);
			dblSrcPercent = 1.0 - dblSoutheastOffsetPercent - dblEastOffsetPercent -
				dblSouthOffsetPercent;

			//last row and column are skipped to avoid lots of bounds logic here
			pSrc = pSrcTop + (xSrc * wBPP);
			pSrcEast = pSrc + wBPP;
			pSrcSouth = pSrc + wPitch;
			pSrcSoutheast = pSrcSouth + wBPP;

			pDest[0] = static_cast<Uint8>(pSrc[0] * dblSrcPercent +
						pSrcEast[0] * dblEastOffsetPercent +
						pSrcSouth[0] * dblSouthOffsetPercent +
						pSrcSoutheast[0] * dblSoutheastOffsetPercent);

			pDest[1] = static_cast<Uint8>(pSrc[1] * dblSrcPercent +
						pSrcEast[1] * dblEastOffsetPercent +
						pSrcSouth[1] * dblSouthOffsetPercent +
						pSrcSoutheast[1] * dblSoutheastOffsetPercent);

			pDest[2] = static_cast<Uint8>(pSrc[2] * dblSrcPercent +
						pSrcEast[2] * dblEastOffsetPercent +
						pSrcSouth[2] * dblSouthOffsetPercent +
						pSrcSoutheast[2] * dblSoutheastOffsetPercent);

			dblSrcX += dblIncSrcX;
			pDest += wBPP;
		}
		dblSrcY += dblIncSrcY;

		if (dwMaxTime && SDL_GetTicks() > dwStopTime) break;
	}

	UnlockDestSurface();

	return (yDest - nStartY) + 1;
}

//***************************************************************************************
bool CScalerWidget::CreateNewTrueScaleSurface()
//Creates a new surface that can display all of the child widgets in true scale.
//
//Returns:
//True if successful, false if not.
{
	SDL_Rect ContainerRect;
	this->pTrueScaleContainer->GetRect(ContainerRect);
	ASSERT(ContainerRect.w != 0 && ContainerRect.h!=0);

	//If I already have a true-scale surface, free it.
	if (this->pTrueScaleSurface)
	{
		SDL_FreeSurface(this->pTrueScaleSurface);
		this->pTrueScaleSurface = NULL;
	}

	//Create the new surface.
	this->pTrueScaleSurface = CBitmapManager::ConvertSurface(SDL_CreateRGBSurface(
			SDL_SWSURFACE, ContainerRect.w, ContainerRect.h, g_pTheBM->BITS_PER_PIXEL, 0, 0, 0, 0));
	
	//Delete the true-to-dest scale map since it has pointers into the deleted
	//pixel data.
	if (this->ppTrueToDestScaleMap)
	{
		delete [] this->ppTrueToDestScaleMap;
		this->ppTrueToDestScaleMap = NULL;
	}

	return (this->pTrueScaleSurface != NULL);
}

//***************************************************************************************
bool CScalerWidget::CalcScaleInstructions()
//Calculate instructions for scaling.
//
//Returns:
//True if successful, false if not.
{
	ASSERT(this->w && this->h);
	ASSERT(this->pTrueScaleSurface);

	//Delete the true-to-dest scale map because the destination size may have changed.
	if (this->ppTrueToDestScaleMap)
	{
		delete [] this->ppTrueToDestScaleMap;
		this->ppTrueToDestScaleMap = NULL;
	}

	//Alloc a new true-to-dest scale map.
	this->ppTrueToDestScaleMap = new Uint8 *[this->w * this->h];

	//Calc number of source pixels to move for each dest pixel.
	SDL_Rect ContainerRect;
	this->pTrueScaleContainer->GetRect(ContainerRect);
	const double dblIncSrcX = (double) ContainerRect.w / (double) this->w;
	const double dblIncSrcY = (double) ContainerRect.h / (double) this->h;
	
	//Set up some shorthand vars for better speed in loop.
	const UINT yEnd = this->y + this->h;
	const UINT xEnd = this->x + this->w;
	const UINT wPitch =  this->pTrueScaleSurface->pitch;
	const UINT wBPP = this->pTrueScaleSurface->format->BytesPerPixel;
	ASSERT(wBPP);
	const int thisx = this->x;
	Uint8 *pSrcPixels = static_cast<Uint8 *>(this->pTrueScaleSurface->pixels);
#if GAME_BYTEORDER == GAME_BYTEORDER_BIG
		//NOTE:  no "fudgeFactor" applied here; only in drawing routines.
#endif

	//Each iteration fills map for one destination row.
	UINT xSrc, ySrc;
	double dblSrcX, dblSrcY = 0;
	Uint8 *pSrcTop, *pSrc;
	Uint8 * *ppMapElement = this->ppTrueToDestScaleMap;
	UINT xDest, yDest;
	for (yDest = this->y; yDest < yEnd; ++yDest)
	{
		dblSrcX = 0;
		ySrc = static_cast<UINT>(dblSrcY);
		pSrcTop = pSrcPixels +  (ySrc * wPitch);

		//Each iteration fills map for one destination pixel.
		for (xDest = thisx; xDest < xEnd; ++xDest)
		{
			xSrc = static_cast<UINT>(dblSrcX);
			pSrc = pSrcTop + (xSrc * wBPP);
			*(ppMapElement++) = pSrc;

			dblSrcX += dblIncSrcX;
		}

		dblSrcY += dblIncSrcY;
	}

	ASSERT(ppMapElement - this->ppTrueToDestScaleMap == static_cast<int>(this->w * this->h));

	return true;
}
