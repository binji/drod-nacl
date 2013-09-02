// $Id: Clone.cpp 9898 2012-03-04 07:21:11Z mrimer $

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
 *
 * ***** END LICENSE BLOCK ***** */

#include "Clone.h"

//
//Public methods.
//

//*****************************************************************************************
CClone::CClone(
//Constructor.
//
//Params:
	CCurrentGame *pSetCurrentGame)   //(in)   If NULL (default) then
									//    class can only be used for
									//    accessing data, and not
									//    for game processing.
	: CPlayerDouble(M_CLONE, pSetCurrentGame)
{ }

//*****************************************************************************************
UINT CClone::GetIdentity() const
//Returns: what the clone looks like
{
	if (!this->pCurrentGame)
		return M_CLONE;
	const CSwordsman& player = this->pCurrentGame->swordsman;
	switch (player.wAppearance)
	{
		case M_BEETHRO: case M_NONE:
			return M_CLONE;
		case M_GUNTHRO:
			return M_GUNTHRO;
		case M_BEETHRO_IN_DISGUISE:
			return M_GUARD;
		default: return player.wAppearance;
	}
}

//*****************************************************************************
bool CClone::IsFlying() const
//Returns: whether clone is flying, based on player appearance
{
	const UINT identity = GetIdentity();
	return bIsEntityFlying(identity);
}

//*****************************************************************************
bool CClone::IsMonsterTarget() const
//Returns: whether the clone is targeted by monsters
{
	//By default, clones are targets
	if (!this->pCurrentGame)
		return true;
	const CSwordsman& player = this->pCurrentGame->swordsman;
	if (player.wAppearance == M_NONE)
		return true;

	//Otherwise, clones are only targets if the player is
	return player.IsTarget();
}

//*****************************************************************************
bool CClone::IsHiding() const
//Returns: whether the clone is visible
{
	//By default, clones aren't hiding/invisible
	if (!this->pCurrentGame)
		return false;

	//Clones are invisible if the player has drunk an Invisibility Potion
	const CSwordsman& player = this->pCurrentGame->swordsman;
	if (player.bIsInvisible)
		return true;

	//Otherwise, clones are invisible if the player can hide in water
	//and the clone is standing in shallow water
	if (IsWading() && player.GetWaterTraversalState() == WTrv_CanHide)
		return true;

	return false;
}

//*****************************************************************************************
void CClone::Process(
//Process a clone for movement.
//
//Params:
	const int /*nLastCommand*/,   //(in) Last swordsman command.
	CCueEvents &CueEvents)  //(in/out) Accepts pointer to an IDList object that will be populated
							//with codes indicating events that happened that may correspond to
							//sound or graphical effects.
{
	//Light any fuse stood on.
	if (this->pCurrentGame->swordsman.CanLightFuses())
		this->pCurrentGame->pRoom->LightFuse(CueEvents, this->wX, this->wY, true);
}

//*****************************************************************************
bool CClone::SetSwordSheathed()
//Sets and returns whether a clone's sword is sheathed.
//Clones sword state should be synched to the player's general sword state.
{
	if (!this->pCurrentGame)
		return this->bSwordSheathed;

	if (CPlayerDouble::SetSwordSheathed())
		return true;
	//If player or player's identitiy type is marked to not have a sword, then clones do not either.
	const CSwordsman& player = this->pCurrentGame->swordsman;
	if (!bEntityHasSword(GetIdentity()) || player.bSwordOff || player.bNoSword)
	{
		this->bSwordSheathed = true;
		return true;
	}
	return false;
}
