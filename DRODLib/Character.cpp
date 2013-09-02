// $Id: Character.cpp 10072 2012-04-04 01:02:58Z TFMurphy $

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

#include "Character.h"
#include "Clone.h"
#include "Db.h"
#include "DbHolds.h"
#include "EvilEye.h"
#include "Phoenix.h"
#include "../Texts/MIDs.h"

#include <BackEndLib/Base64.h>
#include <BackEndLib/Ports.h>
#include <BackEndLib/Files.h>

const UINT NPC_DEFAULT_SWORD = UINT(-1);

#define NO_LABEL (-1)

#define NO_OVERRIDE (UINT(-9999))

//Literals used to query and store values for the NPC in the packed vars object.
#define commandStr_3_0_2_2 "SerializedCommands"
#define commandStr "Commands"
#define idStr "id"
#define numCommandsStr "NumCommands"
#define scriptIDstr "ScriptID"
#define visibleStr "visible"

#define ColorStr "Color"
#define SwordStr "Sword"
#define ParamXStr "XParam"
#define ParamYStr "YParam"
#define ParamWStr "WParam"
#define ParamHStr "HParam"
#define ParamFStr "FParam"

#define SKIP_WHITESPACE(str, index) while (iswspace(str[index])) ++index

//*****************************************************************************
bool addWithClamp(int& val, const int operand)
//Multiplies two integers, ensuring the product doesn't overflow.
//
//Returns: false if actual result can't be given (i.e. value overflowed), otherwise true
{
	const double newVal = (double)val + operand;
	if (newVal > INT_MAX)
	{
		val = INT_MAX;
		return false;
	}
	if (newVal < INT_MIN)
	{
		val = INT_MIN;
		return false;
	}
	val += operand;
	return true;
}

//*****************************************************************************
inline bool multWithClamp(int& val, const int operand)
//Multiplies two integers, ensuring the product doesn't overflow.
//
//Returns: false if actual result can't be given (i.e. value overflowed), otherwise true
{
	const double newVal = (double)val * operand;
	if (newVal > INT_MAX)
	{
		val = INT_MAX;
		return false;
	}
	if (newVal < INT_MIN)
	{
		val = INT_MIN;
		return false;
	}
	val *= operand;
	return true;
}

//*****************************************************************************
SPEAKER getSpeakerType(const MONSTERTYPE eType)
//Return: corresponding speaker enumeration for monster type, if supported.
{
	switch (eType)
	{
		//Character monster psuedo-types.
		case M_BEETHRO: return Speaker_Beethro;
		case M_BEETHRO_IN_DISGUISE: return Speaker_BeethroInDisguise;
		case M_GUNTHRO: return Speaker_Gunthro;
		case M_NEGOTIATOR: return Speaker_Negotiator;
		case M_NEATHER:
		case M_CITIZEN1: return Speaker_Citizen1;
		case M_CITIZEN2: return Speaker_Citizen2;
		case M_CITIZEN3: return Speaker_Citizen3;
		case M_CITIZEN4: return Speaker_Citizen4;
		case M_GOBLINKING: return Speaker_GoblinKing;
		case M_INSTRUCTOR: return Speaker_Instructor;
		case M_MUDCOORDINATOR: return Speaker_MudCoordinator;
		case M_TARTECHNICIAN: return Speaker_TarTechnician;
		case M_EYE_ACTIVE: return Speaker_EyeActive;

		//Monster types.
		case M_ROACH: return Speaker_Roach;
		case M_QROACH: return Speaker_QRoach;
		case M_REGG: return Speaker_RoachEgg;
		case M_GOBLIN: return Speaker_Goblin;
		case M_WWING: return Speaker_WWing;
		case M_EYE: return Speaker_Eye;
		case M_SERPENT: return Speaker_Serpent;
		case M_TARMOTHER: return Speaker_TarMother;
		case M_TARBABY: return Speaker_TarBaby;
		case M_BRAIN: return Speaker_Brain;
		case M_MIMIC: return Speaker_Mimic;
		case M_SPIDER: return Speaker_Spider;
		case M_SERPENTG: return Speaker_SerpentG;
		case M_SERPENTB: return Speaker_SerpentB;
		case M_ROCKGOLEM: return Speaker_RockGolem;
		case M_WATERSKIPPER: return Speaker_WaterSkipper;
		case M_SKIPPERNEST: return Speaker_WaterSkipperNest;
		case M_AUMTLICH: return Speaker_Aumtlich;
		case M_CLONE: return Speaker_Clone;
		case M_DECOY: return Speaker_Decoy;
		case M_WUBBA: return Speaker_Wubba;
		case M_SEEP: return Speaker_Seep;
		case M_STALWART: return Speaker_Stalwart;
		case M_STALWART2: return Speaker_Stalwart2;
		case M_HALPH: return Speaker_Halph;
		case M_HALPH2: return Speaker_Halph2;
		case M_SLAYER: return Speaker_Slayer;
		case M_SLAYER2: return Speaker_Slayer2;
		case M_FEGUNDO: return Speaker_Fegundo;
		case M_FEGUNDOASHES: return Speaker_FegundoAshes;
		case M_GUARD: return Speaker_Guard;
		case M_MUDMOTHER: return Speaker_MudMother;
		case M_MUDBABY: return Speaker_MudBaby;
		case M_GELMOTHER: return Speaker_GelMother;
		case M_GELBABY: return Speaker_GelBaby;
		case M_CITIZEN: return Speaker_Citizen;
		case M_ROCKGIANT: return Speaker_RockGiant;

		default: return Speaker_None;
	}
}

//*****************************************************************************
UINT getSpeakerNameText(const UINT wSpeaker, string& color)
//Returns: MID corresponding to name text for this speaker.
//Outputs color string for HTML text.
{
	UINT dwSpeakerTextID;
	switch (wSpeaker)
	{
		//Custom speakers.
		case Speaker_BeethroInDisguise: dwSpeakerTextID = MID_BeethroInDisguise; color = "1010A0"; break;
		case Speaker_Beethro: dwSpeakerTextID = MID_Beethro; color = "0000A0"; break;
		case Speaker_Gunthro: dwSpeakerTextID = MID_Gunthro; color = "000080"; break;
		case Speaker_Negotiator: dwSpeakerTextID = MID_Negotiator; color = "0000FF"; break;
		case Speaker_Citizen1: dwSpeakerTextID = MID_Citizen1; color = "800080"; break;
		case Speaker_Citizen2: dwSpeakerTextID = MID_Citizen2; color = "0000A0"; break;
		case Speaker_Citizen3: dwSpeakerTextID = MID_Citizen3; color = "800080"; break;
		case Speaker_Citizen4: dwSpeakerTextID = MID_Citizen4; color = "0000A0"; break;
		case Speaker_GoblinKing: dwSpeakerTextID = MID_GoblinKing; color = "004000"; break;
		case Speaker_Instructor: dwSpeakerTextID = MID_Instructor; color = "0000FF"; break;
		case Speaker_MudCoordinator: dwSpeakerTextID = MID_MudCoordinator; color = "800000"; break;
		case Speaker_TarTechnician: dwSpeakerTextID = MID_TarTechnician; color = "0000FF"; break;
		case Speaker_Custom: dwSpeakerTextID = MID_Custom; color = "202020"; break;
		case Speaker_None: dwSpeakerTextID = MID_None; color = "000000"; break;
		case Speaker_Self: dwSpeakerTextID = MID_Self; color = "000000"; break;
		case Speaker_Player: dwSpeakerTextID = MID_Player; color = "0000A0"; break;
		case Speaker_EyeActive: dwSpeakerTextID = MID_EvilEyeActive; color = "FF0000"; break;

		//Monster speakers.
		case Speaker_Halph: dwSpeakerTextID = MID_Halph; color = "804020"; break;
		case Speaker_Halph2: dwSpeakerTextID = MID_Halph2; color = "804020"; break;
		case Speaker_Slayer: dwSpeakerTextID = MID_Slayer; color = "B00060"; break;
		case Speaker_Slayer2: dwSpeakerTextID = MID_Slayer2; color = "B00060"; break;
		case Speaker_Goblin: dwSpeakerTextID = MID_Goblin; color = "008000"; break;
		case Speaker_RockGolem: dwSpeakerTextID = MID_StoneGolem; color = "800000"; break;
		case Speaker_Guard: dwSpeakerTextID = MID_Guard; color = "400000"; break;
		case Speaker_Stalwart: dwSpeakerTextID = MID_Stalwart; color = "D0D080"; break;
		case Speaker_Stalwart2: dwSpeakerTextID = MID_Stalwart2; color = "806040"; break;
		case Speaker_Roach: dwSpeakerTextID = MID_Roach; color = "202020"; break;
		case Speaker_QRoach: dwSpeakerTextID = MID_RoachQueen; color = "404040"; break;
		case Speaker_RoachEgg: dwSpeakerTextID = MID_Roach; color = "202020"; break;
		case Speaker_WWing: dwSpeakerTextID = MID_Wraithwing; color = "000000"; break;
		case Speaker_Eye: dwSpeakerTextID = MID_EvilEye; color = "0000FF"; break;
		case Speaker_Serpent: dwSpeakerTextID = MID_Serpent; color = "FF0000"; break;
		case Speaker_TarMother: dwSpeakerTextID = MID_TarMother; color = "0000FF"; break;
		case Speaker_TarBaby: dwSpeakerTextID = MID_TarBaby; color = "0000FF"; break;
		case Speaker_Brain: dwSpeakerTextID = MID_Brain; color = "FF0000"; break;
		case Speaker_Mimic: dwSpeakerTextID = MID_Mimic; color = "0000A0"; break;
		case Speaker_Spider: dwSpeakerTextID = MID_Spider; color = "101010"; break;
		case Speaker_SerpentG: dwSpeakerTextID = MID_GreenSerpent; color = "00FF00"; break;
		case Speaker_SerpentB: dwSpeakerTextID = MID_BlueSerpent; color = "0000FF"; break;
		case Speaker_WaterSkipper: dwSpeakerTextID = MID_Ant; color = "000000"; break;
		case Speaker_WaterSkipperNest: dwSpeakerTextID = MID_AntHill; color = "000000"; break;
		case Speaker_Aumtlich: dwSpeakerTextID = MID_Zombie; color = "303030"; break;
		case Speaker_Clone: dwSpeakerTextID = MID_Clone; color = "0000A0"; break;
		case Speaker_Decoy: dwSpeakerTextID = MID_Decoy; color = "000080"; break;
		case Speaker_Wubba: dwSpeakerTextID = MID_Wubba; color = "000000"; break;
		case Speaker_Seep: dwSpeakerTextID = MID_Ghost; color = "000000"; break;
		case Speaker_Fegundo: dwSpeakerTextID = MID_Phoenix; color = "800000"; break;
		case Speaker_FegundoAshes: dwSpeakerTextID = MID_Phoenix; color = "800000"; break;
		case Speaker_MudMother: dwSpeakerTextID = MID_MudMother; color = "FF0000"; break;
		case Speaker_MudBaby: dwSpeakerTextID = MID_MudBaby; color = "FF0000"; break;
		case Speaker_GelMother: dwSpeakerTextID = MID_GelMother; color = "00FF00"; break;
		case Speaker_GelBaby: dwSpeakerTextID = MID_GelBaby; color = "00FF00"; break;
		case Speaker_Citizen: dwSpeakerTextID = MID_Citizen; color = "D0D000"; break;
		case Speaker_RockGiant: dwSpeakerTextID = MID_Splitter; color = "800000"; break;
		default: dwSpeakerTextID = MID_None; color = "FF0000"; break;
	}
	return dwSpeakerTextID;
}

inline bool bCommandHasData(const UINT eCommand)
//Returns: whether this script command has a data record attached to it
{
	switch (eCommand)
	{
		case CCharacterCommand::CC_AmbientSound:
		case CCharacterCommand::CC_AmbientSoundAt:
		case CCharacterCommand::CC_PlayVideo:
		case CCharacterCommand::CC_SetMusic:
			return true;
		default:
			return false;
	}
}

//
//Public methods.
//

//*****************************************************************************
CCharacter::CCharacter(
//Constructor.
//
//Params:
	CCurrentGame *pSetCurrentGame)   //(in)   If NULL (default) then
									//    class can only be used for
									//    accessing data, and not
									//    for game processing.
	: CPlayerDouble(M_CHARACTER, pSetCurrentGame, GROUND_AND_SHALLOW_WATER,
			SPD_CHARACTER)  //put last in process sequence so all cue events will have
					        //occurred and can be detected by the time Process() is called
	, dwScriptID(0)
	, wIdentity(UINT(-1)) //none
	, wLogicalIdentity(UINT(-1))
	, pCustomChar(NULL)
	, bVisible(false)
	, bGhostImage(false)
	, bScriptDone(false), bReplaced(false)
	, bGlobal(false)
	, bNewEntity(false)
	, bYesNoQuestion(false)
	, bPlayerTouchedMe(false)
	, wCurrentCommandIndex(0)
	, wTurnDelay(0)
	, eImperative(ScriptFlag::Vulnerable)
	, wXRel(0), wYRel(0)
	, bMovingRelative(false)
	, bSafeToPlayer(true)
	, bSwordSafeToPlayer(false)
	, bEndWhenKilled(false)
	, movementIQ(SmartOmniDirection)
	, bWaitingForCueEvent(false)
	, bIfBlock(false)
	, wLastSpeechLineNumber(0)

	, color(0), sword(NPC_DEFAULT_SWORD)
	, paramX(NO_OVERRIDE), paramY(NO_OVERRIDE), paramW(NO_OVERRIDE), paramH(NO_OVERRIDE), paramF(NO_OVERRIDE)
{
}

//*****************************************************************************
void CCharacter::ChangeHold(
//Call this when a character is being moved from an old hold to a new hold.
	CDbHold* pOldHold, //may be NULL.  This indicates character is copied within the same hold.
	CDbHold* pNewHold,
	CImportInfo& info, //(in/out) media copy info
	const bool bGetNewScriptID) //[default=true]
{
	ASSERT(pNewHold);

	if (bGetNewScriptID)
		this->dwScriptID = pNewHold->GetNewScriptID();

	for (UINT wIndex=0; wIndex<this->commands.size(); ++wIndex)
	{
		CCharacterCommand& c = this->commands[wIndex];
		//Merge script vars from a different source hold.
		if (pOldHold && pOldHold->dwHoldID != pNewHold->dwHoldID)
		{
			switch (c.command)
			{
				case CCharacterCommand::CC_WaitForVar:
				case CCharacterCommand::CC_VarSet:
				{
					//Update var refs.
					if (c.x >= (UINT)ScriptVars::FirstPredefinedVar)
						break; //predefined var IDs remain the same

					const WCHAR *pVarName = pOldHold->GetVarName(c.x);
					UINT uVarID = pNewHold->GetVarID(pVarName);
					if (!uVarID && pVarName)
					{
						//A var with this (valid) name doesn't exist in the
						//destination hold -- add one.
						uVarID = pNewHold->AddVar(pVarName);
					}
					//Update the var ID to match the ID of the var with this
					//name in the destination hold.
					c.x = uVarID;
				}
				break;
				case CCharacterCommand::CC_AmbientSound:
				case CCharacterCommand::CC_AmbientSoundAt:
				case CCharacterCommand::CC_PlayVideo:
					//Make a copy of the media object in the new hold.
					CDbData::CopyObject(info, c.w);
				break;
				default: break;
			}
		}

		//Point all data objects to the destination hold.
		if (c.pSpeech)
		{
			CDbDatum *pSound = (CDbDatum*)c.pSpeech->GetSound();
			if (pSound)
			{
				pSound->dwHoldID = pNewHold->dwHoldID;
				pSound->Update();
			}
		}
	}
}

//*****************************************************************************
UINT CCharacter::getPredefinedVar(const UINT varIndex) const
//Returns: the value of the predefined var with this relative index
{
	ASSERT(this->pCurrentGame);
	const CSwordsman& player = (this->pCurrentGame->swordsman);
	ASSERT(varIndex >= (UINT)ScriptVars::FirstPredefinedVar);
	switch (varIndex)
	{
		case ScriptVars::P_MONSTER_COLOR:
			return this->color;
		case ScriptVars::P_MONSTER_SWORD:
			return this->sword;

		//Room position.
		case ScriptVars::P_PLAYER_X:
			return player.wX;
		case ScriptVars::P_PLAYER_Y:
			return player.wY;
		case ScriptVars::P_PLAYER_O:
			return player.wO;
		case ScriptVars::P_MONSTER_X:
			return this->wX;
		case ScriptVars::P_MONSTER_Y:
			return this->wY;
		case ScriptVars::P_MONSTER_O:
			return this->wO;

		//Script parameter overrides.
		case ScriptVars::P_SCRIPT_X:
			return this->paramX;
		case ScriptVars::P_SCRIPT_Y:
			return this->paramY;
		case ScriptVars::P_SCRIPT_W:
			return this->paramW;
		case ScriptVars::P_SCRIPT_H:
			return this->paramH;
		case ScriptVars::P_SCRIPT_F:
			return this->paramF;

		default: ASSERT(!"GetVar val not supported"); return 0;
	}
}

//*****************************************************************************
void CCharacter::setPredefinedVar(
	const UINT varIndex, const UINT val,
	CCueEvents &CueEvents)
