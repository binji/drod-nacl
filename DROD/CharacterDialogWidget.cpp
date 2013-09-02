// $Id: CharacterDialogWidget.cpp 10007 2012-03-24 18:18:29Z mrimer $

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
 * Portions created by the Initial Developer are Copyright (C) 2002, 2005
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "CharacterDialogWidget.h"
#include "DrodBitmapManager.h"
#include "DrodFontManager.h"
#include "EditRoomScreen.h"
#include "FaceWidget.h"
#include <FrontEndLib/ButtonWidget.h>
#include <FrontEndLib/ImageWidget.h>
#include <FrontEndLib/LabelWidget.h>
#include <FrontEndLib/ListBoxWidget.h>
#include <FrontEndLib/TextBoxWidget.h>
#include <FrontEndLib/OptionButtonWidget.h>

#include "../DRODLib/Db.h"
#include "../DRODLib/MonsterFactory.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Files.h>
#include <BackEndLib/Wchar.h>
#include <BackEndLib/Clipboard.h>

#include <wctype.h>

//NOTE: tag #'s should not conflict with other widgets on screen
const UINT TAG_CANCEL = 999;
const UINT TAG_GRAPHICLISTBOX = 998;
const UINT TAG_COMMANDSLISTBOX = 997;
const UINT TAG_ADDCOMMAND = 996;
const UINT TAG_DELETECOMMAND = 995;
const UINT TAG_ISVISIBLE = 994;
const UINT TAG_CHARACTERS = 993;

const UINT TAG_ACTIONLISTBOX = 989;
const UINT TAG_EVENTLISTBOX = 988;
const UINT TAG_DELAY = 987;
const UINT TAG_WAIT = 986;
const UINT TAG_SPEAKERLISTBOX = 985;
const UINT TAG_MOODLISTBOX = 984;
const UINT TAG_SPEECHTEXT = 983;
const UINT TAG_ADDSOUND = 982;
const UINT TAG_SOUNDNAME_LABEL = 981;
const UINT TAG_DIRECTIONLISTBOX = 980;
const UINT TAG_ONOFFLISTBOX = 979;
const UINT TAG_ONOFFLISTBOX2 = 978;
const UINT TAG_OPENCLOSELISTBOX = 977;
const UINT TAG_GOTOLABELTEXT = 976;
const UINT TAG_GOTOLABELLISTBOX = 975;
const UINT TAG_MUSICLISTBOX = 974;
const UINT TAG_WAITFLAGSLISTBOX = 973;
const UINT TAG_MOVERELX = 972;
const UINT TAG_MOVERELY = 971;
const UINT TAG_IMPERATIVELISTBOX = 970;

const UINT TAG_EVENTLABEL = 969;
const UINT TAG_WAITLABEL = 968;
const UINT TAG_DELAYLABEL = 967;
const UINT TAG_SPEAKERLABEL = 966;
const UINT TAG_MOODLABEL = 965;
const UINT TAG_TEXTLABEL = 964;
const UINT TAG_DIRECTIONLABEL = 963;
const UINT TAG_GOTOLABEL = 962;
const UINT TAG_DISPLAYSPEECHLABEL = 961;
const UINT TAG_MUSICLABEL = 960;
const UINT TAG_NOTURNING = 959;
const UINT TAG_SINGLESTEP = 958;
const UINT TAG_CUTSCENELABEL = 957;
const UINT TAG_MOVERELXLABEL = 956;
const UINT TAG_MOVERELYLABEL = 955;
const UINT TAG_LOOPSOUND = 954;
const UINT TAG_WAITABSLABEL = 953;
const UINT TAG_SKIPENTRANCELABEL = 952;
const UINT TAG_TESTSOUND = 951;

const UINT TAG_VARADD = 949;
const UINT TAG_VARREMOVE = 948;
const UINT TAG_VARLIST = 947;
const UINT TAG_VAROPLIST = 946;
const UINT TAG_VARCOMPLIST = 945;
const UINT TAG_VARNAMETEXTLABEL = 944;
const UINT TAG_VARVALUELABEL = 943;
const UINT TAG_VARVALUE = 942;

const UINT TAG_GRAPHICLISTBOX2 = 939;
const UINT TAG_ITEMLISTBOX = 938;

const UINT TAG_CHARACTERNAME = 929;
const UINT TAG_ADDCHARACTER = 928;
const UINT TAG_CHARACTERLISTBOX = 927;
const UINT TAG_CHARGRAPHICLISTBOX = 926;
const UINT TAG_AVATARFACE = 925;
const UINT TAG_CUSTOMAVATAR = 924;
const UINT TAG_DELETECHARACTER = 923;
const UINT TAG_DEFAULTAVATAR = 922;
const UINT TAG_TILESIMAGE = 921;
const UINT TAG_CUSTOMTILES = 920;
const UINT TAG_DEFAULTTILES = 919;

const UINT TAG_WATERTRAVERSALLISTBOX = 918;
const UINT TAG_EDITDEFAULTSCRIPT = 917;
const UINT TAG_CUSTOM_NPCS = 916;
const UINT TAG_GLOBALSCRIPTLISTBOX = 915;
//const UINT TAG_CUSTOM_NPC_ID = 914;
const UINT TAG_DIRECTIONLISTBOX2 = 910;
const UINT TAG_VISUALEFFECTS_LISTBOX = 909;
const UINT TAG_DIRECTIONLISTBOX3 = 908;
const UINT TAG_ONOFFLISTBOX3 = 907;
const UINT TAG_DIRECTIONLABEL2 = 906;
const UINT TAG_SOUNDEFFECTLABEL = 905;

const UINT TAG_ADDCOMMAND2 = 899;
const UINT TAG_DELETECOMMAND2 = 898;
const UINT TAG_DEFAULTCOMMANDSLISTBOX = 897;
const UINT TAG_OK2 = 896;

const UINT MAX_TEXT_LABEL_SIZE = 100;

const UINT CX_DIALOG = 820;
const UINT CY_DIALOG = 688;

const UINT CX_SPACE = 15;
const UINT CY_SPACE = 10;

const SURFACECOLOR PaleRed = {255, 192, 192};

#define NOT_FOUND (UINT(-1))

void stripTrailingWhitespace(WSTRING& text)
{
	UINT textLength = text.length();
	while (textLength && iswspace(text[textLength-1]))
		text.resize(--textLength);
}

//******************************************************************************
CRenameDialogWidget::CRenameDialogWidget(
//Constructor.
//
//Params:
	const UINT dwSetTagNo,                //(in)   Required params for CDialogWidget
	const int nSetX, const int nSetY,      //    constructor.
	const UINT wSetW, const UINT wSetH,    //
	const bool bListBoxDoubleClickReturns) //[default = false]
	: CDialogWidget(dwSetTagNo, nSetX, nSetY, wSetW, wSetH, bListBoxDoubleClickReturns)
{
}

//*****************************************************************************
void CRenameDialogWidget::OnBetweenEvents()
{
	//When reentering the dialog, after having prompted the user for parameter
	//info for a custom NPC default script command, reenter the default script dialog.
	if (GetTagNo() == TAG_CUSTOM_NPCS)
	{
		CCharacterDialogWidget *pParent = DYN_CAST(CCharacterDialogWidget*,
				CWidget*, this->pParent);
		if (pParent->IsEditingDefaultScript())
		{
			pParent->EditDefaultScriptForCustomNPC();

			//If the parent has been flagged to deactivate while editing the
			//default script, this dialog should be deactivated now.
			if (pParent->IsDeactivating() && !IsDeactivating())
				Deactivate();
		}
	}
}

//*****************************************************************************
void CRenameDialogWidget::OnDoubleClick(
//Handles double click event.
//
//Params:
	const UINT dwTagNo) //(in) Widget event applies to.
{
	CDialogWidget::OnDoubleClick(dwTagNo);
	if (IsDeactivating())
		return;

	//ClickedSelection is queried in the methods called below.

	switch (dwTagNo)
	{
		case TAG_VARLIST:
		{
			CCharacterDialogWidget *pParent = DYN_CAST(CCharacterDialogWidget*,
						CWidget*, this->pParent);
			pParent->RenameVar();
		}
		break;

		case TAG_CHARACTERLISTBOX:
		{
			CCharacterDialogWidget *pParent = DYN_CAST(CCharacterDialogWidget*,
						CWidget*, this->pParent);
			pParent->RenameCharacter();
		}
		break;

		case TAG_DEFAULTCOMMANDSLISTBOX:
		{
			CCharacterDialogWidget *pParent = DYN_CAST(CCharacterDialogWidget*,
						CWidget*, this->pParent);
			pParent->EditClickedCommand();

			if (pParent->IsDeactivating())
			{
				if (!IsDeactivating())
					Deactivate(); //exit to prompt user
			}
		}
		break;

		default: return; //don't need to redraw anything
	}

	if (this->pParent)
		this->pParent->Paint();   //refresh screen
	Paint();
}

//*****************************************************************************
void CRenameDialogWidget::OnKeyDown(
//Called when widget receives SDL_KEYDOWN event.
//
//Params:
	const UINT dwTagNo,       //(in)   Widget that event applied to.
	const SDL_KeyboardEvent &Key) //(in)   Event.
{
	//Trap ESC so it doesn't close the parent dialog.
	if (Key.keysym.sym == SDLK_ESCAPE)
	{
		CWidget *pWidget = GetWidget(TAG_OK2);
		if (!pWidget)
			pWidget = GetWidget(TAG_OK);
		if (pWidget)
			OnClick(pWidget->GetTagNo()); //deactivate
		return;
	}

	CCharacterDialogWidget *pParent = DYN_CAST(CCharacterDialogWidget*,
			CWidget*, this->pParent);
	if (pParent->IsEditingDefaultScript())
		pParent->OnKeyDown(dwTagNo, Key);
}

//*****************************************************************************
void CRenameDialogWidget::OnRearranged(const UINT dwTagNo)
//Called when the default commands list has been reordered.
{
	CCharacterDialogWidget *pParent = DYN_CAST(CCharacterDialogWidget*,
			CWidget*, this->pParent);
	if (pParent->IsEditingDefaultScript())
		pParent->OnRearranged(dwTagNo);
}

//*****************************************************************************
CCharacterDialogWidget::CCharacterDialogWidget(
//Constructor.
//
//Params:
	const UINT dwSetTagNo,                //(in)   Required params for CWidget
	const int nSetX, const int nSetY)         //    constructor.
	: CDialogWidget(dwSetTagNo, nSetX, nSetY, CX_DIALOG, CY_DIALOG)
	, pGraphicListBox(NULL), pCommandsListBox(NULL)
	, pAddCommandDialog(NULL), pAddCharacterDialog(NULL)
	, pDefaultScriptCommandsListBox(NULL)
	, pScriptDialog(NULL)
	, pDirectionListBox2(NULL), pDirectionListBox3(NULL)
	, pOnOffListBox3(NULL)
	, pVisualEffectsListBox(NULL)
	, pActionListBox(NULL), pEventListBox(NULL)
	, pSpeakerListBox(NULL), pMoodListBox(NULL)
	, pDirectionListBox(NULL)
	, pWaterTraversalListBox(NULL), pGlobalScriptListBox(NULL)
	, pOnOffListBox(NULL), pOnOffListBox2(NULL), pOpenCloseListBox(NULL)
	, pGotoLabelListBox(NULL), pMusicListBox(NULL)
	, pVarListBox(NULL), pVarOpListBox(NULL), pVarCompListBox(NULL)
	, pWaitFlagsListBox(NULL), pImperativeListBox(NULL), pBuildItemsListBox(NULL)
	, pCharNameText(NULL), pCharListBox(NULL)

	, pCharacter(NULL)
	, pCommand(NULL)
	, pSound(NULL)
	, wIncrementedLabel(0)
	, bEditingCommand(false), bRetainFields(false)
	, bEditingDefaultScript(false)
	, defaultScriptCustomCharID(0)
{
	static const UINT CX_TITLE = 240;
	static const UINT CY_TITLE = 30;
	static const int X_TITLE = (CX_DIALOG - CX_TITLE) / 2;
	static const int Y_TITLE = CY_SPACE;

	static const int X_COMMANDSLABEL = CX_SPACE;
	static const int Y_COMMANDSLABEL = Y_TITLE + CY_TITLE + CY_SPACE;
	static const UINT CX_COMMANDSLABEL = 110;
	static const UINT CY_COMMANDSLABEL = 30;
	static const int X_COMMANDS = X_COMMANDSLABEL;
	static const int Y_COMMANDS = Y_COMMANDSLABEL + CY_COMMANDSLABEL;
	static const UINT CX_COMMANDS = 610;
	static const UINT CY_COMMANDS = 25*22 + 4;
#ifdef RUSSIAN_BUILD
	static const UINT CX_ADDCOMMAND = 180;
	static const UINT CX_DELETECOMMAND = 180;
#else
	static const UINT CX_ADDCOMMAND = 130;
	static const UINT CX_DELETECOMMAND = 150;
#endif
	static const int X_ADDCOMMAND = X_COMMANDS + CX_COMMANDS - CX_ADDCOMMAND - CX_SPACE;
	static const int Y_ADDCOMMAND = Y_COMMANDSLABEL - 4;
	static const UINT CY_ADDCOMMAND = CY_STANDARD_BUTTON;
	static const int X_DELETECOMMAND = X_ADDCOMMAND - CX_DELETECOMMAND - CX_SPACE;
	static const int Y_DELETECOMMAND = Y_ADDCOMMAND;
	static const UINT CY_DELETECOMMAND = CY_STANDARD_BUTTON;

	static const UINT CX_GRAPHICLISTBOX = 170;
	static const int X_GRAPHICLABEL = CX_DIALOG - CX_GRAPHICLISTBOX - CX_SPACE;
	static const int Y_GRAPHICLABEL = Y_ADDCOMMAND;
	static const UINT CX_GRAPHICLABEL = 80;
	static const UINT CY_GRAPHICLABEL = 30;

	static const int X_CHARACTERS = X_GRAPHICLABEL;
	static const int Y_CHARACTERS = Y_COMMANDS;
	static const UINT CX_CHARACTERS = CX_GRAPHICLISTBOX - CX_SPACE*2;
	static const UINT CY_CHARACTERS = CY_STANDARD_BUTTON;

	static const int X_GRAPHICLISTBOX = X_GRAPHICLABEL;
	static const int Y_GRAPHICLISTBOX = Y_CHARACTERS + CY_CHARACTERS + CY_SPACE;
	static const UINT CY_GRAPHICLISTBOX = 23*22 + 4;

	static const int X_ISVISIBLE = X_GRAPHICLABEL;
	static const int Y_ISVISIBLE = Y_GRAPHICLISTBOX + CY_GRAPHICLISTBOX;
	static const UINT CX_ISVISIBLE = CX_GRAPHICLISTBOX;
	static const UINT CY_ISVISIBLE = CY_STANDARD_OPTIONBUTTON;

	static const UINT CX_BUTTON = 70;
	static const int X_OKBUTTON = (CX_DIALOG - (CX_BUTTON + CX_SPACE)) / 2;
	static const int Y_OKBUTTON = CY_DIALOG - CY_STANDARD_BUTTON - CY_SPACE;

	AddWidget(new CLabelWidget(0L, X_TITLE, Y_TITLE, CX_TITLE, CY_TITLE,
			F_Header, g_pTheDB->GetMessageText(MID_CustomizeCharacter)));

	//Commands.
	AddWidget(new CButtonWidget(TAG_ADDCOMMAND, X_ADDCOMMAND, Y_ADDCOMMAND,
			CX_ADDCOMMAND, CY_ADDCOMMAND, g_pTheDB->GetMessageText(MID_AddCommand)));
	AddWidget(new CButtonWidget(TAG_DELETECOMMAND, X_DELETECOMMAND, Y_DELETECOMMAND,
			CX_DELETECOMMAND, CY_DELETECOMMAND, g_pTheDB->GetMessageText(MID_DeleteCommand)));

	AddWidget(new CLabelWidget(0L, X_COMMANDSLABEL, Y_COMMANDSLABEL,
			CX_COMMANDSLABEL, CY_COMMANDSLABEL, F_Small, g_pTheDB->GetMessageText(MID_Commands)));
	this->pCommandsListBox = new CListBoxWidget(TAG_COMMANDSLISTBOX, X_COMMANDS, Y_COMMANDS,
			CX_COMMANDS, CY_COMMANDS, false, true, true);
	AddWidget(this->pCommandsListBox);

	//Appearance (character/tile graphic).
	CButtonWidget *pButton = new CButtonWidget(TAG_CHARACTERS,
			X_CHARACTERS, Y_CHARACTERS, CX_CHARACTERS, CY_CHARACTERS,
			g_pTheDB->GetMessageText(MID_Characters));
	AddWidget(pButton);

	AddWidget(new CLabelWidget(0L, X_GRAPHICLABEL, Y_GRAPHICLABEL,
			CX_GRAPHICLABEL, CY_GRAPHICLABEL, F_Small, g_pTheDB->GetMessageText(MID_Graphic)));
	this->pGraphicListBox = new CListBoxWidget(TAG_GRAPHICLISTBOX,
			X_GRAPHICLISTBOX, Y_GRAPHICLISTBOX, CX_GRAPHICLISTBOX, CY_GRAPHICLISTBOX, true);
	AddWidget(this->pGraphicListBox);

	this->pIsVisibleButton = new COptionButtonWidget(TAG_ISVISIBLE,
			X_ISVISIBLE, Y_ISVISIBLE, CX_ISVISIBLE, CY_ISVISIBLE,
			g_pTheDB->GetMessageText(MID_IsVisible), false);
	AddWidget(this->pIsVisibleButton);

	//OK/cancel buttons.
	pButton = new CButtonWidget(
			TAG_OK, X_OKBUTTON, Y_OKBUTTON, CX_BUTTON, CY_STANDARD_BUTTON,
			g_pTheDB->GetMessageText(MID_Okay));
	AddWidget(pButton);

	AddCommandDialog();
	AddCharacterDialog();
	AddScriptDialog();
}

//*****************************************************************************
CCharacterDialogWidget::~CCharacterDialogWidget()
{
	ASSERT(this->commandBuffer.empty());
}

//*****************************************************************************
void CCharacterDialogWidget::OnBetweenEvents()
{
	//When reentering the dialog, after having prompted the user for parameter
	//info for a custom NPC default script command, reenter the custom character
	//dialog in order to pop up the default script dialog again.
	if (this->bEditingDefaultScript)
	{
		//Select the custom NPC whose script is being edited.
		ASSERT(this->defaultScriptCustomCharID);
		this->pCharListBox->SelectItem(this->defaultScriptCustomCharID);

		EditCustomCharacters();
	}
}

//*****************************************************************************
void CCharacterDialogWidget::FinishCommand(
//Finish filling in the parameters to the command being added.
//
//Params:
	const UINT wX, const UINT wY, //(in)
	const UINT wW, const UINT wH) //(in) [default=-1]
{
	ASSERT(this->pCommand);
	this->pCommand->x = wX;
	this->pCommand->y = wY;
	if (wW != static_cast<UINT>(-1))
		this->pCommand->w = wW;
	if (wH != static_cast<UINT>(-1))
		this->pCommand->h = wH;
	AddCommand();
	SetWidgetStates();
	ASSERT(!this->pCommand);
}

//*****************************************************************************
void CCharacterDialogWidget::FinishCommandAndExit()
//If a command being defined can not be completed, exit the dialog gracefully,
//leaving it in a proper state.
{
	FinishCommand(0,0);

	//Save the NPC and script data in its current state to the room/hold.
	UpdateCharacter();

	if (this->bEditingDefaultScript)
		FinishEditingDefaultScript();
}

//*****************************************************************************
bool CCharacterDialogWidget::RenameCharacter()
//Prompt the user to rename the selected custom hold character.
//
//Returns: whether a character was renamed
{
	if (!this->pCharListBox->ClickedSelection())
		return false;

	const UINT charID = this->pCharListBox->GetSelectedItem();
	if (!charID)
		return false;

	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));
	ASSERT(pEditRoomScreen);
	ASSERT(pEditRoomScreen->pHold);

	WSTRING wstr = this->pCharListBox->GetSelectedItemText();
	const UINT answerTagNo = pEditRoomScreen->ShowTextInputMessage(
			MID_RenameCharacterPrompt, wstr, false, true);
	if (answerTagNo != TAG_OK)
		return false;
	ASSERT(!wstr.empty());

	if (!pEditRoomScreen->pHold->RenameCharacter(charID, wstr))
	{
		pEditRoomScreen->ShowOkMessage(MID_CharNameDuplicationError);
		return false;
	}

	this->pCharListBox->SetSelectedItemText(wstr.c_str());
	this->pCharListBox->Paint();
	return true;
}

//*****************************************************************************
bool CCharacterDialogWidget::RenameVar()
//Prompt the user to rename the selected hold variable.
//
//Returns: whether a var was renamed
{
	if (!this->pVarListBox->ClickedSelection())
		return false;

	const UINT varID = this->pVarListBox->GetSelectedItem();
	if (!varID)
		return false;

	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));
	ASSERT(pEditRoomScreen);
	ASSERT(pEditRoomScreen->pHold);

	//Not allowed to rename predefined variables.
	if (varID >= (UINT)ScriptVars::FirstPredefinedVar)
	{
		pEditRoomScreen->ShowOkMessage(MID_VarRenameNotAllowed);
		return false;
	}

	bool bGoodSyntax;
	WSTRING wstr;
	do {
		wstr = this->pVarListBox->GetSelectedItemText();
		const UINT answerTagNo = pEditRoomScreen->ShowTextInputMessage(
				MID_RenameVariablePrompt, wstr, false, true);
		if (answerTagNo != TAG_OK)
			return false;
		ASSERT(!wstr.empty());

		bGoodSyntax = CDbHold::IsVarNameGoodSyntax(wstr.c_str());
		if (!bGoodSyntax)
			pEditRoomScreen->ShowOkMessage(MID_VarNameSyntaxError);
	} while (!bGoodSyntax);

	if (!pEditRoomScreen->pHold->RenameVar(varID, wstr))
	{
		pEditRoomScreen->ShowOkMessage(MID_VarNameDuplicationError);
		return false;
	}

	this->pVarListBox->SetSelectedItemText(wstr.c_str());
	this->pVarListBox->Paint();
	return true;
}

//*****************************************************************************
void CCharacterDialogWidget::AddCharacterDialog()
//Create another dialog for managing custom characters.
//Separated from constructor for readability.
{
	static const UINT CX_CHAR_DIALOG = 820;
	static const UINT CY_CHAR_DIALOG = 650;

	static const UINT CY_TITLE = 30;
	static const int Y_TITLE = CY_SPACE;

	static const int X_TEXTLABEL = CX_SPACE;
	static const int Y_TEXTLABEL = Y_TITLE + CY_TITLE;
	static const UINT CX_TEXTLABEL = 220;
	static const UINT CY_TEXTLABEL = 30;
	static const int X_TEXT = X_TEXTLABEL;
	static const int Y_TEXT = Y_TEXTLABEL + CY_TEXTLABEL;
	static const UINT CX_TEXT = CX_TEXTLABEL;
	static const UINT CY_TEXT = 30;

	static const int X_ADDCHAR = X_TEXT + CX_TEXT + CX_SPACE;
	static const int Y_ADDCHAR = Y_TEXT;
#ifdef RUSSIAN_BUILD
	static const int CX_ADDCHAR = 200;
#else
	static const int CX_ADDCHAR = 130;
#endif
	static const int CY_ADDCHAR = CY_STANDARD_BUTTON;

	static const int X_CHARLABEL = X_TEXTLABEL;
	static const int Y_CHARLABEL = Y_TEXT + CY_TEXT + CY_SPACE*2;
	static const UINT CX_CHARLABEL = CX_TEXT;
	static const UINT CY_CHARLABEL = CY_TEXTLABEL;
	static const int X_CHARLISTBOX = X_CHARLABEL;
	static const int Y_CHARLISTBOX = Y_CHARLABEL + CY_CHARLABEL;
	static const UINT CX_CHARLISTBOX = CX_TEXT;
	static const UINT CY_CHARLISTBOX = 16*22 + 4; //16 slots

	static const int X_EDITDEFAULTSCRIPT = X_CHARLISTBOX;
	static const int Y_EDITDEFAULTSCRIPT = Y_CHARLISTBOX + CY_CHARLISTBOX + CY_SPACE/2;
	static const int CX_EDITDEFAULTSCRIPT = 170;
	static const int CY_EDITDEFAULTSCRIPT = CY_STANDARD_BUTTON;
	static const int Y_DELETE = Y_EDITDEFAULTSCRIPT + CY_EDITDEFAULTSCRIPT + CY_SPACE;

	static const int X_DELETE = X_CHARLISTBOX;
	static const int CX_DELETE = 100;
	static const int CY_DELETE = CY_STANDARD_BUTTON;

	static const int X_GRAPHICLABEL = X_CHARLABEL + CX_CHARLABEL + CX_SPACE;
	static const int Y_GRAPHICLABEL = Y_CHARLABEL;
	static const UINT CX_GRAPHICLABEL = 190;
	static const UINT CY_GRAPHICLABEL = CY_CHARLABEL;
	static const int X_GRAPHICLISTBOX3 = X_GRAPHICLABEL;
	static const int Y_GRAPHICLISTBOX3 = Y_CHARLISTBOX;
	static const UINT CX_GRAPHICLISTBOX3 = CX_GRAPHICLABEL;
	static const UINT CY_GRAPHICLISTBOX3 = CY_CHARLISTBOX;

	static const int X_AVATARLABEL = X_GRAPHICLABEL + CX_GRAPHICLABEL + CX_SPACE;
	static const int Y_AVATARLABEL = Y_GRAPHICLABEL;
	static const UINT CY_AVATARLABEL = CY_TEXTLABEL;

	static const int X_AVATAR = X_AVATARLABEL;
	static const int Y_AVATAR = Y_AVATARLABEL + CY_AVATARLABEL;
	static const UINT CX_AVATAR = CX_FACE;
	static const UINT CY_AVATAR = CY_FACE;
	static const UINT CX_AVATARLABEL = CX_AVATAR;

	static const int X_SETAVATAR = X_AVATAR;
	static const int Y_SETAVATAR = Y_AVATAR + CY_AVATAR + CY_SPACE;
#ifdef RUSSIAN_BUILD
	static const int CX_SETAVATAR = 230;
	static const int CX_DEFAULTAVATAR = 160;
#else
	static const int CX_SETAVATAR = 130;
	static const int CX_DEFAULTAVATAR = CX_SETAVATAR;
#endif
	static const int CY_SETAVATAR = CY_STANDARD_BUTTON;

	static const int X_DEFAULTAVATAR = X_AVATAR;
	static const int Y_DEFAULTAVATAR = Y_SETAVATAR + CY_SETAVATAR + CY_SPACE/2;
	static const int CY_DEFAULTAVATAR = CY_SETAVATAR;

	static const int X_TILESLABEL = X_DEFAULTAVATAR + CX_DEFAULTAVATAR + CX_SPACE;
	static const int Y_TILESLABEL = Y_GRAPHICLABEL;
	static const UINT CX_TILESLABEL = 100;
	static const UINT CY_TILESLABEL = CY_TEXTLABEL;

	static const int X_TILES = X_TILESLABEL;
	static const int Y_TILES = Y_TILESLABEL + CY_TILESLABEL;
//	static const UINT CX_TILES = 9 * CDrodBitmapManager::CX_TILE;
	static const UINT CY_TILES = 4 * CDrodBitmapManager::CY_TILE;

	static const int X_SETTILES = X_TILES;
	static const int Y_SETTILES = Y_TILES + CY_TILES + CY_SPACE;
#ifdef RUSSIAN_BUILD
	static const int CX_SETTILES = 170;
	static const int CX_DEFAULTTILES = 160;
#else
	static const int CX_SETTILES = 130;
	static const int CX_DEFAULTTILES = CX_SETTILES;
#endif
	static const int CY_SETTILES = CY_STANDARD_BUTTON;

	static const int X_DEFAULTTILES = X_TILES;
	static const int Y_DEFAULTTILES = Y_SETTILES + CY_SETTILES + CY_SPACE/2;
	static const int CY_DEFAULTTILES = CY_SETTILES;

	static const UINT CX_OKBUTTON = 70;
	static const int X_OKBUTTON = (CX_CHAR_DIALOG - CX_OKBUTTON) / 2;
	static const int Y_OKBUTTON = CY_CHAR_DIALOG - CY_STANDARD_BUTTON - CY_SPACE;

	ASSERT(!this->pAddCharacterDialog);
	this->pAddCharacterDialog = new CRenameDialogWidget(TAG_CUSTOM_NPCS,
			-175, GetY() + (GetH()-CY_CHAR_DIALOG)/2,
			CX_CHAR_DIALOG, CY_CHAR_DIALOG);
	CLabelWidget *pTitle = new CLabelWidget(0L, 0, Y_TITLE,
			CX_CHAR_DIALOG, CY_TITLE, F_Header, g_pTheDB->GetMessageText(MID_CharacterManagement));
	pTitle->SetAlign(CLabelWidget::TA_CenterGroup);
	this->pAddCharacterDialog->AddWidget(pTitle);

	//New character.
	this->pAddCharacterDialog->AddWidget(new CLabelWidget(0L, X_TEXTLABEL,
			Y_TEXTLABEL, CX_TEXTLABEL, CY_TEXTLABEL, F_Small, g_pTheDB->GetMessageText(MID_NewCharacterName)));
	this->pCharNameText = new CTextBoxWidget(TAG_CHARACTERNAME, X_TEXT, Y_TEXT,
			CX_TEXT, CY_TEXT);
	this->pAddCharacterDialog->AddWidget(this->pCharNameText);
	CButtonWidget *pButton = new CButtonWidget(TAG_ADDCHARACTER, X_ADDCHAR,
			Y_ADDCHAR, CX_ADDCHAR, CY_ADDCHAR, g_pTheDB->GetMessageText(MID_AddCharacter));
	this->pAddCharacterDialog->AddWidget(pButton);

	//Character list.
	this->pAddCharacterDialog->AddWidget(new CLabelWidget(0L, X_CHARLABEL, Y_CHARLABEL,
			CX_CHARLABEL, CY_CHARLABEL, F_Small, g_pTheDB->GetMessageText(MID_Characters)));
	this->pCharListBox = new CListBoxWidget(TAG_CHARACTERLISTBOX,
			X_CHARLISTBOX, Y_CHARLISTBOX, CX_CHARLISTBOX, CY_CHARLISTBOX, true);
	this->pAddCharacterDialog->AddWidget(this->pCharListBox);

	//Edit NPC default script button.
	pButton = new CButtonWidget(TAG_EDITDEFAULTSCRIPT, X_EDITDEFAULTSCRIPT, Y_EDITDEFAULTSCRIPT,
			CX_EDITDEFAULTSCRIPT, CY_EDITDEFAULTSCRIPT, g_pTheDB->GetMessageText(MID_EditDefaultScript));
	this->pAddCharacterDialog->AddWidget(pButton);

	//Delete button.
	pButton = new CButtonWidget(TAG_DELETECHARACTER, X_DELETE, Y_DELETE,
			CX_DELETE, CY_DELETE, g_pTheDB->GetMessageText(MID_Delete));
	this->pAddCharacterDialog->AddWidget(pButton);

	//Graphic list.
	this->pAddCharacterDialog->AddWidget(new CLabelWidget(0L, X_GRAPHICLABEL, Y_GRAPHICLABEL,
			CX_GRAPHICLABEL, CY_GRAPHICLABEL, F_Small, g_pTheDB->GetMessageText(MID_Graphic)));
	CListBoxWidget *pGraphicListBox = new CListBoxWidget(TAG_CHARGRAPHICLISTBOX,
			X_GRAPHICLISTBOX3, Y_GRAPHICLISTBOX3, CX_GRAPHICLISTBOX3, CY_GRAPHICLISTBOX3,
			true);
	PopulateGraphicListBox(pGraphicListBox);
	pGraphicListBox->AddItem(M_BEETHRO_IN_DISGUISE, g_pTheDB->GetMessageText(MID_BeethroInDisguise));
	pGraphicListBox->SelectItem(M_BEETHRO);
	this->pAddCharacterDialog->AddWidget(pGraphicListBox);

	//Avatar.
	this->pAddCharacterDialog->AddWidget(new CLabelWidget(0L, X_AVATARLABEL, Y_AVATARLABEL,
			CX_AVATARLABEL, CY_AVATARLABEL, F_Small, g_pTheDB->GetMessageText(MID_Avatar)));
	CFaceWidget *pFace = new CFaceWidget(TAG_AVATARFACE,
			X_AVATAR, Y_AVATAR, CX_AVATAR, CY_AVATAR);
	this->pAddCharacterDialog->AddWidget(pFace);
	pFace->PaintFull();
	pButton = new CButtonWidget(TAG_CUSTOMAVATAR, X_SETAVATAR, Y_SETAVATAR,
			CX_SETAVATAR, CY_SETAVATAR, g_pTheDB->GetMessageText(MID_CustomAvatar));
	this->pAddCharacterDialog->AddWidget(pButton);
	pButton = new CButtonWidget(TAG_DEFAULTAVATAR, X_DEFAULTAVATAR, Y_DEFAULTAVATAR,
			CX_DEFAULTAVATAR, CY_DEFAULTAVATAR, g_pTheDB->GetMessageText(MID_Default));
	this->pAddCharacterDialog->AddWidget(pButton);

	//Tiles.
	this->pAddCharacterDialog->AddWidget(new CLabelWidget(0L, X_TILESLABEL, Y_TILESLABEL,
			CX_TILESLABEL, CY_TILESLABEL, F_Small, g_pTheDB->GetMessageText(MID_Tiles)));
	this->pAddCharacterDialog->AddWidget(new CImageWidget(TAG_TILESIMAGE, X_TILES,
			Y_TILES, wszEmpty));
	pButton = new CButtonWidget(TAG_CUSTOMTILES, X_SETTILES, Y_SETTILES,
			CX_SETTILES, CY_SETTILES, g_pTheDB->GetMessageText(MID_CustomTiles));
	this->pAddCharacterDialog->AddWidget(pButton);
	pButton = new CButtonWidget(TAG_DEFAULTTILES, X_DEFAULTTILES, Y_DEFAULTTILES,
			CX_DEFAULTTILES, CY_DEFAULTTILES, g_pTheDB->GetMessageText(MID_Default));
	this->pAddCharacterDialog->AddWidget(pButton);

	//OK.
	pButton = new CButtonWidget(TAG_OK, X_OKBUTTON, Y_OKBUTTON, CX_OKBUTTON,
			CY_STANDARD_BUTTON, g_pTheDB->GetMessageText(MID_Okay));
	this->pAddCharacterDialog->AddWidget(pButton);

	//Add to main dialog.
	AddWidget(this->pAddCharacterDialog);
	this->pAddCharacterDialog->Center();
	this->pAddCharacterDialog->Hide();
}

