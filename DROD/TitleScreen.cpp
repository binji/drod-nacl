// $Id: TitleScreen.cpp 10007 2012-03-24 18:18:29Z mrimer $

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

#include "TitleScreen.h"
#include "BrowserScreen.h"
#include "DrodBitmapManager.h"
#include "DrodFontManager.h"
#include "DrodSound.h"
#include "DemoScreen.h"
#include "BloodEffect.h"

#include <FrontEndLib/ImageWidget.h>
#include <FrontEndLib/LabelWidget.h>
#include <FrontEndLib/HyperLinkWidget.h>
#include <FrontEndLib/MenuWidget.h>

#include "../Texts/MIDs.h"
#include "../DRODLib/Db.h"

#include <BackEndLib/Browser.h>
#include <BackEndLib/Files.h>
#include <BackEndLib/Internet.h>
#include <BackEndLib/MessageIDs.h>
#include <BackEndLib/Ports.h>

#include <math.h>

#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

const UINT TAG_MENU = 1000;
const UINT TAG_PLAYMENU = 1001;
const UINT TAG_INTERNET_ICON = 1010;
const UINT TAG_CARAVEL_LOGO = 1011;
const UINT TAG_HYPERLINK_START = 10000;

const UINT dwDisplayDuration = 60000;  //ms

//Internet connection state icons.
const WCHAR wszSignalNo[] = {We('S'),We('i'),We('g'),We('n'),We('a'),We('l'),We('N'),We('o'),We(0)};
const WCHAR wszSignalYes[] = {We('S'),We('i'),We('g'),We('n'),We('a'),We('l'),We('Y'),We('e'),We('s'),We(0)};
const WCHAR wszSignalBad[] = {We('S'),We('i'),We('g'),We('n'),We('a'),We('l'),We('B'),We('a'),We('d'),We(0)};
const WCHAR wszSignalGood[] = {We('S'),We('i'),We('g'),We('n'),We('a'),We('l'),We('G'),We('o'),We('o'),We('d'),We(0)};

#define TITLE_BACKGROUND (0)
#define LIGHT_MASK       (1)

const int CX_CARAVEL_LOGO = 132;
const int CY_CARAVEL_LOGO = 132;

const WCHAR wszCaravelLogo[] = {
	We('C'),We('a'),We('r'),We('a'),We('v'),We('e'),We('l'),We('L'),We('o'),We('g'),We('o'),We('S'),We('E'),We(0)
};
const WCHAR wszTitleLogo[] = {
	We('g'),We('e'),We('b'),We('_'),We('l'),We('o'),We('g'),We('o'),We(0)
};

#define FOREGROUND_IMAGE   (2)
#define WINDOW_LIGHTS_MASK (3)

const int X_TITLE = 50;
const int Y_TITLE = 50;

float getDarkFactorByTimeOfDay()
{
	time_t t = time(NULL);
	tm* pLocalTime = localtime(&t);
	const float time_of_day = pLocalTime->tm_hour + pLocalTime->tm_min/60.f;
	static const float MIN_DARK_FACTOR = 0.1f;
	static const float DAWN = 6.f, MORNING = 8.f, EVENING = 19.f, NIGHT = 21.f;
	if (time_of_day <= DAWN || time_of_day >= NIGHT)
		return MIN_DARK_FACTOR; //full night
	else if (time_of_day >= MORNING && time_of_day <= EVENING)
		return 1.0f; //full day
	else if (time_of_day < MORNING) //sunrise
		return MIN_DARK_FACTOR + (((time_of_day - DAWN) / (MORNING - DAWN)) * (1.f - MIN_DARK_FACTOR));
	else //sunset
		return MIN_DARK_FACTOR + (((NIGHT - time_of_day) / (NIGHT - EVENING)) * (1.f - MIN_DARK_FACTOR));
}
bool areLightsOn(float darkFactor)
{
	return darkFactor < 0.5f;
}

#define PI (3.1415926535f)
#define TWOPI (2.0f * PI)

//
//Protected methods.
//

//******************************************************************************
CTitleScreen::CTitleScreen() : CDrodScreen(SCR_Title)
	, bReloadDemos(true)
	, dwNonTutorialHoldID(0)
	, pMenu(NULL), pPlayMenu(NULL)
	, dwFirstPaint(0)
	, bSavedGameExists(false)
	, wNewsHandle(0)
	, pMarqueeWidget(NULL)
	, bWaitingForHoldlist(false)
	, bPredarken(true), bReloadGraphics(false)
	, fDarkFactor(0.20f)
