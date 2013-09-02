// $Id: RestoreScreen.cpp 9952 2012-03-17 21:50:06Z mrimer $

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
 * Contributor(s): Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "RestoreScreen.h"
#include "BrowserScreen.h"
#include "DemosScreen.h"
#include "DrodBitmapManager.h"
#include "DrodFontManager.h"
#include "DrodScreenManager.h"
#include "GameScreen.h"
#include <FrontEndLib/ButtonWidget.h>
#include <FrontEndLib/LabelWidget.h>
#include <FrontEndLib/ScrollableWidget.h>

#include "../DRODLib/Db.h"
#include "../DRODLib/CueEvents.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Assert.h>
#include <BackEndLib/Wchar.h>

#include <sstream>

//Widget tags.
const UINT TAG_ROOM_START = 1010;
const UINT TAG_ROOM_WIDGET = 1011;

const UINT TAG_LEVEL_START = 1020;

const UINT TAG_HOLD_START = 1021;
const UINT TAG_LEVEL_LBOX = 1022;
const UINT TAG_POSITION_LABEL = 1023;

const UINT TAG_LEVEL_EXPLORED = 1030;
const UINT TAG_LEVEL_SECRETS = 1031;
const UINT TAG_SAVED_GAME_STATS = 1032;

const UINT TAG_RESTORE = 1091;
const UINT TAG_CANCEL = 1092;
const UINT TAG_HELP = 1093;

//Reserve 2000 to 2999 for checkpoint tags.
const UINT TAG_CHECKPOINT = 2000;
#define IS_CHECKPOINT_TAG(t) ((t) >= 2000 && (t) < 3000)

//
//Protected methods.
//

//*****************************************************************************************
CRestoreScreen::CRestoreScreen()
	: CDrodScreen(SCR_Restore)
	, dwSelectedSavedGameID(0L), dwLastGameID(0L)
	, wConqueredRooms(0)
	, bHoldConquered(false), bResetWidgets(true)
	, pCurrentRestoreGame(NULL)
	, pRoomWidget(NULL)
	, pScaledRoomWidget(NULL)
	, pMapWidget(NULL)
	, pLevelListBoxWidget(NULL)
