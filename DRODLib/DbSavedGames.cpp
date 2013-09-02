// $Id: DbSavedGames.cpp 10073 2012-04-04 03:04:29Z TFMurphy $

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
 * 1997, 2000, 2001, 2002 Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 * JP Burford (jpburford)
 *
 * ***** END LICENSE BLOCK ***** */

//DbSavedGames.cpp
//Implementation of CDbSavedGame and CDbSavedGames.

#include "DbSavedGames.h"

#include "CurrentGame.h"
#include "Db.h"
#include "DbProps.h"
#include "DbXML.h"
#include "MonsterFactory.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Base64.h>
#include <BackEndLib/Exception.h>
#include <BackEndLib/Ports.h>

//
//CDbSavedGame public methods.
//

//*******************************************************************************
CDbSavedGame::~CDbSavedGame()
//Destructor.
{
	Clear();
}

//*****************************************************************************
CDbRoom* CDbSavedGame::GetRoom()
//Gets room associated with a saved game.
//
//Returns:
//Pointer to loaded room which caller must delete, or NULL if no matching room
//was found.
const
{
	CDbRoom *pRoom = new CDbRoom();
	if (pRoom)
	{
		if (!pRoom->Load(this->dwRoomID))
		{
			delete pRoom;
			pRoom=NULL;
		}
	}
	return pRoom;
}

//*******************************************************************************
bool CDbSavedGame::Load(
//Loads a saved game from database into this object.
//
//Params:
	const UINT dwLoadSavedGameID,   //(in) ID of saved game to load.
	const bool bQuick) //(in) [default=false] load only quick data members
//
//Returns:
//True if successful, false if not.
{
	try {
	
	Clear();

	//Find record with matching saved game ID.
	ASSERT(IsOpen());
	c4_View SavedGamesView;
	const UINT dwSavedGameI = LookupRowByPrimaryKey(dwLoadSavedGameID,
			V_SavedGames, SavedGamesView);
	if (dwSavedGameI == ROW_NO_MATCH) throw CException("CDbSavedGame::Load");
	c4_RowRef row = SavedGamesView[dwSavedGameI];

	//Load in props from SavedGames record.
	this->dwSavedGameID = (UINT) (p_SavedGameID(row));
	ASSERT(this->dwSavedGameID == dwLoadSavedGameID);
	this->dwPlayerID = (UINT) (p_PlayerID(row));
	this->dwRoomID = (UINT) (p_RoomID(row));
	this->eType = (SAVETYPE) (int) p_Type(row);
	this->wCheckpointX = (UINT) p_CheckpointX(row);
	this->wCheckpointY = (UINT) p_CheckpointY(row);
	this->bIsHidden = ( p_IsHidden(row)!=0 );

	if (!this->dwRoomID && this->eType != ST_DemoUpload && //Placeholder record.
			this->eType != ST_PlayerTotal && this->eType != ST_PlayerTotalMerge)
		throw CException("CDbSavedGame::Load");

	this->bPartialLoad = bQuick;
	if (!bQuick)
	{
		UINT dwRoomI, dwRoomCount;
		c4_View ConqueredRoomsView;
		c4_View ExploredRoomsView;
		c4_View CompletedScriptsView;
		c4_View GlobalScriptsView;
		UINT dwScriptI, dwScriptCount;

		this->wStartRoomX = (UINT) p_StartRoomX(row);
		this->wStartRoomY = (UINT) p_StartRoomY(row);
		this->wStartRoomO = (UINT) p_StartRoomO(row);
		this->wStartRoomAppearance = (UINT) p_StartRoomAppearance(row);
		this->bStartRoomSwordOff = p_StartRoomSwordOff(row) != 0;
		this->wStartRoomWaterTraversal = (UINT) p_StartRoomWaterTraversal(row);
		this->Created = (time_t) p_Created(row);
		this->LastUpdated = (time_t) p_LastUpdated(row);
		this->Commands = p_Commands(row);

		this->dwLevelDeaths = (UINT) p_LevelDeaths(row);
		this->dwLevelKills = (UINT) p_LevelKills(row);
		this->dwLevelMoves = (UINT) p_LevelMoves(row);
		this->dwLevelTime = (UINT) p_LevelTime(row);
		this->stats = p_Stats(row);
		this->wVersionNo = p_Version(row);

		//Populate conquered room list.
		ConqueredRoomsView = p_ConqueredRooms(row);
		dwRoomCount = ConqueredRoomsView.GetSize();
		for (dwRoomI = 0; dwRoomI < dwRoomCount; ++dwRoomI)
			this->ConqueredRooms += (UINT) p_RoomID(ConqueredRoomsView[dwRoomI]);

		//Populate explored room list.
		ExploredRoomsView = p_ExploredRooms(row);
		dwRoomCount = ExploredRoomsView.GetSize();
		for (dwRoomI = 0; dwRoomI < dwRoomCount; ++dwRoomI)
			this->ExploredRooms += (UINT) p_RoomID(ExploredRoomsView[dwRoomI]);

		//Populate completed scripts list.
		CompletedScriptsView = p_CompletedScripts(row);
		dwScriptCount = CompletedScriptsView.GetSize();
		for (dwScriptI = 0; dwScriptI < dwScriptCount; ++dwScriptI)
			this->CompletedScripts += (UINT) p_ScriptID(CompletedScriptsView[dwScriptI]);

		//Populate global scripts list.
		GlobalScriptsView = p_GlobalScripts(row);
		dwScriptCount = GlobalScriptsView.GetSize();
		for (dwScriptI = 0; dwScriptI < dwScriptCount; ++dwScriptI)
			this->GlobalScripts += (UINT) p_CharID(GlobalScriptsView[dwScriptI]);
	}

	}
	catch (CException&)
	{
		Clear();
		return false;
	}
	return true;
}

//*****************************************************************************
MESSAGE_ID CDbSavedGame::SetProperty(
//Used during XML data import.
//According to pType, convert string to proper datatype and member
//
//Returns: whether operation was successful
//
//Params:
	const PROPTYPE pType,   //(in) property (data member) to set
	char* const str,        //(in) string representation of value
	CImportInfo &info,      //(in/out) Import data
	bool &bSaveRecord)      //(out) whether record should be saved
{
	static PrimaryKeyMap::iterator localID;
	switch (pType)
	{
		case P_SavedGameID:
			this->dwSavedGameID = static_cast<UINT>(atol(str));
			if (!this->dwSavedGameID)
				return MID_FileCorrupted;  //corrupt data

			//Look up local ID.
			localID = info.SavedGameIDMap.find(this->dwSavedGameID);
			if (localID != info.SavedGameIDMap.end())
				//Error - this saved game should not have been imported yet
				return MID_FileCorrupted;

			if (bSaveRecord)
			{
				//Add a new record to the DB.
				const UINT dwOldLocalID = this->dwSavedGameID;
				this->dwSavedGameID = 0;
				Update();
				info.SavedGameIDMap[dwOldLocalID] = this->dwSavedGameID;
			} else {
				//This saved game is being ignored.
				//(It belongs to a non-existant hold/level/room.)
				info.SavedGameIDMap[this->dwSavedGameID] = 0;   //skip records with refs to this saved game ID
			}
			break;
		case P_PlayerID:
			//This ID field is the first received.
			this->dwPlayerID = static_cast<UINT>(atol(str));
			if (!this->dwPlayerID)
				return MID_FileCorrupted;  //corrupt data (saved game must have player)

			//Look up local ID.
			localID = info.PlayerIDMap.find(this->dwPlayerID);
			if (localID == info.PlayerIDMap.end())
				return MID_FileCorrupted;  //record should exist now
			this->dwPlayerID = localID->second;

			//When importing saved games only, and not during a hold upgrade,
			//they become owned by the current player.
			//The above id mapping is still performed as a perfunctory integrity check.
			if (info.typeBeingImported == CImportInfo::SavedGame &&
					!info.bImportingSavedGames) //don't transfer ownership during hold upgrade
			{
				this->dwPlayerID = g_pTheDB->GetPlayerID();
				if (!this->dwPlayerID)
				{
					//There is no current player to merge this saved game into.
					//Quit import.
					bSaveRecord = false;
					return MID_SavesIgnored;
				}
			}
			break;
		case P_RoomID:
			this->dwRoomID = static_cast<UINT>(atol(str));

			//Look up local ID.
			localID = info.RoomIDMap.find(this->dwRoomID);
			//If room ID doesn't exist, then the saved game probably exists for a removed room.
			this->dwRoomID = localID != info.RoomIDMap.end() ? localID->second : 0;
			break;
		case P_Type:
			this->eType = static_cast<SAVETYPE>(atol(str));
			if (this->eType == ST_PlayerTotal || this->eType == ST_Progress
					|| this->eType == ST_ProgressEndHold)
			{
				//A zero room ID is valid for these saved game types.
				this->eType = ST_PlayerTotalMerge;
			}
			else if (!this->dwRoomID)
			{
				//For other types, a zero ID indicates records for this room are
				//being ignored -- don't save this game.
				bSaveRecord = false;
			}
			if (info.typeBeingImported == CImportInfo::SavedGame || info.typeBeingImported == CImportInfo::Player)
			{
				//When importing a record claiming the hold is conquered,
				//only keep it if the room it's in has a hold exit.
				if (this->eType == ST_EndHold)
				{
					CDbRoom *pRoom = g_pTheDB->Rooms.GetByID(this->dwRoomID);
					if (!pRoom)
						bSaveRecord = false;	//invalid save
					else
					{
						if (!pRoom->CanEndHoldHere())
							bSaveRecord = false;
						delete pRoom;
					}
				}

				//When importing only saved games, certain types of saved game records
				//get ignored.
				if (info.typeBeingImported == CImportInfo::SavedGame)
				{
					switch (this->eType)
					{
						case ST_Unknown:
						case ST_Continue:
							bSaveRecord = false;
						break;
						default: break;	//all other types get conditionally saved
					}
				}
			}
			break;
		case P_CheckpointX:
			this->wCheckpointX = static_cast<UINT>(atoi(str));
			break;
		case P_CheckpointY:
			this->wCheckpointY = static_cast<UINT>(atoi(str));
			break;
		case P_IsHidden:
			this->bIsHidden = (atoi(str) != 0);
			break;
		case P_LastUpdated:
			this->LastUpdated = (time_t)atol(str);
			break;
		case P_StartRoomX:
			this->wStartRoomX = static_cast<UINT>(atoi(str));
			break;
		case P_StartRoomY:
			this->wStartRoomY = static_cast<UINT>(atoi(str));
			break;
		case P_StartRoomO:
			this->wStartRoomO = static_cast<UINT>(atoi(str));
			break;
		case P_StartRoomAppearance:
			this->wStartRoomAppearance = static_cast<UINT>(atoi(str));
			break;
		case P_StartRoomSwordOff:
			this->bStartRoomSwordOff = (atoi(str) != 0);
			break;
		case P_StartRoomWaterTraversal:
			this->wStartRoomWaterTraversal = static_cast<UINT>(atoi(str));
			break;
		case P_CompletedScripts:
		{
			//Parse list of script IDs.
			char *token = strtok((char*)str, " ");
			while (token)
			{
				//IDs don't need to be converted.
				const UINT dwScriptID = static_cast<UINT>(atol(token));
				if (!dwScriptID)
					return MID_FileCorrupted;  //corrupt file
				this->CompletedScripts += dwScriptID;
				token = strtok(NULL, " ");
			}
		}
		break;
		case P_GlobalScripts:
		{
			//Parse list of char IDs.
			char *token = strtok((char*)str, " ");
			while (token)
			{
				//IDs don't need to be converted.
				const UINT dwCharID = static_cast<UINT>(atol(token));
				if (!dwCharID)
					return MID_FileCorrupted;  //corrupt file
				this->GlobalScripts += dwCharID;
				token = strtok(NULL, " ");
			}
		}
		break;
		case P_ConqueredRooms:
		{
			//Parse list of room IDs.
			char *token = strtok((char*)str, " ");
			while (token)
			{
				//IDs will be matched to local ones later on completion of import.
				const UINT dwRoomID = static_cast<UINT>(atol(token));
				if (dwRoomID)
					this->ConqueredRooms += dwRoomID;
				//else skip corrupted ID
				token = strtok(NULL, " ");
			}
		}
		break;
		case P_ExploredRooms:
		{
			//Parse list of room IDs.
			char *token = strtok((char*)str, " ");
			while (token)
			{
				//IDs will be matched to local ones later on completion of import.
				const UINT dwRoomID = static_cast<UINT>(atol(token));
				if (dwRoomID)
					this->ExploredRooms += dwRoomID;
				//else skip corrupted ID
				token = strtok(NULL, " ");
			}
		}
		break;
		case P_Created:
			this->Created = (time_t)atol(str);
			break;
		case P_Commands:
		{
			BYTE *data;
			Base64::decode(str,data);
			this->Commands = (const BYTE*)data;
			delete[] data;
			break;
		}
		case P_LevelDeaths:
			this->dwLevelDeaths = static_cast<UINT>(atol(str));
			break;
		case P_LevelKills:
			this->dwLevelKills = static_cast<UINT>(atol(str));
			break;
		case P_LevelMoves:
			this->dwLevelMoves = static_cast<UINT>(atol(str));
			break;
		case P_LevelTime:
			this->dwLevelTime = static_cast<UINT>(atol(str));
			break;
		case P_Stats:
		{
			BYTE *data;
			Base64::decode(str,data);
			this->stats.UseOldFormat(info.wVersion < 201);
			this->stats = (const BYTE*)data;
			delete[] data;
			break;
		}
		case P_Version:
			this->wVersionNo = static_cast<UINT>(atol(str));
			break;
		default:
			return MID_FileCorrupted;
	}

	return MID_ImportSuccessful;
}

