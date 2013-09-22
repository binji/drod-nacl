// $Id: GameScreen.cpp 10064 2012-04-02 18:16:52Z TFMurphy $

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
 * Richard Cookney (timeracer), JP Burford (jpburford), John Wm. Wicks (j_wicks),
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#include "GameScreen.h"
#include "BrowserScreen.h"
#include "DemosScreen.h"
#include "EditRoomScreen.h"
#include "DrodBitmapManager.h"
#include "DrodFontManager.h"
#include "DrodScreenManager.h"
#include "DrodSound.h"
#include "TileImageCalcs.h"

#include "BloodEffect.h"
#include "CheckpointEffect.h"
#include "DebrisEffect.h"
#include "EvilEyeGazeEffect.h"
#include "ExplosionEffect.h"
#include "SparkEffect.h"
#include "SplashEffect.h"
#include "SteamEffect.h"
#include "StrikeOrbEffect.h"
#include "SwordsmanSwirlEffect.h"
#include "SwordSwingEffect.h"
#include "TarStabEffect.h"
#include "TrapdoorFallEffect.h"
#include "VerminEffect.h"
#include "WadeEffect.h"

#include "ClockWidget.h"
#include "FaceWidget.h"
#include "MapWidget.h"
#include "RoomWidget.h"
#include "RoomEffectList.h"
#include <FrontEndLib/Fade.h>
#include <FrontEndLib/BumpObstacleEffect.h>
#include <FrontEndLib/ButtonWidget.h>
#include <FrontEndLib/DialogWidget.h>
#include <FrontEndLib/ExpandingTextEffect.h>
#include <FrontEndLib/FlashMessageEffect.h>
#include <FrontEndLib/FrameWidget.h>
#include <FrontEndLib/LabelWidget.h>
#include <FrontEndLib/ListBoxWidget.h>
#include <FrontEndLib/MenuWidget.h>
#include <FrontEndLib/TextBoxWidget.h>
#include <FrontEndLib/TextEffect.h>
#include <FrontEndLib/TransTileEffect.h>
#include "../Texts/MIDs.h"

#include "../DRODLib/Character.h"
#include "../DRODLib/CueEvents.h"
#include "../DRODLib/Db.h"
#include "../DRODLib/DbPlayers.h"
#include "../DRODLib/DbRooms.h"
#include "../DRODLib/DbXML.h"
#include "../DRODLib/GameConstants.h"
#include "../DRODLib/Mimic.h"
#include "../DRODLib/Monster.h"
#include "../DRODLib/MonsterFactory.h"
#include "../DRODLib/MonsterPiece.h"
#include "../DRODLib/TileConstants.h"
#include <BackEndLib/Assert.h>
#include <BackEndLib/Clipboard.h>
#include <BackEndLib/CoordIndex.h>
#include <BackEndLib/CoordStack.h>
#include <BackEndLib/Files.h>
#include <BackEndLib/IDList.h>
#include <BackEndLib/Wchar.h>
#include <BackEndLib/Ports.h>

//#define ENABLE_CHEATS

const UINT MAX_REPEAT_RATE = 50;

const UINT TAG_CLOCK = 1010;
const UINT TAG_MENUPROMPT = 1011;
const UINT TAG_MENUFRAME = 1012;
const UINT TAG_MENU = 1013;
const UINT TAG_FACE = 1014;
const UINT TAG_BIGMAPCONTAINER = 1015;
const UINT TAG_BIGMAP = 1016;

const UINT TAG_ESC = 1020;
const UINT TAG_HELP = 1021;
const UINT TAG_ONLINE_HELP = 1022;
const UINT TAG_OPEN_STATSBOX = 1023;

const UINT TAG_STATSBOX = 1030;
const UINT TAG_GAMESTATSFRAME = 1031;
const UINT TAG_MOVES = 1032;
const UINT TAG_KILLS = 1033;
const UINT TAG_GAMESTATS = 1034;
const UINT TAG_CHATENABLE = 1035;
const UINT TAG_CHATWHISPERSONLY = 1036;
const UINT TAG_CHATINPUT = 1037;
const UINT TAG_CHATUSERS = 1038;

const UINT TAG_UNDO_FROM_QUESTION = UINT(-9); //unused value

const UINT CX_SPACE = 12;
const UINT CY_SPACE = 12;

UINT wMood = SONG_AMBIENT;
int nDangerLevel = 0;	//for setting room mood

typedef map<ROOMCOORD, vector<UINT> > TilesMap;

const SURFACECOLOR lockColor = {255, 255, 128};
const SDL_Color Red = {196, 0, 0, 0}; //cutscene sign text color

//
//CGameScreen public methods.
//

//*****************************************************************************
bool CGameScreen::LoadContinueGame()
//Loads current game from current player's continue saved game slot.
//
//Returns:
//True if successful, false if not.
{
	//Load the game.
	const UINT dwContinueID = g_pTheDB->SavedGames.FindByContinue();
	if (!dwContinueID) return false;
	return LoadSavedGame(dwContinueID);
}

//*****************************************************************************
bool CGameScreen::LoadSavedGame(
//Loads current game from a saved game.
//
//Params:
	const UINT dwSavedGameID, //(in)   Saved game to load.
	bool bRestoreFromStart, //(in)   If true, game will be restored from start
							//    without playing back commands.  Default is
							//    false.
	const bool bNoSaves) //[default=false]
//
//Returns:
//True if successful, false if not.
{
	//Get rid of current game if needed.
	DeleteCurrentGame();

	//Load the game.
	this->bIsSavedGameStale = false;
	ClearCueEvents();
	this->pCurrentGame = g_pTheDB->GetSavedCurrentGame(dwSavedGameID,
			this->sCueEvents, bRestoreFromStart, bNoSaves);
	if (!this->pCurrentGame)
		return false;

	this->bPlayTesting = false;
	this->wForbidUndoBeforeTurn = this->wUndoToTurn = this->pCurrentGame->wTurnNo;
	this->bRoomClearedOnce = this->pCurrentGame->IsCurrentRoomPendingExit();

	SetSignTextToCurrentRoom();
	return true;
}

//*****************************************************************************
bool CGameScreen::LoadNewGame(
//Loads current game from beginning of specified hold.
//
//Returns:
//True if successful, false if not.
//
//Params:
	const UINT dwHoldID)   //(in)
{
	//Get rid of current game if needed.
	DeleteCurrentGame();

	//Load the game.
	this->bIsSavedGameStale = false;
	ClearCueEvents();
	this->pCurrentGame = g_pTheDB->GetNewCurrentGame(dwHoldID, this->sCueEvents);
	if (!this->pCurrentGame) return false;

	//Set current game for widgets.
	if (!this->pMapWidget->LoadFromCurrentGame(this->pCurrentGame) ||
			!this->pRoomWidget->LoadFromCurrentGame(this->pCurrentGame))
		return false;

	this->bPlayTesting = false;
	this->wForbidUndoBeforeTurn = this->wUndoToTurn = 0;
	this->bRoomClearedOnce = this->pCurrentGame->IsCurrentRoomPendingExit();

	SetSignTextToCurrentRoom();
	return true;
}

//*****************************************************************************
bool CGameScreen::ShouldShowLevelStart()
//Used by screen-changing code outside of CGameScreen to determine if the
//level start screen should be shown before the game screen.
//
//Returns:
//True if it should, false if not.
{
	if (this->bIsSavedGameStale)
		return false;

	const bool bShowLevelStart = this->pCurrentGame->ShowLevelStart();
	if (bShowLevelStart)
		//I only want the caller to go to the level start screen once for each
		//time a saved game is loaded.  The save game is now "stale" until
		//one of the saved game loading methods of this class is called.  Then
		//it will set the flag to false again.
		this->bIsSavedGameStale = true;

	return bShowLevelStart;
}

//*****************************************************************************
//Call to set whether cue events fired on this turn will freeze the music mood
bool CGameScreen::IsMusicStyleFrozen(CCueEvents& CueEvents) const
{
	const CAttachableObject *pObj = CueEvents.GetFirstPrivateData(CID_SetMusic);
	if (!pObj)
		return false;

	const CCoord *pMusicID = dynamic_cast<const CCoord*>(pObj);
	if (pMusicID && pMusicID->wX == UINT(SONGID_DEFAULT))
		return false;

	//Any other custom music setting freezes music
	return true;
}

//*****************************************************************************
void CGameScreen::SetMusicStyle()
//Changes the music to match room style, according to game mood.
//If music style already matches the current style, nothing will happen.
{
	if (this->bMusicStyleFrozen)
		return;

	//Determine game mood.
	ASSERT(this->pCurrentGame);
	ASSERT(this->pCurrentGame->pRoom);
	nDangerLevel = this->pCurrentGame->pRoom->DangerLevel();
	wMood = !nDangerLevel ? SONG_AMBIENT :
			nDangerLevel < 30 ? SONG_PUZZLE : SONG_ATTACK;

	CRoomScreen::SetMusicStyle(this->pCurrentGame->pRoom->style, wMood);
}

//*****************************************************************************
bool CGameScreen::TestRoom(
//Returns: whether room is successfully loaded for testing
//
//Params:
	const UINT dwRoomID,   //(in) room to start in
	const UINT wX, const UINT wY, const UINT wO) //(in) Starting position
{
	//Get rid of current game if needed.
	DeleteCurrentGame();

	//Load the game.
	this->bIsSavedGameStale = false;
	ClearCueEvents();
	this->pCurrentGame = g_pTheDB->GetNewTestGame(dwRoomID, this->sCueEvents, wX, wY, wO,
			true); //don't save to DB while testing
	if (!this->pCurrentGame) return false;

	//Set current game for widgets.
	if (!this->pMapWidget->LoadFromCurrentGame(this->pCurrentGame) ||
			!this->pRoomWidget->LoadFromCurrentGame(this->pCurrentGame))
		return false;

	this->bPlayTesting = true;
	this->wForbidUndoBeforeTurn = this->wUndoToTurn = 0;
	this->bRoomClearedOnce = this->pCurrentGame->IsCurrentRoomPendingExit();
	this->bShowingCutScene = false;

	SetSignTextToCurrentRoom();
	return true;
}

//*****************************************************************************
bool CGameScreen::UnloadGame()
//Deletes the current game.
//
//Returns true if successful.
{
	LOGCONTEXT("CGameScreen::UnloadGame");
	if (this->pCurrentGame)
	{
		//Save current game to continue slot, unless this is CGameScreen-derived
		//CDemoScreen, or after play-testing.
		if (GetScreenType() == SCR_Game && !this->bPlayTesting &&
				g_pTheDB->GetHoldID() != g_pTheDB->Holds.GetHoldIDWithStatus(CDbHold::Tutorial))
		{
			this->pCurrentGame->SaveToContinue();
			g_pTheDB->Commit();
		}

		//Free current game.
		DeleteCurrentGame();
	}
	this->pRoomWidget->ResetRoom();
	ClearCueEvents(); //so no cues play on reentering screen

	return true;
}

//
//CGameScreen protected methods.
//

//*****************************************************************************
CGameScreen::CGameScreen(const SCREENTYPE eScreen) : CRoomScreen(eScreen)
	, pCurrentGame(NULL)
	, pRoomWidget(NULL)

	, bShowLevelStartBeforeActivate(false)
	, bPersistentEventsDrawn(false)
	, bNeedToProcessDelayedQuestions(false)
	, bShowingBigMap(false), bShowingCutScene(false)

	, pFaceWidget(NULL)
	, pClockWidget(NULL)
	, pMenuDialog(NULL)
	, pSpeechBox(NULL)
	, pBigMapWidget(NULL)

	, dwNextSpeech(0)
	, bShowingSubtitlesWithVoice(true)
	, dwTimeMinimized(0), dwLastCutSceneMove(0), dwSavedMoveDuration(0)

	, bIsSavedGameStale(false)
	, bRoomClearedOnce(false)
	, bMusicStyleFrozen(false)
	, wUndoToTurn(0), wForbidUndoBeforeTurn(0)

	, fPos(NULL)

	, wUploadingDemoHandle(0)
	, dwUploadingDemo(0)

	, dwTimeInRoom(0), dwLastTime(0)
	, dwTotalPlayTime(0)
//Constructor.
{
	this->fPos = new float[3];
	this->fPos[0] = this->fPos[1] = this->fPos[2] = 0.0;

	static const int X_ROOM = 163;
	static const int Y_ROOM = 40;
	static const int X_FACE = 15;
	static const int Y_FACE = 15;
	static const int X_CLOCK = 4;
	static const int Y_CLOCK = 413;

#ifdef RUSSIAN_BUILD
	static const int X_ESC = 15;
	static const int Y_ESC = 718;
	static const UINT CX_ESC = 150;
	static const UINT CY_ESC = 30;

	static const int X_HELP = 30;
	static const int Y_HELP = 740;
	static const UINT CX_HELP = CX_ESC;
	static const UINT CY_HELP = 30;
#else
	static const int X_ESC = 6;
	static const int Y_ESC = 726;
	static const UINT CX_ESC = 86;
	static const UINT CY_ESC = 32;

	static const int X_HELP = 96;
	static const int Y_HELP = Y_ESC;
	static const UINT CX_HELP = 64;
	static const UINT CY_HELP = 32;
#endif

	static const UINT CX_OPENSTATS_BUTTON = 30;
	static const UINT CY_OPENSTATS_BUTTON = 27;
	static const int X_OPENSTATS_BUTTON = 1024 - CX_OPENSTATS_BUTTON - 10;
	static const int Y_OPENSTATS_BUTTON = 5;

	static const UINT CX_MENUBOX = 800;
	static const UINT CY_MENUBOX = 400;
	static const int X_TEXT = CX_SPACE;
	static const int Y_TEXT = CY_SPACE;
	static const UINT CX_TEXT = CX_MENUBOX - (X_TEXT * 2);
	static const UINT CY_TEXT = CY_MENUBOX - (Y_TEXT * 2) - CY_SPACE;

	static const int X_MENU = 0;
	static const int Y_MENU = 0;
	static const int CX_MENU = CX_TEXT;
	static const int CY_MENU = CY_MENUBOX;

	//Pop-up map
	static const UINT BIGMAP_MARGIN = 100;
	const UINT CX_BIGMAP = CDrodBitmapManager::CX_ROOM - 2*BIGMAP_MARGIN;
	const UINT CY_BIGMAP = CDrodBitmapManager::CY_ROOM - 2*BIGMAP_MARGIN;

	//Add widgets.
	this->pRoomWidget = new CRoomWidget(TAG_ROOM, X_ROOM, Y_ROOM,
			CDrodBitmapManager::CX_ROOM, CDrodBitmapManager::CY_ROOM);
	AddWidget(this->pRoomWidget);

	this->pFaceWidget = new CFaceWidget(TAG_FACE, X_FACE, Y_FACE, CX_FACE, CY_FACE);
	AddWidget(this->pFaceWidget);

	this->pClockWidget = new CClockWidget(TAG_CLOCK, X_CLOCK, Y_CLOCK, CX_CLOCK, CY_CLOCK);
	AddWidget(this->pClockWidget);

	CLabelWidget *pLabel = new CLabelWidget(TAG_ESC, X_ESC, Y_ESC, CX_ESC, CY_ESC, F_ButtonWhite,
			g_pTheDB->GetMessageText(MID_EscMenu));
	pLabel->SetClickable(eScreen != SCR_Demo);
	AddWidget(pLabel);

	if (eScreen != SCR_Demo)
	{
		this->pMapWidget->Enable();
		this->pMapWidget->bUserMoveable = false;

		pLabel = new CLabelWidget(TAG_HELP, X_HELP, Y_HELP, CX_HELP, CY_HELP, F_ButtonWhite,
				g_pTheDB->GetMessageText(MID_F1Help));
		pLabel->SetClickable(true);
		AddWidget(pLabel);

		CButtonWidget *pOpenStatsButton = new CButtonWidget(TAG_OPEN_STATSBOX,
				X_OPENSTATS_BUTTON, Y_OPENSTATS_BUTTON, CX_OPENSTATS_BUTTON, CY_OPENSTATS_BUTTON,
				wszQuestionMark);
		pOpenStatsButton->SetFocusAllowed(false);
		AddWidget(pOpenStatsButton);

		//Menu dialog.
		this->pMenuDialog = new CDialogWidget(0L, 0, 0, CX_MENUBOX, CY_MENUBOX);
		this->pMenuDialog->Hide();
		AddWidget(this->pMenuDialog);

		CLabelWidget *pLabel = new CLabelWidget(TAG_MENUPROMPT, X_TEXT, Y_TEXT,
				CX_TEXT, CY_TEXT, FONTLIB::F_Message, wszEmpty);
		this->pMenuDialog->AddWidget(pLabel);

		CFrameWidget *pFrame = new CFrameWidget(TAG_MENUFRAME, X_TEXT - 3, Y_TEXT - 3,
				CX_TEXT + 6, CY_TEXT + 6, NULL);
		pFrame->Disable();
		this->pMenuDialog->AddWidget(pFrame);

		CMenuWidget *pMenu = new CMenuWidget(TAG_MENU, X_MENU, Y_MENU, CX_MENU, CY_MENU,
				F_Hyperlink, F_ActiveHyperlink, F_ExtHyperlink);
		this->pMenuDialog->AddWidget(pMenu);

		//Level list dialog box.
		this->pSpeechBox = new CEntranceSelectDialogWidget(0L);
		AddWidget(this->pSpeechBox);
		this->pSpeechBox->Move(
			X_ROOM + (CDrodBitmapManager::CX_ROOM - this->pSpeechBox->GetW()) / 2,
			Y_ROOM + (CDrodBitmapManager::CY_ROOM - this->pSpeechBox->GetH()) / 2);   //center over room widget
		this->pSpeechBox->Hide();

		//Pop-up map.
		CScrollableWidget *pScrollingMap = new CScrollableWidget(TAG_BIGMAPCONTAINER, 0, 0,
				CX_BIGMAP, CY_BIGMAP);
		pScrollingMap->Hide();
		this->pRoomWidget->AddWidget(pScrollingMap);
		pScrollingMap->Center();
		this->pBigMapWidget = new CMapWidget(TAG_BIGMAP, 0, 0,
				CDrodBitmapManager::DISPLAY_COLS, CDrodBitmapManager::DISPLAY_ROWS, NULL);
		this->pBigMapWidget->bUserMoveable = false;
		this->pBigMapWidget->Hide();
		pScrollingMap->AddWidget(this->pBigMapWidget);

		AddRoomStatsDialog();
	}
}

//******************************************************************************
CGameScreen::~CGameScreen()
{
	delete[] this->fPos;
	UnloadGame();
}

//*****************************************************************************
void CGameScreen::AddRoomStatsDialog()
//Show dialog box displaying current game stats.
{
	static const UINT CX_SPACE = 10;
	static const UINT CY_SPACE = 10;

	static const UINT CX_DIALOG = 610;
	static const UINT CY_DIALOG = 610;

	//Room stats.
	static const int Y_HEADER = 15;
	static const int X_HEADER = 20;
	static const UINT CX_HEADER = CX_DIALOG - 2*X_HEADER;
	static const UINT CY_HEADER = 36;

	static const UINT CY_TEXT_LABEL = 40;
	static const int X_TEXT_LABEL = 5;
	static const int X_VALUE_LABEL = 200;
	static const UINT CX_TEXT_LABEL = X_VALUE_LABEL - X_TEXT_LABEL;
	static const UINT CX_VALUE_LABEL = 85;

	static const int Y_TITLE = 5;
	static const UINT CX_TITLE = X_VALUE_LABEL + CX_VALUE_LABEL - X_HEADER;
	static const UINT CY_TEXT_H = 30;
	static const int Y_THISROOM = Y_TITLE + CY_SPACE/2;
	static const int Y_MOVES = Y_THISROOM + CY_TEXT_H;
	static const int Y_KILLS = Y_MOVES + CY_TEXT_H;
	static const int Y_ROOMS = Y_KILLS + CY_TEXT_H;
	static const UINT CY_ROOMS = 260;

	static const int Y_FRAME = Y_HEADER + CY_TEXT_LABEL + CY_SPACE;
	static const UINT CY_FRAME = Y_ROOMS + CY_ROOMS;

	//Online help.
	static const int X_ONLINEHELP_BUTTON = 60;
	static const UINT CX_ONLINEHELP_BUTTON = 175;
	static const UINT CY_ONLINEHELP_BUTTON = CY_STANDARD_BUTTON;
	static const int Y_ONLINEHELP_BUTTON = Y_FRAME + CY_FRAME + CY_SPACE * 2;

	//Chat.
	static const int X_CHATOPTION = 295;
	static const int Y_CHATOPTION = Y_FRAME + CY_FRAME;
	static const UINT CY_CHATOPTION = CY_STANDARD_OPTIONBUTTON;
	static const UINT CX_CHATOPTION = CX_DIALOG - X_CHATOPTION - CX_SPACE;
	static const int X_CHATWHISPEROPTION = X_CHATOPTION;
	static const int Y_CHATWHISPEROPTION = Y_CHATOPTION + CY_CHATOPTION + CY_SPACE;
	static const UINT CX_CHATWHISPEROPTION = CX_CHATOPTION;

	static const int X_CHATLABEL = 20;
	static const int Y_CHATLABEL = Y_CHATWHISPEROPTION + CY_SPACE * 2;
	static const UINT CX_CHATLABEL = CX_DIALOG - X_CHATLABEL*2;
	static const UINT CY_CHATLABEL = 28;

	static const int X_CHATINPUT = X_CHATLABEL;
	static const int Y_CHATINPUT = Y_CHATLABEL + CY_CHATLABEL;
	static const UINT CX_CHATINPUT = CX_CHATLABEL;

	static const int X_USERLIST = X_VALUE_LABEL + CX_VALUE_LABEL + CX_SPACE;
	static const int Y_USERLISTLABEL = Y_HEADER + CY_HEADER;
	static const UINT CY_USERLISTLABEL = 27;
	static const int Y_USERLIST = Y_USERLISTLABEL + CY_USERLISTLABEL;
	static const UINT CX_USERLIST = CX_DIALOG - X_USERLIST - X_HEADER;
	static const UINT CY_USERLIST = 14*22 + 4;

	static const UINT CX_OK_BUTTON = 80;

	CDialogWidget *pStatsBox = new CDialogWidget(TAG_STATSBOX, 0, 0, CX_DIALOG, CY_DIALOG);

	CLabelWidget *pLabel = new CLabelWidget(0L, X_HEADER, Y_HEADER, CX_HEADER,
			CY_HEADER, F_Header, g_pTheDB->GetMessageText(MID_CurrentGameStats));
	pLabel->SetAlign(CLabelWidget::TA_CenterGroup);
	pStatsBox->AddWidget(pLabel);

	//Game/hold info.
	CFrameWidget *pFrame = new CFrameWidget(TAG_GAMESTATSFRAME, X_HEADER - 5, Y_FRAME,
			CX_TITLE + 5, CY_FRAME, wszEmpty);
	pStatsBox->AddWidget(pFrame);

	//Room info.
	WSTRING text = g_pTheDB->GetMessageText(MID_StatsBoxThisRoom);
	text += wszColon;
	pFrame->AddWidget(
			new CLabelWidget(0L, X_TEXT_LABEL, Y_THISROOM, CX_TEXT_LABEL, CY_TEXT_LABEL,
					F_Message, text.c_str()));
	pFrame->AddWidget(
			new CLabelWidget(0L, X_TEXT_LABEL, Y_MOVES, CX_TEXT_LABEL, CY_TEXT_LABEL,
					F_Message, g_pTheDB->GetMessageText(MID_MovesMade)));
	pFrame->AddWidget(
			new CLabelWidget(0L, X_TEXT_LABEL, Y_KILLS, CX_TEXT_LABEL, CY_TEXT_LABEL,
					F_Message, g_pTheDB->GetMessageText(MID_MonstersKilled)));

	//Moves made and monsters killed in room.
	pFrame->AddWidget(
			new CLabelWidget(TAG_MOVES, X_VALUE_LABEL, Y_MOVES, CX_VALUE_LABEL, CY_TEXT_LABEL,
					F_Message, wszEmpty));
	pFrame->AddWidget(
			new CLabelWidget(TAG_KILLS, X_VALUE_LABEL, Y_KILLS, CX_VALUE_LABEL, CY_TEXT_LABEL,
					F_Message, wszEmpty));
	//Current game stats.
	pFrame->AddWidget(
			new CLabelWidget(TAG_GAMESTATS, X_TEXT_LABEL, Y_ROOMS, CX_TITLE, CY_ROOMS,
					F_Message, wszEmpty));

	//Online help.
	CButtonWidget *pOnlineHelpButton = new CButtonWidget(
			TAG_ONLINE_HELP, X_ONLINEHELP_BUTTON, Y_ONLINEHELP_BUTTON, CX_ONLINEHELP_BUTTON, CY_ONLINEHELP_BUTTON,
			g_pTheDB->GetMessageText(MID_OnlineHelp));
	pStatsBox->AddWidget(pOnlineHelpButton);

	//Chat.
	pStatsBox->AddWidget(new COptionButtonWidget(TAG_CHATENABLE, X_CHATOPTION, Y_CHATOPTION,
			CX_CHATOPTION, CY_CHATOPTION, g_pTheDB->GetMessageText(MID_ChatEnableOnGameScreen)));
	pStatsBox->AddWidget(new COptionButtonWidget(TAG_CHATWHISPERSONLY, X_CHATWHISPEROPTION, Y_CHATWHISPEROPTION,
			CX_CHATWHISPEROPTION, CY_CHATOPTION, g_pTheDB->GetMessageText(MID_ReceiveWhispersOnly)));
	pStatsBox->AddWidget(new CLabelWidget(0, X_CHATLABEL, Y_CHATLABEL,
			CX_CHATLABEL, CY_CHATLABEL, F_FrameCaption, g_pTheDB->GetMessageText(MID_ChatTitle)));
	pStatsBox->AddWidget(new CTextBoxWidget(TAG_CHATINPUT, X_CHATINPUT, Y_CHATINPUT,
			CX_CHATINPUT, CY_STANDARD_TBOX));
	pStatsBox->AddWidget(new CLabelWidget(0, X_USERLIST, Y_USERLISTLABEL,
			CX_USERLIST, CY_USERLISTLABEL, F_FrameCaption, g_pTheDB->GetMessageText(MID_PlayersChatting)));
	pStatsBox->AddWidget(new CListBoxWidget(TAG_CHATUSERS, X_USERLIST, Y_USERLIST,
			CX_USERLIST, CY_USERLIST, false, false, true));

	//Buttons.
	CButtonWidget *pButton = new CButtonWidget(TAG_OK,
			(CX_DIALOG - CX_OK_BUTTON) / 2, CY_DIALOG - CY_STANDARD_BUTTON - CY_SPACE*2,
			CX_OK_BUTTON, CY_STANDARD_BUTTON, g_pTheDB->GetMessageText(MID_Okay));
	pStatsBox->AddWidget(pButton);

	this->pRoomWidget->AddWidget(pStatsBox,true);
	pStatsBox->Center();
	pStatsBox->Hide();
}

//*****************************************************************************
void CGameScreen::AddSoundEffect(const VisualEffectInfo* pEffect)
//Add a sound effect to the room.
//wX/Y: sound location, wValue2: play sound or not
{
	const CMoveCoordEx2& effect = pEffect->effect;
	const bool bSound = effect.wValue2 != 0;

	if (bSound)
	{
		UINT soundID=SEID_NONE;
		switch (effect.wValue)
		{
			case VET_BLOODSPLAT: case VET_SEEPSPLAT: soundID = SEID_SPLAT; break;
			case VET_MUDSPLAT: case VET_TARSPLAT: case VET_GELSPLAT: soundID = SEID_STABTAR; break;
			case VET_SLAYERSPLAT: soundID = SEID_SLAYER_DIE; break;
			case VET_GOLEMSPLAT: case VET_DEBRIS: soundID = SEID_BREAKWALL; break;
			case VET_SPARKS: soundID = SEID_STARTFUSE; break;
			case VET_EXPLOSION: soundID = SEID_BOMBEXPLODE; break;
			case VET_SPLASH: soundID = SEID_SPLASH; break;
			case VET_STEAM: soundID = SEID_SIZZLE; break;
			case VET_BOLT: soundID = SEID_ORBHITQUIET; break;
			default: break;
		}

		if (soundID != (UINT)SEID_NONE)
			g_pTheSound->PlaySoundEffect(soundID);
	}
}

//*****************************************************************************
void CGameScreen::ApplyINISettings()
//(Re)query the INI for current values and apply them.
{
	CDrodScreen::ApplyINISettings();

	//Set game replay speed optimization.
	string str;
	if (CFiles::GetGameProfileString("Customizing", "MaxDelayForUndo", str))
		this->pCurrentGame->SetComputationTimePerSnapshot(atol(str.c_str()));

	//Force room style reload.
	this->pRoomWidget->UpdateFromCurrentGame(true);
}

bool CGameScreen::CanShowVarUpdates() {
	return
		this->bPlayTesting ||
		(this->pCurrentGame && this->pCurrentGame->pHold && (
			g_pTheNet->IsBetaHold(this->pCurrentGame->pHold->dwHoldID) ||
			this->pCurrentGame->pHold->PlayerCanEdit(g_pTheDB->GetPlayerID()))
		);
}