//Constructor.
{
	SetKeyRepeat(66);

	this->imageFilenames.push_back(string("Background"));

	static const UINT CX_SPACE = 10;
	static const UINT CY_SPACE = 10;
	static const UINT CY_TITLE = 50;

#ifdef RUSSIAN_BUILD
	static const UINT CX_RESTORE_BUTTON = 140;
	static const UINT CX_ROOM_START = 160;
	static const UINT CY_TITLE_SPACE = 12;
#else
	static const UINT CX_RESTORE_BUTTON = 110;
	static const UINT CX_ROOM_START = 130;
	static const UINT CY_TITLE_SPACE = 15;
#endif

	static const int Y_TITLE = CY_TITLE_SPACE;

	static const UINT CY_RESTORE_BUTTON = CY_STANDARD_BUTTON;
	static const UINT CX_CANCEL_BUTTON = 110;
	static const UINT CY_CANCEL_BUTTON = CY_RESTORE_BUTTON;
	static const UINT CX_HELP_BUTTON = CX_CANCEL_BUTTON;
	static const UINT CY_HELP_BUTTON = CY_RESTORE_BUTTON;
	const int X_CANCEL_BUTTON = this->w / 2;
	const int X_RESTORE_BUTTON = X_CANCEL_BUTTON - CX_RESTORE_BUTTON - CX_SPACE;
	const int X_HELP_BUTTON = X_CANCEL_BUTTON + CX_CANCEL_BUTTON + CX_SPACE;
	const int Y_RESTORE_BUTTON = this->h - CY_SPACE - CY_RESTORE_BUTTON;
	const int Y_CANCEL_BUTTON = Y_RESTORE_BUTTON;
	const int Y_HELP_BUTTON = Y_RESTORE_BUTTON;

	//Mini-room widget has strict proportions and its dimensions will define 
	//placement of most everything else.
	static const int Y_CHOOSE_POS_LABEL = Y_TITLE + CY_TITLE + CY_TITLE_SPACE;
	static const UINT CY_CHOOSE_POS_LABEL = CY_STANDARD_BUTTON;
	static const int Y_POSITION_LABEL = Y_CHOOSE_POS_LABEL + CY_CHOOSE_POS_LABEL;
	static const UINT CY_POSITION_LABEL = 25;
	static const int Y_MINIROOM = Y_POSITION_LABEL + CY_POSITION_LABEL;
	const UINT CY_MINIROOM = this->h - Y_MINIROOM - CY_STANDARD_BUTTON - CY_SPACE * 2 - 6;
	//Width of mini-room must be proportional to regular room display.
	static const UINT CX_MINIROOM = CY_MINIROOM * CDrodBitmapManager::CX_ROOM /
			CDrodBitmapManager::CY_ROOM;
	const int X_MINIROOM = this->w - CX_SPACE - CX_MINIROOM;

	static const int X_CHOOSE_POS_LABEL = X_MINIROOM;
	static const UINT CX_CHOOSE_POS_LABEL = 250;
	const int X_ROOM_START = this->w - CX_SPACE - CX_ROOM_START - 1;
	static const int Y_ROOM_START = Y_CHOOSE_POS_LABEL;
	static const UINT CY_ROOM_START = CY_CHOOSE_POS_LABEL;
	static const int X_POSITION_LABEL = X_MINIROOM;
	static const UINT CX_POSITION_LABEL = CX_MINIROOM;

	const UINT CX_MAP = this->w - CX_SPACE - CX_MINIROOM - CX_SPACE - CX_SPACE;
	const UINT CY_MAP = CY_MINIROOM * CX_MAP / CX_MINIROOM;
	static const int X_MAP = CX_SPACE;
	const int Y_MAP = Y_RESTORE_BUTTON - CY_MAP - CY_SPACE - 6;
	static const int X_CHOOSE_ROOM_LABEL = X_MAP;
	static const UINT CX_CHOOSE_ROOM_LABEL = 160;
	static const int CY_CHOOSE_ROOM_LABEL = CY_STANDARD_BUTTON;
	const int Y_CHOOSE_ROOM_LABEL = Y_MAP - CY_CHOOSE_ROOM_LABEL - 1;
	static const UINT CX_LEVEL_START = CX_ROOM_START;
	const int X_LEVEL_START = X_MAP + CX_MAP - CX_LEVEL_START - 2;
	const int Y_LEVEL_START = Y_CHOOSE_ROOM_LABEL;
	static const int CY_LEVEL_START = CY_CHOOSE_ROOM_LABEL;

	static const int X_CHOOSE_LEVEL_LABEL = CX_SPACE;
	static const int Y_CHOOSE_LEVEL_LABEL = Y_CHOOSE_POS_LABEL;
	static const UINT CX_CHOOSE_LEVEL_LABEL = 160;
	static const UINT CY_CHOOSE_LEVEL_LABEL = CY_STANDARD_BUTTON;
	static const UINT CX_GAME_START = CX_ROOM_START;
	static const UINT CY_GAME_START = CY_CHOOSE_LEVEL_LABEL;
	static const int X_GAME_START = X_MAP + CX_MAP - CX_GAME_START - 2;
	static const int Y_GAME_START = Y_CHOOSE_LEVEL_LABEL;
	static const int X_LEVEL_LBOX = X_CHOOSE_LEVEL_LABEL;
	static const int Y_LEVEL_LBOX = Y_CHOOSE_LEVEL_LABEL + CY_CHOOSE_LEVEL_LABEL + 1;
	static const UINT CX_LEVEL_LBOX = CX_MAP;
	static const UINT CY_LEVEL_LBOX = Y_CHOOSE_ROOM_LABEL - Y_CHOOSE_LEVEL_LABEL -
			CY_CHOOSE_LEVEL_LABEL - CY_SPACE - 1;

	static const int X_LEVEL_EXPLORED_LABEL = X_MAP;
	static const UINT CX_LEVEL_EXPLORED_LABEL = 80;
	static const int Y_LEVEL_EXPLORED_LABEL = Y_MAP + CY_MAP;
	const UINT CY_LEVEL_EXPLORED_LABEL = 25;
	static const int X_LEVEL_EXPLORED = X_LEVEL_EXPLORED_LABEL + CX_LEVEL_EXPLORED_LABEL;
	static const UINT CX_LEVEL_EXPLORED = 50;
	static const int Y_LEVEL_EXPLORED = Y_LEVEL_EXPLORED_LABEL;
	const UINT CY_LEVEL_EXPLORED = CY_LEVEL_EXPLORED_LABEL;

	static const int X_LEVEL_SECRET_LABEL = X_LEVEL_EXPLORED + CX_LEVEL_EXPLORED;
	static const UINT CX_LEVEL_SECRET_LABEL = X_MAP + CX_MAP - X_LEVEL_SECRET_LABEL;
	static const int Y_LEVEL_SECRET_LABEL = Y_LEVEL_EXPLORED_LABEL;
	const UINT CY_LEVEL_SECRET_LABEL = CY_LEVEL_EXPLORED_LABEL;

	static const int X_HOLD_STATS_LABEL = X_LEVEL_EXPLORED_LABEL;
	static const UINT CX_HOLD_STATS_LABEL = CX_MAP;
	static const int Y_HOLD_STATS_LABEL = Y_LEVEL_EXPLORED_LABEL + CY_LEVEL_EXPLORED;
	const UINT CY_HOLD_STATS_LABEL = 25;

	CButtonWidget *pButton;

	//Title.
	CLabelWidget *pTitle = new CLabelWidget(0L, 0, Y_TITLE,
			this->w, CY_TITLE, F_Title, g_pTheDB->GetMessageText(MID_RestoreGame));
	pTitle->SetAlign(CLabelWidget::TA_CenterGroup);
	AddWidget(pTitle);

	//Restore, cancel and help buttons.
	pButton = new CButtonWidget(TAG_RESTORE, X_RESTORE_BUTTON, Y_RESTORE_BUTTON, 
				CX_RESTORE_BUTTON, CY_RESTORE_BUTTON, g_pTheDB->GetMessageText(MID_Restore));
	AddWidget(pButton);

	pButton = new CButtonWidget(TAG_CANCEL, X_CANCEL_BUTTON, Y_CANCEL_BUTTON,
				CX_CANCEL_BUTTON, CY_CANCEL_BUTTON, g_pTheDB->GetMessageText(MID_Cancel));
	AddWidget(pButton);

	pButton = new CButtonWidget(TAG_HELP, X_HELP_BUTTON, Y_HELP_BUTTON, 
				CX_HELP_BUTTON, CY_HELP_BUTTON, g_pTheDB->GetMessageText(MID_Help));
	AddWidget(pButton);
	AddHotkey(SDLK_F1,TAG_HELP);

	//Level selection area.
	AddWidget(new CLabelWidget(0L, X_CHOOSE_LEVEL_LABEL, Y_CHOOSE_LEVEL_LABEL, 
				CX_CHOOSE_LEVEL_LABEL, CY_CHOOSE_LEVEL_LABEL, F_Header,
				g_pTheDB->GetMessageText(MID_ChooseLevel)));
	pButton = new CButtonWidget(TAG_HOLD_START, X_GAME_START, Y_GAME_START, 
				CX_GAME_START, CY_GAME_START, g_pTheDB->GetMessageText(MID_HoldStart));
	AddWidget(pButton);

	this->pLevelListBoxWidget = new CListBoxWidget(TAG_LEVEL_LBOX,
			X_LEVEL_LBOX, Y_LEVEL_LBOX, CX_LEVEL_LBOX, CY_LEVEL_LBOX);
	AddWidget(this->pLevelListBoxWidget);

	//Room selection area.
	AddWidget(new CLabelWidget(0L, X_CHOOSE_ROOM_LABEL, Y_CHOOSE_ROOM_LABEL, 
				CX_CHOOSE_ROOM_LABEL, CY_CHOOSE_ROOM_LABEL, F_Header, 
				g_pTheDB->GetMessageText(MID_ChooseRoom)));
	pButton = new CButtonWidget(TAG_LEVEL_START, X_LEVEL_START, Y_LEVEL_START, 
				CX_LEVEL_START, CY_LEVEL_START, g_pTheDB->GetMessageText(MID_LevelStart));
	AddWidget(pButton);

	CScrollableWidget *pScrollingMap = new CScrollableWidget(0, X_MAP, Y_MAP,
			CX_MAP, CY_MAP);
	AddWidget(pScrollingMap);
	this->pMapWidget = new CMapWidget(TAG_MAP, 0, 0,
			CDrodBitmapManager::DISPLAY_COLS, CDrodBitmapManager::DISPLAY_ROWS, NULL);
	pScrollingMap->AddWidget(this->pMapWidget);

	//Level stats.
	AddWidget(new CLabelWidget(0L, X_LEVEL_EXPLORED_LABEL, Y_LEVEL_EXPLORED_LABEL,
				CX_LEVEL_EXPLORED_LABEL, CY_LEVEL_EXPLORED_LABEL, F_Small,
				g_pTheDB->GetMessageText(MID_Complete)));
	AddWidget(new CLabelWidget(TAG_LEVEL_EXPLORED, X_LEVEL_EXPLORED, Y_LEVEL_EXPLORED,
				CX_LEVEL_EXPLORED, CY_LEVEL_EXPLORED, F_Small, wszQuestionMark));
	AddWidget(new CLabelWidget(TAG_LEVEL_SECRETS, X_LEVEL_SECRET_LABEL, Y_LEVEL_SECRET_LABEL,
				CX_LEVEL_SECRET_LABEL, CY_LEVEL_SECRET_LABEL, F_Small,
				g_pTheDB->GetMessageText(MID_SecretsFound)));
	AddWidget(new CLabelWidget(TAG_SAVED_GAME_STATS, X_HOLD_STATS_LABEL, Y_HOLD_STATS_LABEL,
				CX_HOLD_STATS_LABEL, CY_HOLD_STATS_LABEL, F_Small, wszQuestionMark));

	//Position selection area.
	AddWidget(new CLabelWidget(0L, X_CHOOSE_POS_LABEL, Y_CHOOSE_POS_LABEL,
				CX_CHOOSE_POS_LABEL, CY_CHOOSE_POS_LABEL, F_Header,
				g_pTheDB->GetMessageText(MID_ChoosePosition)));
	pButton = new CButtonWidget(TAG_ROOM_START, X_ROOM_START, Y_ROOM_START,
				CX_ROOM_START, CY_ROOM_START, g_pTheDB->GetMessageText(MID_RoomStart));
	AddWidget(pButton);

	AddWidget(new CLabelWidget(TAG_POSITION_LABEL, X_POSITION_LABEL, Y_POSITION_LABEL, 
				CX_POSITION_LABEL, CY_POSITION_LABEL, F_Small, wszEmpty));

	this->pScaledRoomWidget = new CScalerWidget(TAG_ROOM_WIDGET, X_MINIROOM, Y_MINIROOM, 
			CX_MINIROOM, CY_MINIROOM, false);
	AddWidget(this->pScaledRoomWidget);
	this->pRoomWidget = new CRoomWidget(0L, 0, 0, CDrodBitmapManager::CX_ROOM,
			CDrodBitmapManager::CY_ROOM);
	this->pRoomWidget->SetAnimateMoves(false);
	this->pScaledRoomWidget->AddScaledWidget(this->pRoomWidget);
}