//*****************************************************************************
bool CDbSavedGame::Update()
//Updates database with saved game.
{
	g_pTheDB->SavedGames.ResetMembership();

	if (this->dwSavedGameID == 0)
		//Insert a new saved game.
		return UpdateNew();

	//Update existing saved game.
	return UpdateExisting();
}

//
//CDbSavedGame protected methods.
//

//*******************************************************************************
void CDbSavedGame::Clear(
//Frees resources associated with object and zeroes members.
//
//Params:
	const bool bNewGame)  //(in)   whether new game is starting [default=true]
{
	this->dwRoomID=this->dwSavedGameID=0L;
	this->bIsHidden=false;
	this->wStartRoomX=this->wStartRoomY=this->wStartRoomO=
			this->wCheckpointX=this->wCheckpointY=0;
	this->wStartRoomAppearance = M_BEETHRO; //use default value
	this->bStartRoomSwordOff = false;
	this->wStartRoomWaterTraversal = WTrv_AsPlayerRole;
	this->eType = ST_Unknown;

	this->dwPlayerID = 0L;

	if (bNewGame)
	{
		this->ConqueredRooms.clear();
		this->ExploredRooms.clear();
		this->CompletedScripts.clear();
		this->GlobalScripts.clear();
		this->stats.Clear();
		this->dwLevelDeaths = this->dwLevelKills = this->dwLevelMoves = this->dwLevelTime = 0L;
	}
	this->Commands.Clear();

	this->wVersionNo = 0;
}

//
//CDbSavedGame private methods.
//

//*****************************************************************************
bool CDbSavedGame::UpdateNew()
//Add new SavedGames record to database.
{
	LOGCONTEXT("CDbSavedGame::UpdateNew");
	ASSERT(this->dwSavedGameID == 0);
	ASSERT(IsOpen());
	
	this->dwSavedGameID = GetIncrementedID(p_SavedGameID);

	CIDSet::const_iterator iter;
	UINT wCount = 0;

	//Get explored rooms subview ready.
	c4_View ExploredRoomsView;
	ExploredRoomsView.SetSize(ExploredRooms.size()); //speed optimization
	for (iter = ExploredRooms.begin(); iter != ExploredRooms.end(); ++iter)
	{
		ASSERT(*iter);
		p_RoomID(ExploredRoomsView[wCount++]) = *iter;
	}

	//Get conquered rooms subview ready.
	c4_View ConqueredRoomsView;
	ConqueredRoomsView.SetSize(ConqueredRooms.size());
	wCount = 0;
	for (iter = ConqueredRooms.begin(); iter != ConqueredRooms.end(); ++iter)
	{
		ASSERT(*iter);
		p_RoomID(ConqueredRoomsView[wCount++]) = *iter;
	}

	//Get completed scripts subview ready.
	c4_View CompletedScriptsView;
	CompletedScriptsView.SetSize(CompletedScripts.size());
	wCount = 0;
	for (iter = CompletedScripts.begin(); iter != CompletedScripts.end(); ++iter)
	{
		ASSERT(*iter);
		p_ScriptID(CompletedScriptsView[wCount++]) = *iter;
	}

	//Get global scripts subview ready.
	c4_View GlobalScriptsView;
	GlobalScriptsView.SetSize(GlobalScripts.size());
	wCount = 0;
	for (iter = GlobalScripts.begin(); iter != GlobalScripts.end(); ++iter)
	{
		ASSERT(*iter);
		p_CharID(GlobalScriptsView[wCount++]) = *iter;
	}

	//Get stats into a buffer that can be written to db.
	UINT dwStatsSize;
	BYTE *pbytStatsBytes = this->stats.GetPackedBuffer(dwStatsSize);
	if (!pbytStatsBytes) return false;
	c4_Bytes StatsBytes(pbytStatsBytes, dwStatsSize);

	this->Created.SetToNow();
	this->LastUpdated.SetToNow();
	UINT dwCommandsSize;
	BYTE *pbytCommands = this->Commands.GetPackedBuffer(dwCommandsSize);
	c4_Bytes CommandsBytes(pbytCommands, dwCommandsSize);

	//Write SavedGames record.
	c4_RowRef row = g_pTheDB->SavedGames.GetNewRow();
	p_SavedGameID(row) = this->dwSavedGameID;
	p_PlayerID(row) = this->dwPlayerID;
	p_RoomID(row) = this->dwRoomID;
	p_Type(row) = this->eType;
	p_CheckpointX(row) = this->wCheckpointX;
	p_CheckpointY(row) = this->wCheckpointY;
	p_IsHidden(row) = this->bIsHidden;
	p_LastUpdated(row) = UINT(this->LastUpdated);
	p_StartRoomX(row) = this->wStartRoomX;
	p_StartRoomY(row) = this->wStartRoomY;
	p_StartRoomO(row) = this->wStartRoomO;
	p_StartRoomAppearance(row) = this->wStartRoomAppearance;
	p_StartRoomSwordOff(row) = this->bStartRoomSwordOff;
	p_StartRoomWaterTraversal(row) = this->wStartRoomWaterTraversal;
	p_ExploredRooms(row) = ExploredRoomsView;
	p_ConqueredRooms(row) = ConqueredRoomsView;
	p_CompletedScripts(row) = CompletedScriptsView;
	p_GlobalScripts(row) = GlobalScriptsView;
	p_Created(row) = UINT(this->Created);
	p_Commands(row) = CommandsBytes;
	p_Stats(row) = StatsBytes;
	p_LevelDeaths(row) = this->dwLevelDeaths;
	p_LevelKills(row) = this->dwLevelKills;
	p_LevelMoves(row) = this->dwLevelMoves;
	p_LevelTime(row) = this->dwLevelTime;
	p_Version(row) = this->wVersionNo;

	delete[] pbytCommands;
	delete[] pbytStatsBytes;

	CDb::addSavedGameToRoom(this->dwSavedGameID, this->dwRoomID);
	return true;
}