//Constructor.
{
	//Load image assets.
	this->imageFilenames.push_back(string("gunthro_title_back"));
	this->imageFilenames.push_back(string("TitleLightMask"));
	this->imageFilenames.push_back(string("gunthro_title_front"));
	this->imageFilenames.push_back(string("gunthro_title_windows"));

	g_pTheDBM->LoadGeneralTileImages();

	this->currentDemo = this->ShowSequenceDemoIDs.end();

	const bool bDemo = !IsGameFullVersion();

	AddWidget(new CImageWidget(0, X_TITLE, Y_TITLE, wszTitleLogo));
	AddWidget(new CImageWidget(TAG_CARAVEL_LOGO, CScreen::CX_SCREEN - CX_CARAVEL_LOGO, CScreen::CY_SCREEN - CY_CARAVEL_LOGO, wszCaravelLogo));

	//Option menu in lower center.
#ifdef RUSSIAN_BUILD
	const int CX_MENU = 441;
	const int CX_MENU2 = 375;
#else
	const int CX_MENU = 355;
	const int CX_MENU2 = 355;
#endif
	const int X_MENU = 80;
	const int X_MENU2 = 80;
	const int Y_MENU = 340;
	const int CY_MENU = 360;
	this->pMenu = new CMenuWidget(TAG_MENU, X_MENU, Y_MENU, CX_MENU, CY_MENU,
			F_TitleMenu, F_TitleMenuActive, F_TitleMenuSelected);
	this->pMenu->SetButtonSound(SEID_SWORDS);
	AddWidget(this->pMenu);
	this->pMenu->AddText(g_pTheDB->GetMessageText(MID_TitlePlayMenu), MNU_PLAYMENU);
	this->pMenu->AddText(g_pTheDB->GetMessageText(MID_TitlePlayer), MNU_WHO);
	this->pMenu->AddText(g_pTheDB->GetMessageText(MID_TitleSettings), MNU_SETTINGS);
	this->pMenu->AddText(g_pTheDB->GetMessageText(MID_TitleDemo), MNU_DEMO);
	this->pMenu->AddText(g_pTheDB->GetMessageText(MID_TitleBuild), MNU_BUILD);
	this->pMenu->AddText(g_pTheDB->GetMessageText(MID_TitleHelp), MNU_HELP);
	if (bDemo)
		this->pMenu->AddText(g_pTheDB->GetMessageText(MID_BuyNow), MNU_BUY);
	this->pMenu->AddText(g_pTheDB->GetMessageText(MID_TitleQuit), MNU_QUIT);

	//Play sub-menu.
	this->pPlayMenu = new CMenuWidget(TAG_PLAYMENU, X_MENU2, Y_MENU, CX_MENU2, CY_MENU,
			F_TitleMenu, F_TitleMenuActive, F_TitleMenuSelected);
	this->pPlayMenu->SetButtonSound(SEID_SWORDS);
	AddWidget(this->pPlayMenu);
	this->pPlayMenu->AddText(g_pTheDB->GetMessageText(MID_TitleTutorial), MNU_TUTORIAL);
	this->pPlayMenu->AddText(g_pTheDB->GetMessageText(MID_TitleNewGame), MNU_NEWGAME);
	this->pPlayMenu->AddText(g_pTheDB->GetMessageText(MID_TitleContinue), MNU_CONTINUE);
	this->pPlayMenu->AddText(g_pTheDB->GetMessageText(MID_TitleRestore), MNU_RESTORE);
	this->pPlayMenu->AddText(g_pTheDB->GetMessageText(MID_TitleWhere), MNU_WHERE);
	this->pPlayMenu->AddText(g_pTheDB->GetMessageText(MID_TitleChat), MNU_CHAT);
	this->pPlayMenu->AddText(g_pTheDB->GetMessageText(MID_TitleMainMenu), MNU_MAINMENU);
	this->pPlayMenu->Hide();

	//Draw version number in top right corner.
	UINT wVersionW, wVersionH;
	g_pTheFM->GetTextWidthHeight(F_TitleMarquee, wszVersionReleaseNumber, wVersionW, wVersionH);
	static const UINT CY_VERSION_LABEL = wVersionH;
	static const int X_VERSION_LABEL = 6;
#ifdef RUSSIAN_BUILD
	static const int Y_VERSION_LABEL = 0;
#else
	static const int Y_VERSION_LABEL = -4;
#endif
	AddWidget(new CLabelWidget(0, X_VERSION_LABEL, Y_VERSION_LABEL,
         wVersionW, CY_VERSION_LABEL, F_TitleMarquee, wszVersionReleaseNumber));

	//Internet connection status icon.
	static const int X_INTERNET_ICON = X_VERSION_LABEL;
	static const int Y_INTERNET_ICON = Y_VERSION_LABEL + CY_VERSION_LABEL + 9;
	CImageWidget *pInternetIcon = new CImageWidget(TAG_INTERNET_ICON,
			X_INTERNET_ICON, Y_INTERNET_ICON, wszSignalNo);
	pInternetIcon->SetAlpha(150);
	AddWidget(pInternetIcon);

	//Scroll text in marquee in bottom center.
	static const UINT MARQUEE_MARGIN = 0;
	static const UINT CX_MARQUEE = CX_SCREEN - 2*MARQUEE_MARGIN - (CX_CARAVEL_LOGO+1);
	static const int X_MARQUEE = MARQUEE_MARGIN;
	static const UINT CY_MARQUEE = 30;
	static const int Y_MARQUEE = CY_SCREEN - CY_MARQUEE - 10;
	this->pMarqueeWidget = new CMarqueeWidget(0, X_MARQUEE, Y_MARQUEE,
			CX_MARQUEE, CY_MARQUEE, 10);
	AddWidget(this->pMarqueeWidget);
}

//******************************************************************************
void CTitleScreen::LoadDemos()
//Load demo show sequence for currently selected hold.
{
	if (!this->bReloadDemos)
		this->bReloadDemos = true; //reload them next time by default, but not this time
	else
	{
		//Get all show demos in the active hold.
		this->ShowSequenceDemoIDs.clear();
		CDb db;
		db.Demos.FilterByShow();
		CIDSet demoIDs = db.Demos.GetIDs();
		const UINT activeHoldID = g_pTheDB->GetHoldID();
		for (CIDSet::const_iterator demo = demoIDs.begin(); demo != demoIDs.end(); ++demo)
		{
			const UINT dwDemoHoldID = CDb::getHoldOfDemo(*demo);
			if (dwDemoHoldID == activeHoldID)
				this->ShowSequenceDemoIDs += *demo;
		}
		this->currentDemo = this->ShowSequenceDemoIDs.end();
	}
}

//******************************************************************************
bool CTitleScreen::SetForActivate()
//Called before screen is activated and first paint.
//
//Returns:
//True if activation should continue, false if not.
{
	if (UnloadOnDeactivate())
		this->bPredarken = true; //need to redarken background if assets are being reloaded
	this->bReloadGraphics = false;

	//Background is lit according to time of day.
	this->fDarkFactor = getDarkFactorByTimeOfDay();

	SetCursor(CUR_Wait);

	//There ought to always be an active player at the title screen.
	const UINT dwPlayerID = g_pTheDB->GetPlayerID();
	ASSERT(dwPlayerID);

	//If the tutorial was just played, then select the previous non-tutorial hold.
	if (this->dwNonTutorialHoldID)
	{
		//If a tutorial game was being played, unload it.
		CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
				g_pTheSM->GetScreen(SCR_Game));
		ASSERT(pGameScreen);
		if (pGameScreen->IsGameLoaded())
			pGameScreen->UnloadGame();

		g_pTheDB->SetHoldID(this->dwNonTutorialHoldID);
		this->dwNonTutorialHoldID = 0;
	}

	LoadDemos();

	ASSERT(!this->bWaitingForHoldlist); //no previous transaction should be left uncompleted
	if (g_pTheNet->IsEnabled()) //if CaravelNet connection hasn't been disabled
	{
		CDbPlayer *pPlayer = g_pTheDB->GetCurrentPlayer();
		if (pPlayer)
		{
			//If net connect is set, get news text.
			if (pPlayer->Settings.GetVar(useInternetStr, false))
			{
				if (this->wstrNewsText.empty())	//query news text only once
					RequestNews();
			}

			delete pPlayer;
		}
	}
	SetMenuOptionStatus();

	g_pTheSound->PlaySong(SONGID_INTRO);

	SetCursor();

	this->dwFirstPaint = SDL_GetTicks();
	this->pMenu->ResetSelection();
	this->pPlayMenu->ResetSelection();

   return CDrodScreen::SetForActivate();
}