//*****************************************************************************
void CRestoreScreen::ClearState()
{
	this->Checkpoints.clear();
	delete this->pCurrentRestoreGame;
	this->pCurrentRestoreGame = NULL;
}

//******************************************************************************
CRestoreScreen::~CRestoreScreen()
{
	ClearState();
}

//******************************************************************************
bool CRestoreScreen::SetForActivate()
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
	if (!this->bResetWidgets)
		this->bResetWidgets = true;	//reset next time, by default
	else
	{
		//Get widgets and current games ready.
		if (!SetWidgets())
			return false;

		SelectFirstWidget(false);
	}

	return true;
}

//
//Private methods.
//

//******************************************************************************
void CRestoreScreen::OnClick(
//Called when widget receives a click event.
//
//Params:
	const UINT dwTagNo) //(in)   Widget that event applied to.
{
	switch (dwTagNo)
	{
		case TAG_ESCAPE:
		case TAG_CANCEL:
			GoToScreen(SCR_Return);
		break;

		case TAG_RESTORE:
			RestoreGame();
		break;

		case TAG_HELP:
			CBrowserScreen::SetPageToLoad("restore.html");
			GoToScreen(SCR_Browser);
		break;

		case TAG_HOLD_START:
		{
			//For currently selected hold.
			this->pLevelListBoxWidget->SelectItem(
					this->pCurrentRestoreGame->pHold->dwLevelID);
			ChooseLevelStart(this->pCurrentRestoreGame->pHold->dwLevelID);
			Paint();
		}
		break;

		case TAG_LEVEL_START:
			ChooseLevelStart(this->pCurrentRestoreGame->pLevel->dwLevelID);
			Paint();
		break;

		case TAG_ROOM_START:
			ChooseRoomStart(this->pCurrentRestoreGame->pRoom->dwRoomX,
					this->pCurrentRestoreGame->pRoom->dwRoomY);
			Paint();
		break;      

		default:
			if (IS_CHECKPOINT_TAG(dwTagNo))
			{
				ChooseCheckpoint(dwTagNo);
				SelectWidget(TAG_RESTORE);
				this->pScaledRoomWidget->Paint();
				this->pMapWidget->RequestPaint();
			}
		break;
	}  //...switch (dwActionNo)
}

