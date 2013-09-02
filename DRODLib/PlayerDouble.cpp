// $Id: PlayerDouble.cpp 9850 2012-02-18 16:20:30Z mrimer $

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

#include "PlayerDouble.h"

//
//Public methods.
//

//*****************************************************************************
CPlayerDouble::CPlayerDouble(
//Constructor.
//
//Params:
	const UINT wSetType,    //(in) type of derived monster
	CCurrentGame *pSetCurrentGame,   //(in)   If NULL (default) then
									//    class can only be used for
									//    accessing data, and not
									//    for game processing.
	const MovementType eMovement,   //(in)    [default=GROUND_AND_SHALLOW_WATER]
	const UINT wSetProcessSequence)  //(in)   [default=100]
	: CMonster(wSetType, pSetCurrentGame, eMovement, wSetProcessSequence)
	, wSwordMovement(NO_ORIENTATION)
	, bSwordSheathed(false)
	, bNoSword(false)
	, bFrozen(false)
	, bWaitedOnHotFloorLastTurn(false)
{ }

//*****************************************************************************
bool CPlayerDouble::CheckForDamage(CCueEvents& CueEvents)
//Returns: whether monster was killed by room-inflicted damage
{
	const bool bOnHotTile = this->pCurrentGame->pRoom->GetOSquare(this->wX, this->wY) == T_HOT;

	//Damaged by remaining stationary on a hot tile?
	const bool bMoved = this->wX != this->wPrevX || this->wY != this->wPrevY;
	if (bOnHotTile && !bMoved)
	{
		const UINT wIdentity = GetResolvedIdentity();
		//Flying and tarstuff identities are safe from hot tiles.
		if (bIsEntityFlying(wIdentity) || bIsMonsterTarstuff(wIdentity))
			return false;

		if (!CanHaste() || !this->pCurrentGame->swordsman.bIsHasted || this->bWaitedOnHotFloorLastTurn)
		{
			CCueEvents Ignored;
			if (OnStabbed(Ignored, this->wX, this->wY))
			{
				//Add special cue events here instead of inside OnStabbed.
				CueEvents.Add(CID_MonsterBurned, this);
				return true;
			}
		}
	}
	this->bWaitedOnHotFloorLastTurn = bOnHotTile && !bMoved;
	return false;
}

//*****************************************************************************
bool CPlayerDouble::DoesSquareContainObstacle(
//Override for doubles.
//
//Params:
	const UINT wCol, const UINT wRow) //(in) Coords of square to evaluate.  Must be valid.
const
{
	//Most of the checks done in base method.
	if (CMonster::DoesSquareContainObstacle(wCol, wRow))
		return true;

	//Can't step on the player.
	return this->pCurrentGame->IsPlayerAt(wCol, wRow);
}

//*****************************************************************************
bool CPlayerDouble::DoesSquareRemoveSword(const UINT wCol, const UINT wRow) const
//Returns: whether the tile being considered will prevent the double from having a sword
{
	ASSERT(this->pCurrentGame->pRoom);

	const UINT wOSquare = this->pCurrentGame->pRoom->GetOSquare(wCol,wRow);
	const UINT wIdentity = GetResolvedIdentity();
	if (wOSquare == T_GOO)
		return true;

	if (wOSquare == T_SHALLOW_WATER)
	{
		switch(this->wType)
		{
			case M_CLONE:
				return this->pCurrentGame->swordsman.GetWaterTraversalState() == WTrv_CanHide;
			default:
				return bCanEntityHideInShallowWater(wIdentity);
		}
	}

	return false;
}

//*****************************************************************************
bool CPlayerDouble::HasSword() const
//Returns: true when double has unsheathed sword
{
	return !(this->bNoSword || this->bSwordSheathed);
}

//*****************************************************************************
bool CPlayerDouble::IsTileObstacle(
//Override for doubles.
//
//Params:
	const UINT wLookTileNo) //(in)   Tile to evaluate.  Note each tile# will always be
						//    found on the same layer of squares.
//
//Returns:
//True if tile is an obstacle, false if not.
const
{
	return (
			//All the things a double can step onto.
			!bIsPotion(wLookTileNo) &&
			CMonster::IsTileObstacle(wLookTileNo)
			);
}