//
//Private methods.
//

//*****************************************************************************
void CTitleScreen::OnBetweenEvents()
//Handle events
{
	Animate();

	PollForNews();

	PollForHoldList();

	ImportQueuedFiles(); //import any files specified for import now

	CDrodScreen::OnBetweenEvents();

	//Show tool tip for highlighted menu option.
	if (this->pMenu->IsVisible())
		switch (this->pMenu->GetOnOption())
		{
			case MNU_SETTINGS: RequestToolTip(MID_SettingsTip); break;
			case MNU_HELP: RequestToolTip(MID_HelpTip); break;
			case MNU_DEMO: RequestToolTip(MID_DemoTip); break;
			case MNU_QUIT: RequestToolTip(MID_QuitTip); break;
			case MNU_BUILD: RequestToolTip(MID_BuildTip); break;
			case MNU_WHO: RequestToolTip(MID_WhoTip); break;
			case MNU_PLAYMENU: RequestToolTip(MID_PlayMenuTip); break;
			case MNU_BUY: RequestToolTip(MID_BuyNowTip); break;
			default: break;
		}
	else
		switch (this->pPlayMenu->GetOnOption())
		{
			case MNU_TUTORIAL: RequestToolTip(MID_TutorialTip); break;
			case MNU_NEWGAME: RequestToolTip(MID_NewGameTip); break;
			case MNU_CONTINUE: RequestToolTip(MID_ContinueTip); break;
			case MNU_RESTORE: RequestToolTip(MID_RestoreTip); break;
			case MNU_WHERE: RequestToolTip(MID_WhereTip); break;
			case MNU_CHAT: RequestToolTip(MID_ChatTip); break;
			case MNU_MAINMENU: RequestToolTip(MID_MainMenuTip); break;
			default: break;
		}

	//Go to demo screen after a map BG cycle.
	if (SDL_GetTicks() - this->dwFirstPaint > dwDisplayDuration)
	{
		const UINT dwDemoID = GetNextDemoID();
		if (dwDemoID)
		{
			CDemoScreen *pDemoScreen = DYN_CAST(CDemoScreen*, CScreen*,
					g_pTheSM->GetScreen(SCR_Demo));
			if (!pDemoScreen || !pDemoScreen->LoadDemoGame(dwDemoID))
			{
				ShowOkMessage(MID_LoadGameFailed);
				this->dwFirstPaint = SDL_GetTicks();
			} else {
				this->bReloadDemos = false;
				pDemoScreen->SetReplayOptions(false);
				GoToScreen(SCR_Demo);
			}
		}
	}
}

//*****************************************************************************
void CTitleScreen::OnClick(
//Called when widget receives a click event.
//
//Params:
	const UINT dwTagNo) //(in)   Widget that event applied to.
{
	if (dwTagNo >= TAG_HYPERLINK_START)
	{
		CHyperLinkWidget *pHyperLink = DYN_CAST(CHyperLinkWidget*, CWidget*,
				this->pMarqueeWidget->GetWidgetPart(dwTagNo));
		ASSERTP(pHyperLink != NULL, "Missing hyperlink: GetWidget returned NULL");
		ASSERT(pHyperLink->IsExternal());

		string strLink;
		UnicodeToAscii(pHyperLink->GetLink(), strLink);
		SetFullScreen(false);
		OpenExtBrowser(strLink.c_str());
	}
}

//*****************************************************************************
void CTitleScreen::OnDeactivate()
{
	//Must wait for pending transactions to complete before exiting.
	WCHAR temp[16];
	UINT dwStart = SDL_GetTicks();
	while (!PollForNews())
	{
		//News has not come in yet.  Wait until it does.
		SetCursor(CUR_Internet);

		//Show seconds that have passed.
		const UINT wSeconds = (SDL_GetTicks() - dwStart) / 1000;
		if (wSeconds) //don't show for first second to minimize distraction
		{
			WSTRING wstr = _itoW(wSeconds, temp, 10);
			wstr += wszSpace;
			wstr += g_pTheDB->GetMessageText(MID_TryingToConnectWithDisconnectPrompt);
			CScreen::ShowStatusMessage(wstr.c_str());
		}

		SDL_Delay(20);
		if (PollForCNetInterrupt())
		{
			//User has opted to disconnect from CaravelNet.
			g_pTheNet->Disable();
			this->wNewsHandle = 0; //ignore response
			break;
		}
	}
	dwStart = SDL_GetTicks();
	while (!PollForHoldList())
	{
		SetCursor(CUR_Internet);

		//Show seconds that have passed.
		const UINT wSeconds = (SDL_GetTicks() - dwStart) / 1000;
		if (wSeconds) //don't show for first second to minimize distraction
		{
			WSTRING wstr = _itoW(wSeconds, temp, 10);
			wstr += wszSpace;
			wstr += g_pTheDB->GetMessageText(MID_TryingToConnectWithDisconnectPrompt);
			CScreen::ShowStatusMessage(wstr.c_str());
		}

		SDL_Delay(20);
		if (PollForCNetInterrupt())
		{
			g_pTheNet->Disable();
			this->bWaitingForHoldlist = false; //ignore response
			break;
		}
	}

	HideStatusMessage();
	SetCursor();
}