//Sets the value of the predefined var with this relative index to the specified value
{
	CCurrentGame *pGame = const_cast<CCurrentGame*>(this->pCurrentGame);

	ASSERT(varIndex >= (UINT)ScriptVars::FirstPredefinedVar);
	switch (varIndex)
	{
		case ScriptVars::P_MONSTER_COLOR:
			this->color = val;
		break;
		case ScriptVars::P_MONSTER_SWORD:
			this->sword = val;
		break;

		//Room position.
		case ScriptVars::P_PLAYER_X:
			if (pGame->pRoom->IsValidColRow(val, pGame->swordsman.wY))
				pGame->swordsman.Move(val, pGame->swordsman.wY);
		break;
		case ScriptVars::P_PLAYER_Y:
			if (pGame->pRoom->IsValidColRow(pGame->swordsman.wX, val))
				pGame->swordsman.Move(pGame->swordsman.wX, val);
		break;
		case ScriptVars::P_PLAYER_O:
			if (IsValidOrientation(val) && val != NO_ORIENTATION)
				pGame->swordsman.SetOrientation(val);
		break;
		case ScriptVars::P_MONSTER_X:
		{
			//Ensure square is valid and available.
			const CDbRoom& room = *(pGame->pRoom);
			if (room.IsValidColRow(val, this->wY) &&
					(!IsVisible() || (!room.GetMonsterAtSquare(val, this->wY) &&
					!this->pCurrentGame->IsPlayerAt(val, this->wY))))
				TeleportCharacter(val, this->wY, CueEvents);
		}
		break;
		case ScriptVars::P_MONSTER_Y:
		{
			//Ensure square is valid and available.
			const CDbRoom& room = *(pGame->pRoom);
			if (room.IsValidColRow(this->wX, val) &&
					(!IsVisible() || (!room.GetMonsterAtSquare(this->wX, val) &&
					!this->pCurrentGame->IsPlayerAt(this->wX, val))))
				TeleportCharacter(this->wX, val, CueEvents);
		}
		break;
		case ScriptVars::P_MONSTER_O:
			if (IsValidOrientation(val) && val != NO_ORIENTATION)
				this->wO = val;
		break;

		//Script parameter overrides.
		case ScriptVars::P_SCRIPT_X:
			this->paramX = val;
		break;
		case ScriptVars::P_SCRIPT_Y:
			this->paramY = val;
		break;
		case ScriptVars::P_SCRIPT_W:
			this->paramW = val;
		break;
		case ScriptVars::P_SCRIPT_H:
			this->paramH = val;
		break;
		case ScriptVars::P_SCRIPT_F:
			this->paramF = val;
		break;

		default:
			CCurrentGame *pGame = const_cast<CCurrentGame*>(this->pCurrentGame);
			pGame->ProcessCommandSetVar(varIndex, val);
		break;
	}
}

//*****************************************************************************
bool CCharacter::HasSpecialDeath() const
//Returns: whether NPC's monster type has special death behavior
{
	switch (GetResolvedIdentity())
	{
		case M_FEGUNDO:
		case M_ROCKGOLEM:
			return true;
		default: return false;
	}
}

//*****************************************************************************
void CCharacter::ReflectX(CDbRoom *pRoom)
//Update script commands to work properly when the room is reflected horizontally (about the x-axis).
{
	CMonster::ReflectX(pRoom);
	for (vector<CCharacterCommand>::iterator command = this->commands.begin();
			command != this->commands.end(); ++command)
	{
		switch (command->command)
		{
			case CCharacterCommand::CC_AppearAt:
			case CCharacterCommand::CC_MoveTo:
			case CCharacterCommand::CC_Speech:
			case CCharacterCommand::CC_ActivateItemAt:
			case CCharacterCommand::CC_WaitForDoorTo:
			case CCharacterCommand::CC_GameEffect:
				command->x = (pRoom->wRoomCols-1) - command->x;
			break;
			case CCharacterCommand::CC_WaitForRect:
			case CCharacterCommand::CC_WaitForNotRect:
			case CCharacterCommand::CC_BuildMarker:
			case CCharacterCommand::CC_WaitForItem:
				command->x = (pRoom->wRoomCols-1) - command->x - command->w;
			break;

			case CCharacterCommand::CC_FaceDirection:
			case CCharacterCommand::CC_WaitForPlayerToFace:
			case CCharacterCommand::CC_WaitForPlayerToMove:
				if (IsValidOrientation(command->x))
					command->x = nGetO(-nGetOX(command->x),nGetOY(command->x));
				else
					command->x = command->x == CMD_C ? CMD_CC : CMD_C;
			break;

			case CCharacterCommand::CC_MoveRel:
				command->x = (UINT)(-((int)command->x));
			break;
			case CCharacterCommand::CC_GenerateEntity:
				command->x = (pRoom->wRoomCols-1) - command->x;
				if (IsValidOrientation(command->w))
					command->w = nGetO(-nGetOX(command->w),nGetOY(command->w));
				else
					command->w = command->w == CMD_C ? CMD_CC : CMD_C;
			break;

			default:	break;
		}
	}
}

//*****************************************************************************
void CCharacter::ReflectY(CDbRoom *pRoom)
//Update script commands to work properly when the room is reflected vertically (about the y-axis).
{
	CMonster::ReflectY(pRoom);
	for (vector<CCharacterCommand>::iterator command = this->commands.begin();
			command != this->commands.end(); ++command)
	{
		switch (command->command)
		{
			case CCharacterCommand::CC_AppearAt:
			case CCharacterCommand::CC_MoveTo:
			case CCharacterCommand::CC_Speech:
			case CCharacterCommand::CC_ActivateItemAt:
			case CCharacterCommand::CC_WaitForDoorTo:
			case CCharacterCommand::CC_GameEffect:
				command->y = (pRoom->wRoomRows-1) - command->y;
			break;
			case CCharacterCommand::CC_WaitForRect:
			case CCharacterCommand::CC_WaitForNotRect:
			case CCharacterCommand::CC_BuildMarker:
			case CCharacterCommand::CC_WaitForItem:
				command->y = (pRoom->wRoomRows-1) - command->y - command->h;
			break;

			case CCharacterCommand::CC_FaceDirection:
			case CCharacterCommand::CC_WaitForPlayerToFace:
			case CCharacterCommand::CC_WaitForPlayerToMove:
				if (IsValidOrientation(command->x))
					command->x = nGetO(nGetOX(command->x),-nGetOY(command->x));
				else
					command->x = command->x == CMD_C ? CMD_CC : CMD_C;
			break;

			case CCharacterCommand::CC_GenerateEntity:
				command->y = (pRoom->wRoomRows-1) - command->y;
				if (IsValidOrientation(command->w))
					command->w = nGetO(nGetOX(command->w),-nGetOY(command->w));
				else
					command->w = command->w == CMD_C ? CMD_CC : CMD_C;
			break;

			case CCharacterCommand::CC_MoveRel:
				command->y = (UINT)(-((int)command->y));
			break;
			default:	break;
		}
	}
}

//*****************************************************************************
CMonster* CCharacter::Replicate() const
//Make duplicate objects for the command vector.
{
	CCharacter *pCharacter = new CCharacter(*this);
	pCharacter->dwScriptID = this->dwScriptID; //must be reassigned correctly later
	for (vector<CCharacterCommand>::iterator command = pCharacter->commands.begin();
			command != pCharacter->commands.end(); ++command)
	{
		if (command->pSpeech)
		{
			//Duplicate speech/sound data records for saving to DB.
			CDbSpeech *pSpeech = new CDbSpeech(*(command->pSpeech), true);
			ASSERT(pSpeech);
			const CDbDatum *pSound = command->pSpeech->GetSound();
			if (pSound)
			{
				CDbDatum *pNewData = new CDbDatum(*pSound, true);
				ASSERT(pNewData);
				pSpeech->SetSound(pNewData);
			}
			delete command->pSpeech;
			command->pSpeech = pSpeech;
		}
	}
	return pCharacter;
}

//*****************************************************************************
bool CCharacter::ResetLevelExits()
//Returns: whether any level exit commands were reset
{
	bool bUpdated = false;
	for (vector<CCharacterCommand>::iterator command = this->commands.begin();
			command != this->commands.end(); ++command)
	{
		if (command->command == CCharacterCommand::CC_LevelEntrance)
		{
			command->x = 0;
			command->y = 0;
			bUpdated = true;
		}
	}
	return bUpdated;
}

//*****************************************************************************
bool CCharacter::OnStabbed(CCueEvents &CueEvents, const UINT /*wX*/, const UINT /*wY*/)
//Returns: whether character was killed
{
	if (this->eImperative == ScriptFlag::Invulnerable)
		return false;

	CueEvents.Add(CID_MonsterDiedFromStab, this);
	return true;
}

//*****************************************************************************
bool CCharacter::IsValidExpression(
//Parses an expression to determine its syntactical validity.
//See ::parseExpression for supported grammar
//
//Returns: whether the expression is valid
//
//Params:
	const WCHAR *pwStr, UINT& index, CDbHold *pHold,
	const bool bExpectCloseParen) //[default=false] whether a close paren should mark the end of this (nested) expression
{
	ASSERT(pwStr);
	ASSERT(pHold);
	SKIP_WHITESPACE(pwStr, index);

	if (pwStr[index] == W_t('+') || pwStr[index] == W_t('-'))
		++index;

	if (!IsValidTerm(pwStr, index, pHold))
		return false;

	SKIP_WHITESPACE(pwStr, index);
	while (pwStr[index]!=0)
	{
		//Parse another term.
		if (pwStr[index] == W_t('+') || pwStr[index] == W_t('-'))
			++index;
		else if (bExpectCloseParen && pwStr[index] == W_t(')')) //closing nested expression
			return true; //caller will parse the close paren
		else
			return false; //invalid symbol between terms

		if (!IsValidTerm(pwStr, index, pHold))
			return false;
	}

	return true;
}

//*****************************************************************************
bool CCharacter::IsValidTerm(const WCHAR *pwStr, UINT& index, CDbHold *pHold)
//Returns: whether a term in an expression is valid
{
	if (!IsValidFactor(pwStr, index, pHold))
		return false;

	while (pwStr[index]!=0)
	{
		//Parse another term.
		SKIP_WHITESPACE(pwStr, index);

		if (pwStr[index] == W_t('*') || pwStr[index] == W_t('/') || pwStr[index] == W_t('%'))
			++index;
		else
			//no more factors in this term
			return true;

		if (!IsValidFactor(pwStr, index, pHold))
			return false;
	}

	return true;
}

//*****************************************************************************
bool CCharacter::IsValidFactor(const WCHAR *pwStr, UINT& index, CDbHold *pHold)
//Returns: whether a factor in an expression is valid
{
	SKIP_WHITESPACE(pwStr, index);

	//A nested expression?
	if (pwStr[index] == W_t('('))
	{
		++index;
		if (!IsValidExpression(pwStr, index, pHold, true)) //recursive call
			return false;
		if (pwStr[index] != W_t(')')) //should be parsing the close parenthesis at this point
			return false; //missing close parenthesis

		++index;
		return true;
	}

	//Number?
	if (iswdigit(pwStr[index]))
	{
		//Parse past digits.
		++index;
		while (iswdigit(pwStr[index]))
			++index;

		if (iswalpha(pwStr[index])) //i.e. of form <digits><alphas>
			return false; //invalid var name

		return true;
	}

	//Variable identifier?
	if (pwStr[index] == W_t('_') || iswalpha(pwStr[index])) //valid first char
	{
		//Find spot where var identifier ends.
		int endIndex = index + 1;
		int spcTrail = 0;
		while (CDbHold::IsVarCharValid(pwStr[endIndex]))
		{
			if (pwStr[endIndex] == W_t(' '))
				++spcTrail;
			else
				spcTrail = 0;
			++endIndex;
		}

		WSTRING wVarName(pwStr + index, endIndex - index - spcTrail);
		index = endIndex;

		//Is it a predefined var?
		const ScriptVars::Predefined eVar = ScriptVars::parsePredefinedVar(wVarName);
		if (eVar != ScriptVars::P_NoVar)
			return true;

		//Is it a hold var?
		if (pHold->GetVarID(wVarName.c_str()))
			return true;

		//Unrecognized identifier.
		return false;
	}

	//Invalid identifier
	return false;
}

//*****************************************************************************
int CCharacter::parseExpression(
//Parse and evaluate a simple nested expression for the grammar
//
// expression = ["+"|"-"] term {("+"|"-") term}
//
// term = factor {("*"|"/"|"%") factor}
//
// factor = var | number | "(" expression ")"
//
//Params:
	const WCHAR *pwStr, UINT& index, CCurrentGame *pGame, CCharacter *pNPC, //[default=NULL]
	const bool bExpectCloseParen) //[default=false] whether a close paren should mark the end of this (nested) expression
{
	ASSERT(pwStr);
	ASSERT(pGame);
	SKIP_WHITESPACE(pwStr, index);

	bool bAdd = true; //otherwise subtract
	if (pwStr[index] == W_t('+'))
		++index;
	else if (pwStr[index] == W_t('-'))
	{
		bAdd = false;
		++index;
	}

	const int term = parseTerm(pwStr, index, pGame, pNPC);
	int val = (bAdd ? term : -term);

	SKIP_WHITESPACE(pwStr, index);
	while (pwStr[index]!=0)
	{
		//Parse another term.
		if (pwStr[index] == W_t('+'))
		{
			bAdd = true;
			++index;
		}
		else if (pwStr[index] == W_t('-'))
		{
			bAdd = false;
			++index;
		}
		else if (bExpectCloseParen && pwStr[index] == W_t(')')) //closing nested expression
			return val; //caller will parse the close paren
		else
		{
			//parse error -- return the current value
			CFiles f;
			string str;
			UnicodeToAscii(pwStr + index, str);
			str += ": Parse error (bad symbol)";
			f.AppendErrorLog(str.c_str());
			return val;
		}

		const int term = parseTerm(pwStr, index, pGame, pNPC);
		val += (bAdd ? term : -term);
	}

	return val;
}

//*****************************************************************************
int CCharacter::parseTerm(const WCHAR *pwStr, UINT& index, CCurrentGame *pGame, CCharacter *pNPC)
//Parse and evaluate a term in an expression.
{
	int val = parseFactor(pwStr, index, pGame, pNPC);

	while (pwStr[index]!=0)
	{
		//Parse another term.
		SKIP_WHITESPACE(pwStr, index);

		bool bMultiply;
		bool bMod = false;
		if (pwStr[index] == W_t('*'))
		{
			bMultiply = true;
			++index;
		}
		else if (pwStr[index] == W_t('/'))
		{
			bMultiply = false;
			++index;
		}
		else if (pwStr[index] == W_t('%'))
		{
			bMod = true;
			++index;
		} else {
			//no more factors in this term -- return result
			return val;
		}

		const int factor = parseFactor(pwStr, index, pGame, pNPC);
		if (bMod)
		{
			if (factor) //no mod by zero
				val = val % factor;
		}
		else if (bMultiply)
			val *= factor;
		else {
			if (factor) //no divide by zero
				val /= factor;
		}
	}

	return val;
}

//*****************************************************************************
int CCharacter::parseFactor(const WCHAR *pwStr, UINT& index, CCurrentGame *pGame, CCharacter *pNPC)
//Parse and evaluate a term in an expression.
{
	SKIP_WHITESPACE(pwStr, index);

	//A nested expression?
	if (pwStr[index] == W_t('('))
	{
		++index;
		int val = parseExpression(pwStr, index, pGame, pNPC, true); //recursive call
		SKIP_WHITESPACE(pwStr, index);
		if (pwStr[index] == W_t(')'))
			++index;
		else
		{
			//parse error -- return the current value
			CFiles f;
			string str;
			UnicodeToAscii(pwStr, str);
			str += ": Parse error (missing close parenthesis)";
			f.AppendErrorLog(str.c_str());
		}
		return val;
	}

	//Number?
	if (iswdigit(pwStr[index]))
	{
		int val = _Wtoi(pwStr + index);

		//Parse past digits.
		++index;
		while (iswdigit(pwStr[index]))
			++index;

		if (iswalpha(pwStr[index])) //i.e. of form <digits><alphas>
		{
			//Invalid var name -- skip to end of it and return zero value.
			while (CDbHold::IsVarCharValid(pwStr[index]))
				++index;

			CFiles f;
			string str;
			UnicodeToAscii(pwStr, str);
			str += ": Parse error (invalid var name)";
			f.AppendErrorLog(str.c_str());

			return 0;
		}

		return val;
	}

	//Variable identifier?
	if (pwStr[index] == W_t('_') || iswalpha(pwStr[index])) //valid first char
	{
		//Find spot where var identifier ends.
		int endIndex = index + 1;
		int spcTrail = 0;
		while (CDbHold::IsVarCharValid(pwStr[endIndex]))
		{
			if (pwStr[endIndex] == W_t(' '))
				++spcTrail;
			else
				spcTrail = 0;
			++endIndex;
		}

		WSTRING wVarName(pwStr + index, endIndex - index - spcTrail);
		index = endIndex;

		//Is it a predefined var?
		int val = 0;
		const ScriptVars::Predefined eVar = ScriptVars::parsePredefinedVar(wVarName);
		if (eVar != ScriptVars::P_NoVar)
		{
			if (pNPC)
				val = int(pNPC->getPredefinedVar(eVar));
			else
				val = pGame->getVar(eVar);
		} else {
			//Is it a local hold var?
			char *varName = pGame->pHold->getVarAccessToken(wVarName.c_str());
			const UNPACKEDVARTYPE vType = pGame->stats.GetVarType(varName);
			const bool bValidInt = vType == UVT_int || vType == UVT_uint || vType == UVT_unknown;
			if (bValidInt)
				val = pGame->stats.GetVar(varName, (int)0);
			//else: unrecognized identifier -- just return a zero value
		}
		return val;
	}

	//Invalid identifier
	CFiles f;
	string str;
	UnicodeToAscii(pwStr + index, str);
	str += ": Parse error (invalid var name)";
	f.AppendErrorLog(str.c_str());

	return 0;
}