//*****************************************************************************
bool CPlayerDouble::IsSafeToStab(const UINT wFromX, const UINT wFromY, const UINT wSO) const
//Returns: true if standing at (wFromX, wFromY) with sword orientation at wSO won't stab anything
//dangerous, or a guard or Slayer.  Otherwise false.
{
	ASSERT(this->pCurrentGame->pRoom);
	CDbRoom& room = *(this->pCurrentGame->pRoom);
	ASSERT(room.IsValidColRow(wFromX,wFromY)); //we shouldn't be calling IsSafeToStab
							//when we're considering standing on a tile that isn't valid.

	//Determine where the sword would be.
	const UINT wSX = wFromX + nGetOX(wSO);
	const UINT wSY = wFromY + nGetOY(wSO);

	if (!room.IsValidColRow(wSX,wSY))
		return true; //sword would be out-of-bounds and wouldn't hit anything

 	//If sword is put away on this tile, no stabbing would occur, so this position is always safe.
	if (DoesSquareRemoveSword(wFromX,wFromY))
		return true;

	//Never stab a bomb.
	const UINT wTSquare = room.GetTSquare(wSX,wSY);
	if (wTSquare == T_BOMB)
		return false;

	//Don't stab a guard or Slayer.
	CMonster *pMonster = room.GetMonsterAtSquare(wSX, wSY);
	const UINT type = pMonster ? pMonster->GetIdentity() : M_NONE;
	if ((type == M_GUARD || type == M_SLAYER || type == M_SLAYER2) &&
			pMonster != static_cast<const CMonster*>(this)) //impossible to stab self -- ignore this case
		return false;
	if (bIsSmitemaster(type) || type == M_CLONE || bIsStalwart(type))
		return true;  //Can stab NPCs of Beethro, Gunthro, clone or stalwart type.

	return true;
}

//*****************************************************************************
bool CPlayerDouble::MakeSlowTurnIfOpen(const UINT wDesiredO)
//Make a slow turn toward the desired orientation if it's safe.
{
	const UINT wOldO = this->wO;
	if (!MakeSlowTurn(wDesiredO))
		return false;  //already facing desired orientation
	if (!IsSafeToStab(this->wX, this->wY, this->wO))
	{
		this->wO = wOldO; //don't turn
		this->wSwordMovement = NO_ORIENTATION;
		return false;
	}

	this->wSwordMovement = CSwordsman::GetSwordMovement(
			this->wO == nNextCO(wOldO) ? CMD_C : CMD_CC, this->wO);
	return true;
}

//*****************************************************************************
bool CPlayerDouble::MakeSlowTurnIfOpenTowards(const UINT wTX, const UINT wTY)
//Make a slow turn toward (wTX, wTY) if it's safe.
{
	const UINT wOldO = this->wO;
	if (!MakeSlowTurnTowards(wTX, wTY))
		return false;  //already facing desired orientation
	if (!IsSafeToStab(this->wX, this->wY, this->wO))
	{
		this->wO = wOldO; //don't turn
		this->wSwordMovement = NO_ORIENTATION;
		return false;
	}
	return true;
}

//*****************************************************************************
bool CPlayerDouble::MakeSlowTurnTowards(const UINT wTX, const UINT wTY)
//Make a 45 degree (1/8th rotation) turn toward the desired orientation.
//When facing opposite the desired direction, turn in the way that puts the
//sword closest to the target.
//
//Returns: whether a turn was made
{
	ASSERT(this->wO != NO_ORIENTATION);

	const UINT wDesiredO = GetOrientationFacingTarget(wTX, wTY);
	if (this->wO == wDesiredO) return false; //no turn needed
	ASSERT(wDesiredO != NO_ORIENTATION);

	//Determine whether turning clockwise or counter-clockwise would
	//reach the desired orientation more quickly.
	const UINT wRotDist = RotationalDistanceCO(wDesiredO);
	bool bTurnCW;
	if (wRotDist < 4)
		bTurnCW = true;
	else if (wRotDist > 4)
		bTurnCW = false;
	else {
		//Since I'm facing opposite the target, determine which way would be more
		//advantageous to turn, i.e., gets my sword closer to the target.
		bTurnCW = nDist(wTX, wTY, this->wX + nGetOX(nNextCO(this->wO)),
				this->wY + nGetOY(nNextCO(this->wO))) < nDist(wTX, wTY,
				this->wX + nGetOX(nNextCCO(this->wO)), this->wY + nGetOY(nNextCCO(this->wO)));
	}
	this->wO = bTurnCW ? nNextCO(this->wO) : nNextCCO(this->wO);
	this->wSwordMovement = CSwordsman::GetSwordMovement(bTurnCW ? CMD_C : CMD_CC, this->wO);
	return true;
}

//*****************************************************************************
bool CPlayerDouble::SetSwordSheathed()
//Sets and returns whether double's sword gets sheathed on this tile.
{
	ASSERT(this->pCurrentGame);
	return this->bSwordSheathed = DoesSquareRemoveSword(this->wX, this->wY);
}

void CPlayerDouble::DoubleMove(
	CCueEvents &CueEvents,  //(out)  Add cue events if appropriate.
	const int dx, const int dy)   //(in)   Movement offset.
{
	ASSERT(dx || dy); //there should always be movement

	MakeStandardMove(CueEvents, dx, dy);
}
