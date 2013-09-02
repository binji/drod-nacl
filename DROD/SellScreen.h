// $Id: SellScreen.h 9779 2011-11-20 01:37:22Z mrimer $

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
 * Portions created by the Initial Developer are Copyright (C)
 * 2003, 2005 Caravel Software. All Rights Reserved.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef SELLSCREEN_H
#define SELLSCREEN_H

#include "DrodScreen.h"

class CSellScreen : public CDrodScreen
{
protected:
	friend class CDrodScreenManager;
	CSellScreen();

	virtual void  OnBetweenEvents();
	virtual void  OnClick(const UINT dwTagNo);
	virtual void  OnDeactivate();
	virtual bool  OnQuit();
	virtual bool  SetForActivate();

public:
	 virtual void  Paint(bool bUpdateRect=true);

private:
	void MoveScreenshots(SDL_Surface *pDestSurface, const bool bUpdateRect);
	void SelectNextSellText();

	int nextSShotToDisplay;
	Uint32 dwNextSShotMove;

	int sellTextIndex;
};

#endif //...#ifndef SELLSCREEN_H