//*******************************************************************************
bool CDbSavedGame::UpdateExisting()
//Update an existing SavedGames record in database.
{
	LOGCONTEXT("CDbSavedGame::UpdateExisting");
	ASSERT(this->dwSavedGameID != 0);
	ASSERT(IsOpen());

	//Lookup SavedGames record.
	c4_View SavedGamesView;
	const UINT dwSavedGameI = LookupRowByPrimaryKey(this->dwSavedGameID,
			V_SavedGames, SavedGamesView);
	if (dwSavedGameI == ROW_NO_MATCH)
	{
		ASSERT(!"Bad SavedGameID.");
		return false;
	}

	CIDSet::const_iterator iter;
	UINT wCount = 0;

	//Get explored rooms subview ready.
	c4_View ExploredRoomsView;
	ExploredRoomsView.SetSize(ExploredRooms.size()); //speed optimization
	for (iter = ExploredRooms.begin(); iter != ExploredRooms.end(); ++iter)
	{
		ASSERT(*iter);
		p_RoomID(ExploredRoomsView[wCount++]) = *iter;
	}

	//Get conquered rooms subview ready.
	c4_View ConqueredRoomsView;
	ConqueredRoomsView.SetSize(ConqueredRooms.size());
	wCount = 0;
	for (iter = ConqueredRooms.begin(); iter != ConqueredRooms.end(); ++iter)
	{
		ASSERT(*iter);
		p_RoomID(ConqueredRoomsView[wCount++]) = *iter;
	}

	//Get completed scripts subview ready.
	c4_View CompletedScriptsView;
	CompletedScriptsView.SetSize(CompletedScripts.size());
	wCount = 0;
	for (iter = CompletedScripts.begin(); iter != CompletedScripts.end(); ++iter)
	{
		ASSERT(*iter);
		p_ScriptID(CompletedScriptsView[wCount++]) = *iter;
	}

	//Get global scripts subview ready.
	c4_View GlobalScriptsView;
	GlobalScriptsView.SetSize(GlobalScripts.size());
	wCount = 0;
	for (iter = GlobalScripts.begin(); iter != GlobalScripts.end(); ++iter)
	{
		ASSERT(*iter);
		p_CharID(GlobalScriptsView[wCount++]) = *iter;
	}

	//Get stats into a buffer that can be written to db.
	UINT dwStatsSize;
	BYTE *pbytStatsBytes = this->stats.GetPackedBuffer(dwStatsSize);
	if (!pbytStatsBytes) return false;
	c4_Bytes StatsBytes(pbytStatsBytes, dwStatsSize);

	//Update SavedGames record.
	if (!CDb::FreezingTimeStamps())
		this->LastUpdated.SetToNow();
	UINT dwCommandsSize;
	BYTE *pbytCommands = this->Commands.GetPackedBuffer(dwCommandsSize);
	c4_Bytes CommandsBytes(pbytCommands, dwCommandsSize);

	c4_RowRef row = SavedGamesView[dwSavedGameI];
	CDb::moveSavedGame(this->dwSavedGameID, UINT(p_RoomID(row)), this->dwRoomID);

	p_SavedGameID(row) = this->dwSavedGameID;
	p_PlayerID(row) = this->dwPlayerID;
	p_RoomID(row) = this->dwRoomID;
	p_Type(row) = this->eType;
	p_CheckpointX(row) = this->wCheckpointX;
	p_CheckpointY(row) = this->wCheckpointY;
	p_IsHidden(row) = this->bIsHidden;
	p_LastUpdated(row) = UINT(this->LastUpdated);
	p_StartRoomX(row) = this->wStartRoomX;
	p_StartRoomY(row) = this->wStartRoomY;
	p_StartRoomO(row) = this->wStartRoomO;
	p_StartRoomAppearance(row) = this->wStartRoomAppearance;
	p_StartRoomSwordOff(row) = this->bStartRoomSwordOff;
	p_StartRoomWaterTraversal(row) = this->wStartRoomWaterTraversal;
	p_ExploredRooms(row) = ExploredRoomsView;
	p_ConqueredRooms(row) = ConqueredRoomsView;
	p_CompletedScripts(row) = CompletedScriptsView;
	p_GlobalScripts(row) = GlobalScriptsView;
	p_Created(row) = UINT(this->Created);
	p_Commands(row) = CommandsBytes;
	p_Stats(row) = StatsBytes;
	p_LevelDeaths(row) = this->dwLevelDeaths;
	p_LevelKills(row) = this->dwLevelKills;
	p_LevelMoves(row) = this->dwLevelMoves;
	p_LevelTime(row) = this->dwLevelTime;
	p_Version(row) = this->wVersionNo;

	delete[] pbytCommands;
	delete[] pbytStatsBytes;

	return true;
}

//*****************************************************************************
bool CDbSavedGame::SetMembers(
//For copy constructor and assignment operator.
//
//Params:
	const CDbSavedGame &Src)
{
	//primitive types
	this->dwSavedGameID = Src.dwSavedGameID;
	this->dwRoomID = Src.dwRoomID;
	this->dwPlayerID = Src.dwPlayerID;
	this->bIsHidden = Src.bIsHidden;
	this->eType = Src.eType;
	this->wCheckpointX = Src.wCheckpointX;
	this->wCheckpointY = Src.wCheckpointY;
	this->wStartRoomX = Src.wStartRoomX;
	this->wStartRoomY = Src.wStartRoomY;
	this->wStartRoomO = Src.wStartRoomO;
	this->wStartRoomAppearance = Src.wStartRoomAppearance;
	this->bStartRoomSwordOff = Src.bStartRoomSwordOff;
	this->wStartRoomWaterTraversal = Src.wStartRoomWaterTraversal;

	//object members
	this->ExploredRooms = Src.ExploredRooms;
	this->ConqueredRooms = Src.ConqueredRooms;
	this->CompletedScripts = Src.CompletedScripts;
	this->GlobalScripts = Src.GlobalScripts;
	this->Created = Src.Created;
	this->LastUpdated = Src.LastUpdated;
	this->Commands = Src.Commands;

	//Overall game "scores"
	this->dwLevelDeaths = Src.dwLevelDeaths;
	this->dwLevelKills = Src.dwLevelKills;
	this->dwLevelMoves = Src.dwLevelMoves;
	this->dwLevelTime = Src.dwLevelTime;
	this->stats = Src.stats;
	this->wVersionNo = Src.wVersionNo;

	return true;
}

//
//CDbSavedGames public methods.
//

//*******************************************************************************
void CDbSavedGames::AddRoomsToPlayerTally(
//Adds these rooms to the player's room tally.
	const UINT dwPlayerID, const CIDSet& ConqueredRooms, const CIDSet& ExploredRooms)
{
	const UINT dwID = FindByType(ST_PlayerTotal, dwPlayerID);
	CDbSavedGame *pPlayerProgress = GetByID(dwID);
	if (!pPlayerProgress)
	{
		//Make new save record to store player's room tallies.
		pPlayerProgress = GetNew();
		pPlayerProgress->dwPlayerID = dwPlayerID;
		pPlayerProgress->eType = ST_PlayerTotal;
		pPlayerProgress->bIsHidden = true;
	}

	const UINT wConqueredSize = pPlayerProgress->ConqueredRooms.size(),
		wExploredSize = pPlayerProgress->ExploredRooms.size();
	pPlayerProgress->ConqueredRooms += ConqueredRooms;
	pPlayerProgress->ExploredRooms += ExploredRooms;

	//Ensure no zero entry is retained.
	pPlayerProgress->ConqueredRooms -= 0;
	pPlayerProgress->ExploredRooms -= 0;

	if (pPlayerProgress->ConqueredRooms.size() > wConqueredSize ||
		 pPlayerProgress->ExploredRooms.size() > wExploredSize)
		pPlayerProgress->Update();

	delete pPlayerProgress;		
}

//*****************************************************************************
void CDbSavedGames::MergePlayerTotals(
	UINT dwPlayerID
)
//After a player import, merges imported PlayerTotalMerge saves into
//a single PlayerTotal save.
{
	ASSERT(dwPlayerID);
	const UINT dwExistingSave = FindByType(ST_PlayerTotal, dwPlayerID);
	CDbSavedGame *pPlayerProgress = GetByID(dwExistingSave);
	while (UINT dwMergeSave = FindByType(ST_PlayerTotalMerge, dwPlayerID))
	{
		CDbSavedGame *pMergeSave = GetByID(dwMergeSave);
		ASSERT(pMergeSave);
		if (!pMergeSave) return;
		if (!pPlayerProgress)
		{
			//No existing PlayerTotal save - convert the first imported
			//PlayerTotalMerge save to the PlayerTotal save.
			pPlayerProgress = pMergeSave;
			pPlayerProgress->eType = ST_PlayerTotal;
			pPlayerProgress->bIsHidden = true;
			pPlayerProgress->Update(); // avoid finding again in this loop
		}
		else
		{
			pPlayerProgress->ConqueredRooms += pMergeSave->ConqueredRooms;
			pPlayerProgress->ExploredRooms += pMergeSave->ConqueredRooms;
			delete pMergeSave;
			Delete(dwMergeSave);
		}
	}

	if (pPlayerProgress)
	{
		//Ensure no zero entry is saved.
		pPlayerProgress->ConqueredRooms -= 0;
		pPlayerProgress->ExploredRooms -= 0;
		pPlayerProgress->Update();
		delete pPlayerProgress;
	}
}

//*******************************************************************************
bool CDbSavedGames::CleanupPlayerTallies()
//Removes rooms which do not exist from all "PlayerTotal" saved games.
//These might be left over from a 3.0.0 bug, or left by a deleted hold.
{
	bool any_changed = false;
	CDb db;
	CIDSet allRooms;
	db.Rooms.GetIDs(allRooms);

	const UINT dwSavedGamesCount = GetViewSize();
	for (UINT dwSavedGamesI=0; dwSavedGamesI<dwSavedGamesCount; ++dwSavedGamesI)
	{
		c4_RowRef row = GetRowRef(V_SavedGames, dwSavedGamesI);
		if (SAVETYPE(int(p_Type(row))) != ST_PlayerTotal)
			continue;

		CDbSavedGame *pPlayerProgress = GetByID(UINT(p_SavedGameID(row)));
		ASSERT(pPlayerProgress);
		if (!pPlayerProgress) continue;
		bool changed = false;

		CIDSet::iterator iter = pPlayerProgress->ExploredRooms.begin();
		while (iter != pPlayerProgress->ExploredRooms.end())
		{
			if (!allRooms.has(*iter))
			{
				changed = true;
				iter = pPlayerProgress->ExploredRooms.erase(iter);
			}
			else
				++iter;
		}

		iter = pPlayerProgress->ConqueredRooms.begin();
		while (iter != pPlayerProgress->ConqueredRooms.end())
		{
			if (!db.Rooms.Exists(*iter))
			{
				changed = true;
				iter = pPlayerProgress->ConqueredRooms.erase(iter);
			}
			else
				++iter;
		}

		if (changed)
		{
			pPlayerProgress->Update();
			any_changed = true;
		}
		delete pPlayerProgress;
	}
	return any_changed;
}
		
