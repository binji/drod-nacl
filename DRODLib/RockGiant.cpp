// $Id: RockGiant.cpp 9742 2011-10-22 16:12:27Z mrimer $

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
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//RockGiant.cpp
//Implementation of CRockGiant.

#include "RockGiant.h"

//
//Public methods.
//

//*****************************************************************************************
void CRockGiant::Process(
//Process a roach for movement.
//
//Params:
	const int /*nLastCommand*/,   //(in) Last swordsman command.
	CCueEvents &CueEvents)  //(in/out) Accepts pointer to an IDList object that will be populated
							//with codes indicating events that happened that may correspond to
							//sound or graphical effects.
{
	if (this->bBroken) return; //can't move

	//Find where to move to.
	UINT wX, wY;
	if (!GetTarget(wX,wY)) return;

	//Get movement offsets.
	int dxFirst, dyFirst, dx, dy;
	if (!GetDirectMovement(wX, wY, dxFirst, dyFirst, dx, dy, DirectOnly))
		return;
 
	//Move rock giant to new destination square.
	MakeStandardMove(CueEvents,dx,dy);
	SetOrientation(dxFirst, dyFirst);
}

//*****************************************************************************
bool CRockGiant::CheckForDamage(CCueEvents& CueEvents)
//Returns: whether monster was killed by room-inflicted damage
{
	//Damaged by remaining stationary on a hot tile?
	if (this->wX != this->wPrevX || this->wY != this->wPrevY)
		return false;

	CCueEvents Ignored;
	if (this->pCurrentGame->pRoom->GetOSquare(this->wX, this->wY) == T_HOT)
		if (OnStabbed(Ignored, this->wX, this->wY))
		{
			CueEvents.Add(CID_MonsterBurned, this);
			((CCurrentGame*)this->pCurrentGame)->TallyKill(this->wType);
			return false; //don't remove the monster from the room
		}

	return false;
}

//*****************************************************************************************
bool CRockGiant::OnStabbed(
//When stabbed, rock giant is disabled but doesn't disappear.
//
//Params:
	CCueEvents &CueEvents,  //(in/out) Accepts pointer to an IDList object that will be populated
							//with codes indicating events that happened that may correspond to
							//sound or graphical effects.
	const UINT /*wX*/, const UINT /*wY*/) //(in) unused
{
	if (this->bBroken)
		return false;  //no further effect

	//The monster becomes inactive, but should not be removed from the room.
	CueEvents.Add(CID_MonsterDiedFromStab, this);

	//When killed over shallow water, rock giants become a stepping stone
	//Delay dealing with it until ProcessSwordHit
	if (!(this->pCurrentGame->pRoom->GetOSquare(this->wX, this->wY) == T_SHALLOW_WATER))
	{
		this->bBroken = true;
		this->wO = NO_ORIENTATION; //show disabled graphic
		this->wPrevX = this->wX;
		this->wPrevY = this->wY;

		this->pCurrentGame->pRoom->DecMonsterCount();

		//Since rock giant becomes an obstacle, pathmaps might need updating.
		this->pCurrentGame->pRoom->UpdatePathMapAt(this->wX, this->wY);
		this->pCurrentGame->pRoom->RecalcStationPaths();
	}

	return true;
}
