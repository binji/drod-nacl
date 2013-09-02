// $Id: Colors.h 10030 2012-03-29 07:00:37Z mrimer $

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

#ifndef COLORS_H
#define COLORS_H

#include <SDL.h>

#if defined(__APPLE__)
	//Alpha channel comes first in SDL_Surfaces (e.g. ARGB/ABGR)
	//So when working directly with 32-bit color pixels, use bytes 2-4 instead of bytes 1-3
#	define GAME_RENDERING_OFFSET
#	define PIXEL_FUDGE_FACTOR (1)
#else
#	define PIXEL_FUDGE_FACTOR (0)
#endif

struct SURFACECOLOR 
{
	Uint8 byt1;
	Uint8 byt2; 
	Uint8 byt3;
};

SURFACECOLOR   GetSurfaceColor(const SDL_Surface *pSurface, Uint8 bytRed, 
		Uint8 bytGreen, Uint8 bytBlue);

#endif //#ifndef COLORS_H
