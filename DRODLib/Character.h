// $Id: Character.h 10027 2012-03-28 20:11:54Z TFMurphy $

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

//Character.h
//Declarations for CCharacter.
//Class for handling scriptable monster character game logic.

//A character may take the form of Beethro, an NPC or most monsters.
//It is used to deliver story elements to the player.
//
//Remarks about character scripting:
//
//1.  Each character script begins execution when a room is entered.
//    Commands that require expending a turn are delayed until the player moves.
//2.  A script can be made to run only once by executing the CC_EndScript or
//    CC_EndScriptOnExit command.
//    The character will be removed from play for the remainder of the current game.
//3.  Character monsters are not used to determine room completion.
//    That is, character monsters will remain in a room even after it has been
//    conquered.  To remove a character once a room has been beaten, insert a CC_GoTo
//    end of the script on "WaitForCleanRoom" event at the beginning.
//4.  A character can be turned into a regular NPC or monster with the
//    CC_TurnIntoMonster command.  In this case, (a) its script will count as
//    not having been completed and the character will always exist in the room,
//    even after it has been conquered (see 3), and (b) it might need to be killed
//    in order to conquer the room.

#ifndef CHARACTER_H
#define CHARACTER_H

#include "MonsterFactory.h"
#include "CueEvents.h"
#include "GameConstants.h"
#include "DbSpeech.h"
#include "PlayerDouble.h"
#include "PlayerStats.h"

#include <vector>
using std::vector;

//Speakers for whom some faces are implemented.
enum SPEAKER
{
	//Custom speakers available to everyone.
	Speaker_Beethro=0,	//don't change these constants
	Speaker_Gunthro=51,
	Speaker_Citizen1=6,
	Speaker_Citizen2=7,
	Speaker_Citizen3=45,
	Speaker_Citizen4=46,
	Speaker_Custom=5,
	Speaker_EyeActive=15,
	Speaker_GoblinKing=8, //deprecated
	Speaker_Instructor=10, //deprecated
	Speaker_MudCoordinator=11, //deprecated
	Speaker_Negotiator=3, //deprecated
	Speaker_None=4,
	Speaker_TarTechnician=13, //deprecated
	Speaker_BeethroInDisguise=47,
	Speaker_Self=50,
	Speaker_Player=52,

	//Monster speakers.
	Speaker_Halph=1,
	Speaker_Slayer=2,
	Speaker_Goblin=9,
	Speaker_RockGolem=12,
	Speaker_Guard=14,
	Speaker_Stalwart=17,
	Speaker_Roach=18,
	Speaker_QRoach=19,
	Speaker_RoachEgg=20,
	Speaker_WWing=21,
	Speaker_Eye=22,
	Speaker_Serpent=23,
	Speaker_TarMother=24,
	Speaker_TarBaby=25,
	Speaker_Brain=26,
	Speaker_Mimic=27,
	Speaker_Spider=28,
	Speaker_SerpentG=29,
	Speaker_SerpentB=30,
	Speaker_WaterSkipper=31,
	Speaker_WaterSkipperNest=32,
	Speaker_Aumtlich=33,
	Speaker_Clone=34,
	Speaker_Decoy=35,
	Speaker_Wubba=36,
	Speaker_Seep=37,
	Speaker_Fegundo=38,
	Speaker_FegundoAshes=39,
	Speaker_MudMother=40,
	Speaker_MudBaby=41,
	Speaker_GelMother=42,
	Speaker_GelBaby=43,
	Speaker_Citizen=16,
	Speaker_RockGiant=44,
	Speaker_Slayer2=48,
	Speaker_Halph2=49,
	Speaker_Stalwart2=53,

	Speaker_Count=54
};

SPEAKER getSpeakerType(const MONSTERTYPE eType);
UINT   getSpeakerNameText(const UINT wSpeaker, string& color);

//Flags bits.
namespace ScriptFlag
{
	static const UINT PLAYER  = 0x00000001;
	static const UINT HALPH   = 0x00000002;
	static const UINT MONSTER = 0x00000004;
	static const UINT NPC     = 0x00000008;
	static const UINT PDOUBLE = 0x00000010;
	static const UINT SELF    = 0x00000020;
	static const UINT SLAYER  = 0x00000040;
	static const UINT BEETHRO = 0x00000080;
	static const UINT STALWART= 0x00000100;