//*****************************************************************************
void CGameScreen::ChatPolling(const UINT tagUserList)
//Override CDrodScreen's method.
{
	//During playtesting, a temporary player profile is used that doesn't
	//have the player's CaravelNet user settings.
	//So, when processing CaravelNet transactions, set the active player
	//profile to the real user for the duration of the transaction.
	CEditRoomScreen *pEditRoomScreen = NULL;
	if (this->bPlayTesting)
	{
		pEditRoomScreen = DYN_CAST(CEditRoomScreen*, CScreen*,
				g_pTheSM->GetScreen(SCR_EditRoom));
		ASSERT(pEditRoomScreen);
		g_pTheDB->SetPlayerID(pEditRoomScreen->GetSavePlayerID(), false); //don't relogin to CaravelNet
	}

	CDrodScreen::ChatPolling(tagUserList);

	if (this->bPlayTesting)
	{
		ASSERT(pEditRoomScreen);
		g_pTheDB->SetPlayerID(pEditRoomScreen->GetTestPlayerID(), false);
	}
}

//******************************************************************************
void CGameScreen::ClearCueEvents()
//Clears static instance of the cue events.
{
	this->sCueEvents.Clear();
}

//******************************************************************************
void CGameScreen::DisplayChatText(const WSTRING& text, const SDL_Color& color)
{
	this->pRoomWidget->DisplayChatText(text, color);
}

//******************************************************************************
void CGameScreen::DrawCurrentTurn()
//Redraws everything to show the current game state.
{
	UpdateSound();

	//Refresh pointers but don't reload front-end resources.
	this->pRoomWidget->LoadFromCurrentGame(this->pCurrentGame, false);

	this->bPersistentEventsDrawn = this->bNeedToProcessDelayedQuestions = false;
	const SCREENTYPE eNextScreen = ProcessCueEventsBeforeRoomDraw(this->sCueEvents);
	ASSERT(eNextScreen == SCR_Game);

	this->pRoomWidget->DontAnimateMove();
	this->pRoomWidget->Paint();

	ProcessCueEventsAfterRoomDraw(this->sCueEvents);
	this->pFaceWidget->Paint();
	this->pMapWidget->DrawMapSurfaceFromRoom(this->pCurrentGame->pRoom);
	this->pMapWidget->RequestPaint();
	SynchScroll();
	PaintScroll();
	PaintClock();
}

//******************************************************************************
void CGameScreen::LogHoldVars()
//Output the current state of all hold vars to clipboard and a text file.
{
	ASSERT(this->pCurrentGame);

	string strPos;
	WSTRING wstrPos;
	char temp[16];
	bool bFirst = true;

	wstrPos = (const WCHAR *)this->pCurrentGame->pLevel->NameText;
	wstrPos += wszColon;
	this->pCurrentGame->pRoom->GetLevelPositionDescription(wstrPos, true);
	wstrPos += wszColon;
	UnicodeToAscii(wstrPos, strPos);

	string str = "Game vars ";
	str += strPos;

	for (UNPACKEDVAR *pVar = this->pCurrentGame->stats.GetFirst();
			pVar != NULL; pVar = this->pCurrentGame->stats.GetNext())
	{
		//Skip non-vars (i.e. not of format "v<varID>".
		if (pVar->name[0] != 'v')
			continue;

		if (bFirst)
		{
			str += NEWLINE;
			bFirst = false;
		}
		const UINT wVarID = atoi(pVar->name.c_str() + 1); //skip the "v"
		string tempStr;
		UnicodeToAscii(this->pCurrentGame->pHold->GetVarName(wVarID), tempStr);
		str += tempStr;
		str += ": ";
		const bool bInteger = pVar->eType == UVT_int;
		if (bInteger)
		{
			const int nVal = this->pCurrentGame->stats.GetVar(pVar->name.c_str(), (int)0);
			str += _itoa(nVal, temp, 10);
		} else {
			WSTRING wstr = this->pCurrentGame->stats.GetVar(pVar->name.c_str(), wszEmpty);
			string tempStr2;
			UnicodeToAscii(wstr, tempStr2);
			str += tempStr2;
		}
		str += NEWLINE;
	}
	if (bFirst) //no vars were encountered
		str += " None set" NEWLINE;
	CClipboard::SetString(str);
	CFiles f;
	f.AppendUserLog(str.c_str());
}

//******************************************************************************
bool CGameScreen::SetForActivate()
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
	//Caller didn't load a current game.
	if (!this->pCurrentGame) {ASSERT(!"Current game not set."); return false;}
	this->bMusicStyleFrozen = false;

	//Set current game for widgets.
	if (!this->pMapWidget->LoadFromCurrentGame(this->pCurrentGame) ||
			!this->pRoomWidget->LoadFromCurrentGame(this->pCurrentGame))
		return false;

	UpdateSound();
	this->pRoomWidget->AllowSleep(false);
	this->pRoomWidget->ClearEffects();
	if (!CanShowVarUpdates())
		this->pRoomWidget->ShowVarUpdates(false);
	this->pMapWidget->DrawMapSurfaceFromRoom(this->pCurrentGame->pRoom);
	SetSignTextToCurrentRoom();
	this->pFaceWidget->SetMood(Mood_Normal);
	SynchScroll();
	PaintClock(true);

	//Init the keysym-to-command map and load other player settings.
	ApplyPlayerSettings();
	this->dwTimeInRoom = this->dwLastTime = 0;

	//Set frame rate as high as needed for smooth animations.
	SetBetweenEventsInterval(15); //60+ fps

	SetMusicStyle();
	SwirlEffect();
	ClearSpeech();
	SetGameAmbience(true);
	AmbientSoundSetup();

	SelectFirstWidget(false);

	this->pCurrentGame->UpdateTime(SDL_GetTicks());
	ShowLockIcon(this->pCurrentGame->bRoomExitLocked);

	this->pRoomWidget->DontAnimateMove();

	//Never return to the restore screen.  Yank it out of the return list so
	//that we go back to the title screen instead.
	if (g_pTheSM->GetReturnScreenType() == SCR_Restore)
		g_pTheSM->RemoveReturnScreen();

	//Whenever entering the game screen,
	//remind the player if the level requirements have been completed.
	const bool bRestoringGame = GetScreenType() == SCR_Game &&
			g_pTheSM->GetReturnScreenType() == SCR_Title;
	if (this->pCurrentGame->IsCurrentLevelComplete() && bRestoringGame)
		this->sCueEvents.Add(CID_CompleteLevel);

	this->signColor = Black;

	//When coming from another screen, cue events might have been generated that
	//need to be displayed when returning to this screen.
	//Examples of this are the "exit level!" or "secret room!" cues that should
	//display on level entrance, or persistent effects like burning fuses.
	//
	//Also, the level/hold might be exited on the first move.
	//In this event, just show the exit event stuff here and then decide whether
	//to continue activating the game screen.
	this->bPersistentEventsDrawn = this->bNeedToProcessDelayedQuestions = false;
	ProcessSpeechCues(this->sCueEvents);
	SCREENTYPE eNextScreen = ProcessCueEventsAfterRoomDraw(this->sCueEvents);
	if (eNextScreen == SCR_Game)
		if (ProcessExitLevelEvents(this->sCueEvents, eNextScreen))
		{
			//Transition to specified screen instead of the game screen.
			if (eNextScreen != SCR_Game)
				g_pTheDSM->InsertReturnScreen(eNextScreen);
		}

	this->chat.SetRefreshInterval(20000); //20s automatic refresh
	this->chat.refreshNow();

	//Eat existing events since early key presses could cause unexpected player movement.
	ClearEvents();

	//If the game says to go to a different screen, then don't stay on this screen.
	return eNextScreen == SCR_Game;
}

//*****************************************************************************
void CGameScreen::SetGameAmbience(const bool bRecalc) //[default=false]
//Sets various ambient effects based on game state.
{
	//Beethro can go to sleep if room is peaceful/static and no dialog is occurring.
	if (this->pCurrentGame)
	{
		if (bRecalc)
			nDangerLevel = this->pCurrentGame->pRoom->DangerLevel();

		ASSERT(this->pRoomWidget);
		UINT wX, wY;
		this->pCurrentGame->GetSwordsman(wX, wY);
		const bool bValidTile = this->pCurrentGame->pRoom->IsValidColRow(wX, wY) &&
				this->pCurrentGame->pRoom->GetOSquare(wX, wY) != T_HOT;
		this->pRoomWidget->AllowSleep(this->speech.empty() && !nDangerLevel &&
				bIsSmitemaster(this->pCurrentGame->swordsman.wAppearance) &&
				!this->pCurrentGame->dwCutScene &&
				this->pCurrentGame->pRoom->briars.empty() &&
				this->pCurrentGame->pRoom->LitFuses.empty() &&
				!this->pCurrentGame->swordsman.wPlacingDoubleType &&
				bValidTile &&
				GetScreenType() != SCR_Demo);
	}
}

//*****************************************************************************
void CGameScreen::OnActiveEvent(const SDL_ActiveEvent &Active)
{
	CEventHandlerWidget::OnActiveEvent(Active);

	if (this->pCurrentGame)
	{
		this->dwTotalPlayTime += this->pCurrentGame->UpdateTime(SDL_GetTicks()); //add time until app activity change

		const Uint8 state = SDL_GetAppState();
		const bool bActive = (state & SDL_APPACTIVE) == SDL_APPACTIVE;
		if (!bActive)
		{
			//When app is minimized or w/o focus, pause game time and speech.
			this->pCurrentGame->UpdateTime();
			if (!this->dwTimeMinimized)
				this->dwTimeMinimized = SDL_GetTicks();
		} else {
			//Unpause speech.
			if (this->dwTimeMinimized)
			{
				if (this->dwNextSpeech)
					this->dwNextSpeech += SDL_GetTicks() - this->dwTimeMinimized;
				this->dwTimeMinimized = 0;
			}
		}
	}
}

//*****************************************************************************
void CGameScreen::OnBetweenEvents()
//Called between frames.
{
	UploadDemoPolling();

	if (this->bShowingBigMap)
		return;

	if (this->bEnableChat)
		ChatPolling(TAG_CHATUSERS);

	//Continue displaying animated room and new chat messages received
	//while the chat/status dialog is visible (i.e. typing a chat message, etc).
	if (GetScreenType() == SCR_Game)
	{
		CDialogWidget *pChatBox = DYN_CAST(CDialogWidget*, CWidget*,
				this->pRoomWidget->GetWidget(TAG_STATSBOX));
		if (pChatBox->IsVisible())
		{
			this->pRoomWidget->Paint(false);
			pChatBox->Paint(false);
			this->pRoomWidget->UpdateRect();
			return; //everything below is paused while entering a chat message
		}
	}

	if (this->dwTimeMinimized)
		return; //don't do the rest while minimized

	if (this->pCurrentGame)
	{
		//Keep sign synched with current game state.
		if ((this->pCurrentGame->dwCutScene != 0) != this->bShowingCutScene)
		{
			this->bShowingCutScene = (this->pCurrentGame->dwCutScene != 0);
			SetSignTextToCurrentRoom();
			PaintSign();
		}

		//Handle question prompts that were delayed while screen was being activated.
		if (this->bNeedToProcessDelayedQuestions)
		{
			SCREENTYPE eNextScreen = SCR_Game;
			ProcessQuestionPrompts(sCueEvents, eNextScreen);
			if (eNextScreen != SCR_Game)
			{
				if (IsDeactivating())
					SetDestScreenType(eNextScreen); //override any other specified destination screen
				else
					GoToScreen(eNextScreen);
			}
			return;
		}

		if (!this->pCurrentGame->IsCutScenePlaying())
		{
			this->dwLastCutSceneMove = 0;
			//Return per-move duration to what it was before the cut scene.
			if (this->dwSavedMoveDuration)
			{
				this->pRoomWidget->SetMoveDuration(this->dwSavedMoveDuration);
				this->dwSavedMoveDuration = 0;
			}
		} else if (GetScreenType() == SCR_Game) { //don't automatically advance turns during demo playback
			//Turns pass automatically when a cut scene is playing.

			//Is it time to advance the cut scene by a game turn?
			const Uint32 dwNow = SDL_GetTicks();
			if (!this->dwLastCutSceneMove)
				this->dwLastCutSceneMove = dwNow;
			if (dwNow - this->dwLastCutSceneMove >= this->pCurrentGame->dwCutScene)
			{
				//Save per-move duration before cut scene began.
				if (!this->dwSavedMoveDuration)
				{
					this->dwSavedMoveDuration = this->pRoomWidget->GetMoveDuration();
					this->pRoomWidget->FinishMoveAnimation(); //finish last move now
				}
				//Set per-move duration so cut scene moves advance smoothly.
				this->pRoomWidget->SetMoveDuration(this->pCurrentGame->dwCutScene);

				//Advance the cut scene another turn.
				const SCREENTYPE eNextScreen = ProcessCommand(CMD_ADVANCE_CUTSCENE);
				this->dwLastCutSceneMove = dwNow;
				if (eNextScreen != SCR_Game)
				{
					if (IsDeactivating())
						SetDestScreenType(eNextScreen); //override any other specified destination screen
					else
						GoToScreen(eNextScreen);
					return;
				}
			}
		}

		//Time to play a thunder sound effect?
		if (!this->pRoomWidget->playThunder.empty())
		{
			if (SDL_GetTicks() >= this->pRoomWidget->playThunder.front())
			{
				this->pRoomWidget->playThunder.pop();
				g_pTheSound->PlaySoundEffect(SEID_THUNDER, NULL, NULL, false, 1.0f + fRAND_MID(0.2f));
			}
		}
	}

	CRoomScreen::OnBetweenEvents();
	ProcessSpeech();
}

//*****************************************************************************
void CGameScreen::OnClick(
//Called when widget receives a mouse click event.
//
//Params:
	const UINT dwTagNo) //(in) Widget that event applied to.
{
	ShowCursor();

	//Widgets that may be clicked on whether or not the level map is being shown.
	switch (dwTagNo)
	{
		case TAG_MAP:
		{
			if (!this->pCurrentGame)
				break;
			const UINT roomID = this->pCurrentGame->pLevel->GetRoomIDAtCoords(
				this->pMapWidget->dwClickedRoomX, this->pMapWidget->dwClickedRoomY);
			if (!roomID)
			{
				ToggleBigMap();
				break;
			}

			//When current room is clicked on the minimap, pop-up or hide the big level map.
			if (roomID == this->pCurrentGame->pRoom->dwRoomID)
			{
				ToggleBigMap();
			} else {
				//Show the room clicked on immediately.  No pop-up map after.

				//Don't change display in the middle of an action.
				if (!this->bNeedToProcessDelayedQuestions)
				{
					if (this->pCurrentGame->ExploredRooms.has(roomID))
						ShowRoomTemporarily(roomID);
					else ToggleBigMap();
				}
			}
		}
		break;
	}

	if (this->bShowingBigMap)
	{
		switch (dwTagNo)
		{
			case TAG_BIGMAP: //show room at this location
			{
				//Show view of the clicked room.
				const UINT roomID = this->pCurrentGame->pLevel->GetRoomIDAtCoords(
					this->pBigMapWidget->dwClickedRoomX, this->pBigMapWidget->dwClickedRoomY);
				if (!roomID)
				{
					//No room to show.
					HideBigMap();
					break;
				}
				if (roomID == this->pCurrentGame->pRoom->dwRoomID)
				{
					//Clicking on current room just returns to normal play.
					HideBigMap();
				} else {
					if (this->pCurrentGame->ExploredRooms.has(roomID))
					{
						//Show this room.
						ShowRoomTemporarily(roomID);
					} else {
						//No room to show.
						HideBigMap();
					}
				}
			}
			break;
			case TAG_MAP:
				//Was handled above.
			break;
			case TAG_BIGMAPCONTAINER:
				//Don't hide map when interacting with map widgets.
			break;
			default: //clicking elsewhere returns to normal play
				HideBigMap();
			break;
		}

		//Don't process any other input while the map is being shown.
		return;
	}

	switch (dwTagNo)
	{
		case TAG_ESC:
			//Esc -- menu
			GoToScreen(SCR_Return);
		break;
		case TAG_HELP:
			//F1 -- help
			GotoHelpPage();
		break;
		case TAG_OPEN_STATSBOX:
			OpenStatsBox();
		break;
	}
}

//*****************************************************************************
void CGameScreen::OnDeactivate()
{
	g_pTheDBM->fLightLevel = 1.0; //full light level
	this->dwTimeMinimized = 0;
	g_pTheSound->StopAllSoundEffects(); //stop any game sounds that were playing

	if (this->pCurrentGame && GetScreenType() == SCR_Game)
	{
		this->pCurrentGame->UpdateTime();   //stop timing game play

		//Save updated persistent player info.
		CDbPlayer *pCurrentPlayer = g_pTheDB->GetCurrentPlayer();
		CDbPackedVars& vars = pCurrentPlayer->Settings;

		//Total play time.
		UINT dwPreviousTotal = vars.GetVar(totalPlayTime, UINT(0));
		vars.SetVar(totalPlayTime, dwPreviousTotal + this->dwTotalPlayTime/1000); //ms -> s
		const CDbHold::HoldStatus status = this->pCurrentGame->pHold->status;
		if (status == CDbHold::TCB || status == CDbHold::Tutorial) //in TCB+tutorial only
		{
			dwPreviousTotal = vars.GetVar(TCBtotalPlayTime, UINT(0));
			vars.SetVar(TCBtotalPlayTime, dwPreviousTotal + this->dwTotalPlayTime/1000); //ms -> s
		}

		//Save chat preference.
		vars.SetVar(enableChat, this->bEnableChat);
		vars.SetVar(receiveWhispersOnly, this->bReceiveWhispersOnly);
		vars.SetVar(moveCounterStr, this->pRoomWidget->IsShowingMoveCount());

		pCurrentPlayer->Update();

		//Save to the continue slot whenever leaving the game screen.
		if (!this->bPlayTesting && this->pCurrentGame->pHold->status != CDbHold::Tutorial)
		{
			//Stop any demo recording.
			if (this->pCurrentGame->IsDemoRecording() &&
					(GetDestScreenType() == SCR_Return || GetDestScreenType() == SCR_None))
				this->pCurrentGame->EndDemoRecording();

			this->pCurrentGame->SaveToContinue();

			//Does player seem frustrated?
			//This is calculated by considering the amount of time the player
			//spends in a room before exiting the game screen.  In successful play,
			//this screen is usually left right when a player enters a new room,
			//upon completion of the previous task.  If a player quits after
			//spending time not beating a room, they are probably frustrated.
			static const Uint32 dwFrustratedTime = 150000;	//2.5 minutes
			if (this->dwTimeInRoom > dwFrustratedTime)
			{
				const bool bShowHelp = pCurrentPlayer &&
						pCurrentPlayer->Settings.GetVar("ShowHelp", true) &&
						this->pCurrentGame->pHold->status == CDbHold::GetOfficialHoldStatus() &&
						this->pCurrentGame->pHold->Entrances.size() < CDrodScreen::EntrancesInFullVersion(); //demo hold
				if (bShowHelp)
				{
					//Show hint prompt for the player only once.
					pCurrentPlayer->Settings.SetVar("ShowHelp", 0);
					pCurrentPlayer->Update();

					switch (ShowYesNoMessage(MID_NeedHelpPrompt))
					{
						case TAG_YES:
							CBrowserScreen::SetPageToLoad("advice.html");
							g_pTheSM->InsertReturnScreen(SCR_Browser);
						break;
						case TAG_NO:
							ShowOkMessage(MID_NeedHelpNo);
						break;
						default: break;
					}
				}
			}

			g_pTheDB->Commit();
		}

		delete pCurrentPlayer;

		//Ensure all internet upload requests have been sent.
		WaitToUploadDemos();
	}

	UploadExploredRooms();

	CRoomScreen::OnDeactivate();
}

//*****************************************************************************
void CGameScreen::OnDoubleClick(
//Called when widget receives a mouse double click event.
//
//Params:
	const UINT dwTagNo)   //(in) Widget that event applied to.
{
	switch (dwTagNo)
	{
		case TAG_FACE:
			ShowSpeechLog();
		break;
		default: break;
	}
}

//*****************************************************************************
void CGameScreen::OnKeyDown(
//Called when widget receives SDL_KEYDOWN event.
//
//Params:
	const UINT dwTagNo,       //(in)   Widget that event applied to.
	const SDL_KeyboardEvent &Key) //(in)   Key event.
{
#if defined (ENABLE_CHEATS)
	CCueEvents Ignored;
	bool bExitRoomForCheat = false;
#endif

	if (this->bShowingBigMap)
	{
		HideBigMap();
		return;
	}

	CScreen::OnKeyDown(dwTagNo, Key);

	//Check for a game command.
	if (Key.keysym.sym > -1 && Key.keysym.sym < SDLK_LAST)
	{
		int nCommand = this->KeysymToCommandMap[Key.keysym.sym];

		bool bMacro = (Key.keysym.mod & KMOD_CTRL) != 0; //two-move combo activation

		//Battle key indicates performing the reverse of the last movement command.
		if (nCommand == CMD_BATTLE_KEY)
		{
			bMacro = false;
			const UINT wLastCommandIndexPlusOne = this->pCurrentGame->Commands.Count();
			if (!wLastCommandIndexPlusOne)
				nCommand = CMD_UNSPECIFIED; //there is no previous command
			else {
				CDbCommands::const_iterator cmd = this->pCurrentGame->Commands.Get(
						wLastCommandIndexPlusOne-1);
				switch (cmd->bytCommand)
				{
					case CMD_NW: nCommand = CMD_SE; break;
					case CMD_N: nCommand = CMD_S; break;
					case CMD_NE: nCommand = CMD_SW; break;
					case CMD_W: nCommand = CMD_E; break;
					case CMD_E: nCommand = CMD_W; break;
					case CMD_SW: nCommand = CMD_NE; break;
					case CMD_S: nCommand = CMD_N; break;
					case CMD_SE: nCommand = CMD_NW; break;
					case CMD_C: nCommand = CMD_CC; break;
					case CMD_CC: nCommand = CMD_C; break;
					//Battle key is inoperative for other commands.
					default: nCommand = CMD_UNSPECIFIED; break;
				}
			}
		}
		if (nCommand != CMD_UNSPECIFIED)
		{
			//Hide mouse cursor while playing.
			HideCursor();

			if (nCommand == CMD_RESTART && (Key.keysym.mod & KMOD_CTRL) != 0)
				nCommand = CMD_RESTART_PARTIAL;
			if (nCommand == CMD_RESTART && (Key.keysym.mod & KMOD_ALT) != 0)
				nCommand = CMD_RESTART_FULL;

			//Only allow inputting player movement commands when no cutscene is playing.
			if (!this->pCurrentGame->IsCutScenePlaying() ||
					(nCommand >= CMD_WAIT && nCommand != CMD_EXEC_COMMAND))
			{
				SCREENTYPE eNextScreen = ProcessCommand(nCommand, bMacro);
				if (eNextScreen != SCR_Game)
				{
					if (IsDeactivating())
						SetDestScreenType(eNextScreen); //override any other specified destination screen
					else
						GoToScreen(eNextScreen);
					return;
				}
			}
		}
	}

	//Check for other keys.
	switch (Key.keysym.sym)
	{
		case SDLK_TAB:
			//Game command key.
			if (!this->bShowingBigMap)
			{
				if ((Key.keysym.mod & (KMOD_CTRL | KMOD_ALT)) == 0)
				{
					//If clone is placed, transfer to next one.
					if (this->pCurrentGame)
					{
						CMonster *pMonster = this->pCurrentGame->pRoom->FindNextClone();
						if (pMonster)
							ProcessCommand(CMD_CLONE, pMonster->wX, pMonster->wY);
					}
				}
			}
		break;

		case SDLK_F1:
			GotoHelpPage();
		break;

		case SDLK_F4:
#if defined(__linux__) || defined(__FreeBSD__)
		case SDLK_PAUSE:  case SDLK_BREAK:
#endif
		if ((Key.keysym.mod & (KMOD_CTRL | KMOD_ALT)) == 0)
		{
			if (this->bPlayTesting) break;
			if (!this->pCurrentGame->wTurnNo) break;
			//Save a demo for the current room from entrance up to the present turn.
			WSTRING wstrDescription = this->pCurrentGame->AbbrevRoomLocation();
			const UINT dwTagNo = ShowTextInputMessage(MID_DescribeDemo,
					wstrDescription);
			if (dwTagNo == TAG_OK)
			{
				this->pCurrentGame->BeginDemoRecording( (wstrDescription.size()==0) ?
						wszEmpty : wstrDescription.c_str(), false);
				ShowOkMessage(this->pCurrentGame->EndDemoRecording() ?
						MID_DemoSaved : MID_DemoNotSaved);
				PaintSign();
			}
		}
		break;

		case SDLK_F5:
			if (this->bPlayTesting) break;
			if (this->pCurrentGame->IsDemoRecording())
			{
				//End recording and save demo.
				const UINT dwTagNo = this->pCurrentGame->EndDemoRecording();
				SetSignTextToCurrentRoom();
				PaintSign();

				if (!dwTagNo)
					ShowOkMessage(MID_DemoNotSaved);
				else if (dwTagNo != TAG_ESCAPE)
					ShowOkMessage(MID_DemoSaved);
			}
			else
			{
				WSTRING wstrDescription = this->pCurrentGame->AbbrevRoomLocation();
				const UINT dwTagNo = ShowTextInputMessage(MID_DescribeDemo,
						wstrDescription);
				if (dwTagNo == TAG_OK)
				{
					this->pCurrentGame->BeginDemoRecording( (wstrDescription.size()==0) ?
							wszEmpty : wstrDescription.c_str() );

					//Repaint sign to show new recording status.
					SetSignTextToCurrentRoom();
					PaintSign();
				}
			}
		break;

		//Room demos displayed will be of active room in current game.
		case SDLK_F6:
			ASSERT(this->pCurrentGame);
			ASSERT(this->pCurrentGame->pRoom);
			ShowDemosForRoom(this->pCurrentGame->pRoom->dwRoomID);
		break;

		//Room screenshot.
		case SDLK_F11:
		if (Key.keysym.mod & KMOD_CTRL)
		{
			g_pTheSound->PlaySoundEffect(SEID_SCREENSHOT);
			SDL_Surface *pRoomSurface = SDL_CreateRGBSurface(
					SDL_SWSURFACE, this->pRoomWidget->GetW(), this->pRoomWidget->GetH(),
					g_pTheBM->BITS_PER_PIXEL, 0, 0, 0, 0);
			if (!pRoomSurface) break;
			SDL_Rect screenRect = {this->pRoomWidget->GetX(), this->pRoomWidget->GetY(),
					this->pRoomWidget->GetW(), this->pRoomWidget->GetH()};
			SDL_Rect roomRect = {0, 0, this->pRoomWidget->GetW(), this->pRoomWidget->GetH()};
			SDL_BlitSurface(GetDestSurface(), &screenRect, pRoomSurface, &roomRect);
			SaveSurface(pRoomSurface);
			SDL_FreeSurface(pRoomSurface);
		}
		break;

		//Show room/game stats.
		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			if (!(Key.keysym.mod & (KMOD_ALT|KMOD_CTRL)))
			{
				g_pTheSound->PlaySoundEffect(SEID_BUTTON);
				OpenStatsBox();
			} else if (Key.keysym.mod & (KMOD_CTRL)) {
				ShowChatHistory(this->pSpeechBox);
			}
		break;

		//Toggle room lock.
		case SDLK_LSHIFT: case SDLK_RSHIFT:
			this->pCurrentGame->bRoomExitLocked = !this->pCurrentGame->bRoomExitLocked;
			g_pTheSound->PlaySoundEffect(this->pCurrentGame->bRoomExitLocked ?
					SEID_WISP : SEID_CHECKPOINT);
			ShowLockIcon(this->pCurrentGame->bRoomExitLocked);
		break;

		//Skip cutscene/clear playing speech.
		case SDLK_SPACE:
		{
			if (this->pCurrentGame && this->pCurrentGame->IsCutScenePlaying()) {
				if (this->pCurrentGame->dwCutScene == 1)
				{
					//When speeding past a cutscene, space clears all speech and subtitles of any sort.
					ClearSpeech(true);
				} else {
					this->pCurrentGame->dwCutScene = 1; //run cutscene quickly to its conclusion
				}
			} else {
				//Clear all speech and subtitles of any sort.
				ClearSpeech(true);
				this->pRoomWidget->RemoveLastLayerEffectsOfType(ECHATTEXT);
			}
		}
		break;

		//dev keys
		case SDLK_F3:
			//Force full style reload.
			this->pRoomWidget->UpdateFromCurrentGame(true);
		break;
		//Persistent move count display / Frame rate / Game var output.
		case SDLK_F7:
#ifdef ENABLE_CHEATS
			if (Key.keysym.mod & KMOD_SHIFT)
				LogHoldVars();
			else
#endif
			if (Key.keysym.mod & KMOD_CTRL) {
#ifndef ENABLE_CHEATS
				if (CanShowVarUpdates())
#endif
					this->pRoomWidget->ToggleVarDisplay();
			} else if (Key.keysym.mod & KMOD_ALT) {
				this->pRoomWidget->ToggleFrameRate();
			} else {
				this->pRoomWidget->ToggleMoveCount();
			}
		break;

#ifdef ENABLE_CHEATS
		//cheat keys
		case SDLK_F8:
		{
			//Warp to next level entrance.
			ClearSpeech();
			const UINT wEntranceIndex = this->pCurrentGame->pHold->GetEntranceIndex(
					this->pCurrentGame->pEntrance);
			if (wEntranceIndex == (UINT)-1)
			{
				//Entrance isn't a part of the current hold.  Just restart the hold.
				ASSERT(!"Entrance doesn't belong to hold.");
				DeleteCurrentGame();
				this->pCurrentGame = g_pTheDB->GetNewCurrentGame(
						g_pTheDB->GetHoldID(), Ignored);
				this->pRoomWidget->LoadFromCurrentGame(this->pCurrentGame);
				this->pMapWidget->LoadFromCurrentGame(this->pCurrentGame);
			}
			else
			{
				//Warp to next level entrance.
				const UINT wNextEntranceIndex = wEntranceIndex+1 <
						this->pCurrentGame->pHold->Entrances.size() ? wEntranceIndex+1 : 0;
				const UINT dwOldRoomID = this->pCurrentGame->pRoom->dwRoomID;
				this->pCurrentGame->LoadFromLevelEntrance(this->pCurrentGame->pHold->dwHoldID,
						this->pCurrentGame->pHold->Entrances[wNextEntranceIndex]->dwEntranceID,
						Ignored);
				if (this->pCurrentGame->pRoom->dwRoomID != dwOldRoomID)
					this->pRoomWidget->LoadRoomImages();

				this->pRoomWidget->UpdateFromCurrentGame();
				this->pMapWidget->UpdateFromCurrentGame();
			}
			this->bMusicStyleFrozen = false;
			SetSignTextToCurrentRoom();
			SetMusicStyle();
			PaintSign();
			this->pMapWidget->RequestPaint();
			this->pRoomWidget->ClearEffects();
			this->pRoomWidget->Paint();
			ShowPlayerFace();
			this->bRoomClearedOnce = this->pCurrentGame->IsCurrentRoomPendingExit();
		}
		break;
#endif
#if defined (ENABLE_CHEATS)
		//Toggle sword.
		case SDLK_F9:
			this->pCurrentGame->swordsman.bSwordOff = !this->pCurrentGame->swordsman.bSwordOff;
		break;
		case SDLK_BACKSLASH: //conquer room
			if (!pCurrentGame->IsCurrentRoomConquered())
			{
				this->pCurrentGame->SetCurrentRoomConquered();
				this->pCurrentGame->SetRoomVisited();
				this->pCurrentGame->RestartRoom(Ignored);
				this->pRoomWidget->RenderRoomLighting();
				this->pRoomWidget->ResetForPaint();
				bExitRoomForCheat = true;
				//Kludge vars that might have been set via script when restarting the room
				this->pCurrentGame->SetPlayerRole(this->pCurrentGame->wStartRoomAppearance, Ignored);
				this->pCurrentGame->swordsman.bSwordOff = this->pCurrentGame->bStartRoomSwordOff;
				this->pCurrentGame->swordsman.wWaterTraversal = this->pCurrentGame->wStartRoomWaterTraversal;
				this->pCurrentGame->RestartRoom(Ignored); //do a second time to make sure all vars are inited properly
			}
		break;
#endif
#ifdef ENABLE_CHEATS
		case SDLK_UP: //Up arrow.
			if (pCurrentGame->SetPlayerToNorthExit())
			{
				bExitRoomForCheat = pCurrentGame->LoadNewRoomForExit(N, Ignored);
			}
		break;

		case SDLK_DOWN:   //Down arrow.
			if (pCurrentGame->SetPlayerToSouthExit())
			{
				bExitRoomForCheat = pCurrentGame->LoadNewRoomForExit(S, Ignored);
			}
		break;

		case SDLK_LEFT: //Left arrow.
			if (pCurrentGame->SetPlayerToWestExit())
			{
				bExitRoomForCheat = pCurrentGame->LoadNewRoomForExit(W, Ignored);
			}
		break;

		case SDLK_RIGHT: //Right arrow.
			if (pCurrentGame->SetPlayerToEastExit())
			{
				bExitRoomForCheat = pCurrentGame->LoadNewRoomForExit(E, Ignored);
			}
		break;
#endif

		default: break;
	}

#if defined (ENABLE_CHEATS)
	if (bExitRoomForCheat)
	{
		this->pRoomWidget->UpdateFromCurrentGame();
		this->pMapWidget->UpdateFromCurrentGame();
		ClearSpeech();
		SetGameAmbience(true);
		SetSignTextToCurrentRoom();
		PaintSign();
		HideScroll();
		this->bMusicStyleFrozen = false;
		SetMusicStyle();
		this->pFaceWidget->SetReading(false);
		this->pMapWidget->RequestPaint();
		this->pRoomWidget->ClearEffects();
		this->pRoomWidget->Paint();
		this->pFaceWidget->Paint();
		PaintClock(true);
		this->bRoomClearedOnce = false;
	}
#endif
}

