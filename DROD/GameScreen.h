// $Id: GameScreen.h 9989 2012-03-22 17:18:17Z mrimer $

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
 * Mike Rimer (mrimer)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef GAMESCREEN_H
#define GAMESCREEN_H

#include "RoomScreen.h"

#include "../DRODLib/CurrentGame.h"
#include <BackEndLib/Types.h>

#include <SDL.h>

#include <queue>
using std::deque;

//***************************************************************************************
class CFaceWidget;
class CClockWidget;
class CFiredCharacterCommand;
class CEntranceSelectDialogWidget;
class CSubtitleEffect;
struct VisualEffectInfo;
class CGameScreen : public CRoomScreen
{
public:
	CCurrentGame * GetCurrentGame() const {return this->pCurrentGame;}
	CFaceWidget*   GetFaceWidget() const {return this->pFaceWidget;}
	bool     IsGameLoaded() const {return this->pCurrentGame!=NULL;}
	bool     LoadContinueGame();
	bool     LoadNewGame(const UINT dwHoldID);
	bool     LoadSavedGame(const UINT dwSavedGameID, bool bRestoreFromStart = false, const bool bNoSaves = false);
	static WSTRING PrintRank(const int nRanking, const bool bTie);
	SCREENTYPE     ProcessCommand(const int nCommand, const UINT wX, const UINT wY);
	void		SetGameAmbience(const bool bRecalc=false);
	void     SetMusicStyle();
	bool     ShouldShowLevelStart();
	bool     TestRoom(const UINT dwRoomID, const UINT wX, const UINT wY,
			const UINT wO);
	bool     UnloadGame();

protected:
	friend class CDrodScreenManager;

	CGameScreen(const SCREENTYPE eScreen=SCR_Game);
	virtual ~CGameScreen();

	void           AddSoundEffect(const VisualEffectInfo* pEffect);
	virtual void   ApplyINISettings();
	virtual void   ChatPolling(const UINT tagUserList);
	void           ClearCueEvents();
	void           ClearSpeech(const bool bForceClearAll=true);
	virtual void   DisplayChatText(const WSTRING& text, const SDL_Color& color);
	void           DrawCurrentTurn();
	virtual void   OnActiveEvent(const SDL_ActiveEvent &Active);
	virtual void   OnBetweenEvents();
	virtual void   OnDeactivate();
	virtual void   OnSelectChange(const UINT dwTagNo);
	virtual void   Paint(bool bUpdateRect=true);
	void           PaintClock(const bool bShowImmediately=false);
	void           PlaySoundEffect(const UINT eSEID, float* pos=NULL, float* vel=NULL,
			const bool bUseVoiceVolume=false, float frequencyMultiplier=1.0f);
	SCREENTYPE     ProcessCommand(const int nCommand, const bool bMacro=false);
	bool           ProcessExitLevelEvents(CCueEvents& CueEvents, SCREENTYPE& eNextScreen);
	void           ProcessFuseBurningEvents(CCueEvents& CueEvents);
	void           ProcessMovieEvents(CCueEvents& CueEvents);
	void           ProcessQuestionPrompts(CCueEvents& CueEvents, SCREENTYPE& eNextScreen);
	void           RetainSubtitleCleanup(const bool bVal=false);
	virtual bool   SetForActivate();
	virtual bool   UnloadOnDeactivate() const {return false;}

	//These are called by CDemoScreen.
	void           SetSignTextToCurrentRoom();

