// $Id: ImageWidget.h 8019 2007-07-14 22:30:11Z trick $

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

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include "Widget.h"
#include <BackEndLib/Wchar.h>

//******************************************************************************
class CImageWidget : public CWidget
{
public:
	CImageWidget(const UINT dwSetTagNo, const int nSetX, const int nSetY, SDL_Surface *pSurface);
	CImageWidget(const UINT dwSetTagNo, const int nSetX, const int nSetY, const WCHAR *pwczFilename);
	~CImageWidget();

	SDL_Surface* GetImageSurface() const {return this->pImageSurface;}
	virtual void   Paint(bool bUpdateRect = true);

	void   SetAlpha(const Uint8 setAlpha);
	void   SetImage(SDL_Surface *pSurface);
	void   SetImage(const WCHAR *pwczFilename);

private:
	SDL_Surface *pImageSurface;
	WSTRING wFilename;
	Uint8 alpha;
};

#endif //#ifndef IMAGEWIDGET_H
