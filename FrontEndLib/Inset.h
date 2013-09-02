// $Id: Inset.h 8741 2008-03-12 22:44:33Z mrimer $

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

#ifndef INSET_H
#define INSET_H

#include <BackEndLib/Types.h>

static const UINT CY_INSET_BORDER = 2;
static const UINT CX_INSET_BORDER = 2;

struct SDL_Surface;
void DrawInset(const int nX, const int nY, const UINT wW, const UINT wH,
		SDL_Surface *pPartsSurface, SDL_Surface *pDestSurface,
		const bool bDrawCenter=true, const bool bUpdateRect=false,
		const bool bDisabled=false);

#endif //...#infdef INSET_H