//*****************************************************************************
void CCharacterDialogWidget::AddCommandDialog()
//Create another dialog for entering a script command.
//Separated from constructor for readability.
{
	static const UINT CY_COMMAND_DIALOG = 412;
	static const UINT CY_ACTIONLISTBOX = 13*22+4;

#ifdef RUSSIAN_BUILD
	static const UINT CX_COMMAND_DIALOG = 795;
	static const UINT CX_TITLE = 350;
	static const UINT CX_ACTIONLISTBOX = 265;
#else
	static const UINT CX_COMMAND_DIALOG = 775;
	static const UINT CX_TITLE = 250;
	static const UINT CX_ACTIONLISTBOX = 245;
#endif
	static const UINT CY_TITLE = 30;
	static const int X_TITLE = (CX_COMMAND_DIALOG - CX_TITLE) / 2;
	static const int Y_TITLE = CY_SPACE;

	static const int X_ACTIONLABEL = CX_SPACE;
	static const int Y_ACTIONLABEL = Y_TITLE + CY_TITLE;
	static const UINT CX_ACTIONLABEL = 90;
	static const UINT CY_ACTIONLABEL = 30;
	static const int X_ACTIONLISTBOX = X_ACTIONLABEL;
	static const int Y_ACTIONLISTBOX = Y_ACTIONLABEL + CY_ACTIONLABEL;

	static const UINT CX_EVENTLISTBOX = 350;
	static const UINT CY_EVENTLISTBOX = CY_ACTIONLISTBOX;
	static const int X_EVENTLABEL = X_ACTIONLISTBOX + CX_ACTIONLISTBOX + CX_SPACE;
	static const int Y_EVENTLABEL = Y_ACTIONLABEL;
	static const UINT CX_EVENTLABEL = CX_EVENTLISTBOX;
	static const UINT CY_EVENTLABEL = 30;
	static const int X_EVENTLISTBOX = X_EVENTLABEL;
	static const int Y_EVENTLISTBOX = Y_EVENTLABEL + CY_EVENTLABEL;

	static const int X_WAITLABEL = X_EVENTLISTBOX;
	static const int Y_WAITLABEL = Y_ACTIONLABEL;
#ifdef RUSSIAN_BUILD
	static const UINT CX_WAITLABEL = 165;
#else
	static const UINT CX_WAITLABEL = 130;
#endif
	static const UINT CY_WAITLABEL = 30;
	static const int X_WAIT = X_WAITLABEL;
	static const int Y_WAIT = Y_WAITLABEL + CY_WAITLABEL;
	static const UINT CX_WAIT = 60;
	static const UINT CY_WAIT = 30;

	static const int X_TEXTLABEL = X_WAITLABEL;
	static const int Y_TEXTLABEL = Y_ACTIONLABEL;
	static const UINT CX_TEXTLABEL = 130;
	static const UINT CY_TEXTLABEL = CY_WAITLABEL;
	static const int X_TEXT = X_TEXTLABEL;
	static const int Y_TEXT = Y_TEXTLABEL + CY_TEXTLABEL;
	static const UINT CX_TEXT = CX_COMMAND_DIALOG - X_TEXT - CX_SPACE*2;
	static const UINT CY_TEXT = CY_WAIT;

	static const int X_ADDSOUND = X_TEXTLABEL;
	static const int Y_ADDSOUND = Y_TEXT + CY_TEXT + CY_SPACE;
	static const int CX_ADDSOUND = 130;
	static const int CY_ADDSOUND = CY_STANDARD_BUTTON;
	static const int X_TESTSOUND = X_ADDSOUND + CX_ADDSOUND + CX_SPACE;
	static const int Y_TESTSOUND = Y_ADDSOUND;
	static const int CX_TESTSOUND = 80;
	static const UINT CY_TESTSOUND = CY_ADDSOUND;
	static const int X_SOUNDNAMELABEL = X_TESTSOUND + CX_TESTSOUND + CX_SPACE/2;
	static const int Y_SOUNDNAMELABEL = Y_TESTSOUND;
	static const int CX_SOUNDNAMELABEL = CX_COMMAND_DIALOG - X_SOUNDNAMELABEL - CX_SPACE;
	static const UINT CY_SOUNDNAMELABEL = CY_TEXT;

	static const int X_DELAYLABEL = X_WAITLABEL;
	static const int Y_DELAYLABEL = Y_ADDSOUND + CY_ADDSOUND + CY_SPACE;
#ifdef RUSSIAN_BUILD
	static const UINT CX_DELAYLABEL = 160;
	static const UINT CX_DELAY = 80;
#else
	static const UINT CX_DELAYLABEL = 130;
	static const UINT CX_DELAY = 60;
#endif
	static const UINT CY_DELAYLABEL = CY_WAITLABEL;
	static const int X_DELAY = X_DELAYLABEL;
	static const int Y_DELAY = Y_DELAYLABEL + CY_DELAYLABEL;
	static const UINT CY_DELAY = CY_WAIT;

#ifdef RUSSIAN_BUILD
	static const UINT CX_SPEAKERLISTBOX = 190;
#else
	static const UINT CX_SPEAKERLISTBOX = 200;
#endif
	static const int X_SPEAKERLABEL = X_DELAYLABEL + CX_DELAYLABEL + CX_SPACE;
	static const int Y_SPEAKERLABEL = Y_DELAYLABEL;
	static const UINT CX_SPEAKERLABEL = CX_SPEAKERLISTBOX;
	static const UINT CY_SPEAKERLABEL = CY_WAITLABEL;
	static const int X_SPEAKERLISTBOX = X_SPEAKERLABEL;
	static const int Y_SPEAKERLISTBOX = Y_SPEAKERLABEL + CY_SPEAKERLABEL;
	static const UINT CY_SPEAKERLISTBOX = 7*22+4;

	static const UINT CX_MOODLISTBOX = 100;
	static const int X_MOODLABEL = X_SPEAKERLABEL + CX_SPEAKERLABEL + CX_SPACE;
	static const int Y_MOODLABEL = Y_DELAYLABEL;
	static const UINT CX_MOODLABEL = CX_MOODLISTBOX;
	static const UINT CY_MOODLABEL = CY_WAITLABEL;
	static const int X_MOODLISTBOX = X_MOODLABEL;
	static const int Y_MOODLISTBOX = Y_MOODLABEL + CY_MOODLABEL;
	static const UINT CY_MOODLISTBOX = 5*22+4;

	static const UINT CX_BUTTON = 80;
	static const int X_OKBUTTON = (CX_COMMAND_DIALOG - (CX_BUTTON + CX_SPACE) * 2) / 2;
	static const int Y_OKBUTTON = CY_COMMAND_DIALOG - CY_STANDARD_BUTTON - CY_SPACE;
	static const int X_CANCELBUTTON = X_OKBUTTON + CX_BUTTON + CX_SPACE;
	static const int Y_CANCELBUTTON = Y_OKBUTTON;

	static const UINT CX_DIRECTIONLISTBOX = 210;
	static const UINT CY_DIRECTIONLISTBOX = 10*22+4;
	static const int X_DIRECTIONLABEL = X_EVENTLISTBOX;
	static const int Y_DIRECTIONLABEL = Y_ACTIONLABEL;
	static const UINT CX_DIRECTIONLABEL = CX_EVENTLISTBOX;
	static const UINT CY_DIRECTIONLABEL = 30;
	static const int X_DIRECTIONLISTBOX = X_EVENTLABEL;
	static const int Y_DIRECTIONLISTBOX = Y_EVENTLABEL + CY_EVENTLABEL;

	static const UINT CX_ONOFFLISTBOX = 100;
	static const UINT CY_ONOFFLISTBOX = 53;
	static const int X_ONOFFLISTBOX = X_EVENTLISTBOX;
	static const int Y_ONOFFLISTBOX = Y_ACTIONLISTBOX;

	static const int X_ONOFFLISTBOX2 = X_ONOFFLISTBOX + CX_ONOFFLISTBOX + CX_SPACE;

	static const UINT CX_WATERTRAVERSALLISTBOX = 250;
	static const UINT CY_WATERTRAVERSALLISTBOX = 4*22 + 4;
	static const int X_WATERTRAVERSALLISTBOX = X_ONOFFLISTBOX;
	static const int Y_WATERTRAVERSALLISTBOX = Y_ONOFFLISTBOX;

	static const UINT CX_IMPERATIVELISTBOX = 250;
	static const UINT CY_IMPERATIVELISTBOX = 12*22 + 4;
	static const int X_IMPERATIVELISTBOX = X_ONOFFLISTBOX;
	static const int Y_IMPERATIVELISTBOX = Y_ONOFFLISTBOX;

	static const UINT CX_OPENCLOSELISTBOX = 100;
	static const UINT CY_OPENCLOSELISTBOX = 53;
	static const int X_OPENCLOSELISTBOX = X_EVENTLISTBOX;
	static const int Y_OPENCLOSELISTBOX = Y_ACTIONLISTBOX;

	static const int X_GOTOLABEL = X_WAITLABEL;
	static const int Y_GOTOLABEL = Y_WAITLABEL;
	static const UINT CX_GOTOLABEL = CX_TEXTLABEL;
	static const UINT CY_GOTOLABEL = CY_WAITLABEL;
	static const int X_GOTOLABELTEXT = X_DELAYLABEL;
	static const int Y_GOTOLABELTEXT = Y_GOTOLABEL + CY_GOTOLABEL;
	static const UINT CX_GOTOLABELTEXT = CX_TEXT;
	static const UINT CY_GOTOLABELTEXT = CY_WAIT;

	static const UINT CX_GOTOLABELLISTBOX = CX_GOTOLABELTEXT;
	static const UINT CY_GOTOLABELLISTBOX = 10*22+4;
	static const int X_GOTOLABELLISTBOX = X_EVENTLISTBOX;
	static const int Y_GOTOLABELLISTBOX = Y_GOTOLABELTEXT + CY_GOTOLABELTEXT + CY_SPACE;

	static const int X_DISPLAYSPEECHLABEL = X_WAITLABEL;
	static const int Y_DISPLAYSPEECHLABEL = Y_WAITLABEL;
	static const UINT CX_DISPLAYSPEECHLABEL = CX_WAITLABEL;
	static const UINT CY_DISPLAYSPEECHLABEL = CY_WAITLABEL;

	static const int X_NOTURNINGLABEL = X_ONOFFLISTBOX;
	static const int Y_NOTURNINGLABEL = Y_WAITLABEL;
	static const UINT CX_NOTURNINGLABEL = CX_TEXTLABEL;
	static const UINT CY_NOTURNINGLABEL = CY_WAITLABEL;

	static const int X_SINGLESTEPLABEL = X_ONOFFLISTBOX2;
	static const int Y_SINGLESTEPLABEL = Y_WAITLABEL;
	static const UINT CX_SINGLESTEPLABEL = CX_TEXTLABEL;
	static const UINT CY_SINGLESTEPLABEL = CY_WAITLABEL;

	static const int X_SKIPENTRANCE = X_ONOFFLISTBOX;
	static const int Y_SKIPENTRANCE = Y_WAITLABEL;
	static const UINT CX_SKIPENTRANCE = 200;
	static const UINT CY_SKIPENTRANCE = CY_WAITLABEL;

	static const UINT CX_MUSICLISTBOX = 450;
	static const int X_MUSICLABEL = X_DELAYLABEL;
	static const int Y_MUSICLABEL = Y_DELAYLABEL;
	static const UINT CX_MUSICLABEL = CX_MUSICLISTBOX;
	static const UINT CY_MUSICLABEL = CY_MUSICLABEL;
	static const UINT CY_MUSICLISTBOX = CY_ACTIONLISTBOX;
	static const int X_MUSICLISTBOX = X_EVENTLISTBOX;
	static const int Y_MUSICLISTBOX = Y_EVENTLISTBOX;

	static const int X_WAITFLAGSLISTBOX = X_MUSICLISTBOX;
	static const int Y_WAITFLAGSLISTBOX = Y_ONOFFLISTBOX + CY_ONOFFLISTBOX + CY_SPACE;
	static const UINT CX_WAITFLAGSLISTBOX = 100;
	static const UINT CY_WAITFLAGSLISTBOX = 9*22+4;

	//Widgets and for variable handling commands.
	static const int X_VARTEXTLABEL = X_WAITLABEL;
	static const int Y_VARTEXTLABEL = Y_WAITLABEL;
	static const UINT CX_VARTEXTLABEL = 190;
	static const UINT CY_VARTEXTLABEL = CY_WAITLABEL;

	static const UINT CX_VARLISTBOX = 280;
	static const UINT CY_VARLISTBOX = 10*22+4;
	static const int X_VARLISTBOX = X_EVENTLISTBOX;
	static const int Y_VARLISTBOX = Y_TEXT + CY_TEXT + CY_SPACE;

	static const int X_VARADD = X_VARLISTBOX + CX_VARLISTBOX + CX_SPACE/2;
	static const int Y_VARADD = Y_VARLISTBOX;
	static const int CX_VARADD = 90;
	static const int CY_VARADD = CY_STANDARD_BUTTON;

	static const int X_VARREMOVE = X_VARADD + CX_VARADD + CX_SPACE;
	static const int Y_VARREMOVE = Y_VARADD;
	static const int CX_VARREMOVE = 85;
	static const int CY_VARREMOVE = CY_VARADD;

	static const int X_VAROPLIST = X_VARADD;
	static const int Y_VAROPLIST = Y_VARREMOVE + CY_VARREMOVE + CY_SPACE;
	static const int CX_VAROPLIST = 100;
	static const int CY_VAROPLIST = 8*22 + 4;

	static const int X_VARCOMPLIST = X_VARADD;
	static const int Y_VARCOMPLIST = Y_VAROPLIST;
	static const int CX_VARCOMPLIST = CX_VAROPLIST;
	static const int CY_VARCOMPLIST = 4*22 + 4;

	static const int X_VARVALUELABEL = X_VAROPLIST + CX_VAROPLIST + CX_SPACE/2;
	static const UINT CY_VARVALUELABEL = CY_WAITLABEL;
	static const int Y_VARVALUELABEL = Y_VAROPLIST;
	static const UINT CX_VARVALUELABEL = 100;

	static const int X_VARVALUE = X_VARVALUELABEL;
	static const int Y_VARVALUE = Y_VARVALUELABEL + CY_VARVALUELABEL;
	static const UINT CX_VARVALUE = 90;
	static const UINT CY_VARVALUE = 30;

	static const int X_GRAPHICLISTBOX2 = X_EVENTLISTBOX;
	static const int Y_GRAPHICLISTBOX2 = Y_ACTIONLISTBOX;
	static const UINT CX_GRAPHICLISTBOX2 = 250;
	static const UINT CY_GRAPHICLISTBOX2 = CY_ACTIONLISTBOX;

	static const int X_GLOBALSCRIPTLISTBOX = X_GRAPHICLISTBOX2;
	static const int Y_GLOBALSCRIPTLISTBOX = Y_GRAPHICLISTBOX2;
	static const UINT CX_GLOBALSCRIPTLISTBOX = CX_GRAPHICLISTBOX2;
	static const UINT CY_GLOBALSCRIPTLISTBOX = CY_GRAPHICLISTBOX2;

	static const UINT CX_DIRECTIONLISTBOX2 = CX_DIRECTIONLISTBOX;
	static const UINT CY_DIRECTIONLISTBOX2 = 8*22+4;
	static const int X_DIRECTIONLABEL2 = X_GRAPHICLISTBOX2 + CX_GRAPHICLISTBOX2 + CX_SPACE;
	static const int Y_DIRECTIONLABEL2 = Y_DIRECTIONLABEL;
	static const UINT CX_DIRECTIONLABEL2 = CX_DIRECTIONLABEL;
	static const UINT CY_DIRECTIONLABEL2 = 30;
	static const int X_DIRECTIONLISTBOX2 = X_DIRECTIONLABEL2;
	static const int Y_DIRECTIONLISTBOX2 = Y_DIRECTIONLABEL2 + CY_DIRECTIONLABEL2;

	static const int X_DIRECTIONLISTBOX3 = X_DIRECTIONLISTBOX;
	static const int Y_DIRECTIONLISTBOX3 = Y_DIRECTIONLISTBOX;
	static const UINT CX_DIRECTIONLISTBOX3 = CX_DIRECTIONLISTBOX2;
	static const UINT CY_DIRECTIONLISTBOX3 = 9*22 + 4;

	static const int X_EFFECTLISTBOX = X_DIRECTIONLISTBOX3 + CX_DIRECTIONLISTBOX3 + CX_SPACE;
	static const int Y_EFFECTLISTBOX = Y_GRAPHICLISTBOX2;
	static const UINT CX_EFFECTLISTBOX = 200;
	static const UINT CY_EFFECTLISTBOX = CY_ACTIONLISTBOX;

	static const int X_SOUNDEFFECTLABEL = X_DIRECTIONLISTBOX3;
	static const int Y_SOUNDEFFECTLABEL = Y_DIRECTIONLISTBOX3 + CY_DIRECTIONLISTBOX3 + CY_SPACE/2;
	static const UINT CX_SOUNDEFFECTLABEL = CX_DIRECTIONLISTBOX3;
	static const UINT CY_SOUNDEFFECTLABEL = CY_WAITLABEL;

	static const int X_ONOFFLISTBOX3 = X_ONOFFLISTBOX;
	static const int Y_ONOFFLISTBOX3 = Y_DIRECTIONLISTBOX3 + CY_DIRECTIONLISTBOX3 + CY_SPACE/2 + CY_WAITLABEL;

	static const int X_ITEMLISTBOX = X_GRAPHICLISTBOX2;
	static const int Y_ITEMLISTBOX = Y_GRAPHICLISTBOX2;
	static const UINT CX_ITEMLISTBOX = 220;
	static const UINT CY_ITEMLISTBOX = CY_GRAPHICLISTBOX2;

	static const int X_CUTSCENELABEL = X_WAITLABEL;
	static const int Y_CUTSCENELABEL = Y_WAITLABEL;
	static const UINT CX_CUTSCENELABEL = 200;
	static const UINT CY_CUTSCENELABEL = CY_WAITLABEL;

	static const int X_MOVERELXLABEL = X_SINGLESTEPLABEL + CX_SINGLESTEPLABEL;
	static const int Y_MOVERELXLABEL = Y_WAITLABEL;
	static const UINT CX_MOVERELXLABEL = CX_TEXTLABEL;
	static const UINT CY_MOVERELXLABEL = CY_WAITLABEL;

	static const int X_MOVERELX = X_MOVERELXLABEL;
	static const int Y_MOVERELX = Y_MOVERELXLABEL + CY_MOVERELXLABEL;
	static const UINT CX_MOVERELX = 60;
	static const UINT CY_MOVERELX = CY_WAIT;

	static const int X_MOVERELYLABEL = X_MOVERELXLABEL + CX_MOVERELXLABEL;
	static const int Y_MOVERELYLABEL = Y_WAITLABEL;
	static const UINT CX_MOVERELYLABEL = CX_TEXTLABEL;
	static const UINT CY_MOVERELYLABEL = CY_WAITLABEL;

	static const int X_MOVERELY = X_MOVERELYLABEL;
	static const int Y_MOVERELY = Y_MOVERELYLABEL + CY_MOVERELYLABEL;
	static const UINT CX_MOVERELY = CX_MOVERELX;
	static const UINT CY_MOVERELY = CY_MOVERELX;

	static const int X_LOOPSOUND = X_WAITLABEL;
	static const int Y_LOOPSOUND = Y_WAITLABEL;
	static const UINT CX_LOOPSOUND = 200;
	static const UINT CY_LOOPSOUND = CY_WAITLABEL;

	ASSERT(!this->pAddCommandDialog);
	this->pAddCommandDialog = new CRenameDialogWidget(0L, 0, 0,
			CX_COMMAND_DIALOG, CY_COMMAND_DIALOG);
	this->pAddCommandDialog->AddWidget(new CLabelWidget(0L, X_TITLE, Y_TITLE,
			CX_TITLE, CY_TITLE, F_Header, g_pTheDB->GetMessageText(MID_AddScriptCommand)));

	//Action list.
	this->pAddCommandDialog->AddWidget(new CLabelWidget(0L, X_ACTIONLABEL, Y_ACTIONLABEL,
			CX_ACTIONLABEL, CY_ACTIONLABEL, F_Small, g_pTheDB->GetMessageText(MID_Action)));
	this->pActionListBox = new CListBoxWidget(TAG_ACTIONLISTBOX,
			X_ACTIONLISTBOX, Y_ACTIONLISTBOX, CX_ACTIONLISTBOX, CY_ACTIONLISTBOX);
	this->pAddCommandDialog->AddWidget(this->pActionListBox);
	PopulateCommandListBox();

	//Event checking.
	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_EVENTLABEL, X_EVENTLABEL, Y_EVENTLABEL,
			CX_EVENTLABEL, CY_EVENTLABEL, F_Small, g_pTheDB->GetMessageText(MID_Event)));
	this->pEventListBox = new CListBoxWidget(TAG_EVENTLISTBOX,
			X_EVENTLISTBOX, Y_EVENTLISTBOX, CX_EVENTLISTBOX, CY_EVENTLISTBOX, true);
	this->pAddCommandDialog->AddWidget(this->pEventListBox);
	PopulateEventListBox();

	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_NOTURNING, X_NOTURNINGLABEL,
			Y_NOTURNINGLABEL, CX_NOTURNINGLABEL, CY_NOTURNINGLABEL, F_Small,
			g_pTheDB->GetMessageText(MID_NoTurning)));

	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_SINGLESTEP, X_SINGLESTEPLABEL,
			Y_SINGLESTEPLABEL, CX_SINGLESTEPLABEL, CY_SINGLESTEPLABEL, F_Small,
			g_pTheDB->GetMessageText(MID_SingleStep)));

	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_SKIPENTRANCELABEL, X_SKIPENTRANCE,
			Y_SKIPENTRANCE, CX_SKIPENTRANCE, CY_SKIPENTRANCE, F_Small,
			g_pTheDB->GetMessageText(MID_SkipEntranceDisplay)));

	//Wait # of turns.
	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_WAITLABEL, X_WAITLABEL,
			Y_WAITLABEL, CX_WAITLABEL, CY_WAITLABEL, F_Small, g_pTheDB->GetMessageText(MID_TurnsToWait)));
	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_WAITABSLABEL, X_WAITLABEL,
			Y_WAITLABEL, CX_WAITLABEL, CY_WAITLABEL, F_Small, g_pTheDB->GetMessageText(MID_WaitUntilTurn)));
	CTextBoxWidget *pWait = new CTextBoxWidget(TAG_WAIT, X_WAIT, Y_WAIT,
			CX_WAIT, CY_WAIT, 4);
	pWait->SetDigitsOnly(true);
	pWait->SetText(wszZero);
	this->pAddCommandDialog->AddWidget(pWait);

	//Speech dialog.
	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_TEXTLABEL, X_TEXTLABEL,
			Y_TEXTLABEL, CX_TEXTLABEL, CY_TEXTLABEL, F_Small, g_pTheDB->GetMessageText(MID_DialogText)));
	CTextBoxWidget *pDialogue = new CTextBoxWidget(TAG_SPEECHTEXT, X_TEXT, Y_TEXT,
			CX_TEXT, CY_TEXT, 1024);
	this->pAddCommandDialog->AddWidget(pDialogue);

	//Sound clip.
	CButtonWidget *pAddSoundButton = new CButtonWidget(TAG_ADDSOUND, X_ADDSOUND,
			Y_ADDSOUND, CX_ADDSOUND, CY_ADDSOUND, g_pTheDB->GetMessageText(MID_AddSound));
	this->pAddCommandDialog->AddWidget(pAddSoundButton);
	CButtonWidget *pTestSoundButton = new CButtonWidget(TAG_TESTSOUND, X_TESTSOUND,
			Y_TESTSOUND, CX_TESTSOUND, CY_TESTSOUND, g_pTheDB->GetMessageText(MID_TestSound));
	this->pAddCommandDialog->AddWidget(pTestSoundButton);
	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_SOUNDNAME_LABEL, X_SOUNDNAMELABEL,
			Y_SOUNDNAMELABEL, CX_SOUNDNAMELABEL, CY_SOUNDNAMELABEL, F_Small, wszEmpty));

	//Speech delay.
	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_DELAYLABEL, X_DELAYLABEL,
			Y_DELAYLABEL, CX_DELAYLABEL, CY_DELAYLABEL, F_Small, g_pTheDB->GetMessageText(MID_SpeechDelay)));
	CTextBoxWidget *pDelay = new CTextBoxWidget(TAG_DELAY, X_DELAY, Y_DELAY,
			CX_DELAY, CY_DELAY, 4);
	pDelay->SetDigitsOnly(true);
	pDelay->SetText(wszZero);
	this->pAddCommandDialog->AddWidget(pDelay);

	//Speaker.
	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_SPEAKERLABEL, X_SPEAKERLABEL, Y_SPEAKERLABEL,
			CX_SPEAKERLABEL, CY_SPEAKERLABEL, F_Small, g_pTheDB->GetMessageText(MID_Speaker)));
	this->pSpeakerListBox = new CListBoxWidget(TAG_SPEAKERLISTBOX,
			X_SPEAKERLISTBOX, Y_SPEAKERLISTBOX, CX_SPEAKERLISTBOX, CY_SPEAKERLISTBOX);
	this->pAddCommandDialog->AddWidget(this->pSpeakerListBox);

	//Music dialog.
	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_MUSICLABEL, X_MUSICLABEL,
			Y_MUSICLABEL, CX_MUSICLABEL, CY_MUSICLABEL, F_Small, g_pTheDB->GetMessageText(MID_MusicType)));
	this->pMusicListBox = new CListBoxWidget(TAG_MUSICLISTBOX,
			X_MUSICLISTBOX, Y_MUSICLISTBOX, CX_MUSICLISTBOX, CY_MUSICLISTBOX);
	this->pAddCommandDialog->AddWidget(this->pMusicListBox);
	this->pMusicListBox->AddItem(static_cast<UINT>(SONGID_DEFAULT), g_pTheDB->GetMessageText(MID_MusicDefault));
	this->pMusicListBox->AddItem(static_cast<UINT>(SONGID_CUSTOM), g_pTheDB->GetMessageText(MID_MusicCustom));
	this->pMusicListBox->AddItem(SONGID_NONE, g_pTheDB->GetMessageText(MID_MusicQuiet));
	this->pMusicListBox->AddItem(SONGID_INTRO, g_pTheDB->GetMessageText(MID_MusicTitle));
	this->pMusicListBox->AddItem(SONGID_WINGAME, g_pTheDB->GetMessageText(MID_MusicWinGame));
	this->pMusicListBox->AddItem(SONGID_ENDOFTHEGAME, g_pTheDB->GetMessageText(MID_MusicFinale));
	CFiles f;
	list<WSTRING> styles;
	if (f.GetGameProfileString("Graphics", "Style", styles))
	{
		UINT wCount = SONGID_COUNT;
		for (list<WSTRING>::iterator style = styles.begin(); style != styles.end(); ++style)
		{
			for (UINT mood=0; mood<SONG_MOOD_COUNT; ++mood)
			{
				WSTRING wstrMoodText;
				AsciiToUnicode(moodText[mood], wstrMoodText);
				WSTRING wstr = *style + wstrMoodText;
				this->pMusicListBox->AddItem(wCount++, wstr.c_str());
			}
		}
	}
	this->pMusicListBox->SelectLine(0);

	//Moods.
	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_MOODLABEL, X_MOODLABEL, Y_MOODLABEL,
			CX_MOODLABEL, CY_MOODLABEL, F_Small, g_pTheDB->GetMessageText(MID_Mood)));
	this->pMoodListBox = new CListBoxWidget(TAG_MOODLISTBOX,
			X_MOODLISTBOX, Y_MOODLISTBOX, CX_MOODLISTBOX, CY_MOODLISTBOX);
	this->pAddCommandDialog->AddWidget(this->pMoodListBox);
	this->pMoodListBox->AddItem(Mood_Normal, g_pTheDB->GetMessageText(MID_Normal));
	this->pMoodListBox->AddItem(Mood_Happy, g_pTheDB->GetMessageText(MID_Happy));
	this->pMoodListBox->AddItem(Mood_Aggressive, g_pTheDB->GetMessageText(MID_Aggressive));
	this->pMoodListBox->AddItem(Mood_Nervous, g_pTheDB->GetMessageText(MID_Nervous));
	this->pMoodListBox->AddItem(Mood_Strike, g_pTheDB->GetMessageText(MID_Striking));
	this->pMoodListBox->SelectLine(0);

	//Direction list box.
	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_DIRECTIONLABEL, X_DIRECTIONLABEL, Y_DIRECTIONLABEL,
			CX_DIRECTIONLABEL, CY_DIRECTIONLABEL, F_Small, g_pTheDB->GetMessageText(MID_ChooseDirection)));
	this->pDirectionListBox = new CListBoxWidget(TAG_DIRECTIONLISTBOX,
			X_DIRECTIONLISTBOX, Y_DIRECTIONLISTBOX, CX_DIRECTIONLISTBOX, CY_DIRECTIONLISTBOX);
	this->pAddCommandDialog->AddWidget(this->pDirectionListBox);
	this->pDirectionListBox->AddItem(NW, g_pTheDB->GetMessageText(MID_NorthWest));
	this->pDirectionListBox->AddItem(N, g_pTheDB->GetMessageText(MID_North));
	this->pDirectionListBox->AddItem(NE, g_pTheDB->GetMessageText(MID_NorthEast));
	this->pDirectionListBox->AddItem(W, g_pTheDB->GetMessageText(MID_West));
	this->pDirectionListBox->AddItem(E, g_pTheDB->GetMessageText(MID_East));
	this->pDirectionListBox->AddItem(SW, g_pTheDB->GetMessageText(MID_SouthWest));
	this->pDirectionListBox->AddItem(S, g_pTheDB->GetMessageText(MID_South));
	this->pDirectionListBox->AddItem(SE, g_pTheDB->GetMessageText(MID_SouthEast));
	this->pDirectionListBox->AddItem(CMD_C, g_pTheDB->GetMessageText(MID_Clockwise));
	this->pDirectionListBox->AddItem(CMD_CC, g_pTheDB->GetMessageText(MID_CounterClockwise));
	this->pDirectionListBox->SelectLine(0);

	//Direction list box #2.
	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_DIRECTIONLABEL2, X_DIRECTIONLABEL2, Y_DIRECTIONLABEL2,
			CX_DIRECTIONLABEL2, CY_DIRECTIONLABEL2, F_Small, g_pTheDB->GetMessageText(MID_ChooseDirection)));
	this->pDirectionListBox2 = new CListBoxWidget(TAG_DIRECTIONLISTBOX2,
			X_DIRECTIONLISTBOX2, Y_DIRECTIONLISTBOX2, CX_DIRECTIONLISTBOX2, CY_DIRECTIONLISTBOX2);
	this->pAddCommandDialog->AddWidget(this->pDirectionListBox2);
	this->pDirectionListBox2->AddItem(NW, g_pTheDB->GetMessageText(MID_NorthWest));
	this->pDirectionListBox2->AddItem(N, g_pTheDB->GetMessageText(MID_North));
	this->pDirectionListBox2->AddItem(NE, g_pTheDB->GetMessageText(MID_NorthEast));
	this->pDirectionListBox2->AddItem(W, g_pTheDB->GetMessageText(MID_West));
	this->pDirectionListBox2->AddItem(E, g_pTheDB->GetMessageText(MID_East));
	this->pDirectionListBox2->AddItem(SW, g_pTheDB->GetMessageText(MID_SouthWest));
	this->pDirectionListBox2->AddItem(S, g_pTheDB->GetMessageText(MID_South));
	this->pDirectionListBox2->AddItem(SE, g_pTheDB->GetMessageText(MID_SouthEast));
	this->pDirectionListBox2->SelectLine(0);

	//Direction list box #3.
	this->pDirectionListBox3 = new CListBoxWidget(TAG_DIRECTIONLISTBOX3,
			X_DIRECTIONLISTBOX3, Y_DIRECTIONLISTBOX3, CX_DIRECTIONLISTBOX3, CY_DIRECTIONLISTBOX3);
	this->pAddCommandDialog->AddWidget(this->pDirectionListBox3);
	this->pDirectionListBox3->AddItem(NW, g_pTheDB->GetMessageText(MID_NorthWest));
	this->pDirectionListBox3->AddItem(N, g_pTheDB->GetMessageText(MID_North));
	this->pDirectionListBox3->AddItem(NE, g_pTheDB->GetMessageText(MID_NorthEast));
	this->pDirectionListBox3->AddItem(W, g_pTheDB->GetMessageText(MID_West));
	this->pDirectionListBox3->AddItem(NO_ORIENTATION, g_pTheDB->GetMessageText(MID_Center));
	this->pDirectionListBox3->AddItem(E, g_pTheDB->GetMessageText(MID_East));
	this->pDirectionListBox3->AddItem(SW, g_pTheDB->GetMessageText(MID_SouthWest));
	this->pDirectionListBox3->AddItem(S, g_pTheDB->GetMessageText(MID_South));
	this->pDirectionListBox3->AddItem(SE, g_pTheDB->GetMessageText(MID_SouthEast));
	this->pDirectionListBox3->SelectItem(NO_ORIENTATION);

	this->pOnOffListBox3 = new CListBoxWidget(TAG_ONOFFLISTBOX3,
			X_ONOFFLISTBOX3, Y_ONOFFLISTBOX3, CX_ONOFFLISTBOX, CY_ONOFFLISTBOX);
	this->pAddCommandDialog->AddWidget(this->pOnOffListBox3);
	this->pOnOffListBox3->AddItem(0, g_pTheDB->GetMessageText(MID_Off));
	this->pOnOffListBox3->AddItem(1, g_pTheDB->GetMessageText(MID_On));
	this->pOnOffListBox3->SelectLine(0);

	//Shallow Water traversal list box.
	this->pWaterTraversalListBox = new CListBoxWidget(TAG_WATERTRAVERSALLISTBOX,
			X_WATERTRAVERSALLISTBOX, Y_WATERTRAVERSALLISTBOX, CX_WATERTRAVERSALLISTBOX, CY_WATERTRAVERSALLISTBOX);
	this->pAddCommandDialog->AddWidget(this->pWaterTraversalListBox);
	this->pWaterTraversalListBox->AddItem(WTrv_AsPlayerRole, g_pTheDB->GetMessageText(MID_WT_PlayerRole));
	this->pWaterTraversalListBox->AddItem(WTrv_NoEntry, g_pTheDB->GetMessageText(MID_WT_NoEntry));
	this->pWaterTraversalListBox->AddItem(WTrv_CanWade, g_pTheDB->GetMessageText(MID_WT_CanWade));
	this->pWaterTraversalListBox->AddItem(WTrv_CanHide, g_pTheDB->GetMessageText(MID_WT_CanHide));
	this->pWaterTraversalListBox->SelectLine(0);

	//Visual effects.
	this->pVisualEffectsListBox = new CListBoxWidget(TAG_VISUALEFFECTS_LISTBOX,
			X_EFFECTLISTBOX, Y_EFFECTLISTBOX, CX_EFFECTLISTBOX, CY_EFFECTLISTBOX, true);
	this->pAddCommandDialog->AddWidget(this->pVisualEffectsListBox);
	this->pVisualEffectsListBox->AddItem(VET_BLOODSPLAT, g_pTheDB->GetMessageText(MID_BloodSplatterEffect));
	this->pVisualEffectsListBox->AddItem(VET_MUDSPLAT, g_pTheDB->GetMessageText(MID_MudSplatterEffect));
	this->pVisualEffectsListBox->AddItem(VET_TARSPLAT, g_pTheDB->GetMessageText(MID_TarSplatterEffect));
	this->pVisualEffectsListBox->AddItem(VET_GELSPLAT, g_pTheDB->GetMessageText(MID_GelSplatterEffect));
	this->pVisualEffectsListBox->AddItem(VET_SEEPSPLAT, g_pTheDB->GetMessageText(MID_SeepSplatterEffect));
	this->pVisualEffectsListBox->AddItem(VET_GOLEMSPLAT, g_pTheDB->GetMessageText(MID_GolemSplatterEffect));
	this->pVisualEffectsListBox->AddItem(VET_SLAYERSPLAT, g_pTheDB->GetMessageText(MID_SlayerSplatterEffect));
	this->pVisualEffectsListBox->AddItem(VET_DEBRIS, g_pTheDB->GetMessageText(MID_DebrisEffect));
	this->pVisualEffectsListBox->AddItem(VET_SPARKS, g_pTheDB->GetMessageText(MID_SparkEffect));
	this->pVisualEffectsListBox->AddItem(VET_EXPLOSION, g_pTheDB->GetMessageText(MID_ExplosionEffect));
	this->pVisualEffectsListBox->AddItem(VET_SPLASH, g_pTheDB->GetMessageText(MID_SplashEffect));
	this->pVisualEffectsListBox->AddItem(VET_STEAM, g_pTheDB->GetMessageText(MID_SteamEffect));
	this->pVisualEffectsListBox->AddItem(VET_SWIRL, g_pTheDB->GetMessageText(MID_SwirlEffect));
	this->pVisualEffectsListBox->AddItem(VET_VERMIN, g_pTheDB->GetMessageText(MID_VerminEffect));
	this->pVisualEffectsListBox->AddItem(VET_BOLT, g_pTheDB->GetMessageText(MID_BoltEffect));
	this->pVisualEffectsListBox->SelectLine(0);

	this->pWaitFlagsListBox = new CListBoxWidget(TAG_WAITFLAGSLISTBOX,
			X_WAITFLAGSLISTBOX, Y_WAITFLAGSLISTBOX, CX_WAITFLAGSLISTBOX, CY_WAITFLAGSLISTBOX);
	this->pAddCommandDialog->AddWidget(this->pWaitFlagsListBox);
	this->pWaitFlagsListBox->SelectMultipleItems(true);
	this->pWaitFlagsListBox->AddItem(ScriptFlag::PLAYER, g_pTheDB->GetMessageText(MID_Player));
	this->pWaitFlagsListBox->AddItem(ScriptFlag::HALPH, g_pTheDB->GetMessageText(MID_Halph2));
	this->pWaitFlagsListBox->AddItem(ScriptFlag::MONSTER, g_pTheDB->GetMessageText(MID_Monster));
	this->pWaitFlagsListBox->AddItem(ScriptFlag::NPC, g_pTheDB->GetMessageText(MID_NPC));
	this->pWaitFlagsListBox->AddItem(ScriptFlag::PDOUBLE, g_pTheDB->GetMessageText(MID_PlayerDouble));
	this->pWaitFlagsListBox->AddItem(ScriptFlag::SELF, g_pTheDB->GetMessageText(MID_Self));
	this->pWaitFlagsListBox->AddItem(ScriptFlag::SLAYER, g_pTheDB->GetMessageText(MID_Slayer2));
	this->pWaitFlagsListBox->AddItem(ScriptFlag::BEETHRO, g_pTheDB->GetMessageText(MID_Beethro));
	this->pWaitFlagsListBox->AddItem(ScriptFlag::STALWART, g_pTheDB->GetMessageText(MID_Stalwart));
	this->pWaitFlagsListBox->SelectLine(0);

	this->pBuildItemsListBox = new CListBoxWidget(TAG_ITEMLISTBOX,
			X_ITEMLISTBOX, Y_ITEMLISTBOX, CX_ITEMLISTBOX, CY_ITEMLISTBOX);
	this->pAddCommandDialog->AddWidget(this->pBuildItemsListBox);
	this->pBuildItemsListBox->AddItem(static_cast<UINT>(-1), g_pTheDB->GetMessageText(MID_None));
	this->pBuildItemsListBox->AddItem(T_FLOOR, g_pTheDB->GetMessageText(MID_Floor));
	this->pBuildItemsListBox->AddItem(T_FLOOR_M, g_pTheDB->GetMessageText(MID_FloorMosaic));
	this->pBuildItemsListBox->AddItem(T_FLOOR_ROAD, g_pTheDB->GetMessageText(MID_FloorRoad));
	this->pBuildItemsListBox->AddItem(T_FLOOR_GRASS, g_pTheDB->GetMessageText(MID_FloorGrass));
	this->pBuildItemsListBox->AddItem(T_FLOOR_DIRT, g_pTheDB->GetMessageText(MID_FloorDirt));
	this->pBuildItemsListBox->AddItem(T_FLOOR_ALT, g_pTheDB->GetMessageText(MID_FloorAlt));
	this->pBuildItemsListBox->AddItem(T_FLOOR_IMAGE, g_pTheDB->GetMessageText(MID_FloorImage));
	this->pBuildItemsListBox->AddItem(T_GOO, g_pTheDB->GetMessageText(MID_Goo));
	this->pBuildItemsListBox->AddItem(T_HOT, g_pTheDB->GetMessageText(MID_Hot));
	this->pBuildItemsListBox->AddItem(T_WALL, g_pTheDB->GetMessageText(MID_Wall));
	this->pBuildItemsListBox->AddItem(T_WALL_B, g_pTheDB->GetMessageText(MID_BrokenWall));
	this->pBuildItemsListBox->AddItem(T_WALL_IMAGE, g_pTheDB->GetMessageText(MID_WallImage));
	this->pBuildItemsListBox->AddItem(T_TRAPDOOR, g_pTheDB->GetMessageText(MID_Trapdoor));
	this->pBuildItemsListBox->AddItem(T_TRAPDOOR2, g_pTheDB->GetMessageText(MID_Trapdoor2));
	this->pBuildItemsListBox->AddItem(T_PIT, g_pTheDB->GetMessageText(MID_Pit));
	this->pBuildItemsListBox->AddItem(T_PIT_IMAGE, g_pTheDB->GetMessageText(MID_PitImage));
	this->pBuildItemsListBox->AddItem(T_WATER, g_pTheDB->GetMessageText(MID_Water));
	this->pBuildItemsListBox->AddItem(T_SHALLOW_WATER, g_pTheDB->GetMessageText(MID_ShallowWater));
	this->pBuildItemsListBox->AddItem(T_BRIDGE, g_pTheDB->GetMessageText(MID_Bridge));
	this->pBuildItemsListBox->AddItem(T_BRIDGE_H, g_pTheDB->GetMessageText(MID_Bridge_H));
	this->pBuildItemsListBox->AddItem(T_BRIDGE_V, g_pTheDB->GetMessageText(MID_Bridge_V));
	this->pBuildItemsListBox->AddItem(T_TUNNEL_N, g_pTheDB->GetMessageText(MID_Tunnel_N));
	this->pBuildItemsListBox->AddItem(T_TUNNEL_E, g_pTheDB->GetMessageText(MID_Tunnel_E));
	this->pBuildItemsListBox->AddItem(T_TUNNEL_S, g_pTheDB->GetMessageText(MID_Tunnel_S));
	this->pBuildItemsListBox->AddItem(T_TUNNEL_W, g_pTheDB->GetMessageText(MID_Tunnel_W));
	this->pBuildItemsListBox->AddItem(T_BOMB, g_pTheDB->GetMessageText(MID_Bomb));
	this->pBuildItemsListBox->AddItem(T_FUSE, g_pTheDB->GetMessageText(MID_Fuse));
	this->pBuildItemsListBox->AddItem(T_POTION_C, g_pTheDB->GetMessageText(MID_ClonePotion));
	this->pBuildItemsListBox->AddItem(T_POTION_D, g_pTheDB->GetMessageText(MID_DecoyPotion));
	this->pBuildItemsListBox->AddItem(T_POTION_K, g_pTheDB->GetMessageText(MID_MimicPotion));
	this->pBuildItemsListBox->AddItem(T_POTION_I, g_pTheDB->GetMessageText(MID_InvisPotion));
	this->pBuildItemsListBox->AddItem(T_POTION_SP, g_pTheDB->GetMessageText(MID_SpeedPotion));
	this->pBuildItemsListBox->AddItem(T_HORN_SQUAD, g_pTheDB->GetMessageText(MID_SquadHorn));
	this->pBuildItemsListBox->AddItem(T_HORN_SOLDIER, g_pTheDB->GetMessageText(MID_SoldierHorn));
	this->pBuildItemsListBox->AddItem(T_EMPTY, g_pTheDB->GetMessageText(MID_RemoveItem));
	this->pBuildItemsListBox->SelectLine(0);

	//Yes/No selection.
	this->pOnOffListBox = new CListBoxWidget(TAG_ONOFFLISTBOX,
			X_ONOFFLISTBOX, Y_ONOFFLISTBOX, CX_ONOFFLISTBOX, CY_ONOFFLISTBOX);
	this->pAddCommandDialog->AddWidget(this->pOnOffListBox);
	this->pOnOffListBox->AddItem(0, g_pTheDB->GetMessageText(MID_Off));
	this->pOnOffListBox->AddItem(1, g_pTheDB->GetMessageText(MID_On));
	this->pOnOffListBox->SelectLine(0);

	this->pOnOffListBox2 = new CListBoxWidget(TAG_ONOFFLISTBOX2,
			X_ONOFFLISTBOX2, Y_ONOFFLISTBOX, CX_ONOFFLISTBOX, CY_ONOFFLISTBOX);
	this->pAddCommandDialog->AddWidget(this->pOnOffListBox2);
	this->pOnOffListBox2->AddItem(0, g_pTheDB->GetMessageText(MID_Off));
	this->pOnOffListBox2->AddItem(1, g_pTheDB->GetMessageText(MID_On));
	this->pOnOffListBox2->SelectLine(0);

	this->pImperativeListBox = new CListBoxWidget(TAG_IMPERATIVELISTBOX,
			X_IMPERATIVELISTBOX, Y_IMPERATIVELISTBOX, CX_IMPERATIVELISTBOX, CY_IMPERATIVELISTBOX);
	this->pAddCommandDialog->AddWidget(this->pImperativeListBox);
	PopulateImperativeListBox();

	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_DISPLAYSPEECHLABEL,
			X_DISPLAYSPEECHLABEL, Y_DISPLAYSPEECHLABEL,
			CX_DISPLAYSPEECHLABEL, CY_DISPLAYSPEECHLABEL,
			F_Small, g_pTheDB->GetMessageText(MID_DisplaySpeech)));

	//Open/close selection.
	this->pOpenCloseListBox = new CListBoxWidget(TAG_OPENCLOSELISTBOX,
			X_OPENCLOSELISTBOX, Y_OPENCLOSELISTBOX, CX_OPENCLOSELISTBOX, CY_OPENCLOSELISTBOX);
	this->pAddCommandDialog->AddWidget(this->pOpenCloseListBox);
	this->pOpenCloseListBox->AddItem(OA_CLOSE, g_pTheDB->GetMessageText(MID_Close));
	this->pOpenCloseListBox->AddItem(OA_OPEN, g_pTheDB->GetMessageText(MID_Open));
	this->pOpenCloseListBox->SelectLine(0);

	//Goto label.
	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_GOTOLABEL, X_GOTOLABEL,
			Y_GOTOLABEL, CX_GOTOLABEL, CY_GOTOLABEL, F_Small, g_pTheDB->GetMessageText(MID_Label)));
	CTextBoxWidget *pGotoLabelText = new CTextBoxWidget(TAG_GOTOLABELTEXT, X_GOTOLABELTEXT, Y_GOTOLABELTEXT,
			CX_GOTOLABELTEXT, CY_GOTOLABELTEXT, MAX_TEXT_LABEL_SIZE);
	this->pAddCommandDialog->AddWidget(pGotoLabelText);

	//Goto label list.
	this->pGotoLabelListBox = new CListBoxWidget(TAG_GOTOLABELLISTBOX,
			X_GOTOLABELLISTBOX, Y_GOTOLABELLISTBOX, CX_GOTOLABELLISTBOX, CY_GOTOLABELLISTBOX, true);
	this->pAddCommandDialog->AddWidget(this->pGotoLabelListBox);

	//Variable handling widgets.
	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_VARNAMETEXTLABEL,
			X_VARTEXTLABEL, Y_VARTEXTLABEL, CX_VARTEXTLABEL, CY_VARTEXTLABEL,
			F_Small, g_pTheDB->GetMessageText(MID_VarNameText)));

	this->pVarListBox = new CListBoxWidget(TAG_VARLIST,
			X_VARLISTBOX, Y_VARLISTBOX, CX_VARLISTBOX, CY_VARLISTBOX, true);
	this->pAddCommandDialog->AddWidget(this->pVarListBox);

	CButtonWidget *pVarAddButton = new CButtonWidget(TAG_VARADD, X_VARADD,
			Y_VARADD, CX_VARADD, CY_VARADD, g_pTheDB->GetMessageText(MID_VarAdd));
	this->pAddCommandDialog->AddWidget(pVarAddButton);

	CButtonWidget *pVarRemoveButton = new CButtonWidget(TAG_VARREMOVE, X_VARREMOVE,
			Y_VARREMOVE, CX_VARREMOVE, CY_VARREMOVE, g_pTheDB->GetMessageText(MID_VarRemove));
	this->pAddCommandDialog->AddWidget(pVarRemoveButton);

	this->pVarOpListBox = new CListBoxWidget(TAG_VAROPLIST,
			X_VAROPLIST, Y_VAROPLIST, CX_VAROPLIST, CY_VAROPLIST);
	this->pAddCommandDialog->AddWidget(this->pVarOpListBox);
	this->pVarOpListBox->AddItem(ScriptVars::Assign, g_pTheDB->GetMessageText(MID_VarAssign));
	this->pVarOpListBox->AddItem(ScriptVars::AssignText, g_pTheDB->GetMessageText(MID_VarAssignText));
	this->pVarOpListBox->AddItem(ScriptVars::AppendText, g_pTheDB->GetMessageText(MID_VarAppendText));
	this->pVarOpListBox->AddItem(ScriptVars::Inc, g_pTheDB->GetMessageText(MID_VarInc));
	this->pVarOpListBox->AddItem(ScriptVars::Dec, g_pTheDB->GetMessageText(MID_VarDec));
	this->pVarOpListBox->AddItem(ScriptVars::MultiplyBy, g_pTheDB->GetMessageText(MID_VarMultiplyBy));
	this->pVarOpListBox->AddItem(ScriptVars::DivideBy, g_pTheDB->GetMessageText(MID_VarDivideBy));
	this->pVarOpListBox->AddItem(ScriptVars::Mod, g_pTheDB->GetMessageText(MID_VarMod));
	this->pVarOpListBox->SelectLine(0);

	this->pVarCompListBox = new CListBoxWidget(TAG_VARCOMPLIST,
			X_VARCOMPLIST, Y_VARCOMPLIST, CX_VARCOMPLIST, CY_VARCOMPLIST);
	this->pAddCommandDialog->AddWidget(this->pVarCompListBox);
	this->pVarCompListBox->AddItem(ScriptVars::Equals, g_pTheDB->GetMessageText(MID_VarEquals));
	this->pVarCompListBox->AddItem(ScriptVars::EqualsText, g_pTheDB->GetMessageText(MID_VarEqualsText));
	this->pVarCompListBox->AddItem(ScriptVars::Greater, g_pTheDB->GetMessageText(MID_VarGreater));
	this->pVarCompListBox->AddItem(ScriptVars::Less, g_pTheDB->GetMessageText(MID_VarLess));
	this->pVarCompListBox->SelectLine(0);

	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_VARVALUELABEL,
			X_VARVALUELABEL, Y_VARVALUELABEL, CX_VARVALUELABEL, CY_VARVALUELABEL,
			F_Small, g_pTheDB->GetMessageText(MID_VarOperand)));

	CTextBoxWidget *pVarOperand = new CTextBoxWidget(TAG_VARVALUE, X_VARVALUE, Y_VARVALUE,
			CX_VARVALUE, CY_VARVALUE);
	pVarOperand->SetText(wszZero);
	this->pAddCommandDialog->AddWidget(pVarOperand);

	this->pPlayerGraphicListBox = new CListBoxWidget(TAG_GRAPHICLISTBOX2,
			X_GRAPHICLISTBOX2, Y_GRAPHICLISTBOX2, CX_GRAPHICLISTBOX2, CY_GRAPHICLISTBOX2, true);
	this->pAddCommandDialog->AddWidget(this->pPlayerGraphicListBox);

	this->pGlobalScriptListBox = new CListBoxWidget(TAG_GLOBALSCRIPTLISTBOX,
			X_GLOBALSCRIPTLISTBOX, Y_GLOBALSCRIPTLISTBOX, CX_GLOBALSCRIPTLISTBOX, CY_GLOBALSCRIPTLISTBOX, true);
	this->pAddCommandDialog->AddWidget(this->pGlobalScriptListBox);

	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_CUTSCENELABEL, X_CUTSCENELABEL,
			Y_CUTSCENELABEL, CX_CUTSCENELABEL, CY_CUTSCENELABEL, F_Small,
			g_pTheDB->GetMessageText(MID_CutSceneIncrement)));

	//Relative movement.
	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_MOVERELXLABEL, X_MOVERELXLABEL,
			Y_MOVERELXLABEL, CX_MOVERELXLABEL, CY_MOVERELXLABEL, F_Small,
			g_pTheDB->GetMessageText(MID_MoveRelX)));
	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_MOVERELYLABEL, X_MOVERELYLABEL,
			Y_MOVERELYLABEL, CX_MOVERELYLABEL, CY_MOVERELYLABEL, F_Small,
			g_pTheDB->GetMessageText(MID_MoveRelY)));
	CTextBoxWidget *pRelX = new CTextBoxWidget(TAG_MOVERELX, X_MOVERELX, Y_MOVERELX,
			CX_MOVERELX, CY_MOVERELX, 4);
	pRelX->SetDigitsOnly(true);
	pRelX->SetAllowNegative(true);
	pRelX->SetText(wszZero);
	this->pAddCommandDialog->AddWidget(pRelX);
	CTextBoxWidget *pRelY = new CTextBoxWidget(TAG_MOVERELY, X_MOVERELY, Y_MOVERELY,
			CX_MOVERELY, CY_MOVERELY, 4);
	pRelY->SetDigitsOnly(true);
	pRelY->SetAllowNegative(true);
	pRelY->SetText(wszZero);
	this->pAddCommandDialog->AddWidget(pRelY);

	//Sounds.
	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_LOOPSOUND, X_LOOPSOUND,
			Y_LOOPSOUND, CX_LOOPSOUND, CY_LOOPSOUND, F_Small,
			g_pTheDB->GetMessageText(MID_LoopSound)));

	this->pAddCommandDialog->AddWidget(new CLabelWidget(TAG_SOUNDEFFECTLABEL,
			X_SOUNDEFFECTLABEL, Y_SOUNDEFFECTLABEL, CX_SOUNDEFFECTLABEL, CY_SOUNDEFFECTLABEL,
			F_Small, g_pTheDB->GetMessageText(MID_SoundEffect)));

	//OK/cancel buttons.
	CButtonWidget *pOKButton = new CButtonWidget(
			TAG_OK, X_OKBUTTON, Y_OKBUTTON, CX_BUTTON, CY_STANDARD_BUTTON,
			g_pTheDB->GetMessageText(MID_Okay));
	this->pAddCommandDialog->AddWidget(pOKButton);
	CButtonWidget *pCancelButton = new CButtonWidget(
			TAG_CANCEL, X_CANCELBUTTON, Y_CANCELBUTTON, CX_BUTTON, CY_STANDARD_BUTTON,
			g_pTheDB->GetMessageText(MID_Cancel));
	this->pAddCommandDialog->AddWidget(pCancelButton);

	//Add to main dialog.
	AddWidget(this->pAddCommandDialog);
	this->pAddCommandDialog->Center();
	this->pAddCommandDialog->Hide();
}

