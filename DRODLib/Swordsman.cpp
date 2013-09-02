// $Id: Swordsman.cpp 9802 2012-01-14 14:09:41Z TFMurphy $

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

//Swordsman.cpp.
//Implementation of CSwordsman,

#include "Swordsman.h"
#include "MonsterFactory.h"
#include <BackEndLib/Assert.h>

//
// Public methods
//

//*****************************************************************************
bool CSwordsman::CanLightFuses() const
//Returns: whether the player is able to light fuses
{
	return bIsSmitemaster(this->wAppearance) || this->bCanGetItems;
}

//*****************************************************************************
bool CSwordsman::CanStepOnMonsters() const
//Returns: true if player in current role can step on (and kill) other monsters
{
	//Human roles can't step on monsters.
	//Exception: Slayer
	switch (this->wAppearance)
	{
		case M_NEGOTIATOR: case M_INSTRUCTOR:
		case M_CITIZEN:
		case M_CITIZEN1: case M_CITIZEN2:
		case M_CITIZEN3: case M_CITIZEN4:
		case M_MUDCOORDINATOR: case M_TARTECHNICIAN:
		case M_WUBBA:
		case M_HALPH: case M_HALPH2:
		case M_CLONE: case M_DECOY: case M_MIMIC:
		case M_GUARD: case M_STALWART: case M_STALWART2:
		case M_BEETHRO: case M_BEETHRO_IN_DISGUISE:
		case M_GUNTHRO:
		case M_NONE:
			return false;
		default:
			//All other (monster) roles can step on other monsters.
			return true;
	}
}

//*****************************************************************************
bool CSwordsman::CanWadeInShallowWater() const
//Returns: true if current player role can wade in shallow water
{
	return bIsEntitySwimming(this->wAppearance) ||
			bIsEntityFlying(this->wAppearance) ||
			this->GetWaterTraversalState() >= WTrv_CanWade;
}

//*****************************************************************************
void CSwordsman::Clear()
{
	this->wX = this->wY = this->wPrevX = this->wPrevY = this->wO = this->wPrevO =
			this->wSwordX = this->wSwordY = 0;
	this->wAppearance = this->wIdentity = M_BEETHRO;
	this->bSwordOff = false;
	this->wWaterTraversal = WTrv_AsPlayerRole;
	ResetStats();
}

//*****************************************************************************
UINT CSwordsman::GetWaterTraversalState() const
//Returns: the value of wWaterTraversal, with WTrv_AsPlayerRole converted as appropriate
{
	//Seep, Flying and swimming roles can never wade or hide
	const UINT wRole = this->wAppearance;
	if (bIsEntityFlying(wRole) || bIsEntitySwimming(wRole) || wRole == M_SEEP)
		return WTrv_NoEntry;

	UINT wWT = this->wWaterTraversal;
	if (wWT == WTrv_AsPlayerRole)
	{
		if (bCanEntityHideInShallowWater(wRole))
			wWT = WTrv_CanHide;
		else if (bCanEntityWadeInShallowWater(wRole))
			wWT = WTrv_CanWade;
		else
			wWT = WTrv_NoEntry;
	}

	if (this->bCanGetItems && wWT != WTrv_CanHide)
	{
		//Humans and Goblins gain Hiding capability from Power Tokens
		//All other land-based monsters gain Wading capability
		if (bIsHuman(wRole) || wRole == M_GOBLIN || wRole == M_GOBLINKING)
			wWT = WTrv_CanHide;
		else
			wWT = WTrv_CanWade;
	}

	return wWT;
}

//*****************************************************************************
bool CSwordsman::IsInRoom() const
//Returns: whether player is controlling a character in the game room
{
	//'none' indicates player is not controlling any character
	return this->wAppearance != M_NONE;
}

//*****************************************************************************
bool CSwordsman::IsStabbable() const
//Returns: true if player in current role is vulnerable to sword hits
{
	//Human roles can't step on monsters.
	//Exception: Slayer
	switch (this->wAppearance)
	{
		case M_WUBBA:
		case M_FEGUNDO:
		case M_NONE:
			return false;
		default:
			//All other (monster) roles are vulnerable.
			return true;
	}
}

