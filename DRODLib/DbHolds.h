// $Id: DbHolds.h 9798 2011-12-10 18:27:48Z mrimer $

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

//DbHolds.h
//Declarations for CDbHolds and CDbHold.
//Classes for accessing hold data from database.

#ifndef DBHOLDS_H
#define DBHOLDS_H

#include "DbVDInterface.h"
#include "DbLevels.h"
#include "DbSavedGames.h"
#include "Character.h"
#include "GameConstants.h"
#include "ImportInfo.h"
#include "PlayerStats.h"

#include <BackEndLib/Assert.h>

//Literals used to query and store values for Hold Characters in the packed vars object.
#define scriptIDstr "ScriptID"

//*****************************************************************************
//To facilitate DB queries.
struct HoldStats
{
	CIDSet rooms, requiredRooms, secretRooms;
};

//*****************************************************************************
//Per-hold script vars.
struct HoldVar
{
	HoldVar()
		: dwVarID(0)
	{ }
	HoldVar(const UINT dwVarID, const WCHAR* pwszName)
		: dwVarID(dwVarID)
	{
		ASSERT(pwszName);
		this->varNameText = pwszName;
	}
	UINT dwVarID;  //unique ID
	WSTRING varNameText;
};

//*****************************************************************************
//Per-hold definable NPC character.
struct HoldCharacter
{
	HoldCharacter()
		: dwCharID(0), wType(0), dwDataID_Avatar(0), dwDataID_Tiles(0)
		, dwScriptID(0), pCommands(NULL)
	{ }
	HoldCharacter(const HoldCharacter& that, const bool bReplicateData=false)
		: dwCharID(that.dwCharID), charNameText(that.charNameText), wType(that.wType)
		, dwDataID_Avatar(that.dwDataID_Avatar), dwDataID_Tiles(that.dwDataID_Tiles)
		, dwScriptID(that.dwScriptID), ExtraVars(that.ExtraVars), pCommands(NULL)
	{
		if (that.pCommands)
		{
			this->pCommands = new COMMANDPTR_VECTOR;
			const UINT numCommands = that.pCommands->size();
			for (UINT i=0; i<numCommands; ++i)
			{
				CCharacterCommand *pOldCommand = (*that.pCommands)[i];
				CCharacterCommand *pCommand = new CCharacterCommand(*pOldCommand, bReplicateData);
				this->pCommands->push_back(pCommand);
			}
		}
	}
	HoldCharacter(const UINT dwCharID, const WCHAR* pwszName,
			const UINT dwScriptID,
			const UINT wType=0, const UINT dwDataID_Avatar=0, const UINT dwDataID_Tiles=0,
			const CDbPackedVars& ExtraVars=CDbPackedVars()
	)
		: dwCharID(dwCharID), wType(wType)
		, dwDataID_Avatar(dwDataID_Avatar), dwDataID_Tiles(dwDataID_Tiles)
		, ExtraVars(ExtraVars), pCommands(NULL)
	{
		ASSERT(pwszName);
		this->charNameText = pwszName;
		this->dwScriptID = dwScriptID;
		this->ExtraVars.SetVar(scriptIDstr, dwScriptID);
	}
	~HoldCharacter()
	{
		deleteWorkingCommands();
	}

	void deleteWorkingCommands()
	{
		if (this->pCommands)
		{
			for (COMMANDPTR_VECTOR::iterator command = this->pCommands->begin();
					command != this->pCommands->end(); ++command)
				delete *command;
			delete this->pCommands;
			this->pCommands = NULL;
		}
	}

	UINT dwCharID;  //unique ID
	WSTRING charNameText;
	UINT wType;  //character's analogous monster type
	UINT dwDataID_Avatar; //avatar image ref (optional)
	UINT dwDataID_Tiles;  //tileset image ref (optional)
	UINT dwScriptID;  //unique script ID
	CDbPackedVars ExtraVars; //holds default character script
	COMMANDPTR_VECTOR *pCommands; //working copy of default character script
};

//*****************************************************************************
//A level entrance within a hold.
class CEntranceData
{
public:
	CEntranceData()
		: dwEntranceID(0), dwRoomID(0), wX(0), wY(0), wO(0), bIsMainEntrance(false)
		, bShowDescription(true)
	{ }
	CEntranceData(const UINT dwEntranceID, const UINT dwDescriptionMessageID,
			const UINT dwRoomID, const UINT wX, const UINT wY, const UINT wO,
			const bool bIsMainEntrance, const bool bShowDescription)
		: dwEntranceID(dwEntranceID), dwRoomID(dwRoomID), wX(wX), wY(wY), wO(wO)
		, bIsMainEntrance(bIsMainEntrance), bShowDescription(bShowDescription)
	{if (dwDescriptionMessageID)
		this->DescriptionText.Bind(dwDescriptionMessageID);}
	~CEntranceData()
	{ }

	CEntranceData(CEntranceData &Src) {SetMembers(Src);}
	CEntranceData& operator= (CEntranceData &Src)
	{
		SetMembers(Src);
		return *this;
	}

	WSTRING GetPositionDescription();

