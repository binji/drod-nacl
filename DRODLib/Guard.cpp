// $Id: Guard.cpp 9800 2012-01-14 01:59:13Z TFMurphy $

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

//Guard.cpp
//Implementation of CGuard.

#include "Guard.h"

//
//Public methods.
//

//*****************************************************************************************
CGuard::CGuard(
//Constructor.
//
//Params:
	CCurrentGame *pSetCurrentGame)   //(in)   If NULL (default) then
									//    class can only be used for
									//    accessing data, and not
									//    for game processing.
	: CPlayerDouble(M_GUARD, pSetCurrentGame,
					GROUND_AND_SHALLOW_WATER, SPD_GUARD) //move after slayer, before monsters
{ }

//*****************************************************************************
bool CGuard::DoesSquareContainObstacle(
//Override for guards -- they can't step on attackable monsters.
//
//Params:
	const UINT wCol, const UINT wRow) //(in) Coords of square to evaluate.  Must be valid.
const
{
	//Most of the checks done in base method.
	if (CMonster::DoesSquareContainObstacle(wCol, wRow))
		return true;

	//Can't move onto monsters -- even target ones.
	if (this->pCurrentGame->pRoom->GetMonsterAtSquare(wCol, wRow))
		return true;

	//Can't step on the player.
	return this->pCurrentGame->IsPlayerAt(wCol, wRow);
}

//*****************************************************************************
bool CGuard::IsOpenMove(const int dx, const int dy) const
{
	if (!CPlayerDouble::IsOpenMove(dx,dy))
		return false;

	UINT wSO = this->wO;
	if (!HasSword())
		wSO = nGetO(dx,dy); //will end up facing this way after moving

	return IsSafeToStab(this->wX + dx, this->wY + dy, wSO);
}

//*****************************************************************************************
void CGuard::Process(
//Process guard for movement.
//NOTE: Code copied and dumbed-down from slayer behavior.
//
//Params:
	const int /*nLastCommand*/,   //(in) Last swordsman command.
	CCueEvents &CueEvents)  //(in/out) Accepts pointer to an IDList object that will be populated
							//with codes indicating events that happened that may correspond to
							//sound or graphical effects.
{
	this->wSwordMovement = NO_ORIENTATION;

	//Don't move if another monster is already killing player.
	if (CueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerDied), CIDA_PlayerDied)) return;

	UINT wX, wY;
	int dx, dy;
	if (!GetTarget(wX,wY)) return;

	//1. Can I kill target this turn?  (Do it.)
	const UINT wMySX = GetSwordX(), wMySY = GetSwordY();
	const UINT wDistMySwordFromSwordsman = nDist(wMySX, wMySY, wX, wY);
	if (wDistMySwordFromSwordsman == 0)
		return; //my sword is already on the target -- don't do anything else
	const UINT wMyDistFromSwordsman = nDist(this->wX, this->wY, wX, wY);

	//If I think that I can kill my target during this turn, take the
	//move immediately.  I'm not smart enough to concern myself
	//with whether I have my sword sheathed or not this close to the target.
	if (wDistMySwordFromSwordsman == 1)	
	{
		//See if directly moving to square will work.  (Copied from CMonster::GetBestMove().)
		dx = wX - wMySX;
		dy = wY - wMySY;
		//Use the old IsOpenMove behavior
		const bool bFoundDir = CPlayerDouble::IsOpenMove(dx,dy) &&
				IsSafeToStab(this->wX + dx, this->wY + dy, this->wO);
		if (bFoundDir)
		{
			DoubleMove(CueEvents,dx,dy);
			this->wSwordMovement = nGetO(dx,dy);
			if (!HasSword())
				this->wO = this->wSwordMovement; //automatically face this direction
			return;
		}
		//See if rotating sword will work.
		if (wMyDistFromSwordsman == 1)
		{
			if (MakeSlowTurnIfOpenTowards(wX,wY))
				return;
		}
	}
	//If I haven't found a killing move (successful or not), and I'm next to a target
	//but without a sword, "run" into it (i.e. point towards it).
	if (!HasSword() && wMyDistFromSwordsman == 1)
	{
		dx = sgn(wX - this->wX);
		dy = sgn(wY - this->wY);
		SetOrientation(dx,dy);
		this->wSwordMovement = nGetO(dx,dy);
		return;
	}

	//2. Not close to the target -- advance using intelligent path finding.
	//Use brained motion to chase the player or a Beethro NPC,
	//but unbrained motion to chase a Stalwart.
	int dxFirst, dyFirst;
	UINT wSX, wSY;
	const bool bSwordsmanInRoom = this->pCurrentGame->GetSwordsman(wSX, wSY);
	if (!bSwordsmanInRoom || wSX != wX || wSY != wY ||
		!GetBrainDirectedMovement(dxFirst, dyFirst, dx, dy, SmartOmniDirection))
		GetBeelineMovementSmart(wX, wY, dxFirst, dyFirst, dx, dy, true);    

	if (dx || dy)
	{
		//If not facing the direction it will move, turn guard that way.
		//Else move guard (in direction it's facing) forward to new destination square.
		if (!HasSword() || !MakeSlowTurnIfOpenTowards(wX, wY))
		{
			//2b. Planning on moving forward.  Make sure I won't stab another guard.
			UINT wSO = this->wO;
			if (!HasSword())
				wSO = nGetO(dx,dy); //will end up facing this way after moving

			if (IsSafeToStab(this->wX + dx, this->wY + dy, wSO))
			{
				DoubleMove(CueEvents, dx, dy);
				this->wSwordMovement = nGetO(dx,dy);
				if (!HasSword())
					this->wO = this->wSwordMovement; //automatically face this direction
			}
		}
	} else {
		//Can't move -- just turn toward swordsman.
		MakeSlowTurnIfOpenTowards(wX,wY);
	}
}
