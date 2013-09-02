// $Id: AnimatedTileEffect.cpp 8019 2007-07-14 22:30:11Z trick $

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
 * Portions created by the Initial Developer are Copyright (C) 2002, 2005, 2008
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "MovingTileEffect.h"
#include "BitmapManager.h"
#include <BackEndLib/Assert.h>

#include <math.h>

//********************************************************************************
CMovingTileEffect::CMovingTileEffect(
//Constructor.
//
//Params:
	CWidget *pSetWidget,       //(in) Should be a room widget.
	const UINT wTileNo,		   //(in) Tile to draw.
	const CCoord &StartCoord,  //(in) Starting location of moving image (in room tiles)
	const CCoord &DestCoord,   //(in) End location (in pixels)
	const float fSpeed,        //(in) Pixels per second
	const bool bUseLightLevel, //(in) whether to draw at light level specified in bitmap manager [default=false]
	const UINT eType)          //(in) Type of effect [default=EFFECTLIB::EGENERIC]
	: CAnimatedTileEffect(pSetWidget,StartCoord,UINT(-1),wTileNo,bUseLightLevel,eType)
	, behavior(Accelerating)
	, fSpeed(fSpeed)
	, bEndEffectAtDestination(true)
{
	ASSERT(fSpeed > 0.0);

	this->fX = (float)this->wX;
	this->fY = (float)this->wY;

	this->wMoveToX = DestCoord.wX;
	this->wMoveToY = DestCoord.wY;
}

//*****************************************************************************
void CMovingTileEffect::MoveTo(
//Say to move effect to (x,y) at specified speed.
//The position is updated each time ::DrawTile is invoked.
//If DrawTile is not called, UpdateLocation may be called each frame instead.
//
//Params:
	const UINT wX, const UINT wY, //(in) destination (in pixels)
	const float fSpeed,           //number of pixels per second to move
	                              //to destination.  A speed of 0 (default)
											//means to move instantly.
	const bool bEndEffectAtDestination) //(in) [default=false]
{
	this->wMoveToX = wX;
	this->wMoveToY = wY;
	this->fSpeed = fSpeed;
	this->bEndEffectAtDestination = bEndEffectAtDestination;
}

//********************************************************************************
bool CMovingTileEffect::UpdateLocation()
//Move effect to a different location, if specified.
//
//Returns: whether already at destination
{
	if (this->wMoveToX == this->wX && this->wMoveToY == this->wY)
		return true; //already at destination

	if (this->fSpeed == 0.0)
	{
		//Move instantly to destination.
		this->wX = this->wMoveToX;
		this->wY = this->wMoveToY;
		this->fX = float(this->wX);
		this->fY = float(this->wY);
	} else {
		//Move according to indicated speed.
		const Uint32 dwNow = SDL_GetTicks();
		int deltaTime;
		switch (this->behavior)
		{
			case Accelerating:
				deltaTime = TimeElapsed();
			break;
			case UniformSpeed:
			default:
				deltaTime = dwNow - this->dwTimeOfLastMove;
			break;
		}
		this->dwTimeOfLastMove = dwNow;
		if (deltaTime <= 0)
			deltaTime = 1;

		//Amount of movement (in pixels) this frame
		const float fVel = (this->fSpeed * deltaTime) / 1000.0f;

		//Move in straight line to destination.
		const float fDeltaX = this->wMoveToX - this->fX;
		const float fDeltaY = this->wMoveToY - this->fY;
		const float fAngle = atan2(fDeltaY, fDeltaX ? fDeltaX : 0.00001f);

		const float fNewX = this->fX + cos(fAngle) * fVel;
		const float fNewY = this->fY + sin(fAngle) * fVel;

		//Ensure we don't overshoot our destination.
		if (((this->fX-this->wMoveToX) * (fNewX-this->wMoveToX)) < 0.0)
		{
			//a negative value implies opposite signs, which in turn indicates
			//opposing trajectories.  In other words, the new location is
			//past the destination.
			this->fX = (float)this->wMoveToX;
		}
		else
			this->fX = fNewX;
		if (((this->fY-this->wMoveToY) * (fNewY-this->wMoveToY)) < 0.0)
			this->fY = (float)this->wMoveToY;
		else
			this->fY = fNewY;

		//Draw at this location.
		this->wX = (UINT)this->fX;
		this->wY = (UINT)this->fY;
	}

	this->dirtyRects[0].x = this->wX;
	this->dirtyRects[0].y = this->wY;

	return false;
}

//********************************************************************************
bool CMovingTileEffect::Draw(SDL_Surface* pDestSurface)
//Draw a tile, moving it to the indicated destination.
//
//Returns:
//True if effect should continue, or false if effect is done.
{
	if (TimeElapsed() >= this->dwDuration)
		return false; //Effect is done.

	if (!pDestSurface)
		pDestSurface = GetDestSurface();

	const bool bEndEffect = UpdateLocation() && this->bEndEffectAtDestination;

	//Make sure effect is drawn only within parent widget bounds.
	ASSERT(this->pOwnerWidget);
	SDL_Rect OwnerRect, BlitRect = {0, 0, CBitmapManager::CX_TILE, CBitmapManager::CY_TILE};
	this->pOwnerWidget->GetRect(OwnerRect);
	if (!CWidget::GetBlitRectFromClipRect(this->wX, this->wY, OwnerRect, BlitRect))
		return !bEndEffect;

	if (!bEndEffect)
	{
		//Draw part of image within parent area.
		g_pTheBM->BlitTileImagePart(this->wTileNo,
				this->wX + BlitRect.x, this->wY + BlitRect.y,
				BlitRect.x, BlitRect.y, BlitRect.w, BlitRect.h,
				pDestSurface, this->bUseLightLevel);
	}

	return true;
}