//*******************************************************************************
void CDbSavedGames::Delete(
//Deletes a saved games record.
//NOTE: Don't call this directly to delete saved games belonging to demos.
//Instead call CDbDemos::Delete() to delete the demo and associated saved game.
//
//Params:
	const UINT dwSavedGameID) //(in)   Saved game to delete.
{
	ASSERT(dwSavedGameID);

	c4_View SavedGamesView;
	const UINT dwSavedGameRowI = LookupRowByPrimaryKey(dwSavedGameID,
			V_SavedGames, SavedGamesView);
	if (dwSavedGameRowI==ROW_NO_MATCH) {ASSERT(!"Bad dwSavedGameID."); return;}

	CDb::deleteSavedGame(dwSavedGameID); //call first
	SavedGamesView.RemoveAt(dwSavedGameRowI);

	//After object is deleted, membership might change, so reset the flag.
	this->bIsMembershipLoaded = false;
}

//*******************************************************************************
void CDbSavedGames::DeleteForRoom(
//Deletes all saved games records (and demos) for the specified room.
//
//Params:
	const UINT dwRoomID)   //(in)  Room to delete data from.
{
	//Delete demos and their saved games...
	CDb db;
	CIDSet ids = CDb::getDemosInRoom(dwRoomID);
	CIDSet::const_iterator iter;
	for (iter = ids.begin(); iter != ids.end(); ++iter)
		db.Demos.Delete(*iter);

	//...and then saved games w/o demos.
	ids = CDb::getSavedGamesInRoom(dwRoomID);
	for (iter = ids.begin(); iter != ids.end(); ++iter)
		db.SavedGames.Delete(*iter);
}

//*******************************************************************************
void CDbSavedGames::VerifyForRoom(
//Verifies all saved games records (and demos) for the specified room.
//Truncates unplayable commands.
//
//Params:
		const UINT dwRoomID)   //(in)  Room's saved games to delete.
{
	//Test demos and their saved games...
	CDb db;
	CIDList DemoStats;
	CIDSet ids = CDb::getDemosInRoom(dwRoomID);
	CIDSet::const_iterator iter;
	for (iter = ids.begin(); iter != ids.end(); ++iter)
	{
		CDbDemo *pDemo = db.Demos.GetByID(*iter);
		ASSERT(pDemo);
		if (!pDemo->Test(DemoStats))
			db.Demos.Delete(*iter);
		else
		{
			//Mark whether these demos result in victory or death.
			const bool bDeath = GetDemoStatBool(DemoStats, DS_DidPlayerDie) ||
					GetDemoStatBool(DemoStats, DS_DidHalphDie);
			const bool bVictory = GetDemoStatBool(DemoStats, DS_WasRoomConquered);
			if (pDemo->IsFlagSet(CDbDemo::Victory) != bVictory ||
					pDemo->IsFlagSet(CDbDemo::Death) != bDeath)
			{
				pDemo->SetFlag(CDbDemo::Victory, bVictory);
				pDemo->SetFlag(CDbDemo::Death, bDeath);
				pDemo->Update();
			}
		}
		delete pDemo;
	}

	//...and then saved games w/o demos.
	CCueEvents Ignored;
	CCurrentGame *pCurrentGame;
	ids = CDb::getSavedGamesInRoom(dwRoomID);
	for (iter = ids.begin(); iter != ids.end(); ++iter)
	{
		pCurrentGame = db.GetSavedCurrentGame(*iter, Ignored, true);
		ASSERT(pCurrentGame);
		if (!pCurrentGame)
		{
			//Saved game can't even be loaded -- data was corrupted?
			db.SavedGames.Delete(*iter);
		} else {
			if (!pCurrentGame->PlayAllCommands(Ignored, true))
				pCurrentGame->Update(); //truncate commands that can't be played back
			delete pCurrentGame;
		}
	}
}

//*****************************************************************************
bool CDbSavedGames::IsDuplicateRecord(RecordMap& exportInfo, CDbSavedGame *pSavedGame)
//Returns: true if record is a semantic duplicate 
{
	//What objectID is this saved game related to?
	UINT semanticID;
	switch (pSavedGame->eType)
	{
		case ST_LevelBegin: //these saved games are specific for a level
			semanticID = g_pTheDB->Rooms.GetLevelIDForRoom(pSavedGame->dwRoomID);
		break;
		case ST_Continue: //these saved games are specific for a hold
		case ST_EndHold:
			semanticID = g_pTheDB->Rooms.GetHoldIDForRoom(pSavedGame->dwRoomID);
		break;
		case ST_Progress: //these saved game records are universal, i.e. should be no more than one unique instance of each
		case ST_ProgressEndHold:
		case ST_DemoUpload:
		case ST_PlayerTotal:
			semanticID = 0;
		break;
		case ST_PlayerTotalMerge: // may have multiples
		case ST_Demo: //may have multiple demos for a room, so don't need to check for duplicates
			return false;
		default: //saved game is specific for a room
			semanticID = pSavedGame->dwRoomID;
		break;
	}

	//Find records tracked for this ID.
	RecordMap::iterator roomExportInfo = exportInfo.find(semanticID);
	if (roomExportInfo == exportInfo.end())
	{
		exportInfo[semanticID] = RecordInfoVector();
		roomExportInfo = exportInfo.find(semanticID);
		ASSERT(roomExportInfo != exportInfo.end());
	} else {
		//Don't export semantic duplicates.
		const RecordInfoVector& records = roomExportInfo->second;
		for (UINT i=0; i<records.size(); ++i)
		{
			const RecordInfo& record = records[i];

			if (record.vType != V_SavedGames) continue; //only checking for same record type
			if (record.playerID != pSavedGame->dwPlayerID) continue; //ignore saves for other players
			if (record.subType != (UINT)pSavedGame->eType) continue; //ignore different types of saved games

			switch (record.subType)
			{
				case ST_RoomBegin: //only need one of these per room (or level/hold, as semantically distinguished above)
				case ST_LevelBegin:
				case ST_Continue:
				case ST_SecretConquered: 
				case ST_EndHold:
					return true; //duplicate semantic information -- don't need to export this record
				break;
				case ST_Checkpoint: //should have one per position
					if (record.x == pSavedGame->wCheckpointX && record.y == pSavedGame->wCheckpointY)
						return true;
				break;
				case ST_DemoUpload: //deprecated -- not needed in 3.0
					return true;
				break;
				case ST_PlayerTotalMerge:
				case ST_Demo:
					ASSERT(false); //should have been handled above
				break;
				default: //other types aren't considered duplicates
				break; 
			}
		}
	}

	//Add record.
	RecordInfo info(V_SavedGames, pSavedGame->dwSavedGameID);
	info.playerID = pSavedGame->dwPlayerID;
	info.subType = UINT(pSavedGame->eType);
	info.x = pSavedGame->wCheckpointX;
	info.y = pSavedGame->wCheckpointY;
	roomExportInfo->second.push_back(info);
	return false;
}