//*****************************************************************************
void CCharacter::Process(
//Process a character for movement.
//
//Params:
	const int nLastCommand,   //(in) Last player command.
	CCueEvents &CueEvents)  //(in/out) Accepts pointer to a cues object that will be populated
							//with codes indicating events that happened that may correspond to
							//sound or graphical effects.
{
//If executing a command normally, calling this will end the character's turn.
//If the command was being evaluated as an If conditional, continue processing the next command.
#define STOP_COMMAND {if (!this->wJumpLabel) goto Finish; this->wJumpLabel=0; bProcessNextCommand=true; break;}

//Call this one instead if evaluating the condition took a turn and no more commands should be executed now.
#define STOP_DONECOMMAND {if (!this->wJumpLabel) goto Finish; this->wJumpLabel=0; break;}

	CCurrentGame *pGame = const_cast<CCurrentGame*>(this->pCurrentGame);
	CDbRoom& room = *(pGame->pRoom);
	CSwordsman& player = pGame->swordsman;

	//Quick exit when done.
	if (this->wCurrentCommandIndex >= this->commands.size())
		goto Finish;

	//Only character monsters taking up a single tile are implemented.
	ASSERT(!bIsSerpent(GetResolvedIdentity()));

	{ //Wrap variables initialized within jump, to make g++ happy

	//On player room entrance, as a preprocessing step, execute commands that
	//don't require a turn to execute (e.g., visibility, gotos, etc.).
	bool bExecuteNoMoveCommands = pGame->ExecutingNoMoveCommands();

	this->bWaitingForCueEvent = this->bIfBlock = false;
	this->wSwordMovement = NO_ORIENTATION;
	this->wJumpLabel = 0;

	//Simple infinite loop checking.
	UINT wTurnCount = 0, wVarSets = 0;
	UINT px, py, pw, ph, pflags;  //command parameters

	bool bProcessNextCommand;     //true if next command should be initiated this turn
	do {
		if (this->wTurnDelay)
		{
			//Skip turn.
			--this->wTurnDelay;
			pGame->bContinueCutScene = true;
			goto Finish;
		}

		//Stop if no more commands to play.
		if (this->wCurrentCommandIndex >= this->commands.size())
			goto Finish;

		bProcessNextCommand = false;

		//While script commands are still executing, any cut scene playing may continue.
		pGame->bContinueCutScene = true;

		CCharacterCommand& command = this->commands[this->wCurrentCommandIndex];
		switch (command.command)
		{
			case CCharacterCommand::CC_Appear:
			{
				//Appear at current square.
				bProcessNextCommand = true;

				if (this->bVisible) break; //already in room
				const UINT identity = GetResolvedIdentity();
				if (identity >= CHARACTER_TYPES)
					break;	//nothing to show -- can't appear

				//Ensure square is available before appearing.
				ASSERT(room.IsValidColRow(this->wX, this->wY));
				if (room.GetMonsterAtSquare(this->wX, this->wY) != NULL ||
						pGame->IsPlayerAt(this->wX, this->wY) ||
						room.IsSwordAt(this->wX, this->wY))
					STOP_COMMAND;

				//Place character on starting square.
				this->bVisible = true;
				SetSwordSheathed();
				ASSERT(!room.pMonsterSquares[room.ARRAYINDEX(this->wX,this->wY)]);
				room.pMonsterSquares[room.ARRAYINDEX(this->wX,this->wY)] = this;

				//Check for stepping on pressure plate.
				if (room.GetOSquare(this->wX, this->wY) == T_PRESSPLATE && !IsFlying())
					room.ActivateOrb(this->wX, this->wY, CueEvents, OAT_PressurePlate);

				if (this->eImperative == ScriptFlag::RequiredToConquer)
				{
					++room.wMonsterCount;
					CueEvents.Add(CID_NPCTypeChange);
				}
				bExecuteNoMoveCommands = true;	//allow executing commands that don't require moves immediately
			}
			break;
			case CCharacterCommand::CC_AppearAt:
			{
				//Appear at square (x,y).
				bProcessNextCommand = true;

				if (this->bVisible) break; //already in room
				const UINT identity = GetResolvedIdentity();
				if (identity >= CHARACTER_TYPES)
					break;	//nothing to show -- can't appear

				//Ensure square is available before appearing.
				getCommandXY(command, px, py);
				if (!room.IsValidColRow(px,py) ||
						room.GetMonsterAtSquare(px, py) != NULL ||
						pGame->IsPlayerAt(px, py) ||
						room.IsSwordAt(px, py))
					STOP_COMMAND;

				//Place character on starting square.
				this->bVisible = true;
				this->wPrevX = this->wX = px;
				this->wPrevY = this->wY = py;
				ASSERT(!room.pMonsterSquares[room.ARRAYINDEX(this->wX,this->wY)]);
				room.pMonsterSquares[room.ARRAYINDEX(this->wX,this->wY)] = this;
				SetSwordSheathed();

				//Check for stepping on pressure plate.
				if (room.GetOSquare(this->wX, this->wY) == T_PRESSPLATE && !IsFlying())
					room.ActivateOrb(this->wX, this->wY, CueEvents, OAT_PressurePlate);

				if (this->eImperative == ScriptFlag::RequiredToConquer)
				{
					++room.wMonsterCount;
					CueEvents.Add(CID_NPCTypeChange);
				}
				bExecuteNoMoveCommands = true;	//allow executing commands that don't require moves immediately
			}
			break;
			case CCharacterCommand::CC_Disappear:
			{
				//Remove character from room (i.e. remove from view and action,
				//but script keeps running).
				bProcessNextCommand = true;

				if (!this->bVisible) break; //not in room

				if (this->eImperative == ScriptFlag::RequiredToConquer)
				{
					room.DecMonsterCount();
					CueEvents.Add(CID_NPCTypeChange);
				}

				Disappear();
				bExecuteNoMoveCommands = true;	//allow executing commands that don't require moves immediately
			}
			break;

			case CCharacterCommand::CC_MoveTo:
			{
				//Move to square (x,y) or target set in flags.
				//If w is set, then forbid turning while moving.
				//If h is set, then take only a single step before advancing to next command.
				//However, if the NPC is not visible, then a coord change to the destination occurs

				if (bExecuteNoMoveCommands && IsVisible())
				{
					//If the move is used as an If condition, back up to the If
					//so the check is performed for real next turn.
					if (this->wJumpLabel)
						--this->wCurrentCommandIndex;
					goto Finish;
				}

				//Move toward square (x,y) or target set by flags.
				UINT wDestX, wDestY;
				getCommandParams(command, px, py, pw, ph, pflags);
				wDestX = px;
				wDestY = py;
				if (!room.IsValidColRow(px,py))
				{
					bProcessNextCommand = true;
					break;
				}

				if (pflags)
				{
					CCoord *pDest = NULL;
					if ((pflags & ScriptFlag::PLAYER) != 0)
						pDest = (CCoord*)&player;
					else if ((pflags & ScriptFlag::HALPH) != 0)
					{
						if (!(pDest = room.GetMonsterOfType(M_HALPH)))
							pDest = room.GetMonsterOfType(M_HALPH2);
					}
					else if ((pflags & ScriptFlag::MONSTER) != 0)
						pDest = room.pFirstMonster;
					else if ((pflags & ScriptFlag::NPC) != 0)
						pDest = room.GetMonsterOfType(M_CHARACTER);
					else if ((pflags & ScriptFlag::PDOUBLE) != 0)
					{
						if (!(pDest = room.GetMonsterOfType(M_MIMIC)))
							if (!(pDest = room.GetMonsterOfType(M_DECOY)))
								pDest = room.GetMonsterOfType(M_CLONE);
					}
					else if ((pflags & ScriptFlag::SELF) != 0)
						break; //always at this position by definition
					else if ((pflags & ScriptFlag::SLAYER) != 0)
					{
						if (!(pDest = room.GetMonsterOfType(M_SLAYER)))
							pDest = room.GetMonsterOfType(M_SLAYER2);
					}
					else if ((pflags & ScriptFlag::BEETHRO) != 0)
					{
						if (bIsSmitemaster(player.wAppearance))
							pDest = (CCoord*)&player;
						else
							pDest = room.GetNPCBeethro();
					}
					else if ((pflags & ScriptFlag::STALWART) != 0)
					{
						if (!(pDest = room.GetMonsterOfType(M_STALWART)))
							pDest = room.GetMonsterOfType(M_STALWART2);
					}
					if (!pDest)
						STOP_COMMAND;

					wDestX = pDest->wX;
					wDestY = pDest->wY;
				}

				//When not visible, go to destination instantly.
				if (!IsVisible())
				{
					CCoordSet coords(this->wX, this->wY);
					coords.insert(wDestX, wDestY);

					this->wX = wDestX;
					this->wY = wDestY;

					room.Plot(coords); //update room
				}

				if (this->wX == wDestX && this->wY == wDestY)
				{
					//At destination -- done with this command.
					bProcessNextCommand = true;
					break;
				}
				int dxFirst, dyFirst, dx, dy;
				switch (this->movementIQ)
				{
					case DirectOnly:
						GetBeelineMovementDumb(wDestX, wDestY, dxFirst, dyFirst, dx, dy);
					break;
					default:
					case SmartOmniDirection:
						GetBeelineMovementSmart(wDestX, wDestY, dxFirst, dyFirst,
							dx, dy, true);
					break;
				}
				if (!dx && !dy)
				{
					if (ph)
					{
						//If single step, then advance to next command when can't move
						if (!pw)  //allow turning to face the intended direction
						{
							if (!bEntityHasSword(GetResolvedIdentity())) //only if w/o a sword
								SetOrientation(dxFirst,dyFirst);
						}
						break;
					}
					STOP_COMMAND;
				}

				//If monster type has a sword, then it must rotate slowly, and
				//it can't move on the same turn it is rotating.
				const bool bAllowTurning = !pw;
				if (bAllowTurning)
					if (bEntityHasSword(GetResolvedIdentity()) && HasSword())
					{
						const UINT wOldO = this->wO;
						if (MakeSlowTurn(nGetO(dx, dy))) //toward desired direction
						{
							this->wSwordMovement = CSwordsman::GetSwordMovement(
									this->wO == nNextCO(wOldO) ? CMD_C : CMD_CC, this->wO);
							if (ph)  //single step?
								break;
							STOP_DONECOMMAND;
						}
					}

				//If moving toward a target entity, the NPC can't step on it
				//unless it's the player and the NPC can kill him,
				//so don't try to move if already adjacent.
				if (!(pflags && nDist(this->wX, this->wY, wDestX, wDestY) == 1 &&
				(!(pflags & ScriptFlag::PLAYER) || this->bSafeToPlayer)))
					MoveCharacter(dx, dy, bAllowTurning, CueEvents);

				//Repeat command until arrived at destination.
				if (ph)  //single step?
					break;
				if (this->wX != px || this->wY != py)
					STOP_DONECOMMAND;
			}
			break;
			case CCharacterCommand::CC_MoveRel:
			{
				//Move (x,y) relative to current position. If w is set, then forbid turning while moving.
				//If h is set, then take only a single step before advancing to next command.
				//However, if the NPC is not visible, then a coord change to the destination occurs
				if (bExecuteNoMoveCommands && IsVisible())
				{
					//If the move is used as an If condition, back up to the If
					//so the check is performed for real next turn.
					if (this->wJumpLabel)
						--this->wCurrentCommandIndex;
					goto Finish;
				}

				//Move relative (x,y).
				getCommandRect(command, px, py, pw, ph);
				if (!this->bMovingRelative)
				{
					//Get relative vector first time this very command is invoked.
					if (!px && !py)
					{
						//Relative movement is zero -- nothing to do.
						bProcessNextCommand = true;
						break;
					}
					int nXDest = (int)this->wX + (int)px;
					int nYDest = (int)this->wY + (int)py;
					if (nXDest < 0)
						nXDest = 0;
					else if (nXDest >= (int)room.wRoomCols)
						nXDest = room.wRoomCols - 1;
					if (nYDest < 0)
						nYDest = 0;
					else if (nYDest >= (int)room.wRoomRows)
						nYDest = room.wRoomRows - 1;

					//When not visible, go to destination instantly.
					if (!IsVisible())
					{
						CCoordSet coords(this->wX, this->wY);
						coords.insert(nXDest, nYDest);

						this->wX = nXDest;
						this->wY = nYDest;

						room.Plot(coords); //update room
						bProcessNextCommand = true;
						break;
					}

					this->wXRel = nXDest;
					this->wYRel = nYDest;
					this->bMovingRelative = true;
				}
				int dxFirst, dyFirst, dx, dy;
				switch (this->movementIQ)
				{
					case DirectOnly:
						GetBeelineMovementDumb(this->wXRel, this->wYRel,
							dxFirst, dyFirst,	dx, dy);
					break;
					default:
					case SmartOmniDirection:
						GetBeelineMovementSmart(this->wXRel, this->wYRel,
							dxFirst, dyFirst, dx, dy, true);
					break;
				}
				if (!dx && !dy)
				{
					if (ph && IsVisible())
					{
						//If single step, then advance to next command when can't move
						if (!pw)  //allow turning to face the intended direction
						{
							if (!bEntityHasSword(GetResolvedIdentity())) //only if w/o a sword
								SetOrientation(dxFirst,dyFirst);
						}
						this->bMovingRelative = false;
						break;
					}
					STOP_COMMAND;
				}

				//If monster type has a sword, then it must rotate slowly, and
				//it can't move on the same turn it is rotating.
				const bool bAllowTurning = !pw;
				if (bAllowTurning)
					if (bEntityHasSword(GetResolvedIdentity()) && HasSword())
					{
						const UINT wOldO = this->wO;
						if (MakeSlowTurn(nGetO(dx, dy))) //toward desired direction
						{
							this->wSwordMovement = CSwordsman::GetSwordMovement(
									this->wO == nNextCO(wOldO) ? CMD_C : CMD_CC, this->wO);
							if (ph)  //single step?
							{
								this->bMovingRelative = false;
								break;
							}
							STOP_DONECOMMAND;
						}
					}

				MoveCharacter(dx, dy, bAllowTurning, CueEvents);

				//Repeat command until arrived at destination.
				if (ph)  //single step?
				{
					this->bMovingRelative = false;
					break;
				}
				if (this->wX != this->wXRel || this->wY != this->wYRel)
					STOP_DONECOMMAND;

				//Arrived.
				this->bMovingRelative = false;
			}
			break;
			case CCharacterCommand::CC_FaceDirection:
			{
				//Turn to face indicated direction.
				if (bExecuteNoMoveCommands && this->bVisible)
				{
					//If the move is used as an If condition, back up to the If
					//so the check is performed for real next turn.
					if (this->wJumpLabel)
						--this->wCurrentCommandIndex;
					goto Finish;
				}

				const UINT wOldO = this->wO;
				getCommandX(command, px);
				switch (px)
				{
					case CMD_C: this->wO = nNextCO(this->wO);
						this->wSwordMovement = CSwordsman::GetSwordMovement(CMD_C, this->wO);
					break;
					case CMD_CC: this->wO = nNextCCO(this->wO);
						this->wSwordMovement = CSwordsman::GetSwordMovement(CMD_CC, this->wO);
					break;
					default:
						if (!IsValidOrientation(px) && px != NO_ORIENTATION)
							break; //not valid -- do nothing
						this->wO = px;
					break;
				}
				SetSwordSheathed();
				if (!this->bVisible || //turning doesn't take time when not in room
						wOldO == this->wO) //already facing this way
					bProcessNextCommand = true;
			}
			break;
			case CCharacterCommand::CC_ActivateItemAt:
			{
				getCommandXY(command, px, py);
				if (!room.IsValidColRow(px, py))
					break;

				//Activate item at (x,y).  Works only for the following items.
				const UINT lightVal = room.tileLights.GetAt(px, py);
				if (bIsWallLightValue(lightVal))
				{
					//Toggle wall light.  Doesn't expend a turn.
					room.tileLights.Add(px, py, WALL_LIGHT + ((lightVal + LIGHT_OFF) % 256));
					CueEvents.Add(CID_LightToggled);
					bProcessNextCommand = true;
				}
				switch (room.GetBottomTSquare(px, py))
				{
					case T_LIGHT:
						//Toggle a light.  Doesn't expend a turn.
						room.ToggleLight(px, py, CueEvents);
						bProcessNextCommand = true;
					break;
					case T_TOKEN:
						//Activate a token.  Doesn't expend a turn.
						room.ActivateToken(CueEvents, px, py);
						bProcessNextCommand = true;
					break;

					case T_BOMB:
						if (pGame->wTurnNo > 0) //not on room entrance, since the player could die immediately
						{
							//Explode a bomb immediately.  Doesn't expend a turn.
							CCoordStack bomb(px, py);
							room.BombExplode(CueEvents, bomb);
							bProcessNextCommand = true;
						} else {
							//Pause script until after first turn to explode the bomb.
							STOP_COMMAND;
						}
					break;
					case T_FUSE:
						//Light fuse.  Doesn't expend a turn.
						if (room.GetTSquare(px, py) != T_MIRROR)
							room.LightFuse(CueEvents, px, py,
								//don't cause waiting another turn when executed on room entrance
								pGame->wTurnNo > 0);
						bProcessNextCommand = true;
					break;

					case T_ORB:
						//Activate orb.
						if (bExecuteNoMoveCommands) return;
						room.ActivateOrb(px, py, CueEvents, OAT_ScriptOrb);
					break;

					default:
						if (room.GetOSquare(px, py) == T_PRESSPLATE)
						{
							//Activate pressure plate.
							if (bExecuteNoMoveCommands) return;

							//Determine tile location of pressure plate data object.
							UINT wX = px, wY = py;
							COrbData *pData = room.GetPressurePlateAtCoords(wX, wY);
							if (pData)
							{
								//Activate the pressure plate from this tile.
								wX = pData->wX;
								wY = pData->wY;
							}
							room.ActivateOrb(wX, wY, CueEvents, OAT_ScriptPlate);
						} else {
							//No item to active on this tile.  Just continue script.
							bProcessNextCommand = true;
						}
					break;
				}
			}
			break;

			case CCharacterCommand::CC_Wait:
			{
				//Set to wait for X turns.
				getCommandX(command, px);
				if (px)
				{
					//If a move can't be made right now, then don't register
					//the delay until next turn to avoid an additional turn delay
					//in loops after it completes.
					if (bExecuteNoMoveCommands)
						STOP_COMMAND;

					this->wTurnDelay = px;
					--this->wTurnDelay; //This turn counts as one of them.
				}
				//else: when a 0 wait is specified, stop executing commands until the next turn
			}
			break;
			case CCharacterCommand::CC_WaitForCueEvent:
			{
				//Wait for cue event X to fire.
				const CUEEVENT_ID cid = static_cast<CUEEVENT_ID>(command.x);
				if (!CueEvents.HasOccurred(cid))
				{
					//If NPC is waiting, try to catch event at end of game turn in CheckForCueEvent().
					//!!NOTE: This won't work for conditional "If <late cue event>".
					if (!this->wJumpLabel)
						this->bWaitingForCueEvent = true;
					STOP_COMMAND;
				}
				bProcessNextCommand = true;
			}
			break;

			case CCharacterCommand::CC_WaitForRect:
			case CCharacterCommand::CC_WaitForNotRect:
			{
				//Wait until a specified entity is in rect (x,y,w,h).
				// -OR-
				//Wait until NONE of the specified entities are in rect (x,y,w,h).
				//
				//Note that width and height are zero-indexed.
				bool bFound = false;
				getCommandParams(command, px, py, pw, ph, pflags);
				if (!room.IsValidColRow(px, py) || !room.IsValidColRow(px+pw, py+ph))
					STOP_COMMAND;

				if (!bFound && (!pflags || (pflags & ScriptFlag::PLAYER) != 0))
				{
					//Check for player by default if no flags are selected.
					if (player.IsInRoom() &&
							player.wX >= px && player.wX <= px + pw &&
							player.wY >= py && player.wY <= py + ph)
						bFound = true;
				}
				if (!bFound && (pflags & ScriptFlag::HALPH) != 0)
				{
					if ((player.wAppearance == M_HALPH || player.wAppearance == M_HALPH2) &&
							player.wX >= px && player.wX <= px + pw &&
							player.wY >= py && player.wY <= py + ph)
						bFound = true;
					else if (room.IsMonsterInRectOfType(px, py,
							px + pw, py + ph, M_HALPH, true))
						bFound = true;
					else if (room.IsMonsterInRectOfType(px, py,
							px + pw, py + ph, M_HALPH2, true))
						bFound = true;
				}
				if (!bFound && (pflags & ScriptFlag::MONSTER) != 0)
				{
					//excludes player doubles, friendly enemies, and NPCs
					if (!bIsHuman(player.wAppearance) &&
							player.wX >= px && player.wX <= px + pw &&
							player.wY >= py && player.wY <= py + ph)
						bFound = true;
					else if (room.IsMonsterInRect(px, py,
							px + pw, py + ph))
						bFound = true;
				}
				if (!bFound && (pflags & ScriptFlag::NPC) != 0)
				{
					//visible characters only
					if (room.IsMonsterInRectOfType(px, py,
							px + pw, py + ph, M_CHARACTER))
						bFound = true;
				}
				if (!bFound && (pflags & ScriptFlag::PDOUBLE) != 0)
				{
					//All player double types
					if (bIsBeethroDouble(player.wAppearance) && !bIsSmitemaster(player.wAppearance) &&
							player.wX >= px && player.wX <= px + pw &&
							player.wY >= py && player.wY <= py + ph)
						bFound = true;
					else if (room.IsMonsterInRectOfType(px, py,
							px + pw, py + ph, M_MIMIC))
						bFound = true;
					else if (room.IsMonsterInRectOfType(px, py,
							px + pw, py + ph, M_DECOY))
						bFound = true;
					else if (room.IsMonsterInRectOfType(px, py,
							px + pw, py + ph, M_CLONE))
						bFound = true;
				}
				if (!bFound && (pflags & ScriptFlag::SELF) != 0)
				{
					if (this->wX >= px && this->wX <= px + pw &&
							this->wY >= py && this->wY <= py + ph)
						bFound = true;
				}
				if (!bFound && (pflags & ScriptFlag::SLAYER) != 0)
				{
					if ((player.wAppearance == M_SLAYER || player.wAppearance == M_SLAYER2) &&
							player.wX >= px && player.wX <= px + pw &&
							player.wY >= py && player.wY <= py + ph)
						bFound = true;
					else if (room.IsMonsterInRectOfType(px, py,
							px + pw, py + ph, M_SLAYER, true))
						bFound = true;
					else if (room.IsMonsterInRectOfType(px, py,
							px + pw, py + ph, M_SLAYER2, true))
						bFound = true;
				}
				if (!bFound && (pflags & ScriptFlag::BEETHRO) != 0)
				{
					//Check for Beethro (can detect NPC Beethros).
					if (bIsSmitemaster(player.wAppearance))
					{
						if (player.wX >= px && player.wX <= px + pw &&
								player.wY >= py && player.wY <= py + ph)
							bFound = true;
					}
					CMonster *pNPCBeethro = pGame->pRoom->GetNPCBeethro();
					if (pNPCBeethro)
					{
						const UINT wSX = pNPCBeethro->wX;
						const UINT wSY = pNPCBeethro->wY;
						if (wSX >= px && wSX <= px + pw &&
								wSY >= py && wSY <= py + ph)
							bFound = true;
					}
				}
				if (!bFound && (pflags & ScriptFlag::STALWART) != 0)
				{
					if (bIsStalwart(player.wAppearance) &&
							player.wX >= px && player.wX <= px + pw &&
							player.wY >= py && player.wY <= py + ph)
						bFound = true;
					else if (room.IsMonsterInRectOfType(px, py,
							px + pw, py + ph, M_STALWART, true))
						bFound = true;
					else if (room.IsMonsterInRectOfType(px, py,
							px + pw, py + ph, M_STALWART2, true))
						bFound = true;
				}

				if ((command.command == CCharacterCommand::CC_WaitForRect && !bFound) ||
					 (command.command == CCharacterCommand::CC_WaitForNotRect && bFound))
					STOP_COMMAND;
				bProcessNextCommand = true;
			}
			break;

			case CCharacterCommand::CC_WaitForDoorTo:
			{
				//Wait for door at (x,y) to (w=close/open).
				getCommandXY(command, px, py);
				const UINT wTile = room.GetOSquare(px, py);
				if (command.w==(UINT)OA_CLOSE && !bIsDoor(wTile))
					STOP_COMMAND;  //door hasn't closed yet
				if (command.w==(UINT)OA_OPEN && bIsDoor(wTile))
					STOP_COMMAND;  //door hasn't opened yet
				bProcessNextCommand = true;
			}
			break;
			case CCharacterCommand::CC_WaitForTurn:
			{
				//Wait until play reaches turn X.
				getCommandX(command, px);
				if (pGame->wSpawnCycleCount < px)
					STOP_COMMAND;
				bProcessNextCommand = true;
			}
			break;
			case CCharacterCommand::CC_WaitForCleanRoom:
			{
				//Wait until room has been conquered (i.e. green doors have opened).
				if (!room.bGreenDoorsOpened)
					STOP_COMMAND;
				bProcessNextCommand = true;
			}
			break;
			case CCharacterCommand::CC_WaitForPlayerToFace:
			{
				//Wait until player faces orientation X.
				if (!player.IsInRoom())
					STOP_COMMAND;

				getCommandX(command, px);
				switch (px)
				{
					case CMD_C:
						if (player.wO != nNextCO(player.wPrevO))
							STOP_COMMAND;
						break;
					case CMD_CC:
						if (player.wO != nNextCCO(player.wPrevO))
							STOP_COMMAND;
						break;
					default:
						if (player.wO != px)
							STOP_COMMAND;
					break;
				}
				bProcessNextCommand = true;
			}
			break;
			case CCharacterCommand::CC_WaitForPlayerToMove:
			{
				//Wait until player moves in direction X.
				if (!player.IsInRoom())
					STOP_COMMAND;

				const bool bPlayerMoved = player.wX != player.wPrevX ||
						player.wY != player.wPrevY;
				getCommandX(command, px);
				switch (px)
				{
					case CMD_C:
						if (player.wO != nNextCO(player.wPrevO))
							STOP_COMMAND;
						break;
					case CMD_CC:
						if (player.wO != nNextCCO(player.wPrevO))
							STOP_COMMAND;
						break;
					default:
						if (!bPlayerMoved || CSwordsman::GetSwordMovement( //conversion routine
								nLastCommand, NO_ORIENTATION) != px)
							STOP_COMMAND;
					break;
				}
				bProcessNextCommand = true;
			}
			break;
			case CCharacterCommand::CC_WaitForPlayerToTouchMe:
			{
				//Wait until player bumps into me (on this turn).
				if (player.wX == this->wX && player.wY == this->wY)
					this->bPlayerTouchedMe = true; //standing on an invisible NPC counts

				if (!this->bPlayerTouchedMe)
					STOP_COMMAND;
				bProcessNextCommand = true;
			}
			break;


			case CCharacterCommand::CC_Label:
				//A comment or destination marker for a GoTo command.
				bProcessNextCommand = true;
			break;
			case CCharacterCommand::CC_GoTo:
			{
				//Continue executing script commands from marked jump point.
				//Will continue script if jump point is invalid.
				const int wNextIndex = GetIndexOfCommandWithLabel(command.x);
				if (wNextIndex != NO_LABEL)
					this->wCurrentCommandIndex = wNextIndex;
				else
					++this->wCurrentCommandIndex; //invalid jump -- just play next command
				bProcessNextCommand = true;
			}
			continue;   //don't increment wCurrentCommandIndex again

			case CCharacterCommand::CC_Speech:
			{
				//Deliver speech dialog.
				if (!command.pSpeech)
					break; //robustness check
				CFiredCharacterCommand *pSpeech = new CFiredCharacterCommand(this, &command,
					pGame->wTurnNo, this->dwScriptID, this->wCurrentCommandIndex);
				pSpeech->text = pGame->ExpandText(
						(const WCHAR*)command.pSpeech->MessageText);

				CueEvents.Add(CID_Speech, pSpeech);	//don't attach object to event
				bProcessNextCommand = true;
			}
			break;
			case CCharacterCommand::CC_FlushSpeech:
			{
				//Remove queued speech commands (front end: either display immediately or discard).
				CFiredCharacterCommand *pFlushSpeech = new CFiredCharacterCommand(this, &command,
					pGame->wTurnNo, this->dwScriptID, this->wCurrentCommandIndex);
				pFlushSpeech->bFlush = true;
				pFlushSpeech->bPlaySound = command.x != 0;
				CueEvents.Add(CID_Speech, pFlushSpeech);	//don't attach -- it should be deleted by handler
				bProcessNextCommand = true;
			}
			break;

			case CCharacterCommand::CC_AnswerOption:
			{
				//Text answer option (speech text) for a Question command that jumps to Label (x).
				this->answerOptions += this->wCurrentCommandIndex;
				bProcessNextCommand = true;
			}
			break;
			case CCharacterCommand::CC_Question:
			{
				//Ask a yes/no or multiple choice question (speech text).

				//Only wait on question if it's being asked on room entrance.
				if (pGame->ExecutingNoMoveCommands())
				{
					//If the question follows an If condition, back up to the If
					//so the check is performed for real next turn.
					if (this->wJumpLabel)
						--this->wCurrentCommandIndex;
					return;
				}

				//Ignore set of answers if question is an If condition (i.e. follows an If command)
				this->bYesNoQuestion = this->answerOptions.empty() || this->bIfBlock;
				CDbSpeech *pSpeech = command.pSpeech;
				ASSERT(pSpeech);
				const WCHAR *pText = (const WCHAR*)pSpeech->MessageText;
				WSTRING wstr = pGame->ExpandText(pText);
				CMonsterMessage *pMessage = new CMonsterMessage(
						this->bYesNoQuestion ? MMT_YESNO : MMT_MENU, wstr.c_str(), this);
				CueEvents.Add(CID_MonsterSpoke, pMessage, true);

				//Stop processing until answer to question is received, then resume next turn.
				++this->wCurrentCommandIndex;
				goto Finish;
			}
			break;

			case CCharacterCommand::CC_SetMusic:
			{
				//Set music being played to X (custom Y/label).
				if (!command.label.empty())
					CueEvents.Add(CID_SetMusic, new CAttachableWrapper<WSTRING>(command.label), true);
				else
					CueEvents.Add(CID_SetMusic, new CCoord(command.x, command.y), true);
				bProcessNextCommand = true;
			}
			break;

			case CCharacterCommand::CC_Imperative:
			{
				//Set NPC imperative status.
				const ScriptFlag::Imperative eNewImperative = (ScriptFlag::Imperative)command.x;
				bool bChangeImperative = true;
				switch (eNewImperative)
				{
					case ScriptFlag::Safe:
						this->bSafeToPlayer = true;
						bChangeImperative = false;
					break;
					case ScriptFlag::SwordSafeToPlayer:
						this->bSwordSafeToPlayer = true;
						bChangeImperative = false;
					break;
					case ScriptFlag::EndWhenKilled:
						this->bEndWhenKilled = true;
						bChangeImperative = false;
					break;
					case ScriptFlag::Deadly:
						this->bSafeToPlayer = this->bSwordSafeToPlayer = false;
						bChangeImperative = false;
					break;
					case ScriptFlag::DieSpecial:
						bChangeImperative = !HasSpecialDeath();
						//no break
					case ScriptFlag::Die:
						if (bExecuteNoMoveCommands && bChangeImperative)
							return; //wait until first move to die

						//Stop script execution whether visible or not.
						if (bChangeImperative)
							this->wCurrentCommandIndex = this->commands.size();
						if (this->bVisible && IsAlive())
						{
							//NPC dies.
							if (!bExecuteNoMoveCommands)
								CueEvents.Add(CID_MonsterDiedFromStab, this);
							if (bChangeImperative)
							{
								//Normal death behavior.
								CCueEvents Ignored;
								SetKillInfo(NO_ORIENTATION); //center stab effect
								room.KillMonster(this, Ignored, true);
							} else {
								//Special death behavior.
								switch (GetResolvedIdentity())
								{
									case M_ROCKGOLEM: this->wO = NO_ORIENTATION; break;
									case M_FEGUNDO: TurnIntoMonster(CueEvents, true); break;
									default: break;
								}
							}
						}
					break;
					case ScriptFlag::FlexibleBeelining:
						this->movementIQ = SmartOmniDirection;
						bChangeImperative = false;
					break;
					case ScriptFlag::DirectBeelining:
						this->movementIQ = DirectOnly;
						bChangeImperative = false;
					break;
					case ScriptFlag::NoGhostDisplay:
						this->bGhostImage = false;
					break;
					case ScriptFlag::GhostDisplay:
						this->bGhostImage = true;
					break;
					default: break;
				}
				if (bChangeImperative)
				{
					//Character's status has been modified -- update room state.
					const ScriptFlag::Imperative eOldImperative = this->eImperative;
					if (this->bVisible && eNewImperative != eOldImperative)
					{
						if (eOldImperative == ScriptFlag::RequiredToConquer)
						{
							if (IsAlive()) //Monster count may've already been decremented by Die/DieSpecial
								room.DecMonsterCount();
							CueEvents.Add(CID_NPCTypeChange);
						}
						else if (eNewImperative == ScriptFlag::RequiredToConquer)
						{
							++room.wMonsterCount;
							CueEvents.Add(CID_NPCTypeChange);
						}
					}

					this->eImperative = eNewImperative;
				}
				if (IsAlive())
					bProcessNextCommand = true;
			}
			break;
			case CCharacterCommand::CC_TurnIntoMonster:
				TurnIntoMonster(CueEvents);
			break;
			case CCharacterCommand::CC_EndScript:
				//Remove character from any future play in the current game.
				this->bScriptDone = true;
				this->wCurrentCommandIndex = this->commands.size();
			goto Finish;
			case CCharacterCommand::CC_EndScriptOnExit:
				//Remove character from any future play in the current game once the room is exited.
				this->bScriptDone = true;
				bProcessNextCommand = true;
			break;

			case CCharacterCommand::CC_StartGlobalScript:
			{
				UINT dwCharID = (UINT)command.x;
				if (!pGame->GlobalScriptsRunning.has(dwCharID))
					room.AddNewGlobalScript(dwCharID, CueEvents);

				bProcessNextCommand = true;
			}
			break;

			case CCharacterCommand::CC_If:
				//Begin a conditional block if the next command is satisfied.
				//If it is not satisfied, the code block will be skipped.
				++this->wCurrentCommandIndex;
				this->wJumpLabel = this->wCurrentCommandIndex + 1;
				this->bIfBlock = true;
				bProcessNextCommand = true;
			continue;   //perform the jump check below next iteration
			case CCharacterCommand::CC_IfElse:
			{
				//Marks the beginning of a code block executed when an CC_If condition was not satisfied.
				//Note that reaching this command indicates an If (true) block has successfully
				//completed, thus the following Else code block should be skipped.
				//
				//If an Else command is encountered in the code (w/o any previous If)
				//then it will effective function as an If-false and skip the next block.
				this->bIfBlock = true; //this will skip the subsequent code block
				bProcessNextCommand = true;
			}
			break;
			case CCharacterCommand::CC_IfEnd:
				//Ends a conditional If or IfElse block.
				bProcessNextCommand = true;
			break;

			case CCharacterCommand::CC_LevelEntrance:
				//Takes player to level entrance X.  If Y is set, skip level entrance display.
				if (!pGame->wTurnNo)
					return; //don't execute on the room entrance move -- execute next turn

				//When saving room data in GotoLevelEntrance,
				//this NPC should start at the next script command
				//next time the script is processed.
				++this->wCurrentCommandIndex;

				getCommandXY(command, px, py); //NOTE: only py is considered here
				if (!CueEvents.HasOccurred(CID_ExitLevelPending)) //don't queue more than one level exit
					pGame->GotoLevelEntrance(CueEvents, command.x, py != 0);

				--this->wCurrentCommandIndex; //revert to current command so it increments correctly for global scripts
			break;

			case CCharacterCommand::CC_VarSet:
			{
				//Sets var X (operation Y) W, e.g. X += 5

				//Get variable.
				CDbPackedVars& stats = pGame->stats;
				char varID[10], varName[11] = "v";
				UNPACKEDVARTYPE vType = UVT_int;

				const bool bPredefinedVar = command.x >= UINT(ScriptVars::FirstPredefinedVar);
				int predefinedVarVal;
				bool bValidInt = true;
				if (!bPredefinedVar)
				{
					//Get local hold var.
					_itoa(command.x, varID, 10);
					strcat(varName, varID);

					//Enforce basic type checking.
					vType = stats.GetVarType(varName);
					bValidInt = vType == UVT_int || vType == UVT_uint || vType == UVT_unknown;
				} else {
					predefinedVarVal = int(getPredefinedVar(command.x));
				}

				const bool bSetNumber = !(command.y == ScriptVars::AssignText ||
						command.y == ScriptVars::AppendText);

				int operand = int(command.w); //expect an integer by default
				if (!operand && !command.label.empty() && bSetNumber)
				{
					//Operand is not just an integer, but a text expression.
					UINT index=0;
					operand = parseExpression(command.label.c_str(), index, pGame, this);
				}

				int x=0;

				switch (command.y)
				{
					case ScriptVars::Assign:
						x = operand;
					break;
					case ScriptVars::Inc:
						if (bValidInt)
							x = bPredefinedVar ? predefinedVarVal : stats.GetVar(varName, (int)0);
						addWithClamp(x, operand);
					break;
					case ScriptVars::Dec:
						if (bValidInt)
							x = bPredefinedVar ? predefinedVarVal : stats.GetVar(varName, (int)0);
						addWithClamp(x, -operand);
					break;
					case ScriptVars::MultiplyBy:
						if (bValidInt)
							x = bPredefinedVar ? predefinedVarVal : stats.GetVar(varName, (int)0);
						multWithClamp(x, operand);
					break;
					case ScriptVars::DivideBy:
						if (bValidInt)
							x = bPredefinedVar ? predefinedVarVal : stats.GetVar(varName, (int)0);
						if (operand)
							x /= operand;
					break;
					case ScriptVars::Mod:
						if (bValidInt)
							x = bPredefinedVar ? predefinedVarVal : stats.GetVar(varName, (int)0);
						if (operand)
							x = x % operand;
					break;

					case ScriptVars::AssignText:
					{
						WSTRING text = pGame->ExpandText(command.label.c_str());
						stats.SetVar(varName, text.c_str());
					}
					break;
					case ScriptVars::AppendText:
					{
						WSTRING text = stats.GetVar(varName, wszEmpty);
						text += pGame->ExpandText(command.label.c_str());
						stats.SetVar(varName, text.c_str());
					}
					break;
					default: break;
				}
				if (bSetNumber)
				{
					if (bPredefinedVar)
						setPredefinedVar(command.x, x, CueEvents);
					else
						stats.SetVar(varName, x);
				}

				//When a var is set, this might get it out of an otherwise infinite loop.
				++wVarSets;
				wTurnCount = 0;

				bProcessNextCommand = true;
			}
			break;
			case CCharacterCommand::CC_WaitForVar:
			{
				//Wait until var X (comparison Y) W, e.g. X >= 5

				//Get variable.
				CDbPackedVars& stats = pGame->stats;
				char varID[10], varName[11] = "v";
				UNPACKEDVARTYPE vType = UVT_int;

				const bool bPredefinedVar = command.x >= UINT(ScriptVars::FirstPredefinedVar);
				bool bValidInt = true;
				if (!bPredefinedVar)
				{
					//Get local hold var.
					_itoa(command.x, varID, 10);
					strcat(varName, varID);

					//Enforce basic type checking.
					vType = stats.GetVarType(varName);
					bValidInt = vType == UVT_int || vType == UVT_uint || vType == UVT_unknown;
				}

				int operand = int(command.w); //expect an integer value by default
				if (!operand && !command.label.empty())
				{
					//Operand is not just an integer, but a text expression.
					UINT index=0;
					operand = parseExpression(command.label.c_str(), index, pGame, this);
				}

				int x=0;
				const bool bNumber = bValidInt && command.y != ScriptVars::EqualsText;
				if (bNumber)
					x = (bPredefinedVar ? int(getPredefinedVar(command.x))
							: stats.GetVar(varName, (int)0));

				switch (command.y)
				{
					case ScriptVars::Equals: if (x != operand) STOP_COMMAND; break;
					case ScriptVars::Greater: if (x <= operand) STOP_COMMAND; break;
					case ScriptVars::Less: if (x >= operand) STOP_COMMAND; break;
					case ScriptVars::EqualsText:
					{
						WSTRING wStr;
						if (vType == UVT_wchar_string)
							wStr = stats.GetVar(varName, wszEmpty);
						if (wStr.compare(command.label) != 0) STOP_COMMAND;
					}
					break;
					default: break;
				}

				bProcessNextCommand = true;
			}
			break;

			case CCharacterCommand::CC_SetPlayerAppearance:
			{
				if (this->bIfBlock)
				{
					//As an If condition, this acts as a query that is true when
					//the player is in this role.
					if (player.wIdentity != command.x)
						STOP_COMMAND;
				} else {
					//Sets player to look like entity X.
					pGame->SetPlayerRole(command.x, CueEvents);
				}
				bProcessNextCommand = true;
			}
			break;
			case CCharacterCommand::CC_SetNPCAppearance:
			{
				//Sets this NPC to look like entity X.
				this->wIdentity = this->wLogicalIdentity = command.x;
				ResolveLogicalIdentity(pGame->pHold);
				bProcessNextCommand = true;
			}
			break;
			case CCharacterCommand::CC_CutScene:
				//Begin cut scene (if X is set), else ends cut scene.
				getCommandX(command, px);
				pGame->dwCutScene = px;
				bProcessNextCommand = true;
				//If ending a cut scene, then wait until next turn to make further moves.
				if (!px)
					bExecuteNoMoveCommands = true;
			break;

			case CCharacterCommand::CC_SetPlayerSword:
			{
				//If X is set, player is given a sword, else it is taken away.
				player.bSwordOff = !command.x;
				pGame->SetCloneSwordsSheathed(); //synch clones
				bProcessNextCommand = true;
			}
			break;

			case CCharacterCommand::CC_SetWaterTraversal:
			{
				if (player.wWaterTraversal != command.x)
				{
					bool wOldCanHide = player.GetWaterTraversalState() == WTrv_CanHide;
					player.wWaterTraversal = command.x;
					bool wNewCanHide = player.GetWaterTraversalState() == WTrv_CanHide;

					//Check if we need to update sword sheathing/invisibility
					if (wOldCanHide != wNewCanHide)
					{
						pGame->SetPlayerSwordSheathed();
						pGame->SetCloneSwordsSheathed(); //synch clones
					}
				}

				bProcessNextCommand = true;
			}
			break;
			case CCharacterCommand::CC_WaitForItem:
				//Wait for game element (flags) to exist in rect (x,y,w,h).
				if (!IsTileAt(command))
					STOP_COMMAND;
				bProcessNextCommand = true;
			break;
			case CCharacterCommand::CC_GenerateEntity:
			{
				//Generates a new entity of type h in the room at (x,y) with orientation w.

				//Ensure square is available before appearing.
				//Custom NPCs may appear where another entity is in the room,
				//but they will not be allowed to appear in the room until the
				//tile is vacant.
				getCommandRect(command, px, py, pw, ph);
				const UINT identity = ph;
				if (!room.IsValidColRow(px,py) || (identity < CUSTOM_CHARACTER_FIRST &&
						(room.GetMonsterAtSquare(px, py) != NULL ||
							pGame->IsPlayerAt(px, py))))
					STOP_COMMAND;

				//Place new entity on this tile.
				if (pw == CMD_C)
					pw = nNextCO(this->wO);
				else if (pw == CMD_CC)
					pw = nNextCCO(this->wO);

				pGame->AddNewEntity(CueEvents, identity, px, py, pw);
				CueEvents.Add(CID_NPCTypeChange);

				bProcessNextCommand = true;
			}
			break;

			case CCharacterCommand::CC_GameEffect:
			{
				//Cues the front end to generate a graphic+sound effect (w,h,flags) at (x,y).
				getCommandParams(command, px, py, pw, ph, pflags);
				VisualEffectInfo *pEffect = new VisualEffectInfo(px, py, pw, ph, pflags, this->wX, this->wY);
				CueEvents.Add(CID_VisualEffect, pEffect, true);

				bProcessNextCommand = true;
			}
			break;

			case CCharacterCommand::CC_BuildMarker:
			{
				//Mark rect (x,y,w,h) for building game element (flags).
				BuildTiles(command);
				bProcessNextCommand = true;
			}
			break;
			case CCharacterCommand::CC_WaitForNoBuilding:
			{
				bool bFound = false;
				//Wait until no build markers are queued in rect (x,y,w,h).
				getCommandRect(command, px, py, pw, ph);
				for (UINT y=py; !bFound && y <= py + ph; ++y)
					for (UINT x=px; !bFound && x <= px + pw; ++x)
						if (room.building.get(x,y))
							bFound = true;
				if (bFound)
					STOP_COMMAND;
				bProcessNextCommand = true;
			}
			break;

			case CCharacterCommand::CC_AmbientSound:
				//Play sound with DataID w (0 stops ambient sounds).
				//If h is set, loop indefinitely.
				CueEvents.Add(CID_AmbientSound, new CMoveCoordEx(UINT(-1), UINT(-1), command.w, command.h), true);
				bProcessNextCommand = true;
			break;
			case CCharacterCommand::CC_AmbientSoundAt:
				//Play sound with DataID=w (0 stops ambient sounds) at (x,y).
				//If h is set, loop indefinitely.
				getCommandXY(command, px, py);
				CueEvents.Add(CID_AmbientSound, new CMoveCoordEx(px, py, command.w, command.h), true);
				bProcessNextCommand = true;
			break;
			case CCharacterCommand::CC_PlayVideo:
				//Play video at (x,y) with DataID=w.
				getCommandXY(command, px, py);
				CueEvents.Add(CID_PlayVideo, new CMoveCoord(px, py, command.w), true);
				bProcessNextCommand = true;
			break;

			//Deprecated commands
			case CCharacterCommand::CC_GotoIf:
			case CCharacterCommand::CC_WaitForHalph:
			case CCharacterCommand::CC_WaitForNotHalph:
			case CCharacterCommand::CC_WaitForMonster:
			case CCharacterCommand::CC_WaitForNotMonster:
			case CCharacterCommand::CC_WaitForCharacter:
			case CCharacterCommand::CC_WaitForNotCharacter:
				ASSERT(!"Deprecated script command");
			break;

			default: ASSERT(!"Bad CCharacter command"); break;
		}

		++this->wCurrentCommandIndex;

		//If MoveRel command was used as an If condition, then reset the relative
		//movement destination for the next relative movement command.
		if (this->bIfBlock)
			this->bMovingRelative = false;

		//Go to jump point if this command executed successfully.
		if (this->wJumpLabel)
		{
			const int wNextIndex = this->bIfBlock ? this->wJumpLabel :
					GetIndexOfCommandWithLabel(this->wJumpLabel);
			if (wNextIndex != NO_LABEL)
				this->wCurrentCommandIndex = wNextIndex;
			this->wJumpLabel = 0;
			this->bIfBlock = false;
		}
		else if (this->bIfBlock) //arriving here indicates If condition failed
			FailedIfCondition();

		//Stop script if more than a certain number of commands have played through
		//on this turn, indicating a probable infinite loop.
		static const UINT VARSET_LIMIT = 1000;
		if (++wTurnCount > this->commands.size() || wVarSets > VARSET_LIMIT)
		{
			this->wCurrentCommandIndex = this->commands.size();
			CFiles f;
			f.AppendUserLog("Character script is in an infinite loop" NEWLINE);
		}
	} while (bProcessNextCommand);

	} //Wrap variables initialized within jump, to make g++ happy

Finish:
	if (this->bVisible && bIsBeethroDouble(GetResolvedIdentity()))
	{
		//Light any fuse stood on.
 		room.LightFuse(CueEvents, this->wX, this->wY, true);
	}

	this->bPlayerTouchedMe = false; //this flag is reset at the end of each turn

#undef STOP_COMMAND
#undef STOP_DONECOMMAND
}