//*****************************************************************************
void CRestoreScreen::OnDoubleClick(const UINT dwTagNo)
{
	switch (dwTagNo)
	{
		case TAG_ROOM_WIDGET:
			RestoreGame();
		break;
	}
}

//*****************************************************************************
void CRestoreScreen::OnKeyDown(
//Called when widget receives SDL_KEYDOWN event.
//
//Params:
	const UINT dwTagNo,       //(in)   Widget that event applied to.
	const SDL_KeyboardEvent &Key) //(in)   Key event.
{
	CScreen::OnKeyDown(dwTagNo, Key);

	switch (Key.keysym.sym)
	{
		case SDLK_F6:
		{
			if (!this->pCurrentRestoreGame) break;
			CScreen *pScreen = g_pTheSM->GetScreen(SCR_Demos);
			if (!pScreen)
			{
				ShowOkMessage(MID_CouldNotLoadResources);
				break;
			}
			CDemosScreen *pDemosScreen = DYN_CAST(CDemosScreen*, CScreen*, pScreen);
			ASSERT(pDemosScreen);

			pDemosScreen->ShowRoom(this->pCurrentRestoreGame->pRoom->dwRoomID);
			GoToScreen(SCR_Demos);

			this->bResetWidgets = false;	//keep current room active on return
		}
		break;

		default: break;
	}
}

//*****************************************************************************
void CRestoreScreen::OnSelectChange(
//Handles a selection change.
//
//Params:
	const UINT dwTagNo) //(in) Widget event applies to.
{
	switch (dwTagNo)
	{
		case TAG_LEVEL_LBOX:
			ChooseLevelLatest(this->pLevelListBoxWidget->GetSelectedItem());
			Paint();
		break;

		case TAG_MAP:
		{
			UINT dwRoomX, dwRoomY;
			this->pMapWidget->RequestPaint();
			this->pMapWidget->GetSelectedRoomXY(dwRoomX, dwRoomY);
			ChooseRoomLatest(dwRoomX, dwRoomY);
			Paint();
		}
		break;
	}
}

