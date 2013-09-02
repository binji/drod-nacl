#include "PlayerStats.h"
#include "Db.h"
#include "../Texts/MIDs.h"

using namespace ScriptVars;

//*****************************************************************************
//Shorter forms of the user-visible natural language var names.
//Used to pack and unpack the vars into a CDbPackedVars object.
const char ScriptVars::predefinedVarTexts[PredefinedVarCount][13] =
{
	"_SWORD",
	"_MyColor",
	"_MySword",
	"_X", "_Y", "_O",
	"_MyX", "_MyY", "_MyO",
	"_TotalMoves", "_TotalTime",
	"_MyScriptX", "_MyScriptY", "_MyScriptW", "_MyScriptH", "_MyScriptF"
};

//Message texts corresponding to the above short var texts.
const UINT ScriptVars::predefinedVarMIDs[PredefinedVarCount] = {
	MID_VarSword,
	MID_VarMonsterColor,
	MID_VarMonsterSword,
	MID_VarX, MID_VarY, MID_VarO,
	MID_VarMonsterX, MID_VarMonsterY, MID_VarMonsterO,
	MID_TotalMoves, MID_TotalTime,
	MID_VarMonsterParamX, MID_VarMonsterParamY, MID_VarMonsterParamW, MID_VarMonsterParamH, MID_VarMonsterParamF
};

string ScriptVars::midTexts[PredefinedVarCount]; //inited on first call

//*****************************************************************************
//Global game vars.  A subset of the predefined vars.
const Predefined ScriptVars::globals[numGlobals] = {
	P_SWORD,
	P_TOTALMOVES,
	P_TOTALTIME
};

//The MIDs for the global var subset.
const UINT ScriptVars::globalVarMIDs[numGlobals] = {
	predefinedVarMIDs[0],  //equipment

	predefinedVarMIDs[9], //tally stats
	predefinedVarMIDs[10]
};

//Match the global var texts in 'globalVarMIDs' to their short form texts in
//'predefinedVarTexts'.
const char* ScriptVars::globalVarShortNames[numGlobals] = {
	predefinedVarTexts[0],  //equipment

	predefinedVarTexts[9], //tally stats
	predefinedVarTexts[10]
};

//*****************************************************************************
string ScriptVars::getVarName(const Predefined var)
//Returns: pointer to the name of this pre-defined var, or empty string if no match
{
	if (var < FirstPredefinedVar)
		return string();

	init(); //ensure texts are populated

	const UINT index = -1 - FirstPredefinedVar;
	ASSERT(index < PredefinedVarCount);
	return midTexts[index];
}

//*****************************************************************************
WSTRING ScriptVars::getVarNameW(const Predefined var)
//Returns: pointer to the name of this pre-defined var, or NULL if no match
{
	string varName = getVarName(var);

	WSTRING wstr;
	AsciiToUnicode(varName.c_str(), wstr);
	return wstr;
}

//*****************************************************************************
void ScriptVars::init()
//Init 'midTexts' on first call.
//Much faster than repeated multiple DB queries.
{
	UINT index=0;
	if (midTexts[0].empty())
	{
		for (int i=-1; i>=FirstPredefinedVar; --i, ++index)
		{
			//Get user-readable form of var.
			ASSERT(index < PredefinedVarCount);
			const WCHAR *pText = g_pTheDB->GetMessageText(predefinedVarMIDs[index]);
			UnicodeToAscii(pText, midTexts[index]);
		}
		ASSERT(!midTexts[0].empty());
	}
}

//*****************************************************************************
Predefined ScriptVars::parsePredefinedVar(const WSTRING& wstr)
//Returns: the enumeration for this variable name, or P_NoVar if not recognized
{
	string str;
	UnicodeToAscii(wstr, str);
	return parsePredefinedVar(str);
}

//*****************************************************************************
Predefined ScriptVars::parsePredefinedVar(const string& str)
{
	init();

	const char *pText = str.c_str();
	UINT index=0;
	for (int i=-1; i>=FirstPredefinedVar; --i, ++index)
	{
/* //the user doesn't know about these string literals
		if (!stricmp(pText, predefinedVarTexts[index]))
			return Predefined(i);
*/

		//Compare against user-readable form of var.
		if (!_stricmp(pText, midTexts[index].c_str()))
			return Predefined(i);
	}
	return P_NoVar;
}

//*****************************************************************************
UINT PlayerStats::getVar(const Predefined var) const
//Gets specified var's value
{
	switch (var)
	{
		case P_SWORD: return this->sword;

		case P_TOTALMOVES: return this->totalMoves;
		case P_TOTALTIME: return this->totalTime;

		case P_NoVar:
		default: return 0;
	}
}

//*****************************************************************************
void PlayerStats::setVar(const Predefined var, const UINT val)
//Sets specified var to given value
{
	switch (var)
	{
		case P_SWORD: this->sword = val; break;

		case P_TOTALMOVES: this->totalMoves = val; break;
		case P_TOTALTIME: this->totalTime = val; break;

		case P_NoVar:
		default: break;
	}
}

//***************************************************************************************
void PlayerStats::Pack(CDbPackedVars& stats)
//Writes player RPG stats to the stats buffer.
{
	for (UINT i=PredefinedVarCount; i--; )
	{
		UINT val;
		switch (i)
		{
			//These case values correspond to the ordering of values in the 'Predefined' enumeration,
			//and not the (negative) enumeration values themselves.
			case 1: val = this->sword; break;

			case 10: val = this->totalMoves; break;
			case 11: val = this->totalTime; break;

			default: break;
		}

		if ((i > 1 && i < 10) || i > 11)
			continue; //these are not player stats

		stats.SetVar(predefinedVarTexts[i], val);
	}
}

//***************************************************************************************
void PlayerStats::Unpack(CDbPackedVars& stats)
//Reads player RPG stats from the stats buffer.
{
	for (UINT i=PredefinedVarCount; i--; )
	{
		if ((i > 1 && i < 10) || i > 11)
			continue; //these are not player stats

		UINT val = stats.GetVar(predefinedVarTexts[i], UINT(0));
		switch (i)
		{
			//These case values correspond to the ordering of values in the 'Predefined' enumeration,
			//and not the (negative) enumeration values themselves.
			case 1: this->sword = val; break;

			case 10: this->totalMoves = val; break;
			case 11: this->totalTime = val; break;

			default:
				ASSERT(!"Case should be skipped above");
				break;
		}
	}
}
