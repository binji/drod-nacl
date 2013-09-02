// $Id: VarMonitorEffect.h 9742 2011-10-22 16:12:27Z mrimer $

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
 * Portions created by the Initial Developer are Copyright (C) 2006
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef VARMONITOREFFECT_H
#define VARMONITOREFFECT_H

#include "DrodEffect.h"
#include "../DRODLib/CurrentGame.h"

//*****************************************************************************
class CRoomWidget;
class CVarMonitorEffect : public CEffect
{
public:
	CVarMonitorEffect(CWidget *pSetWidget);

	virtual bool Draw(SDL_Surface* pDestSurface=NULL);
	void SetText(const WCHAR* pText);

private:
	int x, y;          //pixel position
	UINT lastTurn;      //last turn var query was made

	Uint32 lastUpdate; //tick last update was displayed
	VARMAP lastVars;   //latest displayed var state

	WSTRING text;
	CRoomWidget *pRoomWidget;
};

#endif //...#ifndef VARMONITOREFFECT_H
