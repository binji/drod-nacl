// $Id: FrameRateEffect.h 8019 2007-07-14 22:30:11Z trick $

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

#ifndef FRAMERATEEFFECT_H
#define FRAMERATEEFFECT_H

#include "Effect.h"

//****************************************************************************************
class CFrameRateEffect : public CEffect
{
public:
	CFrameRateEffect(CWidget *pSetWidget);
	
	virtual bool Draw(SDL_Surface* pDestSurface=NULL);

private:
	int      x, y;
	UINT dwLastDrawTime;
	UINT  wFrameCount;
	float fLastFrameCount;
};

#endif //...#ifndef FRAMERATEEFFECT_H
