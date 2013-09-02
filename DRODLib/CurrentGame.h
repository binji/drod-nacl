// $Id: CurrentGame.h 10001 2012-03-24 17:49:39Z mrimer $

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
 * Michael Welsh Duggan (md5i), Richard Cookney (timeracer), JP Burford (jpburford),
 * John Wm. Wicks (j_wicks), Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

//CurrentGame.h
//Declarations for CCurrentGame.h.
//
//GENERAL
//
//Class for accessing and manipulating data used for current game.  This class should
//be completely insulated from graphics, sound, and input devices.  CCurrentGame can be
//used without any UI, since there are no UI-related calls inside of it or its called
//code.
//
//USAGE
//
//Get an instance of CCurrentGame using either CDb::GetSavedCurrentGame() or
//CDb::GetNewCurrentGame().  Your commands will probably either come from a UI (user types
//commands with keyboard) or from a prerecorded game.  These commands are passed via
//CCurrentGame::ProcessCommand().  ProcessCommand() will return cue events, a full set of cues
//available from CueEvents.h.  Some cue events have to be responded to or the
//game will not appear to work, i.e. CID_MonsterKilledPlayer.  Others cue events, like
//CID_TarDestroyed, do not require handling, but can be used to cue sound and graphical
//effects.
//
//Multiple instances of CCurrentGame may be open simultaneously, but don't access
//CCurrentGame instances on different threads.
//
//DOES MY NEW METHOD GO IN CDbRoom OR CCurrentGame?
//
//This question comes up because CDbRoom and CCurrentGame both seem to lay claim to
//the same problem space.  CDbRoom is for accessing room data both before and after the
//swordsman has arrived in that room.  CCurrentGame contains the position of the swordsman,
//game state information, and an instance of CDbRoom that is the current room the swordsman
//is in.
//
//Here is the process to figure out which place the method goes:
//
//  Does the method access members of just one of the classes?  If so, then method goes
//  in that class.
//
//  Does the method need to write to a member of CCurrentGame?  If so, then method goes
//  in CCurrentGame.
//
//Assuming that the above two questions were answered "no", the method should go in CDbRoom.

#ifndef CURRENTGAME_H
#define CURRENTGAME_H
#ifdef WIN32
#  pragma warning(disable:4786)
#endif

#include <BackEndLib/Assert.h>
#include <BackEndLib/AttachableObject.h>
#include <BackEndLib/Coord.h>
#include <BackEndLib/CoordStack.h>
#include <BackEndLib/Types.h>
#include "DbDemos.h"
#include "DbSavedGames.h"
#include "Monster.h"
#include "MonsterMessage.h"
#include "Swordsman.h"

#include <list>
#include <string>
#include <queue>
using std::list;
using std::string;
using std::queue;

class CDbLevel;
class CDbHold;

//*******************************************************************************
//Type used for grouping all the demo recording vars together.
struct DEMO_REC_INFO
{
	UINT dwDescriptionMessageID; //Description of demo.
	UINT  wBeginTurnNo;        //Turn at which recording began.
	UINT dwPrevDemoID;        //DemoID for a previous demo record with recording
									//for room visited before the current.  That record
									//may be updated to link to demo record storing recording
									//for current room.
	UINT dwFirstDemoID;       //DemoID of first demo in a series or demoID of the only
									//demo if there was just one.
	UINT dwFlags;          //explicit properties of demo

	void SetFlag(const CDbDemo::DemoFlag eFlag, const bool bSet=true)
	{
		ASSERT(eFlag < CDbDemo::MaxFlags);
		const UINT dwFlagVal = 1 << eFlag;
		if (bSet)
		{
			this->dwFlags |= dwFlagVal;   //set bit
		} else {
			this->dwFlags &= ~dwFlagVal;  //inverse mask to reset bit
		}
	}
};

//Auto-save option flags.
static const UINT ASO_NONE = 0L;
static const UINT ASO_CHECKPOINT = 1L;
static const UINT ASO_ROOMBEGIN = 2L;
static const UINT ASO_LEVELBEGIN = 4L;
static const UINT ASO_CONQUERDEMO = 8L;
static const UINT ASO_DIEDEMO = 16L;
static const UINT ASO_DEFAULT = ASO_CHECKPOINT | ASO_ROOMBEGIN | ASO_LEVELBEGIN;

static const UINT TIRED_TURN_COUNT = 40;   //# previous turns to check for player becoming tired

static const UINT NO_CONQUER_TOKEN_TURN = UINT(-1);

//*******************************************************************************
struct VarMapInfo {
	bool bInteger;
	UINT val;
	WSTRING wstrVal;
};
typedef std::string VarNameType;
typedef std::map<VarNameType, VarMapInfo> VARMAP;