//*****************************************************************************
bool CSwordsman::IsTarget() const
//Returns: whether player is a target to monsters
{
	return IsInRoom() && (bIsMonsterTarget(this->wAppearance) || //other types that monsters attack
			this->bIsTarget); //explicitly marked as a monster target
}

//*****************************************************************************
bool CSwordsman::Move(
//Move player to new location
//
//Returns: whether player was moved
//
//Params:
	const UINT wSetX, const UINT wSetY) //(in) position to move to
{
	const bool bMoved = !(this->wX == wSetX && this->wY == wSetY);

	//Set new swordsman coords.
	this->wPrevO = this->wO;
	this->wPrevX = this->wX;
	this->wPrevY = this->wY;
	this->wX = wSetX;
	this->wY = wSetY;

	SetSwordCoords();

	return bMoved;
}

//*****************************************************************************
void CSwordsman::ResetStats()
//Reset player stats.
{
	this->wPlacingDoubleType = 0;
	this->bIsDying = false;
	this->bIsInvisible = false;
	this->bIsHiding = false;
	this->bIsHasted = false;
	this->bFrozen = false;
	this->bNoSword = this->bSwordSheathed = this->bIsTarget = this->bCanGetItems = false;
}

//*****************************************************************************
void CSwordsman::RotateClockwise()
//Rotate player clockwise.
//Keeps track of previous orientation.
{
	this->wPrevX = this->wX;
	this->wPrevY = this->wY;
	this->wPrevO = this->wO;
	this->wO = nNextCO(this->wO);
	ASSERT(IsValidOrientation(this->wO));

	SetSwordCoords();
}

//*****************************************************************************
void CSwordsman::RotateCounterClockwise()
//Rotate player counter-clockwise.
//Keeps track of previous orientation.
{
	this->wPrevX = this->wX;
	this->wPrevY = this->wY;
	this->wPrevO = this->wO;
	this->wO = nNextCCO(this->wO);
	ASSERT(IsValidOrientation(this->wO));

	SetSwordCoords();
}

//*****************************************************************************
void CSwordsman::SetOrientation(
//Set player's orientation
//
//Returns: whether player was moved
//
//Params:
	const UINT wO, //(in) new orientation
	const bool updatePrevO)
{
	ASSERT(IsValidOrientation(wO));
	if (updatePrevO)
		this->wPrevO = wO;
	this->wO = wO;

	SetSwordCoords();
}

//*****************************************************************************
bool CSwordsman::HasSword() const
{
	return bEntityHasSword(this->wAppearance) &&
			!(this->bNoSword || this->bSwordSheathed || this->bSwordOff || this->bIsHiding);
}

//*****************************************************************************
UINT CSwordsman::GetSwordMovement(
//Returns: movement sword made to
//
//Params:
	const int nCommand, const UINT wO)     //(in)   Game command.
{
	switch (nCommand)
	{
		case CMD_C: //sword moved orthogonal to direction it's now facing
			switch (wO)
			{
				case NW: return NE;
				case N: return E;
				case NE: return SE;
				case W: return N;
				case E: return S;
				case SW: return NW;
				case S: return W;
				case SE: return SW;
			}
			break;
		case CMD_CC:
			switch (wO)
			{
				case NW: return SW;
				case N: return W;
				case NE: return NW;
				case W: return S;
				case E: return N;
				case SW: return SE;
				case S: return E;
				case SE: return NE;
			}
			break;
		case CMD_NW: return NW;
		case CMD_N: return N;
		case CMD_NE: return NE;
		case CMD_W: return W;
		case CMD_E: return E;
		case CMD_SW: return SW;
		case CMD_S: return S;
		case CMD_SE: return SE;
	}
	return NO_ORIENTATION;
}

//
// Private methods
//

//*****************************************************************************
void CSwordsman::SetSwordCoords()
//Set sword coordinates.
{
	this->wSwordX = this->wX + nGetOX(this->wO);
	this->wSwordY = this->wY + nGetOY(this->wO);
}