//*****************************************************************************
void CDbSavedGames::ExportXML(
//Returns: string containing XML text describing saved game with this ID
//
//Pre-condition: dwSavedGameID is valid
//
//Params:
//NOTE: Unused param names commented out to suppress warning
	const UINT dwSavedGameID, //(in)
	CDbRefs &dbRefs,        //(in/out)
	string &str,            //(in/out)
	const bool bRef)    //(in)
{
#define STARTTAG(vType,pType) "<"; str += ViewTypeStr(vType); str += " "; str += PropTypeStr(pType); str += "='"
#define PROPTAG(pType) "' "; str += PropTypeStr(pType); str += "='"
#define ENDTAG(vType) "</"; str += ViewTypeStr(vType); str += ">\n"
#define CLOSETAG "'/>\n"
#define CLOSESTARTTAG "'>\n"
#define LONGTOSTR(val) _ltoa((val), dummy, 10)

	if (dbRefs.IsSet(V_SavedGames,dwSavedGameID))
		return;

	dbRefs.Set(V_SavedGames,dwSavedGameID);

	CDbSavedGame *pSavedGame = GetByID(dwSavedGameID);
	if (!pSavedGame)
		return; //placeholder record -- not needed

	//Don't export saved games attached to demos that aren't being exported,
	//or "dummy" records tracking which demos have been uploaded.
	if ((pSavedGame->eType == ST_Demo && dbRefs.vTypeBeingExported == V_SavedGames) ||
			pSavedGame->eType == ST_DemoUpload)
	{
		delete pSavedGame;
		return;
	}

	//Track type of saved games being exported.
	if (IsDuplicateRecord(CDbXML::exportInfo, pSavedGame))
	{
		Delete(pSavedGame->dwSavedGameID); //remove from DB to prevent confusion
		delete pSavedGame;
		return;
	}

	//Include corresponding player and room refs.
	g_pTheDB->Players.ExportXML(pSavedGame->dwPlayerID, dbRefs, str, true);
	g_pTheDB->Rooms.ExportXML(pSavedGame->dwRoomID, dbRefs, str, true);

	//First include refs for rooms in explored/conquered lists.
	CIDSet::const_iterator iter;
	for (iter = pSavedGame->ExploredRooms.begin();
			iter != pSavedGame->ExploredRooms.end(); ++iter)
		g_pTheDB->Rooms.ExportXML(*iter, dbRefs, str, true);
	for (iter = pSavedGame->ConqueredRooms.begin();
			iter != pSavedGame->ConqueredRooms.end(); ++iter)
		g_pTheDB->Rooms.ExportXML(*iter, dbRefs, str, true);

	//Prepare data.
	char dummy[32];
	UINT dwBufSize;
	BYTE *const pCommands = pSavedGame->Commands.GetPackedBuffer(dwBufSize);

	str += STARTTAG(V_SavedGames, P_PlayerID);
	str += LONGTOSTR(pSavedGame->dwPlayerID);
	str += PROPTAG(P_RoomID);
	str += LONGTOSTR(pSavedGame->dwRoomID);
	str += PROPTAG(P_Type);
	str += LONGTOSTR((long)pSavedGame->eType);
	str += PROPTAG(P_CheckpointX);
	str += LONGTOSTR(pSavedGame->wCheckpointX);
	str += PROPTAG(P_CheckpointY);
	str += LONGTOSTR(pSavedGame->wCheckpointY);
	str += PROPTAG(P_IsHidden);
	str += LONGTOSTR((long)pSavedGame->bIsHidden);
	str += PROPTAG(P_LastUpdated);
	str += LONGTOSTR(UINT((time_t)pSavedGame->LastUpdated));
	str += PROPTAG(P_StartRoomX);
	str += LONGTOSTR((long)pSavedGame->wStartRoomX);
	str += PROPTAG(P_StartRoomY);
	str += LONGTOSTR((long)pSavedGame->wStartRoomY);
	str += PROPTAG(P_StartRoomO);
	str += LONGTOSTR((long)pSavedGame->wStartRoomO);
	str += PROPTAG(P_StartRoomAppearance);
	str += LONGTOSTR((long)pSavedGame->wStartRoomAppearance);
	str += PROPTAG(P_StartRoomSwordOff);
	str += LONGTOSTR((bool)pSavedGame->bStartRoomSwordOff);
	str += PROPTAG(P_StartRoomWaterTraversal);
	str += LONGTOSTR((long)pSavedGame->wStartRoomWaterTraversal);

	//Put primary key last, so all relevant fields have been set by the
	//time Update() is called during import.
	str += PROPTAG(P_SavedGameID);
	str += LONGTOSTR(pSavedGame->dwSavedGameID);
	//Process room lists.
	if (!pSavedGame->ExploredRooms.empty())
	{
		str += PROPTAG(P_ExploredRooms);
		for (CIDSet::const_iterator iter = pSavedGame->ExploredRooms.begin();
			iter != pSavedGame->ExploredRooms.end(); ++iter)
		{
			str += LONGTOSTR(*iter);
			str += " ";
		}
	}

	if (!pSavedGame->ConqueredRooms.empty())
	{
		str += PROPTAG(P_ConqueredRooms);
		for (CIDSet::const_iterator iter = pSavedGame->ConqueredRooms.begin();
			iter != pSavedGame->ConqueredRooms.end(); ++iter)
		{
			str += LONGTOSTR(*iter);
			str += " ";
		}
	}

	//Process completed scripts.  Refs are not needed.
	if (!pSavedGame->CompletedScripts.empty())
	{
		str += PROPTAG(P_CompletedScripts);
		for (CIDSet::const_iterator iter = pSavedGame->CompletedScripts.begin();
			iter != pSavedGame->CompletedScripts.end(); ++iter)
		{
			str += LONGTOSTR(*iter);
			str += " ";
		}
	}

	//Process global scripts
	if (!pSavedGame->GlobalScripts.empty())
	{
		str += PROPTAG(P_GlobalScripts);
		for (CIDSet::const_iterator iter = pSavedGame->GlobalScripts.begin();
			iter != pSavedGame->GlobalScripts.end(); ++iter)
		{
			str += LONGTOSTR(*iter);
			str += " ";
		}
	}

	str += PROPTAG(P_Created);
	str += LONGTOSTR(UINT((time_t)pSavedGame->Created));
	str += PROPTAG(P_Commands);
	if (dwBufSize > sizeof(BYTE))
		str += Base64::encode(pCommands, dwBufSize-sizeof(BYTE));   //strip null BYTE
	delete[] pCommands;

	if (!bRef)
	{
		str += PROPTAG(P_LevelDeaths);
		str += LONGTOSTR(pSavedGame->dwLevelDeaths);
		str += PROPTAG(P_LevelKills);
		str += LONGTOSTR(pSavedGame->dwLevelKills);
		str += PROPTAG(P_LevelMoves);
		str += LONGTOSTR(pSavedGame->dwLevelMoves);
		str += PROPTAG(P_LevelTime);
		str += LONGTOSTR(pSavedGame->dwLevelTime);
		{
			UINT dwBufSize;
			BYTE *pStats = pSavedGame->stats.GetPackedBuffer(dwBufSize);
			if (dwBufSize > 4)
			{
				str += PROPTAG(P_Stats);
				str += Base64::encode(pStats,dwBufSize-4);   //remove null UINT
			}
			delete[] pStats;
		}
	}
	str += PROPTAG(P_Version);
	str += LONGTOSTR(pSavedGame->wVersionNo);
	str += CLOSETAG;

	delete pSavedGame;

#undef STARTTAG
#undef PROPTAG
#undef ENDTAG
#undef CLOSETAG
#undef CLOSESTARTTAG
#undef LONGTOSTR
}

//*********************************************************************************
void CDbSavedGames::FilterByHold(
//Changes filter so that GetFirst() and GetNext() will return saved games 
//for a specified hold (and player, if specified).
//
//Params:
	const UINT dwSetFilterByHoldID) //(in)   Hold ID to filter by.  Set to 0
								//    for all saved games.
{
	if (this->bIsMembershipLoaded && (dwSetFilterByHoldID != this->dwFilterByHoldID || 
		this->dwFilterByLevelID != 0L || this->dwFilterByRoomID != 0L) )
	{
		//Membership is invalid.
		this->bIsMembershipLoaded = false;
	}

	this->dwFilterByLevelID = this->dwFilterByRoomID = 0L;
	this->dwFilterByHoldID = dwSetFilterByHoldID;
}

//*********************************************************************************
void CDbSavedGames::FilterByLevel(
//Changes filter so that GetFirst() and GetNext() will return saved games 
//for a specified level (and player, if specified). 
//
//Params:
	const UINT dwSetFilterByLevelID)   //(in)   Level ID to filter by.  Set to
								//    0 for all saved games.
{
	if (this->bIsMembershipLoaded && (dwSetFilterByLevelID != this->dwFilterByLevelID || 
		this->dwFilterByHoldID != 0L || this->dwFilterByRoomID != 0L) )
	{
		//Membership is invalid.
		this->bIsMembershipLoaded = false;
	}

	this->dwFilterByHoldID = this->dwFilterByRoomID = 0L;
	this->dwFilterByLevelID = dwSetFilterByLevelID;
}

//*********************************************************************************
void CDbSavedGames::FilterByPlayer(
//Changes filter so that GetFirst() and GetNext() will return saved games 
//for a specified player.  Other saved game filters stay in effect.
//
//Params:
	const UINT dwSetFilterByPlayerID)  //(in)   Player ID to filter by.
								// Set to 0 for all saved games.
								// (Other filters remain in effect.)
{
	if (this->bIsMembershipLoaded && (dwSetFilterByPlayerID !=
			this->dwFilterByPlayerID) )
	{
		//Membership is invalid.
		this->bIsMembershipLoaded = false;
	}

	//Don't zero other filter types.
	this->dwFilterByPlayerID = dwSetFilterByPlayerID;
}

//*********************************************************************************
void CDbSavedGames::FilterByRoom(
//Changes filter so that GetFirst() and GetNext() will return saved games
//for a specified room (and player, if specified).
//
//Params:
	const UINT dwSetFilterByRoomID) //(in)   Room ID to filter by.  Set to 0
								//    for all saved games.
{
	if (this->bIsMembershipLoaded && (dwSetFilterByRoomID != this->dwFilterByRoomID || 
		this->dwFilterByHoldID != 0L || this->dwFilterByLevelID != 0L) )
	{
		//Membership is invalid.
		this->bIsMembershipLoaded = false;
	}

	this->dwFilterByHoldID = this->dwFilterByLevelID = 0L;
	this->dwFilterByRoomID = dwSetFilterByRoomID;
}

//*******************************************************************************
void CDbSavedGames::FindHiddens(const bool bFlag)
//Changes filter so that hidden saved games will also be returned (or not).
//Other filters remain the same.
{
	if (bFlag != this->bLoadHidden)
	{
      //Membership is invalid.
		this->bIsMembershipLoaded = false;
		this->bLoadHidden = bFlag;
	}
}

//*******************************************************************************
UINT CDbSavedGames::FindByConqueredRoom(const UINT dwQueryRoomID)
//Finds ID of saved game that has indicated room conquered
//for the current player and hold.
//Pre-code: room must be in current hold
//
//Returns:
//SavedGameID of a found saved game, or 0 if none were found.
{
	if (!dwQueryRoomID) return 0;

	ASSERT(IsOpen());
	const UINT dwCurrentPlayerID = g_pTheDB->GetPlayerID();
	const UINT dwCurrentHoldID = g_pTheDB->GetHoldID();
	ASSERT(dwCurrentPlayerID);
	ASSERT(dwCurrentHoldID);

	CIDSet savedGamesInHold = CDb::getSavedGamesInHold(dwCurrentHoldID);

	//Each iteration looks at one saved game record for a match.
	c4_View ConqueredRoomsView, SavedGamesView;
	for (CIDSet::const_iterator savedGame = savedGamesInHold.begin();
			savedGame != savedGamesInHold.end(); ++savedGame)
	{
		const UINT dwSavedGameI = LookupRowByPrimaryKey(*savedGame, V_SavedGames, SavedGamesView);
		c4_RowRef row = SavedGamesView[dwSavedGameI];

		if (UINT(p_PlayerID(row)) != dwCurrentPlayerID)
			continue;

		//Check for specified room in this saved game's conquered list.
		ConqueredRoomsView = p_ConqueredRooms(row);
		for (UINT dwRoomI = ConqueredRoomsView.GetSize(); dwRoomI--; )
			if (dwQueryRoomID == UINT(p_RoomID(ConqueredRoomsView[dwRoomI])))
				return UINT(p_SavedGameID(row)); //Found it.
	}

	//Didn't find one.
	return 0;
}