//*****************************************************************************
void CCharacterDialogWidget::AddScriptDialog()
//A pop-up dialog for editing a script only.
{
	static const UINT CX_SCRIPT_DIALOG = 660;
	static const UINT CY_SCRIPT_DIALOG = 630;

	static const UINT CX_TITLE = 240;
	static const UINT CY_TITLE = 30;
	static const int X_TITLE = (CX_SCRIPT_DIALOG - CX_TITLE) / 2;
	static const int Y_TITLE = CY_SPACE;

	static const int X_COMMANDSLABEL = CX_SPACE;
	static const int Y_COMMANDSLABEL = Y_TITLE + CY_TITLE + CY_SPACE;
	static const UINT CX_COMMANDSLABEL = 110;
	static const UINT CY_COMMANDSLABEL = 33;
	static const int X_COMMANDS = X_COMMANDSLABEL;
	static const int Y_COMMANDS = Y_COMMANDSLABEL + CY_COMMANDSLABEL;
	static const UINT CX_COMMANDS = CX_SCRIPT_DIALOG - X_COMMANDS - CX_SPACE;
	static const UINT CY_COMMANDS = 22*22 + 4; //22 items
	static const UINT CX_ADDCOMMAND = 130;

	static const int X_ADDCOMMAND = X_COMMANDS + CX_COMMANDS - CX_ADDCOMMAND;
	static const int Y_ADDCOMMAND = Y_COMMANDSLABEL - 4;
	static const UINT CY_ADDCOMMAND = CY_STANDARD_BUTTON;
	static const UINT CX_DELETECOMMAND = 150;
	static const int X_DELETECOMMAND = X_ADDCOMMAND - CX_DELETECOMMAND - CX_SPACE;
	static const int Y_DELETECOMMAND = Y_ADDCOMMAND;
	static const UINT CY_DELETECOMMAND = CY_STANDARD_BUTTON;

	static const UINT CX_BUTTON = 70;
	static const int X_OKBUTTON = (CX_SCRIPT_DIALOG - (CX_BUTTON + CX_SPACE)) / 2;
	static const int Y_OKBUTTON = CY_SCRIPT_DIALOG - CY_STANDARD_BUTTON - 15;

	ASSERT(!this->pScriptDialog);
	this->pScriptDialog = new CRenameDialogWidget(0, CX_SPACE, GetY() + (GetH()-CY_SCRIPT_DIALOG)/2,
			CX_SCRIPT_DIALOG, CY_SCRIPT_DIALOG);

	this->pScriptDialog->AddWidget(new CLabelWidget(0, X_TITLE, Y_TITLE, CX_TITLE, CY_TITLE,
			F_Header, g_pTheDB->GetMessageText(MID_EditDefaultScript)));

	//Commands.
	this->pScriptDialog->AddWidget(new CButtonWidget(TAG_ADDCOMMAND2, X_ADDCOMMAND, Y_ADDCOMMAND,
			CX_ADDCOMMAND, CY_ADDCOMMAND, g_pTheDB->GetMessageText(MID_AddCommand)));
	this->pScriptDialog->AddWidget(new CButtonWidget(TAG_DELETECOMMAND2, X_DELETECOMMAND, Y_DELETECOMMAND,
			CX_DELETECOMMAND, CY_DELETECOMMAND, g_pTheDB->GetMessageText(MID_DeleteCommand)));

	this->pScriptDialog->AddWidget(new CLabelWidget(0, X_COMMANDSLABEL, Y_COMMANDSLABEL,
			CX_COMMANDSLABEL, CY_COMMANDSLABEL, F_Small, g_pTheDB->GetMessageText(MID_Commands)));
	this->pDefaultScriptCommandsListBox = new CListBoxWidget(TAG_DEFAULTCOMMANDSLISTBOX, X_COMMANDS, Y_COMMANDS,
			CX_COMMANDS, CY_COMMANDS, false, true, true);
	this->pScriptDialog->AddWidget(this->pDefaultScriptCommandsListBox);

	//OK button.
	CButtonWidget *pButton = new CButtonWidget(
			TAG_OK2, X_OKBUTTON, Y_OKBUTTON, CX_BUTTON, CY_STANDARD_BUTTON,
			g_pTheDB->GetMessageText(MID_Okay));
	this->pScriptDialog->AddWidget(pButton);

	//Add to main dialog.
	AddWidget(this->pScriptDialog);
	this->pScriptDialog->Center();
	this->pScriptDialog->Hide();
}

//*****************************************************************************
void CCharacterDialogWidget::AddCustomCharacter()
//Adds a custom character with specified name and image.
{
	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));
	ASSERT(pEditRoomScreen->pHold);
	const WCHAR *pCharName = this->pCharNameText->GetText();
	ASSERT(WCSlen(pCharName) > 0);

	const UINT dwNewCharID = pEditRoomScreen->pHold->AddCharacter(pCharName);
	if (!dwNewCharID)
		pEditRoomScreen->ShowOkMessage(MID_CharNameDuplicationError);
	else {
		HoldCharacter *pChar = pEditRoomScreen->pHold->GetCharacter(dwNewCharID);
		ASSERT(pChar);
		CListBoxWidget *pCharGraphicList = DYN_CAST(CListBoxWidget*, CWidget*,
				GetWidget(TAG_CHARGRAPHICLISTBOX));
		pChar->wType = pCharGraphicList->GetSelectedItem();
		this->pCharNameText->SetText(wszEmpty); //reset for next use

		//Refresh list.  Select new character.
		PopulateMainGraphicList();
		this->pCharListBox->SelectItem(dwNewCharID);
		SelectCharacter();
	}
}

//*****************************************************************************
void CCharacterDialogWidget::DeleteCustomCharacter()
//Deletes selected character (pending room+hold save).
{
	const UINT dwCharID = this->pCharListBox->GetSelectedItem();
	if (!dwCharID)
		return;

	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));

	if (pEditRoomScreen->ShowYesNoMessage(MID_DeleteCharacterPrompt) != TAG_YES)
		return;

	ASSERT(pEditRoomScreen->pHold);
	VERIFY(pEditRoomScreen->pHold->DeleteCharacter(dwCharID));

	//Refresh list.
	PopulateMainGraphicList();

	SetCharacterWidgetStates();
	this->pAddCharacterDialog->Paint();
}

//*****************************************************************************
void CCharacterDialogWidget::EditCharacter(CCharacter* pCharacter)
//Prepares widgets for viewing and editing given character.
{
	ASSERT(pCharacter);
	ASSERT(!this->pCommand);
	ASSERT(!this->pCharacter);
	this->pCharacter = pCharacter;

	PopulateVarList();

	PopulateMainGraphicList();

	CWidget *pButton = this->pAddCommandDialog->GetWidget(TAG_VARREMOVE);
	ASSERT(pButton);
	const bool bEnable = this->pVarListBox->GetItemCount() > 0;
	if (!bEnable && GetSelectedWidget() == pButton)
		SelectNextWidget();
	pButton->Enable(bEnable);

	//Compile temporary commands list from character.
	for (UINT wIndex=0; wIndex<pCharacter->commands.size(); ++wIndex)
	{
		CCharacterCommand *pCommand = new CCharacterCommand(pCharacter->commands[wIndex]);
		this->commands.push_back(pCommand);
	}

	//Generate labels once command list has been compiled.
	PopulateGotoLabelList(this->commands);

	PopulateCommandDescriptions(this->pCommandsListBox, this->commands);

	this->pIsVisibleButton->SetChecked(pCharacter->IsVisible());

	SetWidgetStates();
}

//*****************************************************************************
COMMANDPTR_VECTOR* CCharacterDialogWidget::GetActiveCommands()
//Returns: a pointer to the command list being edited
{
	if (this->bEditingDefaultScript)
	{
		//Select the custom NPC whose script is being edited.
		ASSERT(this->defaultScriptCustomCharID);
		this->pCharListBox->SelectItem(this->defaultScriptCustomCharID);

		HoldCharacter *pChar = GetCustomCharacter();
		ASSERT(pChar);
		ASSERT(pChar->pCommands);
		return pChar->pCommands;
	}

	return &this->commands;
}

//*****************************************************************************
CListBoxWidget* CCharacterDialogWidget::GetActiveCommandListBox()
//Returns: pointer to the active command list box widget
{
	if (this->bEditingDefaultScript)
		return this->pDefaultScriptCommandsListBox;

	return this->pCommandsListBox;
}

//*****************************************************************************
void CCharacterDialogWidget::OnClick(
//Handles click event.
//
//Params:
	const UINT dwTagNo)          //(in)   Widget that received event.
{
	switch (dwTagNo)
	{
		case TAG_OK:
		case TAG_CANCEL:
		{
			//Reflect command changes in character object now.
			UpdateCharacter();

			CDialogWidget::OnClick(dwTagNo); //deactivate
		}
		break;

		case TAG_CHARACTERS:
		{
			//Select this character's custom type, if one is set.
			this->pCharListBox->SelectItem(this->pCharacter->wLogicalIdentity);

			EditCustomCharacters();
		}
		break;

		case TAG_DELETECOMMAND:
		{
			ASSERT(this->pCommandsListBox->GetItemCount() > 0);
			CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
					g_pTheSM->GetScreen(SCR_EditRoom));
			ASSERT(pEditRoomScreen);
			if (pEditRoomScreen->ShowYesNoMessage(MID_DeleteCommandPrompt) == TAG_YES)
			{
				DeleteCommands(this->pCommandsListBox, this->commands);
				SetWidgetStates();
			}
			pEditRoomScreen->SetUpdateRectAfterMessage(true);
			Paint(false);
			pEditRoomScreen->UpdateRect();
		}
		break;

		case TAG_ADDCOMMAND:
		{
			ASSERT(this->pAddCommandDialog);
			ASSERT(!this->pCommand);
			ASSERT(!this->pSound);
			SetActionWidgetStates();
			this->bRetainFields = true;
			UINT dwTagNo;
			bool bLoop=true;
			do {
				dwTagNo = this->pAddCommandDialog->Display();
				switch (dwTagNo)
				{
					case TAG_ADDSOUND: AddSound(); break;
					case TAG_TESTSOUND: TestSound(); break;
					case TAG_VARADD: AddVar(); break;
					case TAG_VARREMOVE: DeleteVar();	break;
					default:
						bLoop = false;
					break;
				}
			} while (bLoop && !IsDeactivating());
			this->bRetainFields = false;
			if (dwTagNo == TAG_OK)
			{
				//Begin adding this command.
				const CCharacterCommand::CharCommand command =
					(CCharacterCommand::CharCommand)this->pActionListBox->GetSelectedItem();
				this->pCommand = new CCharacterCommand();
				this->pCommand->command = command;
				SetCommandParametersFromWidgets(this->pCommandsListBox, this->commands);
			} else {
				//Command addition was canceled.
				//Clear any data generated for the command.
				ASSERT(!this->pSound || !this->pSound->dwDataID);	//should be fresh (not added to DB yet)
				delete this->pSound;
				this->pSound = NULL;
			}
			SetWidgetStates();

			if (this->pParent) this->pParent->Paint();   //refresh screen
			Paint();
		}
		break;
	}
}

//*****************************************************************************
void CCharacterDialogWidget::OnDoubleClick(
//Handles double click event.
//
//Params:
	const UINT dwTagNo) //(in) Widget event applies to.
{
	switch (dwTagNo)
	{
		case TAG_COMMANDSLISTBOX:
			EditClickedCommand();
		break;

		case TAG_CHARACTERLISTBOX:
		{
			//Edit an existing command.
			if (!this->pCharListBox->ClickedSelection()) break;

			HoldCharacter *pChar = GetCustomCharacter();
			if (!pChar) break;

			CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
					g_pTheSM->GetScreen(SCR_EditRoom));
			ASSERT(pEditRoomScreen);
			WSTRING wstr = pChar->charNameText;
			const UINT dwAnswerTagNo = pEditRoomScreen->ShowTextInputMessage(
					MID_NameCharacterPrompt, wstr);
			if (dwAnswerTagNo == TAG_OK)
				pChar->charNameText = wstr;
		}
		break;
	}
}