	//How killing NPC affects the game
	enum Imperative
	{
		Vulnerable=0,        //can be killed (default)
		Invulnerable=1,      //can't be killed
		MissionCritical=2,   //must not be killed, or play restarts
		RequiredToConquer=3, //must be killed to conquer room
		Die=4,               //NPC dies
		DieSpecial=5,        //NPC dies in way distinct to specific monster type
		Safe=6,              //can't kill player by stepping (default)
		Deadly=7,            //can kill player by stepping and sword
		SwordSafeToPlayer=8, //sword doesn't damage player
		EndWhenKilled=9,     //End is invoked when NPC is killed
		FlexibleBeelining=10,//use SmartOmniDirection movement (default)
		DirectBeelining=11,  //use DirectOnly movement
		NoGhostDisplay=12,   //(f.e.) NPC isn't displayed when not visible (default)
		GhostDisplay=13      //(f.e.) NPC is displayed when not visible -- 1.e. a ghost
	};
};

struct HoldCharacter;
class CCharacterCommand
{
public:
	CCharacterCommand()
		: command((CharCommand)0)
		, x(0), y(0), w(0), h(0), flags(0), pSpeech(NULL)
	{}
	CCharacterCommand(const CCharacterCommand& that, const bool bReplicateData=false)
		: command(that.command)
		, x(that.x), y(that.y), w(that.w), h(that.h), flags(that.flags), label(that.label), pSpeech(NULL)
	{
		if (that.pSpeech)
			this->pSpeech = new CDbSpeech(*that.pSpeech, bReplicateData);
	}
	~CCharacterCommand() {delete this->pSpeech;}
	CCharacterCommand& operator=(const CCharacterCommand& that)
	{
		CCharacterCommand self(that);
		swap(self);
		return *this;
	}
	void swap(CCharacterCommand &that) {
		std::swap(command, that.command);
		std::swap(w, that.w);
		std::swap(x, that.x);
		std::swap(y, that.y);
		std::swap(h, that.h);
		std::swap(flags, that.flags);
		std::swap(label, that.label);
		std::swap(pSpeech, that.pSpeech);
	}

	enum CharCommand
	{
		CC_Appear=0,            //Appear at current square.
		CC_AppearAt,            //Appear at square (x,y).
		CC_MoveTo,              //Move to square (x,y) or target set in flags.
		                        //If w is set, then forbid turning while moving.
		                        //If h is set, then take only a single step before advancing to next command.
		CC_Wait,                //Wait X turns.
		CC_WaitForCueEvent,     //Wait for cue event X to fire.
		CC_WaitForRect,         //Wait until an entity in flags is in rect (x,y,w,h).
		CC_Speech,              //Deliver speech referenced by SpeechID, possibly at (x,y).
		CC_Imperative,          //Sets imperative status to X (was CC_Invincibility in 2.0).
		CC_Disappear,           //Disappear. (Use in conjunction with CC_MoveTo command to leave the room.)
		CC_TurnIntoMonster,     //Replace with normal monster of specified type.
		CC_FaceDirection,       //Rotate to face orientation X.
		CC_WaitForNotRect,      //Wait until an entity in flags is not in rect (x,y,w,h).
		CC_WaitForDoorTo,       //Wait for door at (x,y) to (w=close/open).
		CC_Label,               //Destination (x) for a GoTo command.
		CC_GoTo,                //Continue executing script commands from Label (x).
		CC_GotoIf,             //(deprecated) Goto command with (label) if the next command is satisfied.
		CC_WaitForMonster,     //(deprecated) Wait until monster is in rect (x,y,w,h).
		CC_WaitForNotMonster,  //(deprecated) Wait until monster is not in rect (x,y,w,h).
		CC_WaitForTurn,         //Wait until play reaches turn X.
		CC_WaitForCleanRoom,    //Wait until room has been cleaned of monsters.
		CC_WaitForPlayerToFace, //Wait until player faces orientation X.
		CC_ActivateItemAt,      //Activate item at (x,y).  Works only for some items.
		CC_EndScript,           //Removes the character for the rest of the current game.
		CC_WaitForHalph,       //(deprecated) Wait until Halph is in rect (x,y,w,h).
		CC_WaitForNotHalph,    //(deprecated) Wait until Halph is not in rect (x,y,w,h).
		CC_WaitForCharacter,   //(deprecated) Wait until a visible character is in rect (x,y,w,h).
		CC_WaitForNotCharacter,//(deprecated) Wait until no visible character is in rect (x,y,w,h).
		CC_FlushSpeech,         //(Front end) Purge speech events in queue (x=display/erase)
		CC_Question,			   //Ask a yes/no or multiple choice question (speech text).
		CC_SetMusic,			   //Set music being played to X (custom Y/label).
		CC_EndScriptOnExit,     //Removes the character for the rest of the current game when room is exited.
		CC_If,                  //Begin a conditional block if next command is satisfied.
		CC_IfElse,              //Begin a conditional block when command following CC_If was not satisfied (also ends If block).
		CC_IfEnd,               //Ends a conditional If or IfElse block.
		CC_LevelEntrance,       //Takes player to level entrance X.  If Y is set, skip level entrance display.
		CC_VarSet,              //Sets var X (operation Y) W, e.g. X += 5
		CC_WaitForVar,          //Wait until var X (comparison Y) W, e.g. X >= 5
		CC_SetPlayerAppearance, //Sets player to look like entity X.
		CC_CutScene,            //Begin cut scene (if X is set), else ends cut scene.
		CC_MoveRel,             //Move (x,y) relative to current position. If w is set, then forbid turning while moving.
		                        //If h is set, then take only a single step before advancing to next command.
		CC_SetPlayerSword,      //If X is set, player is given a sword, else it is taken away.
		CC_AnswerOption,        //Text answer option (speech text) for a Question command that jumps to Label (x).
		CC_BuildMarker,         //Mark rect (x,y,w,h) for building game element (flags).
		CC_AmbientSound,        //Play sound with DataID=w (0 stops ambient sounds).  If h is set, loop indefinitely.
		CC_AmbientSoundAt,      //Play sound with DataID=w (0 stops ambient sounds) at (x,y).  If h is set, loop indefinitely.
		CC_WaitForNoBuilding,   //Wait until no build markers are queued in rect (x,y,w,h).
		CC_PlayVideo,           //Play video at (x,y) with DataID=w.
		CC_WaitForPlayerToMove, //Wait until player moves in direction X.
		CC_WaitForPlayerToTouchMe, //Wait until player bumps into this NPC.
		CC_SetNPCAppearance,    //Sets this NPC to look like entity X.