//*****************************************************************************
void CGameScreen::OnMouseMotion(
//
//Params:
	const UINT dwTagNo,
	const SDL_MouseMotionEvent &MotionEvent)
{
	if (this->bShowingBigMap)
		return;

	CDrodScreen::OnMouseMotion(dwTagNo, MotionEvent);
	this->pFaceWidget->MovePupils(MotionEvent.x, MotionEvent.y);
}

//*****************************************************************************
void CGameScreen::OnSelectChange(
//Handles a selection change.
//
//Params:
	const UINT dwTagNo) //(in) Widget event applies to.
{
	switch (dwTagNo)
	{
		case TAG_CHATINPUT:
			ReformatChatText(TAG_CHATINPUT, true);
		break;
	}
}

//
//CGameScreen private methods.
//

//*****************************************************************************
void CGameScreen::AddDamageEffect(const UINT wMonsterType, const CMoveCoord& coord)
//Adds an effect for a monster type getting damaged to the room.
{
	//If player stabs monster, center sound on player for nicer effect.
	CSwordsman &player = this->pCurrentGame->swordsman;
	if (player.wSwordX == coord.wX && player.wSwordY == coord.wY)
	{
		this->fPos[0] = static_cast<float>(player.wX);
		this->fPos[1] = static_cast<float>(player.wY);
	} else {
		this->fPos[0] = static_cast<float>(coord.wX);
		this->fPos[1] = static_cast<float>(coord.wY);
	}
	PlaySoundEffect(
			wMonsterType == M_ROCKGOLEM || wMonsterType == M_ROCKGIANT ?
			SEID_BREAKWALL : SEID_SPLAT, this->fPos);

	//Effect shown based on monster type.
	switch (wMonsterType)
	{
		case M_TARBABY:
		case M_TARMOTHER:
			this->pRoomWidget->AddTLayerEffect(
				new CTarStabEffect(this->pRoomWidget, coord,
						GetEffectDuration(7), GetParticleSpeed(4)));
		break;
		case M_MUDBABY:
		case M_MUDMOTHER:
			this->pRoomWidget->AddTLayerEffect(
				new CMudStabEffect(this->pRoomWidget, coord,
						GetEffectDuration(7), GetParticleSpeed(4)));
		break;
		case M_GELBABY:
		case M_GELMOTHER:
			this->pRoomWidget->AddTLayerEffect(
				new CGelStabEffect(this->pRoomWidget, coord,
						GetEffectDuration(7), GetParticleSpeed(4)));
		break;
		case M_SEEP:
			this->pRoomWidget->AddTLayerEffect(
				new CBloodInWallEffect(this->pRoomWidget, coord));
		break;
		case M_ROCKGOLEM:
		case M_ROCKGIANT:
			this->pRoomWidget->AddTLayerEffect(
				new CGolemDebrisEffect(this->pRoomWidget, coord, 10,
						GetEffectDuration(7), GetParticleSpeed(4)));
		break;
		case M_SLAYER:
		case M_SLAYER2:
			PlaySoundEffect(SEID_SLAYER_DIE, this->fPos);
			this->pRoomWidget->AddTLayerEffect(
				new CVerminEffect(this->pRoomWidget, coord, 40, true));
		break;
		default:
			this->pRoomWidget->AddTLayerEffect(
				new CBloodEffect(this->pRoomWidget, coord, 16,
						GetEffectDuration(7), GetParticleSpeed(4)));
		break;
	}
}

//*****************************************************************************
void CGameScreen::AmbientSoundSetup()
//Queries current game for which ambient sounds should be playing at this
//moment of play.
{
	for (vector<CMoveCoordEx>::const_iterator sound = this->pCurrentGame->ambientSounds.begin();
			sound != this->pCurrentGame->ambientSounds.end(); ++sound)
	{
		PlayAmbientSound(sound->wO, sound->wValue != 0, sound->wX, sound->wY);
	}
}

//*****************************************************************************
void CGameScreen::ApplyPlayerSettings()
//Apply player settings to the game screen.
{
	ApplyINISettings();

	CDbPlayer *pCurrentPlayer = g_pTheDB->GetCurrentPlayer();
	if (!pCurrentPlayer) {
		ASSERT(!"CGameScreen::ApplyPlayerSettings(): Couldn't retrieve current player.");
		return; //Corrupt db.
	}

	//Set the keysym-to-command map from player settings.
	InitKeysymToCommandMap(pCurrentPlayer->Settings);

	//Set room widget to either show checkpoints or not.
	this->pRoomWidget->ShowCheckpoints(
			pCurrentPlayer->Settings.GetVar(showCheckpoints, true));

	this->bShowingSubtitlesWithVoice =
			pCurrentPlayer->Settings.GetVar(showSubtitlesWithVoices, true);

	//Enable/disable move counter.
	if (this->pRoomWidget->IsShowingMoveCount() !=
			pCurrentPlayer->Settings.GetVar(moveCounterStr, false))
		this->pRoomWidget->ToggleMoveCount();

	//Move repeat rate.
	const UINT dwRepeatRate = (((long)(pCurrentPlayer->Settings.GetVar("RepeatRate",
			(BYTE)128))) * MAX_REPEAT_RATE / 256) + 1;  //value from 1 to MAX
	const UINT dwTimePerRepeat = 1000L / dwRepeatRate;
	SetKeyRepeat(dwTimePerRepeat);
	this->pRoomWidget->SetMoveDuration(dwTimePerRepeat);

	this->bEnableChat = pCurrentPlayer->Settings.GetVar(enableChat, false);
	this->bReceiveWhispersOnly = pCurrentPlayer->Settings.GetVar(receiveWhispersOnly, false);

	//Reset for this session.
	this->dwTotalPlayTime = 0;

	//Set times when saved games and demos are saved automatically.
	if (!this->bPlayTesting)
	{
		const UINT dwAutoSaveOptions = pCurrentPlayer->Settings.GetVar(
				"AutoSaveOptions", ASO_DEFAULT | ASO_CONQUERDEMO);
		this->pCurrentGame->SetAutoSaveOptions(dwAutoSaveOptions);
		pCurrentPlayer->Update();
	}

	delete pCurrentPlayer;
}

void CGameScreen::GotoHelpPage()
{
	CBrowserScreen::SetPageToLoad("quickstart.html");
	GoToScreen(SCR_Browser);
}

void CGameScreen::OpenStatsBox()
{
	ShowCursor();
	this->dwTotalPlayTime += this->pCurrentGame->UpdateTime(SDL_GetTicks());  //make time current
	DisplayRoomStats();
}

//*****************************************************************************
void CGameScreen::SetSignTextToCurrentRoom()
//Set sign text to description of current room and repaint it.
{
	static const WCHAR wszSignSep[] = { We(':'),We(' '),We(0) };
	WSTRING wstrSignText = (const WCHAR *)this->pCurrentGame->pLevel->NameText;
	if (this->bShowingCutScene)
	{
		wstrSignText += wszSpace;
		wstrSignText += (const WCHAR *) CDbMessageText(MID_PlayingCutScene);
		this->signColor = Red;
	} else {
		ASSERT(this->pCurrentGame);
		ASSERT(this->pCurrentGame->pRoom);
		wstrSignText += wszSignSep;
		this->pCurrentGame->pRoom->GetLevelPositionDescription(wstrSignText);
		this->signColor = Black;
	}

	if (this->pCurrentGame->IsDemoRecording())
	{
		wstrSignText += wszSpace;
		wstrSignText += (const WCHAR *) CDbMessageText(MID_RecordingStatus);
	}
	SetSignText(wstrSignText.c_str());
}

//*****************************************************************************
SCREENTYPE CGameScreen::LevelExit_OnKeydown(
//Handles SDL_KEYDOWN events for the game screen when exiting level.
//
//Params:
	const SDL_KeyboardEvent &KeyboardEvent)   //(in) Event to handle.
//
//Returns:
//Screen to go to next or SCR_Game to remain at game screen.
{
	switch (KeyboardEvent.keysym.sym)
	{
		case SDLK_RALT: case SDLK_LALT:
			return SCR_Game;  //don't advance to next screen
		case SDLK_RETURN:
			if (KeyboardEvent.keysym.mod & KMOD_ALT)
			{
				ToggleScreenSize();
				return SCR_Game;  //changing screen size shouldn't advance to next screen
			}
			break;
		case SDLK_F10:
			ToggleScreenSize();
			return SCR_Game;  //changing screen size shouldn't advance to next screen

		case SDLK_F4:
#if defined(__linux__) || defined(__FreeBSD__)
		case SDLK_PAUSE:  case SDLK_BREAK:
#endif
			if (KeyboardEvent.keysym.mod & (KMOD_ALT | KMOD_CTRL))
				return SCR_None;        //boss key -- exit immediately
		break;
		default: break;
	}

	return SCR_LevelStart;
}

//*****************************************************************************
void CGameScreen::ClearSpeech(const bool bForceClearAll) //[default=true]
//If bForceClearAll is true, then reset queued speech actions, cut any playing
//custom sound clips, and free allocated memory buffers.
//Otherwise, only stop any speech/sound playing that doesn't apply to this point in game time.
{
	const UINT currentTurn = this->pCurrentGame ? this->pCurrentGame->wTurnNo : 0;

	//Keep any speech that hasn't played yet.
	deque<CFiredCharacterCommand*> retain;
	deque<CFiredCharacterCommand*>::iterator iter;
	for (iter=this->speech.begin(); iter!=this->speech.end(); ++iter)
	{
		CFiredCharacterCommand *pCommand = *iter;
		if (pCommand->turnNo < currentTurn && !bForceClearAll)
		{
			//Hook up the command to the current instance of the character with its scriptID.
			ASSERT(this->pCurrentGame);
			if (pCommand->bPseudoMonster)
			{
				//No information is stored to reattach speech command.
				delete pCommand;
			} else {
				pCommand->pExecutingNPC = pCommand->pSpeakingEntity =
						pCurrentGame->pRoom->GetCharacterWithScriptID(pCommand->scriptID);
				ASSERT(pCommand->pExecutingNPC);
				if (pCommand->pExecutingNPC) //robustness
				{
					//This speech command may be reliably retained.
					retain.push_back(pCommand); 

					//Also hook up this wrapper object to the current instance of this script command.
					CCharacter *pCharacter = DYN_CAST(CCharacter*, CMonster*, pCommand->pExecutingNPC);
					ASSERT(pCommand->commandIndex < pCharacter->commands.size());
					pCommand->pCommand = &(pCharacter->commands[pCommand->commandIndex]);
					PrepCustomSpeaker(pCommand);
				} else {
					delete pCommand;
				}
			}
		} else {
			delete pCommand;
		}
	}
	this->speech = retain;

	CutSpeech(bForceClearAll);
	StopAmbientSounds();

	//The call to CutSpeech marks which subtitles should be retained.
	//Now remove subtitles which have not been marked.
	ASSERT(this->pRoomWidget);
	this->pRoomWidget->RemoveLastLayerEffectsOfType(ESUBTITLE, bForceClearAll);

	//If all playing sounds have been stopped, then we can free them.
	//Otherwise, let everything remain for now.
	if (this->speechChannels.empty() && this->ambientChannels.empty())
	{
		if (g_pTheSound)
			g_pTheSound->FreeSoundDump();

		//Show player when no one is speaking.
		ShowPlayerFace(true);
	}

	SetGameAmbience();
}

//*****************************************************************************
void CGameScreen::CutSpeech(const bool bForceClearAll) //[default=true]
//If bForceClearAll is true, stop playing any running sound clips.
//Otherwise, only stop any speech/sound playing that doesn't apply to this point in game time.
{
	const UINT currentTurn = this->pCurrentGame ? this->pCurrentGame->wTurnNo : 0;

	vector<ChannelInfo> retain;
	for (vector<ChannelInfo>::const_iterator channel=this->speechChannels.begin();
			channel!=this->speechChannels.end(); ++channel)
	{
		//Stop this channel from playing if it's a speech sound clip.
		if (g_pTheSound->IsSoundPlayingOnChannel(channel->wChannel) &&
				g_pTheSound->GetSoundIDPlayingOnChannel(channel->wChannel) ==
						static_cast<UINT>(SOUNDLIB::SEID_NONE))
		{
			//If speech is still valid, then let it continue playing.
			if (channel->turnNo < currentTurn && !bForceClearAll)
			{
				retain.push_back(*channel);
				CSubtitleEffect *pEffect = channel->pEffect;
				if (this->pRoomWidget->SubtitlesHas(pEffect))
				{
					//Keep related subtitle effect alive.
					ASSERT(pEffect);
					pEffect->RequestRetainOnClear();
					pEffect->RemoveFromSubtitles();
				} else {
					retain.back().pEffect = NULL; //subtitle effect connected to this channel is finished
				}
			} else {
				VERIFY(g_pTheSound->StopSoundOnChannel(channel->wChannel));
			}
		}
	}
	this->speechChannels = retain;
	if (this->speechChannels.empty())
		this->dwNextSpeech = 0; //can start next speech immediately
}

//*****************************************************************************
void CGameScreen::DeleteCurrentGame()
//Deletes current game.  Ensures any demos are uploaded first.
{
	//Ensure all queued victory demos are submitted before deleting them.
	WaitToUploadDemos();

	delete this->pCurrentGame;
	this->pCurrentGame = NULL;
	this->pRoomWidget->UnloadCurrentGame();

	this->pRoomWidget->RemoveHighlight();
}

//*****************************************************************************
void CGameScreen::DisplayRoomStats()
//Show dialog box displaying current game stats.
{
	CDialogWidget *pStatsBox = DYN_CAST(CDialogWidget*, CWidget*,
			this->pRoomWidget->GetWidget(TAG_STATSBOX));

	pStatsBox->SelectWidget(TAG_CHATINPUT, false);

	WCHAR dummy[32];
	WSTRING text;

	CFrameWidget *pFrame = DYN_CAST(CFrameWidget*, CWidget*, pStatsBox->GetWidget(TAG_GAMESTATSFRAME));
	pFrame->SetCaption((const WCHAR*)this->pCurrentGame->pHold->NameText);

	CLabelWidget *pLabel = DYN_CAST(CLabelWidget*, CWidget*, pStatsBox->GetWidget(TAG_MOVES));
	text = _ltoW(this->pCurrentGame->wPlayerTurn, dummy, 10);
	pLabel->SetText(text.c_str());

	pLabel = DYN_CAST(CLabelWidget*, CWidget*, pStatsBox->GetWidget(TAG_KILLS));
	text = _ltoW(this->pCurrentGame->wMonsterKills, dummy, 10);
	pLabel->SetText(text.c_str());

	//Print number of rooms conquered/explored.
	WSTRING wstrGameStats = wszCRLF;
	wstrGameStats += GetGameStats(false, true);

	WCHAR temp[20];
	wstrGameStats += g_pTheDB->GetMessageText(MID_Rooms);
	wstrGameStats += wszColon;
	wstrGameStats += wszSpace;
	_itoW(this->pCurrentGame->ConqueredRooms.size(), temp, 10);
	wstrGameStats += temp;
	wstrGameStats += wszSpace;
	wstrGameStats += wszForwardSlash;
	wstrGameStats += wszSpace;
	_itoW(this->pCurrentGame->ExploredRooms.size(), temp, 10);
	wstrGameStats += temp;

	pLabel = DYN_CAST(CLabelWidget*, CWidget*, pStatsBox->GetWidget(TAG_GAMESTATS));
	pLabel->SetText(wstrGameStats.c_str());

	//Chat.
	COptionButtonWidget *pOption = DYN_CAST(COptionButtonWidget*, CWidget*, pStatsBox->GetWidget(TAG_CHATENABLE));
	pOption->SetChecked(this->bEnableChat);

	COptionButtonWidget *pWhisperOption = DYN_CAST(COptionButtonWidget*, CWidget*, pStatsBox->GetWidget(TAG_CHATWHISPERSONLY));
	pWhisperOption->SetChecked(this->bReceiveWhispersOnly);

	CTextBoxWidget* pChat = DYN_CAST(CTextBoxWidget*, CWidget*, pStatsBox->GetWidget(TAG_CHATINPUT));
	pChat->SetText(wszEmpty);

	CListBoxWidget *pUserList = DYN_CAST(CListBoxWidget*, CWidget*, pStatsBox->GetWidget(TAG_CHATUSERS));
	if (!g_pTheNet->IsLoggedIn())
	{
		pUserList->Clear();
		pUserList->AddItem(0, g_pTheDB->GetMessageText(MID_CNetNotConnected), true);
	}

	//Display.
	pStatsBox->SetBetweenEventsHandler(this); //keep updating room effects and
							//receiving chat messages while prompt is displayed
	const UINT returnTag = pStatsBox->Display(false);
	pStatsBox->SetBetweenEventsHandler(NULL);
	this->pRoomWidget->DirtyRoom();
	this->pRoomWidget->Paint();

	this->bEnableChat = pOption->IsChecked();
	this->bReceiveWhispersOnly = pWhisperOption->IsChecked();

	switch (returnTag) {
		case TAG_ONLINE_HELP:
			if (this->pCurrentGame && this->pCurrentGame->pRoom)
				CDrodScreen::BrowseForRoomHints(this->pCurrentGame->pRoom);
		break;
		case TAG_OK:
		{
			//Enter console command, when applicable, or send chat.
			const WCHAR *pText = pChat->GetText();
			if (!ParseConsoleCommand(pText)) //Intercept console commands.
				this->chat.SendText(pText, pUserList);
			break;
		}
		default: break;
	}
}

//*****************************************************************************
void CGameScreen::FadeRoom(const bool bFadeIn, const Uint32 dwDuration)
{
	//Prepare fade surface.
	SDL_Rect srcRect;
	this->pRoomWidget->GetRect(srcRect);
	SDL_Rect destRect = {0, 0, srcRect.x + srcRect.w, srcRect.y + srcRect.h};
	SDL_Surface *pSurface = g_pTheBM->ConvertSurface(SDL_CreateRGBSurface(
			SDL_SWSURFACE, destRect.w, destRect.h, g_pTheBM->BITS_PER_PIXEL, 0, 0, 0, 0));
	if (!pSurface) return; //no memory

	SDL_Surface *pDestSurface = GetDestSurface();
	if (bFadeIn)
	{
		SDL_Rect snapshotRect(destRect);
      SDL_BlitSurface(this->pRoomWidget->pRoomSnapshotSurface, &snapshotRect,
				pSurface, &snapshotRect);
		this->pRoomWidget->RenderFogInPit(pSurface);
		this->pRoomWidget->DrawPlatforms(pSurface);
		this->pRoomWidget->DrawMonsters(this->pCurrentGame->pRoom->pFirstMonster, pSurface, true);
		this->pRoomWidget->RenderEnvironment(pSurface);
		this->pRoomWidget->DrawPlayer(this->pCurrentGame->swordsman, pSurface);
	} else {
		SDL_BlitSurface(pDestSurface, &srcRect, pSurface, &destRect);
	}

	//Fade.
	{
		CFade fade(bFadeIn ? NULL : pSurface, bFadeIn ? pSurface : NULL);
		SDL_Surface *pFadingSurface = fade.GetDestSurface();
		const Uint32 dwStart = SDL_GetTicks();
		const Uint32 dwEnd = dwStart + dwDuration;
		Uint32 dwNow = dwStart;
		do
		{
			dwNow = SDL_GetTicks();
			fade.IncrementFade((dwNow - dwStart) / (float)dwDuration);
			SDL_BlitSurface(pFadingSurface, &(bFadeIn ? srcRect : destRect), pDestSurface, &srcRect);
			SDL_UpdateRect(pDestSurface, srcRect.x, srcRect.y, srcRect.w, srcRect.h);
		} while (dwNow < dwEnd);
	}
	SDL_FreeSurface(pSurface);
}

//*****************************************************************************
WSTRING CGameScreen::GetGameStats(
	const bool bHoldTotals, //combine totals for all levels in hold [default=false]
	const bool bOnlyCurrentGameRooms)	//use rooms from only this game [default=false]
const
{
	WCHAR temp[50];
	WSTRING wstrLevelStats;
	UINT dwDeaths, dwKills, dwMoves, dwTime, dwSecrets;

	ASSERT(this->pCurrentGame);
	if (bHoldTotals)
	{
		wstrLevelStats += g_pTheDB->GetMessageText(MID_HoldTotals);
		//Scan through all levels in hold and sum up level stats.
		dwDeaths = dwKills = dwMoves = dwTime = 0;
		ASSERT(this->pCurrentGame->pHold);
		CIDSet levelsInHold = CDb::getLevelsInHold(this->pCurrentGame->pHold->dwHoldID);
		for (CIDSet::const_iterator levelID = levelsInHold.begin(); levelID != levelsInHold.end(); ++levelID)
		{
			CDbLevel *pLevel = g_pTheDB->Levels.GetByID(*levelID);
			ASSERT(pLevel);
			this->pCurrentGame->GetLevelStats(pLevel);
			dwDeaths += this->pCurrentGame->dwLevelDeaths;
			dwKills += this->pCurrentGame->dwLevelKills;
			dwMoves += this->pCurrentGame->dwLevelMoves;
			dwTime += this->pCurrentGame->dwLevelTime;
			delete pLevel;			
		}

		HoldStats holdStats;
		dwSecrets = g_pTheDB->Holds.GetSecretsDone(holdStats,
				this->pCurrentGame->pHold->dwHoldID, g_pTheDB->GetPlayerID(), false);
	} else {
		wstrLevelStats += g_pTheDB->GetMessageText(MID_LevelTotals);
		dwDeaths = this->pCurrentGame->dwLevelDeaths;
		dwKills = this->pCurrentGame->dwLevelKills;
		dwMoves = this->pCurrentGame->dwLevelMoves;
		dwTime = this->pCurrentGame->dwLevelTime;

		UINT dwSecretRooms;
		CIDSet roomsInLevel, playerRoomsExploredInLevel;
		if (!bOnlyCurrentGameRooms)
		{
			//Get all rooms in levels and all rooms player has ever explored in level.
			g_pTheDB->Levels.GetRoomsExplored(this->pCurrentGame->pLevel->dwLevelID,
					g_pTheDB->GetPlayerID(), roomsInLevel, playerRoomsExploredInLevel);
		} else {
			//Get all rooms in level.
			roomsInLevel = CDb::getRoomsInLevel(this->pCurrentGame->pLevel->dwLevelID);
		}
		g_pTheDB->Levels.GetSecretRoomsInSet(roomsInLevel, bOnlyCurrentGameRooms ? 
				this->pCurrentGame->ExploredRooms : playerRoomsExploredInLevel,
				dwSecretRooms, dwSecrets);
	}

	wstrLevelStats += wszCRLF;
	wstrLevelStats += g_pTheDB->GetMessageText(MID_Moves);
	wstrLevelStats += wszColon;
	wstrLevelStats += wszSpace;
	_itoW(dwMoves, temp, 10);
	wstrLevelStats += temp;
	wstrLevelStats += wszCRLF;
	wstrLevelStats += g_pTheDB->GetMessageText(MID_Kills);
	wstrLevelStats += wszColon;
	wstrLevelStats += wszSpace;
	_itoW(dwKills, temp, 10);
	wstrLevelStats += temp;
	wstrLevelStats += wszCRLF;
	wstrLevelStats += g_pTheDB->GetMessageText(MID_Deaths);
	wstrLevelStats += wszColon;
	wstrLevelStats += wszSpace;
	_itoW(dwDeaths, temp, 10);
	wstrLevelStats += temp;
	wstrLevelStats += wszCRLF;
	wstrLevelStats += g_pTheDB->GetMessageText(MID_Time);
	wstrLevelStats += wszColon;
	wstrLevelStats += wszSpace;
	wstrLevelStats += CDate::FormatTime(dwTime / 1000); //convert ms -> s
	wstrLevelStats += wszCRLF;
	wstrLevelStats += g_pTheDB->GetMessageText(MID_SecretsFound);	//no colon after
	wstrLevelStats += wszSpace;
	_itoW(dwSecrets, temp, 10);
	wstrLevelStats += temp;
	wstrLevelStats += wszCRLF;

	return wstrLevelStats;
}