	void ReflectX(const UINT wWidth) {
		this->wX = (wWidth-1) - this->wX;
		this->wO = nGetO(-nGetOX(this->wO),nGetOY(this->wO));
	}
	void ReflectY(const UINT wHeight) {
			this->wY = (wHeight-1) - this->wY;
			this->wO = nGetO(nGetOX(this->wO),-nGetOY(this->wO));
	}

	void SetMembers(CEntranceData &Src, const bool bCopyLocalInfo=true);

	UINT       dwEntranceID;
	CDbMessageText DescriptionText;
	UINT       dwRoomID;
	UINT       wX, wY, wO;
	bool       bIsMainEntrance;
	bool       bShowDescription;
};

typedef vector<CEntranceData*> ENTRANCE_VECTOR;

//*****************************************************************************
class CDbHolds;
class CCurrentGame;
class CDbHold : public CDbBase
{
protected:
	friend class CDbHolds;
	friend class CDbLevel;
	friend class CCurrentGame;
	friend class CDbVDInterface<CDbHold>;
	CDbHold();

	void CopyHoldMedia(CDbHold *pNewHold, CImportInfo& info);

public:
	CDbHold(CDbHold &Src) : CDbBase() {SetMembers(Src);}
	CDbHold &operator= (const CDbHold &Src) {
		SetMembers(Src);
		return *this;
	}

	virtual ~CDbHold();

	UINT        AddCharacter(const WCHAR* pwszName);
	void        AddEntrance(CEntranceData* pEntrance, const bool bReplaceMainEntrance=true);
	UINT        AddVar(const WCHAR* pwszName);
	bool        ChangeAuthor(const UINT dwNewAuthorID);
	bool        DeleteCharacter(const UINT dwCharID);
	bool        DeleteEntrance(CEntranceData *pEntrance);
	void        DeleteEntrancesForRoom(const UINT dwRoomID);
	bool        DeleteVar(const UINT dwVarID);
	const WCHAR *  GetAuthorText() const;
	HoldCharacter* GetCharacter(const UINT dwCharID);
	UINT        GetCharacterID(const WCHAR* pwszName) const;
	CDate       GetCreated() const {return this->Created;}
	CIDSet      GetDeletedDataIDs() const;
	CEntranceData* GetMainEntranceForLevel(const UINT dwLevelID) const;
	UINT        GetMainEntranceIDForLevel(const UINT dwLevelID) const;
	CEntranceData* GetEntrance(const UINT dwEntranceID) const;
	CEntranceData* GetEntranceAt(const UINT dwRoomID, const UINT wX, const UINT wY) const;
	UINT        GetEntranceIndex(CEntranceData *pEntrance) const;
	void        getStats(RoomStats& stats) const;
	UINT        GetMainEntranceRoomIDForLevel(const UINT dwLevelID) const;
	UINT        GetNewScriptID();
	UINT        GetPrimaryKey() const {return this->dwHoldID;}
	UINT        GetScriptID() const {return this->dwScriptID;}
	CDbLevel *  GetStartingLevel() const;
	char*       getVarAccessToken(const WCHAR* pName) const;
	char*       getVarAccessToken(const char* pName) const;
	UINT        GetVarID(const WCHAR* pwszName) const;
	const WCHAR* GetVarName(const UINT dwVarID) const;
	void        InsertLevel(CDbLevel *pLevel, const UINT dwLevelSupplantedID=0L);
	static bool IsVarNameGoodSyntax(const WCHAR* pName);
	static bool IsVarCharValid(WCHAR wc);
	bool        Load(const UINT dwHoldID, const bool bQuick=false);
	CDbHold*    MakeCopy();
	void        MarkSpeechForDeletion(CDbSpeech* pSpeech);
	void        MarkDataForDeletion(const UINT dataID);
	bool        PlayerCanEdit(const UINT playerID) const;
	void        RemoveImageID(const UINT imageID);
	void        RemoveLevel(const UINT dwLevelID, const UINT dwNewEntranceID);
	bool        RenameCharacter(const UINT dwCharID, const WSTRING& newName);
	bool        RenameVar(const UINT dwVarID, const WSTRING& newName);
	bool        Repair();
	bool        SaveCopyOfLevels(CDbHold *pHold, CImportInfo& info);

	//Import handling
	virtual MESSAGE_ID SetProperty(const PROPTYPE pType, const char** atts);
	virtual MESSAGE_ID SetProperty(const PROPTYPE pType, char* const str,
			CImportInfo &info, bool &bSaveRecord);
	virtual MESSAGE_ID SetProperty(const VIEWPROPTYPE vpType, const PROPTYPE pType,
			char* const str, CImportInfo &info);

	virtual bool   Update();
	UINT VarIsSetInAnySavedGame(const char* pszName, const UINT playerID) const;
	
	CDbMessageText DescriptionText;
	CDbMessageText NameText;
	CDbMessageText EndHoldText;   //message shown player on completing hold
	CDate          LastUpdated;
	UINT          dwLevelID;  //first level
	UINT          dwHoldID;
	UINT          dwPlayerID; //author (for GUID)