void CCharacter::BuildTiles(const CCharacterCommand& command)
//Build the specified game element (flags) in rect (x,y,w,h).
{
	UINT px, py, pw, ph, pflags;  //command parameters
	getCommandParams(command, px, py, pw, ph, pflags);

	CDbRoom& room = *(this->pCurrentGame->pRoom);
	if (px >= room.wRoomCols || py >= room.wRoomRows)
		return; //build area is completely out of room bounds -- do nothing

	for (UINT y=py; y <= py + ph && y < room.wRoomRows; ++y)
		for (UINT x=px; x <= px + pw && x < room.wRoomCols; ++x)
		{
			//Mark for building if there is no critical obstruction.
			bool bValid = true;
			if (IsValidTileNo(pflags))
			{
				switch (TILE_LAYER[pflags])
				{
					case 0:  //o-layer
						//Don't build if this element is already there.
						if (room.GetOSquare(x,y) == pflags)
							bValid = false;
					break;
					case 1:  //t-layer
					{
						//Don't build if this element is already there.
						const UINT wTTile = room.GetBottomTSquare(x,y);
						if (wTTile == pflags)
						{
							bValid = false;
							break;
						}
						//Fuses, bombs, mirrors, potions and also briars can be replaced.
						if (wTTile == T_EMPTY || wTTile == T_BOMB || wTTile == T_FUSE || wTTile == T_MIRROR ||
								bIsPotion(wTTile) || bIsBriar(wTTile))
							break;
						//No other item can be built over.
						bValid = false;
					}
					break;
					default: ASSERT(!"Unsupported build layer");
						bValid = false;
					break;
				}
			}
			if (bValid)
				room.building.plot(x,y,pflags);
		}
}