//*****************************************************************************
void CTitleScreen::OnKeyDown(
//Called when widget receives SDL_KEYDOWN event.
//
//Params:
	const UINT /*dwTagNo*/,         //(in)   Widget that event applied to.
	const SDL_KeyboardEvent &Key) //(in)   Key event.
{
	TitleSelection wSetPos;
	switch (Key.keysym.sym)
	{
		case SDLK_RETURN:
			if (!(Key.keysym.mod & KMOD_ALT))
			{
				wSetPos = MNU_CONTINUE; break;
			}
			//else going to next case
		case SDLK_F10:
			ToggleScreenSize();
		return;
		case SDLK_F11:
			SaveSurface();
		return;

		case SDLK_F4:
#if defined(__linux__) || defined(__FreeBSD__)
		case SDLK_PAUSE:  case SDLK_BREAK:
#endif
			if (Key.keysym.mod & (KMOD_ALT | KMOD_CTRL))
				GoToScreen(SCR_None);   //boss key -- exit immediately
		return;

		case SDLK_ESCAPE:
			//Pressing ESC will quit without confirmation dialog from the main menu.
			if (this->pMenu->IsVisible())
				GoToScreen(SCR_None);
			else {
				//Just go back to the main menu.
				this->pMenu->Show();
				this->pPlayMenu->Hide();
				SelectFirstWidget(false);
				Paint();
			}
		return;

		//Menu selections.
		case SDLK_F1: wSetPos = MNU_HELP; break;

		default:
		return;
	}

	//One of the menu items was chosen.
	SCREENTYPE eNextScreen = ProcessMenuSelection(wSetPos);
	if (eNextScreen != SCR_Title)
		GoToScreen(eNextScreen);

	//Delay showing a demo when the user is doing things.
	this->dwFirstPaint = SDL_GetTicks();
}

//*****************************************************************************
void CTitleScreen::OnMouseDown(const UINT /*dwTagNo*/, const SDL_MouseButtonEvent &/*Button*/)
{
}

//*****************************************************************************
void CTitleScreen::OnMouseMotion(
//Handle unhighlighting menu options when mouse is not over menu widget.
//
//Params:
	const UINT dwTagNo,
	const SDL_MouseMotionEvent &MotionEvent)
{
	if (dwTagNo != TAG_MENU && dwTagNo != TAG_PLAYMENU)
	{
		if (this->pMenu->IsVisible())
			this->pMenu->ResetSelection();
		else
			this->pPlayMenu->ResetSelection();
	}
	CDrodScreen::OnMouseMotion(dwTagNo, MotionEvent);

	//Delay showing a demo when the user is doing things.
	if (MotionEvent.xrel || MotionEvent.yrel)
		this->dwFirstPaint = SDL_GetTicks();
}

//*****************************************************************************
void CTitleScreen::OnSelectChange(const UINT dwTagNo)
//Called when a menu selection has occurred.
{
	if (IsDeactivating()) return;

	TitleSelection wSetPos;
	switch (dwTagNo)
	{
		case TAG_MENU:
			wSetPos = (TitleSelection)this->pMenu->GetSelectedOption();
		break;
		case TAG_PLAYMENU:
			wSetPos = (TitleSelection)this->pPlayMenu->GetSelectedOption();
		break;
		default : return;
	}

	const SCREENTYPE eNextScreen = ProcessMenuSelection(wSetPos);
	if (eNextScreen != SCR_Title)
		GoToScreen(eNextScreen);
}

//*****************************************************************************
void CTitleScreen::Paint(
//Paint the whole screen.
//
//Params:
	bool bUpdateRect)          //(in)   If true (default) and destination
										//    surface is the screen, the screen
										//    will be immediately updated in
										//    the widget's rect.
{
	//Preprocess: darken border surface.
	if (this->bPredarken)
	{
		//Use darker screen if high quality graphics are set.
		const float fValue = IsShowingAlphaEffects() ? this->fDarkFactor : 0.75f;
		g_pTheBM->DarkenRect(0, 0, CScreen::CX_SCREEN, CScreen::CY_SCREEN,
				fValue, this->images[TITLE_BACKGROUND]);
		this->bPredarken = false;
	}

	//Draw background.
	g_pTheBM->BlitSurface(this->images[TITLE_BACKGROUND], NULL, GetDestSurface(), NULL);

	//Draw the screen.
	RedrawScreen(bUpdateRect);
}

//*****************************************************************************
bool CTitleScreen::PollForHoldList()
//If requested, waits for CaravelNet hold list to become available
//as an indication of the user being logged in.
//
//Returns: true if ready/done, false if still waiting
{
	if (!this->bWaitingForHoldlist)
		return true;

	//If we don't have a connection, do nothing here.
	if (g_pTheNet->Busy() || !g_pTheNet->IsEnabled())
		return false;

	//Ready to query hold list.
	this->bWaitingForHoldlist = false;

	//Now check for a downloaded hold list as an indication of being logged in.
	CImageWidget *pInternetIcon = DYN_CAST(CImageWidget*, CWidget*, GetWidget(TAG_INTERNET_ICON));
	CDbPlayer *pPlayer = g_pTheDB->GetCurrentPlayer();
	vector<CNetMedia*>& cNetMedia = g_pTheNet->GetCNetMedia();
	if (!cNetMedia.empty())
	{
		pInternetIcon->SetImage(wszSignalGood);
		this->pPlayMenu->Enable(MNU_CHAT, true);
		
		if (pPlayer)
		{
			//Prompt the user if the player has unsent scores for holds that might be on CaravelNet.
			const CIDSet& holdIds = g_pTheNet->GetLocalHolds();
			if (!holdIds.empty() && pPlayer->Settings.GetVar(playerScoresOld, (BYTE)0) != 0)
			{
				CLabelWidget *pText = new CLabelWidget(0, 0, 0, 0, 50, F_TitleMarquee, wszEmpty);
				pText->SetText(g_pTheDB->GetMessageText(MID_UnsentCaravelNetProgress),
						false, 0, true);
				this->pMarqueeWidget->AddPart(pText);

				pText = new CLabelWidget(0, 0, 0, 100, 50, F_TitleMarquee, wszEmpty);
				this->pMarqueeWidget->AddPart(pText);
				this->pMarqueeWidget->Reset();
			}
		}
	} else {
		//Determine whether player is a registered CaravelNet user.
		if (pPlayer)
		{
			//Since news feed was downloaded, this mean player has internet connectivity set.
			//So, if a username and key are set, that means they are invalid
			//since the CaravelNet hold list is empty.
			if (WCSlen((const WCHAR*)pPlayer->CNetNameText) &&
					WCSlen((const WCHAR*)pPlayer->CNetPasswordText))
			{
				pInternetIcon->SetImage(wszSignalBad);

				//Ambient text prompt to update password.
				CLabelWidget *pText = new CLabelWidget(0, 0, 0, 0, 50, F_TitleMarquee, wszEmpty);
				pText->SetText(g_pTheDB->GetMessageText(MID_InvalidCaravelNetKey),
						false, 0, true);
				this->pMarqueeWidget->AddPart(pText);

				pText = new CLabelWidget(0, 0, 0, 100, 50, F_TitleMarquee, wszEmpty);
				this->pMarqueeWidget->AddPart(pText);
				this->pMarqueeWidget->Reset();
			}
		}
	}
	delete pPlayer;

	return true;
}