//*******************************************************************************
class CCharacter;
class CDb;
class CHalph;
class CPlayerDouble;
class CCharacterCommand;
class CFiredCharacterCommand;
struct DEMO_UPLOAD;
class CCurrentGame : public CDbSavedGame
{
protected:
	friend class CDb;
	CCurrentGame();
	CCurrentGame(const CCurrentGame &Src)
		: CDbSavedGame(false), pRoom(NULL), pLevel(NULL),
		  pHold(NULL), pEntrance(NULL), pSnapshotGame(NULL)
	{SetMembers(Src);}

public:
	~CCurrentGame();
	CCurrentGame &operator= (const CCurrentGame &Src) {
		SetMembers(Src);
		return *this;
	}

	WSTRING  AbbrevRoomLocation();
	void     AddNewEntity(CCueEvents& CueEvents, const UINT identity,
			const UINT wX, const UINT wY, const UINT wO);
	void     BeginDemoRecording(const WCHAR* pwczSetDescription,
			const bool bUseCurrentTurnNo=true);
	void     Clear(const bool bNewGame=true);
	static void DiffVarValues(const VARMAP& vars1, const VARMAP& vars2, set<VarNameType>& diff);
	UINT     EndDemoRecording();
	bool     ExecutingNoMoveCommands() const {return this->bExecuteNoMoveCommands;}
	WSTRING  ExpandText(const WCHAR* wText, CCharacter *pCharacter=NULL);
	void     FreezeCommands();
	UINT     GetAutoSaveOptions() const {return this->dwAutoSaveOptions;}
	UINT     GetChecksum() const;
	const CEntity* GetDyingEntity() const {return this->pDyingEntity;}
	const CEntity* GetKillingEntity() const {return this->pKillingEntity;}
	void     GetLevelStats(CDbLevel *pLevel);
	UINT     GetRoomExitDirection(const UINT wMoveO) const;
	WSTRING  GetScrollTextAt(const UINT wX, const UINT wY);
	CEntity* getSpeakingEntity(CFiredCharacterCommand* pFiredCommand);
	bool     GetSwordsman(UINT& wSX, UINT& wSY, const bool bIncludeNonTarget=false) const;
	UINT     GetSwordMovement() const
			{return swordsman.wSwordMovement;}  //note: set in ProcessPlayer
	const CMonsterMessage* GetUnansweredQuestion() const
			{ return UnansweredQuestions.empty() ? NULL : &UnansweredQuestions.front(); }
	UINT     getVar(const UINT varIndex) const;
	void     GetVarValues(VARMAP& vars);
	void     GotoLevelEntrance(CCueEvents& CueEvents, const UINT wEntrance, const bool bSkipEntranceDisplay=false);
	bool     IsCurrentLevelComplete() const;
	bool     IsCurrentRoomConquered() const;
	bool     IsCurrentRoomPendingExit() const;
	bool     IsCurrentRoomExplored() const;
	bool     IsCutScenePlaying() const {return this->dwCutScene && !this->swordsman.wPlacingDoubleType;}
	bool     IsDemoRecording() const {return this->bIsDemoRecording;}
	bool     IsNewRoom() const {return this->bIsNewRoom;}
	bool     IsPlayerAnsweringQuestions() const {return this->UnansweredQuestions.size() != 0;}
	bool     IsPlayerAt(const UINT wX, const UINT wY) const;
	bool     IsPlayerEntranceValid() const;
	bool     IsPlayerDying() const;
	bool     IsPlayerSwordAt(const UINT wX, const UINT wY) const;
	bool     IsPlayerWading() const;
	bool     IsRoomAtCoordsConquered(const UINT dwRoomX, const UINT dwRoomY) const;
	bool     IsRoomAtCoordsExplored(const UINT dwRoomX, const UINT dwRoomY) const;
	static bool IsSupportedPlayerRole(const UINT wType);
	bool     ShouldSaveRoomBegin(const UINT dwRoomID) const;
	bool     LoadFromHold(const UINT dwHoldID, CCueEvents &CueEvents);
	bool     LoadFromLevelEntrance(const UINT dwHoldID, const UINT dwEntranceID,
			CCueEvents &CueEvents);
	bool     LoadFromRoom(const UINT dwRoomID, CCueEvents &CueEvents,
			const UINT wX, const UINT wY, const UINT wO, const UINT wAppearance, const bool bSwordOff,
			const UINT wWaterTraversal,
			const bool bNoSaves=false);
	bool     LoadFromSavedGame(const UINT dwSavedGameID, CCueEvents &CueEvents,
			bool bRestoreAtRoomStart = false, const bool bNoSaves=false);
	bool     LoadNewRoomForExit(const UINT wExitO, CCueEvents &CueEvents);
	void     LoadPrep(const bool bNewGame=true);
	bool     PlayAllCommands(CCueEvents &CueEvents,
			const bool bTruncateInvalidCommands=false);
	bool     PlayCommandsToTurn(const UINT wEndTurnNo, CCueEvents &CueEvents);
	void     PostProcessCharacter(CCharacter* pCharacter, CCueEvents& CueEvents);
	void     ProcessCommandSetVar(const UINT itemID, UINT newVal);
	void     ProcessCommand(int nCommand, CCueEvents &CueEvents,
			const UINT wX=(UINT)-1, const UINT wY=(UINT)-1);
	void     ProcessSwordHit(const UINT wX, const UINT wY, CCueEvents &CueEvents,
			CMonster *pDouble = NULL);
	void     QueryCheckpoint(CCueEvents& CueEvents, const UINT wX, const UINT wY) const;
	void     ReplaceDoubleCommands();
	void     RestartRoom(CCueEvents &CueEvents);
	void     RestartRoomFromLastCheckpoint(CCueEvents &CueEvents);
	void     RestartRoomFromLastDifferentCheckpoint(CCueEvents &CueEvents);
	void     SaveToCheckpoint();
	void     SaveToContinue();
	void     SaveToEndHold();
	void     SaveToLevelBegin();
	void     SaveToRoomBegin();
	void     ScriptCompleted(const CCharacter* pCharacter);
	void     SetAutoSaveOptions(const UINT dwSetAutoSaveOptions)
			{this->dwAutoSaveOptions = dwSetAutoSaveOptions;}
	void     SetCloneSwordsSheathed();
	void     SetComputationTimePerSnapshot(const UINT dwTime);
	void     SetCurrentRoomConquered();
	void     SetCurrentRoomExplored();
	bool     SetDyingEntity(const CEntity* pDyingEntity, const CEntity* pKillingEntity=NULL);
	void     SetExecuteNoMoveCommands(const bool bVal=true) {this->bExecuteNoMoveCommands = bVal;}
	void     SetPlayer(const UINT wSetX, const UINT wSetY);
	void     SetPlayerRole(const UINT wType, CCueEvents& CueEvents);
	bool     SetPlayerSwordSheathed();
	bool     SetPlayerToEastExit();
	bool     SetPlayerToNorthExit();
	bool     SetPlayerToSouthExit();
	bool     SetPlayerToWestExit();
	void     SetRoomStatusFromAllSavedGames();
	void     SetRoomVisited() {this->bIsNewRoom = false;} //back door
	void     SetTurn(UINT wSetTurnNo, CCueEvents &CueEvents);
	bool     ShowLevelStart() const;
	void     SynchClonesWithPlayer(CCueEvents& CueEvents);
	void     TallyKill(const UINT wType);
	bool     TunnelGetExit(const UINT wStartX, const UINT wStartY,
		const int dx, const int dy, UINT& wX, UINT& wY) const;
	void     UndoCommand(CCueEvents &CueEvents);
	void     UndoCommands(const UINT wUndoCount, CCueEvents &CueEvents);
	void     UnfreezeCommands();
	UINT     UpdateTime(const UINT dwTime=0);
	bool     WalkDownStairs();
	UINT    WriteCurrentRoomDieDemo();