		//F&M commands
		CC_SetWaterTraversal,	//Sets Shallow Water traversal for players to X.  Default value is 'As player role'
		CC_StartGlobalScript,   //Starts the specified custom character's default script up as a global script
		CC_WaitForItem,         //Wait for game element (flags) to exist in rect (x,y,w,h).
		CC_GenerateEntity,      //Generates a new entity of type h in the room at (x,y) with orientation w.
		CC_GameEffect,          //Cues the front end to generate a graphic+sound effect (w,h,flags) at (x,y).

		CC_Count
	};

	CharCommand command;
	UINT x, y, w, h, flags;
	WSTRING label;    //goto identifier
	CDbSpeech *pSpeech;
};

typedef vector<CCharacterCommand> COMMAND_VECTOR;
typedef vector<CCharacterCommand*> COMMANDPTR_VECTOR;

class CCharacter : public CPlayerDouble
{
public:
	CCharacter(CCurrentGame *pSetCurrentGame = NULL);
	IMPLEMENT_CLONE(CMonster, CCharacter);
	virtual CMonster* Replicate() const;

	virtual bool   BrainAffects() const {return false;}
	void           ChangeHold(CDbHold* pOldHold, CDbHold* pNewHold, CImportInfo& info, const bool bGetNewScriptID=true);
	void           CheckForCueEvent(CCueEvents &CueEvents);
	virtual bool   CheckForDamage(CCueEvents& CueEvents);
	virtual bool   DoesSquareContainObstacle(const UINT wCol, const UINT wRow) const;
	bool           EndsWhenKilled() const {return this->bEndWhenKilled;}

	void   ExportText(CDbRefs &dbRefs, CStretchyBuffer& str);
	static string ExportXMLSpeech(CDbRefs &dbRefs, const COMMAND_VECTOR& commands, const bool bRef=false);
	MESSAGE_ID ImportSpeech(CImportInfo &info, const bool bHoldChar=false);
	void   ImportText(const char** atts);

	void           FailedIfCondition();
	const CCharacterCommand* GetCommandWithLabel(const UINT label) const;
	virtual UINT   GetIdentity() const {return this->wIdentity;}
	virtual UINT   GetResolvedIdentity() const;
	ScriptFlag::Imperative GetImperative() const {return this->eImperative;}
	UINT           GetNextSpeechID();
	bool           HasSpecialDeath() const;

	void           getCommandParams(const CCharacterCommand& command,
			UINT& x, UINT& y, UINT& w, UINT& h, UINT& f) const;
	void           getCommandRect(const CCharacterCommand& command,
			UINT& x, UINT& y, UINT& w, UINT& h) const;
	void           getCommandX(const CCharacterCommand& command,
			UINT& x) const;
	void           getCommandXY(const CCharacterCommand& command,
			UINT& x, UINT& y) const;

