// $Id: Citizen.cpp 10002 2012-03-24 18:03:06Z mrimer $

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
 * Portions created by the Initial Developer are Copyright (C) 2001, 2002, 2005
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

#include "Citizen.h"
#include "Station.h"

const UINT STUCK_THRESHOLD = 10; //# of turns a citizen waits in a jam to get unstuck

//******************************************************************************
CCitizen::CCitizen(CCurrentGame *pSetCurrentGame)
	: CMonster(M_CITIZEN, pSetCurrentGame, GROUND_AND_SHALLOW_WATER, SPD_CITIZEN) //move after monsters, before NPCs
	, bDone(false)
	, nStationType(-1)
	, nVisitingStation(-1)
	, wTurnsStuck(0)
	, bHasSupply(false)
{
}

//*****************************************************************************
bool CCitizen::CheckForDamage(CCueEvents& CueEvents)
//Returns: whether monster was killed by room-inflicted damage
{
	//Damaged by remaining stationary on a hot tile?
	if (this->wX != this->wPrevX || this->wY != this->wPrevY)
		return false;

	if (this->pCurrentGame->pRoom->GetOSquare(this->wX, this->wY) == T_HOT)
	{
		//Damaged, even though sword hits do not affect citizens.
		CueEvents.Add(CID_MonsterBurned, this);
		return true;
	}
	return false;
}

//******************************************************************************************
bool CCitizen::DoesSquareContainObstacle(
//Override for citizen.  (Copied and revised code from CMonster::DoesSquareContainObstacle.)
//
//Params:
	const UINT wCol, const UINT wRow) //(in) Coords of square to evaluate.
const
{
	ASSERT(this->pCurrentGame);
	CDbRoom& room = *(this->pCurrentGame->pRoom);
	if (!room.IsValidColRow(wCol,wRow))
		return true;

	//Don't consider moving onto same tile as a valid move.
	if (wCol == this->wX && wRow == this->wY)
		return true;

	//No monsters can ever be stepped on.
	if (room.GetMonsterAtSquare(wCol, wRow) != NULL)
		return true;

	//Some layer objects are obstacles.  Check for these.
	if (IsTileObstacle(room.GetOSquare(wCol, wRow)))
		return true;
	if (IsTileObstacle(room.GetTSquare(wCol, wRow)))
		return true;
	if (IsTileObstacle(room.GetFSquare(wCol, wRow)))
		return true;

	//Can't step on any swords.
	if (this->swordsInRoom.Exists(wCol, wRow)) //this set is compiled at beginning of move
		return true;

	//Player can never be stepped on.
	return this->pCurrentGame->IsPlayerAt(wCol, wRow);
}

//******************************************************************************
bool CCitizen::GetGoal(UINT& wX, UINT& wY) const
//Call to query the citizen's current (x,y) destination
//
//Returns: whether citizen has a current (x,y) goal
{
	CDbRoom& room = *(this->pCurrentGame->pRoom);
	if (this->bHasSupply && !room.building.empty())
	{
		const bool bGoalIsCurrent = room.building.get(this->goal.wX, this->goal.wY) != 0;
		if (!bGoalIsCurrent)
			return false;

		//Going to build at this square.
		wX = this->goal.wX;
		wY = this->goal.wY;
		return true;
	}

	if (this->bDone)
		return false; //nowhere (else) to go

	if (this->nVisitingStation < 0)
		return false; //no station selected
	if (static_cast<UINT>(this->nVisitingStation) >= this->visitingSequence.size())
		return false; //route ended

	//Going to this station.
	CStation *pStation = room.stations[this->visitingSequence[this->nVisitingStation]];
	wX = pStation->X();
	wY = pStation->Y();
	return true;
}