	ENTRANCE_VECTOR Entrances;   //all level entrance positions in the hold
	vector<HoldVar> vars;        //all the vars used in the hold
	vector<HoldCharacter> characters; //all custom characters used in the hold

	enum EditAccess
	{
		Anyone=0,
		OnlyYou=1,
		YouAndConquerors=2,
		YouAndMasters=3
	};
	EditAccess     editingPrivileges;  //who can edit the hold

	enum HoldStatus
	{
		NoStatus=-1,
		Homemade=0,
		JtRH=1,     //Journey to Rooted Hold
		Official=2,
		Tutorial=3,
		KDD=4,      //King Dugan's Dungeon 2.0
		TCB=5,      //The City Beneath
		GatEB=6     //Gunthro and the Epic Blunder
	};
	static HoldStatus GetOfficialHoldStatus();
	HoldStatus     status;	//type of hold
	bool           bCaravelNetMedia; //whether d/led from CaravelNet

private:
	void     Clear();
	void     ClearEntrances();
	UINT     GetLocalID(const HoldStatus eStatusMatching=NoStatus) const;
	UINT     GetNewCharacterID();
	bool     LoadCharacters(c4_View &CharsView);
	bool     LoadEntrances(c4_View& EntrancesView);
	bool     LoadVars(c4_View& VarsView);
	void     SaveCharacters(c4_View &CharsView);
	void     SaveEntrances(c4_View& EntrancesView);
	void     SaveVars(c4_View& VarsView);
	bool     SetMembers(const CDbHold& Src, const bool bCopyLocalInfo=true);
	bool     UpdateExisting();
	bool     UpdateNew();

	CDate          Created;    //GUID field
	UINT          dwNewLevelIndex;  //for relative level GUIDs
	UINT          dwScriptID; //incremented ID for scripts in hold
	UINT          dwVarID;    //incremented ID for hold vars
	UINT          dwCharID;   //incremented ID for hold characters

	vector<UINT> deletedTextIDs;   //message text IDs to be deleted on Update
	vector<UINT> deletedDataIDs;   //data IDs to be deleted on Update
	vector<UINT> deletedSpeechIDs; //speech IDs to be deleted on Update
};

//******************************************************************************************
class CDb;
class CDbHolds : public CDbVDInterface<CDbHold>
{
protected:
	friend class CCurrentGame;
	friend class CDb;
	CDbHolds()
		: CDbVDInterface<CDbHold>(V_Holds, p_HoldID)
	{}

public:
	virtual void      Delete(const UINT dwHoldID);
	virtual bool      Exists(const UINT dwID) const;
	void					ExportRoomHeader(WSTRING& roomText, CDbLevel *pLevel,
			CDbRoom *pRoom, ENTRANCE_VECTOR& entrances) const;
	WSTRING           ExportSpeech(const UINT dwHoldID, const bool bCoords=true) const;
	virtual bool      ExportText(const UINT dwHoldID, CDbRefs &dbRefs, CStretchyBuffer &str);
	virtual void      ExportXML(const UINT dwHoldID, CDbRefs &dbRefs, string &str, const bool bRef=false);
	bool        EditableHoldExists() const;
	static UINT      GetAuthorID(const UINT dwHoldID);
	void              GetEntranceIDsForRoom(const UINT dwRoomID, CIDSet& entranceIDs) const;
	void              GetEntrancesForRoom(const UINT dwRoomID, ENTRANCE_VECTOR& entrances) const;
	static UINT      GetLevelIDAtIndex(const UINT dwIndex, const UINT dwHoldID);
	static UINT      GetLevelIDAtOrderIndex(const UINT dwIndex, const UINT dwHoldID);
	static UINT      GetHoldID(const CDate& Created,
			CDbMessageText& HoldNameText, CDbMessageText& origAuthorText);
	WSTRING     GetHoldName(const UINT holdID) const;
	void        GetRooms(const UINT dwHoldID, HoldStats& stats) const;
	static void GetRoomsExplored(const UINT dwHoldID, const UINT dwPlayerID,
			CIDSet& rooms, const bool bOnlyConquered=false);
	static void GetRoomsExplored(const UINT dwHoldID, const UINT dwPlayerID,
			CIDSet& exploredRooms, CIDSet& conqueredRooms);
	WSTRING     GetScriptSpeechText(const COMMAND_VECTOR& commands,
			CDbHold *pHold, CCharacter *pCharacter, WSTRING& roomText,
			CDbLevel *pLevel, CDbRoom *pRoom, ENTRANCE_VECTOR& entrancesIgnored) const;
	UINT        GetSecretsDone(HoldStats& stats, const UINT dwHoldID,
			const UINT dwPlayerID, const bool bConqueredOnly=true) const;
	static UINT GetHoldIDWithStatus(const CDbHold::HoldStatus status);
	bool        IsHoldMastered(const UINT dwHoldID, const UINT playerID) const;
	void        LogScriptVarRefs(const UINT holdID);
	bool        PlayerCanEditHold(const UINT dwHoldID) const;

	static UINT deletingHoldID; //ID of hold in process of being deleted
};

#endif //...#ifndef DBHOLDS_H
