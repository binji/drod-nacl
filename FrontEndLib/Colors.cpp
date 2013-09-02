// $Id: Colors.cpp 10030 2012-03-29 07:00:37Z mrimer $

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
 * Portions created by the Initial Developer are Copyright (C) 2002. 2005
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

#include <SDL.h>

#include "Colors.h"
#include <BackEndLib/Assert.h>
#include <BackEndLib/Types.h>

//***************************************************************************
SURFACECOLOR GetSurfaceColor(
//Gets bytes that can be written to surface to produce a pixel of 
//specified color.
//
//Params:
	const SDL_Surface *pSurface,  //(in)   Surface to calc color for.
	Uint8 bytRed,              //(in)   Desired color.
	Uint8 bytGreen,            //
	Uint8 bytBlue)             //
//
//Returns:
//3 bytes in the structure.
{
	SURFACECOLOR Color;

	ASSERT(pSurface->format->BytesPerPixel>=3);
	
	//Get generic color value.
	Uint32 dwColor = SDL_MapRGB(pSurface->format, bytRed, bytGreen, 
			bytBlue);

	//Get surface bytes for color.
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	Color.byt1 = (dwColor >> 16) & 0xff;
	Color.byt2 = (dwColor >> 8) & 0xff;
	Color.byt3 = dwColor & 0xff;
#elif defined(GAME_RENDERING_OFFSET)
	Color.byt1 = (dwColor >> 8) & 0xff;
	Color.byt2 = (dwColor >> 16) & 0xff;
	Color.byt3 = (dwColor >> 24) & 0xff;
#else
	Color.byt1 = dwColor & 0xff;
	Color.byt2 = (dwColor >> 8) & 0xff;
	Color.byt3 = (dwColor >> 16) & 0xff;
#endif

	return Color;
}
