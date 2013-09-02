// $Id: ShadeEffect.cpp 8019 2007-07-14 22:30:11Z trick $

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
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "ShadeEffect.h"

//********************************************************************************
CShadeEffect::CShadeEffect(
//Constructor.
//
//Params:
	CWidget *pSetWidget,       //(in)   Should be a room widget.
	const CCoord &SetCoord,    //(in)   Location of checkpoint.
	const SURFACECOLOR &Color) //(in)   Color to shade with.
	: CAnimatedTileEffect(pSetWidget,SetCoord,0,0,false,EFFECTLIB::ESHADE)
	, Color(Color)
{
}

//********************************************************************************
bool CShadeEffect::Draw(SDL_Surface* pDestSurface)
//Draw the effect.  Never ends.
//
//Returns:
//True if effect should continue, or false if effect is done.
{
	if (!pDestSurface)
		pDestSurface = GetDestSurface();

	//Add shading to tile.
	ShadeTile(this->Color, pDestSurface);

	//Continue effect.
	return true;
}