//*****************************************************************************
bool CTitleScreen::PollForNews()
//Query for news feed from CaravelNet.
//
//Returns: true if ready/done, false if still waiting
{
	if (!this->wNewsHandle)
		return true;

	const int nStatus = CInternet::GetStatus(this->wNewsHandle);
	if (nStatus < 0)
		return false;

	CImageWidget *pInternetIcon = DYN_CAST(CImageWidget*, CWidget*, GetWidget(TAG_INTERNET_ICON));
	const UINT wHandle = this->wNewsHandle;
	this->wNewsHandle = 0;
	CStretchyBuffer *pBuffer = CInternet::GetResults(wHandle);

	if (!pBuffer) {
		//News request did not return successfully.
		pInternetIcon->SetImage(wszSignalNo);

		//Check for special problems.
#ifdef NO_CURL
		if (nStatus == 0)
#else
		if (nStatus == CURLE_OPERATION_TIMEDOUT)
#endif
		{
			//Not connecting.  Maybe no Internet connection or server is down?

			//Try to get news again.  Disable server requests until news is received.
			g_pTheNet->Disable();
			RequestNews();
			return false;  //still waiting for request
		}
	} else {
		//There is an Internet connection.
		pInternetIcon->SetImage(wszSignalYes);
		g_pTheNet->Enable();

		//Update news text.
		(*pBuffer) += (UINT)0;  //null terminate
		const char *pszFromWeb = (char*)(BYTE*)*pBuffer;
 		ASSERT(pszFromWeb);

		//Check whether news is new.
		string strFromWeb;
		if (!pszFromWeb)
			strFromWeb = "Couldn't get news";
		else
		{
			strFromWeb = pszFromWeb;
			string str;
			CFiles f;
			f.GetGameProfileString("Startup", "LastNews", str);
			if (strFromWeb.size() && str.compare(strFromWeb.c_str()) != 0)
			{
				//New news!
				f.WriteGameProfileString("Startup", "LastNews", strFromWeb.c_str());
				g_pTheSound->PlaySoundEffect(SEID_WISP);
			}
		}

		delete pBuffer;

		AsciiToUnicode(strFromWeb.c_str(), this->wstrNewsText);
		SetNewsText();

		//Wait to query hold list until no delay will be incurred.
		this->bWaitingForHoldlist = true;
	}
	return true;
}

//*****************************************************************************
SCREENTYPE CTitleScreen::ProcessMenuSelection(
//Processes menu selection.  No UI-related performed here.
//
//Params:
	TitleSelection wMenuPos)   //(in) One of the MNU_* constants.
//
//Returns:
//Screen to go to next or SCR_Title to remain at title screen.
{
	switch (wMenuPos)
	{
		case MNU_MAINMENU:
			this->pMenu->Show();
			this->pPlayMenu->Hide();
			SelectFirstWidget(false);
		break;

		case MNU_PLAYMENU:
			this->pMenu->Hide();
			this->pPlayMenu->Show();
			SelectFirstWidget(false);
		break;

		case MNU_TUTORIAL:
		{
			//Select tutorial hold here.
			this->dwNonTutorialHoldID = g_pTheDB->GetHoldID();

			//If a tutorial game was being played, unload it before changing hold ID.
			CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
					g_pTheSM->GetScreen(SCR_Game));
			ASSERT(pGameScreen);
			if (pGameScreen->IsGameLoaded())
				pGameScreen->UnloadGame();

			//Select tutorial based on keyboard configuration.
			UINT dwTutorialHoldID = g_pTheDB->Holds.GetHoldIDWithStatus(CDbHold::Tutorial);
			CFiles Files;
			string strKeyboard;
			if (Files.GetGameProfileString("Localization", "Keyboard", strKeyboard))
				if (atoi(strKeyboard.c_str()) == 1)
					++dwTutorialHoldID;	//ATTN: tutorial holds must be in consecutive order

			g_pTheDB->SetHoldID(dwTutorialHoldID);
		}
		//NO BREAK

		case MNU_NEWGAME:
		{
			const UINT dwCurrentHoldID = g_pTheDB->GetHoldID();
			if (!dwCurrentHoldID) return SCR_Title;
			CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
					g_pTheSM->GetScreen(SCR_Game));
			ASSERT(pGameScreen);
			if (pGameScreen->IsGameLoaded())
				pGameScreen->UnloadGame();
			if (!pGameScreen->LoadNewGame(dwCurrentHoldID))
			{
				ShowOkMessage(MID_LoadGameFailed);
				return SCR_Title;
			}
			ASSERT(pGameScreen->IsGameLoaded());

			this->bReloadDemos = false;
			if (pGameScreen->ShouldShowLevelStart())
				return SCR_LevelStart;

			return SCR_Game;
		}

		case MNU_CONTINUE:
		{
			const UINT dwCurrentHoldID = g_pTheDB->GetHoldID();
			if (!dwCurrentHoldID) return SCR_Title;
			CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
					g_pTheSM->GetScreen(SCR_Game));
			ASSERT(pGameScreen);
			if (!pGameScreen->IsGameLoaded())
			{
				if (!pGameScreen->LoadContinueGame())
				{
					//Probable continue slot is missing.
					if (!pGameScreen->LoadNewGame(dwCurrentHoldID))
					{
						ShowOkMessage(MID_LoadGameFailed);
						return SCR_Title;
					}
				}
			}
			ASSERT(pGameScreen->IsGameLoaded());

			//If game is continued via Enter from main menu, then go to play
			//submenu at the same time play begins so that the user returns
			//their when returning to this screen.
			this->pMenu->Hide();
			this->pPlayMenu->Show();
			SelectFirstWidget(false);

			this->bReloadDemos = false;
			if (pGameScreen->ShouldShowLevelStart())
				return SCR_LevelStart;

			return SCR_Game;
		}

		case MNU_HELP:
			CBrowserScreen::SetPageToLoad(NULL);
			this->bReloadDemos = false;
		return SCR_Browser;

		case MNU_SETTINGS:
			this->bReloadDemos = false;
			ResetCNetStatus();
		return SCR_Settings;

		case MNU_RESTORE:
			ASSERT(this->bSavedGameExists);
			this->bReloadDemos = false;
		return SCR_Restore;

		case MNU_DEMO:
		{
			const UINT dwDemoID = GetNextDemoID();
			if (dwDemoID)
			{
				CDemoScreen *pDemoScreen = DYN_CAST(CDemoScreen*, CScreen*,
						g_pTheSM->GetScreen(SCR_Demo));
				if (!pDemoScreen || !pDemoScreen->LoadDemoGame(dwDemoID))
					ShowOkMessage(MID_LoadGameFailed);
				else {
					this->bReloadDemos = false;
					pDemoScreen->SetReplayOptions(false);
					return SCR_Demo;
				}
			}
		}
		return SCR_Title;

		case MNU_BUILD:
		{
			if (!g_pTheDB->Holds.EditableHoldExists())
			{
				SetCursor();
#ifndef ENABLE_CHEATS
				if (ShowYesNoMessage(MID_CreateHoldPrompt) != TAG_YES)
					break;
#endif
			}

			//Editing room of current game could break it -- so unload it now.
			CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
					g_pTheSM->GetScreen(SCR_Game));
			ASSERT(pGameScreen);
			pGameScreen->UnloadGame();
		}
		return SCR_EditSelect;

		case MNU_QUIT:
		return SCR_Sell;

		case MNU_WHO:
		{
			//Changing player will make the current game invalid -- so unload it now.
			CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
					g_pTheSM->GetScreen(SCR_Game));
			ASSERT(pGameScreen);
			pGameScreen->UnloadGame();

			this->bReloadDemos = false;
			ResetCNetStatus();
		}
		return SCR_SelectPlayer;

		case MNU_WHERE:
		{
			//Deleting hold of current game would cause it to save a continue
			//slot for a room that no longer exists -- so unload the game now.
			CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
					g_pTheSM->GetScreen(SCR_Game));
			ASSERT(pGameScreen);
			pGameScreen->UnloadGame();
		}
		return SCR_HoldSelect;

		case MNU_BUY:
			GoToBuyNow();
		break;

		case MNU_CHAT:
		return SCR_Chat;

		default: break;
	}

	return SCR_Title;
}