//*****************************************************************************
void CCharacter::CheckForCueEvent(CCueEvents &CueEvents) //(in)
//Called once all cue events have been gathered.
//If the current command is waiting for a cue event, satisfying this
//will continue to the next command on the following turn.
{
	if (!this->bWaitingForCueEvent)
		return;
	ASSERT(this->wCurrentCommandIndex < this->commands.size());
	CCharacterCommand& command = this->commands[this->wCurrentCommandIndex];
	ASSERT(command.command == CCharacterCommand::CC_WaitForCueEvent);

	//Wait for cue event X to fire.
	const CUEEVENT_ID cid = static_cast<CUEEVENT_ID>(command.x);
	if (CueEvents.HasOccurred(cid))
	{
		++this->wCurrentCommandIndex;
		this->bWaitingForCueEvent = false; //no longer waiting for the event
	}
}

//*****************************************************************************
bool CCharacter::CheckForDamage(CCueEvents& CueEvents)
//Returns: whether monster was killed by room-inflicted damage
{
	if (!this->bVisible)
		return false;  //NPC not in room
	//invincibility checked in OnStabbed

	return CMonster::CheckForDamage(CueEvents);
}

//*****************************************************************************
void CCharacter::FailedIfCondition()
//An if condition failed.  Move command execution pointer past the if block.
{
	ASSERT(this->bIfBlock);

	//Scan until the end of the If block is encountered.
	//This could be indicated by either an IfElse or IfEnd command.
	UINT wNestingDepth = 0;
	bool bScanning = true;
	do
	{
		if (this->wCurrentCommandIndex >= this->commands.size())
			return; //block continued to the end of the script

		CCharacterCommand& command = this->commands[this->wCurrentCommandIndex];
		switch (command.command)
		{
			case CCharacterCommand::CC_If:
				++wNestingDepth;  //entering a nested If block
			break;
			case CCharacterCommand::CC_IfElse:
				if (wNestingDepth == 0)
					bScanning = false;  //found the If command's matching Else block
			break;
			case CCharacterCommand::CC_IfEnd:
				if (wNestingDepth-- == 0)	//exiting an If block
					bScanning = false;  //found the end of the If block (no Else was found in between)
			break;
			default: break;
		}
		++this->wCurrentCommandIndex;
	} while (bScanning);

	this->bIfBlock = false;
}

//*****************************************************************************
UINT CCharacter::GetResolvedIdentity() const
//Returns: what identity the NPC should take.
//  If the NPC is set to M_CLONE, then it will match the current player role.
//  Call this method instead of GetIdentity to fully resolve what an NPC should look like.
{
	UINT wIdentity = GetIdentity();
	if (wIdentity == M_CLONE)
	{
		//Resolve player appearance.
		CClone clone(const_cast<CCurrentGame*>(this->pCurrentGame));
		return clone.GetIdentity();
	}

	return wIdentity;
}

//*****************************************************************************
bool CCharacter::IsAttackableTarget() const
//Returns: whether this character is of a type and state that monsters can
//attack and kill.
{
	if (!this->bVisible)
		return false;

	if (GetImperative() == ScriptFlag::Invulnerable)
		return false;

	//These types can be attacked and killed.
	const UINT identity = GetResolvedIdentity();
	return bIsSmitemaster(identity) || bIsStalwart(identity);
}

//*****************************************************************************
bool CCharacter::IsFlying() const
//Returns: whether character is flying
{
	const UINT identity = GetResolvedIdentity();
	return bIsEntityFlying(identity);
}

//*****************************************************************************
bool CCharacter::IsFriendly() const
//Returns: whether character is friendly to the player
{
	const UINT identity = GetResolvedIdentity();
	return identity == M_HALPH || identity == M_HALPH2 ||
			bIsStalwart(identity) ||
			this->bSafeToPlayer;
}

//*****************************************************************************
bool CCharacter::IsMonsterTarget() const
//Returns: whether the character is a valid target for monsters
{
	const UINT identity = GetIdentity();
	//Clones are only targets if the player is
	if (identity == M_CLONE)
	{
		if (!this->pCurrentGame)
			return true;
		const CSwordsman& player = this->pCurrentGame->swordsman;
		if (player.wAppearance == M_NONE)
			return true;
		return player.IsTarget();
	}
	return bIsMonsterTarget(identity);
}