	CDbRoom *   pRoom;
	CDbLevel *  pLevel;
	CDbHold *   pHold;
	CEntranceData *pEntrance;

	//Player state
	CSwordsman swordsman;

	//Game state vars
	bool     bIsDemoRecording;
	bool     bIsGameActive;
	UINT     wTurnNo;
	UINT     wPlayerTurn;      //player move #
	UINT     wSpawnCycleCount; //monster move #
	bool     bHalfTurn;        //half a turn taken 
	bool     bBrainSensesSwordsman;
	UINT     wLastCheckpointX, wLastCheckpointY;
	vector<UINT> checkpointTurns; //turn #s a checkpoint was activated
	UINT    dwStartTime;   //keeps track of real play time elapsed
	bool     bHoldMastered; //whether player has mastered hold being played
	UINT    dwCutScene;    //if set, play cut scene moves at specified rate
	bool     bContinueCutScene;
	bool     bWaitedOnHotFloorLastTurn;
	CDbPackedVars statsAtRoomStart; //stats when room was begun
	vector<CMoveCoordEx> ambientSounds;  //ambient sounds playing now
	vector<CCharacterCommand*> roomSpeech; //speech played up to this moment in the current room
	bool     bRoomExitLocked; //safety to prevent player from exiting room when set
	UINT     conquerTokenTurn; //turn player touched a Conquer token

	UINT     wMonsterKills; //total monsters killed in current room
	UINT     wMonsterKillCombo; //total monsters killed without interruption

