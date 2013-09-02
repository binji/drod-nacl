#ifndef PLAYERSTATS_H
#define PLAYERSTATS_H

#include "DbPackedVars.h"
#include <BackEndLib/Wchar.h>
#include <string>
using std::string;

//Script variable manipulation.
//
//To maintain backwards compatibility, don't alter the enumeration values.
namespace ScriptVars
{
	enum Op
	{
		Assign=0,
		Inc=1,
		Dec=2,
		AssignText=3,
		AppendText=4,
		MultiplyBy=5,
		DivideBy=6,
		Mod=7
	};
	enum Comp
	{
		Equals=0,
		Greater=1,
		Less=2,
		EqualsText=3
	};

	//Predefined global and relative game state vars, accessed through these key values.
	enum Predefined
	{
		P_NoVar  =  0,
		P_SWORD = -1,
		P_MONSTER_COLOR = -2,
		P_MONSTER_SWORD = -3,
		P_PLAYER_X = -4,
		P_PLAYER_Y = -5,
		P_PLAYER_O = -6,
		P_MONSTER_X = -7,
		P_MONSTER_Y = -8,
		P_MONSTER_O = -9,
		P_TOTALMOVES = -10,
		P_TOTALTIME = -11,
		P_SCRIPT_X = -12,
		P_SCRIPT_Y = -13,
		P_SCRIPT_W = -14,
		P_SCRIPT_H = -15,
		P_SCRIPT_F = -16,
		FirstPredefinedVar = P_SCRIPT_F, //set this to the last var in the enumeration
		PredefinedVarCount = -int(FirstPredefinedVar)
	};

	void init();
	string getVarName(const ScriptVars::Predefined var);
	WSTRING getVarNameW(const ScriptVars::Predefined var);
	Predefined parsePredefinedVar(const string& str);
	Predefined parsePredefinedVar(const WSTRING& wstr);

	//All predefined vars.
	extern const char predefinedVarTexts[PredefinedVarCount][13];
	extern const UINT predefinedVarMIDs[PredefinedVarCount];
	extern string midTexts[PredefinedVarCount];

	//Global game var subset quick reference.
	static const UINT numGlobals=3;
	extern const Predefined globals[numGlobals];
	extern const UINT globalVarMIDs[numGlobals];
	extern const char* globalVarShortNames[numGlobals];
};

class PlayerStats
{
public:
	PlayerStats() {clear();}
	virtual ~PlayerStats() {}

	void Pack(CDbPackedVars& stats);
	void Unpack(CDbPackedVars& stats);

	virtual void clear() {
		sword = 0;
		totalMoves = totalTime = 0;
	}

	UINT getVar(const WSTRING& wstr) const
	{
		return getVar(ScriptVars::parsePredefinedVar(wstr));
	}
	UINT getVar(const ScriptVars::Predefined var) const;
	void setVar(const ScriptVars::Predefined var, const UINT val);

//protected:
	UINT sword;         //equipment
	UINT totalMoves, totalTime; //tally stats
};

//More stats used for various tally operations.
class RoomStats : public PlayerStats
{
public:
	RoomStats() : PlayerStats() {clear();}
	virtual ~RoomStats() {}

	virtual void clear() {
		PlayerStats::clear();
		yellowDoors = greenDoors = blueDoors = redDoors = blackDoors = 0;
		openYellowDoors = openGreenDoors = openBlueDoors = openRedDoors = openBlackDoors = 0;
		rooms = secrets = levels = 0;
	}

	UINT yellowDoors, greenDoors, blueDoors, redDoors, blackDoors;
	UINT openYellowDoors, openGreenDoors, openBlueDoors, openRedDoors, openBlackDoors;
	UINT rooms, secrets, levels;
};

#endif