//*****************************************************************************
bool CCharacter::IsSwimming() const
//Returns: whether character is swimming
{
	const UINT identity = GetResolvedIdentity();
	return identity == M_WATERSKIPPER || identity == M_SKIPPERNEST;
}

//*****************************************************************************
bool CCharacter::IsTileAt(const CCharacterCommand& command) const
//Returns: whether the specified game element (flags) is in rect (x,y,w,h).
{
	UINT px, py, pw, ph, pflags;  //command parameters
	getCommandParams(command, px, py, pw, ph, pflags);

	CDbRoom& room = *(this->pCurrentGame->pRoom);
	const UINT tile = pflags;
	const bool bRealTile = IsValidTileNo(tile);
	if (bRealTile)
	{
		for (UINT y=py; y <= py + ph; ++y)
		{
			for (UINT x=px; x <= px + pw; ++x)
			{
				switch (TILE_LAYER[tile])
				{
					case 0:  //o-layer
						if (room.GetOSquare(x,y) == tile)
							return true;
					break;
					case 1:  //t-layer
						if (room.GetTSquare(x,y) == tile)
							return true;
					break;
					case 3:  //f-layer
						if (room.GetFSquare(x,y) == tile)
							return true;
					break;
					default: break;
				}
			}
		}
	}

	return false;
}

//*****************************************************************************
bool CCharacter::OnAnswer(
//Overridable method for responding to an answer given by player to a question
//asked by the monster.
//
//Params:
	int nCommand,        //(in)   CMD_YES or CMD_NO, or line # of selected AnswerOption.
	CCueEvents &/*CueEvents*/)  //(out)  Add cue events if appropriate.
//
//Returns:
//True if any cue events were added, false if not.
{
	if (this->bYesNoQuestion)
	{
		//Primitive yes/no answer given.
		if (this->bIfBlock)
		{
			if (nCommand != CMD_YES)
				FailedIfCondition(); //skip if block
			else
			{
				//Enter if block.
				if (this->wJumpLabel)
				{
					const int wNextIndex = this->bIfBlock ? this->wJumpLabel :
							GetIndexOfCommandWithLabel(this->wJumpLabel);
					if (wNextIndex != NO_LABEL)
						this->wCurrentCommandIndex = wNextIndex;
					this->wJumpLabel = 0;
				}
			}
		}
		//else the question was not asked as a conditional
	} else {
		//Answer selected from a set.
		ASSERT(!this->bIfBlock);
		ASSERT(nCommand >= 0);
		ASSERT((UINT)nCommand < this->commands.size());
		const int wNextIndex = GetIndexOfCommandWithLabel(nCommand);
		if (wNextIndex != NO_LABEL)
			this->wCurrentCommandIndex = wNextIndex;

		this->answerOptions.clear(); //reset answer set for next question
	}

	return false;
}

//*****************************************************************************
#define STARTVPTAG(vpType,pType) "<"; str += ViewpropTypeStr(vpType); str += " "; str += PropTypeStr(pType); str += "='"
#define PROPTAG(pType) "' "; str += PropTypeStr(pType); str += "='"
#define ENDVPTAG(vpType) "</"; str += ViewpropTypeStr(vpType); str += ">" NEWLINE
#define CLOSESTARTTAG "'>" NEWLINE
#define LONGTOSTR(val) _ltoa((val), dummy, 10)

//*****************************************************************************
void CCharacter::ExportText(CDbRefs &dbRefs, CStretchyBuffer& str)
//No texts to export here, but owned speech texts are exported.
{
	bool bContainsSpeechText = false;

	//Export speech texts.
	char dummy[32];
	const UINT wNumCommands = this->commands.size();
	for (UINT wIndex=0; wIndex<wNumCommands; ++wIndex)
	{
		CCharacterCommand& command = this->commands[wIndex];
		if (command.pSpeech)
		{
			if (!bContainsSpeechText)
			{
				//Output character ID first time relevant data are encountered.
				bContainsSpeechText = true;

				str += STARTVPTAG(VP_Monsters, P_X);
				str += LONGTOSTR(this->wX);
				str += PROPTAG(P_Y);
				str += LONGTOSTR(this->wY);
				str += CLOSESTARTTAG;
			}

			const UINT dwSpeechID = command.pSpeech->dwSpeechID;
			g_pTheDB->Speech.ExportText(dwSpeechID, dbRefs, str);
		}
	}

	if (bContainsSpeechText)
	{
		str += ENDVPTAG(VP_Monsters);
	}
}
#undef STARTVPTAG
#undef PROPTAG
#undef ENDVPTAG
#undef CLOSESTARTTAG
#undef LONGTOSTR

//*****************************************************************************
string CCharacter::ExportXMLSpeech(
//Returns: string containing XML text describing character with this ID
//
//Params:
	CDbRefs &dbRefs,        //(in/out)
	const COMMAND_VECTOR& commands, //(in)
	const bool bRef) //Only export GUID references [default=false]
{
	string str;

	const UINT wNumCommands = commands.size();
	for (UINT wIndex=0; wIndex<wNumCommands; ++wIndex)
	{
		const CCharacterCommand& command = commands[wIndex];

		//Export character-owned speech records.
		if (command.pSpeech && command.pSpeech->dwSpeechID)
			g_pTheDB->Speech.ExportXML(command.pSpeech->dwSpeechID, dbRefs, str, bRef);

		//Export character-owned sound effects.
		if (bCommandHasData(command.command) && command.w && g_pTheDB->Data.Exists(command.w))
			g_pTheDB->Data.ExportXML(command.w, dbRefs, str, bRef);
	}

	return str;
}

//*****************************************************************************
UINT CCharacter::GetNextSpeechID()
//Returns: speechID of next speech record referenced in character script,
//or 0 if there are no more
{
	while (this->wLastSpeechLineNumber < this->commands.size())
	{
		CCharacterCommand& command = this->commands[this->wLastSpeechLineNumber++];
		if (command.pSpeech)
			return command.pSpeech->dwSpeechID;
	}

	return 0; //no more
}

//*****************************************************************************
MESSAGE_ID CCharacter::ImportSpeech(
//Updates speech and data IDs in script commands.
//
//Params:
	CImportInfo &info,     //(in/out) Import data
	const bool bHoldChar)  //(in) Whether character is a Hold Character Default Script
{
	UINT dwSpeechID, dwDataID, eCommand;
	const UINT wNumCommands = this->ExtraVars.GetVar(numCommandsStr, 0);
	PrimaryKeyMap::iterator localID;

	BYTE *commandBuffer = (BYTE*)this->ExtraVars.GetVar(commandStr, (const void*)(NULL));
	BYTE *oldCommandBuffer = (BYTE*)this->ExtraVars.GetVar(commandStr_3_0_2_2, (const void*)(NULL));
	if (commandBuffer || oldCommandBuffer)
	{
		// Set member vars at the same time
		SetBaseMembersFromExtraVars();

		this->dwScriptID = this->ExtraVars.GetVar(scriptIDstr, 0);

		this->commands.reserve(wNumCommands);

		UINT index = 0;

		if (commandBuffer)
		{
			//Current script data format.
			const UINT bufferSize = this->ExtraVars.GetVarValueSize(commandStr);

			while (index < bufferSize)
			{
				CCharacterCommand command;

				eCommand = readBpUINT(commandBuffer, index);
				command.command = CCharacterCommand::CharCommand(eCommand);
				command.x = readBpUINT(commandBuffer, index);
				command.y = readBpUINT(commandBuffer, index);
				dwDataID = readBpUINT(commandBuffer, index); // set this later
				command.h = readBpUINT(commandBuffer, index);
				command.flags = readBpUINT(commandBuffer, index);
				dwSpeechID = readBpUINT(commandBuffer, index); // set this later

				const UINT labelSize = readBpUINT(commandBuffer, index);
				if (labelSize)
				{
					const UINT wchars = labelSize/sizeof(WCHAR);
#if (GAME_BYTEORDER == GAME_BYTEORDER_BIG)
					WCHAR *pwzBuf = new WCHAR[wchars+1];
					memcpy((void*)pwzBuf, (const void*)(commandBuffer + index), labelSize);
					LittleToBig(pwzBuf, wchars);
					command.label.assign(pwzBuf, wchars);
					delete[] pwzBuf;
#else
					command.label.assign((const WCHAR*)(commandBuffer + index), wchars);
#endif
					index += labelSize;
				}

				if (bCommandHasData(eCommand))
				{
					if (eCommand == CCharacterCommand::CC_SetMusic)
						dwDataID = command.y; //SetMusic uses y param to store the music dataID
					if (dwDataID)
					{
						//If data is not present, simply reset this field.
						localID = info.DataIDMap.find(dwDataID);
						dwDataID = localID != info.DataIDMap.end() ? localID->second : 0;
					}
				}
				if (eCommand == CCharacterCommand::CC_SetMusic)
					command.y = dwDataID;
				else
					command.w = dwDataID;

				if (dwSpeechID)
				{
					localID = info.SpeechIDMap.find(dwSpeechID);
					if (localID == info.SpeechIDMap.end())
						return MID_FileCorrupted;  //record should have been loaded already
					dwSpeechID = localID->second;
					command.pSpeech = g_pTheDB->Speech.GetByID(dwSpeechID);
					ASSERT(command.pSpeech);
				}

				Upgrade2_0CommandTo3_0(command, this->commands);
			}
			ASSERT(index == bufferSize);
		} else if (oldCommandBuffer) {
			//3.0.2 rev2 script data format.
			const UINT bufferSize = this->ExtraVars.GetVarValueSize(commandStr_3_0_2_2);

			while (index < bufferSize)
			{
				CCharacterCommand command;

				eCommand = oldCommandBuffer[index++];
				command.command = CCharacterCommand::CharCommand(eCommand);
				command.x = readUINT(oldCommandBuffer, index);
				command.y = readUINT(oldCommandBuffer, index);
				dwDataID = readUINT(oldCommandBuffer, index); // set this later
				command.h = readUINT(oldCommandBuffer, index);
				command.flags = readUINT(oldCommandBuffer, index);
				dwSpeechID = readUINT(oldCommandBuffer, index); // set this later

				const UINT labelSize = readUINT(oldCommandBuffer, index);
				string str;
				str.resize(labelSize);
				for (UINT i=0; i<labelSize; ++i)
					str[i] = oldCommandBuffer[index++];
				Base64::decode(str, command.label);

				if (bCommandHasData(eCommand))
				{
					if (dwDataID)
					{
						//If data is not present, simply reset this field.
						localID = info.DataIDMap.find(dwDataID);
						dwDataID = localID != info.DataIDMap.end() ? localID->second : 0;
					}
				}
				command.w = dwDataID;

				if (dwSpeechID)
				{
					localID = info.SpeechIDMap.find(dwSpeechID);
					if (localID == info.SpeechIDMap.end())
						return MID_FileCorrupted;  //record should have been loaded already
					dwSpeechID = localID->second;
					command.pSpeech = g_pTheDB->Speech.GetByID(dwSpeechID);
					ASSERT(command.pSpeech);
				}

				Upgrade2_0CommandTo3_0(command, this->commands);
			}
			ASSERT(index == bufferSize);
		}
		ASSERT(this->commands.size() == wNumCommands);
	} else {
		//Pre-3.0.2 rev2 script data format.
		char varName[20], num[10];
		for (UINT wIndex=0; wIndex<wNumCommands; ++wIndex)
		{
			_itoa(wIndex, num, 10);
			strcpy(varName, num);
			strcat(varName, "c");
			eCommand = this->ExtraVars.GetVar(varName, (UINT)CCharacterCommand::CC_Count);
			if (bCommandHasData(eCommand))
			{
				strcpy(varName, num);
				strcat(varName, "w");
				dwDataID = this->ExtraVars.GetVar(varName, 0);
				if (dwDataID)
				{
					//If data is not present, simply reset this field.
					localID = info.DataIDMap.find(dwDataID);
					dwDataID = localID != info.DataIDMap.end() ? localID->second : 0;
					this->ExtraVars.SetVar(varName, dwDataID);
				}
			}

			strcpy(varName, num);
			strcat(varName, "s");
			dwSpeechID = this->ExtraVars.GetVar(varName, 0);
			if (dwSpeechID)
			{
				localID = info.SpeechIDMap.find(dwSpeechID);
				if (localID == info.SpeechIDMap.end())
					return MID_FileCorrupted;  //record should have been loaded already
				dwSpeechID = localID->second;
				this->ExtraVars.SetVar(varName, dwSpeechID);
			}
		}

		//Upgrade deprecated 2.0 functionality to 3.0 replacements.
		SetMembersFromExtraVars();
	}

	SetExtraVarsFromMembers(bHoldChar);

	return MID_ImportSuccessful;
}

//******************************************************************************************
bool CCharacter::DoesSquareContainObstacle(
//Override for characters.  Parts copied from CMimic::DoesSquareContainObstacle.
//
//Params:
	const UINT wCol, const UINT wRow) //(in) Coords of square to evaluate.
const
{
	//Code below only applies to characters in human roles.
	if (!bIsHuman(GetResolvedIdentity()))
	{
		//Can't step on the player if flag is set.
		if (this->bSafeToPlayer && this->pCurrentGame->IsPlayerAt(wCol, wRow))
			return true;

		//Rest of the checks for monster types is done in the base method.
		return CMonster::DoesSquareContainObstacle(wCol, wRow);
	}

	//Routine is not written to check the square on which this monster is
	//standing.
	ASSERT(wCol != this->wX || wRow != this->wY);

	CDbRoom& room = *(this->pCurrentGame->pRoom);
	if (!room.IsValidColRow(wCol,wRow)) return true;

	//Check o-square for obstacle.
	UINT wLookTileNo = room.GetOSquare(wCol, wRow);
	if (IsTileObstacle(wLookTileNo))
	{
		//There is something at the destination that is normally an obstacle,
		//but some of them are handled specially.  Check for special handling first.
		switch (wLookTileNo)
		{
			//If standing on a platform, check whether it can move.
			case T_PIT: case T_PIT_IMAGE:
				if (room.GetOSquare(this->wX, this->wY) == T_PLATFORM_P)
				{
					const int nFirstO = nGetO((int)wCol - (int)this->wX, (int)wRow - (int)this->wY);
					if (room.CanMovePlatform(this->wX, this->wY, nFirstO))
						break;
				}
			return true;
			case T_WATER: /*case T_SHALLOW_WATER:*/
				if (room.GetOSquare(this->wX, this->wY) == T_PLATFORM_W)
				{
					const int nFirstO = nGetO((int)wCol - (int)this->wX, (int)wRow - (int)this->wY);
					if (room.CanMovePlatform(this->wX, this->wY, nFirstO))
						break;
				}
			return true;
			default:	return true;
		}
	}

	//Check t-square for obstacle.
	wLookTileNo = room.GetTSquare(wCol, wRow);
	if (IsTileObstacle(wLookTileNo))
	{
		//There is something at the destination that is normally an obstacle,
		//but some of them are handled specially.  Check for special handling first.
		switch (wLookTileNo)
		{
			case T_MIRROR:
			{
				const int dx = (int)wCol - (int)this->wX;
				const int dy = (int)wRow - (int)this->wY;
				if (room.CanPushTo(wCol, wRow, wCol + dx, wRow + dy))
					break; //mirror is not an obstacle
			}
			//NO BREAK
			default:	return true;
		}
	}

	if (IsTileObstacle(room.GetFSquare(wCol, wRow)))
		return true;

	//Check for monster at square.
	CMonster *pMonster = room.GetMonsterAtSquare(wCol, wRow);
	if (pMonster)
		return true;

	//Can't move onto player if set to "safe".
	if (this->bSafeToPlayer && this->pCurrentGame->IsPlayerAt(wCol, wRow))
		return true;

	//Check for player's sword at square.
	if (this->pCurrentGame->IsPlayerSwordAt(wCol, wRow))
		return true;

	//Check for monster sword at square.
	if (room.IsMonsterSwordAt(wCol, wRow, this))
		return true;

	//No obstacle.
	return false;
}

//*****************************************************************************
void CCharacter::getCommandParams(
//Outputs: the parameter values for this command
	const CCharacterCommand& command,
	UINT& x, UINT& y, UINT& w, UINT& h, UINT& f)
const
{
	x = (this->paramX == NO_OVERRIDE ? command.x : this->paramX);
	y = (this->paramY == NO_OVERRIDE ? command.y : this->paramY);
	w = (this->paramW == NO_OVERRIDE ? command.w : this->paramW);
	h = (this->paramH == NO_OVERRIDE ? command.h : this->paramH);
	f = (this->paramF == NO_OVERRIDE ? command.flags : this->paramF);
}

//*****************************************************************************
void CCharacter::getCommandRect(
//Outputs: the parameter values for this command rect
	const CCharacterCommand& command,
	UINT& x, UINT& y, UINT& w, UINT& h)
const
{
	x = (this->paramX == NO_OVERRIDE ? command.x : this->paramX);
	y = (this->paramY == NO_OVERRIDE ? command.y : this->paramY);
	w = (this->paramW == NO_OVERRIDE ? command.w : this->paramW);
	h = (this->paramH == NO_OVERRIDE ? command.h : this->paramH);
}

//*****************************************************************************
void CCharacter::getCommandX(
//Outputs: the X value for this command
	const CCharacterCommand& command, UINT& x)
const
{
	x = (this->paramX == NO_OVERRIDE ? command.x : this->paramX);
}

//*****************************************************************************
void CCharacter::getCommandXY(
//Outputs: the XY values for this command
	const CCharacterCommand& command, UINT& x, UINT& y)
const
{
	x = (this->paramX == NO_OVERRIDE ? command.x : this->paramX);
	y = (this->paramY == NO_OVERRIDE ? command.y : this->paramY);
}