//*****************************************************************************
bool CRestoreScreen::SetWidgets()
//Set up widgets and data used by them when user first arrives at restore
//screen.  Should only be called by SetForActivate().
//
//Returns:
//True if successful, false if not.
{
	bool bSuccess = true;

	//Update level selection list box.
	PopulateLevelListBoxFromSavedGames();

	//Delete any existing current game for this screen.
	delete this->pCurrentRestoreGame;
	this->pCurrentRestoreGame = NULL;

	//Load current room and level from game screen if it has a game loaded.
	CCueEvents Ignored;
	CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_Game));
	ASSERT(pGameScreen);
	const CCurrentGame *pCurrentGame = pGameScreen->GetCurrentGame(); //Delete not needed.
	if (pCurrentGame)
	{
		this->dwLastGameID = g_pTheDB->SavedGames.FindByLevelLatest(
				pCurrentGame->pLevel->dwLevelID);
		if (!this->dwLastGameID) {bSuccess=false; goto Cleanup;}
		this->pCurrentRestoreGame = g_pTheDB->GetSavedCurrentGame(
				this->dwLastGameID, Ignored, false,
				true); //don't save anything to DB during playback
		if (!this->pCurrentRestoreGame) {bSuccess=false; goto Cleanup;}
		this->wConqueredRooms = this->pCurrentRestoreGame->ConqueredRooms.size();
		this->pCurrentRestoreGame->SetRoomStatusFromAllSavedGames();

		ChooseRoomLatest(pCurrentGame->pRoom->dwRoomX, pCurrentGame->pRoom->dwRoomY);
	}
	else //I couldn't get current game from game screen.
	{
		//Load game from the continue slot.
		this->dwLastGameID = g_pTheDB->SavedGames.FindByContinue();
		this->pCurrentRestoreGame = g_pTheDB->GetSavedCurrentGame(
				this->dwLastGameID, Ignored, false,
				true); //don't save anything to DB during playback
		if (!this->pCurrentRestoreGame)
		{
			//No continue slot yet, load from beginning of game.
			this->pCurrentRestoreGame = g_pTheDB->GetNewCurrentGame(g_pTheDB->GetHoldID(), Ignored);
			if (!this->pCurrentRestoreGame) {bSuccess=false; goto Cleanup;}
			this->pCurrentRestoreGame->eType = ST_Continue;
			this->pCurrentRestoreGame->Update();
		}
		this->dwSelectedSavedGameID = this->pCurrentRestoreGame->dwSavedGameID;
		this->wConqueredRooms = this->pCurrentRestoreGame->ConqueredRooms.size();

		UpdateWidgets();

		//Update room label.
		WSTRING wstrDesc;
		this->pCurrentRestoreGame->pRoom->GetLevelPositionDescription(wstrDesc);
		CLabelWidget *pRoomLabel = DYN_CAST(CLabelWidget*, CWidget*,
				GetWidget(TAG_POSITION_LABEL));
		pRoomLabel->SetText(wstrDesc.c_str());

		//Put buttons over the room corresponding to saved games.
		SetCheckpoints();
		ShowCheckpointButtonsForSavedGame(this->dwSelectedSavedGameID);
	}

	//Select level from the list box.
	this->pLevelListBoxWidget->SelectItem(this->pCurrentRestoreGame->pLevel->dwLevelID);
	GetLevelStats(this->pCurrentRestoreGame->pLevel->dwLevelID);

Cleanup:
	return bSuccess;
}

//*****************************************************************************
void CRestoreScreen::ShowCheckpointButtonsForSavedGame(
//Hides checkpoint button corresponding to a saved game and shows all the rest.
//
//Params:
	const UINT dwSavedGameID) //(in)
{
	for (CHECKPOINT_LIST::iterator iSeek = this->Checkpoints.begin();
			iSeek != this->Checkpoints.end(); ++iSeek)
	{
		CHECKPOINT *pCheckpoint = &*iSeek;
		if (pCheckpoint->dwSavedGameID == dwSavedGameID)
			pCheckpoint->pButton->Hide();
		else
			pCheckpoint->pButton->Show();
	}
}

//*****************************************************************************
void CRestoreScreen::SetCheckpoints()
//Adds and removes widgets so that a checkpoint button exists over each 
//checkpoint in the room widget with a corresponding saved game.
{
	//Remove all current checkpoint buttons.
	for (CHECKPOINT_LIST::iterator iSeek = this->Checkpoints.begin();
			iSeek != this->Checkpoints.end(); ++iSeek)
		this->pScaledRoomWidget->RemoveWidget((*iSeek).pButton);
	this->Checkpoints.clear();

	CDb db;
	const UINT dwCurrentPlayerID = g_pTheDB->GetPlayerID();
	ASSERT(dwCurrentPlayerID);
	db.SavedGames.FilterByRoom(this->pCurrentRestoreGame->pRoom->dwRoomID);
	db.SavedGames.FilterByPlayer(dwCurrentPlayerID);

	//Each iteration looks at one saved game in the room.
	UINT dwTagNo = TAG_CHECKPOINT;
	CDbSavedGame *pSavedGame = db.SavedGames.GetFirst();
	while (pSavedGame)
	{
		//Is it a checkpoint saved game?
		if (pSavedGame->eType == ST_Checkpoint) 
		{
			//Yes--add a new button for it.
			CButtonWidget *pCheckpointButton;
			SDL_Rect SquareRect;
			SDL_Rect ScaledRoomRect;
			this->pScaledRoomWidget->GetRect(ScaledRoomRect);

			this->pRoomWidget->GetSquareRect(pSavedGame->wCheckpointX, 
					pSavedGame->wCheckpointY, SquareRect);
			SquareRect.x = this->pScaledRoomWidget->GetScaledX(SquareRect.x) - 
					ScaledRoomRect.x - 5;
			SquareRect.y = this->pScaledRoomWidget->GetScaledY(SquareRect.y) -
					ScaledRoomRect.y - 5;
			SquareRect.w = this->pScaledRoomWidget->GetScaledW(SquareRect.w) + 15;
			SquareRect.h = this->pScaledRoomWidget->GetScaledH(SquareRect.h) + 15;

			static const WCHAR wszX[] = {We('x'),We(0)};
			pCheckpointButton = new CButtonWidget(dwTagNo, SquareRect.x, SquareRect.y,
					SquareRect.w, SquareRect.h, wszX);
			this->pScaledRoomWidget->AddWidget(pCheckpointButton, true);
			++dwTagNo;
			
			//Add button to checkpoint button list.
			if (!pCheckpointButton)
			{
				ASSERT(!"No checkpoint button.");
			}
			else
			{
				CHECKPOINT sNew = { pCheckpointButton, pSavedGame->dwSavedGameID };
				this->Checkpoints.push_back(sNew);
			}
		}
		delete pSavedGame;
		pSavedGame = db.SavedGames.GetNext();
	}
}

