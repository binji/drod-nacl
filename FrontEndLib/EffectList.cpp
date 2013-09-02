// $Id: EffectList.cpp 8139 2007-08-25 06:49:36Z trick $

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

#include "EffectList.h"
#include <BackEndLib/Assert.h>
#include "Screen.h"
#include "AnimatedTileEffect.h"

using namespace std;

//*****************************************************************************
CEffectList::CEffectList(CWidget *pOwnerWidget)
	: pOwnerWidget(pOwnerWidget)
	, pOwnerScreen(NULL)
	, dwTimeEffectsWereFrozen(0L)
{
	if (pOwnerWidget && pOwnerWidget->eType == WT_Screen)
		this->pOwnerScreen = DYN_CAST(CScreen*, CWidget*, pOwnerWidget);
}

CEffectList::~CEffectList()
{
	Clear(false, true);
}

//*****************************************************************************
void CEffectList::AddEffect(
//Adds an effect to the effect list.
//
//Params:
	CEffect *pEffect)          //(in)   Effect to add.
{
	ASSERT(pEffect);
	if (!pEffect) return;   //just-in-case handling

	//List is sorted by draw sequence--smaller values are at beginning.
	//Add effect in front of the first element with a larger or equal draw
	//sequence.
	for (list<CEffect *>::iterator iSeek = this->Effects.begin();
		iSeek != this->Effects.end(); ++iSeek)
	{
		CEffect *pSeekEffect = *iSeek;
		if (pSeekEffect->GetDrawSequence() >= pEffect->GetDrawSequence())
		{
			this->Effects.insert(iSeek, pEffect);
			return;
		}
	}

	//Effect has the largest draw sequence value so it goes at end of list.
	this->Effects.push_back(pEffect);
}

//*****************************************************************************
void CEffectList::Clear(
//Clears all effects from the effect list.
//
//Params:
	const bool bRepaint, //(in)   Touch up affected screen areas before deleting
								//(default = false)
	const bool bForceClearAll) //if set [default=true], delete all effects,
	                     //including those that request to be retained
{
	list<CEffect*> retained;
	for (list<CEffect *>::const_iterator iSeek = this->Effects.begin();
		iSeek != this->Effects.end(); ++iSeek)
	{
		CEffect *pEffect = *iSeek;
		ASSERT(pEffect);
		if (pEffect->bRequestRetainOnClear && !bForceClearAll)
		{
			retained.push_back(pEffect);
			continue;
		}
		if (bRepaint)
		{
			this->pOwnerWidget->RequestPaint();
			if (this->pOwnerScreen)
			{
				for (UINT wIndex=pEffect->dirtyRects.size(); wIndex--; )
					this->pOwnerScreen->UpdateRect(pEffect->dirtyRects[wIndex]);
			}
		}
		delete pEffect;
	}
	this->Effects = retained;
	this->dwTimeEffectsWereFrozen = 0;
}

//*****************************************************************************
bool CEffectList::ContainsEffect(CEffect *pEffect) const
//Returns whether this effect is in the list.
{
	for (list<CEffect *>::const_iterator iSeek = this->Effects.begin();
		iSeek != this->Effects.end(); ++iSeek)
	{
		if (pEffect == (*iSeek))
			return true;
	}

	return false;
}

//*****************************************************************************
bool CEffectList::ContainsEffectOfType(
//Returns true if effect list contains an effect of stated type, else false.
//
//Params:
	const UINT eEffectType) //(in)
const
{
	return GetEffectOfType(eEffectType) != NULL;
}

//*****************************************************************************
bool CEffectList::ContainsEffectOfTypeAt(
//Returns true if effect list contains an effect of stated type at the
//stated position, else false.
//
//Pre-Cond: effect searched for must be derived from an CAnimatedTileEffect
//
//Params:
	const UINT eEffectType, //(in)
	const UINT wX, const UINT wY) //(in) position
const
{
	CAnimatedTileEffect *pTileEffect;
	for (list<CEffect *>::const_iterator iSeek = this->Effects.begin();
		iSeek != this->Effects.end(); ++iSeek)
	{
		if (eEffectType == (*iSeek)->GetEffectType())
		{
			pTileEffect = DYN_CAST(CAnimatedTileEffect*, CEffect*, *iSeek);
			if (wX == pTileEffect->GetX() && wY == pTileEffect->GetY())
				return true;
		}
	}

	return false;
}