//*******************************************************************************
UINT CDbSavedGames::FindByContinue()
//Finds ID of saved game that was saved to continue slot
//for the current player and hold.
//
//Returns:
//SavedGameID of the found continue saved game, or 0 if none were found.
{
	ASSERT(IsOpen());
	const UINT dwCurrentPlayerID = g_pTheDB->GetPlayerID();
	ASSERT(dwCurrentPlayerID);

	CIDSet savedGamesInHold = CDb::getSavedGamesInHold(g_pTheDB->GetHoldID());

	//Each iteration looks at one saved game record for a match.
	c4_View SavedGamesView;
	for (CIDSet::const_iterator savedGame = savedGamesInHold.begin();
			savedGame != savedGamesInHold.end(); ++savedGame)
	{
		const UINT dwSavedGameI = LookupRowByPrimaryKey(*savedGame, V_SavedGames, SavedGamesView);
		ASSERT(dwSavedGameI != ROW_NO_MATCH);
		if (dwSavedGameI == ROW_NO_MATCH)
			continue; //robustness guard
		c4_RowRef row = SavedGamesView[dwSavedGameI];

		if (SAVETYPE(int(p_Type(row))) == ST_Continue)
			if (UINT(p_PlayerID(row)) == dwCurrentPlayerID)
				return UINT(p_SavedGameID(row)); //Found it.
	}

	//Didn't find one.
	return 0;
}

//*******************************************************************************
UINT CDbSavedGames::SaveNewContinue(const UINT dwPlayerID)
//Make new continue saved game slot for this player.
//
//Returns: new saved game ID
{
	CDbSavedGame *pSavedGame = g_pTheDB->SavedGames.GetNew();
	pSavedGame->dwPlayerID = dwPlayerID;
	pSavedGame->dwRoomID = 0;
	pSavedGame->eType = ST_Continue;
	pSavedGame->bIsHidden = true;
	pSavedGame->wStartRoomX = 0;
	pSavedGame->wStartRoomY = 0;
	pSavedGame->dwLevelDeaths = pSavedGame->dwLevelKills =
			pSavedGame->dwLevelMoves = pSavedGame->dwLevelTime = 0L;
	pSavedGame->wVersionNo = VERSION_NUMBER;
	pSavedGame->Update();
	const UINT dwSavedGameID = pSavedGame->dwSavedGameID;
	delete pSavedGame;

	return dwSavedGameID;
}

//*******************************************************************************
UINT CDbSavedGames::FindByContinueLatest(
//Finds the latest continue saved game ID for the given player.
//
//Returns:
//SavedGameID of the found saved game, or 0 if no match found.
//
//Params:
	const UINT dwLookupPlayerID)
{
	ASSERT(IsOpen());
	ASSERT(dwLookupPlayerID);
	const UINT dwSavedGamesCount = GetViewSize();

	//Each iteration looks at one saved game record.
	UINT dwLatestSavedGameIndex = ROW_NO_MATCH;
	UINT dwLatestTime = 0L;
	for (UINT dwSavedGamesI = 0; dwSavedGamesI < dwSavedGamesCount; ++dwSavedGamesI)
	{
		c4_RowRef row = GetRowRef(V_SavedGames, dwSavedGamesI);
		if (SAVETYPE(int(p_Type(row))) != ST_Continue)
			continue;
		if (UINT(p_PlayerID(row)) != dwLookupPlayerID)
			continue;
		if (UINT(p_RoomID(row)) == 0)
			continue;
		if (UINT(p_LastUpdated(row)) > dwLatestTime)
		{
			//This continue saved game is the most recent one found so far.
			dwLatestSavedGameIndex = dwSavedGamesI;
			dwLatestTime = (UINT) p_LastUpdated(row);
		}
	}

	//No continue slot found for player.
	if (dwLatestSavedGameIndex == ROW_NO_MATCH)
		return 0L;

	//Found player's most recent continue slot.
	const UINT dwSavedGameID = p_SavedGameID( GetRowRef(V_SavedGames, dwLatestSavedGameIndex) );
	return dwSavedGameID;
}

//*****************************************************************************
UINT CDbSavedGames::FindByEndHold(
//Finds the end hold saved game for the current player for the given hold.
//
//
//Params:
	const UINT dwQueryHoldID) //(in)  Hold to look in.
//Returns:
//ID of the found saved game, or 0 if no match found.
{
	if (!dwQueryHoldID) return 0;
	ASSERT(IsOpen());

	const UINT dwCurrentPlayerID = g_pTheDB->GetPlayerID();
	ASSERT(dwCurrentPlayerID);

	CIDSet savedGamesInHold = CDb::getSavedGamesInHold(dwQueryHoldID);

	//Each iteration looks at one saved game record for a match.
	c4_View SavedGamesView;
	for (CIDSet::const_iterator savedGame = savedGamesInHold.begin();
			savedGame != savedGamesInHold.end(); ++savedGame)
	{
		const UINT dwSavedGameI = LookupRowByPrimaryKey(*savedGame, V_SavedGames, SavedGamesView);
		ASSERT(dwSavedGameI != ROW_NO_MATCH);
		if (dwSavedGameI == ROW_NO_MATCH)
			continue; //robustness guard
		c4_RowRef row = SavedGamesView[dwSavedGameI];

		if (SAVETYPE(int(p_Type(row))) != ST_EndHold)
			continue;
		if (UINT(p_PlayerID(row)) == dwCurrentPlayerID)
			return UINT(p_SavedGameID(row)); //Found it.
	}

	//No end hold slot found for player in this hold.
	return 0;
}

//*******************************************************************************
UINT CDbSavedGames::FindByLevelBegin(
//Finds a saved game that was saved to a certain level-begin slot.
//
//Params:
	const UINT dwFindLevelID) //(in)   Level to look for.
//
//Returns:
//SavedGameID of the found saved game, or 0 if no match found.
{
	ASSERT(dwFindLevelID);
	ASSERT(IsOpen());

	const UINT dwCurrentPlayerID = g_pTheDB->GetPlayerID();
	ASSERT(dwCurrentPlayerID);

	//Get the saved game IDs for the level.
	CIDSet savedGamesInLevel = CDb::getSavedGamesInLevel(dwFindLevelID);

	//Each iteration looks at one saved game record for a match.
	c4_View SavedGamesView;
	for (CIDSet::const_iterator savedGame = savedGamesInLevel.begin();
			savedGame != savedGamesInLevel.end(); ++savedGame)
	{
		const UINT dwSavedGameI = LookupRowByPrimaryKey(*savedGame, V_SavedGames, SavedGamesView);
		ASSERT(dwSavedGameI != ROW_NO_MATCH);
		if (dwSavedGameI == ROW_NO_MATCH)
			continue; //robustness guard
		c4_RowRef row = SavedGamesView[dwSavedGameI];

		if (SAVETYPE((int) p_Type(row)) != ST_LevelBegin)
			continue;
		if (UINT(p_PlayerID(row)) == dwCurrentPlayerID)
			return UINT(p_SavedGameID(row)); //Found it.
	}
	return 0L;  //Didn't find it.
}

//*******************************************************************************
UINT CDbSavedGames::FindByLevelLatest(
//Finds the latest saved game on a specified level.
//
//Params:
	const UINT dwFindLevelID) //(in)   Level to look for.
//
//Returns:
//SavedGameID of the found saved game, or 0 if no match found.
{
	ASSERT(dwFindLevelID);
	ASSERT(IsOpen());

	const UINT dwCurrentPlayerID = g_pTheDB->GetPlayerID();
	ASSERT(dwCurrentPlayerID);

	//Ensure level exists.
	if (!g_pTheDB->Levels.Exists(dwFindLevelID))
	{
		ASSERT(!"FindByLevelLatest: Couldn't load level.");
		return 0L;
	}

	//Get IDs of saved games for the level.
	CDb db;
	db.SavedGames.FilterByLevel(dwFindLevelID);
	db.SavedGames.FilterByPlayer(dwCurrentPlayerID);
	CIDSet SavedGameIDs = db.SavedGames.GetIDs();
	if (SavedGameIDs.empty())
		return 0L; //No saved games on this level.

	//Find the saved game with latest date.  Each iteration looks at the date of one
	//saved game.
	c4_View SavedGamesView;
	UINT dwLatestSavedGameID = 0, dwLatestTime = 0, exploredRoomCount = 0, conqueredRoomCount = 0, commandCount = 0;
	for (CIDSet::const_iterator iter = SavedGameIDs.begin(); iter != SavedGameIDs.end(); ++iter)
	{
		const UINT dwSavedGameI = LookupRowByPrimaryKey(*iter, V_SavedGames, SavedGamesView);
		if (dwSavedGameI == ROW_NO_MATCH)
		{
			ASSERT(!"Corrupted DB data.");
			continue;
		}
		c4_RowRef row = SavedGamesView[dwSavedGameI];
		if (UINT(p_PlayerID(row)) != dwCurrentPlayerID)
			continue;
		const UINT lastUpdated = UINT(p_LastUpdated(row));
		if (lastUpdated < dwLatestTime)
			continue;
		SAVETYPE eSaveType = (SAVETYPE) (int) p_Type(row);
		if (eSaveType != ST_Demo && eSaveType != ST_Continue)
		{
			//Among saved games imported at the same time, break ties by:
			// most explored rooms, then most conquered rooms, then most commands
			c4_View RoomsView = p_ExploredRooms(row);
			const UINT thisExploredRoomCount = RoomsView.GetSize();
			bool priority = lastUpdated > dwLatestTime;
			if (priority || thisExploredRoomCount >= exploredRoomCount) {
				RoomsView = p_ConqueredRooms(row);
				const UINT thisConqueredRoomCount = RoomsView.GetSize();
				CDbCommands commands;
				commands = p_Commands(row);
				const UINT commandsSize = commands.GetSize();

				if (!priority) {
					if (thisExploredRoomCount > exploredRoomCount) {
						priority = true;
					} else if (thisConqueredRoomCount > conqueredRoomCount) {
						priority = true;
					} else if (thisConqueredRoomCount == conqueredRoomCount) {
						priority = commandsSize > commandCount;
					}
				}

				if (priority) {
					dwLatestSavedGameID = (UINT) p_SavedGameID(row);
					dwLatestTime = lastUpdated;
					exploredRoomCount = thisExploredRoomCount;
					conqueredRoomCount = thisConqueredRoomCount;
					commandCount = commandsSize;
				}
			}
		}
	}
	ASSERT(dwLatestSavedGameID);

	//Return the latest saved game.
	return dwLatestSavedGameID;
}

