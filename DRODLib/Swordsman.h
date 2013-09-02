// $Id: Swordsman.h 9742 2011-10-22 16:12:27Z mrimer $

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
 * Portions created by the Initial Developer are Copyright (C) 1995, 1996, 
 * 1997, 2000, 2001, 2002, 2005 Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//CSwordsman.h
//Declarations for CSwordsman.h.
//
//GENERAL
//
//Class for accessing and manipulating player game state.

#ifndef CSWORDSMAN_H
#define CSWORDSMAN_H

#include "GameConstants.h"
#include "Monster.h"
#include "PlayerStats.h"

#include <BackEndLib/Coord.h>

//How Shallow Water traversal is set for the player
enum WaterTraversal
{
	WTrv_AsPlayerRole=0,	//Use Player Role properties (default)
	WTrv_NoEntry=1,			//Cannot enter Shallow Water
	WTrv_CanWade=2,			//Can walk on Shallow Water
	WTrv_CanHide=3			//Can hide in Shallow Water (sheathes sword)
};

class CSwordsman : public CEntity
{
public:
	CSwordsman() : CEntity() {Clear();}

	bool CanLightFuses() const;
	bool CanStepOnMonsters() const;
	bool CanWadeInShallowWater() const;
	void Clear();
	UINT GetWaterTraversalState() const;
	bool IsInRoom() const;
	bool IsStabbable() const;
	bool IsTarget() const;
	bool IsVisible() const { return !(this->bIsInvisible || this->bIsHiding); }
	bool Move(const UINT wSetX, const UINT wSetY);
	void ResetStats();
	void RotateClockwise();
	void RotateCounterClockwise();
	void SetOrientation(const UINT wO, const bool updatePrevO=true);
	static UINT GetSwordMovement(const int nCommand, const UINT wO);

	UINT wPrevO;

	//Sword position
	bool		HasSword() const;
	UINT     wSwordX;
	UINT     wSwordY;
	UINT     wSwordMovement;
	bool		bNoSword;       //for current room only
	bool		bSwordSheathed;
	bool     bSwordOff;      //no sword indefinitely

	//Double placing
	UINT     wPlacingDoubleType;
	UINT     wDoubleCursorX;
	UINT     wDoubleCursorY;

	//Misc. effects
	bool     bIsDying;   //whether player was killed
	bool     bIsInvisible; //Whether player is invisible due to potion quaffing
	bool     bIsHiding; //whether player is hiding in the scenery
	bool     bIsHasted;   //whether player's speed is doubled
	bool     bFrozen; //prevent moves if frozen
	UINT     wAppearance, wIdentity; //visual appearance, actual logical role enumeration
	bool     bIsTarget;    //whether monsters target non-Beethro role
	bool     bCanGetItems; //whether non-Beethro role can pick up items
	UINT     wWaterTraversal; //can override player role's natural (in)ability to wade or hide in Shallow Water

	PlayerStats st;

private:
	void SetSwordCoords();
};

#endif   //...#ifndef CSWORDSMAN_H