	//Internet.
	static queue<DEMO_UPLOAD*> demosForUpload;
	CIDSet   GlobalScriptsRunning;      //currently running global scripts, saved on room exit

protected:
	void     PostSave(const bool bConqueredOnEntrance, const bool bExploredOnEntrance);
	bool     SavePrep(bool& bExploredOnEntrance);

private:
	void     AddCompletedScripts();
	void     AddRoomsToPlayerTally();
	void     AddQuestionsToList(CCueEvents &CueEvents,
			list<CMonsterMessage> &QuestionList) const;
	void     AmbientSoundTracking(CCueEvents &CueEvents);
	void     BlowHorn(CCueEvents &CueEvents, const UINT wSummonType,
						const UINT wHornX, const UINT wHornY);
	void     DeleteLeakyCueEvents(CCueEvents &CueEvents);
	void     DrankPotion(CCueEvents &CueEvents, const UINT wDoubleType,
							const UINT wPotionX, const UINT wPotionY);
	void     FegundoToAsh(CMonster *pMonster, CCueEvents &CueEvents);
	bool     IsSwordsmanTired();
	void     LoadNewRoomForExit(const UINT dwNewSX, const UINT dwNewSY,
			CDbRoom* pNewRoom, CCueEvents &CueEvents, const bool bSaveGame);
	bool     LoadEastRoom();
	bool     LoadNorthRoom();
	bool     LoadSouthRoom();
	bool     LoadWestRoom();
	bool     PlayerCanExitRoom(const UINT wDirection, UINT &dwNewSX,
			UINT &dwNewSY, CDbRoom* &pNewRoom);
	void     ProcessDoublePlacement(int nCommand, CCueEvents &CueEvents,
			const UINT wX, const UINT wY);
	void     PreprocessMonsters(CCueEvents &CueEvents);
	void     ProcessMonsters(int nLastCommand, CCueEvents &CueEvents);
	void     ProcessSimultaneousSwordHits(CCueEvents &CueEvents);
	void     ProcessPlayer(const int nCommand, CCueEvents &CueEvents);
	void     ProcessPlayer_HandleLeaveLevel(CCueEvents &CueEvents,
			const UINT wEntrance=(UINT)-1, const bool bSkipEntranceDisplay=false);
	bool     ProcessPlayer_HandleLeaveRoom(const UINT wMoveO,
			CCueEvents &CueEvents);
	void     ProcessUnansweredQuestions(const int nCommand,
			list<CMonsterMessage> &UnansweredQuestions, CCueEvents &CueEvents);
	void     SetLevelStats();
	void     SetMembers(const CCurrentGame &Src);
	void     SetMembersAfterRoomLoad(CCueEvents &CueEvents, const bool bResetCommands=true);
	void     SetPlayerMood(CCueEvents &CueEvents);
	void     SetPlayerToRoomStart();
	bool     SetRoomAtCoords(const UINT dwRoomX, const UINT dwRoomY);
	void     SetRoomStartToPlayer();
	void     SwitchToCloneAt(const UINT wX, const UINT wY);
	bool     TakeSnapshotNow() const;
	bool     ToggleGreenDoors();
	bool     TunnelMove(const int dx, const int dy);
	void     UpdatePrevCoords();
	void     UpdatePrevPlatformCoords();
	void     UploadExploredRoom();
	bool     WasRoomConqueredOnThisVisit() const;
	UINT    WriteCurrentRoomConquerDemo();
	UINT    WriteCurrentRoomDemo(DEMO_REC_INFO &dri, const bool bHidden=false,
			const bool bAppendRoomLocation=true);

	vector<CMoveCoord> simulSwordHits;   //vulnerable tar hit simultaneously

	//"swordsman exhausted/relieved" event logic
	unsigned char monstersKilled[TIRED_TURN_COUNT]; //rolling sum of monsters killed in recent turns
	UINT     wMonstersKilledRecently;
	bool     bLotsOfMonstersKilled;

	DEMO_REC_INFO        DemoRecInfo;
	list<CMonsterMessage>   UnansweredQuestions;
	bool              bIsNewRoom;
	bool     bExecuteNoMoveCommands;
	UINT     wAddNewEntityDepth; //track in order to limit recursive depth of AddNewEntity

	const CEntity *pDyingEntity, *pKillingEntity; //records entities that caused the first gameover for turn

	UINT     dwAutoSaveOptions;
	CIDSet   CompletedScriptsPending;   //saved permanently on room exit
	bool     bNoSaves;   //don't save anything to DB when set (e.g., for dummy game sessions)

	CCurrentGame *pSnapshotGame; //for optimized room rewinds
	UINT dwComputationTime; //time required to process game moves up to this point
	UINT dwComputationTimePerSnapshot; //real movement computation time between game state snapshots
	UINT numSnapshots;
};

#endif //...#ifndef CURRENTGAME_H