//*******************************************************************************
UINT CDbSavedGames::FindByRoomBegin(
//Finds a saved game that was saved to a certain room-begin slot.
//
//Params:
	const UINT dwFindRoomID) //(in) Room to look for.
//
//Returns:
//SavedGameID of the found saved game, or 0 if no match found.
{
	ASSERT(dwFindRoomID);
	ASSERT(IsOpen());

	const UINT dwCurrentPlayerID = g_pTheDB->GetPlayerID();
	ASSERT(dwCurrentPlayerID);

	CIDSet savedGamesInRoom = CDb::getSavedGamesInRoom(dwFindRoomID);

	//Each iteration looks at one saved game record for a match.
	c4_View SavedGamesView;
	for (CIDSet::const_iterator savedGame = savedGamesInRoom.begin();
			savedGame != savedGamesInRoom.end(); ++savedGame)
	{
		const UINT dwSavedGameI = LookupRowByPrimaryKey(*savedGame, V_SavedGames, SavedGamesView);
		c4_RowRef row = SavedGamesView[dwSavedGameI];
		ASSERT(dwSavedGameI != ROW_NO_MATCH);
		if (dwSavedGameI == ROW_NO_MATCH)
			continue; //robustness guard

		if (SAVETYPE((int) p_Type(row)) != ST_RoomBegin)
			continue;
		if (UINT(p_PlayerID(row)) == dwCurrentPlayerID)
			return UINT(p_SavedGameID(row)); //Found it.
	}
	return 0L;  //Didn't find it.
}

//*******************************************************************************
UINT CDbSavedGames::FindByRoomLatest(
//Finds the latest saved game in a specified room.
//
//Params:
	const UINT dwFindRoomID) //(in) Room to look for.
//
//Returns:
//SavedGameID of the found saved game, or 0 if no match found.
{
	ASSERT(dwFindRoomID);
	ASSERT(IsOpen());

	const UINT dwCurrentPlayerID = g_pTheDB->GetPlayerID();
	ASSERT(dwCurrentPlayerID);

	//Get IDs of saved games for the room.
	CIDSet savedGamesInRoom = CDb::getSavedGamesInRoom(dwFindRoomID);

	//Find the saved game with latest date.
	//Each iteration looks at the date of one saved game.
	c4_View SavedGamesView;
	UINT dwLatestSavedGameID = 0, dwLatestTime = 0, exploredRoomCount = 0, conqueredRoomCount = 0, commandCount = 0;
	for (CIDSet::const_iterator iter = savedGamesInRoom.begin(); iter != savedGamesInRoom.end(); ++iter)
	{
		const UINT dwSavedGameI = LookupRowByPrimaryKey(*iter, V_SavedGames, SavedGamesView);
		ASSERT(dwSavedGameI != ROW_NO_MATCH);
		c4_RowRef row = SavedGamesView[dwSavedGameI];

		if (p_IsHidden(row) != 0)
			continue;
		if (UINT(p_PlayerID(row)) != dwCurrentPlayerID)
			continue;
		const UINT lastUpdated = UINT(p_LastUpdated(row));
		if (lastUpdated < dwLatestTime)
			continue;
		const SAVETYPE eSaveType = (SAVETYPE) (int) p_Type(row);
		if (eSaveType != ST_Demo && eSaveType != ST_Continue)
		{
			//Among saved games imported at the same time, break ties by:
			// most explored rooms, then most conquered rooms, then most commands
			c4_View RoomsView = p_ExploredRooms(row);
			const UINT thisExploredRoomCount = RoomsView.GetSize();
			bool priority = lastUpdated > dwLatestTime;
			if (priority || thisExploredRoomCount >= exploredRoomCount) {
				RoomsView = p_ConqueredRooms(row);
				const UINT thisConqueredRoomCount = RoomsView.GetSize();
				CDbCommands commands;
				commands = p_Commands(row);
				const UINT commandsSize = commands.GetSize();

				if (!priority) {
					if (thisExploredRoomCount > exploredRoomCount) {
						priority = true;
					} else if (thisConqueredRoomCount > conqueredRoomCount) {
						priority = true;
					} else if (thisConqueredRoomCount == conqueredRoomCount) {
						priority = commandsSize > commandCount;
					}
				}

				if (priority) {
					dwLatestSavedGameID = (UINT) p_SavedGameID(row);
					dwLatestTime = lastUpdated;
					exploredRoomCount = thisExploredRoomCount;
					conqueredRoomCount = thisConqueredRoomCount;
					commandCount = commandsSize;
				}
			}
		}
	}
	ASSERT(dwLatestSavedGameID);

	//Return the latest saved game.
	return dwLatestSavedGameID;
}

//*******************************************************************************
UINT CDbSavedGames::FindByCheckpoint(
//Finds a saved game that was saved to a certain checkpoint slot.
//
//Params:
	const UINT dwFindRoomID,     //(in)   Room to look for.
	const UINT wCol, const UINT wRow)   //(in)   Square containing checkpoint.
//
//Returns:
//SavedGameID of the found saved game, or 0 if no match found.
{
	ASSERT(dwFindRoomID);
	ASSERT(IsOpen());

	const UINT dwCurrentPlayerID = g_pTheDB->GetPlayerID();
	ASSERT(dwCurrentPlayerID);

	CIDSet savedGamesInRoom = CDb::getSavedGamesInRoom(dwFindRoomID);

	//Each iteration looks at one saved game record for a match.
	c4_View SavedGamesView;
	for (CIDSet::const_iterator savedGame = savedGamesInRoom.begin();
			savedGame != savedGamesInRoom.end(); ++savedGame)
	{
		const UINT dwSavedGameI = LookupRowByPrimaryKey(*savedGame, V_SavedGames, SavedGamesView);
		c4_RowRef row = SavedGamesView[dwSavedGameI];

		if (SAVETYPE((int) p_Type(row)) != ST_Checkpoint)
			continue;
		if (UINT(p_PlayerID(row)) == dwCurrentPlayerID)
		{
			const UINT wCheckpointX = p_CheckpointX(row);
			const UINT wCheckpointY = p_CheckpointY(row);
			if (wCheckpointX == wCol && wCheckpointY == wRow)
				return (UINT)p_SavedGameID(row);
		}
	}
	return 0;  //Didn't find it.
}

//*****************************************************************************
UINT CDbSavedGames::FindByType(
//Finds the first saved game of the specified type in the DB.
//
//Params:
	const SAVETYPE eType, //(in)  Saved game type to look up.
	const UINT dwPlayerID,	//(in) [default=0]
	const bool bBackwardsSearch) //[default=true]
//Returns:
//ID of the found saved game, or 0 if no match found.
{
	ASSERT(IsOpen());

	const UINT dwSavedGamesCount = GetViewSize();

	if (bBackwardsSearch)
	{
		//Each iteration looks at one saved game record.
		//Search is done from back to front, as the record we're looking for, as a
		//rule of thumb, will be very recent.
		for (UINT dwSavedGamesI=dwSavedGamesCount; dwSavedGamesI--; )
		{
			c4_RowRef row = GetRowRef(V_SavedGames, dwSavedGamesI);
			if (SAVETYPE(int(p_Type(row))) != eType)
				continue;

			//Optional player ID filter.
			if (dwPlayerID && dwPlayerID != UINT(p_PlayerID(row)))
				continue;

			return UINT(p_SavedGameID(row)); //Found it.
		}
	} else {
		for (UINT dwSavedGamesI=0; dwSavedGamesI<dwSavedGamesCount; ++dwSavedGamesI)
		{
			c4_RowRef row = GetRowRef(V_SavedGames, dwSavedGamesI);
			if (SAVETYPE(int(p_Type(row))) != eType)
				continue;

			//Optional player ID filter.
			if (dwPlayerID && dwPlayerID != UINT(p_PlayerID(row)))
				continue;

			return UINT(p_SavedGameID(row)); //Found it.
		}
	}

	//No saved game of this type found.
	return 0;
}

//*****************************************************************************
UINT CDbSavedGames::GetHoldIDofSavedGame(
//Get the ID of the hold a saved game belongs to.
//
//Params:
	const UINT dwSavedGameID) //(in)
{
	return CDbRooms::GetHoldIDForRoom(GetRoomIDofSavedGame(dwSavedGameID));
}

//*****************************************************************************
UINT CDbSavedGames::GetPlayerIDofSavedGame(
//Get the ID of the player a saved game belongs to.
//
//Params:
	const UINT dwSavedGameID)   //(in)
{
	if (!dwSavedGameID)
		return 0;

	//Look up PlayerID from associated SavedGames record.
	c4_View SavedGamesView;
	const UINT dwSavedGamesRowI = LookupRowByPrimaryKey(dwSavedGameID,
			V_SavedGames, SavedGamesView);
	if (dwSavedGamesRowI == ROW_NO_MATCH) {ASSERT(!"Saved game row missing."); return 0;}
	return UINT(p_PlayerID(SavedGamesView[dwSavedGamesRowI]));
}

//*****************************************************************************
UINT CDbSavedGames::GetRoomIDofSavedGame(const UINT dwSavedGameID)
//Get the ID of the room a saved game belongs to.
{
	if (!dwSavedGameID) return 0;
	c4_View SavedGamesView;
	const UINT dwSavedGamesRowI = LookupRowByPrimaryKey(dwSavedGameID,
			V_SavedGames, SavedGamesView);
	if (dwSavedGamesRowI == ROW_NO_MATCH) {ASSERT(!"Saved game row missing."); return 0;}
	return UINT(p_RoomID(SavedGamesView[dwSavedGamesRowI]));
}

//*****************************************************************************
UINT CDbSavedGames::GetSavedGameID(
//Compares arguments against saved game records in the DB.
//
//Returns: saved game ID if a saved game record in the DB matches these parameters, else 0
	const UINT dwRoomID, const CDate& Created, const UINT dwPlayerID)
{
	ASSERT(IsOpen());

	//Get all saved games in the specified room.
	CIDSet savedGamesInRoom = CDb::getSavedGamesInRoom(dwRoomID);

	//Each iteration checks a demo's GIDs.
	c4_View SavedGamesView;
	for (CIDSet::const_iterator savedGame = savedGamesInRoom.begin();
			savedGame != savedGamesInRoom.end(); ++savedGame)
	{
		const UINT dwSavedGameI = LookupRowByPrimaryKey(*savedGame, V_SavedGames, SavedGamesView);
		c4_RowRef row = SavedGamesView[dwSavedGameI];

		const time_t DemoCreated = (time_t)p_Created(row);
		if (DemoCreated != Created)
			continue;

		//Check author.
		if (UINT(p_PlayerID(row)) == dwPlayerID)
		{
			//Signatures match.  Return this record's ID.
			return (UINT)p_SavedGameID(row);
		}
	}

	//No match.
	return 0L;
}

