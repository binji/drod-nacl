// $Id: MonsterFactory.h 9822 2012-02-06 03:06:42Z TFMurphy $

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

//MonsterFactory.h
//Declarations for CMonsterFactory.
//Class for creating monster objects derived from CMonster.

#ifndef MONSTERFACTORY_H
#define MONSTERFACTORY_H

#include <BackEndLib/Types.h>

//How to add new NPCs.
//BackEnd:
//1. Add enumeration after CHARACTER_FIRST below.
//2. Add to SPEAKER enumeration and getSpeakerType().
//3a. Add to bEntityHasSword if NPC has a sword.
//3b. Add to export scripts, if needed.
//FrontEnd:
//4. To scripting dialog (speaker and graphic list boxes).
//5. Add to CharacterTileImageArray()
//6. Add to SpeakerConstant.
//7. Add subtitle color to SpeakerColor.

//Monster types.  Keep this in order to preserve their integer values.
enum MONSTERTYPE {
	M_ROACH=0,
	M_QROACH,
	M_REGG,
	M_GOBLIN,
	M_NEATHER,
	M_WWING,
	M_EYE,
	M_SERPENT,
	M_TARMOTHER,
	M_TARBABY,
	M_BRAIN,
	M_MIMIC,
	M_SPIDER,

	//2.0 and 3.0 monsters
	M_SERPENTG,
	M_SERPENTB,
	M_ROCKGOLEM,
	M_WATERSKIPPER,
	M_SKIPPERNEST,
	M_AUMTLICH,
	M_CLONE,
	M_DECOY,
	M_WUBBA,
	M_SEEP,
	M_STALWART,
	M_HALPH,
	M_SLAYER,
	M_FEGUNDO,
	M_FEGUNDOASHES,
	M_GUARD,
	M_CHARACTER,
	M_MUDMOTHER,
	M_MUDBABY,
	M_GELMOTHER,
	M_GELBABY,
	M_CITIZEN,
	M_ROCKGIANT,
	M_HALPH2,  //3.0 "skin" variant
	M_SLAYER2, //3.0 "skin" variant
	M_STALWART2, //4.0 "skin" variant

	MONSTER_TYPES, //end of real monster types

	//Character pseudo-types (speakers)
	CHARACTER_FIRST=10000,
	M_NEGOTIATOR=CHARACTER_FIRST,
	M_CITIZEN1,
	M_CITIZEN2,
	M_GOBLINKING,
	M_INSTRUCTOR, //deprecated 2.0 type
	M_MUDCOORDINATOR,
	M_TARTECHNICIAN,
	M_EYE_ACTIVE,
	M_BEETHRO,
	M_CITIZEN3,
	M_CITIZEN4,
	M_BEETHRO_IN_DISGUISE,
	M_GUNTHRO,

	CHARACTER_TYPES,

	CUSTOM_CHARACTER_FIRST=20000, //for custom character IDs

	M_PLAYER = static_cast<UINT>(-3),
	M_CUSTOM = static_cast<UINT>(-2),
	M_NONE = static_cast<UINT>(-1)
};

//for automatic (convenient) inclusion into all monster types
#include "CueEvents.h"
#include "CurrentGame.h"
#include "DbRooms.h"	// circular dependency, needs MONSTERTYPE
#include "GameConstants.h"
#include "TileConstants.h"

static inline bool IsValidMonsterType(const UINT mt) {return (mt<MONSTER_TYPES);}

static inline bool bIsMother(const UINT mt) {
	switch (mt)
	{
		case M_TARMOTHER: case M_MUDMOTHER: case M_GELMOTHER: return true;
		default: return false;
	}
}

static inline bool bIsMonsterTarstuff(const UINT mt) {
	switch (mt)
	{
		case M_TARMOTHER: case M_MUDMOTHER: case M_GELMOTHER:
		case M_TARBABY: case M_MUDBABY: case M_GELBABY:
			return true;
		default: return false;
	}
}

//The standard player character roles
static inline bool bIsSmitemaster(const UINT mt){
	switch (mt)
	{
		case M_BEETHRO: case M_GUNTHRO:
			return true;
		default:
			return false;
	}
}