//*****************************************************************************
void CTitleScreen::Animate()
//Animates the screen.
{
	//Redraw the screen.
	RedrawScreen();
}

//*****************************************************************************
UINT CTitleScreen::GetNextDemoID()
//Returns:
//DemoID of next demo in sequence to show or 0L if there are no demos to show.
{
	if (this->currentDemo == this->ShowSequenceDemoIDs.end())
	{
		//Start from the first demo.
		if (this->ShowSequenceDemoIDs.empty())
			return 0L; //No demos to show.
		this->currentDemo = this->ShowSequenceDemoIDs.begin();
	}

	const UINT dwFirstDemoID = *this->currentDemo;
	UINT dwRetDemoID;   //This demo ID is returned.

	//Advance to next demo ID (for play next time), looping back to start if needed.
	//NOTE: Only select demos from currently selected hold.
	const UINT dwCurrentHoldID = g_pTheDB->GetHoldID();
	UINT dwDemoHoldID;
	do {
		//Skip demos not belonging to the current hold.
		dwRetDemoID = *this->currentDemo;
		dwDemoHoldID = g_pTheDB->Demos.GetHoldIDofDemo(dwRetDemoID);

		//Get next demo marked for show (for next time).
		++this->currentDemo;
		if (this->currentDemo == this->ShowSequenceDemoIDs.end())
			this->currentDemo = this->ShowSequenceDemoIDs.begin();
	} while (dwDemoHoldID != dwCurrentHoldID &&
			*this->currentDemo != dwFirstDemoID);  //haven't wrapped around

	if (*this->currentDemo == dwFirstDemoID &&
			dwDemoHoldID != dwCurrentHoldID)
		dwRetDemoID = 0L; //No demos for this hold were found.

	return dwRetDemoID;
}

bool CTitleScreen::IsShowingAlphaEffects() const
{
	return g_pTheBM->bAlpha || g_pTheBM->eyeCandy;
}

//*****************************************************************************
void CTitleScreen::RedrawScreen(const bool bUpdate) //[default=true]
//Updates the title screen graphics.
{
	//Draw light mask if higher quality graphics are enabled.
	const bool bAlpha = IsShowingAlphaEffects();

	SDL_Surface *pDestSurface = GetDestSurface();

	int nMouseX, nMouseY;
	SDL_GetMouseState(&nMouseX, &nMouseY);

	g_pTheBM->BlitSurface(this->images[TITLE_BACKGROUND], NULL, pDestSurface, NULL);

	const bool bLightsOn = bAlpha && areLightsOn(this->fDarkFactor);
	if (bLightsOn)
		DrawLightMask(pDestSurface, nMouseX, nMouseY, 0.85f/this->fDarkFactor + 0.003f * RAND(100));

	g_pTheBM->BlitSurface(this->images[FOREGROUND_IMAGE], NULL, pDestSurface, NULL);

	//Lights turn on when the environment becomes too dark.
	if (bLightsOn) {
		//Light in windows flickers.
		static const float fLightWavering = 0.2f;
		static const float fDimLights = 1.0f - fLightWavering;
		static float fLightValue = 0.0f;

		static Uint32 dwTimeOfLastFlicker = 0;
		static const Uint32 flickerFPS = 12;
		static const Uint32 flickerMS = 1000/flickerFPS;
		const Uint32 dwNow = SDL_GetTicks();
		if (dwNow - dwTimeOfLastFlicker >= flickerMS) {
			dwTimeOfLastFlicker = dwNow;
			fLightValue = fDimLights + fRAND(fLightWavering);
		}

		static const SDL_Rect lightsRectLeft = {245,230, 154,122};
		static const SDL_Rect lightsRectRight = {612,140, 287,362};
		g_pTheBM->AddMaskAdditive(this->images[WINDOW_LIGHTS_MASK], lightsRectLeft, pDestSurface, lightsRectLeft, fLightValue, true);
		g_pTheBM->AddMaskAdditive(this->images[WINDOW_LIGHTS_MASK], lightsRectRight, pDestSurface, lightsRectRight, fLightValue, true);
	}

	PaintChildren();

	AnimateCaravelLogo(pDestSurface);

	this->pEffects->DrawEffects(!bAlpha);

	if (this->pStatusDialog->IsVisible())
		this->pStatusDialog->Paint();

	if (bUpdate)
	{
		if (bAlpha)
			UpdateRect();
		else
			g_pTheBM->UpdateRects(pDestSurface);
	}
}