//*****************************************************************************
UINT CGameScreen::GetMessageAnswer(const CMonsterMessage *pMsg)
//Display a dialog box with a question and a menu of answer options.
//
//Returns: value of answer selected by user, TAG_UNDO_FROM_QUESTION to undo
{
	//Set question.
	CMenuWidget *pMenu = DYN_CAST(CMenuWidget*, CWidget*, GetWidget(TAG_MENU));
	CLabelWidget *pPrompt = DYN_CAST(CLabelWidget*, CWidget*, GetWidget(TAG_MENUPROMPT));
	const WCHAR *pTextStr = pMsg->message.c_str();
	pPrompt->SetText(pTextStr);

	//Set answer options.
	CMonster *pMonster = pMsg->pSender;
	CCharacter *pCharacter = DYN_CAST(CCharacter*, CMonster*, pMonster);
	ASSERT(!pCharacter->answerOptions.empty());
	UINT count=0; //Up to 9 options + undo are allowed.  Any more choices are ignored.
	WCHAR temp[4];
	for (CIDSet::const_iterator option = pCharacter->answerOptions.begin();
			option != pCharacter->answerOptions.end() && count<9; ++option, ++count)
	{
		ASSERT(*option < pCharacter->commands.size());
		CCharacterCommand& command = pCharacter->commands[*option];
		CDbSpeech *pSpeech = command.pSpeech;
		ASSERT(pSpeech);
		const WCHAR *pText = (const WCHAR*)pSpeech->MessageText;
		WSTRING wstr = wszAmpersand; //add # hotkey
		wstr += _itoW(count+1, temp, 10);
		wstr += wszPeriod;
		wstr += wszSpace;
		wstr += this->pCurrentGame->ExpandText(pText); //resolve var refs
		pMenu->AddText(wstr.c_str(), command.x);
	}

	//Add interface to undo last move.
	WSTRING wstrUndo = wszAmpersand; //add # hotkey
	wstrUndo += _itoW(0, temp, 10);
	wstrUndo += wszPeriod;
	wstrUndo += wszSpace;
	wstrUndo += wszLeftParen;
	wstrUndo += g_pTheDB->GetMessageText(MID_Undo);
	wstrUndo += wszRightParen;
	pMenu->AddText(wstrUndo.c_str(), TAG_UNDO_FROM_QUESTION);

	//Resize label for prompt text height.
	SDL_Rect rect;
	pPrompt->GetRect(rect);
	UINT wTextHeight, wIgnored;
	g_pTheFM->GetTextRectHeight(FONTLIB::F_Message, pTextStr, rect.w, wIgnored, wTextHeight);
	pPrompt->SetHeight(wTextHeight);

	static const UINT FRAME_BUFFER = 3;
	CWidget *pFrame = this->pMenuDialog->GetWidget(TAG_MENUFRAME);

	//Resize rest of dialog widgets.
	const int yMenu = wTextHeight + (CY_SPACE * 2);
	const UINT wMenuHeight = pMenu->DispHeight();
	const UINT wTotalHeight = yMenu + wMenuHeight + FRAME_BUFFER*2 + CY_SPACE;

	this->pMenuDialog->SetHeight(wTotalHeight);

	//Center the dialog over the room.
	this->pMenuDialog->Center();
	this->pMenuDialog->Move(this->pRoomWidget->GetX() + (this->pRoomWidget->GetW() - this->pMenuDialog->GetW()) / 2,
			this->pMenuDialog->GetY());
	ASSERT(this->pMenuDialog->IsInsideOfParent()); //If this fires, the dialog probably has too many options

	pMenu->Move(CX_SPACE, yMenu);

	pFrame->Move(CX_SPACE - FRAME_BUFFER, yMenu - FRAME_BUFFER);
	pFrame->SetHeight(wMenuHeight + FRAME_BUFFER*2);

	//Get answer.
	Paint();
	ShowCursor();
	const UINT dwAnswer = this->pMenuDialog->Display();

	//Cleanup.
	pMenu->clear();
	Paint();
	return dwAnswer;
}

//*****************************************************************************
UINT CGameScreen::GetEffectDuration(const UINT baseDuration) const
//Returns: duration of particles in particle effects
{
	//When player is hasted, particles move at half speed, so they last twice as long
	return this->pCurrentGame && this->pCurrentGame->swordsman.bIsHasted ?
			baseDuration*2 : baseDuration;
}

//*****************************************************************************
UINT CGameScreen::GetParticleSpeed(const UINT baseSpeed) const
//Returns: speed of particles in particle effects
{
	//When player is hasted, particles move at half speed
	return this->pCurrentGame && this->pCurrentGame->swordsman.bIsHasted ?
			(baseSpeed > 1 ? baseSpeed/2 : 1) : baseSpeed;
}

//*****************************************************************************
void CGameScreen::HandleEventsForHoldExit()
//Display end hold stats until user presses a button.
{
	//Necessary to call explicitly because I am taking over event-handling.
	StopKeyRepeating();
	ClearEvents();

	UINT dwNow = SDL_GetTicks() - 100,  //first tick delayed, so start it now
			dwLastAnimate = dwNow;

	//Get any events waiting in the queue.
	//Finish on a key/mouse press.
	SDL_Event event;
	while (true)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_ACTIVEEVENT:
					OnActiveEvent(event.active);
				break;

				case SDL_KEYUP:
				case SDL_MOUSEBUTTONUP:
				case SDL_QUIT:
				return;

				case SDL_MOUSEMOTION:
					SetCursor(); //make mouse cursor visible
				break;

				default: break;
			}
		}
		dwNow = SDL_GetTicks();

		//Animate every so often.
		if (dwNow - dwLastAnimate > 90)
		{
			this->pRoomWidget->pLastLayerEffects->DrawEffects();
			g_pTheBM->UpdateScreen(GetWidgetScreenSurface());
			dwLastAnimate = dwNow;
		}
		//Keep uploading demos.
		UploadDemoPolling();
	}
}

//*****************************************************************************
SCREENTYPE CGameScreen::HandleEventsForLevelExit()
//Plays level exit music, shows player walking down the stairs, etc.
//Allow only certain key commands during this time.
//
//Note that the On*() handlers are not going to be called by CEventHandlerWidget's
//Activate() loop until after this method exits.  Events must be handled here.
//
//Returns:
//Screen to activate after this one.  SCR_None indicates an application exit,
//and SCR_Return indicates the screen previously activated.
{
	bool bDoneDescendingStairs = false;

	//Necessary to call explicitly because I am taking over event-handling.
	StopKeyRepeating();

	//Play level exit music.
	CRoomScreen::SetMusicStyle(this->pCurrentGame->pRoom->style, SONG_EXIT, 1000);

	HideScroll();
	this->pFaceWidget->SetReading(false);
	this->pFaceWidget->SetMood(Mood_Happy);
	this->pRoomWidget->AllowSleep(false);

	//Show the screen after first arriving here.
	Paint();
	UINT dwLastStep = SDL_GetTicks() - 100,  //first step's delayed, so take it a bit faster
			dwLastAnimate = dwLastStep;

	//Process events.
	SDL_Event event;
	SCREENTYPE eNextScreen;
	while (true)
	{
		//Get any events waiting in the queue.
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_ACTIVEEVENT:
					OnActiveEvent(event.active);
				break;

				case SDL_KEYDOWN:
					eNextScreen = LevelExit_OnKeydown(event.key);
					if (eNextScreen != SCR_Game)
					{
						this->pFaceWidget->SetMood(Mood_Normal);
						g_pTheSound->StopSong();
						if (GetScreenType() == SCR_Game)	//don't redraw player at end of demo
							this->pRoomWidget->ShowPlayer();
						this->pRoomWidget->AllowSleep(
								bIsSmitemaster(this->pCurrentGame->swordsman.wAppearance));
						return eNextScreen;
					}
				break;

				case SDL_MOUSEBUTTONUP:	//not DOWN, so mouse up doesn't exit next screen immediately too
					if (event.button.button == SDL_BUTTON_WHEELDOWN ||
							event.button.button == SDL_BUTTON_WHEELUP) break;	//ignore mouse wheel
					this->pFaceWidget->SetMood(Mood_Normal);
					g_pTheSound->StopSong();
					if (GetScreenType() == SCR_Game)	//don't redraw player at end of demo
						this->pRoomWidget->ShowPlayer();
					this->pRoomWidget->AllowSleep(
							bIsSmitemaster(this->pCurrentGame->swordsman.wAppearance));
					return SCR_LevelStart;
				break;

				case SDL_MOUSEMOTION:
					OnMouseMotion(GetTagNo(), event.motion);
				break;

				case SDL_QUIT:
				{
					this->bQuitPrompt = true;
					const UINT ret = ShowYesNoMessage(MID_ReallyQuit);
					this->bQuitPrompt = false;
					if (ret != TAG_NO)
						return SCR_None;
				}
				break;
			}
		}

		//Show swordsman walking down stairs every 360ms.
		const Uint32 dwNow = SDL_GetTicks();
		if (!bDoneDescendingStairs && dwNow - dwLastStep > 360)
		{
			this->pRoomWidget->DirtyRoom();
			bDoneDescendingStairs = !this->pCurrentGame->WalkDownStairs();
			if (bDoneDescendingStairs)
			{
				//done walking down stairs (player disappears)
				this->pRoomWidget->HidePlayer();
				this->pRoomWidget->Paint();

				//Beethro's thinking about the next level now.
				this->pFaceWidget->SetMood(Mood_Normal);
			}
			dwLastStep = dwNow;
		}

		//Animate every so often.
		if (dwNow - dwLastAnimate > 90)
		{
			this->pRoomWidget->Paint();
			this->pFaceWidget->Paint();
			dwLastAnimate = dwNow;
			g_pTheBM->UpdateRects(GetWidgetScreenSurface());
		}

		//Keep playing any remaining speech.
		ProcessSpeech();

		//Update music (switch song or continue music fade if one is in progress).
		g_pTheSound->UpdateMusic();

		//Keep uploading demos.
		UploadDemoPolling();
	}
}

//*****************************************************************************
bool CGameScreen::HandleEventsForPlayerDeath(CCueEvents &CueEvents)
//Displays player death animation:
// wait a short period of time while the scream wave plays,
// a CBloodEffect spurts out of his body,
// and the face widget wags its tongue.
//End after the scream wave finishes.
//Accept no commands during this period.
//
//Note that the On*() handlers are not going to be called by CEventHandlerWidget's
//Activate() loop until after this method exits.  Events must be handled here.
//
//Returns: whether death was not undone
{
	static const Uint32 dwDeathDuration = 2000;

	//Necessary to call explicitly because I am taking over event-handling.
	StopKeyRepeating();

	//Stop currently playing speech line.
	//If player hits undo, then speech will continue at the next line.
	CutSpeech();

	const UINT wOrigO = this->pCurrentGame->swordsman.wO;	//save value for demo record
	bool bSwordSwingsClockwise = true;
	bool bUndoDeath = false;

	//Show the screen after first arriving here.
	this->pFaceWidget->SetReading(false);
	this->pRoomWidget->RemoveTLayerEffectsOfType(ESPARK);	//stop showing where bombs were
	this->pRoomWidget->RemoveMLayerEffectsOfType(EPENDINGBUILD); //stop showing where pending building was
	this->pRoomWidget->RemoveLastLayerEffectsOfType(EEVILEYEGAZE);
	this->pRoomWidget->RemoveHighlight();
	this->pRoomWidget->RemoveLastLayerEffectsOfType(ESHADE); //remove user tile highlight
	this->pRoomWidget->PutTLayerEffectsOnMLayer();	//keep showing whatever effects were showing
	this->pRoomWidget->AllowSleep(false);
	this->pRoomWidget->Paint();

	//Prepare room for fade out.
	this->pRoomWidget->RenderRoomInPlay();
	this->pRoomWidget->RenderEnvironment(this->pRoomWidget->pRoomSnapshotSurface);

	const bool bFade = g_pTheBM->bAlpha;
	CFade *pFade = bFade ? new CFade(this->pRoomWidget->pRoomSnapshotSurface,NULL) : NULL;

	const bool bHalphDied = CueEvents.HasOccurred(CID_HalphDied);
	const bool bPlayerDied = CueEvents.HasOccurred(CID_MonsterKilledPlayer) ||
			CueEvents.HasOccurred(CID_PlayerBurned) ||
			CueEvents.HasOccurred(CID_ExplosionKilledPlayer) ||
			CueEvents.HasOccurred(CID_BriarKilledPlayer);
	const bool bNPCBeethroDied = CueEvents.HasOccurred(CID_NPCBeethroDied);
	const bool bCriticalNPCDied = CueEvents.HasOccurred(CID_CriticalNPCDied) ||
			(!bPlayerDied && !bNPCBeethroDied && !bIsSmitemaster(this->pCurrentGame->swordsman.wAppearance));
	bool bNonMonsterDeath = false;
	const CMonster *pHalph = NULL;
	CMonster *pNPCBeethro = bNPCBeethroDied ? this->pCurrentGame->pRoom->GetNPCBeethro(true) : NULL;
	if (bCriticalNPCDied)
		this->pFaceWidget->SetMood(Mood_Nervous);
	else if (bHalphDied)
	{
		this->pFaceWidget->SetMood(Mood_Nervous);
		pHalph = DYN_CAST(const CMonster*, const CAttachableObject*,
				CueEvents.GetFirstPrivateData(CID_HalphDied));
		PlaySpeakerSoundEffect(pHalph->GetIdentity() == M_HALPH ? SEID_HALPHDIE : SEID_HALPH2_DIE);
	} else {
		this->pFaceWidget->SetMood(Mood_Dying);
		UINT eSoundID;
		const UINT wAppearance = bNPCBeethroDied ? pNPCBeethro->GetIdentity() : this->pCurrentGame->swordsman.wAppearance;
		switch (wAppearance)
		{
			case M_CLONE: case M_DECOY: case M_MIMIC:
			case M_BEETHRO: case M_BEETHRO_IN_DISGUISE: eSoundID = SEID_DIE; break;
			case M_GUNTHRO: eSoundID = SEID_GB_DIE; break;
			case M_NEATHER: eSoundID = SEID_NSCARED; break;
			case M_GOBLIN:	case M_GOBLINKING: eSoundID = SEID_GOB_DIE; break;
			case M_TARBABY: case M_MUDBABY: case M_GELBABY:
			case M_TARMOTHER: case M_MUDMOTHER: case M_GELMOTHER:
				eSoundID = SEID_SPLAT; break;
			case M_ROCKGOLEM: case M_ROCKGIANT: eSoundID = SEID_ROCK_DIE; break;
			case M_CITIZEN1: case M_CITIZEN2: case M_GUARD:
			case M_MUDCOORDINATOR: case M_TARTECHNICIAN:
			case M_CITIZEN: eSoundID = SEID_CIT_DIE; break;
			case M_NEGOTIATOR:
				eSoundID = SEID_NEGO_DIE; break;
			case M_INSTRUCTOR:
			case M_CITIZEN3: case M_CITIZEN4:
				eSoundID = SEID_WOM_DIE; break;
			case M_WUBBA: eSoundID = SEID_WUBBA; break;
			case M_HALPH: eSoundID = SEID_HALPHDIE; break;
			case M_HALPH2: eSoundID = SEID_HALPH2_DIE; break;
			case M_SLAYER: case M_SLAYER2:
				eSoundID = SEID_SLAYER_DIE; break;
			case M_STALWART:
				eSoundID = SEID_STALWART_DIE; break;
			case M_STALWART2:
				eSoundID = SEID_SOLDIER_DIE; break;
			default: eSoundID = SEID_MON_OOF; break;
		}
		PlaySpeakerSoundEffect(eSoundID);

		if (bPlayerDied || bNPCBeethroDied)
		{
			//Some monsters say something when killing player.
			const CMonster *pMonster = DYN_CAST(const CMonster*, const CAttachableObject*,
					 CueEvents.GetFirstPrivateData(bPlayerDied ?
							CID_MonsterKilledPlayer : CID_NPCBeethroDied));
			if (pMonster)
			{
				switch (pMonster->wType)
				{
					case M_SLAYER:
						PlaySpeakerSoundEffect(SEID_SLAYERKILL);
					break;
					case M_SLAYER2:
						PlaySpeakerSoundEffect(SEID_SLAYER2KILL);
					break;
					default: break;
				}
			}
		}
		else	//killed by some other room element.  Handle a bit differently.
			bNonMonsterDeath = true;
	}

	//These events can be played if triggered regardless of who's dying.
	if (CueEvents.HasOccurred(CID_PlayerBurned))
	{
		PlaySoundEffect(SEID_SIZZLE);
		this->pRoomWidget->AddMLayerEffect(
				new CSteamEffect(this->pRoomWidget, this->pCurrentGame->swordsman));
	}

	UINT wSX=0, wSY=0;
	if (bPlayerDied)
	{
		if (!this->pCurrentGame->GetSwordsman(wSX, wSY))
		{
			wSX = this->pCurrentGame->swordsman.wX;
			wSY = this->pCurrentGame->swordsman.wY;
		}
	} else if (bNPCBeethroDied)
	{
		ASSERT(pNPCBeethro);
		wSX = pNPCBeethro->wX;
		wSY = pNPCBeethro->wY;
	}
	CMoveCoord coord(wSX, wSY, NO_ORIENTATION);

	UINT dwStart = SDL_GetTicks();
	Uint32 dwLastFade = 0, dwLastSwordWobble = 0, dwLastMonsterChomp = 0;
	bool bShowEffectOnce = true;
	SDL_Event event;
	while (true)
	{
		//Get any events waiting in the queue.
		while (SDL_PollEvent(&event))
		{
         switch (event.type)
			{
				case SDL_ACTIVEEVENT:
					OnActiveEvent(event.active);
				break;
				case SDL_KEYDOWN:
				{
					const int nCommand = this->KeysymToCommandMap[event.key.keysym.sym];
					if (nCommand == CMD_UNDO)
						bUndoDeath = true;
					else if (nCommand == CMD_RESTART)
						dwStart = 0; //finish effect now
				}
				break;
				default: break;
			}
		}

		//Animate as fast as possible.
		const Uint32 dwNow = SDL_GetTicks();

		//Sword wobbles around.
		if (!bHalphDied && !bCriticalNPCDied && dwNow - dwLastSwordWobble > 50)
		{
			if (pNPCBeethro)
				pNPCBeethro->wO = bSwordSwingsClockwise ?
							nNextCO(pNPCBeethro->wO) : nNextCCO(pNPCBeethro->wO);
			else
				this->pCurrentGame->swordsman.SetOrientation(
					bSwordSwingsClockwise || bNonMonsterDeath ?
							nNextCO(this->pCurrentGame->swordsman.wO) :
							nNextCCO(this->pCurrentGame->swordsman.wO));
			//sometimes sword rotation changes direction
			if (RAND(3) == 0)
				bSwordSwingsClockwise = !bSwordSwingsClockwise;
			dwLastSwordWobble = dwNow;
		}

		//Fade to black.
		if (dwNow - dwLastFade > 100 && g_pTheBM->bAlpha)
		{
			if (bFade)
			{
				const float fFade = (dwNow - dwStart) / (float)dwDeathDuration;
				pFade->IncrementFade(fFade);
				this->pRoomWidget->SetOpacityForMLayerEffectsOfType(ESNOWFLAKE, 1.0f-fFade);
				this->pRoomWidget->SetOpacityForMLayerEffectsOfType(ERAINDROP, 1.0f-fFade);
			}
			this->pRoomWidget->DirtyRoom();  //repaint whole room each fade
			dwLastFade = dwNow;
		}

		//Monster chomps on the player.
		if ((bPlayerDied || bNPCBeethroDied) &&
				this->pRoomWidget->pRoom->IsValidColRow(wSX, wSY))
		{
			if (dwNow - dwLastMonsterChomp > 200)
			{
				//Get killing monster tile.
				UINT wMX = wSX, wMY = wSY;
				const CMonster *pMonster = this->pRoomWidget->pRoom->GetMonsterAtSquare(wMX, wMX);
				if (pMonster && pMonster->IsPiece())
				{
					const CMonsterPiece *pPiece = DYN_CAST(const CMonsterPiece*, const CMonster*, pMonster);
					pMonster = pPiece->pMonster;
					wMX = pMonster->wX;
					wMY = pMonster->wY;
				}

				//Animate killing monster.
				if (this->pRoomWidget->SwitchAnimationFrame(wMX, wMY))
				{
					//Blood effect.
					CEffect *pEffect = NULL;
					const UINT wAppearance = bNPCBeethroDied ? pNPCBeethro->GetIdentity() : this->pCurrentGame->swordsman.wAppearance;
					switch (wAppearance)
					{
						case M_TARBABY: case M_TARMOTHER:
							pEffect = new CTarStabEffect(this->pRoomWidget, coord,
									GetEffectDuration(7), GetParticleSpeed(4));
						break;
						case M_MUDBABY: case M_MUDMOTHER:
							pEffect = new CMudStabEffect(this->pRoomWidget, coord,
									GetEffectDuration(7), GetParticleSpeed(4));
						break;
						case M_GELBABY: case M_GELMOTHER:
							pEffect = new CGelStabEffect(this->pRoomWidget, coord,
									GetEffectDuration(7), GetParticleSpeed(4));
						break;
						case M_ROCKGIANT: case M_ROCKGOLEM:
							pEffect = new CGolemDebrisEffect(this->pRoomWidget, coord, 10,
									GetEffectDuration(7), GetParticleSpeed(4));
						break;
						case M_SEEP:
							pEffect = new CBloodInWallEffect(this->pRoomWidget, coord);
						break;
						case M_SLAYER:
						case M_SLAYER2:
							if (bShowEffectOnce)
							{
								pEffect = new CVerminEffect(this->pRoomWidget, coord, 40, true);
								bShowEffectOnce = false;
							}
						break;
						default:
							pEffect = new CBloodEffect(this->pRoomWidget, coord, 16,
									GetEffectDuration(7), GetParticleSpeed(4));
						break;
					}
					if (pEffect)
						this->pRoomWidget->AddMLayerEffect(pEffect); //must go on top to show up
				}
				dwLastMonsterChomp = dwNow;
			}
		} else {
			//Explosion, flow, falling into pit, or hot tile killed player.
			//!!Show effect here.
		}

		this->pFaceWidget->Paint();
		this->pRoomWidget->Paint();

		g_pTheBM->UpdateRects(GetWidgetScreenSurface());

		//Scream has finished.  Return from animation.
		if (dwNow - dwStart > dwDeathDuration || bUndoDeath)
			break;
	}
	delete pFade;
	this->pCurrentGame->swordsman.SetOrientation(wOrigO);	//restore value

	if (bUndoDeath)
	{
		this->pRoomWidget->DirtyRoom();	//remove fading
		UndoMove();
		return false;
	}

	ClearSpeech();
	this->pFaceWidget->SetMood(Mood_Normal);
	return true;
}

//*****************************************************************************
SCREENTYPE CGameScreen::ProcessCommand(
//Processes game command, making calls to update game data and respond to cue
//events.  If movement macro is requested, perform move combination if
//room-ending events don't occur.
//
//Returns:
//Screen to go to next or SCR_Game to remain at game screen.
//
//Params:
	const int nCommand, const bool bMacro)	//(in) [default=false]
{
	//If question prompts have been delayed until after the transition to the
	//game screen has completed, then forbid the user to input commands before
	//the event engine has gotten around to prompting the user with these questions.
	if (this->bNeedToProcessDelayedQuestions)
		return SCREENTYPE(GetScreenType());

	bool bLeftRoom = false, bPlayerDied = false;
	UINT wNewLightLevel;
	bool bFadeLight = false;

	ASSERT(nCommand != CMD_UNSPECIFIED);
	ASSERT(nCommand < COMMAND_COUNT || nCommand == CMD_ADVANCE_CUTSCENE);
	switch (nCommand)
	{
		case CMD_RESTART: case CMD_RESTART_PARTIAL: case CMD_RESTART_FULL:
			//Rewind moves to previous checkpoints or restart the room.
			this->bMusicStyleFrozen = false;
			if (nCommand == CMD_RESTART_FULL)
				this->pCurrentGame->RestartRoom(this->sCueEvents);
			else if (nCommand == CMD_RESTART_PARTIAL)
				this->pCurrentGame->RestartRoomFromLastDifferentCheckpoint(this->sCueEvents);
			else
				this->pCurrentGame->RestartRoomFromLastCheckpoint(this->sCueEvents);

			this->pRoomWidget->RenderRoomLighting();
			this->pRoomWidget->ResetForPaint();
			this->pMapWidget->DrawMapSurfaceFromRoom(this->pCurrentGame->pRoom);
			this->pMapWidget->RequestPaint();
			SetSignTextToCurrentRoom();
			PaintSign();
			this->pRoomWidget->ClearEffects();
			ClearSpeech();
			g_pTheSound->StopAllSoundEffects(); //stop any game sounds that were playing
			SetGameAmbience(true);
			AmbientSoundSetup(); //determine what sounds should be playing now
			SwirlEffect();
			this->bRoomClearedOnce = this->pCurrentGame->IsCurrentRoomPendingExit();
			this->wUndoToTurn = this->pCurrentGame->wTurnNo;
			if (this->bPlayTesting)
				this->wUndoToTurn = this->pCurrentGame->wTurnNo ? this->pCurrentGame->wTurnNo - 1 : 0;
			else
				this->wForbidUndoBeforeTurn = this->wUndoToTurn = this->pCurrentGame->wTurnNo; //can't undo past restart point
			if (GetScreenType() == SCR_Demo)
				return SCR_Game; //can skip everything below
		break;

		case CMD_UNDO:
			UndoMove();
			return SCR_Game;	//everything below has already been handled in UndoMove

		case CMD_CLONE:
		case CMD_DOUBLE:
			SynchScroll(); //might have changed a scroll being displayed
			PaintScroll();
			//no break
		case CMD_ANSWER:
			//was already handled in the calling ProcessCommand
			break;

		default:
		{
			//Send command to current game and cue events list back.
			if (!this->pCurrentGame->bIsGameActive)
				break;
			ClearCueEvents();

			//Force the room to finish rendering any previous move
			//before this one is made for smoother motion.
			if (this->pRoomWidget->IsMoveAnimating())
			{
				this->pRoomWidget->FinishMoveAnimation();
				this->pRoomWidget->Paint();
			}

			const bool bWasCutScene = this->pCurrentGame->dwCutScene != 0;
			UINT wPrevTurnNo = this->pCurrentGame->wTurnNo;
			this->pCurrentGame->ProcessCommand(nCommand, this->sCueEvents);

			//Undo handling:
			//1. Don't modify undo turn during cut scenes.
			//2. If the command didn't increment wTurnNo, it doesn't affect the undo turn.
			if (!bWasCutScene &&
					this->pCurrentGame->wTurnNo > wPrevTurnNo)
				this->wUndoToTurn = wPrevTurnNo;

			bLeftRoom = this->sCueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerLeftRoom), CIDA_PlayerLeftRoom);
			if (bLeftRoom)
			{
				//If light level is changing, save value for a light fade below.
				bPlayerDied = this->sCueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerDied), CIDA_PlayerDied);
				if (!bPlayerDied && !this->sCueEvents.HasOccurred(CID_ExitLevelPending) &&
						this->pCurrentGame->bIsGameActive &&
						this->pRoomWidget->IsLightingRendered() &&
						!this->pCurrentGame->pRoom->weather.bSkipLightfade)
				{
					bFadeLight = true;
					wNewLightLevel = this->pCurrentGame->pRoom->weather.wLight;
					this->pCurrentGame->pRoom->weather.wLight = this->pRoomWidget->wDark;
				}
				if (bPlayerDied)
					this->pRoomWidget->RenderRoomLighting();
				this->pRoomWidget->ResetForPaint();

				if (!bPlayerDied)
					this->bRoomClearedOnce = false;
				//else if "leaving" is actually death, don't reset this yet in case death is undone
			}
			else if (nCommand < CMD_C || nCommand == CMD_NW)	//if moving, stop showing any sword swings from previous turns
				this->pRoomWidget->RemoveMLayerEffectsOfType(ESWORDSWING);
		}
		break;
	}

	if (!this->sCueEvents.HasOccurred(CID_ExitLevelPending) &&
			!this->sCueEvents.HasOccurred(CID_ExitRoomPending) &&
			!this->sCueEvents.HasOccurred(CID_ExitRoom))
		UpdateSound();
	const Uint32 dwNow = SDL_GetTicks();
	this->dwTotalPlayTime += this->pCurrentGame->UpdateTime(dwNow);

	//Time spent since last move.  Doesn't have to be precise.
	static const Uint32 dwMaxTimeConsideredPerMove = 2550;
	this->dwLastTime = dwNow - this->dwLastTime;
	this->dwTimeInRoom += this->dwLastTime > dwMaxTimeConsideredPerMove ?
			dwMaxTimeConsideredPerMove : this->dwLastTime;
	this->dwLastTime = dwNow;

	//Process cue events list to create effects that should occur before
	//room is drawn.  Might alter cue events when going to a new screen.
	this->bPersistentEventsDrawn = false;
	bPlayerDied = this->sCueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerDied), CIDA_PlayerDied);
	const bool bPlayingVideo = this->sCueEvents.HasOccurred(CID_PlayVideo);
 	SCREENTYPE eNextScreen = ProcessCueEventsBeforeRoomDraw(this->sCueEvents);
	if (eNextScreen == SCR_Game)
	{
		//Redraw the room.
		SetGameAmbience();

		if (!bPlayingVideo)
			this->pRoomWidget->Paint();
		else if (!bLeftRoom) //after a video, only redraw the room (and screen)
			Paint(); //when the room is not exited upon video's completion

		//Process cue events list to create effects that should occur after
		//room is drawn.
		eNextScreen = ProcessCueEventsAfterRoomDraw(this->sCueEvents);

		PaintClock();
		if (bLeftRoom && !this->bIsScrollVisible)
			this->pClockWidget->Paint();	//new clock should be shown right now

		if (bPlayerDied)
		{
			SwirlEffect(); //must call after ProcessCueEventsBeforeRoomDraw()
			ProcessSpeechCues(this->sCueEvents);	//start showing speech at room start
		} else if (nCommand == CMD_CLONE) {
			SwirlEffect();
			g_pTheSound->PlaySoundEffect(SEID_POTION);
		}

		//Movement oscillation macros.
		if (bMacro && nCommand <= CMD_WAIT && !bLeftRoom && this->pCurrentGame->bIsGameActive &&
				!this->pCurrentGame->IsCutScenePlaying())
		{
			//Play the command that will put you back where you were.
			//Performs recursive call.
			//Force the room to finish rendering any previous move
			//before this one is made for smoother motion.
			if (this->pRoomWidget->IsMoveAnimating())
			{
				this->pRoomWidget->FinishMoveAnimation();
				this->pRoomWidget->Paint();
			}
			g_pTheBM->UpdateRects(GetWidgetScreenSurface()); //show state after previous command briefly
			switch (nCommand)
			{
				case CMD_C: eNextScreen = ProcessCommand(CMD_CC); break;
				case CMD_CC: eNextScreen = ProcessCommand(CMD_C); break;
				case CMD_N: eNextScreen = ProcessCommand(CMD_S); break;
				case CMD_W: eNextScreen = ProcessCommand(CMD_E); break;
				case CMD_E: eNextScreen = ProcessCommand(CMD_W); break;
				case CMD_S: eNextScreen = ProcessCommand(CMD_N); break;
				case CMD_NE: eNextScreen = ProcessCommand(CMD_SW); break;
				case CMD_NW: eNextScreen = ProcessCommand(CMD_SE); break;
				case CMD_SE: eNextScreen = ProcessCommand(CMD_NW); break;
				case CMD_SW: eNextScreen = ProcessCommand(CMD_NE); break;
				case CMD_WAIT:
				{
					//Fast-forward to the next event by waiting until it occurs.
					//Special events considered are:
					//1. Start of the next monster spawn cycle.
					//2. Briar expansion.
					const bool bBriarExpanded = this->sCueEvents.HasOccurred(CID_BriarExpanded);
					if (!bBriarExpanded &&
							((this->pCurrentGame->wSpawnCycleCount % TURNS_PER_CYCLE) != 0 ||
								this->pCurrentGame->bHalfTurn))
						eNextScreen = ProcessCommand(CMD_WAIT, true);	//cascading recursive call
				}
				break;
				default: break;
			}
		}

		//Option to log vars each time a new room is entered.
		if (bLeftRoom && this->pCurrentGame && this->pCurrentGame->bIsGameActive)
		{
			string str;
			if (CFiles::GetGameProfileString("Customizing", "LogVars", str))
				if (atoi(str.c_str()) > 0)
					if (g_pTheDB->Holds.PlayerCanEditHold(this->pCurrentGame->pHold->dwHoldID))
						LogHoldVars();
		}
	}

	//Changing to a new light level.
	if (bFadeLight)
	{
		this->pCurrentGame->pRoom->weather.wLight = wNewLightLevel;
		this->pRoomWidget->FadeToLightLevel(wNewLightLevel);
	}

	return eNextScreen;
}

