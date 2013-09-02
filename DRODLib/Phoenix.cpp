// $Id: Phoenix.cpp 9742 2011-10-22 16:12:27Z mrimer $

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

//Phoenix.cpp
//Implementation of CPhoenix.

#include "Phoenix.h"

//
//Public methods.
//

//*****************************************************************************************
CPhoenix::CPhoenix(CCurrentGame *pSetCurrentGame)
	: CMonster(M_FEGUNDO, pSetCurrentGame, AIR, SPD_FEGUNDO) //move after mimics, before stalwarts
{}

//*****************************************************************************
bool CPhoenix::CanFindSwordsman()
//Overridable method for determining if a monster can find the player on its own.
//Currently used by movement routines to see if a fegundo is controllable
//by the player or not.
//
//Returns:
//True if monster can find the swordsman, false if not.
const
{
	ASSERT(this->pCurrentGame);
	UINT wSX, wSY;
	const bool bPlayerInRoom = this->pCurrentGame->GetSwordsman(wSX, wSY);
	if (!bPlayerInRoom)
		return false; //don't check for decoys/stalwarts

	//If player is visible, monster can see him.
	if (this->pCurrentGame->swordsman.IsTarget())
	{
		if (this->pCurrentGame->swordsman.IsVisible())
			return true;
	} else {
		//NPC Beethro always visible and sensed.
		return true;
	}

	//Otherwise, monster can smell him if within range.
	if (CanSmellObjectAt(wSX, wSY))
		return true;

	//decoys/stalwarts will not give fegundo control to an invisible player

	return false;	//player not found
}

//*****************************************************************************
bool CPhoenix::DoesSquareContainObstacle(
//Override for fegundo -- they can't step on attackable monsters or the player.
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

//*****************************************************************************************
void CPhoenix::Process(
//Process a Fegundo for movement.
//
//Params:
	const int /*nLastCommand*/,   //(in) Last swordsman command.
	CCueEvents &CueEvents)  //(in/out) Accepts pointer to a cues object that will be populated
							//with codes indicating events that happened that may correspond to
							//sound or graphical effects.
{
	//Don't move if player is not in the room.
	if (!this->pCurrentGame->swordsman.IsInRoom())
		return;

	//Don't move until player activates a power/control token.
	if (!this->pCurrentGame->swordsman.bCanGetItems)
		return;

	//Always move in the direction the player faces.
	const UINT wUsingO = this->pCurrentGame->swordsman.wO;
	int dx = nGetOX(wUsingO), dy = nGetOY(wUsingO);
	UINT wX = this->wX + dx;
	UINT wY = this->wY + dy;

	//Get movement offsets.
	int dxFirst, dyFirst;
	if (!GetDirectMovement(wX, wY, dxFirst, dyFirst, dx, dy, DirectOnly))
		return;

	SetOrientation(dxFirst, dyFirst);

	//If can't move and something solid is in the way, explode.
	if (!dx && !dy && !DoesArrowPreventMovement(this->wX, this->wY, dxFirst, dyFirst) &&
			!this->pCurrentGame->pRoom->DoesSquarePreventDiagonal(this->wX, this->wY, dxFirst, dyFirst))
	{
		CMonster *pMonster = this->pCurrentGame->pRoom->GetMonsterAtSquare(
				this->wX + dxFirst, this->wY + dyFirst);
		if (!pMonster || pMonster->wType != M_FEGUNDO) //don't explode when another phoenix is in the way
			Explode(CueEvents);
		return;
	}

	MakeStandardMove(CueEvents, dx, dy);

	//Moving onto a checkpoint activates it.
	this->pCurrentGame->QueryCheckpoint(CueEvents, this->wX, this->wY);
}

//*****************************************************************************************
void CPhoenix::Explode(CCueEvents &CueEvents)
//Explodes this Fegundo.
{
	CDbRoom& room = *(this->pCurrentGame->pRoom);
	UINT wTileNo = room.GetFSquare(this->wX,this->wY);

	//The Fegundo burns to ashes and is re-animated 5 turns later.
	//It generates a 3x3 explosion.
	CCoordSet explosion; //what tiles are affected by the explosion
	CCoordStack bombs, coords;
	for (int y = -1; y <= 1; ++y)
		for (int x = -1; x <= 1; ++x)
		{
			//Make sure Force Arrows are obeyed.
			if (bIsArrow(wTileNo) && bIsArrowObstacle(wTileNo,nGetO(x,y)))
				continue;
			this->pCurrentGame->pRoom->ExpandExplosion(CueEvents, coords, this->wX, this->wY,
					this->wX + x, this->wY + y, bombs, explosion);
		}

	//Now process the effects of the explosion.
	for (CCoordSet::const_iterator exp=explosion.begin(); exp!=explosion.end(); ++exp)
		this->pCurrentGame->pRoom->ProcessExplosionSquare(CueEvents, exp->wX, exp->wY,
			(exp->wX != this->wX) || (exp->wY != this->wY)); //explosion does not hurt self, so don't make one here

	CueEvents.Add(CID_FegundoToAsh, this); //caller must replace this with FegundoAshes monster

	//If bombs were set off, explode them now.
	if (bombs.GetSize())
		this->pCurrentGame->pRoom->BombExplode(CueEvents, bombs);

	this->pCurrentGame->pRoom->ConvertUnstableTar(CueEvents);
}