//*****************************************************************************
void CCharacterDialogWidget::OnKeyDown(
//Called when widget receives SDL_KEYDOWN event.
//
//Params:
	const UINT dwTagNo,       //(in)   Widget that event applied to.
	const SDL_KeyboardEvent &Key) //(in)   Event.
{
	CDialogWidget::OnKeyDown(dwTagNo, Key);
	if (IsDeactivating())
	{
		UpdateCharacter();
		return;
	}

	if (this->pAddCommandDialog->IsVisible())
		return; //can't alter the command list while inputting a command

	switch (Key.keysym.sym)
	{
		case SDLK_DELETE:
		{
			CListBoxWidget *pActiveCommandList = GetActiveCommandListBox();
			if (pActiveCommandList->GetItemCount())
			{
				CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
						g_pTheSM->GetScreen(SCR_EditRoom));
				ASSERT(pEditRoomScreen);
				if (pEditRoomScreen->ShowYesNoMessage(MID_DeleteCommandPrompt) == TAG_YES)
				{
					DeleteCommands(pActiveCommandList, *GetActiveCommands());
					SetWidgetStates();
				}
				pEditRoomScreen->SetUpdateRectAfterMessage(true);
				Paint(false);
				pEditRoomScreen->UpdateRect();
			}
		}
		break;

		//Cut/copy/paste from command list.
		case SDLK_x:
		{
			if ((Key.keysym.mod & KMOD_CTRL) == 0)
				break;

			CListBoxWidget *pActiveCommandList = GetActiveCommandListBox();
			vector<void*> keys(pActiveCommandList->GetSelectedItemsPointers());
			if (keys.empty())
				break;

			g_pTheSound->PlaySoundEffect(SEID_TRAPDOOR);
			ClearPasteBuffer();

			UINT wSelectedLine = pActiveCommandList->GetSelectedLineNumber();

			//Remove selected commands from script/command list.
			//Put into paste and clipboard buffers.
			COMMANDPTR_VECTOR *pCommands = GetActiveCommands();

			for (vector<void*>::const_iterator key = keys.begin(); key != keys.end(); ++key)
			{
				void *pKey = *key;
				CCharacterCommand *pCommand = (CCharacterCommand*)(pKey);
				ASSERT(pCommand);

				const UINT wLine = pActiveCommandList->GetLineWithKeyPointer(pKey);
				COMMANDPTR_VECTOR::iterator iter = pCommands->begin() + wLine;
				ASSERT(iter != pCommands->end());
				pCommands->erase(iter);
				pActiveCommandList->RemoveItem(pKey);

				this->commandBuffer.push_back(pCommand);
			}

			//Before removing label ID references, prepare commands so goto-style
			//commands may be pasted correctly.
			prepareForwardReferences(this->commandBuffer);

			//Now remove labels from list.
			for (COMMANDPTR_VECTOR::const_iterator command = this->commandBuffer.begin();
				command != this->commandBuffer.end(); ++command)
			{
				CCharacterCommand *pCommand = *command;
				if (pCommand->command == CCharacterCommand::CC_Label)
					this->pGotoLabelListBox->RemoveItem(pCommand->x);
			}

			//Display remaining commands.
			if (wSelectedLine >= pActiveCommandList->GetItemCount())
				wSelectedLine = pActiveCommandList->GetItemCount()-1;
			pActiveCommandList->SelectLine(wSelectedLine);
			if (IsEditingDefaultScript())
				SetDefaultScriptWidgetStates();
			else
				SetWidgetStates();
			Paint();
		}
		break;
		case SDLK_c:
		{
			if ((Key.keysym.mod & KMOD_CTRL) == 0)
				break;

			CListBoxWidget *pActiveCommandList = GetActiveCommandListBox();
			vector<void*> keys(pActiveCommandList->GetSelectedItemsPointers());
			if (keys.empty())
				break;

			g_pTheSound->PlaySoundEffect(SEID_POTION);

			//Replace paste and clipboard buffers with a copy of the current selection.
			ClearPasteBuffer();

			for (vector<void*>::const_iterator key = keys.begin(); key != keys.end(); ++key)
			{
				//Make a distinct copy of this command.
				CCharacterCommand *pCommand = (CCharacterCommand*)(*key);
				ASSERT(pCommand);
				CCharacterCommand *pCommandCopy = new CCharacterCommand(*pCommand, true);
				this->commandBuffer.push_back(pCommandCopy);
			}

			//Prepare label ID references for pasting.
			prepareForwardReferences(this->commandBuffer);
		}
		break;
		case SDLK_v:
		{
			if ((Key.keysym.mod & KMOD_CTRL) == 0)
				break;

			if (this->commandBuffer.empty())
				break;

			g_pTheSound->PlaySoundEffect(SEID_MIMIC);

			CListBoxWidget *pActiveCommandList = GetActiveCommandListBox();
			ASSERT(pActiveCommandList);
			const UINT wSelectedLine = pActiveCommandList->GetSelectedLineNumber();

			//Insert commands in paste buffer into command list after cursor position.
			COMMANDPTR_VECTOR *pCommands = GetActiveCommands();
			ASSERT(pCommands);
			pCommands->insert(pCommands->begin() + (wSelectedLine + 1),
					this->commandBuffer.begin(), this->commandBuffer.end());

			//Replicate these commands for repeat use in paste buffer.
			COMMANDPTR_VECTOR tempCommandBuffer;
			for (COMMANDPTR_VECTOR::const_iterator command = this->commandBuffer.begin();
					command != this->commandBuffer.end(); ++command)
			{
				CCharacterCommand *pCommand = *command;
				CCharacterCommand *pCommandCopy = new CCharacterCommand(*pCommand, true);
				tempCommandBuffer.push_back(pCommandCopy);

				//Add any labels to the label list.
				if (pCommand->command == CCharacterCommand::CC_Label)
				{
					//Generate unique label info.
					GenerateUniqueLabelName(pCommand->label);
					pCommand->x = ++this->wIncrementedLabel;
					AddLabel(pCommand);
				}
			}

			resolveForwardReferences(this->commandBuffer);

			this->commandBuffer = tempCommandBuffer;

			const UINT deletedCommands = FilterUnsupportedCommands();

			//Show inserted commands.
			const UINT wTopLine = pActiveCommandList->GetTopLineNumber();
			PopulateCommandDescriptions(pActiveCommandList, *pCommands);
			pActiveCommandList->SetTopLineNumber(wTopLine);

			const UINT selectedLine = wSelectedLine + this->commandBuffer.size() - deletedCommands;
			pActiveCommandList->SelectLine(selectedLine);
			pActiveCommandList->RequestPaint();
			if (IsEditingDefaultScript())
				SetDefaultScriptWidgetStates();
			else
				SetWidgetStates();
		}
		break;

		//Select all script commands.
		case SDLK_a:
		{
			if ((Key.keysym.mod & KMOD_CTRL) == 0) //Ctrl-A
				break;

			CListBoxWidget *pActiveCommandList = GetActiveCommandListBox();
			pActiveCommandList->SelectAllLines();
			pActiveCommandList->RequestPaint();
		}
		break;

		//Copy/paste script commands to/from clipboard text.
		case SDLK_b:
		{
			if (this->bEditingCommand)
				break;
			ASSERT(!this->pCommand);

			if ((Key.keysym.mod & KMOD_CTRL) == 0) //Ctrl-b
				break;

			CListBoxWidget *pActiveCommandList = GetActiveCommandListBox();
			COMMANDPTR_VECTOR *pCommands = GetActiveCommands();

			WSTRING wstrCommandsText;
			if ((Key.keysym.mod & KMOD_SHIFT) == 0)
			{
				//Commands -> clipboard
				const CIDSet selectedLines = pActiveCommandList->GetSelectedLineNumbers();
				for (CIDSet::const_iterator line=selectedLines.begin();
						line!=selectedLines.end(); ++line)
				{
					if (*line < pCommands->size())
						wstrCommandsText += toText(*pCommands, (*pCommands)[*line]);
				}
				if (!wstrCommandsText.empty())
					g_pTheSound->PlaySoundEffect(SEID_POTION);
				CClipboard::SetString(wstrCommandsText);
			} else {  //Ctrl-shift-b
				//Clipboard -> commands
				CClipboard::GetString(wstrCommandsText);

				static const WCHAR delimiters[] = {We('\r'),We('\n'),We(0)};
				COMMANDPTR_VECTOR newCommands;
				WCHAR *pwLine, *pwText = (WCHAR*)wstrCommandsText.c_str();

				//Extract one line at a time and parse.
				pwLine = WCStok(pwText, delimiters);
				while (pwLine)
				{
					WSTRING wstrCommand = pwLine; //make copy
					CCharacterCommand *pCommand = fromText(wstrCommand);
					if (pCommand)
						newCommands.push_back(pCommand);
					pwLine = WCStok(NULL, delimiters);
				}

				if (!newCommands.empty())
				{
					g_pTheSound->PlaySoundEffect(SEID_MIMIC);

					resolveForwardReferences(newCommands);

					const UINT wSelectedLine = pActiveCommandList->GetSelectedLineNumber();
					if (wSelectedLine < pCommands->size())
						pCommands->insert(pCommands->begin() + (wSelectedLine + 1),
							newCommands.begin(), newCommands.end());
					else
						pCommands->insert(pCommands->begin(), newCommands.begin(), newCommands.end());

					const UINT deletedCommands = FilterUnsupportedCommands();

					//Show inserted commands.
					const UINT wTopLine = pActiveCommandList->GetTopLineNumber();
					PopulateCommandDescriptions(pActiveCommandList, *pCommands);
					pActiveCommandList->SetTopLineNumber(wTopLine);

					const UINT selectedLine = wSelectedLine + newCommands.size() - deletedCommands;
					pActiveCommandList->SelectLine(selectedLine);
				} else {
					//Sound for empty or invalid clipboard data.
					g_pTheSound->PlaySoundEffect(SEID_CHECKPOINT);
				}
				if (IsEditingDefaultScript())
					SetDefaultScriptWidgetStates();
				else
					SetWidgetStates();
				Paint();
			}
		}
		break;

		default: break;
	}
}

//*****************************************************************************
void CCharacterDialogWidget::OnRearranged(const UINT dwTagNo)
//Called when the commands list has been reordered.
{
	switch (dwTagNo)
	{
		case TAG_COMMANDSLISTBOX:
		case TAG_DEFAULTCOMMANDSLISTBOX:
		{
			//Update pointers to match the order in list box.
			CListBoxWidget *pActiveCommandList = GetActiveCommandListBox();
			COMMANDPTR_VECTOR *pCommands = GetActiveCommands();

			for (UINT wIndex=0; wIndex<pActiveCommandList->GetItemCount(); ++wIndex)
				(*pCommands)[wIndex] = (CCharacterCommand*)(pActiveCommandList->GetKeyPointerAtLine(wIndex));
			const int nSelectedLine = pActiveCommandList->GetSelectedLineNumber();
			ASSERT(nSelectedLine >= 0);
			const UINT wTopLine = pActiveCommandList->GetTopLineNumber();
			PopulateCommandDescriptions(pActiveCommandList, *pCommands);
			pActiveCommandList->SetTopLineNumber(wTopLine);
			pActiveCommandList->SelectLine(nSelectedLine);
			pActiveCommandList->RequestPaint();
		}
		break;

		default: break;
	}
}

//*****************************************************************************
void CCharacterDialogWidget::OnSelectChange(const UINT dwTagNo)
//Called when a list box selection has changed.
{
	switch (dwTagNo)
	{
		case TAG_ACTIONLISTBOX:
			SetActionWidgetStates();
			this->pAddCommandDialog->Paint();
		break;

		case TAG_GRAPHICLISTBOX:
			ASSERT(this->pCharacter);
			this->pCharacter->wLogicalIdentity = this->pGraphicListBox->GetSelectedItem();
			if (this->pCharacter->wLogicalIdentity == M_NONE)
			{
				//Don't allow visible to be checked when "None" graphic setting is selected.
				this->pIsVisibleButton->SetChecked(false);
				this->pIsVisibleButton->RequestPaint();
				this->pCharacter->bVisible = false;
			}
		break;

		case TAG_ISVISIBLE:
			ASSERT(this->pCharacter);
			if ((this->pCharacter->bVisible = this->pIsVisibleButton->IsChecked()))
			{
				//Don't allow "None" graphic setting when visible is checked.
				if (this->pGraphicListBox->GetSelectedItem() == M_NONE)
				{
					this->pCharacter->wLogicalIdentity = M_CITIZEN;
					this->pGraphicListBox->SelectItem(this->pCharacter->wLogicalIdentity);
					this->pGraphicListBox->RequestPaint();
				}
			}
		break;

		case TAG_CHARACTERNAME:
		{
			CButtonWidget *pButton = DYN_CAST(CButtonWidget*, CWidget*,
					GetWidget(TAG_ADDCHARACTER));
			pButton->Enable(!this->pCharNameText->IsEmpty());
			pButton->RequestPaint();
		}
		break;

		case TAG_CHARACTERLISTBOX:
			SelectCharacter();
		break;

		case TAG_CHARGRAPHICLISTBOX:
			SetCustomGraphic();
		break;

		default: break;
	}
}

//*****************************************************************************
void CCharacterDialogWidget::AddCommand()
//Append the newly created/modified command object to the list of commands.
//Update the command list box.
{
	ASSERT(this->pCommand);
	CCharacterCommand *pCommand = this->pCommand;
	this->pCommand = NULL;

	//Remove any loaded sound that was not used in this command.
	delete this->pSound;
	this->pSound = NULL;

	CListBoxWidget *pActiveCommandListBox = GetActiveCommandListBox();
	COMMANDPTR_VECTOR *pCommands = GetActiveCommands();

	int nSelectedLine = pActiveCommandListBox->GetSelectedLineNumber();
	if (this->bEditingCommand)
	{
		//Command parameters updated.
		this->bEditingCommand = false;
	} else {
		//New command added to list.
		ASSERT((int)pCommands->size() >= nSelectedLine+1);
		COMMANDPTR_VECTOR::iterator insertPoint = pCommands->begin();
		for (int n = 0; n < nSelectedLine+1; ++n)
			++insertPoint;
		pCommands->insert(insertPoint, pCommand);
		nSelectedLine = pActiveCommandListBox->AddItemPointer(pCommand,
				wszEmpty,	//real text added below
				false, nSelectedLine+1);
	}

	AddLabel(pCommand);

	//Refresh script
	const UINT wTopLine = pActiveCommandListBox->GetTopLineNumber();
	PopulateCommandDescriptions(pActiveCommandListBox, *pCommands);
	pActiveCommandListBox->SetTopLineNumber(wTopLine);
	pActiveCommandListBox->SelectLine(nSelectedLine);
}

//*****************************************************************************
void CCharacterDialogWidget::AddLabel(CCharacterCommand* pCommand)
//If command is a label, then adds the label ID+text to the label list.
{
	ASSERT(pCommand);
	if (pCommand->command == CCharacterCommand::CC_Label)
	{
		this->pGotoLabelListBox->AddItem(pCommand->x, pCommand->label.c_str());
		if (!this->pGotoLabelListBox->ItemIsSelected())
			this->pGotoLabelListBox->SelectLine(0);
	}
}

//*****************************************************************************
void CCharacterDialogWidget::AddSound()
//If a sound is already loaded, it will be deleted.
//Otherwise, calling this brings up file dialog to load a new sound effect.
{
	//If a sound is already loaded, mark it for deletion.
	bool bDeletingSound = this->pSound != NULL;
	if (this->pCommand)
	{
		CDbSpeech *pSpeech = this->pCommand->pSpeech;
		if (this->bEditingCommand && pSpeech && pSpeech->GetSound())
		{
			bDeletingSound = true;
			CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
					g_pTheSM->GetScreen(SCR_EditRoom));
			ASSERT(pEditRoomScreen);
			ASSERT(pEditRoomScreen->pRoom);
			pEditRoomScreen->pRoom->MarkDataForDeletion(pSpeech->GetSound());
			ASSERT(pSpeech->GetSound() == this->pSound);
			pSpeech->SetSound(NULL);   //deletes this->pSound
			this->pSound = NULL;
		} else {
			//Delete any newly-loaded sound.
		}
	}
	delete this->pSound;
	this->pSound = NULL;
	if (bDeletingSound) { SetActionWidgetStates(); Paint(); return; }

	//Get sound import path.
	CFiles Files;
	CDbPlayer *pCurrentPlayer = g_pTheDB->GetCurrentPlayer();
	WSTRING wstrImportPath = pCurrentPlayer ? pCurrentPlayer->Settings.GetVar(importSoundPath,
			Files.GetDatPath().c_str()) : Files.GetDatPath();

	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));
	ASSERT(pEditRoomScreen);
	WSTRING wstrImportFile;
	const UINT dwTagNo = pEditRoomScreen->SelectFile(wstrImportPath,
			wstrImportFile, MID_ImportSound, false, EXT_OGG | EXT_WAV);
	if (dwTagNo == TAG_OK)
	{
		//Update the path in player settings, so next time dialog
		//comes up it will have the same path.
		if (pCurrentPlayer)
		{
			pCurrentPlayer->Settings.SetVar(importSoundPath, wstrImportPath.c_str());
			pCurrentPlayer->Update();
		}

		//Load sound effect.
		const bool bNewSound = !this->pSound;
		if (bNewSound)
			this->pSound = g_pTheDB->Data.GetNew();
		CStretchyBuffer data;
		if (Files.ReadFileIntoBuffer(wstrImportFile.c_str(),
				data, true))
		{
			if (g_pTheSound->VerifySound(data))
			{
				//Replace loaded sound, here and in DB on room save.
				this->pSound->data.Clear();
				this->pSound->data.Set((const BYTE*)data,data.Size());
				this->pSound->DataNameText = getFilenameFromPath(wstrImportFile.c_str());
				this->pSound->dwHoldID = pEditRoomScreen->pHold->dwHoldID;
				//Don't need to distinguish between WAV and OGG formats, since
				//CSound is able to figure it out by itself.
				//Hence, we just give it an ID for a format type that is known to be a sound format.
				this->pSound->wDataFormat = DATA_OGG;
				//Play sound as confirmation.
				g_pTheSound->PlayVoice(this->pSound->data);
			} else {
				//Not a valid sound file.
				pEditRoomScreen->ShowOkMessage(MID_FileNotValid);
				if (bNewSound)
				{
					delete this->pSound;
					this->pSound = NULL;
				}
			}
		} else {
			pEditRoomScreen->ShowOkMessage(MID_FileNotFound);
			//Leave existing sound data intact if a replacement was unsuccessfully loaded.
			if (bNewSound)
			{
				delete this->pSound;
				this->pSound = NULL;
			}
		}
	}
	delete pCurrentPlayer;

	SetActionWidgetStates();
	if (this->pParent) this->pParent->Paint();   //refresh screen
	Paint();
}

//*****************************************************************************
void CCharacterDialogWidget::AddVar()
//Adds a new hold variable by the name specified in the text box widget.
{
	CTextBoxWidget *pDialogue = DYN_CAST(CTextBoxWidget*, CWidget*,
			this->pAddCommandDialog->GetWidget(TAG_GOTOLABELTEXT));
	ASSERT(pDialogue);
	const WCHAR *pVarName = pDialogue->GetText();
	AddVar(pVarName);
}

//*****************************************************************************
UINT CCharacterDialogWidget::AddVar(const WCHAR* pVarName)
//Adds a new hold variable by the name specified in the text box widget.
{
	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));
	ASSERT(pEditRoomScreen);
	ASSERT(pEditRoomScreen->pHold);

	if (!WCSlen(pVarName))
	{
		pEditRoomScreen->ShowOkMessage(MID_VarNameEmptyError);
		return 0;
	}

	if (!CDbHold::IsVarNameGoodSyntax(pVarName))
	{
		pEditRoomScreen->ShowOkMessage(MID_VarNameSyntaxError);
		return 0;
	}

	const UINT dwNewVarID = pEditRoomScreen->pHold->AddVar(pVarName);
	if (!dwNewVarID)
	{
		pEditRoomScreen->ShowOkMessage(MID_VarNameDuplicationError);
		return 0;
	}
	const UINT line = this->pVarListBox->AddItem(dwNewVarID, pVarName);
	this->pVarListBox->SelectLine(line);

	CWidget *pButton = this->pAddCommandDialog->GetWidget(TAG_VARREMOVE);
	ASSERT(pButton);
	pButton->Enable();
	if (pButton->IsVisible() && this->pAddCommandDialog->IsVisible())
		pButton->RequestPaint();

	return dwNewVarID;
}

//*****************************************************************************
void CCharacterDialogWidget::ClearPasteBuffer()
//Deletes all commands in the command line paste buffer.
{
	COMMANDPTR_VECTOR::iterator command;
	for (command = this->commandBuffer.begin(); command != this->commandBuffer.end(); ++command)
	{
		CCharacterCommand *pCommand = *command;

		//Mark DB members for deletion.
		if (pCommand->pSpeech)
		{
			CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
					g_pTheSM->GetScreen(SCR_EditRoom));
			ASSERT(pEditRoomScreen);
			if (this->bEditingDefaultScript)
			{
				ASSERT(pEditRoomScreen->pHold);
				pEditRoomScreen->pHold->MarkSpeechForDeletion(pCommand->pSpeech);
			} else {
				ASSERT(pEditRoomScreen->pRoom);
				pEditRoomScreen->pRoom->MarkSpeechForDeletion(pCommand->pSpeech);
			}
		}

		delete pCommand;
	}
	this->commandBuffer.clear();
}

//*****************************************************************************
void CCharacterDialogWidget::DeleteCommands(
//Delete selected commands from the given list box and commands vector.
//
//Params:
	CListBoxWidget *pActiveCommandList, COMMANDPTR_VECTOR& commands)
{
	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));
	ASSERT(pEditRoomScreen);
	ASSERT(pEditRoomScreen->pRoom);

	int nSelectedLine = pActiveCommandList->GetSelectedLineNumber();
	ASSERT(nSelectedLine >= 0);
	UINT wTopLine = pActiveCommandList->GetTopLineNumber();

	CIDSet lines = pActiveCommandList->GetSelectedLineNumbers();
	for (CIDSet::const_reverse_iterator line = lines.rbegin(); line != lines.rend(); ++line)
	{
		const UINT wLine = *line;
		CCharacterCommand *pCommand = (CCharacterCommand*)(pActiveCommandList->GetKeyPointerAtLine(wLine));
		ASSERT(pCommand);
		COMMANDPTR_VECTOR::iterator iter = commands.begin() + wLine;
		ASSERT(iter != commands.end());
		commands.erase(iter);
		pActiveCommandList->RemoveItem(pCommand);
		if (pCommand->command == CCharacterCommand::CC_Label)
			this->pGotoLabelListBox->RemoveItem(pCommand->x);
		if (pCommand->pSpeech)
		{
			if (this->bEditingDefaultScript)
			{
				ASSERT(pEditRoomScreen->pHold);
				pEditRoomScreen->pHold->MarkSpeechForDeletion(pCommand->pSpeech);
			} else

				pEditRoomScreen->pRoom->MarkSpeechForDeletion(pCommand->pSpeech);
		}
		delete pCommand;
	}

	PopulateCommandDescriptions(pActiveCommandList, commands);  //refresh script
	const UINT wLines = pActiveCommandList->GetItemCount();
	if (wTopLine >= wLines)
		wTopLine = wLines - 1;
	if (nSelectedLine >= static_cast<int>(wLines))
		nSelectedLine = wLines - 1;
	pActiveCommandList->SetTopLineNumber(wTopLine);
	if (nSelectedLine >= 0)
		pActiveCommandList->SelectLine(nSelectedLine);
}

//*****************************************************************************
void CCharacterDialogWidget::DeleteVar()
//Delete selected hold var.
{
	ASSERT(this->pVarListBox->GetItemCount() > 0);
	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));
	ASSERT(pEditRoomScreen);
	ASSERT(pEditRoomScreen->pHold);
	if (pEditRoomScreen->ShowYesNoMessage(MID_DeleteVarPrompt) != TAG_YES)
		return;

	const UINT dwVarID = this->pVarListBox->GetSelectedItem();
	const UINT line = this->pVarListBox->GetSelectedLineNumber();
	if (pEditRoomScreen->pHold->DeleteVar(dwVarID))
	{
		PopulateVarList();

		//Select closest var in list.
		const UINT numLines = this->pVarListBox->GetItemCount();
		if (line < numLines)
			this->pVarListBox->SelectLine(line);
		else if (numLines > 0)
			this->pVarListBox->SelectLine(numLines-1);

		//Set button state.
		CWidget *pButton = GetWidget(TAG_VARREMOVE);
		ASSERT(pButton);
		const bool bEnable = this->pVarListBox->GetItemCount() > 0;
		if (!bEnable && GetSelectedWidget() == pButton)
			SelectNextWidget();
		pButton->Enable(bEnable);
		pButton->RequestPaint();
	}
}

//*****************************************************************************
void CCharacterDialogWidget::EditClickedCommand()
//Edit an existing command, which was clicked on.
{
	CListBoxWidget *pActiveCommandList = GetActiveCommandListBox();

	if (!pActiveCommandList->ClickedSelection())
		return;

	void *pCommandPointer = pActiveCommandList->GetSelectedItemPointer();
	if (!pCommandPointer)
		return;
	this->pCommand = (CCharacterCommand*)(pCommandPointer);
	ASSERT(this->pCommand);

	this->bEditingCommand = true;
	this->pActionListBox->SelectItem(this->pCommand->command);
	SetWidgetsFromCommandParameters();
	this->bRetainFields = true; //so SetActionWidgetStates doesn't reset the fields
	SetActionWidgetStates();
	bool bSoundAttached = this->pSound != NULL;

	UINT dwTagNo;
	bool bLoop=true;
	do {
		dwTagNo = this->pAddCommandDialog->Display();
		switch (dwTagNo)
		{
			case TAG_ADDSOUND:
				AddSound();
				if (!this->pSound)
					bSoundAttached = false;
			break;
			case TAG_TESTSOUND: TestSound(); break;
			case TAG_VARADD: AddVar(); break;
			case TAG_VARREMOVE: DeleteVar();	break;
			default:
				bLoop = false;
			break;
		}
	} while (bLoop && !IsDeactivating());

	this->bRetainFields = false;
	if (dwTagNo == TAG_OK)
	{
		COMMANDPTR_VECTOR *pCommands = GetActiveCommands();
		SetCommandParametersFromWidgets(pActiveCommandList, *pCommands);
	}
	else
	{
		//Action was canceled -- reset vars.
		if (!this->bEditingCommand || (!bSoundAttached && this->pSound && !this->pSound->dwDataID))
			delete this->pSound; //delete sound only if it's not yet attached to a record
		this->pCommand = NULL;
		this->pSound = NULL;
		this->bEditingCommand = false;
	}

	if (this->pParent)
		this->pParent->Paint();   //refresh screen
	else Paint();
}

//*****************************************************************************
void CCharacterDialogWidget::EditCustomCharacters()
{
	ASSERT(this->pAddCharacterDialog);

	SetCharacterWidgetStates();

	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));
	ASSERT(pEditRoomScreen);

	UINT dwTagNo;
	bool bLoop=true;
	do {
		pEditRoomScreen->Paint(false);

		dwTagNo = this->pAddCharacterDialog->Display();
		this->pAddCharacterDialog->Show(); //keep visible while performing operations below
		switch (dwTagNo)
		{
			case TAG_ADDCHARACTER: AddCustomCharacter(); break;
			case TAG_DELETECHARACTER: DeleteCustomCharacter(); break;
			case TAG_CUSTOMAVATAR: SetCustomImage(); break;
			case TAG_DEFAULTAVATAR: SetDefaultAvatar(); break;
			case TAG_CUSTOMTILES: SetCustomTiles(); break;
			case TAG_DEFAULTTILES: SetDefaultTiles(); break;
			case TAG_EDITDEFAULTSCRIPT: EditDefaultScriptForCustomNPC(); break;
			default:
				bLoop=false;
			break;
		}
	} while (bLoop && !IsDeactivating());

	//Now hide dialog.
	this->pAddCharacterDialog->Hide();

	//Refresh main graphic list.
	PopulateMainGraphicList();

	if (this->pParent)
		this->pParent->Paint();   //refresh screen
	Paint();
}

//*****************************************************************************
void CCharacterDialogWidget::EditDefaultScriptForCustomNPC()
//Edit the default script for the selected custom NPC.
{
	HoldCharacter *pChar = GetCustomCharacter();
	if (!pChar)
		return;

	//Retrieve the custom NPC's default script.
	if (!this->bEditingDefaultScript)
	{
		if (!pChar->pCommands)
		{
			//Allocate a script object, which will serve as a working copy for the script.
			//This working copy will be repacked and updated to the DB when the hold
			//object is saved.
			pChar->pCommands = new COMMANDPTR_VECTOR;
			CCharacter::LoadCommands(pChar->ExtraVars, *pChar->pCommands);
		}
		this->bEditingDefaultScript = true;
		this->defaultScriptCustomCharID = pChar->dwCharID;
	}

	//Different commands/fields are available in default scripts.
	PopulateImperativeListBox(true);

	PopulateGotoLabelList(*pChar->pCommands);

	PopulateCommandDescriptions(this->pDefaultScriptCommandsListBox, *pChar->pCommands);

	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));
	ASSERT(pEditRoomScreen);

	//Edit script.
	UINT dwTagNo;
	bool bLoop=true;
	do {
		SetDefaultScriptWidgetStates();

		pEditRoomScreen->Paint(false);
		pEditRoomScreen->SetUpdateRectAfterMessage(false);
		this->pAddCharacterDialog->Paint();

		dwTagNo = this->pScriptDialog->Display();
		this->pScriptDialog->Show(); //keep visible while performing operations below
		switch (dwTagNo)
		{
			case TAG_DELETECOMMAND2:
			{
				ASSERT(this->pDefaultScriptCommandsListBox->GetItemCount() > 0);
				if (pEditRoomScreen->ShowYesNoMessage(MID_DeleteCommandPrompt) == TAG_YES)
					DeleteCommands(this->pDefaultScriptCommandsListBox, *pChar->pCommands);

				//Repaint dialogs after asking question.
				this->pAddCharacterDialog->Paint(false);
				this->pScriptDialog->Paint(false);
				pEditRoomScreen->UpdateRect();
			}
			break;

			case TAG_ADDCOMMAND2:
			{
				ASSERT(this->pAddCommandDialog);
				ASSERT(!this->pCommand);
				ASSERT(!this->pSound);

				SetActionWidgetStates();
				this->bRetainFields = true;
				UINT dwTagNo;
				bool bLoop=true;
				do {
					dwTagNo = this->pAddCommandDialog->Display();
					switch (dwTagNo)
					{
						case TAG_ADDSOUND: AddSound(); break;
						case TAG_TESTSOUND: TestSound(); break;
						case TAG_VARADD: AddVar(); break;
						case TAG_VARREMOVE: DeleteVar();	break;
						default:
							bLoop = false;
						break;
					}
				} while (bLoop);
				this->bRetainFields = false;
				if (dwTagNo == TAG_OK)
				{
					//Begin adding this command.
					const CCharacterCommand::CharCommand command =
						(CCharacterCommand::CharCommand)this->pActionListBox->GetSelectedItem();
					this->pCommand = new CCharacterCommand();
					this->pCommand->command = command;
					SetCommandParametersFromWidgets(this->pDefaultScriptCommandsListBox, *pChar->pCommands);
				} else {
					//Command addition was canceled.
					//Clear any data generated for the command.
					ASSERT(!this->pSound || !this->pSound->dwDataID);	//should be fresh (not added to DB yet)
					delete this->pSound;
					this->pSound = NULL;
				}
			}
			break;

			default:
				bLoop = false;
			break;
		}
	} while (bLoop && !IsDeactivating());

	//Now hide dialog.
	this->pScriptDialog->Hide();
	pEditRoomScreen->SetUpdateRectAfterMessage(true);

	//Undo default script-specific command options.
	PopulateImperativeListBox();

	if (IsDeactivating())
		return; //indicates the user is being prompted for command parameters -- don't do anything below yet

	//Save edited script back to custom NPC.
	FinishEditingDefaultScript();

	//Reload the gotos for the NPC being edited.
	PopulateGotoLabelList(this->commands);

	this->pAddCharacterDialog->Paint();
}

//*****************************************************************************
UINT CCharacterDialogWidget::FilterUnsupportedCommands()
//Some command options are not supported outside of global script definitions,
//i.e., custom NPC default scripts.
//When commands are pasted into a script,
//ensure only supported commands are retained.
//
//Returns: number of commands deleted
{
	return 0; //nothing needs filtering yet

/*
	if (this->bEditingDefaultScript)
		return 0; //no filtering needed

	COMMANDPTR_VECTOR *pCommands = GetActiveCommands();
	UINT index=0, deletedCount=0;
	while (index<pCommands->size())
	{
		COMMANDPTR_VECTOR::iterator commandIter = pCommands->begin() + index;
		CCharacterCommand *pCommand = *commandIter;
		bool bDeleted = false;
		switch (pCommand->command)
		{
			case CCharacterCommand::CC_Imperative:
				//"Make Global" imperative only available in default scripts.
				if (pCommand->x == ScriptFlag::MakeGlobal)
				{
					pCommands->erase(commandIter);
					delete pCommand;
					bDeleted = true;
					++deletedCount;
				}
			break;

			default: break;
		}

		if (!bDeleted)
			++index;
	}

	return deletedCount;
*/
}

//*****************************************************************************
void CCharacterDialogWidget::FinishEditingDefaultScript()
//A custom NPC's default script is now no longer being edited.
{
	ASSERT(this->bEditingDefaultScript);

	this->bEditingDefaultScript = false;
	this->defaultScriptCustomCharID = 0;
}

//*****************************************************************************
void CCharacterDialogWidget::GenerateUniqueLabelName(WSTRING& label) const
//Alters 'label', if needed, to ensure it is not a duplicate of any
//label name text.
{
	UINT labelID, num=0;
	WSTRING alteredName = label;
	bool bFound;
	do {
		UINT tempIndex = 0;
		labelID = findTextMatch(this->pGotoLabelListBox, alteredName.c_str(), tempIndex, bFound);
		//If a label matching this label's name is found in the label list,
		//alter this one's name until a unique label name is generated.
		if (bFound)
		{
			WCHAR temp[12];
			alteredName = label;
			alteredName += _itoW(num++, temp, 10);
		}
	} while (bFound);
	label = alteredName;
}

//*****************************************************************************
WSTRING CCharacterDialogWidget::GetCommandDesc(
//Returns: a string describing the command in normal language.
//
//Params:
	const COMMANDPTR_VECTOR& commands,
	CCharacterCommand* pCommand)
const
{
	WSTRING wstr;

	wstr += GetPrettyPrinting(commands, pCommand, 6, 3); //indent the If conditions considerably

	//Call language-specific version of method.
	switch (Language::GetLanguage())
	{
		case Language::English:
		case Language::French:
		case Language::Russian:
			wstr += GetCommandDesc_English(commands, *pCommand);
		break;
		default:
			//Language not supported -- just use English grammar.
			wstr += GetCommandDesc_English(commands, *pCommand);
		break;
	}

	return wstr;
}