//*****************************************************************************
SCREENTYPE CGameScreen::ProcessCommand(
//Processes game command, making calls to update game data and respond to cue
//events.
//
//Returns:
//Screen to go to next or SCR_Game to remain at game screen.
//
//Params:
	const int nCommand, const UINT wX, const UINT wY)	//(in)
{
	ASSERT(nCommand != CMD_UNSPECIFIED);
	ASSERT(nCommand < COMMAND_COUNT);
	this->pCurrentGame->ProcessCommand(nCommand, this->sCueEvents, wX, wY);	//back-end logic
	if (nCommand == CMD_CLONE)
		SwirlEffect();
	return ProcessCommand(CMD_DOUBLE);	//use CMD_DOUBLE to handle the front-end stuff here w/o executing another command
}

//*****************************************************************************
SCREENTYPE CGameScreen::ProcessCueEventsBeforeRoomDraw(
//Process cue events list to create effects that should occur before
//room is drawn.
//
//Params:
	CCueEvents &CueEvents) //(in)
//
//Returns:
//Screen to go to next or SCR_Game to remain at game screen.
{
	SCREENTYPE eNextScreen = SCR_Game;

	//Remember for later if player left room (dying, level exit, room exit,
	//win game) because room reloading actions will erase cue events.
	const bool bPlayerLeftRoom = CueEvents.HasAnyOccurred(
			IDCOUNT(CIDA_PlayerLeftRoom), CIDA_PlayerLeftRoom);
	const CAttachableObject *pObj;

	ProcessMovieEvents(CueEvents);

	if (!bPlayerLeftRoom)
		ProcessSpeechCues(CueEvents);   //can be done here only when no ClearSpeech calls will be made below

	const bool bPlayerDied = CueEvents.HasAnyOccurred(IDCOUNT(CIDA_PlayerDied), CIDA_PlayerDied);

	//Notify player when they can't exit the room because they have temporarily locked exit.
	if (CueEvents.HasOccurred(CID_RoomExitLocked))
	{
		g_pTheSound->PlaySoundEffect(SEID_WISP);
		this->pRoomWidget->DisplaySubtitle(g_pTheDB->GetMessageText(MID_RoomLockEnabled),
				this->pCurrentGame->swordsman.wX, this->pCurrentGame->swordsman.wY,
				true);
	}
	//Notify player when they have made a move that was not allowed to be made.
	if (CueEvents.HasOccurred(CID_BumpedMasterWall))
	{
		g_pTheSound->PlaySoundEffect(SEID_WISP);
		this->pRoomWidget->DisplaySubtitle(g_pTheDB->GetMessageText(MID_BumpedMasterWall),
				this->pCurrentGame->swordsman.wX, this->pCurrentGame->swordsman.wY,
				true);
	}

	//Handle private sound channels.
	//Channel n+1 -- Swordsman's voice.
	this->pRoomWidget->RemoveMLayerEffectsOfType(EBUMPOBSTACLE);
	UINT eSoundID = SEID_NONE;
	if (CueEvents.HasOccurred(CID_RoomConquerPending)
		&& !CueEvents.HasOccurred(CID_MonsterExitsRoom)  //Beethro isn't happy about the 'Neather getting away.
		&& !this->bRoomClearedOnce  //Beethro won't laugh multiple times if room is cleared repeatedly.
		&& !bPlayerDied)            //don't laugh on simultaneous death
	{
		this->bRoomClearedOnce = true;
		eSoundID = GetPlayerClearSEID();
	}
	else if (CueEvents.HasOccurred(CID_HitObstacle))
	{
		switch (this->pCurrentGame->swordsman.wAppearance)
		{
			case M_CLONE: case M_DECOY: case M_MIMIC:
			case M_BEETHRO: case M_BEETHRO_IN_DISGUISE: eSoundID = SEID_OOF; break;
			case M_GUNTHRO: eSoundID = SEID_GB_OOF; break;
			case M_NEATHER: eSoundID = SEID_NFRUSTRATED; break;
			case M_GOBLIN:	case M_GOBLINKING: eSoundID = SEID_GOB_OOF; break;
			case M_TARBABY: case M_MUDBABY: case M_GELBABY:
			case M_TARMOTHER: case M_MUDMOTHER: case M_GELMOTHER:
				eSoundID = SEID_TAR_OOF; break;
			case M_ROCKGOLEM: case M_ROCKGIANT: eSoundID = SEID_ROCK_OOF; break;
			case M_CITIZEN1: case M_CITIZEN2: case M_GUARD:
			case M_MUDCOORDINATOR: case M_TARTECHNICIAN:
			case M_CITIZEN: eSoundID = SEID_CIT_OOF; break;
			case M_WUBBA: eSoundID = SEID_WUBBA; break;
			case M_NEGOTIATOR:
				eSoundID = SEID_NEGO_OOF; break;
			case M_INSTRUCTOR:
			case M_CITIZEN3: case M_CITIZEN4:
				eSoundID = SEID_WOM_OOF; break;
			case M_HALPH: break; //not supported
			case M_HALPH2: eSoundID = SEID_HALPH2_OOF; break;
			case M_SLAYER: case M_SLAYER2:
				eSoundID = SEID_SLAYER_OOF; break;
			case M_STALWART:
				eSoundID = SEID_STALWART_OOF; break;
			case M_STALWART2:
				eSoundID = SEID_SOLDIER_OOF; break;
			default: eSoundID = SEID_MON_OOF; break;
		}
		if (g_pTheSound->IsSoundEffectPlaying(eSoundID))
			eSoundID = SEID_NONE;
	}
	else if (CueEvents.HasOccurred(CID_Scared))
	{
		switch (this->pCurrentGame->swordsman.wAppearance)
		{
			case M_CLONE: case M_DECOY: case M_MIMIC:
			case M_BEETHRO: case M_BEETHRO_IN_DISGUISE: eSoundID = SEID_SCARED; break;
			case M_GUNTHRO: eSoundID = SEID_GB_SCARED; break;
			case M_NEATHER: eSoundID = SEID_NSCARED; break;
			case M_GOBLIN:	case M_GOBLINKING: eSoundID = SEID_GOB_SCARED; break;
			case M_TARBABY: case M_MUDBABY: case M_GELBABY:
			case M_TARMOTHER: case M_MUDMOTHER: case M_GELMOTHER:
				eSoundID = SEID_TAR_SCARED; break;
			case M_ROCKGOLEM: case M_ROCKGIANT: eSoundID = SEID_ROCK_SCARED; break;
			case M_CITIZEN1: case M_CITIZEN2: case M_GUARD:
			case M_MUDCOORDINATOR: case M_TARTECHNICIAN:
			case M_CITIZEN: eSoundID = SEID_CIT_SCARED; break;
			case M_WUBBA: eSoundID = SEID_WUBBA; break;
			case M_NEGOTIATOR:
				eSoundID = SEID_NEGO_SCARED; break;
			case M_INSTRUCTOR:
			case M_CITIZEN3: case M_CITIZEN4:
				eSoundID = SEID_WOM_SCARED; break;
			case M_HALPH: break; //not supported
			case M_HALPH2: eSoundID = SEID_HALPH2_SCARED; break;
			case M_SLAYER: case M_SLAYER2:
				eSoundID = SEID_SLAYER_SCARED; break;
			case M_STALWART:
				eSoundID = SEID_STALWART_SCARED; break;
			case M_STALWART2:
				eSoundID = SEID_SOLDIER_SCARED; break;
			default: eSoundID = SEID_MON_OOF; break;
		}
	}
	else if (CueEvents.HasOccurred(CID_SwordsmanTired))
	{
		UINT appearance = this->pCurrentGame->swordsman.wAppearance;
		if (appearance == M_GUNTHRO) {
			eSoundID = SEID_GB_TIRED;
		} else if (bIsBeethroDouble(appearance)) {
			eSoundID = SEID_TIRED;
		}
	}
	if (eSoundID != (UINT)SEID_NONE)
		PlaySpeakerSoundEffect(eSoundID);

	//Apply bump effect
	if (CueEvents.HasOccurred(CID_HitObstacle))
	{
		const CMoveCoord *pMoveCoord = DYN_CAST(const CMoveCoord*, const CAttachableObject*,
				CueEvents.GetFirstPrivateData(CID_HitObstacle) );
		ASSERT(pMoveCoord);
		this->pRoomWidget->AddMLayerEffect(new CBumpObstacleEffect(this->pRoomWidget,
				pMoveCoord->wX, pMoveCoord->wY, pMoveCoord->wO));
	}
	//Update minimap
	if (CueEvents.HasOccurred(CID_RoomConquerPending))
	{
		this->pMapWidget->DrawMapSurfaceFromRoom(this->pCurrentGame->pRoom);
		this->pMapWidget->RequestPaint();
	}

	//Channel n+2 -- 'Neather's voice / Slayer.
	if (CueEvents.HasOccurred(CID_NeatherScared))
	{
		const CMonster *pNeather = DYN_CAST(const CMonster*, const CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_NeatherScared) );
		ASSERT(pNeather);
		this->fPos[0] = static_cast<float>(pNeather->wX);
		this->fPos[1] = static_cast<float>(pNeather->wY);
		PlaySpeakerSoundEffect(SEID_NSCARED, this->fPos);
	}
	else if (CueEvents.HasOccurred(CID_NeatherFrustrated))
	{
		const CMonster *pNeather = DYN_CAST(const CMonster*, const CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_NeatherFrustrated) );
		ASSERT(pNeather);
		this->fPos[0] = static_cast<float>(pNeather->wX);
		this->fPos[1] = static_cast<float>(pNeather->wY);
		PlaySpeakerSoundEffect(SEID_NFRUSTRATED, this->fPos);
	}
	else if (CueEvents.HasOccurred(CID_NeatherLaughing))
	{
		const CMonster *pNeather = DYN_CAST(const CMonster*, const CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_NeatherLaughing) );
		ASSERT(pNeather);
		this->fPos[0] = static_cast<float>(pNeather->wX);
		this->fPos[1] = static_cast<float>(pNeather->wY);
		PlaySpeakerSoundEffect(SEID_NLAUGHING, this->fPos);
	}
	else if (CueEvents.HasOccurred(CID_SlayerEntered))
	{
		const CMonster *pSlayer = DYN_CAST(const CMonster*, const CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_SlayerEntered) );
		ASSERT(pSlayer);
		this->fPos[0] = static_cast<float>(pSlayer->wX);
		this->fPos[1] = static_cast<float>(pSlayer->wY);
		const bool bNear = pSlayer->DistToSwordsman(false) < 5;
		const bool bSlayer1 = pSlayer->wType == M_SLAYER;
		PlaySpeakerSoundEffect(bNear ?
				(bSlayer1 ? SEID_SLAYERENTERNEAR : SEID_SLAYER2ENTERNEAR) :
				(bSlayer1 ? SEID_SLAYERENTERFAR : SEID_SLAYER2ENTERFAR),
				this->fPos);
	}
	else if (CueEvents.HasOccurred(CID_SlayerCombat))
	{
		const CMonster *pSlayer = DYN_CAST(const CMonster*, const CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_SlayerCombat) );
		PlaySpeakerSoundEffect(pSlayer->wType == M_SLAYER ? SEID_SLAYERCOMBAT : SEID_SLAYER2COMBAT);
	}

	//Halph dialogue.
	if (CueEvents.HasOccurred(CID_HalphEntered))
	{
		const CMonster *pHalph = DYN_CAST(const CMonster*, const CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_HalphEntered) );
		PlaySpeakerSoundEffect(pHalph->wType == M_HALPH ? SEID_HALPHENTERED : SEID_HALPH2ENTERED);
	}
	else if (CueEvents.HasOccurred(CID_HalphFollowing))
	{
		const CMonster *pHalph = DYN_CAST(const CMonster*, const CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_HalphFollowing) );
		PlaySpeakerSoundEffect(pHalph->wType == M_HALPH ? SEID_HALPHFOLLOWING : SEID_HALPH2FOLLOWING);
	}
	else if (CueEvents.HasOccurred(CID_HalphWaiting))
	{
		const CMonster *pHalph = DYN_CAST(const CMonster*, const CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_HalphWaiting) );
		PlaySpeakerSoundEffect(pHalph->wType == M_HALPH ? SEID_HALPHWAITING : SEID_HALPH2WAITING);
	}
	else if (CueEvents.HasOccurred(CID_HalphStriking))
	{
		const CMonster *pHalph = DYN_CAST(const CMonster*, const CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_HalphStriking) );
		PlaySpeakerSoundEffect(pHalph->wType == M_HALPH ? SEID_HALPHSTRIKING : SEID_HALPH2STRIKING);
	}
	else if (CueEvents.HasOccurred(CID_HalphCantOpen))
	{
		const CMonster *pHalph = DYN_CAST(const CMonster*, const CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_HalphCantOpen) );
		PlaySpeakerSoundEffect(pHalph->wType == M_HALPH ? SEID_HALPHCANTOPEN : SEID_HALPH2CANTOPEN);
	}
	else if (CueEvents.HasOccurred(CID_HalphInterrupted))
	{
		const CMonster *pHalph = DYN_CAST(const CMonster*, const CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_HalphInterrupted) );
		PlaySpeakerSoundEffect(pHalph->wType == M_HALPH ? SEID_HALPHINTERRUPTED : SEID_HALPH2INTERRUPTED);
	}
	else if (CueEvents.HasOccurred(CID_HalphHurryUp))
	{
		const CMonster *pHalph = DYN_CAST(const CMonster*, const CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_HalphHurryUp) );
		PlaySpeakerSoundEffect(pHalph->wType == M_HALPH ? SEID_HALPHHURRYUP : SEID_HALPH2HURRYUP);
	}

	//Handle dynamically-allocated sound channels -- give most important sounds priority.

	//1st. Player's actions.
	if (CueEvents.HasOccurred(CID_SwingSword))
	{
		PlaySoundEffect(SEID_SWING);
		const CAttachableWrapper<UINT> *pOrientation =
				DYN_CAST(const CAttachableWrapper<UINT>*, const CAttachableObject*,
				CueEvents.GetFirstPrivateData(CID_SwingSword));
		ASSERT(pOrientation);
		this->pRoomWidget->AddMLayerEffect(
				new CSwordSwingEffect(this->pRoomWidget,
					this->pCurrentGame->swordsman, static_cast<UINT>(*pOrientation)));
	}
	if (CueEvents.HasOccurred(CID_Step))
	{
		PlaySoundEffect(this->pCurrentGame && !bIsHuman(this->pCurrentGame->swordsman.wAppearance) ?
				SEID_WALK_MONSTER : SEID_WALK);
	}
	if (CueEvents.HasOccurred(CID_StepOnScroll))
		PlaySoundEffect(SEID_READ);
	if (CueEvents.HasOccurred(CID_DrankPotion))
		PlaySoundEffect(SEID_POTION);
	if (CueEvents.HasOccurred(CID_DoublePlaced))
	{
		PlaySoundEffect(SEID_MIMIC);
		this->pRoomWidget->RenderRoomInPlay(); //remove double placement effect
	}
	if (CueEvents.HasOccurred(CID_CompleteLevel))
	{
		this->pMapWidget->UpdateFromCurrentGame();
		this->pMapWidget->RequestPaint();
	}
	if (CueEvents.HasOccurred(CID_CheckpointActivated) &&
			this->pRoomWidget->AreCheckpointsVisible())
	{
		const CCoord *pCoord = DYN_CAST(const CCoord*, const CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_CheckpointActivated) );
		this->pRoomWidget->AddTLayerEffect(
				new CCheckpointEffect(this->pRoomWidget, *pCoord));
		PlaySoundEffect(SEID_CHECKPOINT);
	}
	if (CueEvents.HasOccurred(CID_OrbActivatedByPlayer))
	{
		pObj = CueEvents.GetFirstPrivateData(CID_OrbActivatedByPlayer);
		if (!pObj)
			PlaySoundEffect(SEID_ORBHIT);   //just play sound
		else while (pObj)
		{
			const COrbData *pOrbData = DYN_CAST(const COrbData*, const CAttachableObject*, pObj);
			//If player hits orb, center sound on player for nicer effect.
			if (this->pCurrentGame->swordsman.wSwordX == pOrbData->wX &&
					this->pCurrentGame->swordsman.wSwordY == pOrbData->wY)
			{
				this->fPos[0] = static_cast<float>(this->pCurrentGame->swordsman.wX);
				this->fPos[1] = static_cast<float>(this->pCurrentGame->swordsman.wY);
			} else {
				this->fPos[0] = static_cast<float>(pOrbData->wX);
				this->fPos[1] = static_cast<float>(pOrbData->wY);
			}
			PlaySoundEffect(pOrbData->eType == OT_BROKEN ?
					SEID_ORBBROKE : SEID_ORBHIT, this->fPos);
			this->pRoomWidget->AddStrikeOrbEffect(*pOrbData);
			pObj = CueEvents.GetNextPrivateData();
		}
	}
	if (CueEvents.HasOccurred(CID_Tunnel))
		PlaySoundEffect(SEID_TUNNEL);
	if (CueEvents.HasOccurred(CID_Horn_Squad))
		PlaySoundEffect(SEID_HORN_SQUAD);
	if (CueEvents.HasOccurred(CID_Horn_Soldier))
		PlaySoundEffect(SEID_HORN_SOLDIER);
	if (CueEvents.HasOccurred(CID_HornFail))
		PlaySoundEffect(SEID_HORN_FAIL);

	//2nd. Important room events.
	if (CueEvents.HasOccurred(CID_RedGatesToggled) ||
			CueEvents.GetFirstPrivateData(CID_RoomConquerPending) || //attached data indicates green door toggle
			CueEvents.HasOccurred(CID_BlackGatesToggled))
		PlaySoundEffect(SEID_DOOROPEN);

	static bool bWispOnPlayer = false;
	if (CueEvents.HasOccurred(CID_WispOnPlayer))
	{
		//Sound wisp only on initial contact.
		if (!bWispOnPlayer)
		{
			PlaySoundEffect(SEID_WISP);
			bWispOnPlayer = true;
		}
	}
	else bWispOnPlayer = false;

	if (CueEvents.HasOccurred(CID_AllBrainsRemoved))
		PlaySoundEffect(SEID_LASTBRAIN);

	if (CueEvents.HasOccurred(CID_PlayerFrozen))
	{
		pObj = CueEvents.GetFirstPrivateData(CID_PlayerFrozen);
		ASSERT(pObj);

		//Several of these could overlap.
		//Don't allow placing more than one sound per tile.
		CCoordSet coords;
		while (pObj)
		{
			const CCoord *pCoord = DYN_CAST(const CCoord*, const CAttachableObject*, pObj);
			coords.insert(pCoord->wX, pCoord->wY);
			pObj = CueEvents.GetNextPrivateData();
		}

		bool bFreezeOneNPC = true;
		for (CCoordSet::const_iterator coord=coords.begin(); coord!=coords.end(); ++coord)
		{
			//To prevent slowdown, only play one sound at a non-player position.
			if (!this->pCurrentGame->IsPlayerAt(coord->wX, coord->wY))
			{
				if (!bFreezeOneNPC)
					continue;
				bFreezeOneNPC = false;
			}

			this->fPos[0] = static_cast<float>(coord->wX);
			this->fPos[1] = static_cast<float>(coord->wY);
			PlaySoundEffect(SEID_FROZEN, this->fPos);
		}
	}

	//3nd. Player actions (possibly multiple instances).
	if (CueEvents.HasOccurred(CID_TokenToggled))
		PlaySoundEffect(SEID_MIMIC);   //SEID_TOKEN

	for (pObj = CueEvents.GetFirstPrivateData(CID_MonsterDiedFromStab);
			pObj != NULL; pObj = CueEvents.GetNextPrivateData())
	{
		const CMonster *pMonster = DYN_CAST(const CMonster*, const CAttachableObject*, pObj);
		const UINT wO = IsValidOrientation(pMonster->wProcessSequence) ?
				pMonster->wProcessSequence : NO_ORIENTATION;
		CMoveCoord coord(pMonster->wX,pMonster->wY,wO);
		AddDamageEffect(pMonster->GetIdentity(), coord);
	}
	for (pObj = CueEvents.GetFirstPrivateData(CID_TrapDoorRemoved);
			pObj != NULL; pObj = CueEvents.GetNextPrivateData())
	{
		const CCoord *pCoord = DYN_CAST(const CCoord*, const CAttachableObject*, pObj);
		//If player drops trapdoor, center sound on player for nicer effect.
		CSwordsman& player = this->pCurrentGame->swordsman;
		if (player.wPrevX == pCoord->wX && player.wPrevY == pCoord->wY)
		{
			this->fPos[0] = static_cast<float>(player.wX);
			this->fPos[1] = static_cast<float>(player.wY);
		} else {
			this->fPos[0] = static_cast<float>(pCoord->wX);
			this->fPos[1] = static_cast<float>(pCoord->wY);
		}
		PlaySoundEffect(SEID_TRAPDOOR, this->fPos);
		const UINT oTile = this->pCurrentGame->pRoom->GetOSquare(pCoord->wX, pCoord->wY);
		if (bIsPit(oTile))
		{
			static const vector<UINT> trapdoorTile(1, TI_TRAPDOOR_F);
			this->pRoomWidget->AddOLayerEffect(
					new CTrapdoorFallEffect(this->pRoomWidget, *pCoord, trapdoorTile,
						player.bIsHasted ? 260 : 130));
		} else if (bIsWater(oTile)) {
			PlaySoundEffect(SEID_SPLASH, this->fPos);
			this->pRoomWidget->AddTLayerEffect(
					new CSplashEffect(this->pRoomWidget, *pCoord));
		}
	}
	if (CueEvents.HasOccurred(CID_ObjectBuilt))
	{
		this->fPos[0] = static_cast<float>(this->pCurrentGame->swordsman.wX);
		this->fPos[1] = static_cast<float>(this->pCurrentGame->swordsman.wY);
		PlaySoundEffect(SEID_TRAPDOOR, this->fPos);
	}
	if (CueEvents.HasOccurred(CID_ObjectFell))
	{
		TilesMap fallingTiles;
		pObj = CueEvents.GetFirstPrivateData(CID_ObjectFell);
		while (pObj)
		{
			//Show object as it falls.
			const CMoveCoordEx *pCoord = DYN_CAST(const CMoveCoordEx*, const CAttachableObject*, pObj);
			
			UINT wTileNo;
			if (pCoord->wValue >= M_OFFSET)
			{
				const UINT eLogicalType = pCoord->wValue - M_OFFSET;
				UINT eMonsterType = eLogicalType; //same by default

				//Look up base type of custom character.
				if (eMonsterType >= CUSTOM_CHARACTER_FIRST)
				{
					HoldCharacter *pChar = this->pCurrentGame->pHold->GetCharacter(eLogicalType);
					if (pChar)
						eMonsterType = pChar->wType;
					else
						eMonsterType = M_CITIZEN1;
				}

				wTileNo = this->pRoomWidget->GetEntityTile(eMonsterType, eLogicalType, pCoord->wO, 0);
			}
			else if (bIsSerpentTile(pCoord->wValue))
				wTileNo = GetTileImageForSerpentPiece(pCoord->wO, pCoord->wValue);
			else {
				wTileNo = GetTileImageForTileNo(pCoord->wValue);
				if (bIsBriar(pCoord->wValue))
				{
					//Examining the room state to render the correct briar shape
					//is no longer valid.  But we can look at the prior tiles
					//rendered to the room to get this information.
					const UINT index = this->pCurrentGame->pRoom->ARRAYINDEX(pCoord->wX,pCoord->wY);
					ASSERT(index < this->pCurrentGame->pRoom->CalcRoomArea());
					const UINT tile = this->pRoomWidget->pwTSquareTI[index];
					if (bIsBriarTI(tile)) //robustness check
						wTileNo = tile;
				}
				if (wTileNo == CALC_NEEDED)
					wTileNo = CalcTileImageFor(this->pCurrentGame->pRoom, pCoord->wValue,
							pCoord->wX, pCoord->wY);
			}
			fallingTiles[ROOMCOORD(pCoord->wX, pCoord->wY)].push_back(wTileNo);
			pObj = CueEvents.GetNextPrivateData();
		}

		UINT wSoundCount=0;
		for (TilesMap::iterator tiles=fallingTiles.begin(); tiles!=fallingTiles.end(); ++tiles)
		{
			const CCoord coord(tiles->first);
			this->pRoomWidget->AddOLayerEffect(
					new CTrapdoorFallEffect(this->pRoomWidget, coord, tiles->second,
					this->pCurrentGame->swordsman.bIsHasted ? 260 : 130));
			if (wSoundCount++ < 3)
			{
				this->fPos[0] = static_cast<float>(coord.wX);
				this->fPos[1] = static_cast<float>(coord.wY);
				PlaySoundEffect(SEID_FALLING, this->fPos);
			}
		}
	}

	if (CueEvents.HasOccurred(CID_PressurePlate))
	{
		pObj = CueEvents.GetFirstPrivateData(CID_PressurePlate);
		if (!pObj)
			PlaySoundEffect(SEID_PRESSPLATE);   //just play sound
		else while (pObj)
		{
			const COrbData *pOrbData = DYN_CAST(const COrbData*, const CAttachableObject*, pObj);
			this->fPos[0] = static_cast<float>(pOrbData->wX);
			this->fPos[1] = static_cast<float>(pOrbData->wY);
			PlaySoundEffect(SEID_PRESSPLATE, this->fPos);
			this->pRoomWidget->AddStrikeOrbEffect(*pOrbData, false);
			pObj = CueEvents.GetNextPrivateData();
		}
	}
	if (CueEvents.HasOccurred(CID_PressurePlateReleased))
	{
		pObj = CueEvents.GetFirstPrivateData(CID_PressurePlateReleased);
		if (!pObj)
			PlaySoundEffect(SEID_PRESSPLATEUP);   //just play sound
		else while (pObj)
		{
			const COrbData *pOrbData = DYN_CAST(const COrbData*, const CAttachableObject*, pObj);
			this->fPos[0] = static_cast<float>(pOrbData->wX);
			this->fPos[1] = static_cast<float>(pOrbData->wY);
			PlaySoundEffect(SEID_PRESSPLATEUP, this->fPos);
			this->pRoomWidget->AddStrikeOrbEffect(*pOrbData, false);
			pObj = CueEvents.GetNextPrivateData();
		}
	}

	for (pObj = CueEvents.GetFirstPrivateData(CID_CrumblyWallDestroyed);
			pObj != NULL; pObj = CueEvents.GetNextPrivateData())
	{
		const CMoveCoord *pCoord = DYN_CAST(const CMoveCoord*, const CAttachableObject*, pObj);
		//If player stabs monster, center sound on player for nicer effect.
		if (this->pCurrentGame->swordsman.wSwordX == pCoord->wX &&
				this->pCurrentGame->swordsman.wSwordY == pCoord->wY)
		{
			this->fPos[0] = static_cast<float>(this->pCurrentGame->swordsman.wX);
			this->fPos[1] = static_cast<float>(this->pCurrentGame->swordsman.wY);
		} else {
			this->fPos[0] = static_cast<float>(pCoord->wX);
			this->fPos[1] = static_cast<float>(pCoord->wY);
		}
		PlaySoundEffect(SEID_BREAKWALL, this->fPos);
		this->pRoomWidget->AddTLayerEffect(
				new CDebrisEffect(this->pRoomWidget, *pCoord, 10,
						GetEffectDuration(5), GetParticleSpeed(4)));
		this->pRoomWidget->AddTLayerEffect(
				new CVerminEffect(this->pRoomWidget, *pCoord));
	}
	for (pObj = CueEvents.GetFirstPrivateData(CID_MonsterPieceStabbed);
			pObj != NULL; pObj = CueEvents.GetNextPrivateData())
	{
		const CMoveCoord *pCoord = DYN_CAST(const CMoveCoord*, const CAttachableObject*, pObj);
		UINT wMonsterType = M_SERPENT; //default
		//If a golem is at the target square, that means a rock giant was shattered.
		if (this->pCurrentGame->pRoom->IsMonsterOfTypeAt(M_ROCKGOLEM, pCoord->wX, pCoord->wY))
			wMonsterType = M_ROCKGIANT;
		AddDamageEffect(wMonsterType, *pCoord);
	}
	for (pObj = CueEvents.GetFirstPrivateData(CID_WubbaStabbed);
			pObj != NULL; pObj = CueEvents.GetNextPrivateData())
	{
		const CMonster *pMonster = DYN_CAST(const CMonster*, const CAttachableObject*, pObj);
		//If player stabs monster, center sound on player for nicer effect.
		if (this->pCurrentGame->swordsman.wSwordX == pMonster->wX &&
			this->pCurrentGame->swordsman.wSwordY == pMonster->wY)
		{
			this->fPos[0] = static_cast<float>(this->pCurrentGame->swordsman.wX);
			this->fPos[1] = static_cast<float>(this->pCurrentGame->swordsman.wY);
		} else {
			this->fPos[0] = static_cast<float>(pMonster->wX);
			this->fPos[1] = static_cast<float>(pMonster->wY);
		}
		PlaySoundEffect(SEID_WUBBA, this->fPos);
	}
	if (CueEvents.HasOccurred(CID_OrbActivated))
	{
		pObj = CueEvents.GetFirstPrivateData(CID_OrbActivated);
		if (!pObj)
			PlaySoundEffect(SEID_ORBHITQUIET);   //just play sound
		else while (pObj)
		{
			const COrbData *pOrbData = DYN_CAST(const COrbData*, const CAttachableObject*, pObj);
			this->fPos[0] = static_cast<float>(pOrbData->wX);
			this->fPos[1] = static_cast<float>(pOrbData->wY);
			PlaySoundEffect(pOrbData->eType == OT_BROKEN ?
					SEID_ORBBROKE : SEID_ORBHITQUIET, this->fPos);
			this->pRoomWidget->AddStrikeOrbEffect(*pOrbData);
			pObj = CueEvents.GetNextPrivateData();
		}
	}
	for (pObj = CueEvents.GetFirstPrivateData(CID_OrbDamaged);
			pObj != NULL; pObj = CueEvents.GetNextPrivateData())
	{
		const COrbData *pOrbData = DYN_CAST(const COrbData*, const CAttachableObject*, pObj);
		this->fPos[0] = static_cast<float>(pOrbData->wX);
		this->fPos[1] = static_cast<float>(pOrbData->wY);
		PlaySoundEffect(SEID_ORBBROKE, this->fPos);
	}
	for (pObj = CueEvents.GetFirstPrivateData(CID_MirrorShattered);
			pObj != NULL; pObj = CueEvents.GetNextPrivateData())
	{
		const CMoveCoord *pCoord = DYN_CAST(const CMoveCoord*, const CAttachableObject*, pObj);
		if (this->pCurrentGame->swordsman.wSwordX == pCoord->wX &&
			this->pCurrentGame->swordsman.wSwordY == pCoord->wY)
		{
			this->fPos[0] = static_cast<float>(this->pCurrentGame->swordsman.wX);
			this->fPos[1] = static_cast<float>(this->pCurrentGame->swordsman.wY);
		} else {
			this->fPos[0] = static_cast<float>(pCoord->wX);
			this->fPos[1] = static_cast<float>(pCoord->wY);
		}
		PlaySoundEffect(SEID_SHATTER, this->fPos);
		this->pRoomWidget->AddTLayerEffect(
				new CDebrisEffect(this->pRoomWidget, *pCoord, 10,
						GetEffectDuration(7), GetParticleSpeed(4))); //!!ShatteringGlass effect
	}
	if (CueEvents.HasOccurred(CID_BombExploded))
	{
		UINT wRandMod = CueEvents.GetOccurrenceCount(CID_BombExploded);
		UINT wRandMax = 25 + (25 * g_pTheBM->eyeCandy);

		pObj = CueEvents.GetFirstPrivateData(CID_BombExploded);
		while (pObj)
		{
			const CCoord *pCoord = DYN_CAST(const CCoord*, const CAttachableObject*, pObj);
			if (RAND(wRandMod) < wRandMax)
			{
				CMoveCoord moveCoord(pCoord->wX, pCoord->wY, NO_ORIENTATION);
				this->pRoomWidget->AddTLayerEffect(
						new CDebrisEffect(this->pRoomWidget, moveCoord, 3,
								GetEffectDuration(7), GetParticleSpeed(4)));
				this->fPos[0] = static_cast<float>(pCoord->wX);
				this->fPos[1] = static_cast<float>(pCoord->wY);
				PlaySoundEffect(SEID_BOMBEXPLODE, this->fPos);
			}
			pObj = CueEvents.GetNextPrivateData();
		}
	}
	if (CueEvents.HasOccurred(CID_OrbActivatedByDouble))
	{
		pObj = CueEvents.GetFirstPrivateData(CID_OrbActivatedByDouble);
		if (!pObj)
			PlaySoundEffect(SEID_ORBHITQUIET);   //just play sound
		else while (pObj)
		{
			const COrbData *pOrbData = DYN_CAST(const COrbData*, const CAttachableObject*, pObj);
			this->fPos[0] = static_cast<float>(pOrbData->wX);
			this->fPos[1] = static_cast<float>(pOrbData->wY);
			const bool bOrb = this->pCurrentGame->pRoom->GetTSquare(
					pOrbData->wX, pOrbData->wY) == T_ORB; //as opposed to a pressure plate
			PlaySoundEffect(bOrb ?
					(pOrbData->eType == OT_BROKEN ? SEID_ORBBROKE : SEID_ORBHITQUIET) :
					SEID_PRESSPLATE, this->fPos);
			this->pRoomWidget->AddStrikeOrbEffect(*pOrbData, bOrb);
			pObj = CueEvents.GetNextPrivateData();
		}
	}
	if (CueEvents.HasOccurred(CID_TarstuffDestroyed))
	{
		//Green/clean map room might need to be reverted back to red
		//if tar was hit and tar babies were created in a clean room.
		//This can be accomplished by redrawing the room whenever tar
		//changes configuration.
		for (pObj = CueEvents.GetFirstPrivateData(CID_TarstuffDestroyed);
				pObj != NULL; pObj = CueEvents.GetNextPrivateData())
		{
			const CMoveCoordEx *pCoord = DYN_CAST(const CMoveCoordEx*, const CAttachableObject*, pObj);
			//If player stabs tar, center sound on player for nicer effect.
			if (this->pCurrentGame->swordsman.wSwordX == pCoord->wX &&
				this->pCurrentGame->swordsman.wSwordY == pCoord->wY)
			{
				this->fPos[0] = static_cast<float>(this->pCurrentGame->swordsman.wX);
				this->fPos[1] = static_cast<float>(this->pCurrentGame->swordsman.wY);
			} else {
				this->fPos[0] = static_cast<float>(pCoord->wX);
				this->fPos[1] = static_cast<float>(pCoord->wY);
			}
			PlaySoundEffect(SEID_STABTAR, this->fPos);
			switch (pCoord->wValue)
			{
				case T_TAR:
					this->pRoomWidget->AddTLayerEffect(
							new CTarStabEffect(this->pRoomWidget, *pCoord,
									GetEffectDuration(7), GetParticleSpeed(4)));
				break;
				case T_MUD:
					this->pRoomWidget->AddTLayerEffect(
							new CMudStabEffect(this->pRoomWidget, *pCoord,
									GetEffectDuration(7), GetParticleSpeed(4)));
				break;
				case T_GEL:
					this->pRoomWidget->AddTLayerEffect(
							new CGelStabEffect(this->pRoomWidget, *pCoord,
									GetEffectDuration(7), GetParticleSpeed(4)));
				break;
				default: ASSERT(!"Invalid tar type"); break;
			}
		}

		//Once tar has been stabbed in a clean room, change the music style.
		if (wMood == SONG_AMBIENT)
			SetMusicStyle();
	}
	if (CueEvents.HasOccurred(CID_StepOnScroll))
	{
		this->pFaceWidget->SetReading(true);
		const CDbMessageText *pScrollText = DYN_CAST(const CDbMessageText*, const CAttachableObject*,
				CueEvents.GetFirstPrivateData(CID_StepOnScroll));
		ASSERT((const WCHAR*)(*pScrollText));
		this->pScrollLabel->SetText((const WCHAR*)(*pScrollText));
		ShowScroll();
	}

	//Remove old sparks before drawing the current ones.
	if (//Leave sparks burning while double is being placed.
		(!this->pCurrentGame->swordsman.wPlacingDoubleType ||
				CueEvents.HasOccurred(CID_DrankPotion)) &&
				!CueEvents.HasOccurred(CID_DoublePlaced))
		this->pRoomWidget->RemoveTLayerEffectsOfType(ESPARK);

	//Spark rendering must come both before and after room is drawn so it will
	//show up correctly both on room entrance and  in double-placing freeze frame.
	if (!bPlayerLeftRoom)
		ProcessFuseBurningEvents(CueEvents);

	//3rd. Monster actions.
	for (pObj = CueEvents.GetFirstPrivateData(CID_SnakeDiedFromTruncation);
			pObj != NULL; pObj = CueEvents.GetNextPrivateData())
	{
		const CMonster *pMonster = DYN_CAST(const CMonster*, const CAttachableObject*, pObj);
		CMoveCoord coord(pMonster->wX,pMonster->wY,pMonster->wO);
		AddDamageEffect(pMonster->GetIdentity(), coord);
	}
	for (pObj = CueEvents.GetFirstPrivateData(CID_EvilEyeWoke);
			pObj != NULL; pObj = CueEvents.GetNextPrivateData())
	{
		const CMoveCoord *pCoord = DYN_CAST(const CMoveCoord*, const CAttachableObject*, pObj);
		this->fPos[0] = static_cast<float>(pCoord->wX);
		this->fPos[1] = static_cast<float>(pCoord->wY);
		PlaySoundEffect(SEID_EVILEYEWOKE, this->fPos);
		this->pRoomWidget->AddMLayerEffect(new CEvilEyeGazeEffect(
				this->pRoomWidget,pCoord->wX,pCoord->wY,pCoord->wO, 500));
	}
	for (pObj = CueEvents.GetFirstPrivateData(CID_MonsterBurned);
			pObj != NULL; pObj = CueEvents.GetNextPrivateData())
	{
		const CMoveCoord *pMoveCoord = DYN_CAST(const CMoveCoord*, const CAttachableObject*, pObj);
		this->fPos[0] = static_cast<float>(pMoveCoord->wX);
		this->fPos[1] = static_cast<float>(pMoveCoord->wY);
		PlaySoundEffect(SEID_SIZZLE, this->fPos);
		this->pRoomWidget->AddMLayerEffect(
				new CSteamEffect(this->pRoomWidget, *pMoveCoord));
	}
	for (pObj = CueEvents.GetFirstPrivateData(CID_FegundoToAsh);
			pObj != NULL; pObj = CueEvents.GetNextPrivateData())
	{
		const CMonster *pMonster = DYN_CAST(const CMonster*, const CAttachableObject*, pObj);
		this->fPos[0] = static_cast<float>(pMonster->wX);
		this->fPos[1] = static_cast<float>(pMonster->wY);
		PlaySoundEffect(SEID_SIZZLE, this->fPos);
		this->pRoomWidget->AddMLayerEffect(
				new CSteamEffect(this->pRoomWidget, *pMonster));
	}
	for (pObj = CueEvents.GetFirstPrivateData(CID_Splash);
			pObj != NULL; pObj = CueEvents.GetNextPrivateData())
	{
		const CCoord *pCoord = DYN_CAST(const CCoord*, const CAttachableObject*, pObj);
		this->fPos[0] = static_cast<float>(pCoord->wX);
		this->fPos[1] = static_cast<float>(pCoord->wY);
		PlaySoundEffect(SEID_SPLASH, this->fPos);
		const UINT oTile = this->pCurrentGame->pRoom->GetOSquare(pCoord->wX, pCoord->wY);
		if (bIsWater(oTile) || bIsSteppingStone(oTile))
			this->pRoomWidget->AddTLayerEffect(
					new CSplashEffect(this->pRoomWidget, *pCoord));
	}
	UINT count=0;
	static const UINT MAX_WADE_SOUNDS = 3;
	for (pObj = CueEvents.GetFirstPrivateData(CID_Wade);
			pObj != NULL && count < MAX_WADE_SOUNDS; pObj = CueEvents.GetNextPrivateData(), ++count)
	{
		const CCoord *pCoord = DYN_CAST(const CCoord*, const CAttachableObject*, pObj);
		this->fPos[0] = static_cast<float>(pCoord->wX);
		this->fPos[1] = static_cast<float>(pCoord->wY);
		PlaySoundEffect(SEID_WADE, this->fPos);
		const UINT oTile = this->pCurrentGame->pRoom->GetOSquare(pCoord->wX, pCoord->wY);
		if (bIsWater(oTile))
			this->pRoomWidget->AddTLayerEffect(
					new CWadeEffect(this->pRoomWidget, *pCoord));
	}

	//Events w/o accompanying sounds.
	if (CueEvents.HasOccurred(CID_ConquerRoom))
	{
		this->pMapWidget->DrawMapSurfaceFromRoom(this->pCurrentGame->pRoom);
		this->pMapWidget->RequestPaint();
	}
	if (CueEvents.HasOccurred(CID_Explosion))
	{
		//Since several explosions could overlap, don't allow placing
		//more than one effect on each tile.
		CCoordSet coords;
		for (pObj = CueEvents.GetFirstPrivateData(CID_Explosion);
				pObj != NULL; pObj = CueEvents.GetNextPrivateData())
		{
			const CCoord *pCoord = DYN_CAST(const CCoord*, const CAttachableObject*, pObj);
			coords.insert(pCoord->wX, pCoord->wY);
		}

		//There still might be a very large number of explosions.
		//Don't show more than can be handled.

		static const UINT max_tiles_per_bomb_explosion = 7*7;
		UINT wRandMod = coords.size();
		UINT wRandMax = max_tiles_per_bomb_explosion + (2 * max_tiles_per_bomb_explosion * g_pTheBM->eyeCandy);

		for (CCoordSet::const_iterator coord=coords.begin(); coord!=coords.end(); ++coord)
		{
			if (RAND(wRandMod) < wRandMax)
				this->pRoomWidget->AddTLayerEffect(
						new CExplosionEffect(this->pRoomWidget, CCoord(coord->wX, coord->wY),
								GetEffectDuration(500)));
		}
	}

	if (CueEvents.HasOccurred(CID_MonsterSpoke))
		//complete turn animation immediately in preparation for question
		this->pRoomWidget->FinishMoveAnimation();

	//Update room lights as needed.
	const bool bLightToggled = CueEvents.HasOccurred(CID_LightToggled);
	if (bLightToggled)
		this->pRoomWidget->RenderRoomLighting();
	this->pRoomWidget->AddPlayerLight();

	//
	//Begin section where room load can occur.  If room load occurs then
	//original cue events from command before room load will be discarded, and cue
	//events from first step into room will be in CueEvents.  Original cue events
	//should be handled before this section or stored in variables for handling
	//after this section.
	//

	//Handle player dying.
	if (bPlayerDied)
	{
		//Update tile image arrays before death sequence.
		if (CueEvents.HasOccurred(CID_Plots) || bLightToggled)
		{
			const CCoordSet *pSet = DYN_CAST(const CCoordSet*, const CAttachableObject*,
					CueEvents.GetFirstPrivateData(CID_Plots) );
			this->pRoomWidget->UpdateFromPlots(pSet, &this->pCurrentGame->pRoom->geometryChanges);
		}
		const bool bDeathNotUndone = HandleEventsForPlayerDeath(CueEvents);
		CueEvents.Clear();	//clear after death sequence (whether move is undone or not)
		//but before room restart so speech on room start can be retained

		//Repaint the sign in case demo recording ended.
		SetSignTextToCurrentRoom();
		PaintSign();

		ASSERT(!(this->pCurrentGame->bIsGameActive && bDeathNotUndone));
		if (GetScreenType() == SCR_Demo) return eNextScreen;	//stop showing demo on death
		if (bDeathNotUndone)
		{
			//Create demo/End demo recording if game is now inactive.
			if (this->pCurrentGame->IsDemoRecording())  //End a demo that is recording.
			{
				if (!this->pCurrentGame->EndDemoRecording())
				{
					ASSERT(!"Failed to save a demo when recording ended.");
				}
				SetSignTextToCurrentRoom();
				PaintSign();
			}
			else if ((this->pCurrentGame->GetAutoSaveOptions() & ASO_DIEDEMO) == ASO_DIEDEMO)
				this->pCurrentGame->WriteCurrentRoomDieDemo();

			ASSERT(!this->pCurrentGame->IsDemoRecording());

			this->bMusicStyleFrozen = false;
			this->pCurrentGame->RestartRoomFromLastCheckpoint(CueEvents);
			if (this->bPlayTesting)
				this->wUndoToTurn = this->pCurrentGame->wTurnNo ? this->pCurrentGame->wTurnNo - 1 : 0;
			else
				this->wForbidUndoBeforeTurn = this->wUndoToTurn = this->pCurrentGame->wTurnNo; //can't undo past restart point
			this->bRoomClearedOnce = this->pCurrentGame->IsCurrentRoomPendingExit();
			UpdateSound();
			ClearSpeech();
			AmbientSoundSetup();
			this->pRoomWidget->ClearEffects();
			this->pRoomWidget->RenderRoomLighting();
			this->pRoomWidget->ResetForPaint();
		}
		SynchScroll();
		PaintScroll();
		this->pMapWidget->DrawMapSurfaceFromRoom(this->pCurrentGame->pRoom);
		this->pMapWidget->RequestPaint();
		return eNextScreen;
	}

	if (ProcessExitLevelEvents(CueEvents, eNextScreen))
		return eNextScreen;

	//
	//End section where room load can occur.
	//

	if (bPlayerLeftRoom) //Went to a new room or this room was reloaded.
	{
		//Play music for style.
		this->bMusicStyleFrozen = IsMusicStyleFrozen(CueEvents);
		SetMusicStyle();

		//When changing rooms, save progress to disk.
		//Call this right before room transition to minimize apparent delay.
		if (!this->bPlayTesting && GetScreenType() == SCR_Game)
			g_pTheDB->Commit();

		//Determine direction of exit (if any).
		//Note that the following only occur if no demo playback is in progress.
		UINT wExitOrientation = NO_ORIENTATION;
		const CAttachableWrapper<UINT> *pExitOrientation =
				DYN_CAST(const CAttachableWrapper<UINT>*, const CAttachableObject*,
				CueEvents.GetFirstPrivateData(CID_ExitRoom));
		//Show transition to new room.
		if (pExitOrientation)
		{
			//Remove ambient sounds.
			StopAmbientSounds();

			wExitOrientation = static_cast<UINT>(*pExitOrientation);

			this->pRoomWidget->ShowRoomTransition(wExitOrientation);
			ClearEventBuffer(); //don't buffer up commands while transitioning, but don't reset pressed keys/buttons

			//Update other UI elements that may have changed.
			this->pMapWidget->UpdateFromCurrentGame();
			this->pMapWidget->DrawMapSurfaceFromRoom(this->pCurrentGame->pRoom);
			this->pMapWidget->RequestPaint();
			SetSignTextToCurrentRoom();
			PaintSign();
			ClearSpeech();
			SetGameAmbience(true);
			UpdateSound();
			ProcessSpeechCues(CueEvents);	//call after ClearSpeech
			this->dwTimeInRoom = this->dwLastTime = 0;
		}
	}
	else //Still in the same room.
	{
		if (CueEvents.HasOccurred(CID_Plots) || bLightToggled)
		{
			//Do an update of tile image arrays.
			const CCoordSet *pSet = DYN_CAST(const CCoordSet*, const CAttachableObject*,
					CueEvents.GetFirstPrivateData(CID_Plots) );
			this->pRoomWidget->UpdateFromPlots(pSet, &this->pCurrentGame->pRoom->geometryChanges);
			this->pMapWidget->DrawMapSurfaceFromRoom(this->pCurrentGame->pRoom);
			this->pMapWidget->RequestPaint();
		}
		else if (CueEvents.HasOccurred(CID_NPCTypeChange))
		{
			//Just ensure minimap state is current.
			this->pMapWidget->DrawMapSurfaceFromRoom(this->pCurrentGame->pRoom);
			this->pMapWidget->RequestPaint();
		}

		if (CueEvents.HasOccurred(CID_Combo))
		{
			AddComboEffect(CueEvents);
		}
	}

	return eNextScreen;
}