//*****************************************************************************
void CTitleScreen::AnimateCaravelLogo(SDL_Surface *pDestSurface)
{
	static const Uint32 FPS = 18;
	static const Uint32 updateMS = 1000/FPS;

	static Uint32 dwTimeOfLastUpdate = 0;
	const Uint32 dwNow = SDL_GetTicks();

	bool update = false;
	if (dwNow - dwTimeOfLastUpdate >= updateMS) {
		dwTimeOfLastUpdate = dwNow;
		update = true;
	}

	AnimateWaves(pDestSurface, update);
	AnimateFlag(pDestSurface, update);
}

//*****************************************************************************
void CTitleScreen::AnimateWaves(SDL_Surface *pDestSurface, bool update)
//Animates the waves in the Caravel logo.
{
	CImageWidget *pCaravelLogo = DYN_CAST(CImageWidget*, CWidget*, GetWidget(TAG_CARAVEL_LOGO));

	//Waves area.
	const int X_WAVES = 38; //for logo in SE corner
	const int Y_WAVES = 94;
	const UINT CX_WAVES = 44;
	const UINT CY_WAVES = 3;

	static UINT wIndex=0;

	if (update) {
		++wIndex;
		if (wIndex==CX_WAVES) wIndex=0;
	}

	//Draw left side of waves.
	SDL_Rect Src = {X_WAVES+wIndex, Y_WAVES, CX_WAVES-wIndex, CY_WAVES};
	SDL_Rect Dest = {pCaravelLogo->GetX() + X_WAVES, pCaravelLogo->GetY() + Y_WAVES, CX_WAVES-wIndex, CY_WAVES};
	SDL_BlitSurface(pCaravelLogo->GetImageSurface(), &Src, pDestSurface, &Dest);
	UpdateRect(Dest);

	//Draw right side of waves.
	if (wIndex)  
	{
		Src.x = X_WAVES;
		Src.w = wIndex;
		Dest.x = pCaravelLogo->GetX() + X_WAVES+CX_WAVES-wIndex;
		Dest.w = wIndex;
		SDL_BlitSurface(pCaravelLogo->GetImageSurface(), &Src, pDestSurface, &Dest);
		UpdateRect(Dest);
	}
}

//*****************************************************************************
void CTitleScreen::AnimateFlag(SDL_Surface *pDestSurface, bool update)
//Animates the flag in the Caravel logo.
{
	CImageWidget *pCaravelLogo = DYN_CAST(CImageWidget*, CWidget*, GetWidget(TAG_CARAVEL_LOGO));

	//Flag area.
	const int X_FLAG = 55; //for logo in SE corner
	const int Y_FLAG = 16;
	const UINT CX_FLAG = 11;
	const UINT CY_FLAG = 4;

	static UINT wIndex=0;

	if (update) {
		++wIndex;
		if (wIndex==CX_FLAG) wIndex=0;
	}

	//Draw left side of flag.
	SDL_Rect Src = {X_FLAG+wIndex, Y_FLAG, CX_FLAG-wIndex, CY_FLAG};
	SDL_Rect Dest = {pCaravelLogo->GetX() + X_FLAG, pCaravelLogo->GetY() + Y_FLAG, CX_FLAG-wIndex, CY_FLAG};
	SDL_BlitSurface(pCaravelLogo->GetImageSurface(), &Src, pDestSurface, &Dest);
	UpdateRect(Dest);

	//Draw right side of flag.
	if (wIndex)  
	{
		Src.x = X_FLAG;
		Src.w = wIndex;
		Dest.x = pCaravelLogo->GetX() + X_FLAG+CX_FLAG-wIndex;
		Dest.y = pCaravelLogo->GetY() + Y_FLAG + 1;
		Dest.w = wIndex;
		SDL_BlitSurface(pCaravelLogo->GetImageSurface(), &Src, pDestSurface, &Dest);
		UpdateRect(Dest);
	}
}

//*****************************************************************************
void CTitleScreen::ResetCNetStatus()
//CaravelNet and internet settings might change on other screens where player
//data and/or the active player are altered.
//Invoke this method to query CaravelNet status again on return to this screen.
{
	CImageWidget *pInternetIcon = DYN_CAST(CImageWidget*, CWidget*, GetWidget(TAG_INTERNET_ICON));
	pInternetIcon->SetImage(wszSignalNo);
	this->wstrNewsText = wszEmpty;
	this->pMarqueeWidget->RemoveParts();

	this->pPlayMenu->Enable(MNU_CHAT, false);

	//When screen is exited, unload assets so they will be reloaded next activation.
	this->bReloadGraphics = true;
}

//*****************************************************************************
void CTitleScreen::RequestNews()
//Send Internet request for news text.
{
	ASSERT(!this->wNewsHandle);

	string ver, newsQuery = CNetInterface::cNetBaseURL + "gamenews.php?game=";
	newsQuery += szDROD;
	newsQuery += "&version=";
	UnicodeToAscii(wszVersionReleaseNumber, ver);
	newsQuery += ver;
#ifdef BETA
	newsQuery += "-BETA-";
	newsQuery += __DATE__;
#endif
	newsQuery += "&language=";
	newsQuery += Language::GetCode(Language::GetLanguage());
	newsQuery += "&OS=";
#ifdef WIN32
	newsQuery += "Windows";
#elif defined(__linux__)
	newsQuery += "Linux";
#elif defined(__FreeBSD__)
	newsQuery += "FreeBSD";
#elif defined(__APPLE__)
	newsQuery += "Apple";
#else
	newsQuery += "Unknown";
#endif

#if defined (BETA)
	int nSpacePos;
	while ((nSpacePos = newsQuery.find(' ', 0)) != -1)
		newsQuery.replace(nSpacePos, 1, "-");
#endif

	CInternet::HttpGet(newsQuery, &this->wNewsHandle);
}