//*****************************************************************************
void CEffectList::DrawEffects(
//Draws list of effects.
//If freezing effects, save time they are frozen to preserve their frame #.
//When unfreezing, update their start time to now.
//
//Params:
	const bool bUpdateScreen,  //(in) Whether screen is updated where effects are drawn
                              //[default = false]
	const bool bFreezeEffects, //(in) Whether effects are frozen after this draw.
										//[default = false]
	SDL_Surface *pDestSurface) //(in) where to draw effects (default = NULL)
{
	list<CEffect *>::const_iterator iSeek = this->Effects.begin();
	bool bRepaintScreen = false;
	while (iSeek != this->Effects.end())
	{
		CEffect *pEffect = *iSeek;
		ASSERT(pEffect);
		//Each iteration draws one effect.
		if (!bFreezeEffects && this->dwTimeEffectsWereFrozen &&
				pEffect->dwTimeStarted <= this->dwTimeEffectsWereFrozen)
		{
			//Unfreeze effect where it left off.
			if (pEffect->dwTimeStarted) //only update time for effects that have drawn at least once
			{
				const Uint32 dwTimeOffset = SDL_GetTicks() - this->dwTimeEffectsWereFrozen;
				pEffect->dwTimeStarted += dwTimeOffset;
				pEffect->dwTimeOfLastMove += dwTimeOffset;
			}
		}

		++iSeek;
		if (pEffect->Draw(pDestSurface))
		{
			if (bUpdateScreen && this->pOwnerScreen)
				for (UINT wIndex=pEffect->dirtyRects.size(); wIndex--; )
					this->pOwnerScreen->UpdateRect(pEffect->dirtyRects[wIndex]);
		} else {
			//Effect is finished--remove from list.
			bRepaintScreen = true; //parent display needs refreshing to erase finished effect
			this->Effects.remove(pEffect);
			delete pEffect;
		}
	}
	if (bRepaintScreen && this->pOwnerScreen)
		this->pOwnerScreen->RequestPaint();  //refresh area of removed effects

	this->dwTimeEffectsWereFrozen = (bFreezeEffects ? SDL_GetTicks() : 0L);
}

//*****************************************************************************
void CEffectList::EraseEffects(
//Erases the area drawn over by the effect list by repainting a given
//background in its place.
//Call when effects are painted over a background that is otherwise never repainted,
//like over a CScreen that is normally painted only once.
//
//Params:
	SDL_Surface* pBackground, const SDL_Rect& rect,	//(in) background under effects
	const bool bUpdate)	//(in) [default=false]
{
	ASSERT(pBackground);

	for (list<CEffect *>::const_iterator iSeek = this->Effects.begin();
		iSeek != this->Effects.end(); ++iSeek)
	{
		CEffect *pEffect = *iSeek;
		ASSERT(pEffect);
		ASSERT(pEffect->pOwnerWidget);
		SDL_Surface *pDestSurface = pEffect->pOwnerWidget->GetDestSurface();
		for (UINT wIndex=pEffect->dirtyRects.size(); wIndex--; )
		{
			//Repaint background over this dirty region.
			SDL_Rect& dirty = pEffect->dirtyRects[wIndex];
			SDL_Rect srcRect = {rect.x + dirty.x, rect.y + dirty.y,
					dirty.w, dirty.h};
			SDL_BlitSurface(pBackground, &srcRect, pDestSurface, &dirty);
			if (bUpdate && this->pOwnerScreen)
				this->pOwnerScreen->UpdateRect(dirty);
		}
	}
}

//*****************************************************************************
CEffect* CEffectList::GetEffectOfType(const UINT eEffectType) const
//Returns: pointer to first effect of specified type if exists, else NULL
{
	for (list<CEffect *>::const_iterator iSeek = this->Effects.begin();
		iSeek != this->Effects.end(); ++iSeek)
	{
		if (eEffectType == (*iSeek)->GetEffectType())
			return *iSeek;
	}

	return NULL;
}

//*****************************************************************************
void CEffectList::SetOpacityForEffectsOfType(const UINT eEffectType, float fOpacity) const
{
	for (list<CEffect *>::const_iterator iSeek = this->Effects.begin();
		iSeek != this->Effects.end(); ++iSeek)
	{
		if (eEffectType == (*iSeek)->GetEffectType())
			(*iSeek)->SetOpacity(fOpacity);
	}
}

//*****************************************************************************
void CEffectList::RemoveEffectsOfType(
//Removes all effects of given type from the list.
//
//Params:
	const UINT eEffectType) //(in)   Type of effect to remove.
{
	bool bRepaint = false;

	//Clear list of given effect type.
	list<CEffect *>::const_iterator iSeek = this->Effects.begin();
	while (iSeek != this->Effects.end())
	{
		CEffect *pEffect = *iSeek;
		ASSERT(pEffect);
		++iSeek;
		if (eEffectType == pEffect->GetEffectType())
		{
			//Remove from list.

			//Damage screen area.
			bRepaint = true;
			if (this->pOwnerScreen)
				for (UINT wIndex=pEffect->dirtyRects.size(); wIndex--; )
					this->pOwnerScreen->UpdateRect(pEffect->dirtyRects[wIndex]);

			this->Effects.remove(pEffect);
			delete pEffect;
		}
	}

	if (bRepaint)
		this->pOwnerWidget->RequestPaint();
}