//***************************************************************************************
SCREENTYPE CGameScreen::ProcessCueEventsAfterRoomDraw(
//Process cue events list to create effects that should occur after
//room is drawn.
//
//Params:
	CCueEvents &CueEvents) //(in)
{
	SCREENTYPE eNextScreen = SCR_Game;
	const CAttachableObject *pObj;

	//Allow handling either before or after room is drawn.
	ProcessMovieEvents(CueEvents);

	const bool bLevelComplete = CueEvents.HasOccurred(CID_CompleteLevel) &&
			this->pCurrentGame->pLevel->bIsRequired;
	const bool bSecretFound = CueEvents.HasOccurred(CID_SecretRoomFound);
	const bool bHoldMastered = CueEvents.HasOccurred(CID_HoldMastered);
	const UINT wSX = this->pCurrentGame->swordsman.wX;
	const UINT wSY = this->pCurrentGame->swordsman.wY;

	if (CueEvents.HasOccurred(CID_CompleteLevel))
	{
		this->pMapWidget->UpdateFromCurrentGame();
		this->pMapWidget->RequestPaint();
		if (bLevelComplete)
		{
			//ensure texts aren't on top of each other
			int yOffset = 0;
			if (bSecretFound)
				yOffset -= 50;
			if (bHoldMastered)
				yOffset -= 50;
			if (!bHoldMastered)
				PlaySoundEffect(SEID_LEVELCOMPLETE);
			this->pRoomWidget->AddLastLayerEffect(new CFlashMessageEffect(
					this->pRoomWidget, g_pTheDB->GetMessageText(MID_LevelComplete),
					yOffset));
		}
	}
	if (bSecretFound)
	{
		//ensure texts aren't on top of each other
		int yOffset = 0;
		if (bLevelComplete)
			yOffset += 50;
		if (bHoldMastered)
			yOffset -= 50;
		if (!bLevelComplete && !bHoldMastered)
			PlaySoundEffect(SEID_SECRET);
		this->pRoomWidget->AddLastLayerEffect(new CFlashMessageEffect(
				this->pRoomWidget, g_pTheDB->GetMessageText(MID_SecretRoomFound),
				yOffset));
	}
	if (bHoldMastered) {
		//ensure texts aren't on top of each other
		int yOffset = 0;
		if (bLevelComplete)
			yOffset += 50;
		if (bSecretFound)
			yOffset += 50;
		PlaySoundEffect(SEID_DOOROPEN);
		this->pRoomWidget->AddLastLayerEffect(new CFlashMessageEffect(
				this->pRoomWidget, g_pTheDB->GetMessageText(MID_HoldMastered),
				yOffset));
	}
	if (this->bIsScrollVisible &&
			this->pCurrentGame->pRoom->GetTSquare(wSX, wSY) != T_SCROLL)
	{
		this->pFaceWidget->SetReading(false);
		HideScroll();
	}

	//Priority of player moods
	ShowPlayerFace();
	if (CueEvents.HasOccurred(CID_SwordsmanAfraid))
		this->pFaceWidget->SetMood(Mood_Nervous);
	else if (CueEvents.HasOccurred(CID_SwordsmanAggressive))
		this->pFaceWidget->SetMood(Mood_Aggressive);
	else if (CueEvents.HasOccurred(CID_SwordsmanNormal))
		this->pFaceWidget->SetMood(Mood_Normal);

	if (CueEvents.HasOccurred(CID_RoomConquerPending))  //priority of temporary moods
	{
		UINT eClearID = GetPlayerClearSEID();
		if (eClearID == (UINT)SEID_NONE) eClearID = SEID_CLEAR;
		this->pFaceWidget->SetMoodToSoundEffect(Mood_Happy, SEID(eClearID));
	}
	else if (CueEvents.HasOccurred(CID_MonsterDiedFromStab))
	{
		bool bBeethroStabbed = false;
		const UINT wSSwordX = this->pCurrentGame->swordsman.wSwordX;
		const UINT wSSwordY = this->pCurrentGame->swordsman.wSwordY;
		pObj = CueEvents.GetFirstPrivateData(CID_MonsterDiedFromStab);
		while (pObj)
		{
			const CMonster *pMonster = DYN_CAST(const CMonster*, const CAttachableObject*, pObj);
			if (wSSwordX == pMonster->wX && wSSwordY == pMonster->wY)
			{
				bBeethroStabbed = true;
				break;
			}
			pObj = CueEvents.GetNextPrivateData();
		}
		if (bBeethroStabbed)
			this->pFaceWidget->SetMood(Mood_Strike,250);
	}
	else if (CueEvents.HasOccurred(CID_Scared))
		this->pFaceWidget->SetMood(Mood_Nervous,250);
	else if (CueEvents.HasOccurred(CID_HitObstacle))
		this->pFaceWidget->SetMood(Mood_Aggressive,250);

	//Music cues.
	pObj = CueEvents.GetFirstPrivateData(CID_SetMusic);
	if (pObj)
	{
		const CCoord *pMusicID = dynamic_cast<const CCoord*>(pObj);
		if (pMusicID)
		{
			if (pMusicID->wX == UINT(SONGID_DEFAULT)) {
				if (this->bMusicStyleFrozen) {
					this->bMusicStyleFrozen = false;
					SetMusicStyle();
				}
			} else {
				this->bMusicStyleFrozen = true;
				if (pMusicID->wX == UINT(SONGID_NONE))	//no music
					g_pTheSound->StopSong();
				else if (pMusicID->wX == UINT(SONGID_CUSTOM)) //custom music
					g_pTheSound->PlayData(pMusicID->wY);
				else if (pMusicID->wX < UINT(SONGID_COUNT))
					g_pTheSound->CrossFadeSong(pMusicID->wX);
			}
		}
		const CAttachableWrapper<WSTRING> *pMusicMood =
				dynamic_cast<const CAttachableWrapper<WSTRING>*>(pObj);	//song name key
		if (pMusicMood)
		{
			//Fade to next song in this mood's track list and update play order.
			ASSERT(!pMusicMood->data.empty());
			CFiles f;
			list<WSTRING> songlist;
			if (f.GetGameProfileString("Songs", pMusicMood->data.c_str(), songlist))
			{
				g_pTheSound->CrossFadeSong(&songlist);
				f.WriteGameProfileString("Songs", pMusicMood->data.c_str(), songlist);
				this->bMusicStyleFrozen = true;
			}
		}
	}

	//Ambient sounds.
	if (CueEvents.HasOccurred(CID_AmbientSound))
	{
		pObj = CueEvents.GetFirstPrivateData(CID_AmbientSound);
		while (pObj)
		{
			//wX/Y: sound location, wO: DataID (0 = stop all), wValue: loop if set
			const CMoveCoordEx *pCoord = DYN_CAST(const CMoveCoordEx*, const CAttachableObject*, pObj);
			PlayAmbientSound(pCoord->wO, pCoord->wValue != 0, pCoord->wX, pCoord->wY);
			pObj = CueEvents.GetNextPrivateData();
		}
	}

	//Scripted effects.
	for (pObj = CueEvents.GetFirstPrivateData(CID_VisualEffect);
			pObj != NULL; pObj = CueEvents.GetNextPrivateData())
	{
		const VisualEffectInfo *pEffect = DYN_CAST(const VisualEffectInfo*, const CAttachableObject*, pObj);
		AddVisualEffect(pEffect, this->pRoomWidget, this->pCurrentGame);
		AddSoundEffect(pEffect);
	}

	//Player doubles might get placed between an Aumtlich and its target.
	if (CueEvents.HasOccurred(CID_DoublePlaced))
	{
		pRoomWidget->RemoveTLayerEffectsOfType(ESPARK);
		this->bPersistentEventsDrawn = false; //Any fuse sparks got removed too
	}

	for (pObj = CueEvents.GetFirstPrivateData(CID_ZombieGaze);
			pObj != NULL; pObj = CueEvents.GetNextPrivateData())
	{
		const CMonster *pMonster = DYN_CAST(const CMonster*, const CAttachableObject*, pObj);
		this->pRoomWidget->AddZombieGazeEffect(pMonster);
	}
	if (CueEvents.HasOccurred(CID_Swordfight))
		PlaySoundEffect(SEID_SWORDS);

	//Process both before and after room is drawn.
	if (!this->bPersistentEventsDrawn)
		ProcessFuseBurningEvents(CueEvents);

	ProcessQuestionPrompts(CueEvents, eNextScreen);

	//Check for winning game.
	if (CueEvents.HasOccurred(CID_WinGame) && GetScreenType() != SCR_Demo)
	{
		LOGCONTEXT("CGameScreen::ProcessCueEventsAfterRoomDraw--Winning game.");
		if (!this->bPlayTesting && GetScreenType() == SCR_Game)
		{
			UploadExploredRooms(ST_EndHold);	//end hold saved game should exist by now
			g_pTheDB->Commit();
		}

		//Update map to latest state.
		this->pMapWidget->DrawMapSurfaceFromRoom(this->pCurrentGame->pRoom);
		this->pMapWidget->RequestPaint();

		//Level exit effect is different depending on method of exit.
		const bool bStairs = bIsStairs(this->pCurrentGame->pRoom->GetOSquare(
				this->pCurrentGame->swordsman.wX, this->pCurrentGame->swordsman.wY));
		SCREENTYPE eExitRetScreen = SCR_Game;
		if (bStairs)
		{
			this->pRoomWidget->AddLastLayerEffect(new CTextEffect(
					this->pRoomWidget, GetGameStats(true).c_str(), F_Stats, 1500));
			eExitRetScreen = HandleEventsForLevelExit(); //walk down stairs first
			this->pRoomWidget->ResetRoom();
		} else {
			//Fade out.
			if (!CueEvents.HasOccurred(CID_PlayVideo))
				Paint();
			FadeRoom(false, 750);

			//Show hold stats over a blank room widget.
			this->pRoomWidget->UnloadCurrentGame();
			this->pRoomWidget->AddLastLayerEffect(new CTextEffect(
					this->pRoomWidget, GetGameStats(true).c_str(), F_Stats, 10000));
			HandleEventsForHoldExit();
		}
		CueEvents.Clear();
		this->pRoomWidget->ClearEffects();
		if (this->bPlayTesting)
		{
			//Return to level editor.
			eNextScreen = SCR_Return;
		} else {
			//Go to end hold screen.
			DeleteCurrentGame(); //unload without saving to continue slot
			eNextScreen = SCR_WinStart;
		}
		UnloadGame();  //current game has ended

		if (eExitRetScreen == SCR_None)
			eNextScreen = SCR_None;
	}

	return eNextScreen;
}

