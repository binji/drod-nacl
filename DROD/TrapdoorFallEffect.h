// $Id: TrapdoorFallEffect.h 9742 2011-10-22 16:12:27Z mrimer $

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

#ifndef TRAPDOORFALLEFFECT_H
#define TRAPDOORFALLEFFECT_H

#include "DrodEffect.h"
#include "TileImageConstants.h"

#include "../DRODLib/DbRooms.h"
#include "../DRODLib/CurrentGame.h"

//****************************************************************************************
class CTrapdoorFallEffect : public CEffect
{
public:
	CTrapdoorFallEffect(CWidget *pSetWidget, const CCoord &SetCoord, const vector<UINT>& tiles,
			const UINT tileFallTime=130);
	~CTrapdoorFallEffect();

	virtual bool   Draw(SDL_Surface* pDestSurface);
	virtual long   GetDrawSequence() const {return 1L + this->wRow;}

private:
	SDL_Surface *pSurface;
	CDbRoom *   pRoom;
	int         xTrapdoor, yTrapdoor;
	UINT     wCol, wRow;
	UINT		wTileNo;
	UINT tileFallTime;
};

#endif //...#ifndef TRAPDOORFALLEFFECT_H