//*****************************************************************************
WSTRING CCharacterDialogWidget::GetCommandDesc_English(
//Print command name.
//
//Params:
	const COMMANDPTR_VECTOR& commands,
	const CCharacterCommand& command)
const
{
	//Print command name.
	WSTRING wstr = this->pActionListBox->GetTextForKey(command.command);
	wstr += wszSpace;

	WCHAR temp[16];
	switch (command.command)
	{
		case CCharacterCommand::CC_AppearAt:
		case CCharacterCommand::CC_ActivateItemAt:
			wstr += wszLeftParen;
			wstr += _itoW(command.x, temp, 10);
			wstr += wszComma;
			wstr += _itoW(command.y, temp, 10);
			wstr += wszRightParen;
		break;
		case CCharacterCommand::CC_MoveRel:
			wstr += wszLeftParen;
			wstr += _itoW((int)command.x, temp, 10);
			wstr += wszComma;
			wstr += _itoW((int)command.y, temp, 10);
			wstr += wszRightParen;
			if (command.w)
			{
				wstr += wszSpace;
				wstr += g_pTheDB->GetMessageText(MID_NoTurning);
			}
			if (command.h)
			{
				wstr += wszSpace;
				wstr += g_pTheDB->GetMessageText(MID_SingleStep);
			}
		break;
		case CCharacterCommand::CC_MoveTo:
			if (command.flags)
			{
				UINT wBitfield = 1;
				for (UINT wBits = 0; wBits<32; ++wBits, wBitfield *= 2)
				{
					if ((command.flags & wBitfield) == wBitfield)
					{
						wstr += this->pWaitFlagsListBox->GetTextForKey(wBitfield);
						wstr += wszSpace;
					}
				}
			} else {
				wstr += wszLeftParen;
				wstr += _itoW((int)command.x, temp, 10);
				wstr += wszComma;
				wstr += _itoW((int)command.y, temp, 10);
				wstr += wszRightParen;
			}
			if (command.w)
			{
				wstr += wszSpace;
				wstr += g_pTheDB->GetMessageText(MID_NoTurning);
			}
			if (command.h)
			{
				wstr += wszSpace;
				wstr += g_pTheDB->GetMessageText(MID_SingleStep);
			}
		break;
		case CCharacterCommand::CC_FaceDirection:
		case CCharacterCommand::CC_WaitForPlayerToFace:
		case CCharacterCommand::CC_WaitForPlayerToMove:
			wstr += this->pDirectionListBox->GetTextForKey(command.x);
		break;

		case CCharacterCommand::CC_WaitForTurn:
			wstr += _itoW(command.x, temp, 10);
		break;
		case CCharacterCommand::CC_Wait:
			wstr += _itoW(command.x, temp, 10);
			wstr += wszSpace;
			wstr += g_pTheDB->GetMessageText(MID_Turns);
		break;
		case CCharacterCommand::CC_WaitForCueEvent:
			wstr += this->pEventListBox->GetTextForKey(command.x);
		break;
		case CCharacterCommand::CC_WaitForRect:
		case CCharacterCommand::CC_WaitForNotRect:
		{
			UINT wBitfield = 1;
			for (UINT wBits = 0; wBits<32; ++wBits, wBitfield *= 2)
			{
				if ((command.flags & wBitfield) == wBitfield)
				{
					wstr += this->pWaitFlagsListBox->GetTextForKey(wBitfield);
					wstr += wszSpace;
				}
			}
		}
		//no break
		case CCharacterCommand::CC_WaitForHalph:
		case CCharacterCommand::CC_WaitForNotHalph:
		case CCharacterCommand::CC_WaitForMonster:
		case CCharacterCommand::CC_WaitForNotMonster:
		case CCharacterCommand::CC_WaitForCharacter:
		case CCharacterCommand::CC_WaitForNotCharacter:
		case CCharacterCommand::CC_WaitForNoBuilding:
			wstr += g_pTheDB->GetMessageText(MID_At);
			wstr += wszSpace;
			wstr += wszLeftParen;
			wstr += _itoW(command.x, temp, 10);
			wstr += wszComma;
			wstr += _itoW(command.y, temp, 10);
			wstr += wszRightParen;
			wstr += wszHyphen;
			wstr += wszLeftParen;
			wstr += _itoW(command.x + command.w, temp, 10);
			wstr += wszComma;
			wstr += _itoW(command.y + command.h, temp, 10);
			wstr += wszRightParen;
		break;
		case CCharacterCommand::CC_WaitForDoorTo:
			wstr += this->pOpenCloseListBox->GetTextForKey(command.w);
			wstr += wszSpace;
			wstr += g_pTheDB->GetMessageText(MID_At);
			wstr += wszSpace;
			wstr += wszLeftParen;
			wstr += _itoW(command.x, temp, 10);
			wstr += wszComma;
			wstr += _itoW(command.y, temp, 10);
			wstr += wszRightParen;
		break;

		case CCharacterCommand::CC_BuildMarker:
			wstr += this->pBuildItemsListBox->GetTextForKey(command.flags);
			wstr += wszSpace;
			wstr += g_pTheDB->GetMessageText(MID_At);
			wstr += wszSpace;
			wstr += wszLeftParen;
			wstr += _itoW(command.x, temp, 10);
			wstr += wszComma;
			wstr += _itoW(command.y, temp, 10);
			wstr += wszRightParen;
			wstr += wszHyphen;
			wstr += wszLeftParen;
			wstr += _itoW(command.x + command.w, temp, 10);
			wstr += wszComma;
			wstr += _itoW(command.y + command.h, temp, 10);
			wstr += wszRightParen;
		break;

		case CCharacterCommand::CC_Speech:
		{
			CDbSpeech *pSpeech = command.pSpeech;
			ASSERT(pSpeech);
			if (!pSpeech)	//robustness
			{
				wstr += wszQuote;
				wstr += wszQuestionMark;
				wstr += wszQuote;
				break;
			}
			if (pSpeech->wMood != Mood_Normal)
			{
				wstr += this->pMoodListBox->GetTextForKey(pSpeech->wMood);
				wstr += wszSpace;
			}
			if (pSpeech->wCharacter != Speaker_None)
			{
				if (pSpeech->wCharacter != Speaker_Custom)
				{
					WSTRING charName = this->pSpeakerListBox->GetTextForKey(pSpeech->wCharacter);
					wstr += charName.length() ? charName : wszQuestionMark;
				} else {
					wstr += g_pTheDB->GetMessageText(MID_At);
					wstr += wszSpace;
					wstr += wszLeftParen;
					wstr += _itoW(command.x, temp, 10);
					wstr += wszComma;
					wstr += _itoW(command.y, temp, 10);
					wstr += wszRightParen;
				}
				wstr += wszComma;
				wstr += wszSpace;
			}
			if (pSpeech->dwDelay)
			{
				wstr += _itoW(pSpeech->dwDelay, temp, 10);
				wstr += wszComma;
				wstr += wszSpace;
			}
			if (pSpeech->dwDataID)
			{
				//Load sound clip name only from DB.
				wstr += GetDataName(pSpeech->dwDataID);
				wstr += wszComma;
				wstr += wszSpace;
			}
			else if (pSpeech->GetSound() &&
					!((CDbDatum*)pSpeech->GetSound())->DataNameText.empty())
			{
				//Sound exists in object, but not yet in DB.  Just display its name.
				wstr += pSpeech->GetSound()->DataNameText;
				wstr += wszComma;
				wstr += wszSpace;
			}
			wstr += wszQuote;
			wstr += (const WCHAR*)(pSpeech->MessageText);
			wstr += wszQuote;
		}
		break;
		case CCharacterCommand::CC_PlayVideo:
		{
			wstr += GetDataName(command.w);

			wstr += wszSpace;
			wstr += g_pTheDB->GetMessageText(MID_At);
			wstr += wszSpace;
			wstr += wszLeftParen;
			wstr += _itoW((int)command.x, temp, 10);
			wstr += wszComma;
			wstr += _itoW((int)command.y, temp, 10);
			wstr += wszRightParen;
		}
		break;

		case CCharacterCommand::CC_FlushSpeech:
		case CCharacterCommand::CC_SetPlayerSword:
			wstr += this->pOnOffListBox->GetTextForKey(command.x);
		break;

		case CCharacterCommand::CC_SetWaterTraversal:
			wstr += this->pWaterTraversalListBox->GetTextForKey(command.x);
		break;

		case CCharacterCommand::CC_StartGlobalScript:
		{
			WSTRING charName = this->pGlobalScriptListBox->GetTextForKey(command.x);
			wstr += charName.length() ? charName : wszQuestionMark;
		}
		break;

		case CCharacterCommand::CC_WaitForItem:
			wstr += this->pBuildItemsListBox->GetTextForKey(command.flags);
			wstr += wszSpace;
			wstr += g_pTheDB->GetMessageText(MID_At);
			wstr += wszSpace;
			wstr += wszLeftParen;
			wstr += _itoW(command.x, temp, 10);
			wstr += wszComma;
			wstr += _itoW(command.y, temp, 10);
			wstr += wszRightParen;
			wstr += wszHyphen;
			wstr += wszLeftParen;
			wstr += _itoW(command.x + command.w, temp, 10);
			wstr += wszComma;
			wstr += _itoW(command.y + command.h, temp, 10);
			wstr += wszRightParen;
		break;

		case CCharacterCommand::CC_GenerateEntity:
		{
			WSTRING charName = this->pPlayerGraphicListBox->GetTextForKey(command.h);
			wstr += charName.length() ? charName : wszQuestionMark;
			wstr += wszSpace;
			wstr += wszLeftParen;
			wstr += _itoW(command.x, temp, 10);
			wstr += wszComma;
			wstr += _itoW(command.y, temp, 10);
			wstr += wszComma;
			wstr += this->pDirectionListBox2->GetTextForKey(command.w);
			wstr += wszRightParen;
		}
		break;

		case CCharacterCommand::CC_GameEffect:
			wstr += this->pDirectionListBox3->GetTextForKey(command.w);
			wstr += wszSpace;
			wstr += this->pVisualEffectsListBox->GetTextForKey(command.h);
			wstr += wszSpace;
			wstr += g_pTheDB->GetMessageText(MID_At);
			wstr += wszSpace;
			wstr += wszLeftParen;
			wstr += _itoW(command.x, temp, 10);
			wstr += wszComma;
			wstr += _itoW(command.y, temp, 10);
			wstr += wszRightParen;
			wstr += wszSpace;
			wstr += this->pOnOffListBox3->GetTextForKey(command.flags);
		break;

		case CCharacterCommand::CC_Imperative:
			wstr += this->pImperativeListBox->GetTextForKey(command.x);
		break;

		case CCharacterCommand::CC_SetMusic:
			if (command.label.size())
				wstr += command.label;
			else if (int(command.x) == SONGID_CUSTOM)
				wstr += GetDataName(command.y);
			else
				wstr += this->pMusicListBox->GetTextForKey(command.x);
		break;

		case CCharacterCommand::CC_Label:
			wstr += wszQuote;
			wstr += command.label;
			wstr += wszQuote;
		break;
		case CCharacterCommand::CC_Question:
		{
			CDbSpeech *pSpeech = command.pSpeech;
			ASSERT(pSpeech);
			wstr += wszQuote;
			if (!pSpeech)	//robustness
				wstr += wszQuestionMark;
			else
				wstr += (const WCHAR*)(pSpeech->MessageText);
			wstr += wszQuote;
		}
		break;
		case CCharacterCommand::CC_AnswerOption:
		{
			const CCharacterCommand *pGotoCommand = GetCommandWithLabel(commands, command.x);
			WSTRING wstrGoto = this->pActionListBox->GetTextForKey(CCharacterCommand::CC_GoTo);
			CDbSpeech *pSpeech = command.pSpeech;
			ASSERT(pSpeech);
			wstr += wszQuote;
			if (!pSpeech)	//robustness
				wstr += wszQuestionMark;
			else
				wstr += (const WCHAR*)(pSpeech->MessageText);
			wstr += wszQuote;
			wstr += wszSpace;
			wstr += wszColon;
			wstr += wszSpace;
			wstr += wstrGoto;
			wstr += wszSpace;
			wstr += pGotoCommand ? pGotoCommand->label : wszQuestionMark;
		}
		break;
		case CCharacterCommand::CC_GoTo:
		case CCharacterCommand::CC_GotoIf:
		{
			const CCharacterCommand *pGotoCommand = GetCommandWithLabel(commands, command.x);
			wstr += wszQuote;
			wstr += pGotoCommand ? pGotoCommand->label : wszQuestionMark;
			wstr += wszQuote;
		}
		break;

		case CCharacterCommand::CC_LevelEntrance:
		{
			CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
					g_pTheSM->GetScreen(SCR_EditRoom));
			ASSERT(pEditRoomScreen->pHold);
			CEntranceData *pEntrance = pEditRoomScreen->pHold->GetEntrance(command.x);

			if (command.y)
			{
				wstr += wszLeftParen;
				wstr += g_pTheDB->GetMessageText(MID_SkipEntranceDisplay);
				wstr += wszRightParen;
				wstr += wszSpace;
			}
			if (!pEntrance)
				wstr += g_pTheDB->GetMessageText(MID_DefaultExit);
			else
			{
				wstr += wszQuote;
				wstr += pEntrance->GetPositionDescription();
				wstr += wszQuote;
			}
		}
		break;

		case CCharacterCommand::CC_VarSet:
		{
			const WCHAR *wszVarName = this->pVarListBox->GetTextForKey(command.x);
			wstr += WCSlen(wszVarName) ? wszVarName : wszQuestionMark;
			wstr += wszSpace;
			switch (command.y)
			{
				case ScriptVars::AppendText: wstr += wszPlus; //no break
				case ScriptVars::Assign:
				case ScriptVars::AssignText: wstr += wszEqual; break;
				case ScriptVars::Inc: wstr += wszPlus; break;
				case ScriptVars::Dec: wstr += wszHyphen; break;
				case ScriptVars::MultiplyBy: wstr += wszAsterisk; break;
				case ScriptVars::DivideBy: wstr += wszForwardSlash; break;
				case ScriptVars::Mod: wstr += wszPercent; break;
				default: wstr += wszQuestionMark; break;
			}
			wstr += wszSpace;
			switch (command.y)
			{
				case ScriptVars::AppendText:
				case ScriptVars::AssignText:
					wstr += wszQuote;
					wstr += command.label;
					wstr += wszQuote;
				break;
				default:
					if (!command.label.empty())
						wstr += command.label;
					else
						wstr += _itoW(command.w, temp, 10);
				break;
			}
		}
		break;

		case CCharacterCommand::CC_WaitForVar:
		{
			const WCHAR *wszVarName = this->pVarListBox->GetTextForKey(command.x);
			wstr += WCSlen(wszVarName) ? wszVarName : wszQuestionMark;
			wstr += wszSpace;
			switch (command.y)
			{
				case ScriptVars::Equals:
				case ScriptVars::EqualsText: wstr += wszEqual; break;
				case ScriptVars::Greater: wstr += wszCloseAngle; break;
				case ScriptVars::Less: wstr += wszOpenAngle; break;
				default: wstr += wszQuestionMark; break;
			}
			wstr += wszSpace;
			switch (command.y)
			{
				case ScriptVars::EqualsText:
					wstr += wszQuote;
					wstr += command.label;
					wstr += wszQuote;
				break;
				default:
					if (!command.label.empty())
						wstr += command.label;
					else
						wstr += _itoW(command.w, temp, 10);
				break;
			}
		}
		break;

		case CCharacterCommand::CC_SetPlayerAppearance:
		case CCharacterCommand::CC_SetNPCAppearance:
		{
			WSTRING charName = this->pPlayerGraphicListBox->GetTextForKey(command.x);
			wstr += charName.length() ? charName : wszQuestionMark;
		}
		break;

		case CCharacterCommand::CC_CutScene:
			wstr += _itoW(command.x, temp, 10);
		break;

		case CCharacterCommand::CC_AmbientSoundAt:
			wstr += wszLeftParen;
			wstr += _itoW(command.x, temp, 10);
			wstr += wszComma;
			wstr += _itoW(command.y, temp, 10);
			wstr += wszRightParen;
			wstr += wszSpace;
			//NO BREAK

		case CCharacterCommand::CC_AmbientSound:
			if (command.h && command.w)
			{
				wstr += g_pTheDB->GetMessageText(MID_LoopSound);
				wstr += wszSpace;
			}
			if (command.w)
				wstr += GetDataName(command.w);
			else
				wstr += g_pTheDB->GetMessageText(MID_Off);
		break;

		case CCharacterCommand::CC_Appear:
		case CCharacterCommand::CC_Disappear:
		case CCharacterCommand::CC_EndScript:
		case CCharacterCommand::CC_TurnIntoMonster:
		case CCharacterCommand::CC_WaitForCleanRoom:
		case CCharacterCommand::CC_WaitForPlayerToTouchMe:
		case CCharacterCommand::CC_EndScriptOnExit:
		case CCharacterCommand::CC_If:
		case CCharacterCommand::CC_IfElse:
		case CCharacterCommand::CC_IfEnd:
		break;

		default: break;
	}

	return wstr;
}

//*****************************************************************************
HoldCharacter* CCharacterDialogWidget::GetCustomCharacter()
//Return: pointer to selected custom character record
{
	const UINT dwSelectedCharID = this->pCharListBox->GetSelectedItem();
	if (!dwSelectedCharID)
		return NULL; //no character selected

	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));
	ASSERT(pEditRoomScreen->pHold);
	return pEditRoomScreen->pHold->GetCharacter(dwSelectedCharID);
}

//*****************************************************************************
WSTRING CCharacterDialogWidget::GetDataName(const UINT dwID) const
//Load name of data object from DB.
{
	WSTRING wstr = g_pTheDB->Data.GetNameFor(dwID);
	if (wstr.empty())
		wstr = wszQuestionMark;
	return wstr;
}

//*****************************************************************************
WSTRING CCharacterDialogWidget::GetPrettyPrinting(
//Add indenting to clarify the code flow.
//
//Params:
	const COMMANDPTR_VECTOR& commands,
	CCharacterCommand* pCommand,
	const UINT ifIndent, const UINT tabSize)
const
{
	ASSERT(pCommand);

	WSTRING wstr;

	//Determine indentation for command.
	if (pCommand->command == CCharacterCommand::CC_Label)  //labels always have indent 0
		return wstr;

	UINT wNestDepth = 0, wIndent = 2;  //insert past labels

	bool bIfCondition = false;
	for (COMMANDPTR_VECTOR::const_iterator command = commands.begin();
			(*command) != pCommand && command != commands.end(); ++command)
	{
		bIfCondition = false;
		switch ((*command)->command)
		{
			case CCharacterCommand::CC_If:
				bIfCondition = true;
				++wNestDepth; //indent inside of if block
			break;
			case CCharacterCommand::CC_IfEnd:
				if (wNestDepth)
					--wNestDepth;
				else
					wstr += wszExclamation;	//superfluous IfEnd
			break;
			default: break;
		}
	}

	//Unnest If block markers.
	switch (pCommand->command)
	{
		case CCharacterCommand::CC_IfEnd:
		case CCharacterCommand::CC_IfElse:
			if (wNestDepth)
				--wNestDepth;
			else
				wstr += wszExclamation;	//superfluous IfEnd
		//no break
		case CCharacterCommand::CC_Disappear:
		case CCharacterCommand::CC_EndScript:
		case CCharacterCommand::CC_EndScriptOnExit:
		case CCharacterCommand::CC_FlushSpeech:
		case CCharacterCommand::CC_GoTo:
		case CCharacterCommand::CC_If:
		case CCharacterCommand::CC_Imperative:
		case CCharacterCommand::CC_Label:
		case CCharacterCommand::CC_LevelEntrance:
		case CCharacterCommand::CC_SetMusic:
		case CCharacterCommand::CC_Speech:
		case CCharacterCommand::CC_TurnIntoMonster:
		case CCharacterCommand::CC_SetPlayerSword:
		case CCharacterCommand::CC_SetWaterTraversal:
		case CCharacterCommand::CC_StartGlobalScript:
		case CCharacterCommand::CC_AnswerOption:
		case CCharacterCommand::CC_AmbientSound:
		case CCharacterCommand::CC_AmbientSoundAt:
		case CCharacterCommand::CC_PlayVideo:
			if (bIfCondition)
				wstr += wszQuestionMark;	//questionable If condition
		break;

		case CCharacterCommand::CC_VarSet:
			if (bIfCondition)
				wstr += wszQuestionMark;	//questionable If condition
		//no break
		case CCharacterCommand::CC_WaitForVar:
		{
			//Verify integrity of hold var refs.
			switch (pCommand->y)
			{
				case ScriptVars::AppendText:
				case ScriptVars::AssignText:
				break;
				default:
					if (!pCommand->label.empty()) //an expression is used as an operand
					{
						CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
								g_pTheSM->GetScreen(SCR_EditRoom));
						ASSERT(pEditRoomScreen);
						ASSERT(pEditRoomScreen->pHold);
						UINT index=0;
						if (!CCharacter::IsValidExpression(pCommand->label.c_str(), index, pEditRoomScreen->pHold))
							wstr += wszAsterisk; //expression is not valid
					}
				break;
			}
		}
		break;

		//Deprecated commands.
		case CCharacterCommand::CC_GotoIf:
		case CCharacterCommand::CC_WaitForHalph:
		case CCharacterCommand::CC_WaitForNotHalph:
		case CCharacterCommand::CC_WaitForMonster:
		case CCharacterCommand::CC_WaitForNotMonster:
		case CCharacterCommand::CC_WaitForCharacter:
		case CCharacterCommand::CC_WaitForNotCharacter:
			wstr += wszAsterisk;
		break;
		default: break;
	}

	if (bIfCondition)
	{
		wIndent += ifIndent;
		if (bIfCondition)
			if (wNestDepth)  //...but don't include If indentation in the code block
				--wNestDepth;
	}

	wstr.insert(wstr.end(), wIndent + wNestDepth*tabSize, W_t(' '));

	return wstr;
}

//*****************************************************************************
void CCharacterDialogWidget::PopulateImperativeListBox(const bool /*bDefaultScript*/) //[default=false]
//Some imperatives are only supported in default NPC scripts.
{
	this->pImperativeListBox->Clear();

	this->pImperativeListBox->AddItem(ScriptFlag::Vulnerable, g_pTheDB->GetMessageText(MID_Vulnerable));
	this->pImperativeListBox->AddItem(ScriptFlag::Invulnerable, g_pTheDB->GetMessageText(MID_Invulnerable));
	this->pImperativeListBox->AddItem(ScriptFlag::MissionCritical, g_pTheDB->GetMessageText(MID_MissionCritical));
	this->pImperativeListBox->AddItem(ScriptFlag::RequiredToConquer, g_pTheDB->GetMessageText(MID_RequiredToConquer));
	this->pImperativeListBox->AddItem(ScriptFlag::Safe, g_pTheDB->GetMessageText(MID_Safe));
	this->pImperativeListBox->AddItem(ScriptFlag::SwordSafeToPlayer, g_pTheDB->GetMessageText(MID_SwordSafeToPlayer));
	this->pImperativeListBox->AddItem(ScriptFlag::Deadly, g_pTheDB->GetMessageText(MID_Deadly));
	this->pImperativeListBox->AddItem(ScriptFlag::Die, g_pTheDB->GetMessageText(MID_Die));
	this->pImperativeListBox->AddItem(ScriptFlag::DieSpecial, g_pTheDB->GetMessageText(MID_DieSpecial));
	this->pImperativeListBox->AddItem(ScriptFlag::EndWhenKilled, g_pTheDB->GetMessageText(MID_EndWhenKilled));
	this->pImperativeListBox->AddItem(ScriptFlag::DirectBeelining, g_pTheDB->GetMessageText(MID_DirectBeelining));
	this->pImperativeListBox->AddItem(ScriptFlag::FlexibleBeelining, g_pTheDB->GetMessageText(MID_FlexibleBeelining));
	this->pImperativeListBox->AddItem(ScriptFlag::GhostDisplay, g_pTheDB->GetMessageText(MID_NPCGhostDisplay));
	this->pImperativeListBox->AddItem(ScriptFlag::NoGhostDisplay, g_pTheDB->GetMessageText(MID_NPCNoGhostDisplay));
	this->pImperativeListBox->SelectLine(0);
}

//*****************************************************************************
void CCharacterDialogWidget::PopulateCommandListBox()
//Add to list all usable script commands.
{
	this->pActionListBox->AddItem(CCharacterCommand::CC_Appear, g_pTheDB->GetMessageText(MID_Appear));
	this->pActionListBox->AddItem(CCharacterCommand::CC_AppearAt, g_pTheDB->GetMessageText(MID_AppearAt));
	this->pActionListBox->AddItem(CCharacterCommand::CC_Disappear, g_pTheDB->GetMessageText(MID_Disappear));
	this->pActionListBox->AddItem(CCharacterCommand::CC_MoveTo, g_pTheDB->GetMessageText(MID_MoveTo));
	this->pActionListBox->AddItem(CCharacterCommand::CC_MoveRel, g_pTheDB->GetMessageText(MID_MoveRel));
	this->pActionListBox->AddItem(CCharacterCommand::CC_FaceDirection, g_pTheDB->GetMessageText(MID_FaceDirection));
	this->pActionListBox->AddItem(CCharacterCommand::CC_ActivateItemAt, g_pTheDB->GetMessageText(MID_StrikeOrbAt));
	this->pActionListBox->AddItem(CCharacterCommand::CC_Speech, g_pTheDB->GetMessageText(MID_Speech));
	this->pActionListBox->AddItem(CCharacterCommand::CC_FlushSpeech, g_pTheDB->GetMessageText(MID_FlushSpeech));
	this->pActionListBox->AddItem(CCharacterCommand::CC_AmbientSound, g_pTheDB->GetMessageText(MID_AmbientSound));
	this->pActionListBox->AddItem(CCharacterCommand::CC_AmbientSoundAt, g_pTheDB->GetMessageText(MID_AmbientSoundAt));
	this->pActionListBox->AddItem(CCharacterCommand::CC_Imperative, g_pTheDB->GetMessageText(MID_Imperative));
	this->pActionListBox->AddItem(CCharacterCommand::CC_BuildMarker, g_pTheDB->GetMessageText(MID_BuildMarker));
	this->pActionListBox->AddItem(CCharacterCommand::CC_LevelEntrance, g_pTheDB->GetMessageText(MID_GotoLevelEntrance));
	this->pActionListBox->AddItem(CCharacterCommand::CC_PlayVideo, g_pTheDB->GetMessageText(MID_PlayVideo));
	this->pActionListBox->AddItem(CCharacterCommand::CC_SetMusic, g_pTheDB->GetMessageText(MID_SetMusic));
	this->pActionListBox->AddItem(CCharacterCommand::CC_SetPlayerAppearance, g_pTheDB->GetMessageText(MID_SetPlayerAppearance));
	this->pActionListBox->AddItem(CCharacterCommand::CC_SetNPCAppearance, g_pTheDB->GetMessageText(MID_SetNPCAppearance));
	this->pActionListBox->AddItem(CCharacterCommand::CC_SetPlayerSword, g_pTheDB->GetMessageText(MID_SetPlayerSword));
	this->pActionListBox->AddItem(CCharacterCommand::CC_SetWaterTraversal, g_pTheDB->GetMessageText(MID_SetWaterTraversal));
	this->pActionListBox->AddItem(CCharacterCommand::CC_CutScene, g_pTheDB->GetMessageText(MID_CutScene));
	this->pActionListBox->AddItem(CCharacterCommand::CC_AnswerOption, g_pTheDB->GetMessageText(MID_AnswerOption));
	this->pActionListBox->AddItem(CCharacterCommand::CC_Question, g_pTheDB->GetMessageText(MID_Question));
	this->pActionListBox->AddItem(CCharacterCommand::CC_Wait, g_pTheDB->GetMessageText(MID_Wait));
	this->pActionListBox->AddItem(CCharacterCommand::CC_WaitForCueEvent, g_pTheDB->GetMessageText(MID_WaitForEvent));
	this->pActionListBox->AddItem(CCharacterCommand::CC_WaitForRect, g_pTheDB->GetMessageText(MID_WaitForEntity));
	this->pActionListBox->AddItem(CCharacterCommand::CC_WaitForNotRect, g_pTheDB->GetMessageText(MID_WaitWhileEntity));
	this->pActionListBox->AddItem(CCharacterCommand::CC_WaitForDoorTo, g_pTheDB->GetMessageText(MID_WaitForDoorTo));
	this->pActionListBox->AddItem(CCharacterCommand::CC_WaitForTurn, g_pTheDB->GetMessageText(MID_WaitForTurn));
	this->pActionListBox->AddItem(CCharacterCommand::CC_WaitForCleanRoom, g_pTheDB->GetMessageText(MID_WaitForCleanRoom));
	this->pActionListBox->AddItem(CCharacterCommand::CC_WaitForPlayerToFace, g_pTheDB->GetMessageText(MID_WaitForPlayerToFace));
	this->pActionListBox->AddItem(CCharacterCommand::CC_WaitForPlayerToMove, g_pTheDB->GetMessageText(MID_WaitForPlayerToMove));
	this->pActionListBox->AddItem(CCharacterCommand::CC_WaitForPlayerToTouchMe, g_pTheDB->GetMessageText(MID_WaitForPlayerToTouchMe));
	this->pActionListBox->AddItem(CCharacterCommand::CC_WaitForNoBuilding, g_pTheDB->GetMessageText(MID_WaitForNoBuilding));
	this->pActionListBox->AddItem(CCharacterCommand::CC_WaitForItem, g_pTheDB->GetMessageText(MID_WaitForItem));
	this->pActionListBox->AddItem(CCharacterCommand::CC_WaitForVar, g_pTheDB->GetMessageText(MID_WaitForVar));
	this->pActionListBox->AddItem(CCharacterCommand::CC_VarSet, g_pTheDB->GetMessageText(MID_VarSet));
	this->pActionListBox->AddItem(CCharacterCommand::CC_If, g_pTheDB->GetMessageText(MID_If));
	this->pActionListBox->AddItem(CCharacterCommand::CC_IfElse, g_pTheDB->GetMessageText(MID_IfElse));
	this->pActionListBox->AddItem(CCharacterCommand::CC_IfEnd, g_pTheDB->GetMessageText(MID_IfEnd));
	this->pActionListBox->AddItem(CCharacterCommand::CC_Label, g_pTheDB->GetMessageText(MID_Label));
	this->pActionListBox->AddItem(CCharacterCommand::CC_GoTo, g_pTheDB->GetMessageText(MID_GoTo));
	this->pActionListBox->AddItem(CCharacterCommand::CC_TurnIntoMonster, g_pTheDB->GetMessageText(MID_TurnIntoMonster));
	this->pActionListBox->AddItem(CCharacterCommand::CC_GameEffect, g_pTheDB->GetMessageText(MID_VisualEffect));
	this->pActionListBox->AddItem(CCharacterCommand::CC_GenerateEntity, g_pTheDB->GetMessageText(MID_GenerateEntity));
	this->pActionListBox->AddItem(CCharacterCommand::CC_StartGlobalScript, g_pTheDB->GetMessageText(MID_StartGlobalScript));
	this->pActionListBox->AddItem(CCharacterCommand::CC_EndScriptOnExit, g_pTheDB->GetMessageText(MID_EndScriptOnExit));
	this->pActionListBox->AddItem(CCharacterCommand::CC_EndScript, g_pTheDB->GetMessageText(MID_EndScript));
	this->pActionListBox->SelectLine(0);
}