//*****************************************************************************
UINT CGameScreen::GetPlayerClearSEID() const
{
	UINT eSoundID = SEID_NONE;
	switch (this->pCurrentGame->swordsman.wAppearance)
	{
		case M_CLONE: case M_DECOY: case M_MIMIC:
		case M_BEETHRO: case M_BEETHRO_IN_DISGUISE: eSoundID = SEID_CLEAR; break;
		case M_GUNTHRO: eSoundID = SEID_GB_CLEAR; break;
		case M_NEATHER: eSoundID = SEID_NLAUGHING; break;
		case M_GOBLIN:	case M_GOBLINKING: eSoundID = SEID_GOB_CLEAR; break;
		case M_TARBABY: case M_MUDBABY: case M_GELBABY:
		case M_TARMOTHER: case M_MUDMOTHER: case M_GELMOTHER:
			eSoundID = SEID_SPLAT; break;
		case M_ROCKGOLEM: case M_ROCKGIANT: eSoundID = SEID_ROCK_CLEAR; break;
		case M_CITIZEN1: case M_CITIZEN2: case M_GUARD:
		case M_MUDCOORDINATOR: case M_TARTECHNICIAN:
		case M_CITIZEN: eSoundID = SEID_CIT_CLEAR; break;
		case M_WUBBA: eSoundID = SEID_WUBBA; break;
		case M_NEGOTIATOR:
			eSoundID = SEID_NEGO_CLEAR; break;
		case M_INSTRUCTOR:
		case M_CITIZEN3: case M_CITIZEN4:
			eSoundID = SEID_WOM_CLEAR; break;
		case M_HALPH: break; //not supported
		case M_HALPH2: eSoundID = SEID_HALPH2_CLEAR; break;
		case M_SLAYER: break; //not supported
		case M_SLAYER2: eSoundID = SEID_SLAYER_CLEAR; break;
		case M_STALWART:
			eSoundID = SEID_STALWART_CLEAR; break;
		case M_STALWART2:
			eSoundID = SEID_SOLDIER_CLEAR; break;
		default: eSoundID = SEID_MON_CLEAR; break;
	}
	return eSoundID;
}

//*****************************************************************************
bool CGameScreen::ProcessExitLevelEvents(CCueEvents& CueEvents, SCREENTYPE& eNextScreen)
//Returns: whether the game screen should be exited
{
	//Check for level exiting.
	if (!CueEvents.HasOccurred(CID_ExitLevelPending))
		return false;

	const CCoord *pExitInfo =
			DYN_CAST(const CCoord*, const CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_ExitLevelPending));
	const UINT dwEntranceID = pExitInfo->wX;
	const bool bSkipEntranceScreen = pExitInfo->wY != 0;
	if (!dwEntranceID)
		return false; //not a valid event field value -- ignore it

	if (!this->bPlayTesting && GetScreenType() == SCR_Game)
		g_pTheDB->Commit();

	//Update room/map to latest state.
	const CCoordSet *pSet = DYN_CAST(const CCoordSet*, const CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_Plots) );
	this->pRoomWidget->UpdateFromPlots(pSet, &this->pCurrentGame->pRoom->geometryChanges);
	this->pMapWidget->DrawMapSurfaceFromRoom(this->pCurrentGame->pRoom);
	this->pMapWidget->RequestPaint();
	ShowLockIcon(false); //remove room lock display

	//Level exit effect is different depending on method of exit.
	const bool bStairs = bIsStairs(this->pCurrentGame->pRoom->GetOSquare(
			this->pCurrentGame->swordsman.wX, this->pCurrentGame->swordsman.wY));
	if (bStairs)
	{
		//Show swordsman walking down stairs, etc.
		this->pRoomWidget->AddLastLayerEffect(new CTextEffect(
			this->pRoomWidget, GetGameStats(false, true).c_str(), F_Stats, 1500));
		eNextScreen = HandleEventsForLevelExit();
		if (eNextScreen == SCR_None)
			return true; //quit
	} else {
		//Paint room in latest state.
		if (!CueEvents.HasOccurred(CID_PlayVideo))
			Paint();
	}

	//Fade out when level entrance screen is not shown.
	CEntranceData *pEntrance = this->pCurrentGame->pHold->GetEntrance(dwEntranceID);
	const bool bShowLevelEntranceDescription =
			pEntrance && pEntrance->bShowDescription && !bSkipEntranceScreen;
	if (bShowLevelEntranceDescription)
		eNextScreen = SCR_LevelStart;
	else
	{
		if (eNextScreen == SCR_LevelStart)
			eNextScreen = SCR_Game;
		FadeRoom(false, 750);
	}

	//Reset these things.
	ClearSpeech();
	this->pRoomWidget->ClearEffects();

	if (GetScreenType() != SCR_Demo)
	{
		CueEvents.Clear();

		//Handle uploads before loading new level data so room isn't
		//redrawn in incorrect style on error.
		WaitToUploadDemos();

		this->pCurrentGame->LoadFromLevelEntrance(this->pCurrentGame->pHold->dwHoldID,
				dwEntranceID, CueEvents);
		//stop drawing new room until after Level Start screen
		this->pRoomWidget->ResetRoom();
		this->bRoomClearedOnce = this->pCurrentGame->IsCurrentRoomPendingExit();

		if (!bShowLevelEntranceDescription)
		{
			//Handle things now that would normally be done on reactivating
			//this screen after showing the level entrance screen.
			this->pRoomWidget->UpdateFromCurrentGame();
			this->pMapWidget->LoadFromCurrentGame(this->pCurrentGame);
			this->pMapWidget->RequestPaint();
			SetSignTextToCurrentRoom();
			PaintSign();
			ProcessSpeechCues(CueEvents);
			PaintClock(true);
			UpdateSound();
			this->bRoomClearedOnce = false;
			this->dwTimeInRoom = this->dwLastTime = 0;

			//Fade in.
			FadeRoom(true, 500);

			//Throw away everything that happened during fade.
			ClearEvents();

			this->bMusicStyleFrozen = IsMusicStyleFrozen(CueEvents);
			SetMusicStyle();
		}
	}

	return true;
}

//*****************************************************************************
void CGameScreen::ProcessFuseBurningEvents(CCueEvents& CueEvents)
//Handle events for fuse burning.
//This must be called both before and after the room is drawn.
{
	if (!CueEvents.HasOccurred(CID_FuseBurning))
		return;

	ASSERT(this->pCurrentGame);
	const CMoveCoord *pCoord = DYN_CAST(const CMoveCoord*, const CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_FuseBurning));
	ASSERT(pCoord);
	static const UINT MAX_FUSE_SOUNDS = 1;
	UINT wCount = 0;
	while (pCoord)
	{
		const UINT wTSquare = this->pCurrentGame->pRoom->GetTSquare(pCoord->wX, pCoord->wY);
		if (wTSquare == T_FUSE || wTSquare == T_BOMB)	//needed to avoid effects
					//where fuses have already disappeared since the cue event fired
		{
			if (pCoord->wO == NO_ORIENTATION && wTSquare == T_FUSE)
			{
				if (wCount++ < MAX_FUSE_SOUNDS)
				{
					this->fPos[0] = static_cast<float>(pCoord->wX);
					this->fPos[1] = static_cast<float>(pCoord->wY);
					PlaySoundEffect(SEID_STARTFUSE, this->fPos);
				}
			}
			this->pRoomWidget->AddTLayerEffect(
					new CSparkEffect(this->pRoomWidget, *pCoord, 10));
		}
		pCoord = DYN_CAST(const CMoveCoord*, const CAttachableObject*,
				CueEvents.GetNextPrivateData());
	}

	this->bPersistentEventsDrawn = true;
}

//*****************************************************************************
void CGameScreen::ProcessMovieEvents(CCueEvents& CueEvents)
//Handle events for playing movies.
//This must be called both before and after the room is drawn to catch all cases.
{
	if (!CueEvents.HasOccurred(CID_PlayVideo))
		return;

	const CAttachableObject *pObj = CueEvents.GetFirstPrivateData(CID_PlayVideo);
	while (pObj)
	{
		const CMoveCoord *pDataVals = DYN_CAST(const CMoveCoord*, const CAttachableObject*, pObj);
		const UINT dwDataID = static_cast<UINT>(pDataVals->wO);
		if (dwDataID)
		{
			this->pRoomWidget->AllowSleep(false);
			PlayVideo(dwDataID, int(pDataVals->wX), int(pDataVals->wY));
		}

		//Don't reprocess these events if this method is called again.
		//This is done instead of calling ClearEvent so the occurred flag isn't reset.
		CueEvents.Remove(CID_PlayVideo, pObj);

		//The next item is now the first item.
		pObj = CueEvents.GetFirstPrivateData(CID_PlayVideo);
	}
}

//*****************************************************************************
void CGameScreen::ProcessQuestionPrompts(CCueEvents& CueEvents, SCREENTYPE& eNextScreen)
//Only send message prompts when not replaying a demo or transitioning to this screen.
{
	if (GetScreenType() == SCR_Demo)
		return;

	//If a game is being loaded to a turn where the player must answer questions,
	//delay bringing up the question dialog until after the screen transition
	//has completed.
	if (g_pTheSM->bTransitioning)
	{
		this->bNeedToProcessDelayedQuestions = CueEvents.HasOccurred(CID_MonsterSpoke);
		return;
	}

	this->bNeedToProcessDelayedQuestions = false; //must be reset before invoking game commands below

	while (const CMonsterMessage *pMsg = this->pCurrentGame->GetUnansweredQuestion())
	{
		switch (pMsg->eType)
		{
			case MMT_OK:
				if (ShowOkMessage(pMsg->eMessageID) == TAG_QUIT)
					eNextScreen = SCR_None;
			break;

			case MMT_YESNO:
			{
				UINT dwRet;
				if (pMsg->eMessageID)
					dwRet = ShowYesNoMessage(pMsg->eMessageID);
				else
					dwRet = ShowYesNoMessage(pMsg->message.c_str());
				switch (dwRet)
				{
					case TAG_QUIT: eNextScreen = SCR_None; break;
					case TAG_ESCAPE: eNextScreen = SCR_Return; break;
					case TAG_YES:
						//Recursive call.
						eNextScreen = ProcessCommand(CMD_YES);
					break;
					default:
						//Recursive call.
						eNextScreen = ProcessCommand(CMD_NO);
					break;
				}

				//Refresh room in case command changed its state.
				this->pRoomWidget->DirtyRoom();
			}
			break;

			case MMT_MENU:
			{
				const UINT dwAnswer = GetMessageAnswer(pMsg);
				switch (dwAnswer)
				{
					case TAG_QUIT: case TAG_ESCAPE:
						eNextScreen = SCR_Return;
					break;
					case TAG_UNDO_FROM_QUESTION:
						eNextScreen = ProcessCommand(CMD_UNDO);
					break;
					default:
						eNextScreen = ProcessCommand(CMD_ANSWER, dwAnswer/256, dwAnswer%256);
					break;
				}
			}
			break;
		}

		if (!this->pCurrentGame) //game might have been ended and unloaded by a command above
			break;
	}
}

//*****************************************************************************
void CGameScreen::AddComboEffect(CCueEvents& CueEvents)
{
	const CAttachableWrapper<UINT> *pComboNum =
			DYN_CAST(const CAttachableWrapper<UINT>*, const CAttachableObject*,
			CueEvents.GetFirstPrivateData(CID_Combo));
	const UINT combo_num = pComboNum ? static_cast<UINT>(*pComboNum) : 0;

	WCHAR wNum[12];
	_itoW(combo_num, wNum, 10);

	WSTRING wStr = wNum;
	wStr += wszSpace;
	wStr += g_pTheDB->GetMessageText(MID_Combo);
	wStr += wszExclamation;

	float size_multiplier;
	if (combo_num < 15)
		size_multiplier = 0.25f;
	else if (combo_num < 25)
		size_multiplier = 0.35f;
	else if (combo_num < 50)
		size_multiplier = 0.50f;
	else if (combo_num < 100)
		size_multiplier = 0.75f;
	else
		size_multiplier = 1.00f;

	static const int height_offset = 35 + int(25.0f * size_multiplier);
	const int xPos = this->pRoomWidget->GetW() / 2;
	int yPos = this->pRoomWidget->GetH() - height_offset;

	//Don't draw effect over player.
	UINT wSX, wSY;
	if (this->pCurrentGame->GetSwordsman(wSX, wSY)) {
		if (wSY >= this->pCurrentGame->pRoom->wRoomRows * 2/3) {
			yPos = height_offset;
		}
	}

	this->pRoomWidget->AddLastLayerEffect(new CExpandingTextEffect(
			this->pRoomWidget, wStr.c_str(),
			F_ExpandText, xPos, yPos, size_multiplier, 1300));
}

//*****************************************************************************
void CGameScreen::Paint(
//Paint the whole screen.
//
//Params:
	bool bUpdateRect)          //(in)   If true (default) and destination
										//    surface is the screen, the screen
										//    will be immediately updated in
										//    the widget's rect.
{
	PaintBackground();
	PaintScroll();
	PaintSign();
	if (bUpdateRect) //fixes strange room widget rendering bug on continue game
		this->pRoomWidget->ResetForPaint();
	this->pFaceWidget->ResetForPaint();
	PaintChildren();

	if (bUpdateRect)
		UpdateRect();
}

//*****************************************************************************
void CGameScreen::PaintClock(const bool bShowImmediately)	//[default=false]
//Draw clock for current turn, if needed.
{
	//Threat clock is only visible when no scroll is shown.
	if (!this->bIsScrollVisible && this->pCurrentGame)
		this->pClockWidget->ShowClock(this->pCurrentGame->wSpawnCycleCount,
				this->pCurrentGame->bHalfTurn,
				this->pCurrentGame->pRoom->IsTimerNeeded(), bShowImmediately);
	else
		this->pClockWidget->Hide();
}

//*****************************************************************************
void CGameScreen::PlayAmbientSound(
//Plays specified sound effect.
//
//Params:
	const UINT dwDataID, //sound dataID (0 = stop ambient sounds)
	const bool bLoop,     //whether to loop sound indefinitely
	const UINT wX, const UINT wY) //if valid room coord, indicates where sound is located
{
	const bool bPos = this->pCurrentGame->pRoom->IsValidColRow(wX, wY);
	if (!dwDataID)
	{
		//Request to stop ambient sounds.
		if (!bPos)
			StopAmbientSounds();
		else {
			//Stop ambient sounds playing at (x,y).
			vector<ChannelInfo> continuing;
			for (vector<ChannelInfo>::const_iterator ac = this->ambientChannels.begin();
					ac != this->ambientChannels.end(); ++ac)
			{
				if (ac->bUsingPos && wX == static_cast<UINT>(ac->pos[0]) &&
						wY == static_cast<UINT>(ac->pos[1]))
					g_pTheSound->StopSoundOnChannel(ac->wChannel); //stop this sound
				else
					continuing.push_back(*ac); //this sound is still playing
			}
			this->ambientChannels = continuing;
		}
	}

	CStretchyBuffer buffer;
	if (!CDbData::GetRawDataForID(dwDataID, buffer))
		return;

	if (!bPos)
	{
		const int nChannel = g_pTheSound->PlaySoundEffect(buffer, bLoop);
		if (nChannel >= 0)
		{
			//Keep track of which channels are playing ambient sound effects.
			this->ambientChannels.push_back(ChannelInfo(nChannel));
		}
	} else {
		this->fPos[0] = static_cast<float>(wX);
		this->fPos[1] = static_cast<float>(wY);
		const int nChannel = g_pTheSound->PlaySoundEffect(buffer, bLoop, this->fPos);
		if (nChannel >= 0)
		{
			this->ambientChannels.push_back(ChannelInfo(nChannel, true,
					this->fPos[0], this->fPos[1], this->fPos[2]));
		}
	}
}

//*****************************************************************************
void CGameScreen::PlaySoundEffect(
//Wrapper function to set the frequency multiplier for sound effects
//based on game context.
	const UINT eSEID, float* pos, float* vel, //default=[NULL,NULL]
	const bool bUseVoiceVolume,
	float frequencyMultiplier)
{
	//Special contexts where sounds are played back differently for effect.
	if (this->pCurrentGame)
	{
		ASSERT(this->pCurrentGame->pRoom);
		CSwordsman& player = this->pCurrentGame->swordsman;

		if (player.bIsHasted || player.bFrozen)
			frequencyMultiplier *= 0.8f;

		switch (eSEID)
		{
			case SEID_STABTAR:
				if (this->pCurrentGame->pRoom->bBetterVision)
					 frequencyMultiplier *= 1.25f;
			break;
			default: break;
		}
	}

	g_pTheSound->PlaySoundEffect(eSEID, pos, vel, bUseVoiceVolume, frequencyMultiplier);
}

//*****************************************************************************
void CGameScreen::PlaySpeakerSoundEffect(
//Plays sound effects uttered by a human speaker at volume set for speech.
//Only plays when no speech sound clips are playing to avoid the speaker
//uttering two voices in tandem.
//
//Params:
	const UINT eSEID,
	float* pos, float* vel) //[default=NULL]
const
{
	//If a speech sound clip is already playing, don't play sound effect over it.
	for (vector<ChannelInfo>::const_iterator channel=this->speechChannels.begin();
			channel!=this->speechChannels.end(); ++channel)
	{
		if (g_pTheSound->IsSoundPlayingOnChannel(channel->wChannel) &&
				g_pTheSound->GetSoundIDPlayingOnChannel(channel->wChannel) ==
						static_cast<UINT>(SOUNDLIB::SEID_NONE))
			return;
	}

	g_pTheSound->PlaySoundEffect(eSEID, pos, vel, true);
}

//*****************************************************************************
WSTRING CGameScreen::PrintRank(const int nRanking, const bool bTie)
//Returns: a natural language string displaying a rank/place score
{
	WSTRING wStr;

	WCHAR wRank[10];
	_itoW(nRanking, wRank, 10);

	//English format.
	switch (Language::GetLanguage())
	{
		default:
		case Language::English:
		case Language::Russian: //rules below aren't Russian grammar, but texts still work out correctly
			wStr += wRank;
			if (nRanking >= 11 && nRanking <= 13)
				wStr += g_pTheDB->GetMessageText(MID_th);
			else switch (nRanking % 10)
			{
				case 1: wStr += g_pTheDB->GetMessageText(MID_st); break;
				case 2: wStr += g_pTheDB->GetMessageText(MID_nd); break;
				case 3: wStr += g_pTheDB->GetMessageText(MID_rd); break;
				default: wStr += g_pTheDB->GetMessageText(MID_th); break;
			}
			wStr += wszSpace;
			wStr += g_pTheDB->GetMessageText(MID_Rank);
			if (bTie)
			{
				wStr += wszSpace;
				wStr += g_pTheDB->GetMessageText(MID_TieScore);
			}
		break;
	}
	return wStr;
}

//*****************************************************************************
void CGameScreen::PrepCustomSpeaker(CFiredCharacterCommand *pCmd)
//Call to prepare the speaking entity for custom speaker commands.
{
	const CCharacterCommand *pCommand = pCmd->pCommand;
	ASSERT(pCommand);
	const CDbSpeech *pSpeech = pCommand->pSpeech;
	ASSERT(pSpeech);
	if (pSpeech->wCharacter == Speaker_Custom)
	{
		//Check location for a monster/NPC to attach this speech to.
		ASSERT(this->pCurrentGame->pRoom->IsValidColRow(pCommand->x, pCommand->y));
		CMonster *pMonster = this->pCurrentGame->pRoom->GetMonsterAtSquare(
				pCommand->x, pCommand->y);
		if (pMonster)
		{
			//Attach speech to monster at this tile.
			pCmd->pSpeakingEntity = pMonster;
		} else {
			//No monster there -- create temporary pseudo-monster to attach to.
			CCharacter *pCharacter = DYN_CAST(CCharacter*, CMonster*, pCmd->pExecutingNPC);
			ASSERT(pCharacter);
			CCharacter *pSpeaker = new CCharacter();
			pSpeaker->wX = pCommand->x;
			pSpeaker->wY = pCommand->y;
			pSpeaker->wLogicalIdentity = pCharacter->wLogicalIdentity;
			pSpeaker->wIdentity = pCharacter->wIdentity;
			pCmd->pSpeakingEntity = pSpeaker;
			pCmd->bPseudoMonster = true;  //indicates to delete temp monster when speech effect is done
		}
	}
}

//*****************************************************************************
void CGameScreen::ProcessSpeech()
//Play any speech dialog queued to play.
{
	//Determine whether it's time to start playing next speech.
	const Uint32 dwNow = SDL_GetTicks();
	if (dwNow < this->dwNextSpeech)
		return;   //Wait some more.

	//Queue has been emptied and completed.  Return everything to normal.
	if (this->speech.empty() && this->dwNextSpeech)
	{
		//Can start a new speech command at any time.
		this->dwNextSpeech = 0;

		//Return to showing player again.
		ShowPlayerFace(true);
		return;
	}

	//Don't continue speech while action is frozen.
	if (this->pCurrentGame && this->pCurrentGame->swordsman.wPlacingDoubleType)
		return;

	//Start next queued speech.
	while (!this->speech.empty())
	{
		//Get entity speaking.  Set face portrait.
		CFiredCharacterCommand *pCommand = this->speech.front();
		ASSERT(pCommand->pCommand);
		if (!ProcessSpeechSpeaker(pCommand))
		{
			//This character couldn't speak for some reason.  Skip to next speech.
			delete pCommand;
			this->speech.pop_front();
			continue;
		}

		//Play sound clip if allowed.
		CDbSpeech *pSpeech = pCommand->pCommand->pSpeech;
		ASSERT(pSpeech);
		bool bPlayingSound = false;
		int nChannel = -1;
		Uint32 dwDelay = pSpeech->dwDelay;
		if (pCommand->bPlaySound)
		{
			const CDbDatum *pSound = pSpeech->GetSound();
			if (pSound)
			{
				nChannel = g_pTheSound->PlayVoice(pSound->data);
				if (nChannel >= 0)
				{
					//Keep track of which channels are playing speech sound clips.
					//Also track character's ID and turn speech was executed.
					this->speechChannels.push_back(ChannelInfo(nChannel, false, 0.0, 0.0, 0.0,
							pCommand->turnNo, pCommand->scriptID, pCommand->commandIndex));
					this->speechChannels.back().text = pCommand->text;
					bPlayingSound = true;
					//If delay is default, set it to length of the sound sample.
					if (!dwDelay)
						dwDelay = g_pTheSound->GetSoundLength(nChannel);
				}
				pSpeech->UnloadSound();
			}
		}

		//Determine reading/speaking time for speech text.
		static const Uint32 dwBaseDuration = 1000;  //ms
		if (!dwDelay && pSpeech->MessageText.GetSize())
		{
			//Determine a "smart" default duration for this (non-empty) speech.
			dwDelay = dwBaseDuration + pSpeech->MessageText.GetSize() * 50;
		}

		//Add subtitle effect for character giving speech.
		if (this->bShowingSubtitlesWithVoice ||  //always show text
				(nChannel < 0 && dwDelay != 1)) //else, when text is disabled, skip instant subtitles 
					//i.e. when the text for a sound bite is too long to show on one line,
					//and must be broken into multiple lines that are displayed together.
					//A delay of 1ms is used by convention to effect this.
		{
			static const Uint32 dwMinDisplayDuration = 2000;  //ms
			CSubtitleEffect *pEffect = this->pRoomWidget->AddSubtitle(
					pCommand, max(dwMinDisplayDuration, dwDelay));
			if (bPlayingSound)
			{
				ASSERT(!this->speechChannels.empty());
				this->speechChannels.back().pEffect = pEffect;
			}
		}

		//Mark time next speech will begin.
		this->dwNextSpeech = dwNow + dwDelay;

		//Beethro shouldn't be sleeping while dialogue occurs.
		this->pRoomWidget->AllowSleep(false);

		//Done with this speech command.
		delete pCommand;
		this->speech.pop_front();
		break;
	}
}