//*****************************************************************************
void CRestoreScreen::ChooseCheckpoint(
//Chooses saved game from a checkpoint.
//
//Params:
	const UINT dwTagNo) //(in)   Tag# of pressed checkpoint button.
{
	//Find checkpoint info.
	const CHECKPOINT *pCheckpoint = NULL;
	CHECKPOINT_LIST::const_iterator iSeek;
	for (iSeek = this->Checkpoints.begin(); iSeek != this->Checkpoints.end(); ++iSeek)
	{
		pCheckpoint = &*iSeek;
		if (pCheckpoint->pButton->GetTagNo() == dwTagNo) break; //Found it.
	}
	if (iSeek == this->Checkpoints.end()) return; //No match.

	ChooseRoomSavedGame(pCheckpoint->dwSavedGameID);
}

//*****************************************************************************
void CRestoreScreen::ChooseRoomLatest(
//Chooses latest saved game for room in the current level and updates display.
//
//Params:
	const UINT dwRoomX, const UINT dwRoomY) //(in) Coords of chosen room.
{
	//Find the room.
	const UINT dwRoomID = g_pTheDB->Rooms.FindIDAtCoords(
			this->pCurrentRestoreGame->pLevel->dwLevelID, dwRoomX, dwRoomY);
	if (dwRoomID)
	{
		//Find the saved game.
		const UINT dwSavedGameID = g_pTheDB->SavedGames.FindByRoomLatest(dwRoomID);
		if (!dwSavedGameID) return;
		
		ChooseRoomSavedGame(dwSavedGameID);
	}
}

//*****************************************************************************
void CRestoreScreen::ChooseRoomStart(
//Chooses room start saved game for room in the current level and updates display.
//
//Params:
	const UINT dwRoomX, const UINT dwRoomY) //(in) Coords of chosen room.
{
	//Find the room.
	const UINT dwRoomID = g_pTheDB->Rooms.FindIDAtCoords(
			this->pCurrentRestoreGame->pLevel->dwLevelID, dwRoomX, dwRoomY);
	if (dwRoomID)
	{
		//Find the saved game.
		const UINT dwSavedGameID = g_pTheDB->SavedGames.FindByRoomBegin(dwRoomID);
		if (!dwSavedGameID) return;
		
		ChooseRoomSavedGame(dwSavedGameID);
	}
}

//*****************************************************************************
void CRestoreScreen::ChooseRoomSavedGame(
//Chooses a room in the current level and updates display.
//
//Params:
	const UINT dwSavedGameID) //(in) Saved game to use.
{
	ASSERT(dwSavedGameID);

	SetCursor(CUR_Wait);

	//Load the saved game.
	CCueEvents Ignored;
	CCurrentGame *pNewGame = g_pTheDB->GetSavedCurrentGame(dwSavedGameID, Ignored, false,
			true); //don't save anything to DB here
	if (pNewGame)
	{
		//Switch current game over to new one from saved game.
		if (this->pCurrentRestoreGame) delete this->pCurrentRestoreGame;
		this->pCurrentRestoreGame = pNewGame;
		this->dwSelectedSavedGameID = dwSavedGameID;

		UpdateWidgets();
		this->pMapWidget->SelectRoom(this->pCurrentRestoreGame->pRoom->dwRoomX,
				this->pCurrentRestoreGame->pRoom->dwRoomY);
						
		//Update room label.
		WSTRING wstrDesc;
		this->pCurrentRestoreGame->pRoom->GetLevelPositionDescription(wstrDesc);
		CLabelWidget *pRoomLabel = DYN_CAST(CLabelWidget*, CWidget*,
				GetWidget(TAG_POSITION_LABEL));
		pRoomLabel->SetText(wstrDesc.c_str());
		
		//Put buttons over the room corresponding to saved games.
		SetCheckpoints();
		ShowCheckpointButtonsForSavedGame(dwSavedGameID);
	}

	SetCursor();
}