//*****************************************************************************
CIDSet CDbSavedGames::GetConqueredRooms(const UINT savedGameID)
//Returns: set of conquered rooms for saved game with specified ID
{
	CIDSet ids;
	c4_View SavedGamesView;
	const UINT dwSavedGameI = LookupRowByPrimaryKey(savedGameID,
			V_SavedGames, SavedGamesView);
	if (dwSavedGameI == ROW_NO_MATCH)
		return ids;

	c4_View RoomsView = p_ConqueredRooms(SavedGamesView[dwSavedGameI]);
	const UINT roomCount = RoomsView.GetSize();
	for (UINT dwRoomI = 0; dwRoomI < roomCount; ++dwRoomI)
		ids += UINT(p_RoomID(RoomsView[dwRoomI]));

	return ids;
}

//*****************************************************************************
CIDSet CDbSavedGames::GetExploredRooms(const UINT savedGameID)
//Returns: set of explored rooms for saved game with specified ID
{
	CIDSet ids;
	c4_View SavedGamesView;
	const UINT dwSavedGameI = LookupRowByPrimaryKey(savedGameID,
			V_SavedGames, SavedGamesView);
	if (dwSavedGameI == ROW_NO_MATCH)
		return ids;

	c4_View RoomsView = p_ExploredRooms(SavedGamesView[dwSavedGameI]);
	const UINT roomCount = RoomsView.GetSize();
	for (UINT dwRoomI = 0; dwRoomI < roomCount; ++dwRoomI)
		ids += UINT(p_RoomID(RoomsView[dwRoomI]));

	//Add room game was saved in to explored rooms
	ids += (UINT)(p_RoomID(SavedGamesView[dwSavedGameI]));

	return ids;
}

//*****************************************************************************
SAVETYPE CDbSavedGames::GetType(const UINT savedGameID)
//Returns: type of saved game for saved game with specified ID
{
	c4_View SavedGamesView;
	const UINT dwSavedGameI = LookupRowByPrimaryKey(savedGameID,
			V_SavedGames, SavedGamesView);
	if (dwSavedGameI == ROW_NO_MATCH)
		return ST_Unknown;

	return SAVETYPE(int(p_Type(SavedGamesView[dwSavedGameI])));
}

//*******************************************************************************
void CDbSavedGames::UpdatePlayerTallies(
//Modify all ST_PlayerTotals saves when replacing a hold to use the new room IDs.
//
//Params:
	const CImportInfo& info
)
{
	const UINT dwSavedGamesCount = GetViewSize();
	for (UINT dwSavedGamesI=0; dwSavedGamesI<dwSavedGamesCount; ++dwSavedGamesI)
	{
		c4_RowRef row = GetRowRef(V_SavedGames, dwSavedGamesI);
		if (SAVETYPE(int(p_Type(row))) != ST_PlayerTotal)
			continue;

		CDbSavedGame *pPlayerProgress = GetByID(UINT(p_SavedGameID(row)));
		ASSERT(pPlayerProgress);
		if (!pPlayerProgress) continue;
		bool changed = false;

		for (PrimaryKeyMap::const_iterator room_iter=info.RoomIDMap.begin();
		     room_iter!=info.RoomIDMap.end(); ++room_iter)
		{
			if (pPlayerProgress->ExploredRooms.has(room_iter->first))
			{
				changed = true;
				pPlayerProgress->ExploredRooms.erase(room_iter->first);
				pPlayerProgress->ExploredRooms += room_iter->second;
			}
			if (pPlayerProgress->ConqueredRooms.has(room_iter->first))
			{
				changed = true;
				pPlayerProgress->ConqueredRooms.erase(room_iter->first);
				pPlayerProgress->ConqueredRooms += room_iter->second;
			}
		}

		if (changed)
			pPlayerProgress->Update();
		delete pPlayerProgress;
	}
}

//
//CDbSavedGames private methods.
//

//*******************************************************************************
void CDbSavedGames::LoadMembership()
//Load the membership list with all saved game IDs.
//Player filtering is done internal to hold, level, and room filtering.
{
	ASSERT(IsOpen());
	this->MembershipIDs.clear();

	//Call function load membership with appropriate filtering.
	if (this->dwFilterByHoldID)
		LoadMembership_ByHold(this->dwFilterByHoldID);
	else if (this->dwFilterByLevelID)
		LoadMembership_ByLevel(this->dwFilterByLevelID);
	else if (this->dwFilterByRoomID)
		LoadMembership_ByRoom(this->dwFilterByRoomID);
	else if (this->dwFilterByPlayerID)
		LoadMembership_ByPlayer(this->dwFilterByPlayerID);
	else
		LoadMembership_All();

	this->currentRow = this->MembershipIDs.begin();
	this->bIsMembershipLoaded = true;
}

//*******************************************************************************
void CDbSavedGames::LoadMembership_All()
//Loads membership list from all saved games.
{
	const UINT dwSavedGamesCount = GetViewSize();

	//Each iteration gets a saved game ID and puts in membership list.
	for (UINT dwSavedGamesI = 0L; dwSavedGamesI < dwSavedGamesCount; ++dwSavedGamesI)
	{
		c4_RowRef row = GetRowRef(V_SavedGames, dwSavedGamesI);
		if (this->bLoadHidden || p_IsHidden(row) == 0)
			this->MembershipIDs += p_SavedGameID(row);
	}
}

//*******************************************************************************
void CDbSavedGames::LoadMembership_ByRoom(const UINT /*dwByRoomID*/)
//Loads membership list from saved games in a specified room,
//and for specified player, if any.
{
	c4_View SavedGamesView;
	CIDSet savedGamesInRoom = CDb::getSavedGamesInRoom(this->dwFilterByRoomID);

	//Each iteration processes a saved game ID and maybe puts it in membership list.
	for (CIDSet::const_iterator savedGame = savedGamesInRoom.begin();
			savedGame != savedGamesInRoom.end(); ++savedGame)
	{
		const UINT savedGameRowI = LookupRowByPrimaryKey(*savedGame, V_SavedGames, SavedGamesView);
		c4_RowRef row = SavedGamesView[savedGameRowI];

		if (this->bLoadHidden || p_IsHidden(row) == 0)
		{
			//Does player ID match filter?
			if (!this->dwFilterByPlayerID)
				this->MembershipIDs += p_SavedGameID(row);
			else
			{
				const UINT dwPlayerID = (UINT) p_PlayerID(row);
				if (dwPlayerID == this->dwFilterByPlayerID)
					this->MembershipIDs += p_SavedGameID(row);
			}
		}
	}
}

//*******************************************************************************
void CDbSavedGames::LoadMembership_ByPlayer(const UINT dwByPlayerID)
//Loads membership list from saved games for a specified player.
{
	const UINT dwSavedGamesCount = GetViewSize();

	//Each iteration gets a saved game ID and puts in membership list.
	for (UINT dwSavedGamesI = 0L; dwSavedGamesI < dwSavedGamesCount; ++dwSavedGamesI)
	{
		c4_RowRef row = GetRowRef(V_SavedGames, dwSavedGamesI);
		if (UINT(p_PlayerID(row)) == dwByPlayerID && 
				(this->bLoadHidden || p_IsHidden(row) == 0))
			this->MembershipIDs += p_SavedGameID(row);
	}
}

//*******************************************************************************
void CDbSavedGames::LoadMembership_ByLevel(const UINT dwByLevelID)
//Loads membership list from saved games in a specified level,
//and for specified player, if any.
{
	if (!g_pTheDB->Levels.Exists(dwByLevelID))
	{
		ASSERT(!"Failed to retrieve level.");
		return;
	}

	//Compile IDs of all saved games in the specified level.
	c4_View SavedGamesView;
	CIDSet savedGamesInLevel = CDb::getSavedGamesInLevel(dwByLevelID);

	//Each iteration processes a saved game ID and maybe puts it in membership list.
	for (CIDSet::const_iterator savedGame = savedGamesInLevel.begin();
			savedGame != savedGamesInLevel.end(); ++savedGame)
	{
		const UINT savedGameRowI = LookupRowByPrimaryKey(*savedGame, V_SavedGames, SavedGamesView);
		ASSERT(savedGameRowI != ROW_NO_MATCH);
		c4_RowRef row = SavedGamesView[savedGameRowI];
		if (this->bLoadHidden || p_IsHidden(row) == 0)
			if (!this->dwFilterByPlayerID || (UINT(p_PlayerID(row)) == this->dwFilterByPlayerID))
				this->MembershipIDs += p_SavedGameID(row);
	}
}

//*******************************************************************************
void CDbSavedGames::LoadMembership_ByHold(const UINT dwByHoldID)
//Loads membership list from saved games in a specified level,
//and for specified player, if any.
{
	CIDSet savedGamesInHold = CDb::getSavedGamesInHold(dwByHoldID);

	//Each iteration processes a saved game ID and maybe puts it in membership list.
	c4_View SavedGamesView;
	for (CIDSet::const_iterator savedGame = savedGamesInHold.begin();
			savedGame != savedGamesInHold.end(); ++savedGame)
	{
		const UINT savedGameRowI = LookupRowByPrimaryKey(*savedGame, V_SavedGames, SavedGamesView);
		ASSERT(savedGameRowI != ROW_NO_MATCH);
		if (savedGameRowI == ROW_NO_MATCH)
			continue; //robustness guard
		c4_RowRef row = SavedGamesView[savedGameRowI];
		if (this->bLoadHidden || p_IsHidden(row) == 0)
			if (!this->dwFilterByPlayerID || (UINT(p_PlayerID(row)) == this->dwFilterByPlayerID))
				this->MembershipIDs += p_SavedGameID(row);
	}
}