//*****************************************************************************
void CGameScreen::ProcessSpeechCues(CCueEvents& CueEvents)
//Process speech cue events.  This might queue speeches for processing.
{
	if (!CueEvents.HasOccurred(CID_Speech))
		return;

	const CAttachableObject *pObj = CueEvents.GetFirstPrivateData(CID_Speech);
	while (pObj)
	{
		const CFiredCharacterCommand *pCmd = DYN_CAST(const CFiredCharacterCommand*, const CAttachableObject*, pObj);
		if (!pCmd->bFlush)
		{
			//Add this speech command to queue.
			const CCharacterCommand *pCommand = pCmd->pCommand;
			ASSERT(pCommand);
			const CDbSpeech *pSpeech = pCommand->pSpeech;
			ASSERT(pSpeech);
			if (pSpeech) //robustness
			{
				PrepCustomSpeaker(const_cast<CFiredCharacterCommand*>(pCmd));
				this->speech.push_back(const_cast<CFiredCharacterCommand*>(pCmd));
			}
		} else {
			//Flush the speech queue.  Stop any currently-playing sounds.
			if (!pCmd->bPlaySound)
				ClearSpeech(true);
			else
			{
				//Minimize duration of any queued speeches so they get displayed now
				//and new ones can be executed immediately thereafter.
				for (deque<CFiredCharacterCommand*>::iterator iter=this->speech.begin();
						iter!=this->speech.end(); ++iter)
				{
					CFiredCharacterCommand *pCommand = *iter;
					pCommand->bPlaySound = false;
					ASSERT(pCommand->pCommand);
					CDbSpeech *pSpeech = pCommand->pCommand->pSpeech;
					ASSERT(pSpeech);
					pSpeech->dwDelay = 1;
				}
				CutSpeech();
			}
			delete pCmd;
		}
		pObj = CueEvents.GetNextPrivateData();
	}

	//Clear refs to data members now to avoid exceptions later.
	CueEvents.ClearEvent(CID_Speech, false);
}

//*****************************************************************************
bool CGameScreen::ProcessSpeechSpeaker(CFiredCharacterCommand *pCommand)
//Returns: true if valid speaker was found and face set.
//False is returned if the character firing the command is to speak, but is now dead.
{
	//If the scripted speaker has died, it can't perform any more speech commands.
	ASSERT(this->pCurrentGame);
	CEntity *pEntity = this->pCurrentGame->getSpeakingEntity(pCommand);
	ASSERT(pEntity);
	if (pEntity == pCommand->pExecutingNPC && !pCommand->pExecutingNPC->IsAlive())
		return false;

	//Set face giving speech.
	CDbSpeech *pSpeech = pCommand->pCommand->pSpeech;
	ASSERT(pSpeech);
	UINT speaker = pSpeech->wCharacter;

	//Show custom speaker, if set.
	HoldCharacter *pCustomChar = NULL;
	if (speaker >= CUSTOM_CHARACTER_FIRST && speaker != M_NONE)
		pCustomChar = this->pCurrentGame->pHold->GetCharacter(speaker);

	if (speaker >= Speaker_Count && !pCustomChar) //indicates a dangling reference
		pSpeech->wCharacter = Speaker_None;
	if (pSpeech->wCharacter == Speaker_None)
	{
		//Just show player if no speaker is being shown.
		ShowPlayerFace(true);
	} else if (speaker == Speaker_Player) {
		this->pFaceWidget->SetMood((MOOD)pSpeech->wMood, 0, true);
		ShowPlayerFace(false,true);
	} else {
		//Show who is speaking.
		this->pFaceWidget->SetMood((MOOD)pSpeech->wMood, 0, true);
		if (pSpeech->wCharacter != Speaker_Custom && pSpeech->wCharacter != Speaker_Self)
		{
			if (pCustomChar && pCustomChar->dwDataID_Avatar)
			{
				//Show custom character avatar.
				this->pFaceWidget->SetImage(pCustomChar->dwDataID_Avatar);
			} else {
				if (pCustomChar)
					speaker = getSpeakerType(MONSTERTYPE(pCustomChar->wType));
				this->pFaceWidget->SetCharacter((SPEAKER)speaker, true);
			}
		} else {
			//Determine who is speaking.  Show their face, if applicable.
			HoldCharacter *pRemoteCustomChar = NULL;
			if (pCommand->pSpeakingEntity->wType == M_CHARACTER) //another custom speaker?
			{
				CCharacter *pRemoteCharacter = DYN_CAST(CCharacter*, CMonster*,
						pCommand->pSpeakingEntity);
				pRemoteCustomChar = this->pCurrentGame->pHold->GetCharacter(
						pRemoteCharacter->wLogicalIdentity);
			}
			if (pRemoteCustomChar && pRemoteCustomChar->dwDataID_Avatar)
			{
				//Show custom character avatar.
				this->pFaceWidget->SetImage(pRemoteCustomChar->dwDataID_Avatar);
			} else {
				UINT wIdentity = pCommand->pSpeakingEntity->GetIdentity();
				if (wIdentity == M_EYE_ACTIVE)
					wIdentity = M_EYE;       //map to same type

				wIdentity = getSpeakerType((MONSTERTYPE)wIdentity);
				if (wIdentity != Speaker_None)
					this->pFaceWidget->SetCharacter((SPEAKER)wIdentity, true);
			}
		}
		if (this->pFaceWidget->ResolveFace() == FF_Default)
		{
			//Face doesn't exist; show player face in stead of default face
			ShowPlayerFace(true);
		}
	}
	return true;
}

//*****************************************************************************
void CGameScreen::ReattachRetainedSubtitles()
//Call this method after a move undo has cleared most room effects, but
//speech subtitles in progress have been retained.
//Since the room data have been reinstantiated, these dangling effects need
//to be hooked back in to the entities they are tracking.
{
	ASSERT(this->pCurrentGame);
	for (vector<ChannelInfo>::const_iterator channel=this->speechChannels.begin();
			channel!=this->speechChannels.end(); ++channel)
	{
		CSubtitleEffect *pEffect = channel->pEffect;
		if (pEffect)
		{
			//Find current instance of the entity this subtitle effect is following.
			CCharacter *pCharacter = this->pCurrentGame->pRoom->
						GetCharacterWithScriptID(channel->scriptID);
			ASSERT(pCharacter);
			if (pCharacter) //robustness
			{
				CFiredCharacterCommand *pSpeechCommand = new CFiredCharacterCommand(pCharacter,
						&(pCharacter->commands[channel->commandIndex]), channel->turnNo,
						channel->scriptID, channel->commandIndex);
				pSpeechCommand->text = channel->text;
				PrepCustomSpeaker(pSpeechCommand);
				if (ProcessSpeechSpeaker(pSpeechCommand))
				{
					CEntity *pEntity = this->pCurrentGame->getSpeakingEntity(pSpeechCommand);
					pEffect->FollowCoord(pEntity);
					this->pRoomWidget->AddToSubtitles(pEffect);
				} else {
					delete pEffect;
				}
				delete pSpeechCommand;
			} else {
				delete pEffect;
			}
		}
	}
}

//*****************************************************************************
void CGameScreen::RetainSubtitleCleanup(const bool bVal) //[default=false]
//After a move has been undone and currently playing speech and subtitles
//have been successfully retained, unmark subtitle effects from being
//retained on effect clears any longer.
{
	for (vector<ChannelInfo>::const_iterator channel=this->speechChannels.begin();
			channel!=this->speechChannels.end(); ++channel)
	{
		CSubtitleEffect *pEffect = channel->pEffect;
		if (pEffect)
			pEffect->RequestRetainOnClear(bVal);
	}
}

//*****************************************************************************
void CGameScreen::HideBigMap()
//Hides the big level map being displayed, which returns input to normal play.
{
	ASSERT(this->bShowingBigMap);

	CScrollableWidget *pScrollingMap =
			DYN_CAST(CScrollableWidget*, CWidget*, GetWidget(TAG_BIGMAPCONTAINER));
	pScrollingMap->Hide();
	this->pBigMapWidget->Hide();

	this->bShowingBigMap = false;

	//Draw widgets.
	this->pRoomWidget->DirtyRoom();
	PaintChildren();
	UpdateRect();
}

//*****************************************************************************
void CGameScreen::ShowBigMap()
//Display scrollable minimap of the entire level over the room.
//While this map is shown, play commands are suspended.
//Clicking on visited rooms on this map allows the player to view them in their
//current state.  Clicking elsewhere or hitting a key returns to normal play.
{
	ASSERT(!this->bShowingBigMap);
	ASSERT(this->pCurrentGame);

	//Don't expand minimap whenever the current game move is still being resolved.
	if (this->pCurrentGame->IsCutScenePlaying())
		return; //in order to not desynchronize cut scene
	if (this->bNeedToProcessDelayedQuestions)
		return;

	if (!this->pBigMapWidget->LoadFromCurrentGame(this->pCurrentGame))
		return;
	//Show active room in its current state.
	this->pBigMapWidget->DrawMapSurfaceFromRoom(this->pCurrentGame->pRoom);

	this->bShowingBigMap = true;

	CScrollableWidget *pScrollingMap =
			DYN_CAST(CScrollableWidget*, CWidget*, GetWidget(TAG_BIGMAPCONTAINER));
	pScrollingMap->Show();
	this->pBigMapWidget->Show();
}

//*****************************************************************************
void CGameScreen::ShowDemosForRoom(const UINT roomID)
//Transition to the demos screen.  Show demos for specified room ID.
{
	CScreen *pScreen = g_pTheSM->GetScreen(SCR_Demos);
	if (!pScreen)
	{
		ShowOkMessage(MID_CouldNotLoadResources);

		//Redraw widgets.
		PaintChildren();
		UpdateRect();
		return;
	}
	CDemosScreen *pDemosScreen = DYN_CAST(CDemosScreen*, CScreen*, pScreen);
	ASSERT(pDemosScreen);

	pDemosScreen->ShowRoom(roomID);
	GoToScreen(SCR_Demos);
}

//*****************************************************************************
void CGameScreen::ShowRoom(CDbRoom *pRoom)
//Temporarily display another room in place of the current room.
{
	ASSERT(pRoom);

	//Show room's coords in level.
	WSTRING wstr;
	pRoom->GetLevelPositionDescription(wstr, true);
	if (!wstr.empty())
	{
		wstr += wszColon;
		wstr += wszSpace;
	}
	wstr += g_pTheDB->GetMessageText(MID_DisplayingOtherRoom);
	SetSignText(wstr.c_str());
	this->signColor = Blue;
	PaintSign();

	CRoomWidget *pTempRoomWidget = new CRoomWidget(0,
			this->pRoomWidget->GetX(), this->pRoomWidget->GetY(),
			this->pRoomWidget->GetW(),this->pRoomWidget->GetH());
	AddWidget(pTempRoomWidget, true); //load

	if (this->bShowingBigMap)
		this->pBigMapWidget->Hide(); //hide big map while displaying another room

	this->pRoomWidget->Hide();

	pTempRoomWidget->HidePlayer();
	VERIFY(pTempRoomWidget->LoadFromRoom(pRoom));
	pTempRoomWidget->Paint();
	UpdateRect();

	//Display until a key/button is pressed.
	bool bShow = true, bShowDemosForRoom = false;
	SDL_Event event;
	while (bShow)
	{
		//Get any events waiting in the queue.
		while (SDL_PollEvent(&event))
		{
			if (IsDeactivating()) //skip events
				continue;

			switch (event.type)
			{
				case SDL_ACTIVEEVENT:
					OnActiveEvent(event.active);
				break;

				case SDL_KEYDOWN:
					bShow = false;

					//Process other commands in the context of the displayed room.
					switch (event.key.keysym.sym)
					{
						case SDLK_F6:
							bShowDemosForRoom = true;
						break;

						default: break;
					}
				break;

				case SDL_MOUSEBUTTONDOWN:
					bShow = false;
				break;

				case SDL_QUIT:
					this->bQuitPrompt = true;
					if (ShowYesNoMessage(MID_ReallyQuit) != TAG_NO)
					{
						bShow = false;
						GoToScreen(SCR_None);
					}
					this->bQuitPrompt = false;
				break;
			}
		}

		//Update music (switch song or continue music fade if one is in progress).
		g_pTheSound->UpdateMusic();
		SDL_Delay(1); //be nice to the CPU
	}

	RemoveWidget(pTempRoomWidget); //deletes room object

	if (this->bShowingBigMap)
		this->pBigMapWidget->Show();

	//Reload graphics for current room.
	this->pRoomWidget->Show();
	this->pRoomWidget->LoadRoomImages();
	this->pRoomWidget->UpdateFromCurrentGame();
	SetSignTextToCurrentRoom();
	Paint();

	//Handle any special events triggered during room display.
	if (bShowDemosForRoom)
		ShowDemosForRoom(pRoom->dwRoomID);
}

//*****************************************************************************
void CGameScreen::ShowRoomTemporarily(const UINT roomID)
//Temporarily show the indicated room instead of the current room.
{
	CDbRoom *pRoom = g_pTheDB->Rooms.GetByID(roomID);
	if (!pRoom) //be robust to the possibility
		return;

	g_pTheSound->PlaySoundEffect(SEID_CHECKPOINT);

	//Set current room state.
	CCueEvents Ignored;
	CCurrentGame *pTempGame = g_pTheDB->GetImportCurrentGame();
	pTempGame->SetAutoSaveOptions(ASO_NONE);
	pTempGame->pRoom = pRoom;
	pTempGame->swordsman.wAppearance = M_NONE; //not in room
	pRoom->SetCurrentGame(pTempGame);
	const bool bLevelComplete = this->pCurrentGame->IsCurrentLevelComplete();
	pRoom->bGreenDoorsOpened = false;
	pRoom->SetRoomEntryState(Ignored, bLevelComplete, bLevelComplete,
			this->pCurrentGame->ConqueredRooms.has(roomID), pRoom->wMonsterCount);

	ShowRoom(pRoom);

	delete pTempGame; //deletes pRoom
}

//*****************************************************************************
void CGameScreen::ShowLockIcon(const bool bShow)
//Show hide persistent lock icon.
{
	if (bShow)
	{
		CSubtitleEffect *pEffect = new CSubtitleEffect(this, NULL,
				g_pTheDB->GetMessageText(MID_RoomLockIcon), Black, lockColor, 0);
		pEffect->SetOffset(161, 7);
		pEffect->SetAlpha(128);
		this->pEffects->AddEffect(pEffect);
	} else {
		this->pEffects->RemoveEffectsOfType(EFFECTLIB::ESUBTITLE);
	}
}

//*****************************************************************************
void CGameScreen::ShowPlayerFace(
//Show the face of the role the player is in.
//
//Params:
	const bool bOverrideLock, //[default=false]
	const bool bLockMood) //[default=false]
{
	if (!this->pCurrentGame)
		return;

	//Handle custom character images specially.
	UINT dwCharID = this->pCurrentGame->swordsman.wIdentity;
	if (dwCharID >= CUSTOM_CHARACTER_FIRST && dwCharID != M_NONE)
	{
		HoldCharacter *pChar = this->pCurrentGame->pHold->GetCharacter(dwCharID);
		if (pChar && pChar->dwDataID_Avatar)
		{
			//Show custom character image.
			if (bLockMood || bOverrideLock || (this->pFaceWidget->GetImageID() != pChar->dwDataID_Avatar &&
					!this->pFaceWidget->IsMoodLocked()))
				this->pFaceWidget->SetImage(pChar->dwDataID_Avatar);
			return;
		}
		//otherwise show character's functional image below
		if (pChar)
			dwCharID = pChar->wType;
	}

	SPEAKER player = getSpeakerType(MONSTERTYPE(dwCharID));
	if (player == Speaker_None)
	{
		//If player is not in the room, show Beethro's face if NPC Beethro is in the room.
		CMonster *pNPCBeethro = this->pCurrentGame->pRoom->GetNPCBeethro();
		if (pNPCBeethro)
			switch(pNPCBeethro->GetIdentity())
			{
				case M_BEETHRO:
					player = Speaker_Beethro;
				case M_GUNTHRO:
					player = Speaker_Gunthro;
				default:
					player = Speaker_Beethro;
			}
	}

	//Show player face when forced.
	//Otherwise, only show it when the widget is not locked.
	if (bLockMood || bOverrideLock || (this->pFaceWidget->GetCharacter() != player &&
			!this->pFaceWidget->IsMoodLocked()))
	{
		this->pFaceWidget->SetCharacter(player, bLockMood);
		if (!bLockMood) this->pFaceWidget->SetMood(Mood_Normal);
	}
}

//*****************************************************************************
void CGameScreen::ShowSpeechLog()
//Pop up a dialog box that contains a log of the speech played in the current room.
{
	if (!this->pCurrentGame)
		return;
	if (this->pCurrentGame->roomSpeech.empty())
		return;
	if (this->bShowingBigMap)
		return;

	this->pSpeechBox->SetPrompt(MID_SpeechLogTitle);
	this->pSpeechBox->SetCurrentGame(this->pCurrentGame);
	this->pSpeechBox->PopulateList(CEntranceSelectDialogWidget::Speech);
	this->pSpeechBox->SelectItem(0);

	UINT dwItemID=0;
	int playingChannel=-1;
	do {
		const CEntranceSelectDialogWidget::BUTTONTYPE eButton =
				(CEntranceSelectDialogWidget::BUTTONTYPE)this->pSpeechBox->Display();
		dwItemID = eButton == CEntranceSelectDialogWidget::OK ? this->pSpeechBox->GetSelectedItem() : 0;
		if (dwItemID)
		{
			//Play any attached speech for this line.
			ASSERT(this->pCurrentGame->roomSpeech.size() >= dwItemID);
			CCharacterCommand *pCmd = this->pCurrentGame->roomSpeech[dwItemID - 1];
			ASSERT(pCmd);

			const CDbDatum *pSound = pCmd->pSpeech->GetSound();
			if (pSound)
			{
				//Stop any replaying sound.
				if (playingChannel >= 0)
					g_pTheSound->StopSoundOnChannel(playingChannel);

				playingChannel = g_pTheSound->PlayVoice(pSound->data);
				pCmd->pSpeech->UnloadSound();
			}
		}
	} while (dwItemID != 0);

	Paint();
}

//*****************************************************************************
void CGameScreen::StopAmbientSounds()
//Stops all playing ambient sound effects.
{
	for (vector<ChannelInfo>::const_iterator channel=this->ambientChannels.begin();
			channel!=this->ambientChannels.end(); ++channel)
	{
		//Stop this channel from playing if it's an ambient sound clip.
		if (g_pTheSound->IsSoundPlayingOnChannel(channel->wChannel) &&
				g_pTheSound->GetSoundIDPlayingOnChannel(channel->wChannel) ==
						static_cast<UINT>(SOUNDLIB::SEID_NONE))
			VERIFY(g_pTheSound->StopSoundOnChannel(channel->wChannel));
	}
	this->ambientChannels.clear();
}

//*****************************************************************************
void CGameScreen::SwirlEffect()
//Swirl effect to highlight player.
{
	this->pRoomWidget->RemoveMLayerEffectsOfType(ESWIRL);
	this->pRoomWidget->AddMLayerEffect(
			new CSwordsmanSwirlEffect(this->pRoomWidget, this->pCurrentGame));
}

//*****************************************************************************
void CGameScreen::SynchScroll()
//Sets scroll display for the current game state.
{
	const bool bOnScroll = this->pCurrentGame->pRoom->GetTSquare(
			this->pCurrentGame->swordsman.wX, this->pCurrentGame->swordsman.wY) == T_SCROLL;
	this->bIsScrollVisible = bOnScroll;
	this->pFaceWidget->SetReading(bOnScroll);
	if (bOnScroll)
	{
		this->pScrollLabel->SetText(this->pCurrentGame->GetScrollTextAt(
				this->pCurrentGame->swordsman.wX, this->pCurrentGame->swordsman.wY).c_str());
	}
}

//*****************************************************************************
void CGameScreen::ToggleBigMap()
//Toggle pop-up level map.
{
	if (this->bShowingBigMap)
		HideBigMap();
	else
		ShowBigMap();

	//Draw widgets
	PaintChildren();
	UpdateRect();
}

//*****************************************************************************
void CGameScreen::UndoMove()
//Undoes last game move made, if any.
{
	if (!this->pCurrentGame->wTurnNo)
		return; //nothing to undo

#if !defined ENABLE_CHEATS
	if (!this->bPlayTesting && //undo always allowed during playtesting
			this->pCurrentGame->wTurnNo <= this->wUndoToTurn) //undo allowed to this turn
		return;
#endif
	const bool bRecordingDemo = this->pCurrentGame->IsDemoRecording();

	//Undo until the specified turn is reached.
	while (this->pCurrentGame->wTurnNo > this->wUndoToTurn)
	{
		const UINT delta = this->pCurrentGame->wTurnNo - this->wUndoToTurn;
		ClearCueEvents();
		this->pCurrentGame->UndoCommands(delta, this->sCueEvents);
	}

	//While playtesting, undo may continue until first move.
	//Also, whenever moving back to the turn where a double is being placed,
	//that turn may be undo as well.
	if (
#if !defined ENABLE_CHEATS
			(this->bPlayTesting ||
			this->pCurrentGame->swordsman.wPlacingDoubleType) &&
#endif
			this->pCurrentGame->wTurnNo > this->wForbidUndoBeforeTurn) //don't allow undoing before a restore point
		this->wUndoToTurn = this->pCurrentGame->wTurnNo - 1;

	this->pRoomWidget->pRoom = this->pCurrentGame->pRoom; //synch
	ClearSpeech(false);  //retain speech that started before the previous turn

	//Refresh game screen display info.
	if (bRecordingDemo)
	{
		if (this->pCurrentGame->IsDemoRecording())
		{
			//Don't show a long delay between last move and next move in the recorded demo.
			this->pCurrentGame->Commands.ResetTimeOfLastAdd();
		} else {
			//Remove recording text.
			SetSignTextToCurrentRoom();
			PaintSign();
		}
	}
	this->pRoomWidget->ClearEffects();
	this->pRoomWidget->RenderRoomLighting();

	ReattachRetainedSubtitles(); //after clearing effects, rejoin subtitles to current room objects

	SetGameAmbience(true);
	AmbientSoundSetup();
	DrawCurrentTurn();

	RetainSubtitleCleanup();
}

//*****************************************************************************
void CGameScreen::UpdateSound()
//Update listener and sounds state.
{
	//Update player position and orientation in sound engine.
	//const UINT wO = this->pCurrentGame->swordsman.wO;
	float pos[3] = {static_cast<float>(this->pCurrentGame->swordsman.wX),
		static_cast<float>(this->pCurrentGame->swordsman.wY), 0.0f};
	//float dir[3] = {static_cast<float>(nGetOX(wO)), static_cast<float>(nGetOY(wO)), 0.0f};
	float dir[3] = {0.0f,-1.0f,0.0f};	//face "ears" in direction user looks at room
	static float vel[3] = {0.0f, 0.0f, 0.0f}; //no velocity
	g_pTheSound->Update(pos, dir, vel);

	//Update positioned speech and ambient sound effects.
	vector<ChannelInfo>::const_iterator channel;
	vector<ChannelInfo> continuing; //sounds that are still playing
	for (channel=this->speechChannels.begin();
			channel!=this->speechChannels.end(); ++channel)
	{
		if (g_pTheSound->IsSoundPlayingOnChannel(channel->wChannel) &&
				g_pTheSound->GetSoundIDPlayingOnChannel(channel->wChannel) ==
						static_cast<UINT>(SOUNDLIB::SEID_NONE))
		{
			if (channel->bUsingPos)
				g_pTheSound->Update(channel->wChannel, (float*)channel->pos);
			continuing.push_back(*channel);
		} 
	}
	this->speechChannels = continuing;

	continuing.clear();
	for (channel=this->ambientChannels.begin();
			channel!=this->ambientChannels.end(); ++channel)
	{
		if (g_pTheSound->IsSoundPlayingOnChannel(channel->wChannel) &&
				g_pTheSound->GetSoundIDPlayingOnChannel(channel->wChannel) ==
						static_cast<UINT>(SOUNDLIB::SEID_NONE))
		{
			if (channel->bUsingPos)
				g_pTheSound->Update(channel->wChannel, (float*)channel->pos);
			continuing.push_back(*channel);
		}
	}
	this->ambientChannels = continuing;
}

//*****************************************************************************
bool CGameScreen::UploadDemoPolling()
//As the current game queues victory demos for upload, process them here.
//As results are received, 
//
//Returns: false if demo queue is empty, else true
{
	if (!this->pCurrentGame) return false;	//where upload info is stored
	if (!g_pTheNet) return false;

	//Ensure last request was completed before another upload is initiated.
	if (this->wUploadingDemoHandle)
	{
		if (g_pTheNet->GetStatus(this->wUploadingDemoHandle) >= 0)
		{
			//Get ranking.
			CStretchyBuffer* pBuffer = g_pTheNet->GetResults(this->wUploadingDemoHandle);
			if (pBuffer)
			{
				if (pBuffer->Size())
				{
					//Get demo ranking.
					(*pBuffer) += (UINT)0;	//null terminate
					char *pwczRank = (char*)(BYTE*)*pBuffer;
					const int nRanking = atoi(pwczRank);
					if (nRanking && this->pRoomWidget)
					{
						//Demo ranked on hi-score list -- give feedback to user.
						const bool bTie = strstr(pwczRank, ".5") != NULL;
						WSTRING wStr = PrintRank(nRanking, bTie);
						wStr += wszExclamation;
						if (nRanking == 1 && !bTie)
							wStr += wszExclamation;
						this->pRoomWidget->AddLastLayerEffect(new CFlashMessageEffect(
								this->pRoomWidget, wStr.c_str(), -300));	//show at top of room
					}

					//Mark demo as confirmed uploaded to server.
					if (this->dwUploadingDemo)
					{
						//Remove demo from demo upload queue.
						DEMO_UPLOAD *pDemoInfo = CCurrentGame::demosForUpload.front();
						delete pDemoInfo;
						CCurrentGame::demosForUpload.pop();

						CDbDemo *pDemo = g_pTheDB->Demos.GetByID(this->dwUploadingDemo);
						if (pDemo)
						{
							pDemo->SetFlag(CDbDemo::TestedForUpload);
							pDemo->Update();
							delete pDemo;
						} //else the demo was deleted before the response was received
					}
				}
				delete pBuffer;
			}
			this->dwUploadingDemo = this->wUploadingDemoHandle = 0;
		}
	}

	//Upload next queued demo when ready.
	if (!this->wUploadingDemoHandle)
	{
		if (CCurrentGame::demosForUpload.empty())
			return false;	//nothing to upload
		DEMO_UPLOAD *pDemoInfo = CCurrentGame::demosForUpload.front();
		this->wUploadingDemoHandle = g_pTheNet->UploadDemo(pDemoInfo->buffer,
				pDemoInfo->wTurn, pDemoInfo->dwTimeElapsed);
		if (this->wUploadingDemoHandle)
			this->dwUploadingDemo = pDemoInfo->dwDemoID;
	}

	return !CCurrentGame::demosForUpload.empty();
}

//*****************************************************************************
void CGameScreen::UploadExploredRooms(const SAVETYPE eSaveType)	//[default=ST_Continue]
//Upload list of explored rooms to site.
{
	if (!g_pTheNet) return;
	if (GetScreenType() != SCR_Game) return;
	if (this->bPlayTesting) return;

	const UINT dwPlayerID = g_pTheDB->GetPlayerID();
	if (!dwPlayerID) return;

	//Skip for holds that aren't on CaravelNet.
	if (this->pCurrentGame)
		if (!g_pTheNet->IsLocalHold(this->pCurrentGame->pHold->dwHoldID))
			return;

	SetCursor(CUR_Internet);
	string text;
	CIDSet ids(dwPlayerID);
	if (CDbXML::ExportXML(V_Players, ids, text, eSaveType))
		g_pTheNet->UploadExploredRooms(text);
	SetCursor();
}

//*****************************************************************************
void CGameScreen::WaitToUploadDemos()
//Uploads demos to server in order received.
//If an upload times out, the user may opt to cancel the upload.
{
	static const Uint32 INTERNET_TIMEOUT = 10000; //10s
	Uint32 dwNow, dwStartUploading = SDL_GetTicks();
	bool bPolling = false;
	const UINT oldHandle = this->wUploadingDemoHandle;
	while (UploadDemoPolling())
	{
		bPolling = true;
		SetCursor(CUR_Internet);
		ShowStatusMessage(MID_UploadingRoomConquerDemos);
		SDL_Delay(20);
		if (oldHandle != this->wUploadingDemoHandle)
			dwStartUploading = SDL_GetTicks(); //next demo started uploading
		if ((dwNow = SDL_GetTicks()) > dwStartUploading + INTERNET_TIMEOUT)
		{
#ifdef DEBUG
			SetCursor();
			const UINT dwTag = ShowYesNoMessage(MID_InternetTimeoutPrompt);
			SetCursor(CUR_Internet);
			if (dwTag == TAG_YES)
				dwStartUploading = dwNow;	//continue trying to upload
			else
#endif
			{
				//Stop trying to upload for now.
				//Attemps to upload will resume when this screen is reentered.
				this->dwUploadingDemo = this->wUploadingDemoHandle = 0;
				break;
			}
		}
	}
	if (bPolling)
	{
		SetCursor();
		HideStatusMessage();
	}
}