//*****************************************************************************
void CRestoreScreen::ChooseLevelStart(
//Choose a level start saved game and updates display.
//
//Params:
	const UINT dwLevelID)  //(in) Level to load saved game for.
{
	ASSERT(dwLevelID);

	//Find the level start saved game.
	const UINT dwSavedGameID = g_pTheDB->SavedGames.FindByLevelBegin(dwLevelID);
	if (!dwSavedGameID) return;

	ChooseLevelSavedGame(dwSavedGameID);
}

//*****************************************************************************
void CRestoreScreen::ChooseLevelLatest(
//Choose latest saved game in a level and updates display.
//
//Params:
	const UINT dwLevelID)  //(in) Level to load saved game for.
{
	ASSERT(dwLevelID);

	//Don't update when selecting the same level.
	if (dwLevelID == this->pCurrentRestoreGame->pLevel->dwLevelID) return;

	//Find the latest saved game on the level.
	const UINT dwSavedGameID = g_pTheDB->SavedGames.FindByLevelLatest(dwLevelID);
	if (!dwSavedGameID) return;

	GetLevelStats(dwLevelID);

	ChooseLevelSavedGame(dwSavedGameID);
}

//*****************************************************************************
void CRestoreScreen::ChooseLevelSavedGame(
//Chooses a level and updates display.
//
//Params:
	const UINT dwSavedGameID) //(in) Saved game to load.
{
	ASSERT(dwSavedGameID);

	//Load the saved game.
	CCueEvents Ignored;
	CCurrentGame *pNewGame = g_pTheDB->GetSavedCurrentGame(dwSavedGameID, Ignored, false,
			true); //don't save anything to DB here
	if (pNewGame)
	{
		//Switch current game over to new one from saved game.
		if (this->pCurrentRestoreGame) delete this->pCurrentRestoreGame;
		this->pCurrentRestoreGame = pNewGame;
		const UINT dwRoomSavedGameID = dwSavedGameID;
		if (!dwRoomSavedGameID) {ASSERT(!"Bad saved game ID."); return;}
		this->dwSelectedSavedGameID = dwSavedGameID;

		UpdateWidgets();

		//Update room label.
		WSTRING wstrDesc;
		this->pCurrentRestoreGame->pRoom->GetLevelPositionDescription(wstrDesc);
		CLabelWidget *pRoomLabel = DYN_CAST(CLabelWidget*, CWidget*,
				GetWidget(TAG_POSITION_LABEL));
		pRoomLabel->SetText(wstrDesc.c_str());

		//Put buttons over the room corresponding to saved games.
		SetCheckpoints();
		ShowCheckpointButtonsForSavedGame(dwSavedGameID);
	}
}

//*****************************************************************************
void CRestoreScreen::GetLevelStats(const UINT dwLevelID)
//Show player's level completion stats.
//More information is displayed after hold is completed.
{
	const UINT dwPlayerID = g_pTheDB->GetPlayerID();
	CIDSet roomsInLevel, playerRoomsExploredInLevel;
	g_pTheDB->Levels.GetRoomsExplored(dwLevelID, dwPlayerID,
			roomsInLevel, playerRoomsExploredInLevel);

	//Display percent of rooms in level explored.
	WCHAR num[10];
	WSTRING wstr;
	CLabelWidget *pStatsLabel =
			DYN_CAST(CLabelWidget*, CWidget*, GetWidget(TAG_LEVEL_EXPLORED));
	ASSERT(pStatsLabel);
	if (!this->bHoldConquered)
	{
		_itoW(playerRoomsExploredInLevel.size(), num, 10);
		wstr = num;
	} else {
		_itoW(static_cast<int>(playerRoomsExploredInLevel.size() * 100.0 /
				(float)roomsInLevel.size()), num, 10);
		wstr = num;
		wstr += wszPercent;
	}
	pStatsLabel->SetText(wstr.c_str());

	//Display number of these rooms explored or conquered.
	pStatsLabel = DYN_CAST(CLabelWidget*, CWidget*, GetWidget(TAG_LEVEL_SECRETS));
	ASSERT(pStatsLabel);
	wstr = g_pTheDB->GetMessageText(
			this->bHoldConquered ? MID_SecretsConquered : MID_SecretsFound);
	wstr += wszSpace;

	UINT dwSecretRooms, dwSecretRoomsDone;
	if (this->bHoldConquered)
	{
		//Tally only conquered secret rooms once hold is finished.
		g_pTheDB->Levels.GetRoomsExplored(dwLevelID, dwPlayerID,
				roomsInLevel, playerRoomsExploredInLevel, true);
	}
	g_pTheDB->Levels.GetSecretRoomsInSet(roomsInLevel, playerRoomsExploredInLevel,
			dwSecretRooms, dwSecretRoomsDone);
	_itoW(dwSecretRoomsDone, num, 10);
	wstr += num;
	if (this->bHoldConquered)
	{
		//When hold is finished, reveal how many secret rooms are in this level.
		wstr += wszForwardSlash;
		_itoW(dwSecretRooms, num, 10);
		wstr += num;
	}
	pStatsLabel->SetText(wstr.c_str());
}