//******************************************************************************
void CCitizen::Process(
//Process a citizen for movement.
//
//Params:
	const int /*nLastCommand*/,   //(in) Last swordsman command.
	CCueEvents &CueEvents)  //(in/out) Accepts pointer to a cues object that will be populated
		//with codes indicating events that happened that may correspond to
		//sound or graphical effects.
{
	//Init.
	CDbRoom& room = *(this->pCurrentGame->pRoom);
	vector<CStation*>& stations = room.stations;
	if (stations.empty())
		this->bDone = true;

	if (this->bHasSupply && !room.building.empty())
	{
		//Citizen is carrying supplies and there is a build request.
		//Find path to closest build marker.
		this->bDone = false;

		//If a valid goal is one tile away, then the citizen may build there now.
		//Otherwise, confirm a path to the goal is (still) accessible.
		ASSERT(room.IsValidColRow(this->goal.wX, this->goal.wY));
		const bool bGoalIsCurrent = room.building.get(this->goal.wX, this->goal.wY) != 0;
		if (!bGoalIsCurrent ||
				nDist(this->wX, this->wY, this->goal.wX, this->goal.wY) != 1)
		{
			room.GetSwordCoords(this->swordsInRoom); //speed optimization
			if (!bGoalIsCurrent || !ConfirmPath())
			{
				//If it's not, search for a (new) path to the goal.
				this->pathToDest.Clear();

				ASSERT(!room.building.tileSet.empty());
				if (!room.building.tileSet.has(this->wX, this->wY))
				{
					//Must find a path to a goal.
					if (!FindOptimalPathTo(this->wX, this->wY, room.building.tileSet, true))
						this->bHasSupply = false; //No path -- dump supply and go to next station
				} else {
					//A goal is located on this tile.
					this->goal.wX = this->wX;
					this->goal.wY = this->wY;

					//Builder would normally try to build on his own tile, getting stuck
					//as a result.  Therefore, take a step off it to build on it next turn.
					//First, try to step backwards, then forwards.
					//If neither work, then dump supplies and resume station route.
					int dxFirst, dyFirst, dx, dy;
					GetBeelineMovementSmart(this->wX - nGetOX(this->wO), this->wY - nGetOY(this->wO),
							dxFirst, dyFirst, dx, dy, true);
					if (!dx && !dy)
						GetBeelineMovementSmart(this->wX + nGetOX(this->wO), this->wY + nGetOY(this->wO),
								dxFirst, dyFirst, dx, dy, true);
					if (dx || dy)
					{
						SetOrientation(dx, dy);
						MakeStandardMove(CueEvents, dx, dy);
						return;
					}

					this->bHasSupply = false;
				}
			}

			//Take a step toward the closest goal.
			if (this->pathToDest.GetSize())
			{
				UINT wNextX, wNextY;
				this->pathToDest.Top(wNextX, wNextY);
				ASSERT(this->pCurrentGame->swordsman.wX != wNextX ||
						this->pCurrentGame->swordsman.wY != wNextY); //never step on player

				const int dx = wNextX - this->wX;
				const int dy = wNextY - this->wY;
				Move(wNextX, wNextY, &CueEvents);
				this->pathToDest.Pop();  //citizen has now made this move
				SetOrientation(dx, dy);
				return;
			}
		}

		if (this->bHasSupply)
		{
			//Citizen has reached build marker.
			ASSERT(room.IsValidColRow(this->goal.wX, this->goal.wY));
			const int dx = this->goal.wX - this->wX;
			const int dy = this->goal.wY - this->wY;
			if (dx || dy)
				SetOrientation(dx, dy); //face build spot

			//Check for vacancy.
			UINT wBuildTile = room.building.get(this->goal.wX, this->goal.wY) - 1; //1-based
			if (bIsWall(wBuildTile) || bIsCrumblyWall(wBuildTile))
			{
				//If the build tile would fill a square, the tile must be vacant now.
				CMonster *pMonster = room.GetMonsterAtSquare(this->goal.wX, this->goal.wY);
				if (pMonster || (this->pCurrentGame->swordsman.wX == this->goal.wX &&
						this->pCurrentGame->swordsman.wY == this->goal.wY))
				{
					//Build tile is occupied -- wait until vacant.
					return;
				}
			}

			const UINT wOldOTile = room.GetOSquare(this->goal.wX, this->goal.wY);
			const UINT wOldTTile = room.GetTSquare(this->goal.wX, this->goal.wY);
			const UINT wLayer = TILE_LAYER[wBuildTile];
			switch (wLayer)
			{
				case 0:
					//Update room's trapdoor stats when a trapdoor is removed this special way.
					if (!bIsTrapdoor(wBuildTile) && bIsTrapdoor(wOldOTile))
						room.DecTrapdoor(CueEvents);
					else if (bIsTrapdoor(wBuildTile) && !bIsTrapdoor(wOldOTile))
						room.IncTrapdoor(CueEvents);

					//Update platform component.
					if (bIsPlatform(wOldOTile))
					{
						ASSERT(wBuildTile != wOldOTile);
						CPlatform *pPlatform = room.GetPlatformAt(this->goal.wX, this->goal.wY);
						ASSERT(pPlatform);
						pPlatform->RemoveTile(this->goal.wX, this->goal.wY);
					}
					//Update orb data if a pressure plate or yellow door is altered.
					else if (wOldOTile == T_PRESSPLATE)
					{
						room.RemovePressurePlateTile(this->goal.wX, this->goal.wY);
					}
					else if (bIsYellowDoor(wOldOTile))
					{
						room.RemoveYellowDoorTile(this->goal.wX, this->goal.wY, wOldOTile);
					}
				break;
				case 1:
					if (bIsTar(wOldTTile))
						room.StabTar(this->goal.wX, this->goal.wY, CueEvents, true, NO_ORIENTATION);
				break;
			}

			//When water is plotted, redraw water's edge.
			//WARNING: Where plots are needed is front-end implementation dependent.
			if (bIsWater(wBuildTile) || bIsWater(wOldOTile) ||
					bIsSteppingStone(wBuildTile) || bIsSteppingStone(wOldOTile))
			{
				CCoordSet plots;
				for (int nJ=-1; nJ<=1; ++nJ)
					for (int nI=-1; nI<=1; ++nI)
						if ((nI || nJ) && room.IsValidColRow(this->goal.wX+nI, this->goal.wY+nJ))
							plots.insert(this->goal.wX+nI, this->goal.wY+nJ);
				room.Plot(plots, true);
			}

			//When pit is added/removed, redraw this tile's pit edge.
			if (bIsPit(wBuildTile) || bIsPit(wOldOTile))
			{
				CCoordSet plots;
				UINT wY = this->goal.wY + 1;
				//Down.
				while (wY < room.wRoomRows &&
						bIsPit(room.GetOSquare(this->goal.wX, wY)))
				{
					plots.insert(this->goal.wX, wY);
					++wY;
				}

				//Left.
				UINT wX = this->goal.wX - 1;
				wY = this->goal.wY;
				while (wX < room.wRoomCols && bIsPit(room.GetOSquare(wX, wY)))
				{
					plots.insert(wX, wY);
					--wX;
				}
				//Right.
				wX = this->goal.wX + 1;
				while (wX < room.wRoomCols && bIsPit(room.GetOSquare(wX, wY)))
				{
					plots.insert(wX, wY);
					++wX;
				}

				//Down a row, to left and right.
				if (++wY < room.wRoomRows)
				{
					wX = this->goal.wX - 1;
					while (wX < room.wRoomCols && bIsPit(room.GetOSquare(wX, wY)))
					{
						plots.insert(wX, wY);
						--wX;
					}
					wX = this->goal.wX + 1;
					while (wX < room.wRoomCols && bIsPit(room.GetOSquare(wX, wY)))
					{
						plots.insert(wX, wY);
						++wX;
					}
				}

				room.Plot(plots, true);
			}

			//Build there.
			room.Plot(this->goal.wX, this->goal.wY, wBuildTile, NULL, true);

			//When placing a hole, things might fall.
			if (bIsPit(wBuildTile) || bIsWater(wBuildTile))
			{
				room.CheckForFallingAt(this->goal.wX, this->goal.wY, CueEvents);
				room.ConvertUnstableTar(CueEvents);
			}
			CueEvents.Add(CID_ObjectBuilt, new CAttachableWrapper<UINT>(wBuildTile), true);

			//If shallow water is built, a stepping stone may have been created.
			//Update wBuildTile if this is the case
			if (bIsShallowWater(wBuildTile) && bIsSteppingStone(room.GetOSquare(this->goal.wX, this->goal.wY)))
				wBuildTile = T_STEP_STONE;

			//When o-layer changes, refresh bridge supports after plotting new tile.
			if (wLayer == 0)
				room.bridges.built(this->goal.wX, this->goal.wY, wBuildTile);

			room.building.remove(this->goal.wX, this->goal.wY);
			this->bHasSupply = false;
			return;
		}
		//else: citizen has no valid path to a build marker -- go to next station
	}

	if (this->bDone)
		return; //nowhere (else) to go

	//Mark moving obstacles correctly on station path maps for all citizens.
	const UINT wNumStations = stations.size();
	if (stations[0]->UpdateTurn(this->pCurrentGame->wTurnNo))
	{
		for (UINT n=1; n<wNumStations; ++n)
			stations[n]->UpdateTurn(this->pCurrentGame->wTurnNo);
	}

	//If route is completed, try to find a new station to go to.
	if (this->nVisitingStation >= (int)this->visitingSequence.size())
		this->nVisitingStation = -1;

	//If ready, choose a station to move to.
	if (this->nVisitingStation == -1)
	{
		//Find closest unvisited station.
		UINT wDist, wBestDistance = (UINT)-1, wBestIndex = 0;

		UINT n=wNumStations;
		while (n--)
		{
			if (this->nStationType >= 0 && (UINT)this->nStationType != stations[n]->GetType())
				continue; //not visiting stations of this set
			if (sequenceOfStation(n) >= 0)
				continue; //station is already in route -- doesn't need to be added again
			wDist = stations[n]->GetDistanceFrom(this->wX, this->wY);
			if (!wDist) continue;
			if (wDist < wBestDistance || (wDist == wBestDistance &&
					l2DistanceToStationSq(n) < l2DistanceToStationSq(wBestIndex)))
			{
				wBestDistance = wDist;
				wBestIndex = n;
			}
		}

		if (wBestDistance == (UINT)-1)
		{
			//No new station was accessible.
			if (this->visitingSequence.empty())
				return; //search for one next turn
			this->nVisitingStation = 0; //Begin route again from the beginning.
		} else {
			//Extend route -- go visit new station.
			ASSERT(wBestIndex < wNumStations);
			this->nVisitingStation = this->visitingSequence.size();
			this->visitingSequence.push_back(wBestIndex);

			//Only ever visit stations of the set of the first station visited.
			if (!this->nVisitingStation)
				this->nStationType = stations[wBestIndex]->GetType();
		}
	}

	ASSERT(this->nVisitingStation >= 0);
	ASSERT((UINT)this->nVisitingStation < this->visitingSequence.size());
	CStation *pStation = stations[this->visitingSequence[this->nVisitingStation]];

	//Has this station been reached?
	const UINT wStationX = pStation->X(), wStationY = pStation->Y();
	const bool bStationExists = this->pCurrentGame->pRoom->GetTSquare(
			wStationX, wStationY) == T_STATION;
	const UINT wDist = bStationExists ? nDist(this->wX,this->wY,wStationX,wStationY) : 0;
	if (wDist <= 1)
	{
		//Station has been reached.
		if (wDist == 1)
			SetOrientation(wStationX-this->wX, wStationY-this->wY);
		this->bHasSupply = bStationExists;
		this->wTurnsStuck = 0;

		//Go to next station in sequence.
		++this->nVisitingStation;
		if (wNumStations == 1)
			this->bDone = true; //reached the only destination -- nowhere else to go
		return; //don't do anything else this turn
	}

	//Take a step toward station to be visited.
	const UINT wDir = pStation->GetDirectionFrom(this->wX, this->wY);
	if (wDir != NO_ORIENTATION)
	{
		const int dx = nGetOX(wDir), dy = nGetOY(wDir);
		MakeStandardMove(CueEvents,dx,dy);
		SetOrientation(dx, dy);
		this->wTurnsStuck = 0;
	} else {
		//Stuck -- face toward goal.
		SetOrientation(sgn(wStationX - this->wX), sgn(wStationY - this->wY));

		//Traffic jam inhibitor:
		//If this citizen has been stuck for min(X,ST) turns,
		//where X is the distance to the station and ST is STUCK_THRESHOLD,
		//then mark this station as visited and choose the next one.
		//This occurs except when "persistent citizen movement" is set.
		UINT wDistToStation = pStation->GetDistanceFrom(this->wX, this->wY);
		if (wDistToStation > STUCK_THRESHOLD)
			wDistToStation = STUCK_THRESHOLD;
		if (++this->wTurnsStuck > wDistToStation &&
				!this->pCurrentGame->pRoom->bPersistentCitizenMovement)
		{
			++this->nVisitingStation;
			this->wTurnsStuck = STUCK_THRESHOLD/2; //wait less if still stuck
		}
	}
}

//******************************************************************************
float CCitizen::l2DistanceToStationSq(const UINT wStationIndex) const
//Returns: Euclidean distance to station, squared
{
	const vector<CStation*>& stations = this->pCurrentGame->pRoom->stations;
	ASSERT(wStationIndex < stations.size());
	CStation& station = *(stations[wStationIndex]);
	const int dx = this->wX - station.X();
	const int dy = this->wY - station.Y();
	return static_cast<float>(dx * dx + dy * dy);
}

//******************************************************************************
int CCitizen::sequenceOfStation(const UINT wStationIndex) const
//Returns: sequence index of specified station, or -1 if not in sequence.
{
	int nIndex=0;
	for (vector<int>::const_iterator index = this->visitingSequence.begin();
			index != this->visitingSequence.end(); ++index, ++nIndex)
		if (*index == (int)wStationIndex)
			return nIndex;

	//Station not found in station sequence.
	return -1;
}