	UINT  getPredefinedVar(const UINT varIndex) const;

	virtual bool   IsAlive() const {return this->bAlive && !this->bReplaced;}
	virtual bool   IsAggressive() const {return false;}
	virtual bool   IsAttackableTarget() const;
	virtual bool   IsFlying() const;
	virtual bool   IsFriendly() const;
	bool           IsGhostImage() const {return this->bGhostImage;}
	virtual bool   IsMonsterTarget() const;
	virtual bool	IsOpenMove(const int dx, const int dy) const;
	bool           IsSafeToPlayer() const {return this->bSafeToPlayer;}
	virtual bool   IsSwimming() const;
	bool           IsSwordSafeToPlayer() const {return this->bSwordSafeToPlayer;}
	bool           IsTileAt(const CCharacterCommand& command) const;
	virtual bool   IsTileObstacle(const UINT wTileNo) const;
	static bool    IsValidExpression(const WCHAR *pwStr, UINT& index, CDbHold *pHold, const bool bExpectCloseParen=false);
	static bool    IsValidTerm(const WCHAR *pwStr, UINT& index, CDbHold *pHold);
	static bool    IsValidFactor(const WCHAR *pwStr, UINT& index, CDbHold *pHold);
	virtual bool   IsVisible() const {return this->bVisible;}
	bool           JumpToCommandWithLabel(const WCHAR *pText);
	bool           JumpToCommandWithLabel(const UINT num);
	static void    LoadCommands(CDbPackedVars& ExtraVars, COMMAND_VECTOR& commands);
	static void    LoadCommands(CDbPackedVars& ExtraVars, COMMANDPTR_VECTOR& commands);
	virtual bool   OnAnswer(int nCommand, CCueEvents &CueEvents);
	virtual bool   OnStabbed(CCueEvents &CueEvents, const UINT /*wX*/=-1, const UINT /*wY*/=-1);
	static int     parseExpression(const WCHAR *pwStr, UINT& index, CCurrentGame *pGame, CCharacter *pNPC=NULL, const bool bExpectCloseParen=false);
	static int     parseTerm(const WCHAR *pwStr, UINT& index, CCurrentGame *pGame, CCharacter *pNPC);
	static int     parseFactor(const WCHAR *pwStr, UINT& index, CCurrentGame *pGame, CCharacter *pNPC);
	virtual void   Process(const int nLastCommand, CCueEvents &CueEvents);

	virtual void   ReflectX(CDbRoom *pRoom);
	virtual void   ReflectY(CDbRoom *pRoom);
	bool           ResetLevelExits();

	void           ResolveLogicalIdentity(CDbHold *pHold);
	virtual void   SetCurrentGame(const CCurrentGame *pSetCurrentGame);
	void   SetImperative(const ScriptFlag::Imperative eVal) {this->eImperative = eVal;}
	virtual void   SetExtraVarsFromMembers(const bool bHoldChar=false);
	void           SetExtraVarsFromMembersWithoutScript(const bool bHoldChar=false);
	virtual void   SetMembersFromExtraVars();
	virtual void   Save(const c4_RowRef &MonsterRowRef, const bool bSaveScript=true);
	static void    SaveCommands(CDbPackedVars& ExtraVars, const COMMAND_VECTOR& commands);
	static void    SaveCommands(CDbPackedVars& ExtraVars, const COMMANDPTR_VECTOR& commands);
	static void    SaveSpeech(const COMMAND_VECTOR& commands);
	static void    SaveSpeech(const COMMANDPTR_VECTOR& commands);
	virtual void   Delete();

	virtual bool SetSwordSheathed();

	COMMAND_VECTOR commands;
	UINT dwScriptID;       //charater script ref
	UINT  wIdentity;        //monster type
	UINT  wLogicalIdentity; //logical ID (might be a hold custom character type)
	HoldCharacter *pCustomChar; //custom character type
	bool  bVisible;         //on screen in room, or not
	bool  bGhostImage;      //display on screen even if bVisible is false
	bool  bScriptDone;      //true when script has run to completion
	bool  bReplaced;        //true when script command replaces the character
									//with a normal monster
	bool  bGlobal;          //true if this is a global script character
	bool  bNewEntity;       //true if this character was created via AddNewEntity
	bool  bYesNoQuestion;   //question type being asked
	CIDSet answerOptions;   //optional answers supplied to a Question command
	bool  bPlayerTouchedMe; //player bumped into this NPC this turn

private:
	void BuildTiles(const CCharacterCommand& command);
	void Disappear();
	int  GetIndexOfCommandWithLabel(const UINT label) const;
	void MoveCharacter(const int dx, const int dy, const bool bFaceDirection,
			CCueEvents& CueEvents);
	void TeleportCharacter(const UINT wDestX, const UINT wDestY, CCueEvents& CueEvents);
	void TurnIntoMonster(CCueEvents& CueEvents, const bool bSpecial=false);

