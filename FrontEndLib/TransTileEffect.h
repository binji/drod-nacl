// $Id: TransTileEffect.h 8019 2007-07-14 22:30:11Z trick $

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

#ifndef TRANSTILEEFFECT_H
#define TRANSTILEEFFECT_H

#include "AnimatedTileEffect.h"

//****************************************************************************************
class CTransTileEffect : public CAnimatedTileEffect
{
public:
	CTransTileEffect(CWidget *pSetWidget, const CCoord &SetCoord,
			const UINT wTileImageNo, const bool bUseLightLevel=false);
	virtual ~CTransTileEffect();

	virtual bool Draw(SDL_Surface* pDestSurface=NULL);

private:
	bool bFirst;
	static UINT wInstances;
};

#endif //...#ifndef TRANSTILEEFFECT_H