//*****************************************************************************
void CTitleScreen::SetMenuOptionStatus()
//Sets whether menu options are available.
//Post-Cond: sets this->bSavedGameExists
{
	CDb db;

	this->pMenu->Enable(MNU_DEMO, !this->ShowSequenceDemoIDs.empty());

	const UINT dwPlayerID = db.GetPlayerID();
	this->pMenu->Enable(MNU_PLAYMENU, dwPlayerID != 0);
	this->pMenu->Enable(MNU_SETTINGS, dwPlayerID != 0);

	//Check for Tutorial hold.
	this->pPlayMenu->Enable(MNU_TUTORIAL, g_pTheDB->Holds.GetHoldIDWithStatus(CDbHold::Tutorial) != 0);

	db.SavedGames.FilterByHold(g_pTheDB->GetHoldID());
	db.SavedGames.FilterByPlayer(g_pTheDB->GetPlayerID());
	CIDSet SavedGameIDs = db.SavedGames.GetIDs();
	//Saved games for continue slot and demos won't be counted since they are hidden.
	this->bSavedGameExists = !SavedGameIDs.empty();
	this->pPlayMenu->Enable(MNU_RESTORE, this->bSavedGameExists);

	CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*,
			g_pTheSM->GetScreen(SCR_Game));
	ASSERT(pGameScreen);
	const UINT dwHoldID = db.GetHoldID();	//sets hold ID, if needed
	const UINT dwContinueID = g_pTheDB->SavedGames.FindByContinue();
	const bool bGameInProgress = pGameScreen->IsGameLoaded() || dwContinueID;
	this->pPlayMenu->Enable(MNU_NEWGAME, dwHoldID != 0);
	this->pPlayMenu->Enable(MNU_CONTINUE, bGameInProgress);

	this->pPlayMenu->Enable(MNU_CHAT, g_pTheNet->IsLoggedIn());
}

//*****************************************************************************
void CTitleScreen::SetNewsText()
//Parse news text to add the proper widgets to the marquee.
{
	this->pMarqueeWidget->RemoveParts();
	this->pMarqueeWidget->AddPart(MP_Speed, 20);

	static const WCHAR www[] = {We('w'),We('w'),We('w'),We('.'),We(0)};
	static const WCHAR http[] = {We('h'),We('t'),We('t'),We('p'),We(':'),We('/'),We('/'),We(0)};
	static const WCHAR https[] = {We('h'),We('t'),We('t'),We('p'),We('s'),We(':'),We('/'),We('/'),We(0)};

	//Tokenize text based on semantic context.
	const WSTRING& wStr = this->wstrNewsText;
	int nIndex = 0, nFound, nSkipSize;
	CLabelWidget *pText;
	CHyperLinkWidget *pURL;
	UINT dwHyperLinkTag = TAG_HYPERLINK_START;
	do {
		//Search for internet links.
		nSkipSize = 0;
		if ((nFound = wStr.find(http, nIndex)) >= 0)
			nSkipSize = WCSlen(http);
		else if ((nFound = wStr.find(https, nIndex)) >= 0)
			nSkipSize = WCSlen(https);
		else nFound = wStr.find(www, nIndex);

		if (nFound < 0)
		{
			//Nothing special -- add rest of text.
			pText = new CLabelWidget(0, 0, 0, 0, 50, F_TitleMarquee, wszEmpty);
			pText->SetText(wStr.c_str() + nIndex, false,
					nIndex ? g_pTheFM->GetSpaceWidth(F_TitleMarquee) : 0, true);
			this->pMarqueeWidget->AddPart(pText);
			break;
		}

		//URL found.  Add any text before it, then add it.
		ASSERT(nFound >= nIndex);
		if (nFound > nIndex)
		{
			pText = new CLabelWidget(0, 0, 0, 0, 50, F_TitleMarquee, wszEmpty);
			pText->SetText(wStr.substr(nIndex, nFound - nIndex).c_str(),
					false, nIndex ? g_pTheFM->GetSpaceWidth(F_ButtonWhite) : 0, true);
			this->pMarqueeWidget->AddPart(pText);
		}

		//Find where URL ends.
		nIndex = nFound + nSkipSize;
		while (nIndex < (int)wStr.length() && !iswspace(wStr[nIndex]))
			++nIndex;
		pURL = new CHyperLinkWidget(dwHyperLinkTag++, 0, 0, 0, 50,
				F_TitleMarqueeHyperlink, F_TitleMarqueeActiveHyperlink, wszEmpty,
				wStr.substr(nFound, nIndex - nFound).c_str(), false,
				nFound > nIndex ? g_pTheFM->GetSpaceWidth(F_HeaderWhite) : 0);
		pURL->SetText(wStr.substr(nFound + nSkipSize, nIndex - (nFound + nSkipSize)).c_str(),
				false, 0, true);
		this->pMarqueeWidget->AddPart(pURL);

		//Skip trailing whitespace.
		while (nIndex < (int)wStr.length() && iswspace(wStr[nIndex]))
			++nIndex;
	} while (nIndex < (int)wStr.length());

	//Add some space at the end.  Init.
	pText = new CLabelWidget(0, 0, 0, 100, 50, F_TitleMarquee, wszEmpty);
	this->pMarqueeWidget->AddPart(pText);
	this->pMarqueeWidget->Reset();
}

//*****************************************************************************
bool CTitleScreen::UnloadOnDeactivate() const
//Whether to unload graphical assets so they are reloaded next activation.
{
	return true; //always reload assets on screen reentrance in order to change light level
}

//*****************************************************************************
void CTitleScreen::DrawLightMask(SDL_Surface *pDestSurface, int nMouseX, int nMouseY, float fFactor)
{
	//Light mask centered on mouse cursor.
	//Bounded random walk for light jitter.
	static int nXOffset = 0, nYOffset = 0;
	static const int MAX_OFFSET = 2;
	if (RAND(2) && nXOffset < MAX_OFFSET)
		++nXOffset;
	else if (nXOffset > -MAX_OFFSET)
		--nXOffset;
	if (RAND(2) && nYOffset < MAX_OFFSET)
		++nYOffset;
	else if (nYOffset > -MAX_OFFSET)
		--nYOffset;

	{
		static const int nLightMaskW = this->images[LIGHT_MASK]->w;
		static const int nLightMaskH = this->images[LIGHT_MASK]->h;
		SDL_Rect src = {0, 0, nLightMaskW, nLightMaskH}; 
		SDL_Rect dest = {nMouseX + nXOffset - nLightMaskW/2, nMouseY + nYOffset - nLightMaskH/2,
				nLightMaskW, nLightMaskH};
		g_pTheBM->AddMask(this->images[LIGHT_MASK], src, pDestSurface, dest, fFactor);
	}
}