	void setPredefinedVar(UINT varIndex, const UINT val, CCueEvents& CueEvents);

	//changing internal command storage representation --> 3.0.2 rev3
	static UINT  readBpUINT(const BYTE* buffer, UINT& index);
	static void  writeBpUINT(string& buffer, UINT n);
	static void  DeserializeCommand(BYTE* buffer, UINT& index, CCharacterCommand& command);
	static void  DeserializeCommands(BYTE* buffer, const UINT bufferSize, COMMAND_VECTOR& commands);
	static void  DeserializeCommands(BYTE* buffer, const UINT bufferSize, COMMANDPTR_VECTOR& commands);
	static void  SerializeCommand(string& buffer, const CCharacterCommand& command);
	static void  SerializeCommands(string& buffer, const COMMAND_VECTOR& commands);
	static void  SerializeCommands(string& buffer, const COMMANDPTR_VECTOR& commands);

	//changing internal command storage representation from pre-3.0.2 --> 3.0.2 rev2
	static UINT  readUINT(const BYTE* buffer, UINT& index);
	static void  writeUINT(string& buffer, UINT n);
	static void  DeserializeCommands_3_0_2_2(BYTE* buffer, const UINT bufferSize, COMMAND_VECTOR& commands);

	static void  Upgrade2_0CommandTo3_0(CCharacterCommand& command, COMMAND_VECTOR& commands);

	void         SetBaseMembersFromExtraVars();

	UINT wCurrentCommandIndex; //command to play next
	UINT wTurnDelay;        //turns before next command
	ScriptFlag::Imperative eImperative; //imperative status
	UINT wXRel, wYRel;      //destination of relative movement
	bool bMovingRelative;   //true when MoveRel destination is set
	bool bSafeToPlayer;     //whether NPC can kill player by stepping
	bool bSwordSafeToPlayer;//if true, sword stabs won't damage player
	bool bEndWhenKilled;    //script completes when killed
	MovementIQ movementIQ;  //movement behavior

	UINT wJumpLabel;			//if non-zero, jump to the label if this command is satisfied
	bool bWaitingForCueEvent;
	bool bIfBlock;

	UINT wLastSpeechLineNumber; //used during language import

	//Predefined vars.
	UINT color, sword; //cosmetic details
	UINT paramX, paramY, paramW, paramH, paramF; //script-definable script command parameter overrides
};

class CFiredCharacterCommand : public CAttachableObject
{
public:
	CFiredCharacterCommand(CMonster *pMonster, CCharacterCommand *pCommand, const UINT turnNo,
			const UINT scriptID, const UINT commandIndex)
		: CAttachableObject()
		, pSpeakingEntity(pMonster), pExecutingNPC(pMonster)
		, pCommand(pCommand)
		, bPlaySound(true)
		, bFlush(false)
		, bPseudoMonster(false)
		, turnNo(turnNo)
		, scriptID(scriptID)
		, commandIndex(commandIndex)
	{}

	CMonster *pSpeakingEntity, *pExecutingNPC;
	CCharacterCommand *pCommand;
	WSTRING text;     //interpolated subtitle text
	bool bPlaySound;  //whether sound clip will be played
	bool bFlush;      //flag to flush queued speech commands
	bool bPseudoMonster; //attached monster is to be deleted

	UINT turnNo;   //when this command was executed
	UINT scriptID; //unique ID of the character executing this command
	UINT commandIndex; //index of speech command in character's script
};

//*****************************************************************************
//CC_VisualEffect uses to pass data to front end.
struct VisualEffectInfo : public CAttachableObject
{
	VisualEffectInfo(const UINT x, const UINT y, const UINT o, const UINT type, const UINT sound,
			const UINT srcX, const UINT srcY)
		: CAttachableObject()
		, effect(x, y, o, type, sound)
		, source(srcX, srcY)
	{	}
	CMoveCoordEx2 effect;
	CCoord source;
};

#endif //...#ifndef CHARACTER_H