//*****************************************************************************
void CCharacterDialogWidget::PopulateEventListBox()
//Add all cue events that character can check for to event list.
{
	this->pEventListBox->AddItem(CID_RoomConquerPending, g_pTheDB->GetMessageText(MID_ConquerRoomPending));
	this->pEventListBox->AddItem(CID_AllTarRemoved, g_pTheDB->GetMessageText(MID_AllTarRemoved));
	this->pEventListBox->AddItem(CID_AllTrapdoorsRemoved, g_pTheDB->GetMessageText(MID_AllTrapdoorsRemoved));
	this->pEventListBox->AddItem(CID_AshToFegundo, g_pTheDB->GetMessageText(MID_AshToPhoenix));
	this->pEventListBox->AddItem(CID_BombExploded, g_pTheDB->GetMessageText(MID_BombExploded));
	this->pEventListBox->AddItem(CID_CommandKeyPressed, g_pTheDB->GetMessageText(MID_CommandKeyPressed));
	this->pEventListBox->AddItem(CID_CrumblyWallDestroyed, g_pTheDB->GetMessageText(MID_CrumblyWallDestroyed));
	this->pEventListBox->AddItem(CID_DoublePlaced, g_pTheDB->GetMessageText(MID_DoublePlaced));
	this->pEventListBox->AddItem(CID_EggHatched, g_pTheDB->GetMessageText(MID_EggHatched));
	this->pEventListBox->AddItem(CID_EvilEyeWoke, g_pTheDB->GetMessageText(MID_EvilEyeWoke));
	this->pEventListBox->AddItem(CID_BriarExpanded, g_pTheDB->GetMessageText(MID_FlowExpanded));
	this->pEventListBox->AddItem(CID_FuseBurning, g_pTheDB->GetMessageText(MID_FuseBurning));
	this->pEventListBox->AddItem(CID_GelBabyFormed, g_pTheDB->GetMessageText(MID_GelBabyFormed));
	this->pEventListBox->AddItem(CID_GelGrew, g_pTheDB->GetMessageText(MID_GelGrew));
	this->pEventListBox->AddItem(CID_HalphEntered, g_pTheDB->GetMessageText(MID_HalphEntered));
	this->pEventListBox->AddItem(CID_HitObstacle, g_pTheDB->GetMessageText(MID_HitObstacle));
	this->pEventListBox->AddItem(CID_LightToggled, g_pTheDB->GetMessageText(MID_LightToggled));
	this->pEventListBox->AddItem(CID_MirrorShattered, g_pTheDB->GetMessageText(MID_MirrorShattered));
	this->pEventListBox->AddItem(CID_MonsterBurned, g_pTheDB->GetMessageText(MID_MonsterBurned));
	this->pEventListBox->AddItem(CID_MonsterDiedFromStab, g_pTheDB->GetMessageText(MID_MonsterStabbed));
	this->pEventListBox->AddItem(CID_MonsterPieceStabbed, g_pTheDB->GetMessageText(MID_MonsterPieceStabbed));
	this->pEventListBox->AddItem(CID_MudBabyFormed, g_pTheDB->GetMessageText(MID_MudBabyFormed));
	this->pEventListBox->AddItem(CID_MudGrew, g_pTheDB->GetMessageText(MID_MudGrew));
	this->pEventListBox->AddItem(CID_NPCKilled, g_pTheDB->GetMessageText(MID_NPCKilled));
	this->pEventListBox->AddItem(CID_ObjectBuilt, g_pTheDB->GetMessageText(MID_ObjectBuilt));
	this->pEventListBox->AddItem(CID_ObjectFell, g_pTheDB->GetMessageText(MID_ObjectFell));
	this->pEventListBox->AddItem(CID_OrbActivatedByPlayer, g_pTheDB->GetMessageText(MID_PlayerHitsOrb));
	this->pEventListBox->AddItem(CID_OrbActivated, g_pTheDB->GetMessageText(MID_ItemHitsOrb));
	this->pEventListBox->AddItem(CID_OrbActivatedByDouble, g_pTheDB->GetMessageText(MID_MonsterHitsOrb));
	this->pEventListBox->AddItem(CID_OrbDamaged, g_pTheDB->GetMessageText(MID_OrbDamaged));
	this->pEventListBox->AddItem(CID_FegundoToAsh, g_pTheDB->GetMessageText(MID_PhoenixToAsh));
	this->pEventListBox->AddItem(CID_PlayerFrozen, g_pTheDB->GetMessageText(MID_PlayerFrozen));
	this->pEventListBox->AddItem(CID_PressurePlate, g_pTheDB->GetMessageText(MID_PressurePlateActivated));
	this->pEventListBox->AddItem(CID_PressurePlateReleased, g_pTheDB->GetMessageText(MID_PressurePlateReleased));
	this->pEventListBox->AddItem(CID_Scared, g_pTheDB->GetMessageText(MID_Scared));
	this->pEventListBox->AddItem(CID_SlayerEntered, g_pTheDB->GetMessageText(MID_SlayerEntered));
	this->pEventListBox->AddItem(CID_SnakeDiedFromTruncation, g_pTheDB->GetMessageText(MID_SnakeDiedFromTruncation));
	this->pEventListBox->AddItem(CID_Splash, g_pTheDB->GetMessageText(MID_Splash));
	this->pEventListBox->AddItem(CID_SwingSword, g_pTheDB->GetMessageText(MID_PlayerSwingsSword));
	this->pEventListBox->AddItem(CID_TarBabyFormed, g_pTheDB->GetMessageText(MID_TarBabyFormed));
	this->pEventListBox->AddItem(CID_TarstuffDestroyed, g_pTheDB->GetMessageText(MID_TarDestroyed));
	this->pEventListBox->AddItem(CID_TarGrew, g_pTheDB->GetMessageText(MID_TarGrew));
	this->pEventListBox->AddItem(CID_TokenToggled, g_pTheDB->GetMessageText(MID_TokenToggled));
	this->pEventListBox->AddItem(CID_TrapDoorRemoved, g_pTheDB->GetMessageText(MID_TrapDoorRemoved));
	this->pEventListBox->AddItem(CID_Tunnel, g_pTheDB->GetMessageText(MID_Tunnel));
	this->pEventListBox->AddItem(CID_WispOnPlayer, g_pTheDB->GetMessageText(MID_WispOnPlayer));
	this->pEventListBox->AddItem(CID_WubbaStabbed, g_pTheDB->GetMessageText(MID_WubbaStabbed));
	this->pEventListBox->SelectLine(0);
}

//*****************************************************************************
void CCharacterDialogWidget::PopulateGraphicListBox(CListBoxWidget *pListBox)
//Add all monsters that character can appear as to graphics list.
{
	ASSERT(pListBox);

	pListBox->AddItem(M_ROACH, g_pTheDB->GetMessageText(MID_Roach));
	pListBox->AddItem(M_QROACH, g_pTheDB->GetMessageText(MID_RoachQueen));
	pListBox->AddItem(M_GOBLIN, g_pTheDB->GetMessageText(MID_Goblin));
	pListBox->AddItem(M_WWING, g_pTheDB->GetMessageText(MID_Wraithwing));
	pListBox->AddItem(M_EYE, g_pTheDB->GetMessageText(MID_EvilEye));
	pListBox->AddItem(M_TARBABY, g_pTheDB->GetMessageText(MID_TarBaby));
	pListBox->AddItem(M_BRAIN, g_pTheDB->GetMessageText(MID_Brain));
	pListBox->AddItem(M_MIMIC, g_pTheDB->GetMessageText(MID_Mimic));
	pListBox->AddItem(M_SPIDER, g_pTheDB->GetMessageText(MID_Spider));

	pListBox->AddItem(M_ROCKGOLEM, g_pTheDB->GetMessageText(MID_StoneGolem));
	pListBox->AddItem(M_WUBBA, g_pTheDB->GetMessageText(MID_Wubba));
	pListBox->AddItem(M_SEEP, g_pTheDB->GetMessageText(MID_Ghost));
	pListBox->AddItem(M_HALPH, g_pTheDB->GetMessageText(MID_Halph));
	pListBox->AddItem(M_HALPH2, g_pTheDB->GetMessageText(MID_Halph2));
	pListBox->AddItem(M_SLAYER, g_pTheDB->GetMessageText(MID_Slayer));
	pListBox->AddItem(M_SLAYER2, g_pTheDB->GetMessageText(MID_Slayer2));
	pListBox->AddItem(M_GUARD, g_pTheDB->GetMessageText(MID_Guard));
	pListBox->AddItem(M_MUDBABY, g_pTheDB->GetMessageText(MID_MudBaby));
	pListBox->AddItem(M_CLONE, g_pTheDB->GetMessageText(MID_Clone));
	pListBox->AddItem(M_DECOY, g_pTheDB->GetMessageText(MID_Decoy));
	pListBox->AddItem(M_STALWART, g_pTheDB->GetMessageText(MID_Stalwart));
	pListBox->AddItem(M_STALWART2, g_pTheDB->GetMessageText(MID_Stalwart2));

	pListBox->AddItem(M_CITIZEN, g_pTheDB->GetMessageText(MID_Citizen));
	pListBox->AddItem(M_GELBABY, g_pTheDB->GetMessageText(MID_GelBaby));
	pListBox->AddItem(M_WATERSKIPPER, g_pTheDB->GetMessageText(MID_Ant));
	pListBox->AddItem(M_SKIPPERNEST, g_pTheDB->GetMessageText(MID_AntHill));
	pListBox->AddItem(M_FEGUNDO, g_pTheDB->GetMessageText(MID_Phoenix));
	pListBox->AddItem(M_AUMTLICH, g_pTheDB->GetMessageText(MID_Zombie));

	//Character pseudo monster types.
	pListBox->AddItem(M_CITIZEN1, g_pTheDB->GetMessageText(MID_Citizen1));
	pListBox->AddItem(M_CITIZEN2, g_pTheDB->GetMessageText(MID_Citizen2));
	pListBox->AddItem(M_CITIZEN3, g_pTheDB->GetMessageText(MID_Citizen3));
	pListBox->AddItem(M_CITIZEN4, g_pTheDB->GetMessageText(MID_Citizen4));
	pListBox->AddItem(M_GOBLINKING, g_pTheDB->GetMessageText(MID_GoblinKing));
	pListBox->AddItem(M_MUDCOORDINATOR, g_pTheDB->GetMessageText(MID_MudCoordinator));
	pListBox->AddItem(M_TARTECHNICIAN, g_pTheDB->GetMessageText(MID_TarTechnician));
	pListBox->AddItem(M_NEGOTIATOR, g_pTheDB->GetMessageText(MID_Negotiator));
	pListBox->AddItem(M_EYE_ACTIVE, g_pTheDB->GetMessageText(MID_EvilEyeActive));
	pListBox->AddItem(M_BEETHRO, g_pTheDB->GetMessageText(MID_Beethro));
	pListBox->AddItem(M_GUNTHRO, g_pTheDB->GetMessageText(MID_Gunthro));
	pListBox->AddItem(M_NONE, g_pTheDB->GetMessageText(MID_None));
}

//*****************************************************************************
void CCharacterDialogWidget::PopulateCommandDescriptions(
//Fills the given command list box with human-understandable descriptions of
//the specified command script.
//
//Params:
	CListBoxWidget *pCommandList, COMMANDPTR_VECTOR& commands)
{
	pCommandList->Clear();
	for (UINT wIndex=0; wIndex<commands.size(); ++wIndex)
	{
		CCharacterCommand *pCommand = commands[wIndex];
		pCommandList->AddItemPointer(pCommand,
				GetCommandDesc(commands, pCommand).c_str());
	}
	if (commands.size())
		pCommandList->SelectLine(0);
}

//*****************************************************************************
void CCharacterDialogWidget::PopulateCharacterList(CListBoxWidget *pListBox)
//Add all custom hold characters to list.
{
	ASSERT(pListBox);

	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));
	ASSERT(pEditRoomScreen);
	ASSERT(pEditRoomScreen->pHold);
	for (vector<HoldCharacter>::const_iterator ch = pEditRoomScreen->pHold->characters.begin();
			ch != pEditRoomScreen->pHold->characters.end(); ++ch)
		pListBox->AddItem(ch->dwCharID, ch->charNameText.c_str());
}

//*****************************************************************************
void CCharacterDialogWidget::PopulateGotoLabelList(const COMMANDPTR_VECTOR& commands)
//Compile goto label texts.  Determine next label ID.
{
	this->pGotoLabelListBox->Clear();
	this->wIncrementedLabel = 0;

	for (UINT wIndex=0; wIndex<commands.size(); ++wIndex)
	{
		CCharacterCommand *pCommand = commands[wIndex];
		if (pCommand->command == CCharacterCommand::CC_Label)
		{
			this->pGotoLabelListBox->AddItem(pCommand->x, pCommand->label.c_str());
			if (!this->pGotoLabelListBox->ItemIsSelected())
				this->pGotoLabelListBox->SelectLine(0);
			if (pCommand->x > this->wIncrementedLabel)
				this->wIncrementedLabel = pCommand->x;
		}
	}
}

//*****************************************************************************
void CCharacterDialogWidget::PopulateMainGraphicList()
//Refresh the list of available characters.
{
	ASSERT(this->pGraphicListBox);

	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));
	ASSERT(pEditRoomScreen->pHold);

	this->pGraphicListBox->Clear();
	PopulateGraphicListBox(this->pGraphicListBox);
	PopulateCharacterList(this->pGraphicListBox);
	//Select Citizen 1 if a custom char in use was deleted
	this->pGraphicListBox->SelectItem(this->pCharacter ?
			this->pCharacter->wLogicalIdentity < CUSTOM_CHARACTER_FIRST ||
			this->pCharacter->wLogicalIdentity == M_NONE ||
			pEditRoomScreen->pHold->GetCharacter(this->pCharacter->wLogicalIdentity) != NULL ?
				this->pCharacter->wLogicalIdentity : M_CITIZEN1 : M_NONE);

	ASSERT(this->pPlayerGraphicListBox);
	this->pPlayerGraphicListBox->Clear();
	PopulateGraphicListBox(this->pPlayerGraphicListBox);
	PopulateCharacterList(this->pPlayerGraphicListBox);
	this->pPlayerGraphicListBox->SelectItem(M_BEETHRO);

	ASSERT(this->pGlobalScriptListBox);
	this->pGlobalScriptListBox->Clear();
	PopulateCharacterList(this->pGlobalScriptListBox);
	this->pGlobalScriptListBox->SelectLine(0);

	ASSERT(this->pCharListBox);
	this->pCharListBox->Clear();
	PopulateCharacterList(this->pCharListBox);
	this->pCharListBox->SelectLine(0);

	ASSERT(this->pSpeakerListBox);
	PopulateSpeakerList(this->pSpeakerListBox);
}

//*****************************************************************************
void CCharacterDialogWidget::PopulateSpeakerList(CListBoxWidget *pListBox)
//Refresh the list of available speakers.
{
	pListBox->Clear();
	pListBox->SortAlphabetically(true);

	pListBox->AddItem(Speaker_Beethro, g_pTheDB->GetMessageText(MID_Beethro));
	pListBox->AddItem(Speaker_Gunthro, g_pTheDB->GetMessageText(MID_Gunthro));
	pListBox->AddItem(Speaker_SerpentG, g_pTheDB->GetMessageText(MID_GreenSerpent));
	pListBox->AddItem(Speaker_Aumtlich, g_pTheDB->GetMessageText(MID_Zombie));
	pListBox->AddItem(Speaker_Brain, g_pTheDB->GetMessageText(MID_Brain));
	pListBox->AddItem(Speaker_Citizen, g_pTheDB->GetMessageText(MID_Citizen));
	pListBox->AddItem(Speaker_Citizen1, g_pTheDB->GetMessageText(MID_Citizen1));
	pListBox->AddItem(Speaker_Citizen2, g_pTheDB->GetMessageText(MID_Citizen2));
	pListBox->AddItem(Speaker_Citizen3, g_pTheDB->GetMessageText(MID_Citizen3));
	pListBox->AddItem(Speaker_Citizen4, g_pTheDB->GetMessageText(MID_Citizen4));
	pListBox->AddItem(Speaker_Clone, g_pTheDB->GetMessageText(MID_Clone));
	pListBox->AddItem(Speaker_Decoy, g_pTheDB->GetMessageText(MID_Decoy));
	pListBox->AddItem(Speaker_Eye, g_pTheDB->GetMessageText(MID_EvilEye));
	pListBox->AddItem(Speaker_EyeActive, g_pTheDB->GetMessageText(MID_EvilEyeActive));
	pListBox->AddItem(Speaker_Fegundo, g_pTheDB->GetMessageText(MID_Phoenix));
	pListBox->AddItem(Speaker_FegundoAshes, g_pTheDB->GetMessageText(MID_PhoenixAshes));
	pListBox->AddItem(Speaker_GelBaby, g_pTheDB->GetMessageText(MID_GelBaby));
	pListBox->AddItem(Speaker_GelMother, g_pTheDB->GetMessageText(MID_GelMother));
	pListBox->AddItem(Speaker_Goblin, g_pTheDB->GetMessageText(MID_Goblin));
	pListBox->AddItem(Speaker_GoblinKing, g_pTheDB->GetMessageText(MID_GoblinKing));
	pListBox->AddItem(Speaker_Guard, g_pTheDB->GetMessageText(MID_Guard));
	pListBox->AddItem(Speaker_Halph, g_pTheDB->GetMessageText(MID_Halph));
	pListBox->AddItem(Speaker_Halph2, g_pTheDB->GetMessageText(MID_Halph2));
	pListBox->AddItem(Speaker_Mimic, g_pTheDB->GetMessageText(MID_Mimic));
	pListBox->AddItem(Speaker_MudBaby, g_pTheDB->GetMessageText(MID_MudBaby));
	pListBox->AddItem(Speaker_MudCoordinator, g_pTheDB->GetMessageText(MID_MudCoordinator));
	pListBox->AddItem(Speaker_MudMother, g_pTheDB->GetMessageText(MID_MudMother));
	pListBox->AddItem(Speaker_Negotiator, g_pTheDB->GetMessageText(MID_Negotiator));
	pListBox->AddItem(Speaker_Roach, g_pTheDB->GetMessageText(MID_Roach));
	pListBox->AddItem(Speaker_RoachEgg, g_pTheDB->GetMessageText(MID_RoachEgg));
	pListBox->AddItem(Speaker_QRoach, g_pTheDB->GetMessageText(MID_RoachQueen));
	pListBox->AddItem(Speaker_SerpentB, g_pTheDB->GetMessageText(MID_BlueSerpent));
	pListBox->AddItem(Speaker_RockGolem, g_pTheDB->GetMessageText(MID_StoneGolem));
	pListBox->AddItem(Speaker_RockGiant, g_pTheDB->GetMessageText(MID_Splitter));
	pListBox->AddItem(Speaker_Seep, g_pTheDB->GetMessageText(MID_Ghost));
	pListBox->AddItem(Speaker_Serpent, g_pTheDB->GetMessageText(MID_Serpent));
	pListBox->AddItem(Speaker_Slayer, g_pTheDB->GetMessageText(MID_Slayer));
	pListBox->AddItem(Speaker_Slayer2, g_pTheDB->GetMessageText(MID_Slayer2));
	pListBox->AddItem(Speaker_Spider, g_pTheDB->GetMessageText(MID_Spider));
	pListBox->AddItem(Speaker_Stalwart, g_pTheDB->GetMessageText(MID_Stalwart));
	pListBox->AddItem(Speaker_Stalwart2, g_pTheDB->GetMessageText(MID_Stalwart2));
	pListBox->AddItem(Speaker_TarBaby, g_pTheDB->GetMessageText(MID_TarBaby));
	pListBox->AddItem(Speaker_TarMother, g_pTheDB->GetMessageText(MID_TarMother));
	pListBox->AddItem(Speaker_TarTechnician, g_pTheDB->GetMessageText(MID_TarTechnician));
	pListBox->AddItem(Speaker_WaterSkipper, g_pTheDB->GetMessageText(MID_Ant));
	pListBox->AddItem(Speaker_WaterSkipperNest, g_pTheDB->GetMessageText(MID_AntHill));
	pListBox->AddItem(Speaker_WWing, g_pTheDB->GetMessageText(MID_Wraithwing));
	pListBox->AddItem(Speaker_Wubba, g_pTheDB->GetMessageText(MID_Wubba));

	PopulateCharacterList(pListBox);

	pListBox->SortAlphabetically(false);

	pListBox->AddItem(Speaker_None, g_pTheDB->GetMessageText(MID_None));
	pListBox->AddItem(Speaker_Custom, g_pTheDB->GetMessageText(MID_Custom));
	pListBox->AddItem(Speaker_Player, g_pTheDB->GetMessageText(MID_Player));
	pListBox->AddItem(Speaker_Self, g_pTheDB->GetMessageText(MID_Self));

	pListBox->SelectItem(Speaker_Self);
}

//*****************************************************************************
void CCharacterDialogWidget::PopulateVarList()
//Compile active hold's current var list.
{
	this->pVarListBox->Clear();
	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));
	ASSERT(pEditRoomScreen);
	ASSERT(pEditRoomScreen->pHold);
	for (vector<HoldVar>::const_iterator var = pEditRoomScreen->pHold->vars.begin();
			var != pEditRoomScreen->pHold->vars.end(); ++var)
		this->pVarListBox->AddItem(var->dwVarID, var->varNameText.c_str());

	//Add hard-coded global vars to the end of the list.
	this->pVarListBox->SortAlphabetically(false);

#ifdef DROD_TSS
	this->pVarListBox->AddItem(ScriptVars::P_MONSTER_COLOR, g_pTheDB->GetMessageText(MID_VarMonsterColor));
	this->pVarListBox->AddItem(ScriptVars::P_MONSTER_SWORD, g_pTheDB->GetMessageText(MID_VarMonsterSword));
#endif
	this->pVarListBox->AddItem(ScriptVars::P_MONSTER_X, g_pTheDB->GetMessageText(MID_VarMonsterX));
	this->pVarListBox->AddItem(ScriptVars::P_MONSTER_Y, g_pTheDB->GetMessageText(MID_VarMonsterY));
	this->pVarListBox->AddItem(ScriptVars::P_MONSTER_O, g_pTheDB->GetMessageText(MID_VarMonsterO));
	this->pVarListBox->AddItem(ScriptVars::P_SCRIPT_X, g_pTheDB->GetMessageText(MID_VarMonsterParamX));
	this->pVarListBox->AddItem(ScriptVars::P_SCRIPT_Y, g_pTheDB->GetMessageText(MID_VarMonsterParamY));
	this->pVarListBox->AddItem(ScriptVars::P_SCRIPT_W, g_pTheDB->GetMessageText(MID_VarMonsterParamW));
	this->pVarListBox->AddItem(ScriptVars::P_SCRIPT_H, g_pTheDB->GetMessageText(MID_VarMonsterParamH));
	this->pVarListBox->AddItem(ScriptVars::P_SCRIPT_F, g_pTheDB->GetMessageText(MID_VarMonsterParamF));

#ifdef DROD_TSS
	this->pVarListBox->AddItem(ScriptVars::P_SWORD, g_pTheDB->GetMessageText(MID_VarSword));
#endif
	this->pVarListBox->AddItem(ScriptVars::P_PLAYER_X, g_pTheDB->GetMessageText(MID_VarX));
	this->pVarListBox->AddItem(ScriptVars::P_PLAYER_Y, g_pTheDB->GetMessageText(MID_VarY));
	this->pVarListBox->AddItem(ScriptVars::P_PLAYER_O, g_pTheDB->GetMessageText(MID_VarO));

	this->pVarListBox->AddItem(ScriptVars::P_TOTALMOVES, g_pTheDB->GetMessageText(MID_TotalMoves));
	this->pVarListBox->AddItem(ScriptVars::P_TOTALTIME, g_pTheDB->GetMessageText(MID_TotalTime));

	this->pVarListBox->SortAlphabetically(true);
}

//*****************************************************************************
void CCharacterDialogWidget::prepareForwardReferences(const COMMANDPTR_VECTOR& newCommands)
//When cutting commands from the command script, replace label IDs with text
//that can be resolved when the command is pasted back into a script.
{
	for (COMMANDPTR_VECTOR::const_iterator cIter = newCommands.begin();
			cIter != newCommands.end(); ++cIter)
	{
		CCharacterCommand& c = *(*cIter);
		switch (c.command)
		{
			case CCharacterCommand::CC_GoTo:
			case CCharacterCommand::CC_AnswerOption:
				c.label = this->pGotoLabelListBox->GetTextForKey(c.x);
				if (!c.label.empty()) //if label ID is valid, replace ID with this text
					c.x = 0;
			break;
			default: break;
		}
	}
}

//*****************************************************************************
void CCharacterDialogWidget::resolveForwardReferences(const COMMANDPTR_VECTOR& newCommands)
//Resolve forward references for commands just pasted into the current command script.
//For instance, the label text in a "goto <label>" command will be hooked up to
//the label ID of current "Label" command with this text.
{
	for (COMMANDPTR_VECTOR::const_iterator cIter = newCommands.begin();
			cIter != newCommands.end(); ++cIter)
	{
		CCharacterCommand& c = *(*cIter);
		switch (c.command)
		{
			case CCharacterCommand::CC_GoTo:
			case CCharacterCommand::CC_AnswerOption:
			{
				UINT tempIndex = 0;
				bool bFound;
				const UINT labelID = findTextMatch(this->pGotoLabelListBox, c.label.c_str(), tempIndex, bFound);
				c.x = (bFound ? labelID : 0);
				c.label.resize(0);
			}
			break;
			default: break;
		}
	}
}

//*****************************************************************************
void CCharacterDialogWidget::QueryRect()
//Get rectangular area info from user.
{
	//Get location information through CEditRoomScreen.
	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));
	ASSERT(pEditRoomScreen);
	pEditRoomScreen->SetState(ES_GETRECT, true);
	for (UINT wY = this->pCommand->y; wY <= this->pCommand->y + this->pCommand->h; ++wY)
		for (UINT wX = this->pCommand->x; wX <= this->pCommand->x + this->pCommand->w; ++wX)
			pEditRoomScreen->pRoomWidget->AddShadeEffect(wX, wY, PaleRed);
	Deactivate();
}

//*****************************************************************************
void CCharacterDialogWidget::QueryXY()
//Get (x,y) info from user.
{
	//Get location information through CEditRoomScreen.
	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));
	ASSERT(pEditRoomScreen);
	pEditRoomScreen->SetState(ES_GETSQUARE, true);
	pEditRoomScreen->pRoomWidget->AddShadeEffect(
			this->pCommand->x, this->pCommand->y, PaleRed);
	Deactivate();
}

//*****************************************************************************
void CCharacterDialogWidget::SetDefaultScriptWidgetStates()
//Update widget states on main dialog.
{
	CWidget *pButton = GetWidget(TAG_DELETECOMMAND2);
	ASSERT(pButton);
	const bool bEnable = this->pDefaultScriptCommandsListBox->GetItemCount() > 0;
	if (!bEnable && GetSelectedWidget() == pButton)
		SelectNextWidget();
	pButton->Enable(bEnable);
	pButton->RequestPaint();
}

//*****************************************************************************
void CCharacterDialogWidget::SetWidgetStates()
//Update widget states on main dialog.
{
	CWidget *pButton = GetWidget(TAG_DELETECOMMAND);
	ASSERT(pButton);
	const bool bEnable = this->pCommandsListBox->GetItemCount() > 0;
	if (!bEnable && GetSelectedWidget() == pButton)
		SelectNextWidget();
	pButton->Enable(bEnable);
	pButton->RequestPaint();
}

//*****************************************************************************
void CCharacterDialogWidget::SetActionWidgetStates()
//Enable those widgets that are applicable to currently selected command.
{
	//Code is structured in this way to facilitate quick addition of
	//additional action parameters.
	static const UINT NUM_WIDGETS = 33;
	static const UINT widgetTag[NUM_WIDGETS] = {
		TAG_WAIT, TAG_EVENTLISTBOX, TAG_DELAY, TAG_SPEECHTEXT,
		TAG_SPEAKERLISTBOX, TAG_MOODLISTBOX, TAG_ADDSOUND, TAG_TESTSOUND, TAG_DIRECTIONLISTBOX,
		TAG_ONOFFLISTBOX, TAG_OPENCLOSELISTBOX, TAG_GOTOLABELTEXT,
		TAG_GOTOLABELLISTBOX, TAG_MUSICLISTBOX, TAG_ONOFFLISTBOX2,
		TAG_WAITFLAGSLISTBOX, TAG_VARADD, TAG_VARREMOVE,
		TAG_VARLIST, TAG_VAROPLIST, TAG_VARCOMPLIST, TAG_VARVALUE,
		TAG_GRAPHICLISTBOX2, TAG_MOVERELX, TAG_MOVERELY, TAG_IMPERATIVELISTBOX,
		TAG_ITEMLISTBOX, TAG_WATERTRAVERSALLISTBOX, TAG_GLOBALSCRIPTLISTBOX,
		TAG_DIRECTIONLISTBOX2,
		TAG_VISUALEFFECTS_LISTBOX, TAG_DIRECTIONLISTBOX3, TAG_ONOFFLISTBOX3
	};

	static const bool NO_WIDGETS[NUM_WIDGETS] =   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool WAIT[NUM_WIDGETS] =         {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool CUEEVENT[NUM_WIDGETS] =     {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool SPEECH[NUM_WIDGETS] =       {0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool ORIENTATION[NUM_WIDGETS] =  {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
 	static const bool ONOFF[NUM_WIDGETS] =        {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool OPENCLOSE[NUM_WIDGETS] =    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool GOTO[NUM_WIDGETS] =         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool GOTOLIST[NUM_WIDGETS] =     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool MUSIC[NUM_WIDGETS] =        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
 	static const bool MOVE[NUM_WIDGETS] =         {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
 	static const bool WAITFLAGS[NUM_WIDGETS] =    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
 	static const bool VARSET[NUM_WIDGETS] =       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
 	static const bool VARGET[NUM_WIDGETS] =       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool GRAPHIC[NUM_WIDGETS] =      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
 	static const bool MOVEREL[NUM_WIDGETS] =      {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
 	static const bool IMPERATIVE[NUM_WIDGETS] =   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
	static const bool ANSWER[NUM_WIDGETS] =       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool ITEMS[NUM_WIDGETS] =        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0};
 	static const bool XY[NUM_WIDGETS] =           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
 	static const bool WATERTRAVEL[NUM_WIDGETS] =  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0};
	static const bool GLOBALSCRIPT[NUM_WIDGETS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0};
	static const bool NEWENTITY[NUM_WIDGETS] =    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0};
	static const bool EFFECT[NUM_WIDGETS] =       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1};

	static const bool* activeWidgets[CCharacterCommand::CC_Count] = {
		NO_WIDGETS,
		NO_WIDGETS,
		MOVE,
		WAIT,
		CUEEVENT,
		WAITFLAGS,
		SPEECH,
		IMPERATIVE,
		NO_WIDGETS,
		NO_WIDGETS,
		ORIENTATION,
		WAITFLAGS,
		OPENCLOSE,
		GOTO,
		GOTOLIST,
		0,
		0,
		0,
		WAIT,
		NO_WIDGETS,
		ORIENTATION,
		NO_WIDGETS,
		NO_WIDGETS,
		0,
		0,
		0,
		0,
		ONOFF,
		GOTO,
		MUSIC,
		NO_WIDGETS,
		NO_WIDGETS,
		NO_WIDGETS,
		NO_WIDGETS,
		ONOFF,
		VARSET,
		VARGET,
		GRAPHIC,
		WAIT,
		MOVEREL,
		ONOFF,
		ANSWER,
		ITEMS,
		ONOFF,
		ONOFF,
		NO_WIDGETS,
		XY,
		ORIENTATION,
		NO_WIDGETS,
		GRAPHIC,
		WATERTRAVEL,
		GLOBALSCRIPT,
		ITEMS,
		NEWENTITY,
		EFFECT
	};

	static const UINT NUM_LABELS = 23;
	static const UINT labelTag[NUM_LABELS] = {
		TAG_EVENTLABEL, TAG_WAITLABEL, TAG_DELAYLABEL, TAG_SPEAKERLABEL,
		TAG_MOODLABEL, TAG_TEXTLABEL, TAG_DIRECTIONLABEL, TAG_SOUNDNAME_LABEL,
		TAG_GOTOLABEL, TAG_DISPLAYSPEECHLABEL, TAG_MUSICLABEL, TAG_NOTURNING,
		TAG_SINGLESTEP, TAG_VARNAMETEXTLABEL, TAG_VARVALUELABEL, TAG_CUTSCENELABEL,
		TAG_MOVERELXLABEL, TAG_MOVERELYLABEL, TAG_LOOPSOUND, TAG_WAITABSLABEL,
		TAG_SKIPENTRANCELABEL, TAG_DIRECTIONLABEL2, TAG_SOUNDEFFECTLABEL
	};

	static const bool NO_LABELS[NUM_LABELS] =      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool CUEEVENT_L[NUM_LABELS] =     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool WAIT_L[NUM_LABELS] =         {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool WAITABS_L[NUM_LABELS] =      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0};
	static const bool SPEECH_L[NUM_LABELS] =       {0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool TEXT_L[NUM_LABELS] =         {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool ORIENTATION_L[NUM_LABELS] =  {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool GOTO_L[NUM_LABELS] =         {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool DISPSPEECH_L[NUM_LABELS] =   {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool MUSIC_L[NUM_LABELS] =        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool MOVE_L[NUM_LABELS] =         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool VARSET_L[NUM_LABELS] =       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool VARGET_L[NUM_LABELS] =       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
	static const bool CUTSCENE_L[NUM_LABELS] =     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
	static const bool MOVEREL_L[NUM_LABELS] =      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0};
	static const bool LOOPSOUND_L[NUM_LABELS] =    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0};
	static const bool XY_L[NUM_LABELS] =           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0};
	static const bool SKIPENTRANCE_L[NUM_LABELS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0};
	static const bool NEWENTITY_L[NUM_LABELS] =    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0};
	static const bool EFFECT_L[NUM_LABELS] =       {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};

	static const bool* activeLabels[CCharacterCommand::CC_Count] = {
		NO_LABELS,
		NO_LABELS,
		MOVE_L,
		WAIT_L,
		CUEEVENT_L,
		NO_LABELS,
		SPEECH_L,
		NO_LABELS,
		NO_LABELS,
		NO_LABELS,
		ORIENTATION_L,
		NO_LABELS,
		NO_LABELS,
		GOTO_L,
		NO_LABELS,
		0,
		0,
		0,
		WAITABS_L,
		NO_LABELS,
		ORIENTATION_L,
		NO_LABELS,
		NO_LABELS,
		0,
		0,
		0,
		0,
		DISPSPEECH_L,
		TEXT_L,
		MUSIC_L,
		NO_LABELS,
		NO_LABELS,
		NO_LABELS,
		NO_LABELS,
		SKIPENTRANCE_L,
		VARSET_L,
		VARGET_L,
		NO_LABELS,
		CUTSCENE_L,
		MOVEREL_L,
		NO_LABELS,
		TEXT_L,
		NO_LABELS,
		LOOPSOUND_L,
		LOOPSOUND_L,
		NO_LABELS,
		XY_L,
		ORIENTATION_L,
		NO_LABELS,
		NO_LABELS,
		NO_LABELS,
		NO_LABELS,
		NO_LABELS,
		NEWENTITY_L,
		EFFECT_L
	};
	ASSERT(this->pActionListBox->GetSelectedItem() < CCharacterCommand::CC_Count);

	//Set important default settings.
	if (!this->bRetainFields)
	{
		CListBoxWidget *pFlags = DYN_CAST(CListBoxWidget*, CWidget*,
				this->pAddCommandDialog->GetWidget(TAG_WAITFLAGSLISTBOX));
		pFlags->DeselectAll();
		CTextBoxWidget *pDialogue = DYN_CAST(CTextBoxWidget*, CWidget*,
				this->pAddCommandDialog->GetWidget(TAG_SPEECHTEXT));
		pDialogue->SetText(wszEmpty);
		pDialogue = DYN_CAST(CTextBoxWidget*, CWidget*,
				this->pAddCommandDialog->GetWidget(TAG_GOTOLABELTEXT));
		pDialogue->SetText(wszEmpty);
		CListBoxWidget *pMood = DYN_CAST(CListBoxWidget*, CWidget*,
				this->pAddCommandDialog->GetWidget(TAG_MOODLISTBOX));
		pMood->SelectItem(Mood_Normal);
		CTextBoxWidget *pDuration = DYN_CAST(CTextBoxWidget*, CWidget*,
				this->pAddCommandDialog->GetWidget(TAG_DELAY));
		pDuration->SetText(wszZero);
	}

	const UINT dwSelectedItem = this->pActionListBox->GetSelectedItem();
	UINT wIndex;
	for (wIndex=0; wIndex<NUM_WIDGETS; ++wIndex)
	{
		CWidget *pWidget = this->pAddCommandDialog->GetWidget(widgetTag[wIndex]);
		ASSERT(pWidget);
		pWidget->Show(activeWidgets[dwSelectedItem][wIndex]);
	}
	for (wIndex=0; wIndex<NUM_LABELS; ++wIndex)
	{
		CWidget *pWidget = this->pAddCommandDialog->GetWidget(labelTag[wIndex]);
		ASSERT(pWidget);
		pWidget->Show(activeLabels[dwSelectedItem][wIndex]);
	}

	//Set name of loaded sound file, if any.
	CButtonWidget *pButton = DYN_CAST(CButtonWidget*, CWidget*,
			this->pAddCommandDialog->GetWidget(TAG_ADDSOUND));
	pButton->SetCaption(g_pTheDB->GetMessageText(this->pSound ? MID_RemoveSound : MID_AddSound));

	pButton = DYN_CAST(CButtonWidget*, CWidget*,
			this->pAddCommandDialog->GetWidget(TAG_TESTSOUND));
	pButton->Enable(this->pSound != NULL);

	CLabelWidget *pSoundName = DYN_CAST(CLabelWidget*, CWidget*,
			this->pAddCommandDialog->GetWidget(TAG_SOUNDNAME_LABEL));
	ASSERT(pSoundName);
	pSoundName->SetText(this->pSound ? this->pSound->DataNameText.c_str() : wszEmpty);
}

//*****************************************************************************
void CCharacterDialogWidget::SelectCharacter()
//Sets custom character dialog widgets to show the selected character with the specified ID.
{
	SetCharacterWidgetStates();
	this->pAddCharacterDialog->Paint();
}

//*****************************************************************************
UINT CCharacterDialogWidget::SelectMediaID(
//UI for importing, deleting and selecting media data belonging to this hold.
//
//Params:
	const UINT dwDefault, //default selected value
	const CEntranceSelectDialogWidget::DATATYPE eType) //media type
{
	MESSAGE_ID midPrompt = 0;
	switch (eType)
	{
		case CEntranceSelectDialogWidget::Sounds:	midPrompt = MID_SoundSelectPrompt; break;
		case CEntranceSelectDialogWidget::Videos: midPrompt = MID_VideoSelectPrompt; break;
		default: ASSERT(!"UI for this media type not implemented"); return 0;
	}
	ASSERT(midPrompt);

	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));
	ASSERT(pEditRoomScreen->pHold);

	UINT dwVal;
	CEntranceSelectDialogWidget::BUTTONTYPE eButton;
	do {
		dwVal = dwDefault;
		eButton = pEditRoomScreen->SelectListID(
				pEditRoomScreen->pEntranceBox, pEditRoomScreen->pHold,
				dwVal, midPrompt, eType);

		if (eButton == CEntranceSelectDialogWidget::Delete)
		{
			//Remove this media object from the database and make another selection.
			//It's okay if other references to this object remain set to this old record ID.
			//They will robustly default to do nothing.
			pEditRoomScreen->pHold->MarkDataForDeletion(dwVal);
		}
	} while (eButton == CEntranceSelectDialogWidget::Delete);

	const bool bSelected = eButton == CEntranceSelectDialogWidget::OK;
	if (bSelected && !dwVal)
	{
		//Import media from disk into this hold.
		switch (eType)
		{
			case CEntranceSelectDialogWidget::Sounds:
				dwVal = pEditRoomScreen->ImportHoldSound();
			break;
			case CEntranceSelectDialogWidget::Videos:
				dwVal = pEditRoomScreen->ImportHoldVideo();
			break;
			default: break;
		}
	}
	return bSelected ? dwVal : 0;
}

//*****************************************************************************
void CCharacterDialogWidget::SetCustomImage()
//UI for setting the current custom character's avatar or tile set.
{
	HoldCharacter *pChar = GetCustomCharacter();
	if (!pChar)
		return;

	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));
	ASSERT(pEditRoomScreen->pHold);

	//Image management.
SelectImage:
	UINT dwDataID;
	CEntranceSelectDialogWidget::BUTTONTYPE eButton;
	do {
		dwDataID = pChar->dwDataID_Avatar;
		eButton = pEditRoomScreen->SelectListID(pEditRoomScreen->pEntranceBox,
				pEditRoomScreen->pHold, dwDataID,
				MID_ImageSelectPrompt, CEntranceSelectDialogWidget::Images);
		if (eButton != CEntranceSelectDialogWidget::OK &&
				eButton != CEntranceSelectDialogWidget::Delete)
			return;

		if (eButton == CEntranceSelectDialogWidget::Delete)
		{
			//Remove this image from the database and make another selection.
			pEditRoomScreen->pHold->MarkDataForDeletion(dwDataID);
			pChar->dwDataID_Avatar = 0;
		}
	} while (eButton != CEntranceSelectDialogWidget::OK);

	if (dwDataID)
	{
		//Set to selected image from DB.
		pChar->dwDataID_Avatar = dwDataID;
	} else {
		const UINT dwID = pEditRoomScreen->ImportHoldImage(EXT_PNG | EXT_JPEG);
		if (dwID)
			pChar->dwDataID_Avatar = dwID;
		goto SelectImage;	//return to image management
	}

	SetCharacterWidgetStates();
	this->pAddCharacterDialog->Paint();
}