//*****************************************************************************
void CRestoreScreen::UpdateWidgets()
//Update the map and room widgets to reflect the current game.
//Update the map widget to show rooms which are not explored in the saved game.
{
	//Update the map and room widgets with new current game.
	VERIFY(this->pRoomWidget->LoadFromCurrentGame(this->pCurrentRestoreGame));
	VERIFY(this->pMapWidget->LoadFromCurrentGame(this->pCurrentRestoreGame, false));

	//Set conquered/explored status of rooms from all saved games in the level.
	CIDSet CurrentExploredRooms = this->pCurrentRestoreGame->ExploredRooms;
	this->pCurrentRestoreGame->SetRoomStatusFromAllSavedGames();

	CIDSet DarkenedRooms = this->pCurrentRestoreGame->ExploredRooms;
	DarkenedRooms -= CurrentExploredRooms;
	this->pMapWidget->SetDarkenedRooms(DarkenedRooms);

	CLabelWidget *pStatsLabel =	DYN_CAST(CLabelWidget*, CWidget*, GetWidget(TAG_SAVED_GAME_STATS));
	ASSERT(pStatsLabel);

	std::ostringstream str;
	str << " " << this->pCurrentRestoreGame->ConqueredRooms.size() << " / " << CurrentExploredRooms.size();
	WSTRING wstr = g_pTheDB->GetMessageText(MID_TotalRoomStats), wtemp;
	AsciiToUnicode(str.str().c_str(), wtemp);
	wstr += wtemp;
	
	pStatsLabel->SetText(wstr.c_str());
}

//*****************************************************************************
void CRestoreScreen::Paint(
//Paint the screen.
//
//Params:
	bool bUpdateRect)             //(in)   If true (default) and destination
										//    surface is the screen, the screen
										//    will be immediately updated in
										//    the widget's rect.
{
	SDL_BlitSurface(this->images[0], NULL, GetDestSurface(), NULL);

	PaintChildren();
	if (bUpdateRect) UpdateRect();
}

//*****************************************************************************
void CRestoreScreen::PopulateLevelListBoxFromSavedGames()
//Put levels into list box that have at least one saved game associated with
//them.  Determine whether this hold has been completed.
{
	BEGIN_DBREFCOUNT_CHECK;

	this->pLevelListBoxWidget->Clear();

	{
		//Get the hold.
		CDbHold *pHold = g_pTheDB->Holds.GetByID(g_pTheDB->GetHoldID());
		if (!pHold) {ASSERT(!"Failed to retrieve hold."); return;} //Probably corrupted DB.

		//Check for first level.
		CIDSet levelsInHold = CDb::getLevelsInHold(pHold->dwHoldID);
		if (levelsInHold.empty())
		{
			delete pHold; //Hold doesn't have any levels.
			return;
		}

		//Add level IDs containing level start saved games in the current hold.
		SORTED_LEVELS levels;
		for (CIDSet::const_iterator levelID = levelsInHold.begin(); levelID != levelsInHold.end(); ++levelID)
		{
			CDbLevel *pLevel = g_pTheDB->Levels.GetByID(*levelID);
			ASSERT(pLevel);
			if (g_pTheDB->SavedGames.FindByLevelBegin(pLevel->dwLevelID))
				levels.insert(pLevel);
			else
				delete pLevel;
		}

		//Display levels in sorted order.
		for (SORTED_LEVELS::const_iterator level = levels.begin(); level != levels.end(); ++level)
		{
			this->pLevelListBoxWidget->AddItem((*level)->dwLevelID, (*level)->NameText);
			delete *level;
		}

		this->bHoldConquered = (g_pTheDB->SavedGames.FindByEndHold(pHold->dwHoldID) != 0);

		delete pHold;
	}

	END_DBREFCOUNT_CHECK;
}

//*****************************************************************************
void CRestoreScreen::RestoreGame()
{
	if (!this->dwSelectedSavedGameID) return;

	if (this->dwLastGameID)
	{
		//If the level being restored to is the same as the one
		//for the current game or continue slot, prompt the player
		//if a saved game with fewer conquered rooms is being selected.
		CDbSavedGame *pSavedGame = g_pTheDB->SavedGames.GetByID(
				this->dwLastGameID);
		if (pSavedGame)
		{
			CDbRoom *pRoom = g_pTheDB->Rooms.GetByID(pSavedGame->dwRoomID, true);
			delete pSavedGame;
			ASSERT(pRoom);
			if (pRoom->dwLevelID == this->pCurrentRestoreGame->pRoom->dwLevelID)
			{
				if (this->wConqueredRooms >
						this->pCurrentRestoreGame->ConqueredRooms.size())
				{
					if (ShowYesNoMessage(MID_UnconqueredRooms) != TAG_YES)
					{
						delete pRoom;
						return;
					}
				}
			}
			delete pRoom;
		} //else: the "last" saved game could be for an empty continue slot,
		//in which case it shouldn't be compared against.
	}
	CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_Game));
	ASSERT(pGameScreen);
	pGameScreen->LoadSavedGame(this->dwSelectedSavedGameID);
	if (pGameScreen->ShouldShowLevelStart())
		GoToScreen(SCR_LevelStart);
	else
		GoToScreen(SCR_Game);
}