//*****************************************************************************
bool CCharacter::IsOpenMove(const int dx, const int dy) const
//Returns: whether move is possible, and player is not in the way
{
	return CMonster::IsOpenMove(dx,dy) &&
		(!this->bSafeToPlayer || !this->pCurrentGame->IsPlayerAt(this->wX+dx, this->wY+dy));
}

//*****************************************************************************
bool CCharacter::IsTileObstacle(
//Override for NPCs.
//
//Params:
	const UINT wTileNo) //(in)   Tile to evaluate.  Note each tile# will always be
						//    found on the same layer of squares.
//
//Returns:
//True if tile is an obstacle, false if not.
const
{
	switch (GetResolvedIdentity())
	{
		//These types can move through wall.
		//NOTE: For greater scripting flexibility, these types will also be allowed
		//to perform normal movement.
		case M_SEEP:
		{
			return CMonster::IsTileObstacle(wTileNo) &&
				!(bIsWall(wTileNo) || bIsCrumblyWall(wTileNo) || bIsDoor(wTileNo));
			//i.e. tile is considered an obstacle only when it blocks both movement types
		}

		//These types can also move over pits.
		case M_WWING: case M_FEGUNDO:
			return CMonster::IsTileObstacle(wTileNo) && !bIsWater(wTileNo) && !bIsPit(wTileNo);

		case M_WATERSKIPPER:
		case M_SKIPPERNEST:
			return CMonster::IsTileObstacle(wTileNo) && !bIsWater(wTileNo);

		default:	return CMonster::IsTileObstacle(wTileNo);
	}
}

//*****************************************************************************
const CCharacterCommand* CCharacter::GetCommandWithLabel(const UINT label) const
//Returns: pointer to command with specified label, or NULL if none
{
	ASSERT(label);
	for (UINT wIndex=this->commands.size(); wIndex--; )
	{
		const CCharacterCommand& command = this->commands[wIndex];
		if (command.command == CCharacterCommand::CC_Label &&
				label == command.x)
			return &command;
	}
	return NULL;
}

//*****************************************************************************
void CCharacter::LoadCommands(CDbPackedVars& ExtraVars, COMMAND_VECTOR& commands)
{
	commands.clear();
	const UINT wNumCommands = ExtraVars.GetVar(numCommandsStr, UINT(0));
	if (!wNumCommands)
		return;

	commands.reserve(wNumCommands);

	BYTE *commandBuffer = (BYTE*)ExtraVars.GetVar(commandStr, (const void*)(NULL));
	if (commandBuffer)
	{
		const UINT bufferSize = ExtraVars.GetVarValueSize(commandStr);
		DeserializeCommands(commandBuffer, bufferSize, commands);
		ASSERT(commands.size() == wNumCommands);
	} else {
		//Packed var name used for command serialization in version 3.0.2 rev2.
		BYTE *oldCommandBuffer = (BYTE*)ExtraVars.GetVar(commandStr_3_0_2_2, (const void*)(NULL));
		if (oldCommandBuffer)
		{
			const UINT bufferSize = ExtraVars.GetVarValueSize(commandStr_3_0_2_2);
			DeserializeCommands_3_0_2_2(oldCommandBuffer, bufferSize, commands);
			ASSERT(commands.size() == wNumCommands);
		} else {
			//Pre-3.0.2 rev2 script data encapsulation format.

			//Construct each command.
			char num[10];
			char varName[20];
			UINT eCommand;
			UINT dwSpeechID;
			for (UINT wIndex=0; wIndex<wNumCommands; ++wIndex)
			{
				CCharacterCommand command;

				//Vars loaded here must match the naming convention used in CCharacter::Save().
				_itoa(wIndex, num, 10);
				strcpy(varName, num);
				strcat(varName, "c");
				eCommand = ExtraVars.GetVar(varName, (UINT)CCharacterCommand::CC_Count);
				ASSERT(eCommand < CCharacterCommand::CC_Count);
				command.command = (CCharacterCommand::CharCommand)eCommand;

				//Query in alphabetical order for speed.
				strcpy(varName, num);
				strcat(varName, "f");
				command.flags = ExtraVars.GetVar(varName, 0);
				strcpy(varName, num);
				strcat(varName, "h");
				command.h = ExtraVars.GetVar(varName, 0);
				strcpy(varName, num);
				strcat(varName, "l");
				command.label = ExtraVars.GetVar(varName, wszEmpty);
				strcpy(varName, num);
				strcat(varName, "s");
				dwSpeechID = ExtraVars.GetVar(varName, 0);
				strcpy(varName, num);
				strcat(varName, "w");
				command.w = ExtraVars.GetVar(varName, 0);
				strcpy(varName, num);
				strcat(varName, "x");
				command.x = ExtraVars.GetVar(varName, 0);
				strcpy(varName, num);
				strcat(varName, "y");
				command.y = ExtraVars.GetVar(varName, 0);
				if (dwSpeechID)
				{
					command.pSpeech = g_pTheDB->Speech.GetByID(dwSpeechID);
					ASSERT(command.pSpeech);
				}

				Upgrade2_0CommandTo3_0(command, commands);
			}
		}
	}
}

//*****************************************************************************
void CCharacter::LoadCommands(CDbPackedVars& ExtraVars, COMMANDPTR_VECTOR& commands)
//Overloaded method with vector of pointers to commands.
{
	//Delete commands pointed to.
	COMMANDPTR_VECTOR::iterator command;
	for (command = commands.begin(); command != commands.end(); ++command)
		delete *command;
	commands.clear();

	const UINT wNumCommands = ExtraVars.GetVar(numCommandsStr, UINT(0));
	if (!wNumCommands)
		return;

	commands.reserve(wNumCommands);

	BYTE *commandBuffer = (BYTE*)ExtraVars.GetVar(commandStr, (const void*)(NULL));
	if (commandBuffer)
	{
		const UINT bufferSize = ExtraVars.GetVarValueSize(commandStr);
		DeserializeCommands(commandBuffer, bufferSize, commands);
		ASSERT(commands.size() == wNumCommands);
	} else {
		//Pre-3.1 script data encapsulation format:
		ASSERT(!"Shouldn't be encountered in command pointers");
	}
}

//*****************************************************************************
void CCharacter::ResolveLogicalIdentity(CDbHold *pHold)
//Determines semantic identity from logical identity ID.
{
	if (this->wLogicalIdentity >= CHARACTER_TYPES)
	{
		if (this->wLogicalIdentity >= CUSTOM_CHARACTER_FIRST && this->wLogicalIdentity != M_NONE && pHold)
		{
			//Keep reference to custom info.
			this->pCustomChar = pHold->GetCharacter(this->wLogicalIdentity);

			//Show character with designated identity.
			if (this->pCustomChar)
				this->wIdentity = this->pCustomChar->wType;
			else
				//When character has a dangling reference to a custom character definition
				//then default the character's appearance to a citizen.
				//(This could happen if the character was copied to a different hold.)
				this->wIdentity = M_CITIZEN1;
		}

		//Override visibility for non-graphic IDs.
		if (this->wIdentity >= CHARACTER_TYPES && IsVisible())
		{
			if (this->pCurrentGame)
				Disappear();
			else
				this->bVisible = false;
		}
	}
}

//*****************************************************************************
void CCharacter::SetCurrentGame(
//Sets current game pointer for monster.
//This is necessary for many methods of the monster class to work.
//
//Params:
	const CCurrentGame *pSetCurrentGame) //(in)
{
	CMonster::SetCurrentGame(pSetCurrentGame);

	//Check for a custom character.
	ResolveLogicalIdentity(this->pCurrentGame ? this->pCurrentGame->pHold : NULL);

	//Certain character types have special default traits.
	//Assign these when room play is first starting.
	if (pSetCurrentGame->wTurnNo == 0)
	{
		switch (GetIdentity())
		{
			case M_CITIZEN:
			case M_WUBBA:
				SetImperative(ScriptFlag::Invulnerable);
			break;
			case M_BEETHRO: case M_BEETHRO_IN_DISGUISE: case M_GUNTHRO:
			case M_CLONE:
			case M_HALPH: case M_HALPH2:
				SetImperative(ScriptFlag::MissionCritical);
			break;
			default: break;
		}
	}

	//If this NPC is a custom character with no script,
	//then use the default script for this custom character type.
	if (this->pCustomChar && this->commands.empty())
		LoadCommands(this->pCustomChar->ExtraVars, this->commands);

	//Global scripts started without commands should be flagged as done
	//and removed on room exit
	if (this->bGlobal && this->commands.empty())
		this->bScriptDone = true;
}

//*****************************************************************************
void CCharacter::SaveCommands(CDbPackedVars& ExtraVars, const COMMAND_VECTOR& commands)
{
	//First, save speech records owned by script commands to DB.
	SaveSpeech(commands);

	const UINT wNumCommands = commands.size();
	if (wNumCommands)
		ExtraVars.SetVar(numCommandsStr, wNumCommands);

	//Serialize commands into a single buffer.
	string buffer;
	SerializeCommands(buffer, commands);
	if (!buffer.empty())
		ExtraVars.SetVar(commandStr, (void*)(buffer.c_str()), buffer.size(), UVT_byte_buffer);
}

//*****************************************************************************
void CCharacter::SaveCommands(CDbPackedVars& ExtraVars, const COMMANDPTR_VECTOR& commands)
{
	//First, save speech records owned by script commands to DB.
	SaveSpeech(commands);

	const UINT wNumCommands = commands.size();
	if (wNumCommands)
		ExtraVars.SetVar(numCommandsStr, wNumCommands);

	//Serialize commands into a single buffer.
	string buffer;
	SerializeCommands(buffer, commands);
	if (!buffer.empty())
		ExtraVars.SetVar(commandStr, (void*)(buffer.c_str()), buffer.size(), UVT_byte_buffer);
}

//********************* Current serialization ********************************/

//*****************************************************************************
UINT CCharacter::readBpUINT(const BYTE* buffer, UINT& index)
//Deserialize 1..5 bytes --> UINT
{
	const BYTE *buffer2 = buffer + (index++);
	ASSERT(*buffer2); // should not be zero (indicating a negative number)
	UINT n = 0;
	for (;;index++)
	{
		n = (n << 7) + *buffer2;
		if (*buffer2++ & 0x80)
			break;
	}

	return n - 0x80;
}

//*****************************************************************************
void CCharacter::writeBpUINT(string& buffer, UINT n)
//Serialize UINT --> 1..5 bytes
{
	int s = 7;
	while ((n >> s) && s < 32)
		s += 7;

	while (s)
	{
		s -= 7;
		BYTE b = BYTE((n >> s) & 0x7f);
		if (!s)
			b |= 0x80;
		buffer.append(1, b);
	}
}

//*****************************************************************************
void CCharacter::DeserializeCommand(BYTE* buffer, UINT& index, CCharacterCommand& command)
//Extracts commands serialized in 'buffer' into a command.
{
	command.command = CCharacterCommand::CharCommand(readBpUINT(buffer, index));
	command.x = readBpUINT(buffer, index);
	command.y = readBpUINT(buffer, index);
	command.w = readBpUINT(buffer, index);
	command.h = readBpUINT(buffer, index);
	command.flags = readBpUINT(buffer, index);
	const UINT speechID = readBpUINT(buffer, index);
	if (speechID)
	{
		command.pSpeech = g_pTheDB->Speech.GetByID(speechID);
		ASSERT(command.pSpeech); //This was commented out in RPG: may still need to be commented out in F&M
	}

	const UINT labelSize = readBpUINT(buffer, index);
	if (labelSize)
	{
		const UINT wchars = labelSize/sizeof(WCHAR);
#if (GAME_BYTEORDER == GAME_BYTEORDER_BIG)
		WCHAR *pwzBuf = new WCHAR[wchars+1];
		memcpy((void*)pwzBuf, (const void*)(buffer + index), labelSize);
		LittleToBig(pwzBuf, wchars);
		command.label.assign(pwzBuf, wchars);
		delete[] pwzBuf;
#else
		command.label.assign((const WCHAR*)(buffer + index), wchars);
#endif
		index += labelSize;
	}
}

//*****************************************************************************
void CCharacter::DeserializeCommands(BYTE* buffer, const UINT bufferSize, COMMAND_VECTOR& commands)
//Extracts commands serialized in 'buffer' into 'commands'.
{
	UINT index=0;
	while (index < bufferSize)
	{
		CCharacterCommand command;
		DeserializeCommand(buffer, index, command);
		Upgrade2_0CommandTo3_0(command, commands);
	}
	ASSERT(index == bufferSize);
}

//*****************************************************************************
void CCharacter::DeserializeCommands(BYTE* buffer, const UINT bufferSize, COMMANDPTR_VECTOR& commands)
//Overloaded method with a vector of pointers to commands.
{
	UINT index=0;
	while (index < bufferSize)
	{
		CCharacterCommand *pCommand = new CCharacterCommand();
		DeserializeCommand(buffer, index, *pCommand);
		//Upgrade2_0CommandTo3_0(command, commands);
		commands.push_back(pCommand);
	}
	ASSERT(index == bufferSize);
}

//*****************************************************************************
void CCharacter::SerializeCommand(string& buffer, const CCharacterCommand& command)
//Outputs text containing the given command in serialized form.
{
	ASSERT(command.command < CCharacterCommand::CC_Count);

	writeBpUINT(buffer, command.command);
	writeBpUINT(buffer, command.x);
	writeBpUINT(buffer, command.y);
	writeBpUINT(buffer, command.w);
	writeBpUINT(buffer, command.h);
	writeBpUINT(buffer, command.flags);
	writeBpUINT(buffer, command.pSpeech ? command.pSpeech->dwSpeechID : 0);

	const UINT length = command.label.size() * sizeof(WCHAR);
	writeBpUINT(buffer, length);
#if (GAME_BYTEORDER == GAME_BYTEORDER_BIG)
	WCHAR *pBytes = new WCHAR[command.label.size()];
	memcpy(pBytes, command.label.c_str(), length);
	LittleToBig(pBytes, command.label.size());
	const string label(reinterpret_cast<const char*>(pBytes), length);
	buffer += label;
	delete[] pBytes;
#else
	const string label(reinterpret_cast<const char*>(command.label.c_str()), length);
	buffer += label;
#endif
}

//*****************************************************************************
void CCharacter::SerializeCommands(string& buffer, const COMMAND_VECTOR& commands)
//Returns: allocated byte array containing 'commands' in serialized form
{
	const UINT wNumCommands = commands.size();

	buffer.resize(0);
	buffer.reserve(wNumCommands * 10 * sizeof(UINT)); //heuristic

	//Pack each command in sequence.
	for (UINT wIndex=0; wIndex<wNumCommands; ++wIndex)
	{
		const CCharacterCommand& command = commands[wIndex];
		SerializeCommand(buffer, command);
	}
}

//*****************************************************************************
void CCharacter::SerializeCommands(string& buffer, const COMMANDPTR_VECTOR& commands)
//Overloaded method with a vector of pointers to commands.
{
	const UINT wNumCommands = commands.size();

	buffer.resize(0);
	buffer.reserve(wNumCommands * 10 * sizeof(UINT)); //heuristic

	//Pack each command in sequence.
	for (UINT wIndex=0; wIndex<wNumCommands; ++wIndex)
	{
		const CCharacterCommand *pCommand = commands[wIndex];
		SerializeCommand(buffer, *pCommand);
	}
}

//********************* 3.0.2 rev2 serialization *****************************/

//*****************************************************************************
UINT CCharacter::readUINT(const BYTE* buffer, UINT& index)
//Deserialize 4-bytes --> UINT
{
	ASSERT(sizeof(UINT) == 4);
	const BYTE *buffer2 = buffer + index;
	UINT n = 0;
	for (UINT i=4; i--; )
	{
		n <<= 8;
		n += UINT(buffer2[i]);
	}

	index += sizeof(UINT);

	return n;
}

//*****************************************************************************
void CCharacter::writeUINT(string& buffer, UINT n)
//Serialize UINT --> 4-bytes
{
	ASSERT(sizeof(UINT) == 4);
	for (UINT i=5; --i; n >>= 8)
		buffer.append(1, BYTE(n & 0xff));
}

//*****************************************************************************
void CCharacter::DeserializeCommands_3_0_2_2(BYTE* buffer, const UINT bufferSize, COMMAND_VECTOR& commands)
//Extracts commands serialized in 'buffer' into 'commands'.
//This data format was used briefly for game version 3.0.2 rev2.
{
	UINT index=0;
	while (index < bufferSize)
	{
		CCharacterCommand command;

		command.command = CCharacterCommand::CharCommand(buffer[index++]);
		command.x = readUINT(buffer, index);
		command.y = readUINT(buffer, index);
		command.w = readUINT(buffer, index);
		command.h = readUINT(buffer, index);
		command.flags = readUINT(buffer, index);
		const UINT speechID = readUINT(buffer, index);
		if (speechID)
		{
			command.pSpeech = g_pTheDB->Speech.GetByID(speechID);
			ASSERT(command.pSpeech);
		}

		const UINT labelSize = readUINT(buffer, index);
		string str;
		str.resize(labelSize);
		for (UINT i=0; i<labelSize; ++i)
			str[i] = buffer[index++];
		Base64::decode(str, command.label);

		Upgrade2_0CommandTo3_0(command, commands);
	}
	ASSERT(index == bufferSize);
}

//*****************************************************************************
/*
void CCharacter::SerializeCommands_3_0_2_2(string& buffer) const
//Returns: allocated byte array containing 'commands' in serialized form
{
	const UINT wNumCommands = this->commands.size();

	buffer.resize(0);
	buffer.reserve(wNumCommands * 10 * sizeof(UINT)); //heuristic

	//Pack each command in sequence.
	for (UINT wIndex=0; wIndex<wNumCommands; ++wIndex)
	{
		const CCharacterCommand& command = this->commands[wIndex];
		ASSERT(command.command < CCharacterCommand::CC_Count);

		buffer.append(1, BYTE(command.command));
		writeUINT(buffer, command.x);
		writeUINT(buffer, command.y);
		writeUINT(buffer, command.w);
		writeUINT(buffer, command.h);
		writeUINT(buffer, command.flags);
		writeUINT(buffer, command.pSpeech ? command.pSpeech->dwSpeechID : 0);

		const string label = Base64::encode(command.label);
		writeUINT(buffer, label.size());
		buffer += label;
	}
}
*/