//*****************************************************************************
void CCharacterDialogWidget::SetCustomTiles()
//UI for setting the current custom character's avatar or tile set.
{
	HoldCharacter *pChar = GetCustomCharacter();
	if (!pChar)
		return;

	CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_EditRoom));
	ASSERT(pEditRoomScreen->pHold);

	//Image management.
SelectImage:
	UINT dwDataID;
	CEntranceSelectDialogWidget::BUTTONTYPE eButton;
	do {
		dwDataID = pChar->dwDataID_Tiles;
		eButton = pEditRoomScreen->SelectListID(pEditRoomScreen->pEntranceBox,
				pEditRoomScreen->pHold, dwDataID,
				MID_ImageSelectPrompt, CEntranceSelectDialogWidget::Images);
		if (eButton != CEntranceSelectDialogWidget::OK &&
				eButton != CEntranceSelectDialogWidget::Delete)
			return;

		if (eButton == CEntranceSelectDialogWidget::Delete)
		{
			//Remove this image from the database and make another selection.
			pEditRoomScreen->pHold->MarkDataForDeletion(dwDataID);
			pChar->dwDataID_Tiles = 0;
		}
	} while (eButton != CEntranceSelectDialogWidget::OK);

	if (dwDataID)
	{
		//Set to selected image from DB.
		pChar->dwDataID_Tiles = dwDataID;

		//If custom tiles are not the right size, warn the user.
		SDL_Surface *pSurface = g_pTheDBM->LoadImageSurface(dwDataID);
		if (pSurface) {
			if ((pSurface->w % CDrodBitmapManager::CX_TILE) != 0  ||
				(pSurface->h % CDrodBitmapManager::CY_TILE) != 0) {
				pEditRoomScreen->ShowOkMessage(MID_NPCCustomTileSizeWarning);
			}
			SDL_FreeSurface(pSurface);
		}
	} else {
		const UINT dwID = pEditRoomScreen->ImportHoldImage(EXT_PNG);
		if (dwID)
			pChar->dwDataID_Tiles = dwID;
		goto SelectImage;	//return to image management
	}

	SetCharacterWidgetStates();
	this->pAddCharacterDialog->Paint();
}

//*****************************************************************************
void CCharacterDialogWidget::SetDefaultAvatar()
//Reverts the current custom character's avatar to the default.
{
	HoldCharacter *pChar = GetCustomCharacter();
	if (!pChar)
		return;

	if (!pChar->dwDataID_Avatar)
		return;

	pChar->dwDataID_Avatar = 0;
	SetCharacterWidgetStates();
	this->pAddCharacterDialog->Paint();
}

//*****************************************************************************
void CCharacterDialogWidget::SetDefaultTiles()
//Reverts the current custom character's tileset to the default.
{
	HoldCharacter *pChar = GetCustomCharacter();
	if (!pChar)
		return;

	if (!pChar->dwDataID_Tiles)
		return;

	pChar->dwDataID_Tiles = 0;
	SetCharacterWidgetStates();
	this->pAddCharacterDialog->Paint();
}

//*****************************************************************************
void CCharacterDialogWidget::SetCustomGraphic()
//Sets the selected custom character's graphic to indicated choice.
{
	HoldCharacter *pChar = GetCustomCharacter();
	if (pChar)
	{
		CListBoxWidget *pCharGraphicList = DYN_CAST(CListBoxWidget*, CWidget*,
				GetWidget(TAG_CHARGRAPHICLISTBOX));
		pChar->wType = pCharGraphicList->GetSelectedItem();
	}

	SetCharacterWidgetStates();
	this->pAddCharacterDialog->Paint();
}

//*****************************************************************************
void CCharacterDialogWidget::SetCharacterWidgetStates()
//Set widgets for custom character dialog.
{
	const bool bCharExists = !this->pCharListBox->IsEmpty();

	CWidget *pWidget = this->pAddCharacterDialog->GetWidget(TAG_ADDCHARACTER);
	pWidget->Enable(!this->pCharNameText->IsEmpty());

	pWidget = this->pAddCharacterDialog->GetWidget(TAG_DELETECHARACTER);
	pWidget->Enable(bCharExists);

	pWidget = this->pAddCharacterDialog->GetWidget(TAG_CUSTOMAVATAR);
	pWidget->Enable(bCharExists);

	pWidget = this->pAddCharacterDialog->GetWidget(TAG_CUSTOMTILES);
	pWidget->Enable(bCharExists);

	pWidget = this->pAddCharacterDialog->GetWidget(TAG_EDITDEFAULTSCRIPT);
	pWidget->Enable(bCharExists);

	CListBoxWidget *pCharGraphicList = DYN_CAST(CListBoxWidget*, CWidget*,
			GetWidget(TAG_CHARGRAPHICLISTBOX));
	CFaceWidget *pFace = DYN_CAST(CFaceWidget*, CWidget*, GetWidget(TAG_AVATARFACE));
	CImageWidget *pTiles = DYN_CAST(CImageWidget*, CWidget*, GetWidget(TAG_TILESIMAGE));

	CWidget *pDefaultAvatar = GetWidget(TAG_DEFAULTAVATAR);
	CWidget *pDefaultTiles = GetWidget(TAG_DEFAULTTILES);

	HoldCharacter *pChar = GetCustomCharacter();
	if (pChar)
	{
		static const UINT CX_TILES = 9 * CDrodBitmapManager::CX_TILE;
		static const UINT CY_TILES = 4 * CDrodBitmapManager::CY_TILE;

		pCharGraphicList->SelectItem(pChar->wType);
		pFace->SetCharacter(getSpeakerType(MONSTERTYPE(pChar->wType)), false);
		pDefaultAvatar->Enable(pChar->dwDataID_Avatar != 0);
		pDefaultTiles->Enable(pChar->dwDataID_Tiles != 0);
		pFace->SetImage(pChar->dwDataID_Avatar);
		pTiles->SetImage(g_pTheDBM->LoadImageSurface(pChar->dwDataID_Tiles));
		if (pTiles->GetW() > CX_TILES) //bounds checking
			pTiles->SetWidth(CX_TILES);
		if (pTiles->GetH() > CY_TILES)
			pTiles->SetHeight(CY_TILES);
	} else {
		pFace->SetCharacter(getSpeakerType(
				MONSTERTYPE(pCharGraphicList->GetSelectedItem())), false);
		pDefaultAvatar->Disable();
		pDefaultTiles->Disable();
	}
}

//*****************************************************************************
void CCharacterDialogWidget::SetCommandParametersFromWidgets(
//Set command parameters according to widget values.
//
//Params:
	CListBoxWidget *pActiveCommandList, COMMANDPTR_VECTOR& commands)
{
	ASSERT(this->pCommand);

	//When editing a label, or changing a label to a different command type.
	bool bRemovedLabel = false;
	CCharacterCommand::CharCommand oldCommandType = CCharacterCommand::CC_Count;
	if (this->bEditingCommand)
	{
		oldCommandType = this->pCommand->command;
		if (oldCommandType == CCharacterCommand::CC_Label)
		{
			this->pGotoLabelListBox->RemoveItem(this->pCommand->x);
			bRemovedLabel = true;
		}
	}

	this->pCommand->command =
		(CCharacterCommand::CharCommand)this->pActionListBox->GetSelectedItem();
	if (bRemovedLabel && this->pCommand->command == CCharacterCommand::CC_Label)
		bRemovedLabel = false;

	//When the command is changed, reset fields to avoid bad values, possibly leading to crashes.
	if (oldCommandType != this->pCommand->command)
	{
		this->pCommand->x = this->pCommand->y = 0;
		this->pCommand->w = this->pCommand->h = 0;
		this->pCommand->flags = 0;
		this->pCommand->label.resize(0);
	}

	switch (this->pCommand->command)
	{
		case CCharacterCommand::CC_MoveTo:
		{
			this->pCommand->w = this->pOnOffListBox->GetSelectedItem();
			this->pCommand->h = this->pOnOffListBox2->GetSelectedItem();

			//Add set bit-fields.
			this->pCommand->flags = 0;
			CIDSet flagSet = this->pWaitFlagsListBox->GetSelectedItems();
			for (CIDSet::const_iterator flag = flagSet.begin();
					flag != flagSet.end(); ++flag)
				this->pCommand->flags += *flag;

			if (this->pCommand->flags)
				AddCommand();
			else
				QueryXY();
		}
		break;
		case CCharacterCommand::CC_MoveRel:
		{
			CTextBoxWidget *pRel = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_MOVERELX));
			ASSERT(pRel);
			this->pCommand->x = (UINT)(_Wtoi(pRel->GetText()));
			pRel = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_MOVERELY));
			ASSERT(pRel);
			this->pCommand->y = (UINT)(_Wtoi(pRel->GetText()));

			this->pCommand->w = this->pOnOffListBox->GetSelectedItem();
			this->pCommand->h = this->pOnOffListBox2->GetSelectedItem();
			AddCommand();
		}
		break;

		case CCharacterCommand::CC_WaitForDoorTo:
			this->pCommand->w = this->pOpenCloseListBox->GetSelectedItem();
			//NO BREAK
		case CCharacterCommand::CC_AppearAt:
		case CCharacterCommand::CC_ActivateItemAt:
			QueryXY();
		break;

		case CCharacterCommand::CC_Wait:
		case CCharacterCommand::CC_WaitForTurn:
		{
			CTextBoxWidget *pDelay = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_WAIT));
			ASSERT(pDelay);
			this->pCommand->x = _Wtoi(pDelay->GetText());
			AddCommand();
		}
		break;
		case CCharacterCommand::CC_WaitForCueEvent:
		{
			this->pCommand->x = this->pEventListBox->GetSelectedItem();
			AddCommand();
		}
		break;
		case CCharacterCommand::CC_WaitForRect:
		case CCharacterCommand::CC_WaitForNotRect:
		{
			//Add set bit-fields.
			this->pCommand->flags = 0;
			CIDSet flagSet = this->pWaitFlagsListBox->GetSelectedItems();
			for (CIDSet::const_iterator flag = flagSet.begin();
					flag != flagSet.end(); ++flag)
				this->pCommand->flags += *flag;

			QueryRect();
		}
		break;
		case CCharacterCommand::CC_BuildMarker:
		case CCharacterCommand::CC_WaitForItem:
			this->pCommand->flags = this->pBuildItemsListBox->GetSelectedItem();
			QueryRect();
		break;
		case CCharacterCommand::CC_Speech:
		{
			//Speech dialog.
			CTextBoxWidget *pDelay = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_DELAY));
			ASSERT(pDelay);
			CListBoxWidget *pMood = DYN_CAST(CListBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_MOODLISTBOX));
			ASSERT(pMood);
			CTextBoxWidget *pDialogue = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_SPEECHTEXT));
			ASSERT(pDialogue);

			if (!this->pCommand->pSpeech)
				this->pCommand->pSpeech = g_pTheDB->Speech.GetNew();
			this->pCommand->pSpeech->dwDelay = _Wtoi(pDelay->GetText());
			this->pCommand->pSpeech->wCharacter = this->pSpeakerListBox->GetSelectedItem();
			this->pCommand->pSpeech->wMood = pMood->GetSelectedItem();
			const bool bTextWasModified = this->bEditingCommand && WCScmp(
					(const WCHAR*)this->pCommand->pSpeech->MessageText, pDialogue->GetText()) != 0;
			this->pCommand->pSpeech->MessageText = pDialogue->GetText();

			//Import sound clip, if desired.
			if (this->pSound)
			{
				this->pCommand->pSpeech->SetSound(this->pSound);
				this->pSound = NULL;
			}
			if (this->pCommand->pSpeech->wCharacter != Speaker_Custom ||
					bTextWasModified) //don't force repositioning custom text if the user just wanted to modify the text
				AddCommand();
			else {
				//User decides where to place this text message in room.
				QueryXY();
			}
		}
		break;

		case CCharacterCommand::CC_Imperative:
			this->pCommand->x = this->pImperativeListBox->GetSelectedItem();
			AddCommand();
		break;

		case CCharacterCommand::CC_SetWaterTraversal:
			this->pCommand->x = this->pWaterTraversalListBox->GetSelectedItem();
			AddCommand();
		break;

		case CCharacterCommand::CC_GenerateEntity:
			this->pCommand->w = this->pDirectionListBox2->GetSelectedItem();
			this->pCommand->h = this->pPlayerGraphicListBox->GetSelectedItem();
			QueryXY();
		break;

		case CCharacterCommand::CC_GameEffect:
			this->pCommand->w = this->pDirectionListBox3->GetSelectedItem();
			this->pCommand->h = this->pVisualEffectsListBox->GetSelectedItem();
			this->pCommand->flags = this->pOnOffListBox3->GetSelectedItem();
			QueryXY();
		break;

		case CCharacterCommand::CC_StartGlobalScript:
			this->pCommand->x = this->pGlobalScriptListBox->GetSelectedItem();
			if (!this->pCommand->x)
			{
				//No custom character specified.  Don't add the command.
				if (!this->bEditingCommand)
					delete this->pCommand;

				this->pCommand = NULL;
				break;
			}
			AddCommand();
		break;

		case CCharacterCommand::CC_FlushSpeech:
		case CCharacterCommand::CC_SetPlayerSword:
			this->pCommand->x = this->pOnOffListBox->GetSelectedItem();
			AddCommand();
		break;

		case CCharacterCommand::CC_FaceDirection:
		case CCharacterCommand::CC_WaitForPlayerToFace:
		case CCharacterCommand::CC_WaitForPlayerToMove:
			this->pCommand->x = this->pDirectionListBox->GetSelectedItem();
			AddCommand();
		break;

		case CCharacterCommand::CC_SetMusic:
			//Either music ID or name is set.
			this->pCommand->x = this->pMusicListBox->GetSelectedItem();
			this->pCommand->y = 0;
			this->pCommand->label = wszEmpty;
			if ((int)this->pCommand->x == SONGID_CUSTOM)
			{
				const UINT dwVal = SelectMediaID(this->pCommand->y, CEntranceSelectDialogWidget::Sounds);
				if (dwVal)
					this->pCommand->y = dwVal;
				else
				{
					//No dataID was specified.  Don't add the command.
					if (!this->bEditingCommand)
						delete this->pCommand;

					this->pCommand = NULL;
					break;
				}
			}
			else if ((int)this->pCommand->x >= SONGID_COUNT)
			{
				this->pCommand->label = this->pMusicListBox->GetSelectedItemText();
				if (!this->pCommand->label.empty())
					this->pCommand->x = 0;
			}
			AddCommand();
		break;

		case CCharacterCommand::CC_Label:
		{
			CTextBoxWidget *pLabelText = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_GOTOLABELTEXT));
			ASSERT(pLabelText);
			//Provide an ID for this label.
			if (oldCommandType != CCharacterCommand::CC_Label)
				this->pCommand->x = ++this->wIncrementedLabel;
			ASSERT(this->pCommand->x);

			//Ensure this label is unique.
			WSTRING text = pLabelText->GetText();

			stripTrailingWhitespace(text);

			if (this->bEditingCommand)
				this->pCommand->label = wszEmpty; //don't match against the old label
			while (GetCommandWithLabelText(commands, text.c_str()) != NULL)
				text += wszHyphen;
			this->pCommand->label = text;

			//Default empty label to label ID.
			if (this->pCommand->label.empty())
			{
				WCHAR text[MAX_TEXT_LABEL_SIZE];
				this->pCommand->label = _itoW(this->pCommand->x, text, 10);
			}
			AddCommand();
		}
		break;

		case CCharacterCommand::CC_AnswerOption:
			{
				CTextBoxWidget *pLabelText = DYN_CAST(CTextBoxWidget*, CWidget*,
						this->pAddCommandDialog->GetWidget(TAG_GOTOLABELTEXT));
				ASSERT(pLabelText);
				this->pCommand->x = this->pGotoLabelListBox->GetSelectedItem();
				if (!this->pCommand->pSpeech)
					this->pCommand->pSpeech = g_pTheDB->Speech.GetNew();
				this->pCommand->pSpeech->MessageText = pLabelText->GetText();
				if (this->pCommand->x && WCSlen(pLabelText->GetText()))
				{
					AddCommand();
				} else {
					//No answer text or goto destination label was specified.
					//Don't add the command.
					if (!this->bEditingCommand)
						delete this->pCommand;

					this->pCommand = NULL;
				}
			}
		break;

		case CCharacterCommand::CC_Question:
			{
				CTextBoxWidget *pLabelText = DYN_CAST(CTextBoxWidget*, CWidget*,
						this->pAddCommandDialog->GetWidget(TAG_GOTOLABELTEXT));
				ASSERT(pLabelText);
				if (!this->pCommand->pSpeech)
					this->pCommand->pSpeech = g_pTheDB->Speech.GetNew();
				this->pCommand->pSpeech->MessageText = pLabelText->GetText();
			}
			AddCommand();
		break;

		case CCharacterCommand::CC_GoTo:
			this->pCommand->x = this->pGotoLabelListBox->GetSelectedItem();
			if (this->pCommand->x)
				AddCommand();
			else
			{
				//No goto destination label was specified -- don't add the command.
				if (!this->bEditingCommand)
					delete this->pCommand;

				this->pCommand = NULL;
			}
		break;

		case CCharacterCommand::CC_LevelEntrance:
		{
			CEditRoomScreen *pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
					g_pTheSM->GetScreen(SCR_EditRoom));
			ASSERT(pEditRoomScreen->pHold);
			UINT dwVal = this->pCommand->x;
			if (pEditRoomScreen->SelectListID(pEditRoomScreen->pEntranceBox, pEditRoomScreen->pHold,
					dwVal, MID_ExitLevelPrompt) == CEntranceSelectDialogWidget::OK)
			{
				this->pCommand->x = dwVal;
				this->pCommand->y = this->pOnOffListBox->GetSelectedItem();
				AddCommand();
			}
		}
		break;

		case CCharacterCommand::CC_VarSet:
		case CCharacterCommand::CC_WaitForVar:
		{
			this->pCommand->x = this->pVarListBox->GetSelectedItem();
			if (!this->pCommand->x)
			{
				//No variable specified. Don't add the command.
				if (!this->bEditingCommand)
					delete this->pCommand;

				this->pCommand = NULL;
				break;
			}
			this->pCommand->y =
				this->pCommand->command == CCharacterCommand::CC_VarSet ?
					this->pVarOpListBox->GetSelectedItem() :
					this->pVarCompListBox->GetSelectedItem();
			this->pCommand->w = 0; //default

			if (this->pCommand->y == ScriptVars::AppendText ||
				 this->pCommand->y == ScriptVars::AssignText ||
				 this->pCommand->y == ScriptVars::EqualsText)
			{
				CTextBoxWidget *pVarText = DYN_CAST(CTextBoxWidget*, CWidget*,
						this->pAddCommandDialog->GetWidget(TAG_GOTOLABELTEXT));
				ASSERT(pVarText);
				this->pCommand->label = pVarText->GetText();
			} else {
				CTextBoxWidget *pVarOperand = DYN_CAST(CTextBoxWidget*, CWidget*,
						this->pAddCommandDialog->GetWidget(TAG_VARVALUE));
				ASSERT(pVarOperand);
				const WCHAR *pOperandText = pVarOperand->GetText();
				ASSERT(pOperandText);

				//Is operand just a number or is it a more complex expression?
				if (isWInteger(pOperandText))
				{
					this->pCommand->w = _Wtoi(pOperandText);
					this->pCommand->label.resize(0);
				} else {
					this->pCommand->label = pOperandText;
				}
			}

			AddCommand();
		}
		break;

		case CCharacterCommand::CC_SetPlayerAppearance:
		case CCharacterCommand::CC_SetNPCAppearance:
			this->pCommand->x = this->pPlayerGraphicListBox->GetSelectedItem();
			AddCommand();
		break;

		case CCharacterCommand::CC_CutScene:
		{
			CTextBoxWidget *pDelay = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_WAIT));
			ASSERT(pDelay);
			this->pCommand->x = _Wtoi(pDelay->GetText());

			//Force cut scenes to be correctly consistent in demo playback.
			static const UINT wMaxMoveDelay = 255*10; //ms
			if (this->pCommand->x > wMaxMoveDelay)
				this->pCommand->x = wMaxMoveDelay;

			AddCommand();
		}
		break;

		case CCharacterCommand::CC_AmbientSound:
		case CCharacterCommand::CC_AmbientSoundAt:
		{
			const UINT dwVal = SelectMediaID(this->pCommand->w, CEntranceSelectDialogWidget::Sounds);
			this->pCommand->w = dwVal;
			this->pCommand->h = this->pOnOffListBox->GetSelectedItem();
			if (this->pCommand->command == CCharacterCommand::CC_AmbientSound)
				AddCommand();
			else
				QueryXY();
		}
		break;

		case CCharacterCommand::CC_PlayVideo:
		{
			const UINT dwVal = SelectMediaID(this->pCommand->w, CEntranceSelectDialogWidget::Videos);
			if (dwVal)
			{
				CTextBoxWidget *pRel = DYN_CAST(CTextBoxWidget*, CWidget*,
						this->pAddCommandDialog->GetWidget(TAG_MOVERELX));
				ASSERT(pRel);
				this->pCommand->x = (UINT)(_Wtoi(pRel->GetText()));
				pRel = DYN_CAST(CTextBoxWidget*, CWidget*,
						this->pAddCommandDialog->GetWidget(TAG_MOVERELY));
				ASSERT(pRel);
				this->pCommand->y = (UINT)(_Wtoi(pRel->GetText()));

				this->pCommand->w = dwVal;
				AddCommand();
			} else {
				//Don't add the command.
				if (!this->bEditingCommand)
					delete this->pCommand;
				this->pCommand = NULL;
			}
		}
		break;

		case CCharacterCommand::CC_WaitForNoBuilding:
			QueryRect();
		break;

		case CCharacterCommand::CC_Appear:
		case CCharacterCommand::CC_Disappear:
		case CCharacterCommand::CC_EndScript:
		case CCharacterCommand::CC_TurnIntoMonster:
		case CCharacterCommand::CC_WaitForCleanRoom:
		case CCharacterCommand::CC_WaitForPlayerToTouchMe:
		case CCharacterCommand::CC_EndScriptOnExit:
		case CCharacterCommand::CC_If:
		case CCharacterCommand::CC_IfElse:
		case CCharacterCommand::CC_IfEnd:
			AddCommand();
		break;

		//Deprecated commands.
		case CCharacterCommand::CC_GotoIf:
		case CCharacterCommand::CC_WaitForHalph:
		case CCharacterCommand::CC_WaitForNotHalph:
		case CCharacterCommand::CC_WaitForMonster:
		case CCharacterCommand::CC_WaitForNotMonster:
		case CCharacterCommand::CC_WaitForCharacter:
		case CCharacterCommand::CC_WaitForNotCharacter:
		default: ASSERT(!"Invalid character command"); break;
	}

	//Gotos might have become invalid.  Display that immediately.
	if (bRemovedLabel)
		PopulateCommandDescriptions(pActiveCommandList, commands);
}

//*****************************************************************************
void CCharacterDialogWidget::SetWidgetsFromCommandParameters()
//Set state of command settings widgets from command state.
{
	ASSERT(this->pCommand);
	ASSERT(!this->pSound);
	WCHAR temp[500];
	switch (this->pCommand->command)
	{
		case CCharacterCommand::CC_MoveRel:
		{
			CTextBoxWidget *pRel = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_MOVERELX));
			ASSERT(pRel);
			pRel->SetText(_itoW((int)this->pCommand->x, temp, 10));
			pRel = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_MOVERELY));
			ASSERT(pRel);
			pRel->SetText(_itoW((int)this->pCommand->y, temp, 10));

			this->pOnOffListBox->SelectItem(this->pCommand->w);
			this->pOnOffListBox2->SelectItem(this->pCommand->h);
		}
		break;

		case CCharacterCommand::CC_MoveTo:
			this->pOnOffListBox->SelectItem(this->pCommand->w);
			this->pOnOffListBox2->SelectItem(this->pCommand->h);
			SetBitFlags();
		break;

		case CCharacterCommand::CC_Wait:
		case CCharacterCommand::CC_WaitForTurn:
		{
			CTextBoxWidget *pDelay = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_WAIT));
			ASSERT(pDelay);
			pDelay->SetText(_itoW(this->pCommand->x, temp, 10));
		}
		break;

		case CCharacterCommand::CC_WaitForCueEvent:
			this->pEventListBox->SelectItem(this->pCommand->x);
		break;

		case CCharacterCommand::CC_WaitForDoorTo:
			this->pOpenCloseListBox->SelectItem(this->pCommand->w);
		break;

		case CCharacterCommand::CC_LevelEntrance:
			this->pOnOffListBox->SelectItem(this->pCommand->y);
		break;

		case CCharacterCommand::CC_FaceDirection:
		case CCharacterCommand::CC_WaitForPlayerToFace:
		case CCharacterCommand::CC_WaitForPlayerToMove:
			this->pDirectionListBox->SelectItem(this->pCommand->x);
		break;

		case CCharacterCommand::CC_BuildMarker:
		case CCharacterCommand::CC_WaitForItem:
			this->pBuildItemsListBox->SelectItem(this->pCommand->flags);
		break;

		case CCharacterCommand::CC_SetMusic:
			if ((int)this->pCommand->x < SONGID_COUNT)
				this->pMusicListBox->SelectItem(this->pCommand->x);
			else if (this->pCommand->x == static_cast<UINT>(SONGID_CUSTOM))
			{
				this->pMusicListBox->SelectItem(this->pCommand->x);
			} else {
				for (UINT wIndex=this->pMusicListBox->GetItemCount(); wIndex--; )
					if (!this->pCommand->label.compare(this->pMusicListBox->GetTextAtLine(wIndex)))
					{
						this->pMusicListBox->SelectLine(wIndex);
						break;
					}
			}
		break;

		case CCharacterCommand::CC_Speech:
		{
			CTextBoxWidget *pDelay = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_DELAY));
			ASSERT(pDelay);
			CTextBoxWidget *pDialogue = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_SPEECHTEXT));
			ASSERT(pDialogue);

			CDbSpeech *pSpeech = this->pCommand->pSpeech;
			ASSERT(pSpeech);
			if (pSpeech)	//robustness
			{
				pDelay->SetText(_itoW(pSpeech->dwDelay, temp, 10));
				this->pSpeakerListBox->SelectItem(pSpeech->wCharacter);
				this->pMoodListBox->SelectItem(pSpeech->wMood);
				pDialogue->SetText((const WCHAR*)pSpeech->MessageText);

				this->pSound = (CDbDatum*)pSpeech->GetSound(); //loads sound clip from DB
			}
		}
		break;

		case CCharacterCommand::CC_Imperative:
			this->pImperativeListBox->SelectItem(this->pCommand->x);
		break;

		case CCharacterCommand::CC_SetWaterTraversal:
			this->pWaterTraversalListBox->SelectItem(this->pCommand->x);
		break;

		case CCharacterCommand::CC_GenerateEntity:
			this->pDirectionListBox2->SelectItem(this->pCommand->w);
			this->pPlayerGraphicListBox->SelectItem(this->pCommand->h);
		break;

		case CCharacterCommand::CC_GameEffect:
			this->pDirectionListBox3->SelectItem(this->pCommand->w);
			this->pVisualEffectsListBox->SelectItem(this->pCommand->h);
			this->pOnOffListBox3->SelectItem(this->pCommand->flags);
		break;

		case CCharacterCommand::CC_StartGlobalScript:
			this->pGlobalScriptListBox->SelectItem(this->pCommand->x);
		break;

		case CCharacterCommand::CC_FlushSpeech:
		case CCharacterCommand::CC_SetPlayerSword:
			this->pOnOffListBox->SelectItem(this->pCommand->x);
		break;

		case CCharacterCommand::CC_Label:
		{
			CTextBoxWidget *pGotoLabel = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_GOTOLABELTEXT));
			pGotoLabel->SetText(this->pCommand->label.c_str());
		}
		break;
		case CCharacterCommand::CC_Question:
		{
			CTextBoxWidget *pQuestionText = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_GOTOLABELTEXT));
			CDbSpeech *pSpeech = this->pCommand->pSpeech;
			ASSERT(pSpeech);
			if (pSpeech)	//robustness
				pQuestionText->SetText((const WCHAR*)pSpeech->MessageText);
		}
		break;
		case CCharacterCommand::CC_AnswerOption:
		{
			CTextBoxWidget *pAnswerText = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_GOTOLABELTEXT));
			CDbSpeech *pSpeech = this->pCommand->pSpeech;
			ASSERT(pSpeech);
			if (pSpeech)	//robustness
				pAnswerText->SetText((const WCHAR*)pSpeech->MessageText);
			this->pGotoLabelListBox->SelectItem(this->pCommand->x);
		}
		break;
		case CCharacterCommand::CC_GoTo:
			this->pGotoLabelListBox->SelectItem(this->pCommand->x);
		break;

		case CCharacterCommand::CC_WaitForRect:
		case CCharacterCommand::CC_WaitForNotRect:
			SetBitFlags();
		break;

		case CCharacterCommand::CC_VarSet:
		case CCharacterCommand::CC_WaitForVar:
		{
			this->pVarListBox->SelectItem(this->pCommand->x);
			if (this->pCommand->command == CCharacterCommand::CC_VarSet)
				this->pVarOpListBox->SelectItem(this->pCommand->y);
			else
				this->pVarCompListBox->SelectItem(this->pCommand->y);

			CTextBoxWidget *pVarOperand = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_VARVALUE));
			ASSERT(pVarOperand);
			CTextBoxWidget *pVarText = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_GOTOLABELTEXT));
			ASSERT(pVarText);

			if (this->pCommand->y == ScriptVars::AppendText ||
				 this->pCommand->y == ScriptVars::AssignText ||
				 this->pCommand->y == ScriptVars::EqualsText)
			{
				pVarOperand->SetText(wszEmpty);
				pVarText->SetText(this->pCommand->label.c_str());
			} else {
				if (!this->pCommand->label.empty())
					pVarOperand->SetText(this->pCommand->label.c_str());
				else
					pVarOperand->SetText(_itoW(this->pCommand->w, temp, 10));
				pVarText->SetText(wszEmpty);
			}
		}
		break;

		case CCharacterCommand::CC_SetPlayerAppearance:
		case CCharacterCommand::CC_SetNPCAppearance:
			this->pPlayerGraphicListBox->SelectItem(this->pCommand->x);
		break;
		case CCharacterCommand::CC_CutScene:
		{
			CTextBoxWidget *pDelay = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_WAIT));
			ASSERT(pDelay);
			pDelay->SetText(_itoW(this->pCommand->x, temp, 10));
		}
		break;

		case CCharacterCommand::CC_AmbientSound:
		case CCharacterCommand::CC_AmbientSoundAt:
			this->pOnOffListBox->SelectItem(this->pCommand->h);
		break;

		case CCharacterCommand::CC_PlayVideo:
		{
			CTextBoxWidget *pRel = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_MOVERELX));
			ASSERT(pRel);
			pRel->SetText(_itoW((int)this->pCommand->x, temp, 10));
			pRel = DYN_CAST(CTextBoxWidget*, CWidget*,
					this->pAddCommandDialog->GetWidget(TAG_MOVERELY));
			ASSERT(pRel);
			pRel->SetText(_itoW((int)this->pCommand->y, temp, 10));
		}
		break;

		case CCharacterCommand::CC_Appear:
		case CCharacterCommand::CC_AppearAt:
		case CCharacterCommand::CC_Disappear:
		case CCharacterCommand::CC_EndScript:
		case CCharacterCommand::CC_WaitForCleanRoom:
		case CCharacterCommand::CC_WaitForPlayerToTouchMe:
		case CCharacterCommand::CC_ActivateItemAt:
		case CCharacterCommand::CC_TurnIntoMonster:
		case CCharacterCommand::CC_EndScriptOnExit:
		case CCharacterCommand::CC_If:
		case CCharacterCommand::CC_IfElse:
		case CCharacterCommand::CC_IfEnd:
		case CCharacterCommand::CC_WaitForNoBuilding:
			break;

		//Deprecated commands.
		case CCharacterCommand::CC_GotoIf:
		case CCharacterCommand::CC_WaitForHalph:
		case CCharacterCommand::CC_WaitForNotHalph:
		case CCharacterCommand::CC_WaitForMonster:
		case CCharacterCommand::CC_WaitForNotMonster:
		case CCharacterCommand::CC_WaitForCharacter:
		case CCharacterCommand::CC_WaitForNotCharacter:
		default: ASSERT(!"Invalid character command (2)"); break;
	}
	this->pAddCommandDialog->RequestPaint();
}