static inline bool bIsBeethroDouble(const UINT mt) {
	switch (mt)
	{
		case M_MIMIC: case M_CLONE: case M_DECOY:
		case M_BEETHRO: case M_GUNTHRO:
			return true;
		//Don't include M_BEETHRO_IN_DISGUISE because that type is functionally
		//different from Beethro.
		default:
			return false;
	}
}

static inline bool bIsStalwart(const UINT mt){
	switch (mt)
	{
		case M_STALWART: case M_STALWART2:
			return true;
		default:
			return false;
	}
}

//Any sworded monster type.
static inline bool bEntityHasSword(const UINT mt) {
	switch (mt)
	{
		case M_MIMIC: case M_CLONE: case M_DECOY:
		case M_SLAYER:	case M_SLAYER2:
		case M_GUARD: case M_STALWART: case M_STALWART2:
		case M_BEETHRO: case M_BEETHRO_IN_DISGUISE:
		case M_GUNTHRO:
			return true;
		default:
			return false;
	}
}

//All human types.
static inline bool bIsHuman(const UINT mt) {
	switch (mt)
	{
		case M_MIMIC: case M_CLONE: case M_DECOY:
		case M_SLAYER:	case M_SLAYER2:
		case M_GUARD: case M_STALWART: case M_STALWART2:
		case M_BEETHRO: case M_BEETHRO_IN_DISGUISE: case M_GUNTHRO:
		case M_HALPH: case M_HALPH2: case M_NEATHER:
		case M_CITIZEN: case M_CITIZEN1: case M_CITIZEN2: case M_CITIZEN3: case M_CITIZEN4:
		case M_INSTRUCTOR: case M_NEGOTIATOR:
		case M_MUDCOORDINATOR: case M_TARTECHNICIAN:
			return true;
		default:
			return false;
	}
}

//All non-Beethro types that monsters can sense as an enemy.
static inline bool bIsMonsterTarget(const UINT mt) {
	switch (mt)
	{
		case M_BEETHRO: case M_GUNTHRO: case M_CLONE: case M_DECOY:
		case M_STALWART: case M_STALWART2:
			return true;
		default:
			return false;
	}
}

static inline bool bIsEntityTypeVulnerableToHeat(const UINT mt) {
	switch (mt)
	{
		case M_TARMOTHER: case M_TARBABY:
		case M_MUDMOTHER: case M_MUDBABY:
		case M_GELMOTHER: case M_GELBABY:
		case M_NONE:
			return false;
		default:
			return true;
	}
}

static inline bool bIsEntityFlying(const UINT mt) {
	switch (mt)
	{
		case M_WWING: case M_FEGUNDO:
			return true;
		default:
			return false;
	}
}

static inline bool bIsEntitySwimming(const UINT mt) {
	return mt == M_WATERSKIPPER || mt == M_SKIPPERNEST;
}

static inline bool bIsSerpent(const UINT mt) {return mt==M_SERPENT || mt==M_SERPENTG || mt==M_SERPENTB;}

static inline bool bCanEntityHideInShallowWater(const UINT mt) {
	switch(mt)
	{
		case M_BEETHRO: case M_GUNTHRO: case M_DECOY: case M_CLONE: case M_MIMIC:
			return true;
		default:
			return false;
	}
}

static inline bool bCanEntityWadeInShallowWater(const UINT mt) {
	return mt != M_STALWART && mt != M_HALPH &&
			(bIsHuman(mt) || bIsSerpent(mt) || mt == M_BRAIN || mt == M_SPIDER
			|| mt == M_GELBABY || mt == M_ROCKGOLEM || mt == M_ROCKGIANT);
}

class CMonster;
class CMonsterFactory
{
public:
	CMonsterFactory(CCurrentGame *pSetCurrentGame = NULL) : pCurrentGame(pSetCurrentGame) {}
	~CMonsterFactory() {}
	CMonster *GetNewMonster(const MONSTERTYPE eType);

	CCurrentGame *pCurrentGame;
};

#endif //...#ifndef MONSTERFACTORY_H