//*****************************************************************************
void CCharacter::SetExtraVarsFromMembers(const bool bHoldChar)
//Packs command script.
{
	SetExtraVarsFromMembersWithoutScript(bHoldChar);

	//Save command sequence (script) in packed vars member.
	SaveCommands(this->ExtraVars, this->commands);
}

//*****************************************************************************
void CCharacter::SetExtraVarsFromMembersWithoutScript(const bool bHoldChar)
//Packs NPC state info.
{
	//Only save info currently in NPC object's data structures.
	this->ExtraVars.Clear();

	if (!bHoldChar)
	{
		this->ExtraVars.SetVar(idStr, this->wLogicalIdentity);
		this->ExtraVars.SetVar(visibleStr, this->bVisible);
	}

	//Stats.
	if (this->color)
		this->ExtraVars.SetVar(ColorStr, this->color);
	if (this->sword != NPC_DEFAULT_SWORD)
		this->ExtraVars.SetVar(SwordStr, this->sword);
	if (this->paramX != NO_OVERRIDE)
		this->ExtraVars.SetVar(ParamXStr, this->paramX);
	if (this->paramY != NO_OVERRIDE)
		this->ExtraVars.SetVar(ParamYStr, this->paramY);
	if (this->paramW != NO_OVERRIDE)
		this->ExtraVars.SetVar(ParamWStr, this->paramW);
	if (this->paramH != NO_OVERRIDE)
		this->ExtraVars.SetVar(ParamHStr, this->paramH);
	if (this->paramF != NO_OVERRIDE)
		this->ExtraVars.SetVar(ParamFStr, this->paramF);

	//ASSERT(this->dwScriptID);
	this->ExtraVars.SetVar(scriptIDstr, this->dwScriptID);
}

//*****************************************************************************
void CCharacter::SetBaseMembersFromExtraVars()
//Use default values if these packed vars do not exist.
//Otherwise, override them.
{
	this->wLogicalIdentity = this->ExtraVars.GetVar(idStr, 0);

	this->wIdentity = this->wLogicalIdentity; //by default, these are the same

	this->bVisible = this->ExtraVars.GetVar(visibleStr, false);
	if (!this->bVisible)
		this->bSwordSheathed = true;

	//Stats.
	this->color = this->ExtraVars.GetVar(ColorStr, this->color);
	this->sword = this->ExtraVars.GetVar(SwordStr, this->sword);
	this->paramX = this->ExtraVars.GetVar(ParamXStr, this->paramX);
	this->paramY = this->ExtraVars.GetVar(ParamYStr, this->paramY);
	this->paramW = this->ExtraVars.GetVar(ParamWStr, this->paramW);
	this->paramH = this->ExtraVars.GetVar(ParamHStr, this->paramH);
	this->paramF = this->ExtraVars.GetVar(ParamFStr, this->paramF);
}

//*****************************************************************************
void CCharacter::SetMembersFromExtraVars()
//Reads vars from ExtraVars to reconstruct the character's ID and command sequence.
{
	SetBaseMembersFromExtraVars();

	LoadCommands(this->ExtraVars, this->commands);

	this->dwScriptID = this->ExtraVars.GetVar(scriptIDstr, UINT(0));
}

//*****************************************************************************
void CCharacter::Upgrade2_0CommandTo3_0(CCharacterCommand& command, COMMAND_VECTOR& commands)
//Upgrading of deprecated 2.0 commands to 3.0 replacements.
{
	switch (command.command)
	{
		case CCharacterCommand::CC_GotoIf:
			command.command = CCharacterCommand::CC_If;
			//rest of upgrade gets handled after next command is processed
		break;

		case CCharacterCommand::CC_WaitForMonster:
			command.command = CCharacterCommand::CC_WaitForRect;
			command.flags = ScriptFlag::MONSTER;
		break;
		case CCharacterCommand::CC_WaitForNotMonster:
			command.command = CCharacterCommand::CC_WaitForNotRect;
			command.flags = ScriptFlag::MONSTER;
		break;

		case CCharacterCommand::CC_WaitForHalph:
			command.command = CCharacterCommand::CC_WaitForRect;
			command.flags = ScriptFlag::HALPH;
		break;
		case CCharacterCommand::CC_WaitForNotHalph:
			command.command = CCharacterCommand::CC_WaitForNotRect;
			command.flags = ScriptFlag::HALPH;
		break;

		case CCharacterCommand::CC_WaitForCharacter:
			command.command = CCharacterCommand::CC_WaitForRect;
			command.flags = ScriptFlag::NPC;
		break;
		case CCharacterCommand::CC_WaitForNotCharacter:
			command.command = CCharacterCommand::CC_WaitForNotRect;
			command.flags = ScriptFlag::NPC;
		break;

		case CCharacterCommand::CC_SetMusic:
			//Update 2.0 style song refs to the current style naming system.
			if ((int)command.x >= 5)
			{
				string songName;
				switch (command.x)
				{
					case 5: songName = "FoundationExit"; break;
					case 6: songName = "Deep SpacesExit"; break;
					case 7: songName = "IceworksExit"; break;
					case 8: songName = "FoundationAmbient"; break;
					case 9: songName = "FoundationAttack"; break;
					case 10: songName = "FoundationPuzzle"; break;
					case 11: songName = "Deep SpacesAmbient"; break;
					case 12: songName = "Deep SpacesAttack"; break;
					case 13: songName = "Deep SpacesPuzzle"; break;
					case 14: songName = "IceworksAmbient"; break;
					case 15: songName = "IceworksAttack"; break;
					case 16: songName = "IceworksPuzzle"; break;
					default: break; //don't change others
				}
				AsciiToUnicode(songName.c_str(), command.label);
				command.x = 0;
			}
		break;

		case CCharacterCommand::CC_Question:
		case CCharacterCommand::CC_AnswerOption:
			//Update 2.0-format questions and answers to use speech objects
			//to reference message texts instead of a single label text
			//to facilitate localization.
			if (!command.pSpeech)
			{
				command.pSpeech = g_pTheDB->Speech.GetNew();
				command.pSpeech->MessageText = command.label.c_str();
				command.pSpeech->Update();
				command.label.resize(0);
			}
		break;

		default: break; //all other commands are current
	}

	commands.push_back(command);

	//Complete upgrading a half-changed GotoIf command.
	if (commands.size() >= 2)
	{
		CCharacterCommand& oldCommand=commands[commands.size()-2];
		if (oldCommand.command == CCharacterCommand::CC_If && oldCommand.x)
		{
			CCharacterCommand& condition=commands.back();
			if (condition.command == CCharacterCommand::CC_Label)
			{
				//Special case: If ... goto Label won't work in 3.0 syntax.
				//Fortunately, it's a no-op, so removing the If ... goto
				//doesn't change the script semantic.
				commands.erase(commands.begin() + (commands.size()-2));
			} else {
				CCharacterCommand c;
				c.command = CCharacterCommand::CC_GoTo;
				c.x = oldCommand.x;
				oldCommand.x = 0;
				commands.push_back(c);
				CCharacterCommand cEnd;
				cEnd.command = CCharacterCommand::CC_IfEnd;
				commands.push_back(cEnd);
			}
		}
	}
}

//*****************************************************************************
void CCharacter::Save(
//Places monster object member vars into database view.
//
//Params:
	const c4_RowRef &MonsterRowRef,     //(in/out) Open view to fill.
	const bool bSaveScript) //whether to save the NPC script in packed vars [default=true]
{
	//Pack vars.
	if (bSaveScript)
		SetExtraVarsFromMembers();
	else
		SetExtraVarsFromMembersWithoutScript();

	CMonster::Save(MonsterRowRef);
}

//*****************************************************************************
void CCharacter::SaveSpeech(const COMMAND_VECTOR& commands)
//Save speech objects owned by script commands to the DB.
{
	//Save out any new speech records to DB.
	for (UINT wIndex=0; wIndex<commands.size(); ++wIndex)
	{
		const CCharacterCommand& command = commands[wIndex];
		if (command.pSpeech)
			command.pSpeech->Update();
	}
}

//*****************************************************************************
void CCharacter::SaveSpeech(const COMMANDPTR_VECTOR& commands)
//Save speech objects owned by script commands to the DB.
{
	//Save out any new speech records to DB.
	for (UINT wIndex=0; wIndex<commands.size(); ++wIndex)
	{
		const CCharacterCommand *pCommand = commands[wIndex];
		ASSERT(pCommand);
		if (pCommand->pSpeech)
			pCommand->pSpeech->Update();
	}
}

//*****************************************************************************
void CCharacter::Delete()
//Deletes speech records (exclusively) owned by character from DB.
{
	SetMembersFromExtraVars();
	for (UINT wIndex=this->commands.size(); wIndex--; )
	{
		if (this->commands[wIndex].pSpeech)
			g_pTheDB->Speech.Delete(this->commands[wIndex].pSpeech->dwSpeechID);
	}
}

//*****************************************************************************
bool CCharacter::SetSwordSheathed()
//Sets and returns whether NPC's sword is sheathed.
//Currently, this is based on whether double is standing on goo.
{
	if (CPlayerDouble::SetSwordSheathed())
		return true;
	//If player is marked to not have a sword, then NPC Beethro does not either.
	if (bIsSmitemaster(GetResolvedIdentity()) && this->pCurrentGame->swordsman.bSwordOff)
	{
		this->bSwordSheathed = true;
		return true;
	}
	return false;
}

//
// Private methods
//

//*****************************************************************************
void CCharacter::Disappear()
//Removes the NPC from the room, but not the monster list.
{
	ASSERT(this->pCurrentGame);
	ASSERT(this->pCurrentGame->pRoom);
	CDbRoom& room = *(this->pCurrentGame->pRoom);

	this->bVisible = false;
	this->bSwordSheathed = true;
	ASSERT(room.pMonsterSquares[room.ARRAYINDEX(this->wX,this->wY)] == this);
	room.pMonsterSquares[room.ARRAYINDEX(this->wX,this->wY)] = NULL;
}

//*****************************************************************************
int CCharacter::GetIndexOfCommandWithLabel(const UINT label) const
//Returns: index of command with specified label, or NO_LABEL if none
{
	if (label)
		for (UINT wIndex=this->commands.size(); wIndex--; )
		{
			const CCharacterCommand& command = this->commands[wIndex];
			if (command.command == CCharacterCommand::CC_Label &&
					label == command.x)
				return wIndex;
		}
	return NO_LABEL;
}

//*****************************************************************************
bool CCharacter::JumpToCommandWithLabel(const WCHAR *pText)
//Returns: true if label existed and jump succeeded, else false
{
	if (pText)
		for (UINT wIndex=this->commands.size(); wIndex--; )
		{
			const CCharacterCommand& command = this->commands[wIndex];
			if (command.command == CCharacterCommand::CC_Label &&
					!WCScmp(pText, command.label.c_str()))
			{
				this->wCurrentCommandIndex = wIndex;
				return true;
			}
		}
	return false;
}

//*****************************************************************************
bool CCharacter::JumpToCommandWithLabel(const UINT num)
{
	WCHAR temp[12];
	_itoW(num, temp, 10);
	return JumpToCommandWithLabel(temp);
}

//*****************************************************************************
void CCharacter::MoveCharacter(
//Handle an open move and incidental consequences.
//
//Params:
	const int dx, const int dy, //movement delta
	const bool bFaceDirection,
	CCueEvents& CueEvents)      //(in/out)
{
	//Before he moves, remember important square contents.
	CDbRoom& room = *(this->pCurrentGame->pRoom);
	const UINT wOTile = room.GetOSquare(this->wX, this->wY);
	const bool bWasOnTrapdoor = bIsTrapdoor(wOTile);
	const bool bWasOnPlatform = bIsPlatform(wOTile);

	Move(this->wX + dx, this->wY + dy, &CueEvents);
	this->wSwordMovement = nGetO(dx,dy);
	if (bFaceDirection)	//allow turning?
		SetOrientation(dx, dy);	//character faces the direction it actually moves

	//Special actions for human types.
	if (bIsHuman(GetResolvedIdentity()))
	{
		if (bWasOnTrapdoor && bIsBeethroDouble(GetResolvedIdentity()) && HasSword())
			room.DestroyTrapdoor(this->wX - dx, this->wY - dy, CueEvents);
		if (bWasOnPlatform)
		{
			const UINT wOTile = room.GetOSquare(this->wX, this->wY);
			if (bIsPit(wOTile) || bIsDeepWater(wOTile))
				room.MovePlatform(this->wX - dx, this->wY - dy, nGetO(dx,dy));
		}

		//Process any and all of these item interactions.
		UINT tTile = room.GetTSquare(this->wX, this->wY);
		if (tTile==T_MIRROR)
		{
			room.PushObject(this->wX, this->wY, this->wX + dx, this->wY + dy, CueEvents);
			tTile = room.GetTSquare(this->wX, this->wY); //also check what was under the mirror
		}
		if (tTile==T_TOKEN)
			room.ActivateToken(CueEvents, this->wX, this->wY);
	}

	SetSwordSheathed();

	//If player was stepped on, kill him.
	if (!this->bSafeToPlayer && this->pCurrentGame->IsPlayerAt(this->wX, this->wY))
	{
		CCurrentGame *pGame = (CCurrentGame*)this->pCurrentGame; //non-const
		pGame->SetDyingEntity(&pGame->swordsman, this);
		CueEvents.Add(CID_MonsterKilledPlayer, this);
	}
}

//*****************************************************************************
void CCharacter::TeleportCharacter(
	const UINT wDestX, const UINT wDestY, //(in) Destination square
	CCueEvents& CueEvents)      //(in/out)
//Moves a Character from one square to an entirely different one.
//Disappear/AppearAt logic is used for this, since it is not a standard move.
//Target square must be already clear to use: check before using this routine
{
	ASSERT(this->pCurrentGame);
	ASSERT(this->pCurrentGame->pRoom);
	CDbRoom& room = *(this->pCurrentGame->pRoom);

	if (this->bVisible)
	{
		ASSERT(room.pMonsterSquares[room.ARRAYINDEX(this->wX,this->wY)] == this);
		room.pMonsterSquares[room.ARRAYINDEX(this->wX,this->wY)] = NULL;
	}

	CCoordSet coords(this->wX, this->wY);
	coords.insert(wDestX, wDestY);
	this->wPrevX = this->wX;
	this->wPrevY = this->wY;
	this->wX = wDestX;
	this->wY = wDestY;

	if (this->bVisible)
	{
		ASSERT(!room.pMonsterSquares[room.ARRAYINDEX(this->wX,this->wY)]);
		room.pMonsterSquares[room.ARRAYINDEX(this->wX,this->wY)] = this;
		SetSwordSheathed();

		//Check for stepping on pressure plate.
		if (room.GetOSquare(this->wX, this->wY) == T_PRESSPLATE && !IsFlying())
			room.ActivateOrb(this->wX, this->wY, CueEvents, OAT_PressurePlate);
	}

	room.Plot(coords); //update room
}

//*****************************************************************************
void CCharacter::TurnIntoMonster(
//Replace with normal monster of specified type.
//
//Params:
	CCueEvents& CueEvents, //(in/out)
	const bool bSpecial)  //special transformation behavior [default=false]
{
	bool bActiveEye = false;
	UINT identity = GetIdentity();
	if (!IsValidMonsterType(identity))
	{
		if (identity != M_EYE_ACTIVE)
		{
			//This is a pseudo-monster type.  Just stop executing script.
			this->wCurrentCommandIndex = this->commands.size();

			//Special option: when the player is not in the room,
			//replace this character with the player at this location.
			if (this->pCurrentGame->swordsman.wIdentity == M_NONE &&
					identity >= CHARACTER_FIRST) //non-monster only
			{
				if (IsVisible())
					Disappear();
				CCurrentGame *pCurrentGame = const_cast<CCurrentGame*>(this->pCurrentGame);
				CSwordsman& player = pCurrentGame->swordsman;
				player.Move(this->wX, this->wY);
				player.Move(this->wX, this->wY); //removes prev coords
				player.SetOrientation(this->wO);
				pCurrentGame->SetPlayerSwordSheathed();
				pCurrentGame->SetPlayerRole(this->wLogicalIdentity, CueEvents);
			}

			return;
		}

		//Turn active evil eye into evil eye.
		identity = M_EYE;
		bActiveEye = true;
	}

	CCueEvents Ignored;
	this->pCurrentGame->pRoom->KillMonster(this, Ignored, true);

	if (!this->bVisible)
		return; //not in room -- don't change to real monster type

	CMonster *pMonster = this->pCurrentGame->pRoom->AddNewMonster(identity, this->wX, this->wY);
	switch (pMonster->wType)	//fix orientation
	{
		case M_SKIPPERNEST:
		case M_BRAIN: pMonster->wO = NO_ORIENTATION; break;
		case M_FEGUNDO:
			pMonster->wO = this->wO;
			if (bSpecial)
			{
				CPhoenix *pFegundo = DYN_CAST(CPhoenix*, CMonster*, pMonster);
				pFegundo->Explode(CueEvents);
			}
		break;
		case M_EYE:
			if (bActiveEye)
			{
				CEvilEye *pEye = DYN_CAST(CEvilEye*, CMonster*, pMonster);
				pEye->SetActive();
			}
		//NO BREAK
		default: pMonster->wO = this->wO; break;
	}

	if (bEntityHasSword(pMonster->wType))
	{
		CPlayerDouble *pPlayerDouble = DYN_CAST(CPlayerDouble*, CMonster*, pMonster);
		pPlayerDouble->SetSwordSheathed();
	}

	this->bReplaced = true;
	CueEvents.Add(CID_NPCTypeChange);
}