//*****************************************************************************
const CCharacterCommand* CCharacterDialogWidget::GetCommandWithLabel(
//Returns: pointer to command with specified label, or NULL if none
//
//Params:
	const COMMANDPTR_VECTOR& commands,
	const UINT label)
const
{
	if (!label) return NULL;

	for (UINT wIndex=commands.size(); wIndex--; )
	{
		const CCharacterCommand *pCommand = commands[wIndex];
		if (pCommand->command == CCharacterCommand::CC_Label &&
				label == pCommand->x)
			return pCommand;
	}
	return NULL;
}

//*****************************************************************************
const CCharacterCommand* CCharacterDialogWidget::GetCommandWithLabelText(
//Returns: pointer to command with specified label text, or NULL if none
//
//Params:
	const COMMANDPTR_VECTOR& commands,
	const WCHAR* pText)
const
{
	ASSERT(pText);
	for (UINT wIndex=commands.size(); wIndex--; )
	{
		const CCharacterCommand *pCommand = commands[wIndex];
		if (pCommand->command == CCharacterCommand::CC_Label &&
				!WCSicmp(pText, pCommand->label.c_str()))
			return pCommand;
	}
	return NULL;
}

//*****************************************************************************
void CCharacterDialogWidget::SetBitFlags()
//Set list items by bit-field.
{
	this->pWaitFlagsListBox->DeselectAll();

	UINT wBitfield = 1;
	for (UINT wBits = 0; wBits<32; ++wBits, wBitfield *= 2)
	{
		if ((this->pCommand->flags & wBitfield) == wBitfield)
			this->pWaitFlagsListBox->SelectItem(wBitfield,true);
	}
}

//*****************************************************************************
void CCharacterDialogWidget::TestSound()
//Plays the loaded sound effect, if any.
{
	if (this->pSound)
		g_pTheSound->PlayVoice(this->pSound->data);
}

//*****************************************************************************
void CCharacterDialogWidget::UpdateCharacter()
//Transfers all script information to character assigned for editing
//and clears local state.
{
	//CDbDatum *pSound = NULL;
	ASSERT(this->pCharacter);
	this->pCharacter->commands.clear();
	COMMANDPTR_VECTOR::iterator command;
	for (command = this->commands.begin(); command != this->commands.end(); ++command)
	{
		this->pCharacter->commands.push_back(**command);
		delete *command;
	}
	this->commands.clear();
	ClearPasteBuffer();
	this->pCharacter = NULL;
	this->pCommand = NULL;
}

//
//Script Parsing/Assembling functions
//

//*****************************************************************************
UINT CCharacterDialogWidget::findStartTextMatch(
//Returns: key for text found in list box matching start of pText+index
	CListBoxWidget* pListBoxWidget, const WCHAR* pText, UINT& index,
	bool& bFound) //(out) whether a valid value was found
const
{
	ASSERT(pListBoxWidget);

	UINT maxMatchedStringLength=0, matchIndex=NOT_FOUND;
	for (UINT line=pListBoxWidget->GetItemCount(); line--; )
	{
		//Find longest match to make sure correct command is found.
		const WCHAR *pwCommandText = pListBoxWidget->GetTextAtLine(line);
		const UINT len = WCSlen(pwCommandText);
		if (len <= maxMatchedStringLength)
			continue;
		if (!WCSncmp(pText+index, pwCommandText, len))
		{
			maxMatchedStringLength = len;
			matchIndex = line;
		}
	}

	//Match found?
	if (matchIndex < pListBoxWidget->GetItemCount())
	{
		bFound = true;
		index += maxMatchedStringLength; //advance past this text
		return pListBoxWidget->GetKeyAtLine(matchIndex);
	}
	bFound = false;
	return NOT_FOUND;
}

//*****************************************************************************
UINT CCharacterDialogWidget::findTextMatch(
//Returns: key for text found in list box exactly matching from pText+index
	CListBoxWidget* pListBoxWidget, const WCHAR* pText, const UINT index,
	bool& bFound) //(out) whether a valid value was found
const
{
	ASSERT(pListBoxWidget);

	UINT matchIndex=NOT_FOUND;
	for (UINT line=pListBoxWidget->GetItemCount(); line--; )
	{
		//Find longest match to make sure correct command is found.
		const WCHAR *pwCommandText = pListBoxWidget->GetTextAtLine(line);
		if (!WCScmp(pText+index, pwCommandText))
			matchIndex = line;
	}

	//Match found?
	if (matchIndex < pListBoxWidget->GetItemCount())
	{
		bFound = true;
		return pListBoxWidget->GetKeyAtLine(matchIndex);
	}
	bFound = false;
	return NOT_FOUND;
}

//*****************************************************************************
bool getTextToLastQuote(const WCHAR* pText, UINT& pos, WSTRING& foundText)
//Get all text up to last quotation mark.
//Returns: whether ending quotation mark was found
{
	//Find last quotation character.
	const UINT textLength = WCSlen(pText);
	ASSERT(textLength > 0);
	UINT lastPos = textLength - 1;
	while (lastPos > pos && pText[lastPos] != W_t('"'))
		--lastPos;
	if (lastPos <= pos)
		return false;

	foundText.assign(pText + pos, lastPos - pos); //output text
	pos = lastPos + 1; //update cursor index to position after quotation char
	return true;
}

//*****************************************************************************
CCharacterCommand* CCharacterDialogWidget::fromText(
//Parses a line of text into a command.
//
//Returns: pointer to a new character command if text parsed correctly, else NULL
//
//Params:
    WSTRING text)  //Text to parse
{
#define skipWhitespace while (pos < textLength && iswspace(pText[pos])) ++pos

#define skipComma skipWhitespace; if (pText[pos] == W_t(',')) {++pos; skipWhitespace;}
#define skipLeftParen skipWhitespace; if (pText[pos] == W_t('(')) {++pos; skipWhitespace;}
#define skipRightParen skipWhitespace; if (pText[pos] == W_t(')')) {++pos; skipWhitespace;}

#define parseChar(c) skipWhitespace; if (pText[pos++] != W_t(c)) {delete pCommand; return NULL;}

#define parseNumber(n) {UINT oldPos = pos; \
	if (pos < textLength && pText[pos] == W_t('-')) ++pos; \
	while (pos < textLength && iswdigit(pText[pos])) ++pos; \
	if (pos > oldPos)	n = _Wtoi(pText+oldPos); \
	else {delete pCommand; return NULL;}}

#define parseOptionalNumber(n) {UINT oldPos = pos; \
	if (pos < textLength && pText[pos] == W_t('-')) ++pos; \
	while (pos < textLength && iswdigit(pText[pos])) ++pos; \
	n = pos > oldPos ? _Wtoi(pText+oldPos) : 0;}

#define parseOption(x,pListBox,bFound) x = findStartTextMatch(pListBox, pText, pos, bFound)

#define parseMandatoryOption(x,pListBox,bFound) parseOption(x,pListBox,bFound); \
	if (!bFound) {delete pCommand; return NULL;}

	stripTrailingWhitespace(text);

	WCHAR *pText = (WCHAR*)text.c_str();

	//Ignore comment lines.
	if (pText[0] == W_t('#'))
		return NULL;

	const UINT textLength = text.length();
	UINT pos=0;
	bool bFound;

	//Strip pretty printing.
	while (pos < textLength && iswpunct(WCv(pText[pos]))) //skip leading error chars
		++pos;
	skipWhitespace;
	if (pos >= textLength)
		return NULL; //no relevant text on this line

	//Parse command.
	const UINT eCommand = findStartTextMatch(this->pActionListBox, pText, pos, bFound);
	if (eCommand >= CCharacterCommand::CC_Count)
		return NULL; //text doesn't match commands

	CCharacterCommand *pCommand = new CCharacterCommand();
	pCommand->command = CCharacterCommand::CharCommand(eCommand);

	skipWhitespace;

	switch (eCommand)
	{
	case CCharacterCommand::CC_Appear:                                   //No arguments
	case CCharacterCommand::CC_Disappear:
	case CCharacterCommand::CC_EndScript:
	case CCharacterCommand::CC_EndScriptOnExit:
	case CCharacterCommand::CC_If:
	case CCharacterCommand::CC_IfElse:
	case CCharacterCommand::CC_IfEnd:
	case CCharacterCommand::CC_TurnIntoMonster:
	case CCharacterCommand::CC_WaitForCleanRoom:
	case CCharacterCommand::CC_WaitForPlayerToTouchMe:
	break;

	case CCharacterCommand::CC_CutScene:
	case CCharacterCommand::CC_Imperative:
	case CCharacterCommand::CC_Wait:
	case CCharacterCommand::CC_WaitForTurn:
		skipLeftParen;
		parseNumber(pCommand->x);
	break;

	case CCharacterCommand::CC_ActivateItemAt:
	case CCharacterCommand::CC_AppearAt:
	case CCharacterCommand::CC_LevelEntrance:
		skipLeftParen;
		parseNumber(pCommand->x); skipComma;
		parseNumber(pCommand->y);
	break;

	case CCharacterCommand::CC_WaitForDoorTo:
		parseMandatoryOption(pCommand->w,this->pOpenCloseListBox,bFound);
		skipComma;
		skipLeftParen;
		parseNumber(pCommand->x); skipComma;
		parseNumber(pCommand->y);
	break;

	case CCharacterCommand::CC_MoveTo:
	{
		UINT flag;
		do {
			parseOption(flag,this->pWaitFlagsListBox,bFound);
			if (bFound)
				pCommand->flags |= flag;
			skipWhitespace;
		} while (bFound);
	}
	//no break
	case CCharacterCommand::CC_MoveRel:
		if (!pCommand->flags) //MoveTo
		{
			skipLeftParen;
			parseNumber(pCommand->x); skipComma;
			parseNumber(pCommand->y); skipComma;
			skipRightParen;
		}
		skipLeftParen;
		parseOptionalNumber(pCommand->w); skipComma;
		parseOptionalNumber(pCommand->h); skipComma;
	break;

	case CCharacterCommand::CC_WaitForRect:
	case CCharacterCommand::CC_WaitForNotRect:
	{
		UINT flag;
		do {
			parseOption(flag,this->pWaitFlagsListBox,bFound);
			if (bFound)
				pCommand->flags |= flag;
			skipWhitespace;
		} while (bFound);
	}
	//no break
	case CCharacterCommand::CC_WaitForNoBuilding:
		skipLeftParen;
		parseNumber(pCommand->x); skipComma;
		parseNumber(pCommand->y); skipComma;
		skipRightParen;
		skipLeftParen;
		parseNumber(pCommand->w); pCommand->w -= pCommand->x; skipComma;
		parseNumber(pCommand->h); pCommand->h -= pCommand->y;
	break;

	case CCharacterCommand::CC_AmbientSoundAt:
		skipLeftParen;
		parseNumber(pCommand->x); skipComma;
		parseNumber(pCommand->y); skipComma;
		skipRightParen;
		//no break
	case CCharacterCommand::CC_AmbientSound:
		skipLeftParen;
		parseNumber(pCommand->w); skipComma;
		parseNumber(pCommand->h);
	break;

	case CCharacterCommand::CC_PlayVideo:
		skipLeftParen;
		parseNumber(pCommand->x); skipComma;
		parseNumber(pCommand->y); skipComma;
		skipRightParen;
		parseNumber(pCommand->w);
	break;

	case CCharacterCommand::CC_BuildMarker:
	case CCharacterCommand::CC_WaitForItem:
		parseMandatoryOption(pCommand->flags,this->pBuildItemsListBox,bFound);
		skipComma;
		skipLeftParen;
		parseNumber(pCommand->x); skipComma;
		parseNumber(pCommand->y); skipComma;
		skipRightParen;
		skipLeftParen;
		parseNumber(pCommand->w); pCommand->w -= pCommand->x; skipComma;
		parseNumber(pCommand->h); pCommand->h -= pCommand->y;
	break;

	case CCharacterCommand::CC_Label:
	{
		pCommand->label = pText+pos;
		//Generate unique label info.
		GenerateUniqueLabelName(pCommand->label);
		pCommand->x = ++this->wIncrementedLabel;
		AddLabel(pCommand);
	}
	break;

	case CCharacterCommand::CC_GoTo:
		pCommand->label = pText+pos;
		//Caller must look up label ID.
	break;

	case CCharacterCommand::CC_AnswerOption:
	{
		//Answer is all text between outermost quotes.
		parseChar('"');
		WSTRING question;
		const bool bRes = getTextToLastQuote(pText, pos, question);
		if (!bRes)
		{
			delete pCommand;
			return NULL;
		}

		ASSERT(!pCommand->pSpeech);
		pCommand->pSpeech = g_pTheDB->Speech.GetNew();
		pCommand->pSpeech->MessageText = question.c_str();

		skipComma;
		pCommand->label = pText+pos;
		//Caller must look up label ID.
	}
	break;

	case CCharacterCommand::CC_Question:
	{
		ASSERT(!pCommand->pSpeech);
		pCommand->pSpeech = g_pTheDB->Speech.GetNew();
		pCommand->pSpeech->MessageText = pText+pos;
	}
	break;

	case CCharacterCommand::CC_FlushSpeech:
	case CCharacterCommand::CC_SetPlayerSword:
		parseMandatoryOption(pCommand->x,this->pOnOffListBox,bFound);
	break;

	case CCharacterCommand::CC_SetWaterTraversal:
		parseMandatoryOption(pCommand->x,this->pWaterTraversalListBox,bFound);
	break;

	case CCharacterCommand::CC_GenerateEntity:
		parseMandatoryOption(pCommand->h,this->pPlayerGraphicListBox,bFound);
		skipComma;
		parseNumber(pCommand->x); skipComma;
		parseNumber(pCommand->y); skipComma;
		parseMandatoryOption(pCommand->w,this->pDirectionListBox2,bFound);
	break;

	case CCharacterCommand::CC_GameEffect:
		parseMandatoryOption(pCommand->w,this->pDirectionListBox3,bFound);
		skipComma;
		parseMandatoryOption(pCommand->h, this->pVisualEffectsListBox,bFound);
		skipComma;
		skipLeftParen;
		parseNumber(pCommand->x); skipComma;
		parseNumber(pCommand->y); skipComma;
		skipRightParen;
		skipComma;
		parseMandatoryOption(pCommand->flags,this->pOnOffListBox3,bFound);
	break;

	case CCharacterCommand::CC_StartGlobalScript:
		parseMandatoryOption(pCommand->x,this->pGlobalScriptListBox,bFound);
	break;

	case CCharacterCommand::CC_WaitForCueEvent:
		parseMandatoryOption(pCommand->x,this->pEventListBox,bFound);
	break;

	case CCharacterCommand::CC_FaceDirection:
	case CCharacterCommand::CC_WaitForPlayerToFace:
	case CCharacterCommand::CC_WaitForPlayerToMove:
		parseMandatoryOption(pCommand->x,this->pDirectionListBox,bFound);
	break;

	case CCharacterCommand::CC_SetPlayerAppearance:
	case CCharacterCommand::CC_SetNPCAppearance:
		parseMandatoryOption(pCommand->x,this->pPlayerGraphicListBox,bFound);
	break;

	case CCharacterCommand::CC_SetMusic:
		skipLeftParen;
		parseNumber(pCommand->x); skipComma;
		parseNumber(pCommand->y); skipComma;
		pCommand->label += pText + pos;
		if (!pCommand->label.empty())
			pCommand->x = 0; //ensure custom music when specified
	break;

	case CCharacterCommand::CC_Speech:
	{
		//Speech is all text between outermost quotes.
		parseChar('"');
		WSTRING speech;
		const bool bRes = getTextToLastQuote(pText, pos, speech);
		if (!bRes)
		{
			delete pCommand;
			return NULL;
		}

		ASSERT(!pCommand->pSpeech);
		pCommand->pSpeech = g_pTheDB->Speech.GetNew();
		pCommand->pSpeech->MessageText = speech.c_str();
		skipComma;

		parseMandatoryOption(pCommand->pSpeech->wMood,this->pMoodListBox,bFound);
		skipComma;

		parseMandatoryOption(pCommand->pSpeech->wCharacter,this->pSpeakerListBox,bFound);
		skipComma;

		if (pCommand->pSpeech->wCharacter == Speaker_Custom)
		{
			skipLeftParen;
			parseNumber(pCommand->x); skipComma;
			parseNumber(pCommand->y); skipComma;
		}

		parseNumber(pCommand->pSpeech->dwDelay); skipComma;
		//Ignore name of sound data file, if any.
	}
	break;

	case CCharacterCommand::CC_VarSet:
	{
		//Var name is all text between outermost quotes.
		parseChar('"');
		WSTRING varName;
		const bool bRes = getTextToLastQuote(pText, pos, varName);
		if (!bRes)
		{
			delete pCommand;
			return NULL;
		}

		UINT tempIndex = 0;
		pCommand->x = findTextMatch(this->pVarListBox, varName.c_str(), tempIndex, bFound);
		if (!bFound)
		{
			pCommand->x = AddVar(varName.c_str());
			if (!pCommand->x)
			{
				delete pCommand;
				return NULL;
			}
		}

		skipWhitespace;
		if (pos >= textLength)
		{
			delete pCommand;
			return NULL;
		}
		const char varOperator = char(WCv(pText[pos]));
		++pos;
		switch (varOperator)
		{
			default: //robust default for bad operator char
			case '=': pCommand->y = ScriptVars::Assign; break;
			case '+': pCommand->y = ScriptVars::Inc; break;
			case '-': pCommand->y = ScriptVars::Dec; break;
			case '*': pCommand->y = ScriptVars::MultiplyBy; break;
			case '/': pCommand->y = ScriptVars::DivideBy; break;
			case '%': pCommand->y = ScriptVars::Mod; break;
			case ':': pCommand->y = ScriptVars::AssignText; break;
			case ';': pCommand->y = ScriptVars::AppendText; break;
		}

		skipWhitespace;
		switch (pCommand->y)
		{
			case ScriptVars::AppendText:
			case ScriptVars::AssignText:
				pCommand->label = pText + pos;
			break;
			default:
			{
				if (isWInteger(pText + pos))
					pCommand->w = _Wtoi(pText + pos); //get number
				else
					pCommand->label = pText + pos; //get text expression
			}
			break;
		}
	}
	break;

	case CCharacterCommand::CC_WaitForVar:
	{
		//Var name is all text between outermost quotes.
		//(Copied from case VarSet above.)
		parseChar('"');
		WSTRING varName;
		const bool bRes = getTextToLastQuote(pText, pos, varName);
		if (!bRes)
		{
			delete pCommand;
			return NULL;
		}

		UINT tempIndex = 0;
		pCommand->x = findTextMatch(this->pVarListBox, varName.c_str(), tempIndex, bFound);
		if (!bFound)
		{
			pCommand->x = AddVar(varName.c_str());
			if (!pCommand->x)
			{
				delete pCommand;
				return NULL;
			}
		}

		skipWhitespace;
		if (pos >= textLength)
		{
			delete pCommand;
			return NULL;
		}
		const char varOperator = char(WCv(pText[pos]));
		++pos;
		switch (varOperator)
		{
			default: //robust default for bad operator char
			case '=': pCommand->y = ScriptVars::Equals; break;
			case '>': pCommand->y = ScriptVars::Greater; break;
			case '<': pCommand->y = ScriptVars::Less; break;
			case ':': pCommand->y = ScriptVars::EqualsText; break;
		}

		skipWhitespace;
		switch (pCommand->y)
		{
			case ScriptVars::EqualsText:
				pCommand->label = pText + pos;
			break;
			default:
			{
				if (isWInteger(pText + pos))
					pCommand->w = _Wtoi(pText + pos); //get number
				else
					pCommand->label = pText + pos; //get text expression
			}
			break;
		}
	}
	break;

	default: ASSERT(false); break;
	}

	return pCommand;
#undef skipComma
#undef skipWhitespace
#undef skipLeftParen
#undef skipRightParen
#undef parseChar
#undef parseNumber
#undef parseOptionalNumber
#undef parseMandatoryOption
#undef parseOption
}

//*****************************************************************************
WSTRING CCharacterDialogWidget::toText(
//Converts a character command into a simple text format which can be parsed by ::fromText.
//
//Returns: a non-empty string if the operation succeeded, otherwise an empty string
//
//Params:
	const COMMANDPTR_VECTOR& commands,
    CCharacterCommand* pCommand)   //Command to parse
{
#define concatNum(n) wstr += _itoW(n,temp,10)
#define concatNumWithComma(n) concatNum(n); wstr += wszComma;
	WCHAR temp[1024];

	//Get command name.
	ASSERT(pCommand);
	CCharacterCommand& c = *pCommand;

	WSTRING wstr, wstrCommandName = this->pActionListBox->GetTextForKey(c.command);
	if (wstrCommandName.empty())
		return wstr;

	wstr += GetPrettyPrinting(commands, pCommand, 6, 3);
	wstr += wstrCommandName;
	wstr += wszSpace;

	switch (c.command)
	{
	case CCharacterCommand::CC_Appear:
	case CCharacterCommand::CC_Disappear:
	case CCharacterCommand::CC_EndScript:
	case CCharacterCommand::CC_EndScriptOnExit:
	case CCharacterCommand::CC_If:
	case CCharacterCommand::CC_IfElse:
	case CCharacterCommand::CC_IfEnd:
	case CCharacterCommand::CC_TurnIntoMonster:
	case CCharacterCommand::CC_WaitForCleanRoom:
	case CCharacterCommand::CC_WaitForPlayerToTouchMe:
	break;

	case CCharacterCommand::CC_CutScene:
	case CCharacterCommand::CC_Imperative:
	case CCharacterCommand::CC_Wait:
	case CCharacterCommand::CC_WaitForTurn:
		concatNum(c.x);
	break;

	case CCharacterCommand::CC_ActivateItemAt:
	case CCharacterCommand::CC_AppearAt:
	case CCharacterCommand::CC_LevelEntrance:
		concatNumWithComma(c.x);
		concatNum(c.y);
	break;

	case CCharacterCommand::CC_WaitForDoorTo:
		wstr += this->pOpenCloseListBox->GetTextForKey(c.w);
		wstr += wszComma;
		concatNumWithComma(c.x);
		concatNum(c.y);
	break;

	case CCharacterCommand::CC_MoveTo:
	{
		UINT wBitfield = 1;
		for (UINT wBits = 0; wBits<32; ++wBits, wBitfield *= 2)
		{
			if ((c.flags & wBitfield) == wBitfield)
			{
				wstr += this->pWaitFlagsListBox->GetTextForKey(wBitfield);
				wstr += wszSpace;
			}
		}
	}
	//no break
	case CCharacterCommand::CC_MoveRel:
		if (!c.flags) //MoveTo
		{
			concatNumWithComma(c.x);
			concatNumWithComma(c.y);
		}
		concatNumWithComma(c.w);
		concatNum(c.h);
	break;

	case CCharacterCommand::CC_WaitForRect:
	case CCharacterCommand::CC_WaitForNotRect:
	{
		UINT wBitfield = 1;
		for (UINT wBits = 0; wBits<32; ++wBits, wBitfield *= 2)
		{
			if ((c.flags & wBitfield) == wBitfield)
			{
				wstr += this->pWaitFlagsListBox->GetTextForKey(wBitfield);
				wstr += wszSpace;
			}
		}
	}
	//no break
	case CCharacterCommand::CC_WaitForNoBuilding:
		concatNumWithComma(c.x);
		concatNumWithComma(c.y);
		concatNumWithComma(c.x + c.w);
		concatNum(c.y + c.h);
	break;

	case CCharacterCommand::CC_AmbientSoundAt:
		concatNumWithComma(c.x);
		concatNumWithComma(c.y);
		//no break
	case CCharacterCommand::CC_AmbientSound:
		concatNumWithComma(c.w);
		concatNum(c.h);
	break;

	case CCharacterCommand::CC_PlayVideo:
		concatNumWithComma(c.x);
		concatNumWithComma(c.y);
		concatNum(c.w);
	break;

	case CCharacterCommand::CC_BuildMarker:
	case CCharacterCommand::CC_WaitForItem:
		wstr += this->pBuildItemsListBox->GetTextForKey(c.flags);
		wstr += wszComma;
		concatNumWithComma(c.x);
		concatNumWithComma(c.y);
		concatNumWithComma(c.x + c.w);
		concatNum(c.y + c.h);
	break;

	case CCharacterCommand::CC_Label:
		wstr += c.label;
	break;

	case CCharacterCommand::CC_GoTo:
	{
		const CCharacterCommand *pGotoCommand = GetCommandWithLabel(commands, c.x);
		wstr += pGotoCommand ? pGotoCommand->label : wszQuestionMark;
	}
	break;

	case CCharacterCommand::CC_AnswerOption:
	{
		CDbSpeech *pSpeech = c.pSpeech;
		wstr += wszQuote;
		wstr += pSpeech ? (const WCHAR*)(pSpeech->MessageText) : wszQuestionMark;
		wstr += wszQuote;
		wstr += wszComma;
		const CCharacterCommand *pGotoCommand = GetCommandWithLabel(commands, c.x);
		wstr += pGotoCommand ? pGotoCommand->label : wszQuestionMark;
	}
	break;

	case CCharacterCommand::CC_Question:
	{
		CDbSpeech *pSpeech = c.pSpeech;
		wstr += pSpeech ? (const WCHAR*)pSpeech->MessageText : wszQuestionMark;
	}
	break;

	case CCharacterCommand::CC_FlushSpeech:
	case CCharacterCommand::CC_SetPlayerSword:
		wstr += this->pOnOffListBox->GetTextForKey(c.x);
	break;

	case CCharacterCommand::CC_SetWaterTraversal:
		wstr += this->pWaterTraversalListBox->GetTextForKey(c.x);
	break;

	case CCharacterCommand::CC_GenerateEntity:
	{
		WSTRING charName = this->pPlayerGraphicListBox->GetTextForKey(c.h);
		wstr += charName.length() ? charName : wszQuestionMark;
		wstr += wszComma;
		concatNumWithComma(c.x);
		concatNumWithComma(c.y);
		wstr += this->pDirectionListBox2->GetTextForKey(c.w);
	}
	break;

	case CCharacterCommand::CC_GameEffect:
		wstr += this->pDirectionListBox3->GetTextForKey(c.w);
		wstr += wszComma;
		wstr += this->pVisualEffectsListBox->GetTextForKey(c.h);
		wstr += wszComma;
		concatNumWithComma(c.x);
		concatNumWithComma(c.y);
		wstr += this->pOnOffListBox3->GetTextForKey(c.flags);
	break;

	case CCharacterCommand::CC_StartGlobalScript:
	{
		const WSTRING charName = this->pGlobalScriptListBox->GetTextForKey(c.x);
		wstr += charName.length() ? charName : wszQuestionMark;
	}
	break;

	case CCharacterCommand::CC_WaitForCueEvent:
		wstr += this->pEventListBox->GetTextForKey(c.x);
	break;

	case CCharacterCommand::CC_FaceDirection:
	case CCharacterCommand::CC_WaitForPlayerToFace:
	case CCharacterCommand::CC_WaitForPlayerToMove:
		wstr += this->pDirectionListBox->GetTextForKey(c.x);
	break;

	case CCharacterCommand::CC_SetPlayerAppearance:
	case CCharacterCommand::CC_SetNPCAppearance:
	{
		const WSTRING charName = this->pPlayerGraphicListBox->GetTextForKey(c.x);
		wstr += charName.length() ? charName : wszQuestionMark;
	}
	break;

	case CCharacterCommand::CC_SetMusic:
		if (c.label.size())
		{
			concatNumWithComma(0);
			concatNumWithComma(c.y);
			wstr += c.label;
		} else {
			concatNumWithComma(c.x);
			concatNum(c.y);
		}
	break;

	case CCharacterCommand::CC_Speech:
	{
		CDbSpeech *pSpeech = c.pSpeech;
		ASSERT(pSpeech);
		if (!pSpeech)	//robustness
		{
			wstr += wszQuestionMark;
			break;
		}

		wstr += wszQuote;
		wstr += (const WCHAR*)(pSpeech->MessageText);
		wstr += wszQuote;
		wstr += wszComma;

		wstr += this->pMoodListBox->GetTextForKey(pSpeech->wMood);
		wstr += wszComma;

		WSTRING charName = this->pSpeakerListBox->GetTextForKey(pSpeech->wCharacter);
		wstr += charName.length() ? charName : wszQuestionMark;
		wstr += wszComma;
		if (pSpeech->wCharacter == Speaker_Custom)
		{
			concatNumWithComma(c.x);
			concatNumWithComma(c.y);
		}

		concatNumWithComma(pSpeech->dwDelay);

		if (pSpeech->dwDataID)
		{
			//Load sound clip name only from DB.
			wstr += GetDataName(pSpeech->dwDataID);
		}
		else if (pSpeech->GetSound() &&
				!((CDbDatum*)pSpeech->GetSound())->DataNameText.empty())
		{
			//Sound exists in object, but not yet in DB.  Just display its name.
			wstr += pSpeech->GetSound()->DataNameText;
		} else {
			//No attached sound.
			wstr += wszPeriod;
		}
	}
	break;

	case CCharacterCommand::CC_VarSet:
	{
		const WCHAR *wszVarName = this->pVarListBox->GetTextForKey(c.x);
		wstr += wszQuote;
		wstr += WCSlen(wszVarName) ? wszVarName : wszQuestionMark;
		wstr += wszQuote;
		wstr += wszSpace;
		switch (c.y)
		{
			case ScriptVars::Assign: wstr += wszEqual; break;
			case ScriptVars::Inc: wstr += wszPlus; break;
			case ScriptVars::Dec: wstr += wszHyphen; break;
			case ScriptVars::MultiplyBy: wstr += wszAsterisk; break;
			case ScriptVars::DivideBy: wstr += wszForwardSlash; break;
			case ScriptVars::Mod: wstr += wszPercent; break;
			case ScriptVars::AssignText: wstr += wszColon; break;
			case ScriptVars::AppendText: wstr += wszSemicolon; break;
			default: wstr += wszQuestionMark; break;
		}
		wstr += wszSpace;
		switch (c.y)
		{
			case ScriptVars::AppendText:
			case ScriptVars::AssignText:
				wstr += c.label;
			break;
			default:
				if (!c.label.empty())
					wstr += c.label;
				else
					concatNum(c.w);
			break;
		}
	}
	break;

	case CCharacterCommand::CC_WaitForVar:
	{
		const WCHAR *wszVarName = this->pVarListBox->GetTextForKey(c.x);
		wstr += wszQuote;
		wstr += WCSlen(wszVarName) ? wszVarName : wszQuestionMark;
		wstr += wszQuote;
		wstr += wszSpace;
		switch (c.y)
		{
			case ScriptVars::Equals: wstr += wszEqual; break;
			case ScriptVars::EqualsText: wstr += wszColon; break;
			case ScriptVars::Greater: wstr += wszCloseAngle; break;
			case ScriptVars::Less: wstr += wszOpenAngle; break;
			default: wstr += wszQuestionMark; break;
		}
		wstr += wszSpace;
		switch (c.y)
		{
			case ScriptVars::EqualsText:
				wstr += c.label;
			break;
			default:
				if (!c.label.empty())
					wstr += c.label;
				else
					concatNum(c.w);
			break;
		}
	}
	break;

	default: ASSERT(!"Bad command"); break;
	}

	wstr += wszCRLF;
	return wstr;

#undef concatNum
#undef concatNumWithComma
}