	//These are accessed by CDemoScreen.
	CCurrentGame * pCurrentGame;
	CRoomWidget *  pRoomWidget;
	CCueEvents     sCueEvents; //declared static to retain cue events fired when going between screens

private:
	void           AddComboEffect(CCueEvents& CueEvents);
	void           AddDamageEffect(const UINT wMonsterType, const CMoveCoord& coord);
	void           AddRoomStatsDialog();
	void           AmbientSoundSetup();
	void           ApplyPlayerSettings();
	bool           CanShowVarUpdates();
	void           CutSpeech(const bool bForceClearAll=true);
	void           DeleteCurrentGame();
	void           DisplayRoomStats();
	void           FadeRoom(const bool bFadeIn, const Uint32 dwDuration);
	UINT           GetEffectDuration(const UINT baseDuration) const;
	WSTRING        GetGameStats(const bool bHoldTotals=false, const bool bOnlyCurrentGameRooms=false) const;
	UINT           GetMessageAnswer(const CMonsterMessage *pMsg);
	UINT           GetParticleSpeed(const UINT baseSpeed) const;
	UINT           GetPlayerClearSEID() const;
	void           GotoHelpPage();
	void           HandleEventsForHoldExit();
	SCREENTYPE     HandleEventsForLevelExit();
	bool           HandleEventsForPlayerDeath(CCueEvents &CueEvents);
	void           HideBigMap();
	bool           IsMusicStyleFrozen(CCueEvents& CueEvents) const;
	SCREENTYPE     LevelExit_OnKeydown(const SDL_KeyboardEvent &KeyboardEvent);
	void           LogHoldVars();
	virtual void   OnClick(const UINT dwTagNo);
	virtual void   OnDoubleClick(const UINT dwTagNo);
	virtual void   OnKeyDown(const UINT dwTagNo, const SDL_KeyboardEvent &KeyboardEvent);
	virtual void   OnMouseMotion(const UINT dwTagNo, const SDL_MouseMotionEvent &MotionEvent);
	void           OpenStatsBox();
	void           PlayAmbientSound(const UINT dwDataID, const bool bLoop,
			const UINT wX, const UINT wY);
	void           PlaySpeakerSoundEffect(const UINT eSEID, float* pos=NULL, float* vel=NULL) const;
	void           PrepCustomSpeaker(CFiredCharacterCommand *pCmd);
	SCREENTYPE     ProcessCueEventsAfterRoomDraw(CCueEvents &CueEvents);
	SCREENTYPE     ProcessCueEventsBeforeRoomDraw(CCueEvents &CueEvents);
	void           ProcessSpeech();
	void				ProcessSpeechCues(CCueEvents& CueEvents);
	bool           ProcessSpeechSpeaker(CFiredCharacterCommand *pCommand);
	void           ReattachRetainedSubtitles();
	void           ShowBigMap();
	void           ShowDemosForRoom(const UINT roomID);
	void           ShowLockIcon(const bool bShow=true);
	void           ShowPlayerFace(const bool bOverrideLock=false, const bool bLockMood=false);
	void           ShowRoom(CDbRoom *pRoom);
	void           ShowRoomTemporarily(const UINT roomID);
	void           ShowSpeechLog();
	void           SwirlEffect();
	void           StopAmbientSounds();
	void           SynchScroll();
	void           ToggleBigMap();
	void           UndoMove();
	void           UpdateSound();
	bool           UploadDemoPolling();
	void				UploadExploredRooms(const SAVETYPE eSaveType=ST_Continue);
	void           WaitToUploadDemos();

	bool        bShowLevelStartBeforeActivate;
	bool        bPersistentEventsDrawn;
	bool        bNeedToProcessDelayedQuestions;
	bool        bShowingBigMap;
	bool        bShowingCutScene;

	CFaceWidget   *pFaceWidget;
	CClockWidget  *pClockWidget;
	CDialogWidget *pMenuDialog;
	CEntranceSelectDialogWidget *pSpeechBox;
	CMapWidget *pBigMapWidget;

	//Speech.
	deque<CFiredCharacterCommand*> speech; //speech dialog queued to play
	Uint32 dwNextSpeech; //time next speech can begin
	bool   bShowingSubtitlesWithVoice;	//whether subtitles are shown when voices are playing
	Uint32 dwTimeMinimized; //time game is minimized
	Uint32 dwLastCutSceneMove, dwSavedMoveDuration;

	struct ChannelInfo {
		ChannelInfo()
			: wChannel(0), bUsingPos(false)
			, turnNo(0), scriptID(0), commandIndex(0)
			, pEffect(NULL)
		{
			pos[0] = pos[1] = pos[2] = 0.0;
		}
		ChannelInfo(const UINT channel, const bool bUsingPos=false,
				const float posX=0.0, const float posY=0.0, const float posZ=0.0,
				const UINT turnNo=0, const UINT scriptID=0, const UINT commandIndex=0)
			: wChannel(channel), bUsingPos(bUsingPos)
			, turnNo(turnNo), scriptID(scriptID), commandIndex(commandIndex)
			, pEffect(NULL)
		{
			this->pos[0] = posX; this->pos[1] = posY; this->pos[2] = posZ;
		}
		UINT wChannel;
		bool bUsingPos;
		float pos[3];
		UINT turnNo, scriptID, commandIndex;
		CSubtitleEffect *pEffect;
		WSTRING text;
	};
	vector<ChannelInfo> speechChannels;   //audio channels speech sounds are playing on
	vector<ChannelInfo> ambientChannels;  //audio channels ambient sounds are playing on

	bool        bIsSavedGameStale;
	bool        bPlayTesting;  //from editor
	bool        bRoomClearedOnce; //has room cleared event fired for this room
	bool        bMusicStyleFrozen;	//don't change music mood when set
	UINT        wUndoToTurn; //undo moves back to this turn at once
	UINT        wForbidUndoBeforeTurn; //don't allow undoing past this turn
	//bool			bHoldConquered; //whether player has conquered hold being played
	//CIDSet		roomsPreviouslyConquered; //rooms player has conquered previously in hold being played

	float *fPos;   //position vector

	//Internet uploading.
	UINT wUploadingDemoHandle;
	UINT dwUploadingDemo;

	//Auto-help detection.
	Uint32 dwTimeInRoom, dwLastTime;
	Uint32 dwTotalPlayTime;
};

#endif //...#ifndef GAMESCREEN_H
