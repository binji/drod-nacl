// $Id: RoomWidget.cpp 10063 2012-04-02 18:16:01Z TFMurphy $

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

#include "RoomWidget.h"
#include "DrodBitmapManager.h"
#include "DrodScreen.h"
#include "GameScreen.h"

#include "EvilEyeGazeEffect.h"
#include "PendingBuildEffect.h"
#include "RoomEffectList.h"
#include "SnowflakeEffect.h"
#include "RaindropEffect.h"
#include "SparkEffect.h"
#include "StrikeOrbEffect.h"
#include "TileImageCalcs.h"
#include "TileImageConstants.h"
#include "VarMonitorEffect.h"
#include "ZombieGazeEffect.h"

#include "Light.h"
#include "Rectangle.h"
#include "Sphere.h"

#include <FrontEndLib/Bolt.h>
#include <FrontEndLib/Fade.h>
#include <FrontEndLib/FrameRateEffect.h>
#include <FrontEndLib/Pan.h>
#include <FrontEndLib/ShadeEffect.h>
#include <FrontEndLib/FloatEffect.h>
#include <FrontEndLib/TextEffect.h>
#include <FrontEndLib/TransTileEffect.h>

#include "../DRODLib/Citizen.h"
#include "../DRODLib/Clone.h"
#include "../DRODLib/CurrentGame.h"
#include "../DRODLib/EvilEye.h"
#include "../DRODLib/GameConstants.h"
#include "../DRODLib/PlayerDouble.h"
#include "../DRODLib/Monster.h"
#include "../DRODLib/MonsterPiece.h"
#include "../DRODLib/MonsterFactory.h"
#include "../DRODLib/Neather.h"
#include "../DRODLib/Serpent.h"
#include "../DRODLib/Slayer.h"
#include "../DRODLib/Stalwart.h"
#include "../DRODLib/Character.h"
#include "../DRODLib/TileConstants.h"
#include "../DRODLib/Zombie.h"
#include "../DRODLib/Db.h"
#include "../Texts/MIDs.h"
#include <BackEndLib/Assert.h>
#include <BackEndLib/Exception.h>
#include <BackEndLib/Files.h>
#include <BackEndLib/Ports.h>

#include <math.h>

#define IS_COLROW_IN_DISP(c,r) \
		(static_cast<UINT>(c) < CDrodBitmapManager::DISPLAY_COLS && \
		static_cast<UINT>(r) < CDrodBitmapManager::DISPLAY_ROWS)

//Change monster frame once every 5 seconds, on average.
const int MONSTER_ANIMATION_DELAY = 5;

#define BOLTS_SURFACE    (0)
#define FOG_SURFACE      (1)
#define CLOUD_SURFACE    (2)
#define SUNSHINE_SURFACE (3)

//Deep/Shallow Water Opacity
#define PIT_DEEP_OPACITY        (50) //Opacity of PIT_MOSAIC drawn in Deep Water
#define PIT_SHALLOW_OPACITY     (25) //Opacity of PIT_MOSAIC drawn in Shallow Water
#define PIT_BOTTOM_OPACITY      (80) //Opacity of FLOOR_DIRT drawn in Shallow Water
#define SKY_DEEP_OPACITY       (128) //Opacity of SkyImage drawn in Deep Water
#define SKY_SHALLOW_OPACITY     (64) //Opacity of SkyImage drawn in Shallow Water
#define SKY_BOTTOM_OPACITY      (80) //Opacity of FLOOR_DIRT drawn in Shallow Water

const Uint8 MAX_FOG_OPACITY = 128; //[0,255]
const Uint8 MIN_FOG_OPACITY =  64; //[0,255]

const SURFACECOLOR SpeakerColor[Speaker_Count] = {
	{255, 255,   0},  //Beethro
	{255, 135,  25},  //Halph
	{255, 192, 255},  //Slayer
	{224, 186, 163},  //Negotiator
	{255, 255, 255},  //(none)
	{255, 255, 255},  //(custom)
	{192, 192, 255},  //Citizen 1
	{255, 128, 255},  //Citizen 2
	{ 64, 255,  64},  //Goblin King
	{ 32, 255,  32},  //Goblin
	{ 64, 255, 255},  //Instructor
	{255, 100,   0},  //Mud Coordinator
	{192, 128,  96},  //Rock golem
	{163, 128, 255},  //Tar Technician
	{255,  96,  96},  //Guard
	{255, 255, 255},  //Evil eye (active)
	{210, 210, 200},  //Citizen
	{255, 255, 128},  //Stalwart
	{160, 160, 160},  //roach
	{192, 192, 192},  //roach queen
	{160, 160, 160},  //roach egg
	{128, 128, 128},  //wwing
	{255, 255, 255},  //evil eye
	{255,  96,  96},  //red serpent
	{ 64,  64, 192},  //tar mother
	{ 64,  64, 192},  //tarbaby
	{255,  32,  32},  //brain
	{ 35, 200, 220},  //mimic
	{ 96, 192, 192},  //spider
	{ 96, 255,  96},  //green serpent
	{128, 128, 255},  //blue serpent
	{ 48,  48,  48},  //water skipper
	{ 48,  48,  48},  //water skipper nest
	{220, 220, 220},  //aumtlich
	{255, 255,  28},  //clone
	{160, 160, 160},  //decoy
	{255, 255, 255},  //wubba
	{164, 164, 164},  //seep
	{192, 192,  92},  //fegundo
	{192, 192,  92},  //fegundo ashes
	{192,  64,  64},  //mud mother
	{192,  64,  64},  //mudbaby
	{ 64, 192,  64},  //gel mother
	{ 64, 192,  64},  //gelbaby
	{192, 128,  96},  //rock giant
	{255, 192, 255},  //Citizen 3
	{255, 128, 255},  //Citizen 4
	{255, 255,   0},  //Beethro in disguise
	{255, 192, 255},  //Slayer2
	{255, 135,  25},  //Halph2
	{255, 255, 255},  //(self)
	{255, 200,   0},  //Gunthro
	{255, 255, 255},  //(player)
	{160, 160, 255}   //Stalwart2
};

//Light resolution.
#define LIGHT_SPT (8)	//light samples per tile (NxN)
#define LIGHT_SPT_MINUSONE (LIGHT_SPT-1)	//light samples per tile (NxN)
#define LIGHT_BPP (3)   //RGB
const UINT wLightValuesPerTile = LIGHT_SPT*LIGHT_SPT*LIGHT_BPP;	//NxN RGB
const UINT wLightBytesPerTile = wLightValuesPerTile * sizeof(LIGHTTYPE);
const UINT wLightCellSize[LIGHT_SPT_MINUSONE] = {4,3,3,3,3,3,3};	 //pixels in each sub-tile light cell
const UINT wLightCellPos[LIGHT_SPT_MINUSONE] = {0,4,7,10,13,16,19}; //offset of each sub-tile light cell

//Light palette.
#if (GAME_BYTEORDER == GAME_BYTEORDER_BIG)
const float lightMap[3][NUM_LIGHT_TYPES] = {
	{1.0 ,0.0 ,0.0 ,1.0 ,0.5 ,0.5 ,1.0 ,0.5 ,1.0 ,1.0 ,0.15f,0.65f,0.15f,0.65f,1.0  ,1.0  },
	{1.0 ,0.0 ,1.0 ,0.0 ,0.5 ,1.0 ,0.5 ,1.0 ,1.0 ,0.5 ,0.65f,0.15f,1.0  ,1.0  ,0.15f,0.65f},
	{1.0 ,1.0 ,0.0 ,0.0 ,1.0 ,0.5 ,0.5 ,1.0 ,0.5 ,1.0 ,1.0  ,1.0  ,0.65f,0.15f,0.65f,0.15f}
};
#else
const float lightMap[3][NUM_LIGHT_TYPES] = {
	{1.0 ,1.0 ,0.0 ,0.0 ,1.0 ,0.5 ,0.5 ,1.0 ,0.5 ,1.0 ,1.0  ,1.0  ,0.65f,0.15f,0.65f,0.15f},
	{1.0 ,0.0 ,1.0 ,0.0 ,0.5 ,1.0 ,0.5 ,1.0 ,1.0 ,0.5 ,0.65f,0.15f,1.0  ,1.0  ,0.15f,0.65f},
	{1.0 ,0.0 ,0.0 ,1.0 ,0.5 ,0.5 ,1.0 ,0.5 ,1.0 ,1.0 ,0.15f,0.65f,0.15f,0.65f,1.0  ,1.0  }
};
#endif

//Approximately linearly-scaled darkness.
const float darkMap[NUM_DARK_TYPES] = {
	0.94f, 0.88f, 0.83f, 0.78f, 0.74f, 0.70f, 0.66f, 0.62f,
	0.58f, 0.54f, 0.50f, 0.46f, 0.43f, 0.40f, 0.37f, 0.34f,
	0.31f, 0.29f, 0.27f, 0.25f, 0.23f, 0.21f, 0.19f, 0.17f,
	0.15f, 0.13f, 0.11f, 0.09f, 0.07f, 0.05f, 0.025f, 0.0f
};

//Parameters for ambient light level settings.

//Percent of normal light level to use.
const float fRoomLightLevel[LIGHT_LEVELS] = {1.0f, 0.75f, 0.60f, 0.50f, 0.40f, 0.30f, 0.20f};	   //100% down to 20%
//Maximum light addition.
const float fMaxLightIntensity[LIGHT_LEVELS] = {0.3f, 0.6f, 1.2f, 1.6f, 2.0f, 2.6f, 4.0f};	//30-300% increase
//How many times to emphasize lighting on walls for better look.
const float fWallLightingMultiplier[LIGHT_LEVELS] = {1.5f, 1.75f, 1.9f, 2.0f, 2.1f, 2.3f, 3.0f};
//Since northern walls don't fill a whole tile, this makes light angles look a tiny bit better.
const float Y_LIGHT_OFFSET_KLUDGE = 0.01f;

#define LANTERN_LEVEL (4) //at what darkness level the player uses a lamp

//how much of a tile is lighted by a light source
enum LightedType {L_Dark=0, L_Light=1, L_Partial=2, L_PartialItem=3};
LightedType tileLight[2*MAX_LIGHT_DISTANCE+1][2*MAX_LIGHT_DISTANCE+1];
LightedType subtileLight[LIGHT_SPT][LIGHT_SPT];

//Modelling constants.
const float orbRadius = 0.35f;
const float fNorthWallYCoord = 0.36f;

const SURFACECOLOR BlueGreen = {0, 255, 255};
const SURFACECOLOR Fuschia = {255, 0, 64};
const SURFACECOLOR Orange = {255, 128, 0};
const SURFACECOLOR PaleYellow = {255, 255, 128};
const SURFACECOLOR Red = {255, 0, 0};

//light map index
#define NO_COLOR_INDEX (-1)
#define FROZEN_COLOR_INDEX (1)
#define CLONE_COLOR_INDEX (15)

//
//Public methods.
//

//*****************************************************************************
CRoomWidget::CRoomWidget(
//Constructor.
//
//Params:
	UINT dwSetTagNo,                //(in)   Required params for CWidget
	int nSetX, int nSetY,               //    constructor.
	UINT wSetW, UINT wSetH)
	: CWidget(WT_Room, dwSetTagNo, nSetX, nSetY, wSetW, wSetH)

	, pRoomSnapshotSurface(NULL)

	, wDark(0)

	, dwRoomX(0L), dwRoomY(0L)
	, wShowCol(0), wShowRow(0)

	, pCurrentGame(NULL)
	, pRoom(NULL)
	, pwOSquareTI(NULL), pwFSquareTI(NULL), pwTSquareTI(NULL)//, pwMSquareTI(NULL)
	, pwShadowTI(NULL)
	, pbEdges(NULL), shadows(NULL)
	, bLastVision(false)
	, psTempLightBuffer(NULL), psRoomLight(NULL), psCeilingLight(NULL)
	, psPlayerLight(NULL), psDisplayedLight(NULL)
	, pActiveLight(NULL), pActiveLightedTiles(NULL)
	, bRenderRoom(false), bRenderRoomLight(false), bRenderPlayerLight(false)
	, wLastPlayerLightX(UINT(-1)), wLastPlayerLightY(UINT(-1))

	, bShowingPlayer(true)
	, bShowCheckpoints(true)
	, bShowFrameRate(false), bShowMoveCount(false), bShowVarUpdates(false)
	, bAddNEffect(false)
	, bRequestEvilEyeGaze(false)
	, ghostOpacity(255)

	, dwLastDrawSquareInfoUpdateCount(0L)
	, dwLastAnimationFrame(SDL_GetTicks())

	, dwMovementStepsLeft(0), dwMoveDuration(200), dwCurrentDuration(0)
	, dwLastFrame(0), dwTimeSinceLastFrame(0)
	, bFinishLastMovementNow(false)
	, bAnimateMoves(true), bAnimationInProgress(false)
	, bAllowSleep(false)
	, bNextWispFrame(false)
	, bPlayerSleeping(false)

	, bOutside(false), bSkyVisible(false)
	, dwSkyX(0)
	, bLightning(false)
	, dwLightning(0)
	, bFog(false)
	, fFogX(0), fFogY(0)
	, fFogOldX(0), fFogOldY(0)
	, fFogVX(0), fFogVY(0)
	, cFogLayer(0)
	, bClouds(false)
	, fCloudX(0), fCloudY(0)
	, fCloudOldX(0), fCloudOldY(0)
	, fCloudAngle(0)
	, bSunlight(false)
	, wSnow(0)
	, rain(0)
	, bSkipLightfade(false)
	, pSkyImage(NULL)

	, bAllDirty(true)
	, bWasPlacingDouble(false)
	, bWasInvisible(false)
	, wLastTurn((UINT)-1)
	, pTileInfo(NULL)
	, wLastOrientation(0), wLastX(0), wLastY(0)
	, bLastRaised(false)

	, CX_TILE(CBitmapManager::CX_TILE), CY_TILE(CBitmapManager::CY_TILE)

	, time_of_last_weather_render(0)
	, redrawingRowForWeather(0)
	, need_to_update_room_weather(false)
	, time_of_last_sky_move(0)
{
	this->imageFilenames.push_back(string("Bolts"));
	this->imageFilenames.push_back(string("Fog1"));
	this->imageFilenames.push_back(string("Clouds1"));
	this->imageFilenames.push_back(string("Sunshine1"));

	this->pLastLayerEffects = new CRoomEffectList(this);
	this->pMLayerEffects = new CRoomEffectList(this);
	this->pOLayerEffects = new CRoomEffectList(this);
	this->pTLayerEffects = new CRoomEffectList(this);

	//To store unchanging room image.
	//It doesn't need to be this large, but it fixes some surface offset issues.
	//Only the area corresponding to the location of this widget will be used.
	ASSERT(!this->pRoomSnapshotSurface);
	this->pRoomSnapshotSurface = g_pTheBM->ConvertSurface(SDL_CreateRGBSurface(
			SDL_SWSURFACE, CScreen::CX_SCREEN, CScreen::CY_SCREEN, g_pTheBM->BITS_PER_PIXEL, 0, 0, 0, 0));
	if (!this->pRoomSnapshotSurface) throw CException();

	RemoveHighlight();
}

//*****************************************************************************
bool CRoomWidget::AddDoorEffect(
//Add an effect to display the affect an orb agent has on a door.
//
//Returns: whether the orb agent is on a door
//
//Params:
	COrbAgentData *pOrbAgent)  //(in) Orb agent to display
{
	const UINT wX = pOrbAgent->wX, wY = pOrbAgent->wY;
	ASSERT(this->pRoom->IsValidColRow(wX, wY));

	//Get door type being affected.
	const UINT wOriginalTileNo = this->pRoom->GetOSquare(wX, wY);

	//Only works for doors and lights.
	const bool bLight = bIsLight(this->pRoom->GetTSquare(wX,wY));
	if (!bIsYellowDoor(wOriginalTileNo) && !bLight)
		return false;

	//Contains coords to evaluate.
	CCoordSet drawCoords;
	if (bLight)
		drawCoords.insert(wX,wY);
	else
		this->pRoom->GetAllYellowDoorSquares(wX, wY, drawCoords);

	//Each iteration pops one pair of coordinates for plotting,
	//Exits when there are no more coords in stack to plot.
	UINT wDrawX, wDrawY;
	for (CCoordSet::const_iterator door=drawCoords.begin(); door!=drawCoords.end(); ++door)
	{
		wDrawX = door->wX;
		wDrawY = door->wY;
		ASSERT(this->pRoom->IsValidColRow(wDrawX, wDrawY));
		SetPlot(wDrawX, wDrawY);

		//Plot new tile.
		CCoord coord(wDrawX,wDrawY);
		switch(pOrbAgent->action)
		{
			case OA_NULL:
				//Just highlight the door (mouse is over it).
				AddShadeEffect(wDrawX,wDrawY,PaleYellow);
			break;

			case OA_TOGGLE:
				if (!this->pCurrentGame && !bLight)
					AddLastLayerEffect(new CTransTileEffect(this, coord,
							wOriginalTileNo == T_DOOR_Y ? TI_DOOR_YO : TI_DOOR_Y));
				AddShadeEffect(wDrawX,wDrawY,Orange);
			break;

			case OA_OPEN:
				if (!this->pCurrentGame && !bLight)
					AddLastLayerEffect(new CTransTileEffect(this, coord, TI_DOOR_YO));
				AddShadeEffect(wDrawX,wDrawY,BlueGreen);
			break;

			case OA_CLOSE:
				if (!this->pCurrentGame && !bLight)
					AddLastLayerEffect(new CTransTileEffect(this, coord, TI_DOOR_Y));
				AddShadeEffect(wDrawX,wDrawY,Fuschia);
			break;

			default:
				ASSERT(!"AddDoorEffect: Bad orb agent.");
			break;
		}
	}

	return true;
}

//*****************************************************************************
void CRoomWidget::AddLastLayerEffect(
//Adds an effect to the list of effects drawn after the last layer of the room.
//
//Params:
	CEffect *pEffect) //(in)   Effect to add.  CRoomWidget will take ownership
						//    of the pointer, and caller shouldn't delete.
{
	ASSERT(pEffect);
	this->pLastLayerEffects->AddEffect(pEffect);
}

//*****************************************************************************
void CRoomWidget::AddMLayerEffect(
//Adds an effect to the list of effects drawn after the monster layer of the room.
//
//Params:
	CEffect *pEffect) //(in)   Effect to add.  CRoomWidget will take ownership
						//    of the pointer, and caller shouldn't delete.
{
	ASSERT(pEffect);
	this->pMLayerEffects->AddEffect(pEffect);
}

void CRoomWidget::AddOLayerEffect(CEffect *pEffect)
{
	ASSERT(pEffect);
	this->pOLayerEffects->AddEffect(pEffect);
}

//*****************************************************************************
void CRoomWidget::AddTLayerEffect(
//Adds an effect to the list of effects drawn after the transparent layer of
//the room.
//
//Params:
	CEffect *pEffect) //(in)   Effect to add.  CRoomWidget will take ownership
						//    of the pointer, and caller shouldn't delete.
{
	ASSERT(pEffect);
	this->pTLayerEffects->AddEffect(pEffect);
}

//*****************************************************************************
void CRoomWidget::AddToSubtitles(CSubtitleEffect *pEffect)
//Adds a subtitle effect to the list of subtitles being maintained.
{
	pEffect->AddToSubtitles(this->subtitles);
}

//*****************************************************************************
void CRoomWidget::AddPlayerLight(const bool bAlwaysRefresh) //[default=false]
//Sets a flag placing light on player (if visible), in dark rooms, when room is rendered.
{
	if (!IsPlayerLightRendered())
	{
		//don't need to render any light
		this->wLastPlayerLightX = this->wLastPlayerLightY = UINT(-1);
		return;
	}

	if (bAlwaysRefresh)
	{
		this->bRenderPlayerLight = true;
		return;
	}

	//Use player if in room, otherwise use any NPC Beethro in the room
	ASSERT(this->pCurrentGame);
	CEntity *pPlayer = GetLightholder();

	//Redraw light around player whenever he moves in dark rooms.
	//Also call to erase light.
	if ((IsPlayerLightShowing() &&
			this->pCurrentGame->wTurnNo != this->wLastTurn &&
			pPlayer && (pPlayer->wX != this->wLastPlayerLightX ||
					pPlayer->wY != this->wLastPlayerLightY)) || PlayerLightTurnedOff())
		this->bRenderPlayerLight = true;
}

//*****************************************************************************
void CRoomWidget::AddShadeEffect(
//Adds a Shade effect of given color to room tile.
//
//Params:
	const UINT wX, const UINT wY, const SURFACECOLOR &Color) //(in)
{
	if (!this->pRoom->IsValidColRow(wX,wY))
		return;

	CCoord Coord(wX,wY);
	AddLastLayerEffect(new CShadeEffect(this, Coord, Color));
}

//*****************************************************************************
void CRoomWidget::AddStrikeOrbEffect(
//Add a strike orb effect to room.
//
//Params:
	const COrbData &SetOrbData,   //(in) Orb to be struck.
	bool bDrawOrb)
{
	if (bDrawOrb)
		AddTLayerEffect(
			new CAnimatedTileEffect(this, SetOrbData, 220, TI_ORB_L, false, EORBHIT));
	AddMLayerEffect(
		new CStrikeOrbEffect(this, SetOrbData, this->images[BOLTS_SURFACE], false));
}

//*****************************************************************************
CSubtitleEffect* CRoomWidget::AddSubtitle(
//Adds a line of subtitle text for the given speech command.
//
//Returns: pointer to the subtitle effect this command spawns/adds to
//
//Params:
	CFiredCharacterCommand *pFiredCommand, const Uint32 dwDuration)
{
	CMoveCoord *pCoord = this->pCurrentGame->getSpeakingEntity(pFiredCommand);
	ASSERT(pCoord);

	//Get text.
	ASSERT(pFiredCommand);
	ASSERT(pFiredCommand->pCommand);
	CDbSpeech *pSpeech = pFiredCommand->pCommand->pSpeech;
	ASSERT(pSpeech);
	const WSTRING wStr = pFiredCommand->text;

	//Search for subtitle with this unique pCoord (i.e. object identity, not location).
	SUBTITLES::iterator found = this->subtitles.find(pCoord);
	if (found != this->subtitles.end())
	{
		//Add another line of text to existing subtitle effect.
		CSubtitleEffect *pEffect = found->second;
		pEffect->AddTextLine(wStr.c_str(), dwDuration);
		return pEffect;
	}

	//No subtitle is tied to this object -- instantiate a new subtitle effect.

	//Determine speaker color.
	SPEAKER speaker = SPEAKER(pSpeech->wCharacter);
	switch (speaker)
	{
		case Speaker_Custom:
			 //custom speakers have been resolved in CGameScreen::ProcessSpeechCues
			speaker = Speaker_None;
			break;
		case Speaker_Self:
			 //resolve coloring below
			speaker = Speaker_None;
			break;
		case Speaker_Player:
			//get player's base identity type to determine subtitle color
			speaker = getSpeakerType(MONSTERTYPE(this->pCurrentGame->swordsman.wAppearance));
			break;
		default: break;
	}
	HoldCharacter *pCustomChar = this->pCurrentGame->pHold->GetCharacter(speaker);
	SURFACECOLOR color;
	if (speaker != Speaker_None)
	{
		//Use found speaker's color.
		if (pCustomChar) //resolve custom speaker's speaking appearance
			speaker = getSpeakerType(MONSTERTYPE(pCustomChar->wType));
		ASSERT(speaker < Speaker_Count);
		color = SpeakerColor[speaker];
	} else {
		//No speaker is listed:
		//The type of monster speaking determines subtitle background color.
		UINT wIdentity = pFiredCommand->pSpeakingEntity->GetIdentity();
		if (pCustomChar)
			wIdentity = pCustomChar->wType;

		//Speech from no-graphic NPC uses None coloring.
		if (wIdentity >= CUSTOM_CHARACTER_FIRST) //invalid appearance or none
			wIdentity = M_NONE;

		//Speaker appearance must be resolved by now.
		ASSERT(wIdentity < MONSTER_TYPES ||
				(wIdentity >= CHARACTER_FIRST && wIdentity < CHARACTER_TYPES) ||
				wIdentity == M_NONE);

		color = SpeakerColor[getSpeakerType((MONSTERTYPE)wIdentity)];
	}

	//Background color for citizens matches their station-type color.
	CCitizen *pCitizen = dynamic_cast<CCitizen*>(pCoord);
	if (pCitizen)
	{
		const int colorIndex = pCitizen->StationType();
		if (colorIndex >= 0)
		{
			color.byt1 = Uint8((1.0 + lightMap[0][colorIndex]) * 127.5); //half-saturated
			color.byt2 = Uint8((1.0 + lightMap[1][colorIndex]) * 127.5);
			color.byt3 = Uint8((1.0 + lightMap[2][colorIndex]) * 127.5);
		}
	}

	//Speaker text effect.
	CSubtitleEffect *pSubtitle = new CSubtitleEffect(this, pCoord,
			wStr.c_str(), Black, color, dwDuration);
	if (pFiredCommand->bPseudoMonster)
		pSubtitle->FollowCoord(pCoord, true); //delete pseudo monster when subtitle is done
	AddLastLayerEffect(pSubtitle);
	pSubtitle->AddToSubtitles(this->subtitles);
	return pSubtitle;
}

//*****************************************************************************
void CRoomWidget::AddZombieGazeEffect(
//Add a zombie gaze effect to room.
//
//Params:
      const CMonster *pZombie)        //(in) Zombie sending out gaze.
{
	CZombieGazeEffect *pEffect = new CZombieGazeEffect(this, pZombie);
	AddMLayerEffect(pEffect);

	//Add sparks where gaze hits.
	if (pEffect->endCoord.wO != NO_ORIENTATION) //if gaze does not exit room
		AddTLayerEffect(new CSparkEffect(this, pEffect->endCoord, 10, true, true));
}

//*****************************************************************************
void CRoomWidget::AllowSleep(const bool bVal)
//Whether player sleep fidget may be shown.
{
	this->bAllowSleep = bVal;
	if (!bVal)
	{
		if (this->bPlayerSleeping)
			StopSleeping();
	}
}

//*****************************************************************************
void CRoomWidget::HandleMouseUp(const SDL_MouseButtonEvent &Button)
{
	if (!this->pRoom)
		return;

	if (!GetCurrentGame())
		return;

	//Calculate tile clicked on.
	const UINT wX = (Button.x - this->x) / this->CX_TILE;
	const UINT wY = (Button.y - this->y) / this->CY_TILE;
	if (!this->pRoom->IsValidColRow(wX,wY))
		return;

	if (Button.button == SDL_BUTTON_RIGHT)
	{
		DisplayRoomCoordSubtitle(wX,wY);

		if (this->wDark >= LANTERN_LEVEL && IsLightingRendered())
		{
			this->cursorLight.wX = wX;
			this->cursorLight.wY = wY;
			this->bRenderPlayerLight = true;
			Paint();
		}
	}
	else if (Button.button == SDL_BUTTON_LEFT)
	{
		//Interface for mouse-inputted commands.
		CMonster *pMonster = this->pRoom->GetMonsterAtSquare(wX,wY);
		if (pMonster)
			switch (pMonster->wType)
			{
				case M_CLONE:
				{
					//Switching to a clone.
					CScreen *pScreen = DYN_CAST(CScreen*, CWidget*, this->pParent);
					CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*, pScreen);
					pGameScreen->ProcessCommand(CMD_CLONE, wX, wY);
				}
				return;
				default: break;
			}
		else
		if (this->pCurrentGame->swordsman.wPlacingDoubleType
			&& !this->pCurrentGame->IsPlayerAnsweringQuestions())
		{
			//Placing a double (when there are no unanswered questions taking priority)
			CScreen *pScreen = dynamic_cast<CScreen*>(this->pParent);
			//Allow placing a double with the mouse during an active game session,
			//but not during a demo playback or in a miniroom replay, etc.
			if (pScreen && pScreen->GetScreenType() == SCR_Game)
			{
				CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*, pScreen);
				pGameScreen->ProcessCommand(CMD_DOUBLE, wX, wY);
			}
			return;
		}

		//Highlighting a customized item.
		this->wHighlightX = wX;
		this->wHighlightY = wY;
		HighlightSelectedTile();

		//Clicked on player.
		if (this->pCurrentGame->IsPlayerAt(wX,wY) && !this->bPlayerSleeping)
		{
			UINT eSoundID = SEID_NONE;
			switch (this->pCurrentGame->swordsman.wAppearance)
			{
				case M_CLONE: case M_DECOY: case M_MIMIC:
				case M_BEETHRO: case M_BEETHRO_IN_DISGUISE: eSoundID = SEID_HI; break;
				case M_GUNTHRO: eSoundID = SEID_GB_HI; break;
				case M_NEATHER: eSoundID = SEID_NLAUGHING; break;
				case M_EYE: case M_EYE_ACTIVE: eSoundID = SEID_EVILEYEWOKE; break;
				case M_GOBLIN:	case M_GOBLINKING: eSoundID = SEID_GOB_HI; break;
				case M_TARBABY: case M_MUDBABY: case M_GELBABY:
				case M_TARMOTHER: case M_MUDMOTHER: case M_GELMOTHER:
					eSoundID = SEID_SPLAT; break;
				case M_CITIZEN1: case M_CITIZEN2: case M_GUARD:
				case M_MUDCOORDINATOR: case M_TARTECHNICIAN:
				case M_CITIZEN:
					eSoundID = SEID_CIT_HI; break;
				case M_ROCKGOLEM: case M_ROCKGIANT:
					eSoundID = SEID_ROCK_HI; break;
				case M_WUBBA: eSoundID = SEID_WUBBA; break;
				case M_HALPH: break; //not supported
				case M_HALPH2: eSoundID = SEID_HALPH2_HI; break;
				case M_SLAYER: break; //not supported
				case M_SLAYER2: eSoundID = SEID_SLAYER_HI; break;
				case M_ROACH: case M_QROACH: case M_WWING: case M_REGG:
				case M_SERPENT: case M_SPIDER: case M_SERPENTG: case M_SERPENTB:
				case M_WATERSKIPPER: case M_AUMTLICH: case M_SEEP: case M_FEGUNDO:
					eSoundID = SEID_MON_CLEAR; break;
				case M_NEGOTIATOR:
					eSoundID = SEID_NEGO_HI; break;
				case M_INSTRUCTOR:
				case M_CITIZEN3: case M_CITIZEN4:
					eSoundID = SEID_WOM_HI; break;
				case M_STALWART:
					eSoundID = SEID_STALWART_HI; break;
				case M_STALWART2:
					eSoundID = SEID_SOLDIER_HI; break;
				default: break;
			}

			if (eSoundID != (UINT)SEID_NONE)
				g_pTheSound->PlaySoundEffect(eSoundID, NULL, NULL, true);
		}
	}
}

//*****************************************************************************
void CRoomWidget::HighlightSelectedTile()
//Context-sensitive highlighting of selected tile (if any).
{
	const UINT wX = this->wHighlightX, wY = this->wHighlightY;
	if (!this->pRoom->IsValidColRow(wX,wY))
		return;

	//Reset old display info.
	this->pLastLayerEffects->RemoveEffectsOfType(ESHADE);
	static CMoveCoord briarStats(wX, wY, NO_ORIENTATION);
	SUBTITLES::iterator found = this->subtitles.find(&briarStats);
	if (found != this->subtitles.end())
		found->second->SetToText(NULL, 1);

	ASSERT(this->pCurrentGame);
	CMonster *pMonster = this->pRoom->GetMonsterAtSquare(wX,wY);
	bool bRemoveHighlightNextTurn = true;
	bool bForceHighlightRemoval = false;
	if (pMonster)
	{
		switch (pMonster->wType)
		{
			case M_DECOY:
				DrawInvisibilityRange(wX, wY, NULL);
				bRemoveHighlightNextTurn = false;
			break;

			case M_EYE:
			{
				CEvilEye *pEye = DYN_CAST(CEvilEye*, CMonster*, pMonster);
				if (!pEye->IsAggressive())	//show effect if eye is not active
					AddLastLayerEffect(new CEvilEyeGazeEffect(this,wX,wY,pMonster->wO, 2000));
				bForceHighlightRemoval = true; //must show only one turn even if coupled with persistent effects
			}
			break;

			case M_GELMOTHER:
			{
				CCoordSet tiles;
				this->pRoom->GetTarConnectedComponent(wX,wY, tiles);
				for (CCoordSet::const_iterator tile = tiles.begin(); tile != tiles.end(); ++tile)
					AddShadeEffect(tile->wX, tile->wY, PaleYellow);
				bRemoveHighlightNextTurn = false;
			}
			break;

			case M_CITIZEN:
			{
				//Show citizen's current destination.
				CCitizen *pCitizen = DYN_CAST(CCitizen*, CMonster*, pMonster);
				UINT wDestX, wDestY;
				if (pCitizen->GetGoal(wDestX,wDestY))
					AddShadeEffect(wDestX, wDestY, PaleYellow);
				bRemoveHighlightNextTurn = false;
			}
			break;

			case M_STALWART: case M_STALWART2:
			{
				//Show stalwart's current target.
				CStalwart *pStalwart = DYN_CAST(CStalwart*, CMonster*, pMonster);
				UINT wDestX, wDestY;
				if (!pStalwart->bFrozen && pStalwart->GetGoal(wDestX,wDestY))
					AddShadeEffect(wDestX, wDestY, PaleYellow);
				bRemoveHighlightNextTurn = false;
			}
			break;

			case M_SLAYER: case M_SLAYER2:
			{
				static const SURFACECOLOR Purple = {196, 0, 196};
				//Show slayer's wisp.
				CSlayer *pSlayer = DYN_CAST(CSlayer*, CMonster*, pMonster);
				for (WISP piece = pSlayer->Pieces.begin(); piece != pSlayer->Pieces.end(); ++piece)
				{
					CMonsterPiece *pPiece = *piece;
					ASSERT(this->pRoom->IsValidColRow(pPiece->wX, pPiece->wY));
					AddShadeEffect(pPiece->wX, pPiece->wY, Purple);
				}
				bRemoveHighlightNextTurn = false;
			}
			break;

			default: break;
		}
	}

	switch (this->pRoom->GetTSquare(wX,wY))
	{
		case T_ORB:
		{
			COrbData *pOrb = this->pRoom->GetOrbAtCoords(wX,wY);
			if (pOrb)
				for (UINT wIndex=pOrb->agents.size(); wIndex--; )
					AddDoorEffect(pOrb->agents[wIndex]);
			bRemoveHighlightNextTurn = false;
		}
		break;
		case T_BOMB:
			DrawInvisibilityRange(wX, wY, NULL, NULL, 3);
			bRemoveHighlightNextTurn = false;
		break;
		case T_FLOW_SOURCE:
		{
			//How many turns until this root grows: edge pieces / roots.
			const UINT index = this->pRoom->briars.getIndexAt(wX,wY);
			ASSERT(index); //every root should have an index
			const UINT numRoots = this->pRoom->briars.getNumSourcesWithIndex(index);
			CCoordSet tiles = this->pRoom->briars.getEdgeTilesFor(wX,wY);
			const UINT numEdgeTiles = tiles.size();

			WSTRING wstr;
			WCHAR temp[16];
			if (numRoots > 1)
			{
				wstr += _itow(numEdgeTiles, temp, 10);
				wstr += wszForwardSlash;
				wstr += _itow(numRoots, temp, 10);
				wstr += wszEqual;
			}
			wstr += _itow(numEdgeTiles/numRoots, temp, 10);

			briarStats.wX = wX;
			briarStats.wY = wY;
			AddInfoSubtitle(&briarStats, wstr, 0); //indefinite
			bRemoveHighlightNextTurn = false;
		}
		break;
		case T_FLOW_EDGE: case T_FLOW_INNER:
		{
			//Highlight this briar's connected component.
			CCoordSet tiles = this->pRoom->briars.getEdgeTilesFor(wX,wY);
			for (CCoordSet::const_iterator tile = tiles.begin(); tile != tiles.end(); ++tile)
				AddShadeEffect(tile->wX, tile->wY, PaleYellow);
			bRemoveHighlightNextTurn = false;
		}
		break;
		default: break;
	}

	switch (this->pRoom->GetFSquare(wX,wY))
	{
		case T_NODIAGONAL:
		{
			for (UINT wY=this->pRoom->wRoomRows; wY--; )
				for (UINT wX=this->pRoom->wRoomCols; wX--; )
					if (this->pRoom->GetFSquare(wX,wY) == T_NODIAGONAL)
						AddShadeEffect(wX, wY, PaleYellow);
			bRemoveHighlightNextTurn = false;
		}
		break;
		default: break;
	}

	const UINT wOSquare = this->pRoom->GetOSquare(wX,wY);
	switch (wOSquare)
	{
		//Highlight tunnel exit.
		case T_TUNNEL_E: case T_TUNNEL_W: case T_TUNNEL_N: case T_TUNNEL_S:
		{
			int dx=0, dy=0;
			switch (wOSquare)
			{
				case T_TUNNEL_E: dx = 1; dy = 0; break;
				case T_TUNNEL_W: dx = -1; dy = 0; break;
				case T_TUNNEL_N: dx = 0; dy = -1; break;
				case T_TUNNEL_S: dx = 0; dy = 1; break;
				default: ASSERT(!"Invalid tunnel type"); break;
			}
			UINT wExitX, wExitY;
			if (this->pCurrentGame->TunnelGetExit(wX, wY, dx, dy, wExitX, wExitY))
				AddShadeEffect(wExitX, wExitY, PaleYellow);
			bRemoveHighlightNextTurn = false;
		}
		break;
		case T_PRESSPLATE:
		{
			COrbData *pPlate = this->pRoom->GetPressurePlateAtCoords(wX,wY);
			if (pPlate)
				for (UINT wIndex=pPlate->agents.size(); wIndex--; )
					AddDoorEffect(pPlate->agents[wIndex]);
			bRemoveHighlightNextTurn = false;
		}
		break;
		default: break;
	}

	if (bRemoveHighlightNextTurn || bForceHighlightRemoval)
		RemoveHighlight();
}

//*****************************************************************************
void CRoomWidget::AddInfoSubtitle(
	CMoveCoord* pCoord, const WSTRING& wstr, const Uint32 dwDuration,
	const UINT displayLines, const SDL_Color& color, const UINT fadeDuration) //default=[1,Black,500]
{
	ASSERT(pCoord);

	SUBTITLES::iterator found = this->subtitles.find(pCoord);
	if (found != this->subtitles.end())
	{
		//Add text to existing subtitle effect.
		found->second->AddTextLine(wstr.c_str(), dwDuration, color);
	} else {
		CSubtitleEffect *pEffect = new CSubtitleEffect(this, pCoord, wstr.c_str(),
				color, SpeakerColor[Speaker_None], dwDuration, displayLines);
		pEffect->SetFadeDuration(fadeDuration);
		AddLastLayerEffect(pEffect);
		pEffect->AddToSubtitles(this->subtitles);
	}
}

//*****************************************************************************
void CRoomWidget::ClearEffects(
//Clears all effects in the room.
//
//Params:
	const bool bKeepInfoTexts) //(in) If true (default), info text effects will persist
	                           //     and old effects will be erased from room
{
	SynchRoomToCurrentGame();

	//If bKeepInfoTexts is true, then erase (i.e. redraw) effect areas also
	//because the caller is going to keep showing the room.
	const bool bRepaint = bKeepInfoTexts;

	//Clear layer lists except for persistent display effects.
	this->pOLayerEffects->Clear(bRepaint, !bKeepInfoTexts);
	this->pTLayerEffects->Clear(bRepaint, !bKeepInfoTexts);
	this->pMLayerEffects->Clear(bRepaint, !bKeepInfoTexts);
	this->pLastLayerEffects->Clear(bRepaint, !bKeepInfoTexts);

	RemoveHighlight();

	//If these effects were removed, then reset their display flags.
	if (!bKeepInfoTexts)
		this->bShowFrameRate = this->bShowMoveCount = this->bShowVarUpdates = false;
}

//*****************************************************************************
void CRoomWidget::ClearLights()
//Zeroes all light arrays.
{
	ASSERT(this->pRoom);
	const UINT wBytes = this->pRoom->CalcRoomArea() * wLightBytesPerTile;
	if (this->psRoomLight)
	{
		memset(this->psTempLightBuffer, 0, wBytes);
		memset(this->psRoomLight, 0, wBytes);
		memset(this->psCeilingLight, 0, wBytes);
		memset(this->psPlayerLight, 0, wBytes);
		memset(this->psDisplayedLight, 0, wBytes);
	}
	this->lightedPlayerTiles.clear();
	this->lightedRoomTiles.clear();
	this->partialLightedTiles.clear();

	this->tileLightInfo.Init(this->pRoom->wRoomCols, this->pRoom->wRoomRows);
}

//*****************************************************************************
void CRoomWidget::DisplayChatText(const WSTRING& text, const SDL_Color& color)
//Display chat text as a subtitle in the corner of the room.
{
	static CMoveCoord coord(0, 0, NO_ORIENTATION);
	if (this->pCurrentGame) {
		coord.wX = this->pCurrentGame->swordsman.wX >= this->pRoom->wRoomCols/2 ?
				0 : this->pRoom->wRoomCols-1;
		coord.wY = this->pCurrentGame->swordsman.wY >= this->pRoom->wRoomRows/2 ?
				0 : this->pRoom->wRoomRows-1;
	} else {
		coord.wX = coord.wY = 0;
	}

	//When displaying many new chat texts at the same time,
	//extend the display duration up to 30s.
	static const UINT MAX_DISPLAY_DURATION = 30000; //ms
	static const UINT CHAT_FADE_DURATION = 5000;
	static const UINT CHAT_LINES = 7;
	UINT displayDuration = 15000;
	SUBTITLES::iterator found = this->subtitles.find(&coord);
	if (found != this->subtitles.end())
	{
		//Add text to existing subtitle effect.
		displayDuration += found->second->GetDisplayTimeRemaining();
	}
	if (displayDuration > MAX_DISPLAY_DURATION)
		displayDuration = MAX_DISPLAY_DURATION;

	AddInfoSubtitle(&coord, text, displayDuration, CHAT_LINES, color, CHAT_FADE_DURATION);

	//Don't offset subtitle effect by any tiles to appear more compact.
	found = this->subtitles.find(&coord);
	if (found != this->subtitles.end())
	{
		CSubtitleEffect& effect = *(found->second);
		effect.SetEffectType(ECHATTEXT);
		effect.SetOffset(0, 0);
		effect.RequestRetainOnClear();
	}
}

//*****************************************************************************
void CRoomWidget::DisplayRoomCoordSubtitle(const int nMouseX, const int nMouseY)
//Display room coords as a subtitle, based on mouse's screen position.
{
	const UINT wX = (nMouseX - this->x) / this->CX_TILE;
	const UINT wY = (nMouseY - this->y) / this->CY_TILE;
	DisplayRoomCoordSubtitle(wX,wY);
}

//*****************************************************************************
void CRoomWidget::DisplayRoomCoordSubtitle(const UINT wX, const UINT wY)
//Display room coords at (x,y), along with what item is there, as a subtitle.
{
	if (!this->pRoom->IsValidColRow(wX,wY)) return;

	static CMoveCoord coord(wX, wY, NO_ORIENTATION);
	coord.wX = wX;
	coord.wY = wY;

	WSTRING wstr = wszLeftParen;
	WCHAR temp[12];
	wstr += _itoW(wX, temp, 10);
	wstr += wszComma;
	wstr += _itoW(wY, temp, 10);
	wstr += wszRightParen;

	//Identify items on tile.
#define AppendLine(mid) if (mid) {wstr += wszCRLF; wstr += g_pTheDB->GetMessageText(mid);}
	UINT mid = 0;

	//Player.
	if (this->pCurrentGame && this->pCurrentGame->IsPlayerAt(wX,wY))
	{
		AppendLine(MID_Player);
	}

	//Monster.
	CMonster *pMonster = this->pRoom->GetMonsterAtSquare(wX, wY);
	if (pMonster)
	{
		if (pMonster->IsPiece())
		{
			CMonsterPiece *pPiece = DYN_CAST(CMonsterPiece*, CMonster*, pMonster);
			pMonster = pPiece->pMonster;
		}
		mid = TILE_MID[pMonster->wType + M_OFFSET];
		AppendLine(mid);

		//Indicate citizen color.
		if (pMonster->wType == M_CITIZEN)
		{
			CCitizen *pCitizen = DYN_CAST(CCitizen*, CMonster*, pMonster);
			const int stationType = pCitizen->StationType();
			if (stationType >= 0)
			{
				wstr += wszSpace;
				wstr += wszPoundSign;
				wstr += _itoW(stationType, temp, 10);
			}
		}

		//Indicate serpent length.
		if (bIsSerpent(pMonster->wType))
		{
			CSerpent *pSerpent = DYN_CAST(CSerpent*, CMonster*, pMonster);
			const int snakeLength = pSerpent->GetLength();
			wstr += wszSpace;
			wstr += wszLeftBracket;
			wstr += _itoW(snakeLength, temp, 10);
			wstr += g_pTheDB->GetMessageText(MID_SnakeLengthAffix);
			wstr += wszRightBracket;
		}

		//Indicate monster's position in movement order sequence.
		UINT index=1;
		for (CMonster *pTrav = this->pRoom->pFirstMonster; pTrav != NULL;
				pTrav = pTrav->pNext, ++index)
		{
			if (pMonster == pTrav)
			{
				wstr += wszSpace;
				wstr += wszLeftParen;
				wstr += wszPoundSign;
				wstr += _itoW(index, temp, 10);
				wstr += wszRightParen;
				break;
			}
		}

	}

	//Items.
	const UINT tTile = this->pRoom->GetTSquare(wX, wY);
	if (tTile != T_EMPTY)
	{
		switch (tTile)
		{
			case T_TOKEN:
				mid = GetTokenMID(this->pRoom->GetTParam(wX, wY)); break;
			case T_ORB:
			{
				COrbData *pOrb = this->pRoom->GetOrbAtCoords(wX, wY);
				mid = GetOrbMID(pOrb ? pOrb->eType : OT_NORMAL);
			}
			default: mid = TILE_MID[tTile]; break;
		}
		AppendLine(mid);

		//Indicate station color.
		if (tTile == T_STATION)
		{
			wstr += wszSpace;
			wstr += wszLeftParen;
			wstr += wszPoundSign;
			wstr += _itoW(this->pRoom->GetTParam(wX, wY), temp, 10);
			wstr += wszRightParen;
		}
	}

	//Flat layer.
	const UINT fTile = this->pRoom->GetFSquare(wX, wY);
	if (fTile != 0)
	{
		mid = TILE_MID[fTile];
		AppendLine(mid);
		if (bIsArrow(fTile))
		{
			wstr += wszSpace;
			wstr += wszLeftParen;

			UINT arrowmid = 0;
			switch (fTile)
			{
				case T_ARROW_N:  arrowmid = MID_North; break;
				case T_ARROW_NE: arrowmid = MID_NorthEast; break;
				case T_ARROW_E:  arrowmid = MID_East; break;
				case T_ARROW_SE: arrowmid = MID_SouthEast; break;
				case T_ARROW_S:  arrowmid = MID_South; break;
				case T_ARROW_SW: arrowmid = MID_SouthWest; break;
				case T_ARROW_W:  arrowmid = MID_West; break;
				case T_ARROW_NW: arrowmid = MID_NorthWest; break;
			}
			ASSERT(arrowmid);
			wstr += g_pTheDB->GetMessageText(arrowmid);
			wstr += wszRightParen;
		}
	}
	if (this->pRoom->checkpoints.has(wX, wY))
	{
		mid = TILE_MID[T_CHECKPOINT];
		AppendLine(mid);
	}

	//Always describe o-layer.
	const UINT oTile = this->pRoom->GetOSquare(wX, wY);
	switch (oTile)
	{
		case T_PRESSPLATE:
		{
			COrbData *pPlate = this->pRoom->GetPressurePlateAtCoords(wX,wY);
			mid = GetPressurePlateMID(pPlate ? pPlate->eType : OT_NORMAL);
		}
		break;
		default: mid = TILE_MID[oTile]; break;
	}

	AppendLine(mid);

	//Build marker.
	if (this->pRoom->building.get(wX,wY))
	{
		mid = TILE_MID[this->pRoom->building.get(wX,wY) - 1];
		if (mid)
		{
			wstr += wszCRLF;
			wstr += g_pTheDB->GetMessageText(MID_BuildMarker);
			wstr += wszColon;
			wstr += wszSpace;
			wstr += g_pTheDB->GetMessageText(mid);
		}
	}
#undef AppendLine

	AddInfoSubtitle(&coord, wstr, 2000);
}

//*****************************************************************************
void CRoomWidget::DisplaySubtitle(
//Displays a notification at (x,y) that room exiting is locked.
//
//
	const WCHAR *pText, const UINT wX, const UINT wY, const bool bReplace)
{
	if (!this->pRoom->IsValidColRow(wX,wY)) return;
	if (!pText) return;

	static CMoveCoord coord(wX, wY, NO_ORIENTATION);
	CMoveCoord *pCoord = &coord;
	coord.wX = wX;
	coord.wY = wY;

	SUBTITLES::iterator found = this->subtitles.find(pCoord);
	static const int dwDuration = 2000;
	if (found != this->subtitles.end())
	{
		//Replace text or add another line of text to existing subtitle effect.
		if (bReplace)
			found->second->SetToText(pText, dwDuration);
		else
			found->second->AddTextLine(pText, dwDuration);
	} else {
		CSubtitleEffect *pEffect = new CSubtitleEffect(this, pCoord, pText,
				Black, SpeakerColor[Speaker_None], dwDuration, 1);
		AddLastLayerEffect(pEffect);
		pEffect->AddToSubtitles(this->subtitles);
	}
}

//*****************************************************************************
void CRoomWidget::DontAnimateMove()
//Call if the game move just played should not be animated.
{
	if (this->pCurrentGame)
	{
		this->wLastTurn = this->pCurrentGame->wTurnNo;
		SetMoveCountText();
	}
	BetterVisionQuery();
	FinishMoveAnimation();
}

//*****************************************************************************
UINT CRoomWidget::GetCustomEntityTile(
//Determines what sprite tile to show for this custom character role.
//
//Returns: index of custom sprite to show, or TI_UNSPECIFIED if not defined
//
//Params:
	const UINT wLogicalIdentity,      //role's logical ID
	const UINT wO, const UINT wFrame) //orientation and frame number
const
{
	//Check for a custom character tileset.
	if (this->pCurrentGame)
	{
		ASSERT(this->pCurrentGame->pHold);
		HoldCharacter *pCustomChar = this->pCurrentGame->pHold->GetCharacter(wLogicalIdentity);
		return GetCustomEntityTileFor(pCustomChar, wO, wFrame);
	}

	return TI_UNSPECIFIED; //no custom role tiles provided
}

//*****************************************************************************
UINT CRoomWidget::GetCustomEntityTileFor(
	HoldCharacter *pCustomChar,
	const UINT wO, const UINT wFrame) //orientation and frame number
const
{
	if (pCustomChar)
	{
		const UINT wIndex = GetCustomTileIndex(wO);
		UINT wCustomTileImageNo = g_pTheBM->GetCustomTileNo(pCustomChar->dwDataID_Tiles, wIndex, wFrame);
		//Robust frame fetching if not all frames are available in this custom tileset.
		if (wCustomTileImageNo == TI_UNSPECIFIED && wFrame > 0) //allow having only one animation frame
			wCustomTileImageNo = g_pTheBM->GetCustomTileNo(pCustomChar->dwDataID_Tiles, wIndex, 0);
		if (wCustomTileImageNo == TI_UNSPECIFIED && wIndex > 0) //get first orientation frame
			wCustomTileImageNo = g_pTheBM->GetCustomTileNo(pCustomChar->dwDataID_Tiles, 0, wFrame);
		if (wCustomTileImageNo == TI_UNSPECIFIED && wFrame > 0 && wIndex > 0) //get only frame available
			wCustomTileImageNo = g_pTheBM->GetCustomTileNo(pCustomChar->dwDataID_Tiles, 0, 0);
		//if (wCustomTileImageNo != TI_UNSPECIFIED) //this line is not needed since the expected value is returned either way
		return wCustomTileImageNo;
	}

	return TI_UNSPECIFIED;
}

//*****************************************************************************
UINT CRoomWidget::GetCustomTileIndex(const UINT wO)
//Returns: custom image tile index for a given orientation
{
	switch (wO)
	{
		case N: return 0;
		case NE: return 1;
		case E: return 2;
		case SE: return 3;
		case S: return 4;
		case SW: return 5;
		case W: return 6;
		case NW: return 7;
		case NO_ORIENTATION: return 8;
		default: ASSERT(!"Bad orientation"); return 0;
	}
}

//*****************************************************************************
UINT CRoomWidget::GetEntityTile(
//Determines what sprite tile to show for this character role.
//
//Params:
	const UINT wApparentIdentity,     //what stock entity the role looks like
	const UINT wLogicalIdentity,      //role's logical ID
	const UINT wO, const UINT wFrame) //orientation and frame number
const
{
	const UINT tile = GetCustomEntityTile(wLogicalIdentity, wO, wFrame);
	if (tile != TI_UNSPECIFIED)
		return tile;

	//Use stock/default character tileset.
	return GetTileImageForEntity(wApparentIdentity == M_NONE ?
			static_cast<UINT>(CHARACTER_FIRST) : wApparentIdentity, wO, wFrame);
}

/*
// ****************************************************************************
UINT* CRoomWidget::GetMonsterTile(
//
//Params:
	const UINT wCol, const UINT wRow)   //(in) Position of square
//Returns:
//Pointer to animation frame for this square
{
	ASSERT(this->pwMSquareTI);
	ASSERT(wCol < this->pRoom->wRoomCols);
	ASSERT(wRow < this->pRoom->wRoomRows);
	return this->pwMSquareTI + this->pRoom->ARRAYINDEX(wCol,wRow);
}
*/

//*****************************************************************************
void CRoomWidget::GetWeather()
//Get environmental conditions from room data.
{
	if (this->pCurrentGame)
		this->pRoom = this->pCurrentGame->pRoom; //ensure rooms are synched
	ASSERT(this->pRoom);

	if (IsLightingRendered())
	{
		this->wDark = this->pRoom->weather.wLight;
		ASSERT(this->wDark < LIGHT_LEVELS);
		g_pTheDBM->fLightLevel = fRoomLightLevel[this->wDark];
	} else {
		//Full lighting
		this->wDark = 0;
		g_pTheDBM->fLightLevel = 1.0;
	}

	while (!this->playThunder.empty())
		this->playThunder.pop();
	if (!IsWeatherRendered())
	{
		//No weather effects on lower graphics modes.
		this->bOutside = this->bSkyVisible = this->bLightning = this->bClouds =
				this->bSunlight = this->bSkipLightfade = this->bFog = false;
		this->cFogLayer = this->wSnow = this->rain = 0;
		return;
	}

	this->bAllDirty = true;

	this->bOutside = this->pRoom->weather.bOutside;
	this->bSkyVisible = this->bOutside && SkyWillShow();

	this->bLightning = this->pRoom->weather.bLightning;

	this->bClouds = this->pRoom->weather.bClouds;
	ASSERT(this->images[CLOUD_SURFACE]);

	this->bSunlight = this->pRoom->weather.bSunshine;
	ASSERT(this->images[SUNSHINE_SURFACE]);

	this->bSkipLightfade = this->pRoom->weather.bSkipLightfade;

	this->cFogLayer = (BYTE)this->pRoom->weather.wFog;
	ASSERT(this->cFogLayer < FOG_INCREMENTS);
	this->bFog = this->cFogLayer > 0;
	if (this->bFog)
	{
		ASSERT(this->images[FOG_SURFACE]);
		SDL_SetAlpha(this->images[FOG_SURFACE], SDL_SRCALPHA, 128);
	}

	this->wSnow = this->pRoom->weather.wSnow;
	ASSERT(this->wSnow < SNOW_INCREMENTS);

	this->sky = this->pRoom->weather.sky;

	this->rain = this->pRoom->weather.rain;
	ASSERT(this->rain < RAIN_INCREMENTS);
}

//*****************************************************************************
CEntity* CRoomWidget::GetLightholder() const
//Returns: pointer to entity holding a light (lamp) in the room.
//This is the player by default.  However, if the player is not in the room,
//then any NPC Beethro/Gunthro has a light.
{
	if (this->pCurrentGame->swordsman.IsInRoom())
		return &this->pCurrentGame->swordsman;
	return this->pCurrentGame->pRoom->GetNPCBeethro();
}

//*****************************************************************************
bool CRoomWidget::IsLightingRendered() const
//Returns: whether light level effects should be rendered
{
	return g_pTheBM->bAlpha;
}

//*****************************************************************************
bool CRoomWidget::IsPlayerLightShowing() const
//Returns: true if there is a player/NPC Beethro light to show in the room
{
	ASSERT(this->pCurrentGame);
	const CSwordsman& player = this->pCurrentGame->swordsman;
	if (player.IsInRoom()) //if player is in room, light shows when visible
		return player.IsVisible();
	//When player is not in room, any visible NPC Beethro shows light
	if (this->pCurrentGame->pRoom->GetNPCBeethro() != NULL)
		return true;
	return false;
}

//*****************************************************************************
bool CRoomWidget::IsPlayerLightRendered() const
//Returns: true if light is shown around player in the front end.
//Light will be showing in dark rooms when light graphics preferences are enabled.
{
	if (!this->bShowingPlayer)
		return false; //don't show light if player is explicitly hidden

	return this->wDark >= LANTERN_LEVEL && IsLightingRendered() && IsWeatherRendered();
}

//*****************************************************************************
bool CRoomWidget::IsWeatherRendered() const
//Returns: whether weather effects should be rendered
{
	return g_pTheBM->eyeCandy > 0;
}

//*****************************************************************************
bool CRoomWidget::LoadFromCurrentGame(
//Loads widget from current game.
//
//Params:
	CCurrentGame *pSetCurrentGame, const bool bLoad) //[default=true]
//
//Returns:
//True if successful, false if not.
{
	ASSERT(pSetCurrentGame);
	this->pCurrentGame = pSetCurrentGame;
	return LoadFromRoom(pSetCurrentGame->pRoom, bLoad);
}

//*****************************************************************************
bool CRoomWidget::LoadFromRoom(
//Loads widget from current game.
//
//Params:
	CDbRoom *pRoom, //room to display
	const bool bLoad) //[default=true]
//
//Returns:
//True if successful, false if not.
{
	ASSERT(pRoom);
	this->pRoom = pRoom;  //quick access

	//Update vars used for comparison of widget to current game.
	this->style = this->pRoom->style;
	this->dwRoomX = this->pRoom->dwRoomX;
	this->dwRoomY = this->pRoom->dwRoomY;

	//Reset animations.
	this->dwCurrentDuration = this->dwLastFrame = this->dwTimeSinceLastFrame = 0;
	this->wLastTurn = UINT(-1);

	ClearEffects();

	//Load tile images.
	bool bSuccess = true;
	if (bLoad)
	{
		bSuccess = g_pTheDBM->LoadTileImagesForStyle(this->style);
		LoadRoomImages();	//must do before ResetForPaint
	}

	//Set tile image arrays to new current room.  Render room.
	ResetForPaint();

	return bSuccess;
}

//*****************************************************************************
void CRoomWidget::LoadRoomImages()
//Load images specific to this room.
{
	//Query environmental conditions.
	GetWeather();
	if (!this->rain) //raindrop effects should not persist into a non-rainy room
		this->pMLayerEffects->RemoveEffectsOfType(ERAINDROP);

	ClearLights();

	//Load new floor image mosaic, if any.
	if (g_pTheDBM->pTextures[FLOOR_IMAGE])
		SDL_FreeSurface(g_pTheDBM->pTextures[FLOOR_IMAGE]);
	CDbRoom *pThisRoom = this->pCurrentGame ? this->pCurrentGame->pRoom : this->pRoom;
	const UINT dwDataID = pThisRoom->dwDataID;
	g_pTheDBM->pTextures[FLOOR_IMAGE] = g_pTheDBM->LoadImageSurface(dwDataID);

	//Generate room model for lighting.
	this->bRenderRoomLight = true;
	this->bCeilingLightsRendered = false;

	//Load sky image, if applicable.
	if (this->bSkyVisible)
	{
		//Get name of sky image.  Look up default name for this style+light level if none specified.
		WSTRING wstrSkyImageName = this->sky;
		if (wstrSkyImageName.empty())
		{
			WSTRING style = pThisRoom->style;
			g_pTheDBM->ConvertStyle(style);
			style += wszSpace;
			style += wszSKIES;
			list<WSTRING> skies;
			if (CFiles::GetGameProfileString("Graphics", style.c_str(), skies) && !skies.empty())
			{
				//Sky images specified.  Choose the one closest to specified light level.
				UINT wSkyIndex = skies.size() > this->wDark ? this->wDark : skies.size()-1;
				list<WSTRING>::const_iterator sky = skies.begin();
				while (wSkyIndex--)
					++sky;
				wstrSkyImageName = *sky;
			}
		}

		if (wstrSkyImageName.empty())
		{
			//No sky image listed.
			if (this->pSkyImage)
			{
				SDL_FreeSurface(this->pSkyImage);
				this->pSkyImage = NULL;
			}
			this->wstrSkyImage.resize(0);
		} else {
			//If image name is different than the one already loaded, load new sky.
			if (wstrSkyImageName.compare(this->wstrSkyImage))
			{
				if (this->pSkyImage)
					SDL_FreeSurface(this->pSkyImage);
				this->pSkyImage = g_pTheDBM->LoadImageSurface(wstrSkyImageName.c_str(), 0);
				if (!this->pSkyImage)
					this->wstrSkyImage = wszEmpty;
				else
				{
					ASSERT((UINT)this->pSkyImage->w == this->w);
					ASSERT((UINT)this->pSkyImage->h == this->h);
					this->wstrSkyImage = wstrSkyImageName;
				}
			}
		}
	}
}

//*****************************************************************************
bool CRoomWidget::PlayerLightTurnedOff() const
//Returns: true if player was showing a light the last time rendered, but not this turn
{
	return !this->lightedPlayerTiles.empty() && !IsPlayerLightShowing();
}

//*****************************************************************************
void CRoomWidget::PutTLayerEffectsOnMLayer()
//During death sequence, only last (top) layer effects show up.
//This places all effects in the t-layer on the last layer.
{
	this->pMLayerEffects->Effects.insert(this->pMLayerEffects->Effects.end(), this->pTLayerEffects->Effects.begin(), this->pTLayerEffects->Effects.end());

   this->pTLayerEffects->Effects.clear();
}

//*****************************************************************************
void CRoomWidget::SetMoveCountText()
{
	if (!this->bShowMoveCount || !this->pCurrentGame)
		return;
	CEffect *pEffect = this->pLastLayerEffects->GetEffectOfType(ETEXT);
	if (!pEffect)
		return;

	//Don't set the text if there is a different text effect onscreen now.
	CTextEffect *pMoveCount = DYN_CAST(CTextEffect*, CEffect*, pEffect);
	if (pMoveCount->X() == 0 && pMoveCount->Y() == 0)
	{
		WCHAR moveNum[10];
		_itoW(this->pCurrentGame->wPlayerTurn, moveNum, 10);
		pMoveCount->SetText(moveNum, FONTLIB::F_FrameRate);
	}
}

//*****************************************************************************
void CRoomWidget::ShowFrameRate(const bool bVal)
//Shows/hides frame rate as a widget drawn on top everything.
{
	ASSERT(this->bShowFrameRate != bVal);
	this->bShowFrameRate = bVal;
	if (bVal)
	{
		AddLastLayerEffect(new CFrameRateEffect(this));

		//Don't have overlapping display info.
		if (this->bShowMoveCount)
			ShowMoveCount(false);
	} else {
		this->pLastLayerEffects->RemoveEffectsOfType(EFRAMERATE);
	}
}

//*****************************************************************************
void CRoomWidget::ShowMoveCount(const bool bVal)
//Shows/hides move count as a widget drawn on top everything.
{
	ASSERT(this->bShowMoveCount != bVal);
	this->bShowMoveCount = bVal;
	if (bVal)
	{
		CTextEffect *pMoveCount = new CTextEffect(this, wszEmpty, FONTLIB::F_FrameRate);
		pMoveCount->RequestRetainOnClear(true);
		pMoveCount->Move(0,0);
		AddLastLayerEffect(pMoveCount);
		SetMoveCountText();

		//Don't have overlapping display info.
		if (this->bShowFrameRate)
			ShowFrameRate(false);
	} else {
		this->pLastLayerEffects->RemoveEffectsOfType(ETEXT);
	}
}

//*****************************************************************************
void CRoomWidget::ShowVarUpdates(const bool bVal)
//Shows when hold vars change state.
{
	if (this->bShowVarUpdates == bVal)
		return;
	this->bShowVarUpdates = bVal;

	if (bVal)
	{
		CVarMonitorEffect *pEffect = new CVarMonitorEffect(this);
		pEffect->SetText(wszAsterisk);
		AddLastLayerEffect(pEffect);
	} else {
		this->pLastLayerEffects->RemoveEffectsOfType(EVARMONITOR);
	}
}

//*****************************************************************************
void CRoomWidget::ToggleFrameRate()
//Shows/hides frame rate.
{
	ShowFrameRate(!this->bShowFrameRate);
}

//*****************************************************************************
void CRoomWidget::ToggleMoveCount()
//Shows/hides current move count.
{
	ShowMoveCount(!this->bShowMoveCount);
}

//*****************************************************************************
void CRoomWidget::ToggleVarDisplay()
//Shows/hides frame rate.
{
	ShowVarUpdates(!this->bShowVarUpdates);
}

//*****************************************************************************
void CRoomWidget::FadeToLightLevel(
//Crossfades room image from old to new light level.
//
//Params:
	const UINT wNewLight)
{
	StopSleeping();

	if (wNewLight == this->wDark)
		return; //no fade needed

	const UINT wLightDelta = abs(int(this->wDark) - int(wNewLight));
	this->wDark = wNewLight;
	g_pTheDBM->fLightLevel = fRoomLightLevel[this->wDark];

	SDL_Surface *pOldRoomSurface = NULL, *pNewRoomSurface = NULL;

	//Query fade duration.
	string str;
	Uint32 dwFadeDuration = 500/2; //default
	if (CFiles::GetGameProfileString("Customizing", "RoomTransitionSpeed", str))
		dwFadeDuration = atoi(str.c_str()) / 2;
	dwFadeDuration *= wLightDelta;

	if (!IsLightingRendered())
		dwFadeDuration = 0; //don't fade at all if light level is always full

	//Prepare and perform crossfade if duration is non-zero.
	if (dwFadeDuration)
	{
		//Image of room at previous light level.
		pOldRoomSurface = g_pTheBM->ConvertSurface(SDL_CreateRGBSurface(
				SDL_SWSURFACE, this->w, this->h, g_pTheBM->BITS_PER_PIXEL, 0, 0, 0, 0));
		//Image of room at new light level.
		pNewRoomSurface = g_pTheBM->ConvertSurface(SDL_CreateRGBSurface(
				SDL_SWSURFACE, this->w, this->h, g_pTheBM->BITS_PER_PIXEL, 0, 0, 0, 0));
	}

	//If for some reason a surface couldn't be allocated,
	//the new room will be shown without any transition effect.
	if (!pOldRoomSurface || !pNewRoomSurface)
	{
		if (pOldRoomSurface)
			SDL_FreeSurface(pOldRoomSurface);
		if (pNewRoomSurface)
			SDL_FreeSurface(pNewRoomSurface);

		//Just show at new light level.
		ClearEffects();
		UpdateFromCurrentGame();
		Paint();
	} else {
		//Take snapshot of room.
		SDL_Rect rect = {this->x, this->y, this->w, this->h};
		SDL_Rect tempRect = {0, 0, this->w, this->h};
		SDL_BlitSurface(GetDestSurface(), &rect, pOldRoomSurface, &tempRect);

		//Prepare and render new image of room (update this->pRoomSnapshotSurface).
		LoadRoomImages();
		AddPlayerLight(IsPlayerLightRendered());
		DirtyRoom();
		UpdateDrawSquareInfo();
		RenderRoomInPlay(this->wShowCol, this->wShowRow);
		SDL_BlitSurface(this->pRoomSnapshotSurface, &rect, pNewRoomSurface, &tempRect);

		//Render room objects.
		this->x = this->y = 0;
		ASSERT(this->pRoom);
		RenderFogInPit(pNewRoomSurface);
		DrawPlatforms(pNewRoomSurface);
		DrawPlayer(this->pCurrentGame->swordsman, pNewRoomSurface);
		DrawMonsters(this->pRoom->pFirstMonster, pNewRoomSurface, false);
		RenderEnvironment(pNewRoomSurface);
		this->x = rect.x;
		this->y = rect.y;

		//Crossfade.
		{
			CFade fade(pOldRoomSurface, pNewRoomSurface);
			SDL_Surface *pSrcSurface = fade.GetDestSurface();
			SDL_Surface *pDestSurface = GetDestSurface();
			Uint32 dwNow = SDL_GetTicks();
			const UINT dwStart = dwNow;
			const Uint32 dwEnd = dwStart + dwFadeDuration;
			while (dwNow < dwEnd)
			{
				dwNow = SDL_GetTicks();
				fade.IncrementFade((dwNow - dwStart) / (float)dwFadeDuration);
				SDL_BlitSurface(pSrcSurface, &tempRect, pDestSurface, &rect);
				UpdateRect();
				g_pTheBM->UpdateRects(pDestSurface);
			}
		}

		//Done.
		SDL_FreeSurface(pOldRoomSurface);
		SDL_FreeSurface(pNewRoomSurface);
	}
}

//*****************************************************************************
void CRoomWidget::ShowRoomTransition(
//Changes view from old room to current one.
//
//Params:
	const UINT wExitOrientation)  //(in) direction of exit
{
	StopSleeping();

	//Load new floor image mosaic, if any.
	LoadRoomImages();

	string str;
	Uint32 dwPanDuration = 500; //default
	if (CFiles::GetGameProfileString("Customizing", "RoomTransitionSpeed", str))
		dwPanDuration = atoi(str.c_str());

	//Make panning speed consistent along either axis.
	if (wExitOrientation == W || wExitOrientation == E)
		dwPanDuration = static_cast<UINT>(dwPanDuration * (this->pRoom->wRoomCols / float(this->pRoom->wRoomRows)));

	if (IsValidOrientation(wExitOrientation) && wExitOrientation != NO_ORIENTATION &&
			dwPanDuration) //skip effect if value is zero
	{
		//Show a smooth transition between rooms.
		PanDirection panDirection = PanNorth;
		switch (wExitOrientation)
		{
			case N: panDirection = PanNorth; break;
			case S: panDirection = PanSouth; break;
			case E: panDirection = PanEast;  break;
			case W: panDirection = PanWest;  break;
			default: ASSERT(!"Bad pan direction.");  break;
		}

		//Image of room being left.
		SDL_Surface *pOldRoomSurface = g_pTheBM->ConvertSurface(SDL_CreateRGBSurface(
				SDL_SWSURFACE, this->w, this->h, g_pTheBM->BITS_PER_PIXEL, 0, 0, 0, 0));
		//Image of room being entered.
		SDL_Surface *pNewRoomSurface = g_pTheBM->ConvertSurface(SDL_CreateRGBSurface(
				SDL_SWSURFACE, this->w, this->h, g_pTheBM->BITS_PER_PIXEL, 0, 0, 0, 0));
		SDL_Rect rect = {this->x, this->y, this->w, this->h};
		SDL_Rect tempRect = {0, 0, this->w, this->h};

		//If for some reason a surface couldn't be allocated,
		//the new room will be shown without any transition effect.
		if (!pOldRoomSurface || !pNewRoomSurface)
		{
			if (pOldRoomSurface)
				SDL_FreeSurface(pOldRoomSurface);
			if (pNewRoomSurface)
				SDL_FreeSurface(pNewRoomSurface);
		} else {
			//Take snapshot of room being left.
			SDL_BlitSurface(GetDestSurface(), &rect, pOldRoomSurface, &tempRect);

			//Render image of new room (update this->pRoomSnapshotSurface).
			ClearEffects();
			UpdateFromCurrentGame();
			ASSERT(this->pRoomSnapshotSurface);
			SDL_BlitSurface(this->pRoomSnapshotSurface, &rect,
						pNewRoomSurface, &tempRect);
			this->x = this->y = 0;
			ASSERT(this->pRoom);
			RenderFogInPit(pNewRoomSurface);
			DrawPlatforms(pNewRoomSurface);
			DrawMonsters(this->pRoom->pFirstMonster, pNewRoomSurface, false);
			RenderEnvironment(pNewRoomSurface);
			this->x = rect.x;
			this->y = rect.y;

			//Slide from the old to new room.
			CPan pan(pOldRoomSurface, pNewRoomSurface, panDirection);
			const UINT dwStart = SDL_GetTicks();
			SDL_Surface *pDestSurface = GetDestSurface();
			while (true)
			{
				const Uint32 dwNow = SDL_GetTicks();
				pan.IncrementPan((dwNow - dwStart) / (float)dwPanDuration);
				SDL_BlitSurface(pOldRoomSurface, &tempRect,
						pDestSurface, &rect);
				UpdateRect();
				g_pTheBM->UpdateRects(pDestSurface);
				if (dwNow - dwStart > dwPanDuration)
				{
					//Done panning.
					SDL_FreeSurface(pOldRoomSurface);
					SDL_FreeSurface(pNewRoomSurface);
					return;
				}
			}
		}
	}

	//Just show new room.
	ClearEffects();
	UpdateFromCurrentGame();
	Paint();
}

//*****************************************************************************
void CRoomWidget::StopSleeping()
//Stops all effects related to the player sleeping during idle time.
{
	this->bPlayerSleeping = false;
	RemoveMLayerEffectsOfType(EFLOAT);
	g_pTheSound->StopSoundEffect(SEID_SNORING);
}

//*****************************************************************************
bool CRoomWidget::SubtitlesHas(CSubtitleEffect *pEffect) const
//Returns: whether the specified effect is in the set of active subtitles
{
	for (SUBTITLES::const_iterator iter = this->subtitles.begin();
			iter != this->subtitles.end(); ++iter)
	{
		if (pEffect == iter->second)
			return true;
	}
	return false;
}

//*****************************************************************************
void CRoomWidget::UpdateFromCurrentGame(
//Update the room widget so that it is ready to display the room from
//the current game.
	const bool bForceReload) //whether to force reloading the graphics style
{
	//If the room changed, then get the new one.
	ASSERT(this->pCurrentGame);
	SynchRoomToCurrentGame();

	//Room widget can only display standard-sized rooms now.
	ASSERT(this->pRoom->wRoomCols == CDrodBitmapManager::DISPLAY_COLS);
	ASSERT(this->pRoom->wRoomRows == CDrodBitmapManager::DISPLAY_ROWS);

	this->style = this->pRoom->style;
	VERIFY(g_pTheDBM->LoadTileImagesForStyle(this->style, bForceReload));
	if (bForceReload)
		LoadRoomImages(); //ensure these are current with reloaded style

	//Prepare new room.
	AddPlayerLight();
	RenderRoomLighting();
	DirtyRoom();
	if (this->dwRoomX != this->pRoom->dwRoomX ||
		this->dwRoomY != this->pRoom->dwRoomY)
	{
		this->dwRoomX = this->pRoom->dwRoomX;
		this->dwRoomY = this->pRoom->dwRoomY;
	}
	VERIFY(UpdateDrawSquareInfo());

	//Redraw the room.
	RenderRoomInPlay(this->wShowCol, this->wShowRow);
}

//*****************************************************************************
void CRoomWidget::UpdateFromPlots(const CCoordSet *pSet, const CCoordSet *pGeometryChanges)
//Refresh the tile image arrays after plots have been made.
{
	//Next call will recalc the indicated tile images and their neighbors.
	UpdateDrawSquareInfo(pSet, pGeometryChanges);
}

//*****************************************************************************
void CRoomWidget::ResetForPaint()
//Reset the object so that the next paint will draw everything fresh with no
//assumptions about what is already drawn in the widget area.
{
	this->bAllDirty = true;
	this->wLastTurn = UINT(-1);

	SynchRoomToCurrentGame();

	UpdateDrawSquareInfo();
}

//*****************************************************************************
void CRoomWidget::BlitDirtyRoomTiles(const bool bMoveMade)
//Redraw all tiles in room that need refreshing.
{
	const UINT wStartPos = this->wShowRow * this->pRoom->wRoomCols + this->wShowCol;
	const UINT wRowOffset = this->pRoom->wRoomCols - CDrodBitmapManager::DISPLAY_COLS;
	const UINT wXEnd = this->wShowCol + CDrodBitmapManager::DISPLAY_COLS;
	const UINT wYEnd = this->wShowRow + CDrodBitmapManager::DISPLAY_ROWS;

	SDL_Surface *pDestSurface = GetDestSurface();
	SDL_Rect src = {0, 0, CX_TILE, CY_TILE};
	SDL_Rect dest = {0, 0, CX_TILE, CY_TILE};
	TILEINFO *pbMI = this->pTileInfo + wStartPos, *pbRMI;
	UINT wX, wY;
	for (wY = this->wShowRow; wY < wYEnd; ++wY)
	{
		for (wX = this->wShowCol; wX < wXEnd; ++wX)
		{
			if (pbMI->dirty || (bMoveMade && pbMI->monster))
			{
				src.x = dest.x = this->x + (wX - this->wShowCol) * CX_TILE;
				src.y = dest.y = this->y + (wY - this->wShowRow) * CY_TILE;
				SDL_BlitSurface(this->pRoomSnapshotSurface, &src, pDestSurface, &dest);
				pbMI->damaged = 1;   //tile must be updated on screen
				if (bMoveMade && pbMI->monster && wY > this->wShowRow)
				{
					//Monster was possibly raised.  Redraw the tile above it also.
					src.y -= CY_TILE;
					dest.y -= CY_TILE;
					SDL_BlitSurface(this->pRoomSnapshotSurface, &src, pDestSurface, &dest);
					pbRMI = pbMI - this->pRoom->wRoomCols;
					pbRMI->damaged = 1;   //tile must be updated on screen
					pbRMI->dirty = pbRMI->monster = 0;     //tile is no longer dirty
				}
				pbMI->dirty = pbMI->monster = 0;     //tile is no longer dirty
			}
			++pbMI;
		}
		pbMI += wRowOffset;
	}
}

//*****************************************************************************
void CRoomWidget::DrawTileEdges(
//Draws black edges around a tile if needed.
//
//Params:
	const UINT wX, const UINT wY, //(in) Tile coords.
	const EDGES *pbE,             //(in) Edge data for tile.
	SDL_Surface *pDestSurface)    //(in) Where to draw edges.
{
	static const SURFACECOLOR EdgeColor[EDGE_COUNT] = {
		{(Uint8)-1, (Uint8)-1, (Uint8)-1}, {96, 96, 96}, {0, 0, 0}};
	static const UINT CX_TILE = CBitmapManager::CX_TILE;
	static const UINT CY_TILE = CBitmapManager::CY_TILE;
	static const UINT DISPLAY_COLS = CDrodBitmapManager::DISPLAY_COLS;
	static const UINT DISPLAY_ROWS = CDrodBitmapManager::DISPLAY_ROWS;

	const int nX = this->x + wX * CX_TILE, nY = this->y + wY * CY_TILE;

	if (pbE->north)
		DrawRow(nX, nY, CX_TILE, EdgeColor[pbE->north], pDestSurface);
	//join some cases at the corner so it looks better
	else if (wY)
	{
		if (wX)
		{
			if ((pbE - DISPLAY_COLS)->west && (pbE - 1)->north &&
					!(pbE - DISPLAY_COLS)->south && !(pbE - 1)->east) //don't overlap two corners
				DrawPixel(nX, nY, EdgeColor[(pbE-1)->north], pDestSurface);
		}
		if (wX < DISPLAY_COLS-1)
		{
			if ((pbE - DISPLAY_COLS)->east && (pbE + 1)->north &&
					!(pbE - DISPLAY_COLS)->south && !(pbE + 1)->west)
				DrawPixel(nX + CX_TILE-1, nY, EdgeColor[(pbE+1)->north], pDestSurface);
		}
	}

	if (pbE->south)
		DrawRow(nX, nY + CY_TILE-1, CX_TILE, EdgeColor[pbE->south], pDestSurface);
	//join some cases at the corner so it looks better
	else if (wY < DISPLAY_ROWS-1)
	{
		if (wX)
		{
			if ((pbE + DISPLAY_COLS)->west && (pbE - 1)->south &&
					!(pbE + DISPLAY_COLS)->north && !(pbE - 1)->east)
				DrawPixel(nX, nY + CY_TILE-1, EdgeColor[(pbE-1)->south], pDestSurface);
		}
		if (wX < DISPLAY_COLS-1)
		{
			if ((pbE + DISPLAY_COLS)->east && (pbE + 1)->south &&
					!(pbE + DISPLAY_COLS)->north && !(pbE + 1)->west)
				DrawPixel(nX + CX_TILE-1, nY + CY_TILE-1,
						EdgeColor[(pbE+1)->south], pDestSurface);
		}
	}

	if (pbE->west)
		DrawCol(nX, nY, CY_TILE, EdgeColor[pbE->west], pDestSurface);

	if (pbE->east)
		DrawCol(nX + CX_TILE-1, nY, CY_TILE, EdgeColor[pbE->east], pDestSurface);

	//Special edging around the inner wall texture.
	static const int halfWallY = CY_TILE/2;
	if (pbE->bHalfWall)
	{
		//South-facing
		DrawRow(nX, nY + halfWallY, CX_TILE, EdgeColor[2], pDestSurface);
	} else {
		//East- or west-facing
		const bool bNotInnerWall = GetWallTypeAtSquare(this->pRoom, wX, wY) != WALL_INNER;
		if (wX > 0 && (pbE-1)->bHalfWall && bNotInnerWall)
			DrawCol(nX, nY + halfWallY, CY_TILE - halfWallY, EdgeColor[2], pDestSurface);
		else if (wX+1 < this->pRoom->wRoomCols && (pbE+1)->bHalfWall && bNotInnerWall)
			DrawCol(nX + CX_TILE-1, nY + halfWallY, CY_TILE - halfWallY, EdgeColor[2], pDestSurface);
	}
}

//*****************************************************************************
void CRoomWidget::RenderRoomInPlay(
//Repaints the "physical room" (the o- and t-layers) within the given rectangle.
//Uses "smart" room pre-rendering, only painting tiles that have changed since
//last call.
//
//Params:
	const int wCol, const int wRow,     //(in) top-left tile coords
	const int wWidth, const int wHeight)
{
	ASSERT(this->pCurrentGame);
	const CSwordsman& player = this->pCurrentGame->swordsman;

	//Check for state changes that will affect entire room.
	const bool bIsPlacingDouble = player.wPlacingDoubleType != 0;
	const bool bIsInvisible = !player.IsVisible();
	const bool bPlayerIsDying = this->pCurrentGame->IsPlayerDying();
	const bool bStateChanged = (this->bWasPlacingDouble != bIsPlacingDouble) ||
		(this->bWasInvisible && !bIsInvisible) || bPlayerIsDying;
	if (bStateChanged)
		this->bAllDirty = true;

	ASSERT(this->pRoomSnapshotSurface);
	SDL_Surface *pDestSurface = this->pRoomSnapshotSurface;  //draw to here
	ASSERT(!SDL_MUSTLOCK(pDestSurface));   //Don't need to lock this surface.

	//1. Perform final light calculations.
	if (this->bRenderPlayerLight) //render player light when flagged
	{
		//Copy room lighting to display buffer.
		memcpy(this->psDisplayedLight, this->psRoomLight,
				this->pRoom->CalcRoomArea() * wLightBytesPerTile);

		//Reset entity tile lighting from last render.
		CCoordSet::const_iterator tile;
		for (tile = this->lightedPlayerTiles.begin();
				tile != this->lightedPlayerTiles.end(); ++tile)
		{
			const UINT wStartIndex = this->pRoom->ARRAYINDEX(tile->wX,tile->wY) * wLightValuesPerTile;
			memset(this->psPlayerLight + wStartIndex, 0, wLightBytesPerTile);
			this->pTileInfo[this->pRoom->ARRAYINDEX(tile->wX,tile->wY)].dirty = 1;
		}
		this->lightedPlayerTiles.clear();

		//Render light map for this frame.
		const bool bLightIsOff = PlayerLightTurnedOff();
		if (bLightIsOff)
		{
			this->wLastPlayerLightX = this->wLastPlayerLightY = UINT(-1);
		} else {
			CEntity *pEntity = GetLightholder();
			if (pEntity)
			{
				//Properties of player light.
				static const UINT wPlayerLightRadius = 4;
				static const UINT wLightParam = (wPlayerLightRadius-1)*NUM_LIGHT_TYPES+0; //white light

				this->pActiveLightedTiles = &this->lightedPlayerTiles;
				this->pActiveLight = this->psPlayerLight;
				PropagateLight(pEntity->wX, pEntity->wY, wLightParam);
				this->wLastPlayerLightX = pEntity->wX;
				this->wLastPlayerLightY = pEntity->wY;
			}

			//Could propagate light from other entities at this point if desired,
			//adding their light to this buffer.

			//Propagate light from user cursor to see room better.
			if (this->pRoom->IsValidColRow(this->cursorLight.wX,this->cursorLight.wY))
			{
				static const UINT wCursorLightRadius = 5;
				static const UINT wLightParam = (wCursorLightRadius-1)*NUM_LIGHT_TYPES+0; //white light
				PropagateLight(this->cursorLight.wX, this->cursorLight.wY, wLightParam);
			}
		}

		//Add the entity light buffer to the display buffer.
		const UINT wLastCol = this->pRoom->wRoomCols-1;
		const UINT wLastRow = this->pRoom->wRoomRows-1;
		for (tile = this->lightedPlayerTiles.begin();
				tile != this->lightedPlayerTiles.end(); ++tile)
		{
			const UINT wX = tile->wX, wY = tile->wY;
			const UINT wStartIndex = this->pRoom->ARRAYINDEX(wX,wY) * wLightValuesPerTile;
			LIGHTTYPE *pSrc, *pDest = this->psDisplayedLight + wStartIndex;

			//Blur this light if it would smooth some shadow edges.
			if (this->partialLightedTiles.has(wX,wY))
			{
				//Whether there is light at the edges of the adjacent tiles to factor in.
				const bool bUp = wY > 0 && this->tileLightInfo.GetAt(wX,wY-1) != L_Dark;
				const bool bDown = wY < wLastRow && this->tileLightInfo.GetAt(wX,wY+1) != L_Dark;
				const bool bLeft = wX > 0 && this->tileLightInfo.GetAt(wX-1,wY) != L_Dark;
				const bool bRight = wX < wLastCol && this->tileLightInfo.GetAt(wX+1,wY) != L_Dark;

				LowPassLightFilter(this->psPlayerLight + wStartIndex, this->psTempLightBuffer + wStartIndex,
						bLeft, bRight, bUp, bDown);

				//Add the blurred light.
				pSrc = this->psTempLightBuffer + wStartIndex;
			} else {
				//Add light unchanged from the original buffer.
				pSrc = this->psPlayerLight + wStartIndex;
			}

			const LIGHTTYPE *pSrcTileEnd = pSrc + wLightValuesPerTile;
			while (pSrc != pSrcTileEnd)
			{
				//RGB
				UINT val = *pDest + *(pSrc++);
				*(pDest++) = val < LIGHTTYPE(-1) ? val : LIGHTTYPE(-1); //avoid overflow
				val = *pDest + *(pSrc++);
				*(pDest++) = val < LIGHTTYPE(-1) ? val : LIGHTTYPE(-1);
				val = *pDest + *(pSrc++);
				*(pDest++) = val < LIGHTTYPE(-1) ? val : LIGHTTYPE(-1);
			}
		}

		//Done rendering player lighting.
		this->bRenderPlayerLight = false;
	}

	//2. Draw o- and t-layers.
	RenderRoom(wCol, wRow, wWidth, wHeight, false);

	//3. When action is frozen, draw the following here.
	if (((!this->bWasPlacingDouble || this->bAllDirty) && bIsPlacingDouble)
			|| bPlayerIsDying)
	{
		//a. Effects that go on top of room image, under monsters/swordsman.
		RenderFogInPit(pDestSurface);
		DrawPlatforms(pDestSurface);

		this->pOLayerEffects->DrawEffects(false, true, bPlayerIsDying ? NULL : pDestSurface);   //freeze effects
		this->pTLayerEffects->DrawEffects(false, true, bPlayerIsDying ? NULL : pDestSurface);   //freeze effects

		//b. Draw monsters (not killing player).
		DrawMonsters(this->pRoom->pFirstMonster, pDestSurface,
				bIsPlacingDouble || bPlayerIsDying);

		//c. Effects that go on top of monsters/swordsman.
		this->pMLayerEffects->DrawEffects(false, true, bPlayerIsDying ? NULL : pDestSurface);   //freeze effects

		//d. Double placement effects:
		//Make room black-and-white, and draw (unmoving) swordsman on top.
		if (bIsPlacingDouble && !bPlayerIsDying)
		{
			if (player.wPlacingDoubleType == M_DECOY)
			{
				CCoordIndex coords(this->pRoom->wRoomCols,this->pRoom->wRoomRows);
				for (list<CPlayerDouble*>::const_iterator decoy=this->pRoom->Decoys.begin();
						decoy!=this->pRoom->Decoys.end(); ++decoy)
					DrawInvisibilityRange((*decoy)->wX, (*decoy)->wY, pDestSurface, &coords);
			}

			BAndWRect(pDestSurface, this->wShowCol, this->wShowRow);

			ASSERT(this->bShowingPlayer);
			DrawPlayer(player, pDestSurface);
		}
	}

	this->bWasPlacingDouble = bIsPlacingDouble;
	this->bWasInvisible = bIsInvisible;
}

//*****************************************************************************
void CRoomWidget::AddLightInterp(
//Add light color+intensity to this room tile.
//
//Params:
	SDL_Surface *pDestSurface,		//(in) dest surface
	const UINT wX, const UINT wY,	//(in) room position (in tiles)
	LIGHTTYPE* sRGBIntensity,	//(in) what % of light to add
	const float fDark,      //(in) darkness multiplier also being added to tile
	const UINT wTileMask,	//(in) optional tile mask [default = TI_UNSPECIFIED]
	const Uint8 opacity,    //adding light to a transparent object? [default=255]
	const UINT yRaised)     //vertical offset [default=0]
const
{
	if (!(this->pRoom->tileLights.Exists(wX,wY) ||
			this->lightedRoomTiles.has(wX,wY) || this->lightedPlayerTiles.has(wX,wY)))
	{
		//This tile wasn't lighted by anything -- only add darkness.
		if (fDark < 1.0)
		{
			if (wTileMask == TI_UNSPECIFIED)
				g_pTheDBM->DarkenTile(this->x + wX*CX_TILE, this->y + wY*CY_TILE, fDark, pDestSurface);
			else
				g_pTheDBM->DarkenTileWithMask(wTileMask, 0, 0,
					this->x + wX*CX_TILE, this->y + wY*CY_TILE, CX_TILE, CY_TILE, pDestSurface, fDark);
		}
		return;
	}

	UINT wMax = 3 * (UINT(fMaxLightIntensity[this->wDark] * opacity) + 256); //optimization

	const UINT wXPos = this->x + wX*CX_TILE, wYPos = this->y + wY*CY_TILE + yRaised;
	UINT wR[4], wG[4], wB[4];
	UINT wXOffset, wYOffset, wSum;
	UINT i,j,k;	//sub-tile lighting
	for (j=0; j<LIGHT_SPT_MINUSONE; ++j)
	{
		for (i=0; i<LIGHT_SPT_MINUSONE; ++i)
		{
			static const UINT wRowBytes = LIGHT_SPT*LIGHT_BPP;
			wSum = wR[0] = sRGBIntensity[0];
			wSum += wG[0] = sRGBIntensity[1];
			wSum += wB[0] = sRGBIntensity[2];
			wSum += wR[1] = sRGBIntensity[3];
			wSum += wG[1] = sRGBIntensity[4];
			wSum += wB[1] = sRGBIntensity[5];
			wSum += wR[2] = sRGBIntensity[0+wRowBytes];
			wSum += wG[2] = sRGBIntensity[1+wRowBytes];
			wSum += wB[2] = sRGBIntensity[2+wRowBytes];
			wSum += wR[3] = sRGBIntensity[3+wRowBytes];
			wSum += wG[3] = sRGBIntensity[4+wRowBytes];
			wSum += wB[3] = sRGBIntensity[5+wRowBytes];
			if (!wSum)
			{
				//There is no light to add here.
				//If darkness is set, add that.
				if (fDark < 1.0)
				{
					if (wTileMask == TI_UNSPECIFIED)
						g_pTheDBM->DarkenRect(wXPos + wLightCellPos[i], wYPos + wLightCellPos[j],
								wLightCellSize[i], wLightCellSize[j], fDark, pDestSurface);
					else
						g_pTheDBM->DarkenTileWithMask(wTileMask, wLightCellPos[i], wLightCellPos[j],
								wXPos + wLightCellPos[i], wYPos + wLightCellPos[j],
								wLightCellSize[i], wLightCellSize[j], pDestSurface, fDark);
				}

				sRGBIntensity += LIGHT_BPP;
				continue;
			}
			for (k=4; k--; )
			{
				//add 100% to the amount of multiplicative light so the math works correctly in LightenRect
				wR[k] += 256;
				wG[k] += 256;
				wB[k] += 256;

				//Reduce light by darkness factor.
				wR[k] = static_cast<UINT>(wR[k] * fDark);
				wG[k] = static_cast<UINT>(wG[k] * fDark);
				wB[k] = static_cast<UINT>(wB[k] * fDark);

				//Cap values at maximum light addition.  Don't alter hue or saturation.
				wSum = wR[k] + wB[k] + wG[k];
				if (wSum > wMax)
				{
					const float fNormalize = wMax / float(wSum);
					wR[k] = static_cast<UINT>(wR[k] * fNormalize);
					wG[k] = static_cast<UINT>(wG[k] * fNormalize);
					wB[k] = static_cast<UINT>(wB[k] * fNormalize);
				}
			}

			wXOffset = wLightCellPos[i]; //optimization
			wYOffset = wLightCellPos[j];
			g_pTheBM->LightenRect(pDestSurface, wXPos + wXOffset, wYPos + wYOffset,
					wLightCellSize[i], wLightCellSize[j],
					wR, wG, wB,
					wTileMask, wXOffset, wYOffset);

			sRGBIntensity += LIGHT_BPP; //get next sub-tile RGB value
		}
		sRGBIntensity += LIGHT_BPP; //finish row
	}
}

//*****************************************************************************
void CRoomWidget::AddLightOffset(
//Add light color+intensity to a given moving tile with a sprite mask set.
//Calculates dark and light based on the possible 4 tiles the moving tile
//may be straddling.
//
//Use this to add lighting onto a moving sprite.
//Darkness value is mixed together with the light for higher end resolution.
//
//Params:
	SDL_Surface *pDestSurface,		//(in) dest surface
	const UINT wX, const UINT wY,	//(in) room position (in tiles)
	const UINT wXOffset, const UINT wYOffset, //(in) pixel offset
	const UINT wTileMask,	//(in) tile mask
	const Uint8 opacity,    //adding light to a transparent object? [default=255]
	const UINT yRaised)     //vertical offset (not used to determine tile lighting) [default=0]
const
{
	//Must have an offset, otherwise we would've been better off using AddLight
	ASSERT(wXOffset || wYOffset);

	const UINT wXPos = this->x + wX*CX_TILE + wXOffset;
	const UINT wYPos = this->y + wY*CY_TILE + wYOffset + yRaised;

	UINT wOX    = (wX*CX_TILE + wXOffset) / CX_TILE;
	UINT wOY    = (wY*CY_TILE + wYOffset) / CY_TILE;
	UINT wOXOff = (wX*CX_TILE + wXOffset) % CX_TILE;
	UINT wOYOff = (wY*CY_TILE + wYOffset) % CY_TILE;

	const float fMax = 3 * (1.0f + (fMaxLightIntensity[this->wDark] * opacity / 255)); //optimization
	UINT wNX, wNY, wNXPos, wNYPos, w, h;
	const UINT wOffsetWidth  = CX_TILE - wOXOff;
	const UINT wOffsetHeight = CY_TILE - wOYOff;
	for (int nQuadrant = 0; nQuadrant < 4; ++nQuadrant)
	{
		switch(nQuadrant)
		{
			case 0:  //NW Quadrant
				wNX = wOX;     wNXPos = 0;             w = wOffsetWidth;
				wNY = wOY;     wNYPos = 0;             h = wOffsetHeight;
				break;
			case 1:  //NE Quadrant
				wNX = wOX + 1; wNXPos = wOffsetWidth;  w = wOXOff;
				wNY = wOY;     wNYPos = 0;             h = wOffsetHeight;
				break;
			case 2:  //SW Quadrant
				wNX = wOX;     wNXPos = 0;             w = wOffsetWidth;
				wNY = wOY + 1; wNYPos = wOffsetHeight; h = wOYOff;
				break;
			case 3:  //SE Quadrant
				wNX = wOX + 1; wNXPos = wOffsetWidth;  w = wOXOff;
				wNY = wOY + 1; wNYPos = wOffsetHeight; h = wOYOff;
				break;
		}
		if (!w || !h) continue;
  		if (!this->pRoom->IsValidColRow(wNX, wNY)) continue;

		//Calculate dark
		const UINT val = this->pRoom->tileLights.GetAt(wNX, wNY);
		float fDark = 1.0;
		if (bIsDarkTileValue(val))
		{
			ASSERT(val - LIGHT_OFF - 1 < NUM_DARK_TYPES);
			fDark = darkMap[(val - LIGHT_OFF - 1) *3/4]; //add 3/4 darkness
		}

		//Add light to sprite
		if (this->pRoom->tileLights.Exists(wNX,wNY) ||
			 this->lightedRoomTiles.has(wNX,wNY) || this->lightedPlayerTiles.has(wNX,wNY))
		{
//			wXPos/wYPos is where we're drawing the tile.
//          wNX/wNY is what we're using to calculate light
//          wNXPos/wNYPos is the offset of wXPos/wYPos for us to draw.
//          w/h is the width and height of this quadrant
//
//Light resolution.
//#define LIGHT_SPT (8)	//light samples per tile (NxN)
//#define LIGHT_SPT_MINUSONE (LIGHT_SPT-1)	//light samples per tile (NxN)
//#define LIGHT_BPP (3)   //RGB
//const UINT wLightValuesPerTile = LIGHT_SPT*LIGHT_SPT*LIGHT_BPP;	//NxN RGB
//const UINT wLightBytesPerTile = wLightValuesPerTile * sizeof(LIGHTTYPE);
//const UINT wLightCellSize[LIGHT_SPT_MINUSONE] = {4,3,3,3,3,3,3};	 //pixels in each sub-tile light cell
//const UINT wLightCellPos[LIGHT_SPT_MINUSONE] = {0,4,7,10,13,16,19}; //offset of each sub-tile light cell

			float R, G, B;
			UINT wR, wG, wB;
			UINT wLX, wLY, wLW, wLH, wSum;
			UINT i,j,iMin,jMin,iMax,jMax;	//sub-tile lighting
			switch(nQuadrant)
			{
				case 0:  //NW Quadrant
					iMax = LIGHT_SPT_MINUSONE;
					iMin = iMax - 1;
					while(wLightCellPos[iMin] > CX_TILE - w) iMin--;
					jMax = LIGHT_SPT_MINUSONE;
					jMin = jMax - 1;
					while(wLightCellPos[jMin] > CY_TILE - h) jMin--;
					break;
				case 1:  //NE Quadrant
					iMin = 0;
					iMax = iMin + 1;
					while((wLightCellPos[iMax] < w) && (iMax < LIGHT_SPT_MINUSONE)) iMax++;
					jMax = LIGHT_SPT_MINUSONE;
					jMin = jMax - 1;
					while(wLightCellPos[jMin] > CY_TILE - h) jMin--;
					break;
				case 2:  //SW Quadrant
					iMax = LIGHT_SPT_MINUSONE;
					iMin = iMax - 1;
					while(wLightCellPos[iMin] > CX_TILE - w) iMin--;
					jMin = 0;
					jMax = jMin + 1;
					while((wLightCellPos[jMax] < h) && (jMax < LIGHT_SPT_MINUSONE)) jMax++;
					break;
				case 3:  //SE Quadrant
					iMin = 0;
					iMax = iMin + 1;
					while((wLightCellPos[iMax] < w) && (iMax < LIGHT_SPT_MINUSONE)) iMax++;
					jMin = 0;
					jMax = jMin + 1;
					while((wLightCellPos[jMax] < h) && (jMax < LIGHT_SPT_MINUSONE)) jMax++;
					break;
			}
			LIGHTTYPE *sRGBIntensity = this->psDisplayedLight + (wNY*this->pRoom->wRoomCols + wNX)*wLightValuesPerTile
				+ jMin*LIGHT_SPT*LIGHT_BPP;

			for (j = jMin; j<jMax; ++j)
			{
				sRGBIntensity += iMin*LIGHT_BPP; //start row
				for (i = iMin; i<iMax; ++i)
				{
					//Define where and how much of the tile we'll be drawing
					wLX = wLightCellPos[i];
					wLY = wLightCellPos[j];
					wLW = wLightCellSize[i];
					wLH = wLightCellSize[j];
					// Western Quadrants
					if (!(nQuadrant % 2))
					{
						if (i == iMin)
						{
							wLX = 0;
							wLW -= CX_TILE - w - wLightCellPos[i];
						} else {
							wLX -= CX_TILE - w;
						}
					}
					// Eastern Quadrants
					if (nQuadrant % 2)
					{
						if (i+1 == iMax) wLW = w - wLX;
						wLX += wNXPos;
					}
					// Northern Quadrants
					if (nQuadrant < 2)
					{
						if (j == jMin)
						{
							wLY = 0;
							wLH -= CY_TILE - h - wLightCellPos[j];
						} else {
							wLY -= CY_TILE - h;
						}
					}
					// Southern Quadrants
					if (nQuadrant >= 2)
					{
						if (j+1 == jMax) wLH = h - wLY;
						wLY += wNYPos;
					}

					//Average values of four corners for lighting this sub-tile rectangle.
					static const UINT wRowBytes = LIGHT_SPT*LIGHT_BPP;
					wSum = wR = (sRGBIntensity[0] + sRGBIntensity[3] + sRGBIntensity[0+wRowBytes] + sRGBIntensity[3+wRowBytes]) / 4;
					wSum += wG = (sRGBIntensity[1] + sRGBIntensity[4] + sRGBIntensity[1+wRowBytes] + sRGBIntensity[4+wRowBytes]) / 4;
					wSum += wB = (sRGBIntensity[2] + sRGBIntensity[5] + sRGBIntensity[2+wRowBytes] + sRGBIntensity[5+wRowBytes]) / 4;
					if (!wSum)
					{
						//There is no light to add here.
						//If darkness is set, add that.
						if (fDark < 1.0)
						{
							if (wTileMask == TI_UNSPECIFIED)
								g_pTheDBM->DarkenRect(wXPos + wLX, wYPos + wLY,	wLW, wLH, fDark, pDestSurface);
							else
								g_pTheDBM->DarkenTileWithMask(wTileMask, wLX, wLY,
										wXPos + wLX, wYPos + wLY, wLW, wLH, pDestSurface, fDark);
						}

						sRGBIntensity += LIGHT_BPP;
						continue;
					}

					//add 256 (unity) so math for multiplicative light blending works correctly
					R = (wR+256.0f)*fDark/256.0f; //div by 256 instead of 255 for speed optimization
					G = (wG+256.0f)*fDark/256.0f;
					B = (wB+256.0f)*fDark/256.0f;

					//Cap values at maximum light addition.  Don't alter hue or saturation.
					float fSum = R + G + B;
					if (fSum > fMax)
					{
						const float fNormalize = fMax / fSum;
						R *= fNormalize;
						G *= fNormalize;
						B *= fNormalize;
					}

					g_pTheBM->LightenRectWithTileMask(pDestSurface, wXPos + wLX, wYPos + wLY, wLW, wLH,
#if (GAME_BYTEORDER == GAME_BYTEORDER_BIG)
							B, G, R,
#else
							R, G, B,
#endif
							wTileMask, wLX, wLY);

					sRGBIntensity += LIGHT_BPP; //get next sub-tile RGB value
				}
				sRGBIntensity += (LIGHT_SPT-iMax)*LIGHT_BPP; //finish row
			}
		}
	}
}


//*****************************************************************************
void CRoomWidget::AddLight(
//Add light color+intensity to a given screen position (within the room)
//with a sprite mask set.
//
//Use this to add lighting onto the sprite.
//Darkness value is mixed together with the light for higher end resolution.
//
//Params:
	SDL_Surface *pDestSurface,		//(in) dest surface
	const UINT wXPos, const UINT wYPos,	//(in) pixel position
	LIGHTTYPE* sRGBIntensity,	//(in) what % of light to add
	const float fDark,      //(in) darkness multiplier also being added to tile
	const UINT wTileMask,	//(in) tile mask
	const Uint8 opacity)    //adding light to a transparent object? [default=255]
const
{
	//Must be drawn within the room bounds.
	ASSERT((int)wXPos >= this->x);
	ASSERT((int)wYPos >= this->y);
	ASSERT(wXPos <= this->x + this->w - CX_TILE);
	ASSERT(wYPos <= this->y + this->h - CY_TILE);

	const float fMax = 3 * (1.0f + (fMaxLightIntensity[this->wDark] * opacity / 255)); //optimization
	float R, G, B;
	UINT wR, wG, wB;
	UINT wXOffset, wYOffset, wSum;
	UINT i,j;	//sub-tile lighting
	for (j=0; j<LIGHT_SPT_MINUSONE; ++j)
	{
		for (i=0; i<LIGHT_SPT_MINUSONE; ++i)
		{
			//Average values of four corners for lighting this sub-tile rectangle.
			static const UINT wRowBytes = LIGHT_SPT*LIGHT_BPP;
			wSum = wR = (sRGBIntensity[0] + sRGBIntensity[3] + sRGBIntensity[0+wRowBytes] + sRGBIntensity[3+wRowBytes]) / 4;
			wSum += wG = (sRGBIntensity[1] + sRGBIntensity[4] + sRGBIntensity[1+wRowBytes] + sRGBIntensity[4+wRowBytes]) / 4;
			wSum += wB = (sRGBIntensity[2] + sRGBIntensity[5] + sRGBIntensity[2+wRowBytes] + sRGBIntensity[5+wRowBytes]) / 4;
			if (!wSum)
			{
				//There is no light to add here.
				//If darkness is set, add that.
				if (fDark < 1.0)
				{
					if (wTileMask == TI_UNSPECIFIED)
						g_pTheDBM->DarkenRect(wXPos + wLightCellPos[i], wYPos + wLightCellPos[j],
								wLightCellSize[i], wLightCellSize[j], fDark, pDestSurface);
					else
						g_pTheDBM->DarkenTileWithMask(wTileMask, wLightCellPos[i], wLightCellPos[j],
								wXPos + wLightCellPos[i], wYPos + wLightCellPos[j],
								wLightCellSize[i], wLightCellSize[j], pDestSurface, fDark);
				}

				sRGBIntensity += LIGHT_BPP;
				continue;
			}

			//add 256 (unity) so math for multiplicative light blending works correctly
			R = (wR+256.0f)*fDark/256.0f; //div by 256 instead of 255 for speed optimization
			G = (wG+256.0f)*fDark/256.0f;
			B = (wB+256.0f)*fDark/256.0f;

			//Cap values at maximum light addition.  Don't alter hue or saturation.
			float fSum = R + G + B;
			if (fSum > fMax)
			{
				const float fNormalize = fMax / fSum;
				R *= fNormalize;
				G *= fNormalize;
				B *= fNormalize;
			}

			wXOffset = wLightCellPos[i]; //optimization
			wYOffset = wLightCellPos[j];
			g_pTheBM->LightenRectWithTileMask(pDestSurface, wXPos + wXOffset, wYPos + wYOffset,
					wLightCellSize[i], wLightCellSize[j],
#if (GAME_BYTEORDER == GAME_BYTEORDER_BIG)
					B, G, R,
#else
					R, G, B,
#endif
					wTileMask, wXOffset, wYOffset);

			sRGBIntensity += LIGHT_BPP; //get next sub-tile RGB value
		}
		sRGBIntensity += LIGHT_BPP; //finish row
	}
}

//*****************************************************************************
UINT CRoomWidget::GetTextureIndexForTile(const UINT tileNo, const bool bForceBaseImage) const
//Returns: the texture index that corresponds to this room tile
{
	UINT wTextureIndex = FLOOR_ROAD; //Prevent uninitialized
	switch (tileNo)
	{
		case T_FLOOR_ROAD: wTextureIndex = FLOOR_ROAD; break;
		case T_FLOOR_GRASS: wTextureIndex = FLOOR_GRASS; break;
		case T_FLOOR_DIRT: wTextureIndex = FLOOR_DIRT; break;
		case T_FLOOR_ALT: wTextureIndex = FLOOR_ALT; break;
		case T_FLOOR: wTextureIndex = FLOOR_CHECKERED; break;
		case T_FLOOR_M: wTextureIndex = FLOOR_MOSAIC; break;
		case T_PIT:
		case T_PLATFORM_P:
			wTextureIndex = PIT_MOSAIC; break;
		case T_WALL: case T_WALL_H: case T_WALL_B: case T_WALL2:
				wTextureIndex = WALL_MOSAIC; break;
		case T_FLOOR_IMAGE: wTextureIndex =
				!bForceBaseImage && g_pTheDBM->pTextures[FLOOR_IMAGE] ? FLOOR_IMAGE : FLOOR_MOSAIC;
			break;
		case T_PIT_IMAGE: wTextureIndex =
				!bForceBaseImage && g_pTheDBM->pTextures[FLOOR_IMAGE] ? FLOOR_IMAGE : PIT_MOSAIC;
			break;
		case T_WALL_IMAGE: wTextureIndex =
				!bForceBaseImage && g_pTheDBM->pTextures[FLOOR_IMAGE] ? FLOOR_IMAGE : WALL_MOSAIC;
		break;
		default:
			if (!bForceBaseImage)
			{
				wTextureIndex = FLOOR_MOSAIC;
				break;
			}
			ASSERT(!"Invalid tile for texturing");
		break;
	}
	return wTextureIndex;
}

//*****************************************************************************
void CRoomWidget::RenderRoom(
//Render o- and t-layers in room for either game play or the room editor.
//
//Params:
	int wCol, int wRow,     //(in) top-left tile coords [default=(0,0) to (xSize,ySize)]
	int wWidth, int wHeight,
	const bool bEditor)     //[default=true]
{
#define DrawRoomTile(wTileImageNo) g_pTheBM->BlitTileImage(\
		(wTileImageNo), nX, nY, pDestSurface, false, 255)
#define DrawTransparentRoomTile(wTileImageNo,opacity)\
		g_pTheBM->BlitTileImage((wTileImageNo), nX, nY, pDestSurface, false, (opacity))
#define DrawShadow(pwShadowTIs,wNumShadows) g_pTheBM->BlitTileShadows(\
		(pwShadowTIs), (wNumShadows), nX, nY, pDestSurface)
#define AddDark(fLight) g_pTheDBM->DarkenTile(nX, nY, fLight, pDestSurface);
#define AddDarkMask(wTI,fLight) g_pTheDBM->DarkenTileWithMask((wTI), 0, 0,\
		nX, nY, CX_TILE, CY_TILE, pDestSurface, (fLight));
#define IsDeepWaterTile(wTile) ((wTile) == T_WATER || (wTile) == T_TRAPDOOR2 || (wTile) == T_PLATFORM_W)
#define IsShallowTile(wTile) (bIsShallowWater((wTile)) || bIsSteppingStone((wTile)))
#define IsShallowImage(wX,wY) (water[(wX)][(wY)] != T_WATER) &&\
		(IsShallowTile(water[(wX)][(wY)]) ||\
		((bIsPlainFloor(water[(wX)][(wY)]) || water[(wX)][(wY)] == T_GOO) && (\
		IsShallowTile(water[(wX-1)][(wY)]) || IsShallowTile(water[(wX+1)][(wY)]) ||\
		IsShallowTile(water[(wX)][(wY-1)]) || IsShallowTile(water[(wX)][(wY+1)]))))

	BoundsCheckRect(wCol,wRow,wWidth,wHeight);
	const UINT wRowOffset = this->pRoom->wRoomCols - wWidth;
	const UINT wLightRowOffset = wRowOffset * wLightValuesPerTile;
	const UINT wStartPos = wRow * this->pRoom->wRoomCols + wCol;
	const UINT wXEnd = wCol + wWidth;
	const UINT wYEnd = wRow + wHeight;

	//For pit rendering.
	SDL_Surface *pPitsideTexture = g_pTheDBM->pTextures[PITSIDE_MOSAIC];
	if (!pPitsideTexture)
		return;
	const UINT wPitHeight = static_cast<UINT>(pPitsideTexture->h);
	const UINT wPitSideTileWidth = pPitsideTexture->w / CX_TILE;

	SDL_Surface *pDestSurface = this->pRoomSnapshotSurface;  //draw to here

	SDL_Surface *pWaterSurface = NULL;  //For drawing Water/Shallow mixes
	SDL_Surface *pDeepBottom = g_pTheDBM->pTextures[DEEP_MOSAIC];
	SDL_Surface *pShallowBottom = g_pTheDBM->pTextures[SHALLOW_MOSAIC];

	UINT *pwOTI = this->pwOSquareTI + wStartPos;
	UINT *pwFTI = this->pwFSquareTI + wStartPos;
	UINT *pwTTI = this->pwTSquareTI + wStartPos;
	vector<UINT> *pShadows = this->shadows + wStartPos;
	const EDGES *pbE = this->pbEdges + wStartPos;
	const TILEINFO *pbMI = this->pTileInfo + wStartPos;
	LIGHTTYPE *psL = this->psDisplayedLight + wStartPos * wLightValuesPerTile;

	const float fLightLevel = fRoomLightLevel[this->wDark];
	float fDark;
	const bool bAddLight = IsLightingRendered();

	bool bMosaicTile, bTransparentOTile, bTar, bNorthernWall,
			bTIsTransparent, bTIsTranslucent, bAddLightLayers;
	bool bBlitCustomTextureTile = false;
	UINT wTextureIndex;
	UINT wTileNo, wOTileNo, wTTileNo;
	UINT wX, wY, wTI;
	int nX, nY;

	for (wY = wRow; wY < wYEnd; ++wY)
	{
		for (wX = wCol; wX < wXEnd; ++wX)
		{
			if (this->bAllDirty || pbMI->dirty)
			{
				//Render one tile in the room by blitting successive sprites
				//on top of one another.
				nX = this->x + wX * CX_TILE;
				nY = this->y + wY * CY_TILE;
				wOTileNo = this->pRoom->GetOSquare(wX, wY);
				wTTileNo = this->pRoom->GetTSquare(wX, wY);
				wTI = *pwOTI;
				bMosaicTile = bIsMosaic(wOTileNo) || wOTileNo == T_PLATFORM_P || //platforms have pit drawn under them
						wTI == TI_WALL_M; //draw inner wall tiles with mosaic
				//Northern wall tiles have mosaic drawn on the bottom half.
				if ((bNorthernWall = pbE->bHalfWall))	//assign and check
					bMosaicTile = true;
				bTransparentOTile = wOTileNo == T_GOO || wOTileNo == T_FLOOR_IMAGE ||
						(wOTileNo == T_WALL_M && this->pCurrentGame && this->pCurrentGame->bHoldMastered);
				bTar = bIsTar(wTTileNo);
				bTIsTransparent = bTar && bEditor;
				bTIsTranslucent = bTar && this->pRoom->bBetterVision;
				bAddLightLayers = false;

				//Determine this tile's darkness.
				fDark = fLightLevel;
				const UINT darkVal = this->pRoom->tileLights.GetAt(wX, wY);
				if (bIsDarkTileValue(darkVal))
				{
					ASSERT(darkVal - LIGHT_OFF - 1 < NUM_DARK_TYPES);
					fDark *= darkMap[darkVal - LIGHT_OFF - 1];
				}

				//1. Draw opaque (floor) layer.
				const bool bWater = bIsWater(wOTileNo) || wOTileNo == T_PLATFORM_W || bIsSteppingStone(wOTileNo);
				if (bWater)
					goto OLayerDone; //water is handled below
				if (!bMosaicTile && !bTransparentOTile)
				{
					DrawRoomTile(*pwOTI);

					//In cases where doors are 2x2 tiles or thicker, draw filler to
					//remove the dimple on the edge.
					if (bIsDoor(wOTileNo))
						DrawDoorFiller(pDestSurface, wX,wY);

					//Special-case objects have extra effects rendered onto them.
					//Stairs: add special lighting.
					if (bIsStairs(wOTileNo))
					{
						UINT wRow, wCol;
						if (wOTileNo == T_STAIRS)
						{
							CalcStairPosition(this->pRoom, wX, wY, wRow, wCol);
							float fLight = 1.0f - (wCol-1)*0.04f;
							if (fLight < 0.1) fLight = 0.1f;
							g_pTheDBM->DarkenTile(nX, nY, fLight, pDestSurface);
						} else {
							CalcStairUpPosition(this->pRoom, wX, wY, wRow, wCol);
							float fLight = 1.0f + (wCol-1)*0.04f;
							if (fLight > 1.7) fLight = 1.7f;
							g_pTheDBM->LightenTile(pDestSurface, nX, nY, fLight);
						}
					}
				} else {
					//Draw tile from image texture.
					wTileNo = bTransparentOTile ?
							this->pRoom->coveredOSquares.GetAt(wX,wY) : wOTileNo;
					wTextureIndex = GetTextureIndexForTile(wTileNo, true); //base texture

					//Texture coords.
					SDL_Rect src = {0, 0, CX_TILE, CY_TILE};
					SDL_Rect dest = {nX, nY, CX_TILE, CY_TILE};
					ASSERT(wTextureIndex < TEXTURE_COUNT);
					SDL_Surface *pTileTexture = g_pTheDBM->pTextures[wTextureIndex];
					static const int nHalfWallY = CY_TILE/2 +1;
					//Calculate coords, tiling texture from room origin.
					src.x = (wX * CX_TILE) % pTileTexture->w;
					src.y = (wY * CY_TILE) % pTileTexture->h;
					if (bNorthernWall || wTileNo == T_WALL_IMAGE)
					{
						//Blit wall tile.
						DrawRoomTile(*pwOTI);
						if (bNorthernWall)
						{
							//Blit inner wall mosaic on bottom half of the tile only.
							src.y += nHalfWallY;
							src.h -= nHalfWallY;
							dest.y += nHalfWallY;
							dest.h -= nHalfWallY;
						}
					}
					//Blit the base texture.
					if (wTileNo != T_WALL_IMAGE || bNorthernWall)
						SDL_BlitSurface(pTileTexture, &src, pDestSurface, &dest);

					//1a. Special textures are drawn on top of the standard ones.
					//This allows transparency in special image to show the normal texture underneath.
					if (wOTileNo == T_PIT_IMAGE && !g_pTheDBM->pTextures[FLOOR_IMAGE])
						wOTileNo = T_PIT; //revert to normal pit look
					if (wOTileNo == T_WALL_IMAGE && !g_pTheDBM->pTextures[FLOOR_IMAGE])
						wOTileNo = T_WALL; //revert to normal wall look
					wTextureIndex = GetTextureIndexForTile(wOTileNo, false);
					bBlitCustomTextureTile = wTextureIndex == FLOOR_IMAGE;

					//1b. Special-case objects with extra effects rendered onto them.
					switch (wOTileNo)
					{
					case T_FLOOR_IMAGE: break;
					case T_PIT:
					case T_PLATFORM_P:
						//Determine whether pit sides need to be drawn.
						//Only draw pit sides to bottom of pit side mosaic.
						if (pbE->wPitY * CY_TILE < wPitHeight)
						{
							//If the pitside mosaic won't be plotted along its full width,
							//then use a smaller pitside texture for the remainder.
							UINT wPitX = pbE->wPitX % wPitSideTileWidth;
							src.x = wPitX * CX_TILE;
							const bool bFullTexture = wPitX + pbE->wPitRemaining >= wPitSideTileWidth;
							if (!bFullTexture)
								src.x = src.x % g_pTheDBM->pTextures[PITSIDE_SMALL]->w;

							//In order to show shadows under tall arches correctly,
							//must examine how far back the arch recedes.
							UINT wFloorDepth = 1;
							src.y = pbE->wPitY * CY_TILE;
							while (src.y + CY_TILE < (int)wPitHeight) {src.y += CY_TILE; ++wFloorDepth;}

							//If floor is more than one square deep at pit edge,
							//draw successive layers of pitside to portray depth in transparent areas.
							do {
								ASSERT(src.y >= 0);
								const UINT wOSquareAbove = this->pRoom->GetOSquareWithGuessing(wX, wY - pbE->wPitY - wFloorDepth);
								if (!bIsPit(wOSquareAbove) && wOSquareAbove != T_TRAPDOOR &&
										!bIsBridge(wOSquareAbove) && wOSquareAbove != T_PLATFORM_P)
								{
									//Draw partial pit side tile if there's a partial tile at the bottom of the source image.
									dest.h = (int)wPitHeight - src.y < CY_TILE ? wPitHeight - src.y : CY_TILE;

									SDL_BlitSurface(bFullTexture ?
											pPitsideTexture :
											g_pTheDBM->pTextures[PITSIDE_SMALL],
											&src, pDestSurface, &dest);
								}
								src.y -= CY_TILE;
							} while (--wFloorDepth);
						}

						//Draw side edge to trapdoors when hanging over pit.
						switch (this->pRoom->GetOSquareWithGuessing(wX, wY - 1))
						{
							case T_TRAPDOOR:
								DrawRoomTile(TI_TRAPDOOR_EDGE);
							break;
							case T_BRIDGE: case T_BRIDGE_H:
								DrawRoomTile(TI_BRIDGE_HEDGE);
							break;
							case T_BRIDGE_V:
								DrawRoomTile(TI_BRIDGE_VEDGE);
							break;
						}
					break;

					case T_WALL_H:
						//Show hidden broken walls in the editor.
						if (bEditor)
						{
							if (bNorthernWall)
								g_pTheBM->BlitTileImagePart(TI_WALL_H, nX, nY + nHalfWallY,
										0, nHalfWallY, CX_TILE, CY_TILE - nHalfWallY,
										pDestSurface, false, 196);
							else
								DrawTransparentRoomTile(TI_WALL_H, 196);
						}
					break;

					case T_WALL_M:
						//"Master" walls are transparent when hold is mastered.
						if (bTransparentOTile)
							DrawTransparentRoomTile(*pwOTI, 64);
					break;
					case T_GOO:
						ASSERT(bTransparentOTile);
						DrawTransparentRoomTile(*pwOTI, 196);
						break;
					default:	ASSERT(!bTransparentOTile); break;
					}
				}
OLayerDone:
				//1c. Draw outline around squares that need it.
				DrawTileEdges(wX, wY, pbE, pDestSurface);

				//1b (cont.) -- Custom textures are blitted on top of wall edging.
				if (bBlitCustomTextureTile)
				{
					//Calculate coords for special floor texture, tiling from
					//indicated origin.
					SDL_Rect src = {0, 0, CX_TILE, CY_TILE};
					SDL_Rect dest = {nX, nY, CX_TILE, CY_TILE};
					SDL_Surface *pTileTexture = g_pTheDBM->pTextures[wTextureIndex];
					UINT wSize = pTileTexture->w;
					int nI = ((int)wX-(int)this->pRoom->wImageStartX) * CX_TILE;
					while (nI < 0)
						nI += wSize;
					src.x = nI % wSize;
					wSize = pTileTexture->h;
					nI = ((int)wY-(int)this->pRoom->wImageStartY) * CY_TILE;
					while (nI < 0)
						nI += wSize;
					src.y = nI % wSize;

					SDL_BlitSurface(pTileTexture, &src, pDestSurface, &dest);
					bBlitCustomTextureTile = false;
				}

				//2a. Is there water or a water bank here?
				if (bWater || (bIsPlainFloor(wOTileNo) || wOTileNo == T_GOO))
				{
					bool bShallowWater = bIsShallowWater(wOTileNo) || bIsSteppingStone(wOTileNo);
					const UINT wWaterMask = bWater ? wOTileNo :
							CalcTileImageForWater(this->pRoom, wX, wY, T_WATER, &bShallowWater);

					if (bWater || wWaterMask != TI_WATER_NSWE)
					{
						const UINT wShallowTile = bShallowWater ?
								CalcTileImageForWater(this->pRoom, wX, wY, T_SHALLOW_WATER) :
								TI_SHALLOW_TOP;
						UINT wDeepMix = 0, wWSurfMasks = 0;

						if (!bShallowWater)
						{
							UINT water[5][5];
							for (UINT wTX = 0; wTX < 5; wTX++)
								for (UINT wTY = 0; wTY < 5; wTY++)
								{
									if (this->pRoom->IsValidColRow(wTX+wX-2,wTY+wY-2))
										water[wTX][wTY] = this->pRoom->GetOSquare(wTX+wX-2,wTY+wY-2);
									else
										water[wTX][wTY] = T_WATER;
								}

							const bool bNShallow = IsShallowImage(2,1);
							const bool bSShallow = IsShallowImage(2,3);
							const bool bWShallow = IsShallowImage(1,2);
							const bool bEShallow = IsShallowImage(3,2);
							if (bNShallow && bWShallow && !IsDeepWaterTile(water[1][1]))
								wDeepMix++;
							if (bNShallow && bEShallow && !IsDeepWaterTile(water[3][1]))
								wDeepMix += 2;
							if (bSShallow && bWShallow && !IsDeepWaterTile(water[1][3]))
								wDeepMix += 4;
							if (bSShallow && bEShallow && !IsDeepWaterTile(water[3][3]))
								wDeepMix += 8;
						}

						const bool bWaterMix = (bShallowWater && wShallowTile != TI_SHALLOW_TOP) || wDeepMix;
						if (bWaterMix)
						{
							if (!pWaterSurface)
								pWaterSurface = g_pTheBM->ConvertSurface(SDL_CreateRGBSurface(
										SDL_SWSURFACE, CX_TILE, 3*CY_TILE, g_pTheBM->BITS_PER_PIXEL, 0, 0, 0, 0));
							ASSERT(pWaterSurface);
							//pWaterSurface needs to create three things:
							//   1. The correct mixture of TI_WATER_TOP and TI_SHALLOW_TOP,
							//      using wShallowTile as a mask
							//   2. A Deep Water Mask using wShallowTile and wWaterMask
							//   3. A Shallow Water Mask using wShallowTile and wWaterMask
							wWSurfMasks = g_pTheDBM->CreateShallowWaterMix(bWater ? TI_WATER_TOP : wWaterMask,
									wShallowTile, wDeepMix, pWaterSurface);
						}

						SDL_Rect src = {0, 0, CX_TILE, CY_TILE};
						SDL_Rect dest = {nX, nY, CX_TILE, CY_TILE};
						SDL_Rect deepRect = {0, CY_TILE, CX_TILE, CY_TILE};
						SDL_Rect shlwRect = {0, 2*CY_TILE, CX_TILE, CY_TILE};

						//2b. Draw bank and water.
						if (bWater)
						{
							if (bWaterMix)
								g_pTheBM->BlitSurface(pWaterSurface, &src, pDestSurface, &dest);
							else
								DrawRoomTile(bShallowWater ? TI_SHALLOW_TOP : TI_WATER_TOP);
						} else {
							DrawRoomTile(wWaterMask);
							if (bWaterMix)
								g_pTheBM->BlitWithTileMask(wWaterMask, src,
									pWaterSurface, dest, pDestSurface);
							else
								g_pTheBM->BlitTileWithTileMask(wWaterMask,
									bShallowWater ? TI_SHALLOW_TOP : TI_WATER_TOP, dest, pDestSurface);
						}

						//3. Show northern walls reflected in water,
						//   and side edges of trapdoors, etc.
						const UINT wOTileAbove = this->pRoom->GetOSquareWithGuessing(wX, wY-1);
						UINT wRefWallTile = TI_UNSPECIFIED;
						switch (wOTileAbove)
						{
							case T_WALL: case T_WALL2: wRefWallTile = TI_WALL_REF; break;
							case T_WALL_B: wRefWallTile = TI_WALL_BREF; break;
							case T_WALL_H: wRefWallTile = TI_WALL_HREF; break;
							case T_TRAPDOOR: case T_TRAPDOOR2:
								wRefWallTile = TI_TRAPDOOR_EDGE; break;
							case T_BRIDGE: case T_BRIDGE_H:
								wRefWallTile = TI_BRIDGE_HEDGE; break;
							case T_BRIDGE_V: wRefWallTile = TI_BRIDGE_VEDGE; break;
							default: break;
						}
						if (wRefWallTile != TI_UNSPECIFIED)
						{
							if (bWater)
							{
								DrawTransparentRoomTile(wRefWallTile, 128);
							}
							else
								g_pTheBM->BlitTileWithTileMask(wWaterMask, wRefWallTile,
										dest, pDestSurface, 128);
						}

						//4. Add dark+light to o-layer before sky reflection is rendered.
						if (bAddLight)
						{
							AddLightInterp(pDestSurface, wX, wY, psL, fDark);
							bAddLightLayers = true;
						}

						//5. Draw sky/water bottom.
						if (this->bOutside && this->pSkyImage)
						{
							const int wXOffset = this->dwSkyX % this->pSkyImage->w;
							src.x = (int)(wX * CX_TILE + wXOffset);
							src.y = wY * CY_TILE;
							if (bWaterMix)
							{
								if (wWSurfMasks & 0x01)
									g_pTheBM->BlitWithMask(deepRect, pWaterSurface, src,
											this->pSkyImage, dest, pDestSurface,
											SKY_DEEP_OPACITY);
								if (wWSurfMasks & 0x02)
								{
									g_pTheBM->BlitWithMask(shlwRect, pWaterSurface, src,
											this->pSkyImage, dest, pDestSurface,
											SKY_SHALLOW_OPACITY);
									src.x = (wX * CX_TILE) % pShallowBottom->w;
									src.y = (wY * CY_TILE) % pShallowBottom->h;
									g_pTheBM->BlitWithMask(shlwRect, pWaterSurface, src,
											pShallowBottom, dest,
											pDestSurface, SKY_BOTTOM_OPACITY);
								}
							} else if (bWater) {
								g_pTheBM->BlitWrappingSurface(this->pSkyImage, src,
										pDestSurface, dest,
										bShallowWater ? SKY_SHALLOW_OPACITY : SKY_DEEP_OPACITY);
								if (bShallowWater)
								{
									src.x = (wX * CX_TILE) % pShallowBottom->w;
									src.y = (wY * CY_TILE) % pShallowBottom->h;
									g_pTheBM->BlitSurface(pShallowBottom,
											&src, pDestSurface, &dest, SKY_BOTTOM_OPACITY);
								}
							} else {
								g_pTheBM->BlitWithTileMask(wWaterMask, src, this->pSkyImage,
										dest, pDestSurface,
										bShallowWater ? SKY_SHALLOW_OPACITY : SKY_DEEP_OPACITY);
								if (bShallowWater)
								{
									src.x = (wX * CX_TILE) % pShallowBottom->w;
									src.y = (wY * CY_TILE) % pShallowBottom->h;
									g_pTheBM->BlitWithTileMask(wWaterMask, src,
											pShallowBottom, dest,
											pDestSurface, SKY_BOTTOM_OPACITY);
								}
							}
						} else {
							//Calculate coords, tiling pit texture from room origin.
							src.x = (wX * CX_TILE) % pDeepBottom->w;
							src.y = (wY * CY_TILE) % pDeepBottom->h;
							if (bWaterMix)
							{
								if (wWSurfMasks & 0x01)
									g_pTheBM->BlitWithMask(deepRect, pWaterSurface, src,
											pDeepBottom, dest,
											pDestSurface, PIT_DEEP_OPACITY);
								if (wWSurfMasks & 0x02)
								{
									g_pTheBM->BlitWithMask(shlwRect, pWaterSurface, src,
											pDeepBottom, dest,
											pDestSurface, PIT_SHALLOW_OPACITY);
									src.x = (wX * CX_TILE) % pShallowBottom->w;
									src.y = (wY * CY_TILE) % pShallowBottom->h;
									g_pTheBM->BlitWithMask(shlwRect, pWaterSurface, src,
											pShallowBottom, dest,
											pDestSurface, PIT_BOTTOM_OPACITY);
								}
							} else if (bWater) {
								g_pTheBM->BlitSurface(pDeepBottom, &src, pDestSurface,
										&dest, bShallowWater ? PIT_SHALLOW_OPACITY : PIT_DEEP_OPACITY);
								if (bShallowWater)
								{
									src.x = (wX * CX_TILE) % pShallowBottom->w;
									src.y = (wY * CY_TILE) % pShallowBottom->h;
									g_pTheBM->BlitSurface(pShallowBottom, &src, pDestSurface,
											&dest, PIT_BOTTOM_OPACITY);
								}
							} else {
								g_pTheBM->BlitWithTileMask(wWaterMask, src,
										pDeepBottom, dest, pDestSurface,
										bShallowWater ? PIT_SHALLOW_OPACITY : PIT_DEEP_OPACITY);
								if (bShallowWater)
								{
									src.x = (wX * CX_TILE) % pShallowBottom->w;
									src.y = (wY * CY_TILE) % pShallowBottom->h;
									g_pTheBM->BlitWithTileMask(wWaterMask, src,
											pShallowBottom,
											dest, pDestSurface, PIT_BOTTOM_OPACITY);
								}
							}
						}

						//6. Draw stepping stones on top of water.
						if (bIsSteppingStone(wOTileNo))
						{
							DrawRoomTile(TI_STEP_STONE);
							if (bAddLightLayers)
								AddLightInterp(pDestSurface, wX, wY, psL, fDark, TI_STEP_STONE);
						}
					}
				}

				RenderRoomTileObjects(wX, wY, nX, nY, pDestSurface,
						wOTileNo, *pwFTI, *pwTTI, pShadows, psL,
						fDark, bAddLight, bAddLightLayers, bEditor);
			}

			//Advance all pointers to next square.
			++pShadows;
			++pwOTI;
			++pwFTI;
			++pwTTI;
			++pbMI;
			++pbE;
			psL += wLightValuesPerTile;
		}
		//Advance to next row.
		if (wRowOffset)
		{
			pShadows += wRowOffset;
			pwOTI += wRowOffset;
			pwFTI += wRowOffset;
			pwTTI += wRowOffset;
			pbMI += wRowOffset;
			pbE += wRowOffset;
			psL += wLightRowOffset;
		}
	}
	if (pWaterSurface)
		SDL_FreeSurface(pWaterSurface);
}

//*****************************************************************************
void CRoomWidget::RenderRoomTileObjects(
//Render objects above the o-layer for a room tile onto a surface.
//
//Params:
	const UINT wX, const UINT wY,
	const int nX, const int nY,  //used in macros
	SDL_Surface *pDestSurface,
	const UINT wOTileNo, const UINT wFTI, const UINT wTTI,
	vector<UINT> *pShadows,
	LIGHTTYPE *psL,
	const float fDark,
	const bool bAddLight,
	const bool bAddLightLayers,
	const bool bEditor,
	const bool bPitPlatformTiles) //when set to true [default=false], darken only item masks, not entire tile.  Overrided by bAddLightLayers.
{
	ASSERT(this->pRoom);
	const UINT wTTileNo = this->pRoom->GetTSquare(wX, wY);

	const bool bIsPitTile = (bIsPit(wOTileNo) || wOTileNo == T_PLATFORM_P);
	//Pits show only dark.  Light only shines on f+t-layer items.
	//Deal with darkening the pit tile now.
	if (bAddLight && bIsPitTile && !bPitPlatformTiles)
		AddDark(fDark);

	bool bTar = bIsTar(wTTileNo);
	bool bTIsTransparent = bTar && bEditor;
	bool bTIsTranslucent = bTar && this->pRoom->bBetterVision;

	//2b. Add checkpoints on top of o-layer.
	const bool bIsCheckpoint = (this->bShowCheckpoints || bEditor) &&
			this->pRoom->checkpoints.has(wX,wY);
	if (bIsCheckpoint)
	{
		DrawRoomTile(TI_CHECKPOINT);
		if (bAddLightLayers || (bIsPitTile && bAddLight))
			AddLightInterp(pDestSurface, wX, wY, psL, fDark, TI_CHECKPOINT);
	}

	//3. Draw floor (object) layer.
	if (wFTI != TI_TEMPTY)
	{
		DrawRoomTile(wFTI);
		if (bAddLightLayers || (bIsPitTile && bAddLight))
			AddLightInterp(pDestSurface, wX, wY, psL, fDark, wFTI);
	}

	//4. Draw transparent (item) layer.
	if (wTTI != TI_TEMPTY && !bTar)
	{
		switch (wTTileNo)
		{
			case T_TOKEN:
				//When tokens obscure arrows, make token transparent.
				if (bIsArrow(this->pRoom->GetFSquare(wX,wY)))
					DrawTransparentRoomTile(wTTI, 96);
				else
					DrawRoomTile(wTTI);
			break;
			case T_STATION:
			{
				DrawRoomTile(wTTI);
				const UINT tParam = this->pRoom->GetTParam(wX,wY);
				g_pTheBM->LightenRectWithTileMask(pDestSurface, nX, nY,
						CBitmapManager::CX_TILE, CBitmapManager::CY_TILE,
						1.0f+lightMap[0][tParam], 1.0f+lightMap[1][tParam], 1.0f+lightMap[2][tParam],
						wTTI, 0, 0);
			}
			break;
			default: DrawRoomTile(wTTI); break;
		}
		if (bAddLightLayers || (bIsPitTile && bAddLight))
			AddLightInterp(pDestSurface, wX, wY, psL, fDark, wTTI);
	}

	if (bEditor) //Area light sprite only shows in room editor.
	{
		UINT lightVal = this->pRoom->tileLights.GetAt(wX,wY);
		if (bIsWallLightValue(lightVal))
			DrawTransparentRoomTile(TI_WALLLIGHT, 180);
	}

	//5. Cast shadows onto environment, except onto pit.
	if (!bIsPitTile)
	{
		if (pShadows && pShadows->size())
		{
			DrawShadow(&(*pShadows->begin()), pShadows->size());

			//5b. If this is a high obstacle, no shadow should be cast on any part of it.
			if (wTTileNo == T_OBSTACLE)
			{
				UINT wObSize, wXPos, wYPos;
				GetObstacleStats(this->pRoom, wX, wY, wObSize, wXPos, wYPos);
				if ((wXPos || wYPos) &&
						CastsWallShadow(this->pRoom->GetOSquare(wX - wXPos, wY - wYPos)))
				{
					DrawRoomTile(wTTI);
					if (bAddLightLayers)
						AddLightInterp(pDestSurface, wX, wY, psL, fDark, wTTI);
				}
			}
		}

		//6. Room lighting to light everything on this tile.
		//Pits were handled prior to this
		if (bAddLight && !bAddLightLayers)
			AddLightInterp(pDestSurface, wX, wY, psL, fDark);
	}

	//6a. Tarstuff is rendered on top of all light and shadows.
	if (bTar)
	{
		if (bTIsTransparent)
		{
			DrawTransparentRoomTile(wTTI, 128);
		} else if (bTIsTranslucent) {
			DrawTransparentRoomTile(wTTI, 204);
		} else {
			DrawRoomTile(wTTI);
		}
		AddLight(pDestSurface, nX, nY, psL, fDark, wTTI);
	}
}
#undef DrawRoomTile
#undef DrawTransparentRoomTile
#undef DrawShadow
#undef AddDark
#undef AddDarkMask
#undef IsDeepWaterTile
#undef IsShallowTile
#undef IsShallowImage

//*****************************************************************************
void CRoomWidget::Paint(
//Plots current room to display.
//See comments below for an outline of how this is done.
//
//Params:
	bool bUpdateRect)       //(in)   If true (default) and destination
								//    surface is the screen, the screen
								//    will be immediately updated in
								//    the widget's rect.
{
	//Drawing code below needs to be modified to accept offsets.  Until then,
	//this widget can't be offset.
	ASSERT(!IsScrollOffset());

	if (!this->pRoom)
	{
		//Draw black area.
		int nOffsetX, nOffsetY;
		GetScrollOffset(nOffsetX, nOffsetY);
		SURFACECOLOR Color = GetSurfaceColor(GetDestSurface(), 0, 0, 0);
		SDL_Rect rect = {this->x + nOffsetX, this->y + nOffsetY, this->w, this->h};
		DrawFilledRect(rect, Color);
		return;
	}

	bool bMoveMade;
	UINT wTurn;
	if (this->pCurrentGame)
	{
		//Keep room pointer synched.
		if (this->pRoom != this->pCurrentGame->pRoom)
			this->pRoom = this->pCurrentGame->pRoom;

		wTurn = this->pCurrentGame->wTurnNo;
		bMoveMade = (wTurn != this->wLastTurn);
	} else {
		bMoveMade = false;
	}
	CDbRoom& room = *(this->pRoom);

	//1a. Prepare vars for drawing this frame.
	if (bMoveMade)
	{
		//Start new movement animation.
		this->wLastTurn = wTurn;
		this->dwCurrentDuration = this->dwTimeSinceLastFrame; //start into move
		this->cursorLight.wX = this->cursorLight.wY = UINT(-1); //turn off temporary light

		HighlightSelectedTile(); //refresh user highlight according to current room state
	}
	if (this->pCurrentGame) {
		SetFrameVars(bMoveMade);
	}

	//1b. Determine animation progress this frame.
	const Uint32 dwNow = SDL_GetTicks();
	if (!this->dwLastFrame)
		dwLastFrame = dwNow;
	this->dwTimeSinceLastFrame = dwNow - dwLastFrame;
	this->dwCurrentDuration += this->dwTimeSinceLastFrame;
	dwLastFrame = dwNow;

	const bool bIsPlacingDouble = this->pCurrentGame ? this->pCurrentGame->swordsman.wPlacingDoubleType != 0 : false;
	this->dwMovementStepsLeft = this->dwCurrentDuration >= this->dwMoveDuration || bIsPlacingDouble ? 0 :
		(CX_TILE - (this->dwCurrentDuration * CX_TILE / this->dwMoveDuration));
	const bool bMoveAnimationInProgress = this->dwMovementStepsLeft || bMoveMade || this->bAnimationInProgress;
	this->bAnimationInProgress = this->dwMovementStepsLeft != 0; //if true, move animation must be drawn next frame to complete it

	//1c. Prepare room image now, if requested.
	SDL_Surface *pDestSurface = GetDestSurface();
	if (!this->pCurrentGame)
	{
		//Render view of the room out of play.
		RenderRoom(this->wShowCol, this->wShowRow,
				CDrodBitmapManager::DISPLAY_COLS, CDrodBitmapManager::DISPLAY_ROWS, false);

		//Blit entire room.
		SDL_Rect src = {this->x, this->y, this->w, this->h};
		SDL_Rect dest = {this->x, this->y, this->w, this->h};
		SDL_BlitSurface(this->pRoomSnapshotSurface, &src, pDestSurface, &dest);

		RenderFogInPit(pDestSurface);
		DrawPlatforms(pDestSurface);
		DrawMonsters(this->pRoom->pFirstMonster, pDestSurface, false);
		RenderEnvironment(pDestSurface);
		if (bUpdateRect)
			UpdateRect();
		this->bAllDirty = false;
		return; //once the room has been rendered out of play, there's nothing else to render
	}

	if (this->bRenderRoom || this->bRenderPlayerLight)
	{
		RenderRoomInPlay(this->wShowCol, this->wShowRow);
		this->bRenderRoom = false;
	}

	//1d. Animate monster frames.
	const bool bPlayerIsAlive = !this->pCurrentGame->IsPlayerDying();
	if (!bIsPlacingDouble && bPlayerIsAlive)
		AnimateMonsters();

	//2. Erase all room sprites that were drawn last frame.
	ASSERT(this->pRoomSnapshotSurface);
	ASSERT(this->pTileInfo);
	TILEINFO *pbMI = this->pTileInfo;
	TILEINFO *const pbMIStop = pbMI +
			CDrodBitmapManager::DISPLAY_ROWS * CDrodBitmapManager::DISPLAY_COLS;
	if (this->bAllDirty)
	{
		//Re-blit entire room.
		SDL_Rect src = {this->x, this->y, this->w, this->h};
		SDL_Rect dest = {this->x, this->y, this->w, this->h};
		SDL_BlitSurface(this->pRoomSnapshotSurface, &src, pDestSurface, &dest);

		//Undirty all room tiles.
		while (pbMI != pbMIStop)
		{
			pbMI->dirty = pbMI->damaged = pbMI->monster = 0;
			++pbMI;
		}
		pbMI = this->pTileInfo;
	} else {
		//Blit only dirtied room tiles from last turn to save time.
		//Undirty them and mark as "damaged" to indicate their screen regions
		//must be updated.
		while (pbMI != pbMIStop)
			(pbMI++)->damaged = 0;
		pbMI = this->pTileInfo;
		BlitDirtyRoomTiles(bMoveMade);
	}

	//3. Draw room sprites.
	if (bIsPlacingDouble && bPlayerIsAlive)
	{
		//Draw player double placement cursor only.
		//(The action is frozen and nothing else gets drawn.)
		DrawDoubleCursor(this->pCurrentGame->swordsman.wDoubleCursorX,
				this->pCurrentGame->swordsman.wDoubleCursorY, pDestSurface);
		if (this->pCurrentGame->swordsman.wPlacingDoubleType == M_DECOY)
		{
			DrawInvisibilityRange(this->pCurrentGame->swordsman.wDoubleCursorX,
				this->pCurrentGame->swordsman.wDoubleCursorY, pDestSurface);
		}
	} else {
		//When action is not frozen, draw the following:
		if (bPlayerIsAlive)
		{
			//3a. Draw effects that go on top of room image, under monsters/swordsman.
			RenderFogInPit(pDestSurface);
			DrawPlatforms(pDestSurface, false, bMoveAnimationInProgress);
			this->pOLayerEffects->DrawEffects();
			this->pOLayerEffects->DirtyTiles();
			this->pTLayerEffects->DrawEffects();
			this->pTLayerEffects->DirtyTiles();

			//3b. Repaint monsters.
			if (this->bAllDirty || bMoveAnimationInProgress)
			{
				//Draw monsters (that aren't killing swordsman).
				DrawMonsters(room.pFirstMonster, pDestSurface, false,
					bMoveAnimationInProgress);
			} else {
				RedrawMonsters(pDestSurface);
			}

			//3c. Building marker effects.
			this->pMLayerEffects->RemoveEffectsOfType(EPENDINGBUILD);
			if (!room.building.empty())
			{
				bool bFirst = true;
				for (UINT wY=this->wShowRow; wY<this->wShowRow + CDrodBitmapManager::DISPLAY_ROWS; ++wY)
					for (UINT wX=this->wShowCol; wX<this->wShowCol + CDrodBitmapManager::DISPLAY_COLS; ++wX)
					{
						UINT wTileNo, wTile = room.building.get(wX,wY);
						if (wTile)
						{
							--wTile; //convert from 1-based

							//Display empty item as the o-tile below it.
							if (wTile == T_EMPTY)
								wTile = room.GetOSquare(wX,wY);

							switch (wTile)
							{
								case T_WALL_IMAGE:
								case T_WALL: wTileNo = TI_WALL; break;
								case T_WALL_B: wTileNo = TI_WALL_B; break;
								case T_FUSE: wTileNo = TI_FUSE; break;
								case T_WATER: wTileNo = TI_WATER_TOP; break;
								case T_SHALLOW_WATER: wTileNo = TI_SHALLOW_TOP; break;
								case T_PIT_IMAGE:
								case T_PIT: wTileNo = TI_PIT_M; break;
								case T_GOO: wTileNo = TI_GOO_NSWE; break;
								case T_FLOOR_IMAGE: wTileNo = TI_FLOOR; break;
								case T_DOOR_Y: wTileNo = TI_DOOR_Y; break;
								case T_DOOR_YO: wTileNo = TI_DOOR_YO; break;
								case T_DOOR_M: wTileNo = TI_DOOR_M; break;
								case T_DOOR_GO: wTileNo = TI_DOOR_GO; break;
								case T_DOOR_B: wTileNo = TI_DOOR_B; break;
								case T_DOOR_BO: wTileNo = TI_DOOR_BO; break;
								case T_DOOR_C: wTileNo = TI_DOOR_C; break;
								case T_DOOR_CO: wTileNo = TI_DOOR_CO; break;
								case T_DOOR_R: wTileNo = TI_DOOR_R; break;
								case T_DOOR_RO: wTileNo = TI_DOOR_RO; break;
								case T_EMPTY: wTileNo = TI_TEMPTY; ASSERT(!"Not valid tile"); break;
								default: wTileNo = GetTileImageForTileNo(wTile); break;
							}
							if (wTileNo != CALC_NEEDED)
								this->pMLayerEffects->AddEffect(new CPendingBuildEffect(
										this, wTileNo, wX, wY, bFirst));
							bFirst = false;
						}
					}
			}
		}

		//4. Draw player.
		if (this->bShowingPlayer)
			DrawPlayer(this->pCurrentGame->swordsman, pDestSurface);

		//5. If monster is killing swordsman, draw it on top.
		if (!bPlayerIsAlive)
			DrawMonsterKillingPlayer(pDestSurface);

		//5a. Draw effects that go on top of monsters/swordsman.
		this->pMLayerEffects->DrawEffects();
		this->pMLayerEffects->DirtyTiles();
	}

	//6. Draw effects that go on top of everything else drawn in the room.
	if (bPlayerIsAlive)
		RenderEnvironment(pDestSurface);
	this->pLastLayerEffects->DrawEffects();
	this->pLastLayerEffects->DirtyTiles();

	//Last turn/movement should be drawn completely now.
	this->bFinishLastMovementNow = false;
	this->bRequestEvilEyeGaze = false;

	//If any widgets are attached to this one, draw them now.
	PaintChildren();

	//7. Show changes on screen.
	if (bUpdateRect)
		UpdateRoomRects();

	//Everything has been (re)painted by now.
	this->bAllDirty = false;
}

//*****************************************************************************
void CRoomWidget::PaintClipped(int /*nX*/, int /*nY*/, UINT /*wW*/, UINT /*wH*/,
		const bool /*bUpdateRect*/)
{
	//Paint() uses direct access to pixels, so it can't be clipped with
	//SDL_SetClipRect().  Either you need to write PaintClipped() or change
	//the situation which causes this widget to be clipped.
	ASSERT(!"Can't paint clipped.");
}

void CRoomWidget::RedrawMonsters(SDL_Surface* pDestSurface)
{
	//Paint monsters whose tiles have been repainted.
	DrawDamagedMonsters(pDestSurface, true); //Draw ghost NPCs first
	DrawDamagedMonsters(pDestSurface);

	//Redraw monster and NPC swords to ensure no other monster is being drawn on top of them.
	if (!this->dwMovementStepsLeft)
	{
		DrawDamagedMonsterSwords(pDestSurface);
	}
}

//*****************************************************************************
void CRoomWidget::RenderEnvironment(SDL_Surface *pDestSurface)	//[default=NULL]
//(Re)draw environmental effects on tiles being redrawn.
{
	static float fBrilliance = 1.0;	//lightning

	if (!IsWeatherRendered())
		return;

	bool bHasted = false, bIsPlacingDouble = false;
	if (this->pCurrentGame)
	{
		const CSwordsman& player = this->pCurrentGame->swordsman;
		bHasted = player.bIsHasted;
		bIsPlacingDouble = player.wPlacingDoubleType != 0;
	}

	//Add a new snowflake to the room every ~X frames.
	if (this->wSnow && RAND(SNOW_INCREMENTS-1) < this->wSnow &&
			this->w && this->y) //hack: snowflakes draw on room edges during transition -- this should stop it
		AddMLayerEffect(new CSnowflakeEffect(this));

	//Add a new raindrop to the room every ~X frames.
	if (this->rain && RAND(RAIN_INCREMENTS-1) < this->rain && !bIsPlacingDouble &&
			this->w && this->y) //hack: rain draws on room edges during transition -- this should stop it
		AddMLayerEffect(new CRaindropEffect(this, bHasted));

	if (!(this->dwLightning || this->bFog || this->bClouds || this->bSunlight))
		return;	//Nothing else to do.

	if (!pDestSurface) pDestSurface = GetDestSurface();

	//Optimized room-wide blits can be handled here.
	if (this->bAllDirty)
	{
		if (this->dwLightning)
		{
			//Make lightning flicker.  Should only change when entire screen is updated.
			fBrilliance = 1.0f + fMaxLightIntensity[this->wDark] * fRAND(1.0f);
		}
	}

	//Tile-by-tile handling is done here.
	TILEINFO *pbMI = this->pTileInfo;
	CCoordSet dirtyTiles;
	const UINT wXEnd = this->wShowCol + CDrodBitmapManager::DISPLAY_COLS;
	const UINT wYEnd = this->wShowRow + CDrodBitmapManager::DISPLAY_ROWS;

	bool bShowFogHere = false;
	for (UINT wY = this->wShowRow; wY < wYEnd; ++wY)
	{
		for (UINT wX = this->wShowCol; wX < wXEnd; ++wX)
		{
			if (this->bAllDirty || pbMI->damaged)
			{
				dirtyTiles.insert(wX,wY);

				//Draw effects bottom-up.
				const UINT wOSquare = this->pRoom->GetOSquare(wX,wY);

				if (!(bIsPit(wOSquare) || wOSquare == T_PLATFORM_P))
				{
					if (this->bSunlight)
					{
						SDL_Rect src = {wX * CX_TILE,	wY * CY_TILE, CX_TILE, CY_TILE};
						SDL_Rect dest = {this->x + wX * CX_TILE, this->y + wY * CY_TILE, CX_TILE, CY_TILE};
						vector<UINT> &shadows = this->shadows[this->pRoom->ARRAYINDEX(wX,wY)];
						//Determine what type of cloud behavior would be most consistent to model.
						//The open sky model is given priority, otherwise free-moving clouds are used.
						if (this->bClouds && !this->bSkyVisible)
						{
							const int wXOffset = (int)this->fCloudX % this->images[SUNSHINE_SURFACE]->w;
							const int wYOffset = (int)this->fCloudY % this->images[SUNSHINE_SURFACE]->h;
							src.x += wXOffset;
							src.y += wYOffset;
						} else {
							//Cloud shadows move faster over the ground than in the reflection of the sky.
							static const UINT wShadowSpeedMultiplier = 3;
							const int wXOffset = (this->dwSkyX * wShadowSpeedMultiplier)
								% this->images[SUNSHINE_SURFACE]->w;
							src.x += wXOffset;
						}
						UINT* pwShadow = NULL;
						if (shadows.size() > 0)
							pwShadow = &(*shadows.begin());
						g_pTheBM->ShadeWithWrappingSurfaceMask(this->images[SUNSHINE_SURFACE], src,
								pDestSurface, dest, pwShadow, shadows.size());
					}

					if (this->dwLightning)
						g_pTheBM->LightenTile(pDestSurface,
							this->x + wX*CX_TILE, this->y + wY*CY_TILE, fBrilliance);

					//Handles blitting full fog above floor/walls.
					{
						switch (this->cFogLayer)
						{
							default: break;
							case 2: //over floor
								bShowFogHere = !(bIsWall(wOSquare) || bIsCrumblyWall(wOSquare) || bIsDoor(wOSquare));
							break;
							case 3: //over walls (i.e. everything)
								bShowFogHere = true;
							break;
						}
						if (bShowFogHere)
						{
							const int wXOffset = (int)this->fFogX % this->images[FOG_SURFACE]->w;
							const int wYOffset = (int)this->fFogY % this->images[FOG_SURFACE]->h;

							//Fog with uniform alpha.
							SDL_Rect src = {(int)(wXOffset + wX * CX_TILE), (int)(wYOffset + wY * CY_TILE), CX_TILE, CY_TILE};
							SDL_Rect dest = {this->x + wX * CX_TILE, this->y + wY * CY_TILE, CX_TILE, CY_TILE};
							SDL_SetAlpha(this->images[FOG_SURFACE], SDL_SRCALPHA, MIN_FOG_OPACITY);
							g_pTheBM->BlitWrappingSurface(this->images[FOG_SURFACE], src, pDestSurface, dest);
						}
					}
				}
			}
			++pbMI;
		}
	}

	for (vector<CPlatform*>::const_iterator platformIter = this->pRoom->platforms.begin();
			platformIter != this->pRoom->platforms.end(); ++platformIter)
	{
		const CPlatform& platform = *(*platformIter);

		//ignore water platforms
		if (platform.GetTypes().has(T_WATER)) continue;

		const bool bPlatformMoved = platform.xDelta || platform.yDelta;

		//Calculate animation offset (in pixels).
		UINT wXOffset = 0, wYOffset = 0;
		if (bPlatformMoved && this->dwMovementStepsLeft)
		{
			wXOffset = -platform.xDelta * this->dwMovementStepsLeft;
			wYOffset = -platform.yDelta * this->dwMovementStepsLeft;
		}

		CCoordSet tiles;
		platform.GetTiles(tiles);

		for (CCoordSet::const_iterator tileIter = tiles.begin();
				tileIter != tiles.end(); ++tileIter)
		{
			UINT wX = tileIter->wX, wY = tileIter->wY;

			if (this->bAllDirty || dirtyTiles.has(wX,wY))
			{
				UINT wXDest = wX * CX_TILE + wXOffset;
				UINT wYDest = wY * CY_TILE + wYOffset;
				if (this->bSunlight)
				{
					SDL_Rect src = {wXDest, wYDest, CX_TILE, CY_TILE};
					SDL_Rect dest = {this->x + wXDest, this->y + wYDest, CX_TILE, CY_TILE};

					vector<UINT> &shadows = this->shadows[this->pRoom->ARRAYINDEX(wX,wY)];
					UINT* pwShadow = NULL;
					if (!bPlatformMoved && (shadows.size() > 0))
						pwShadow = &(*shadows.begin());

					//Determine what type of cloud behavior would be most consistent to model.
					//The open sky model is given priority, otherwise free-moving clouds are used.
					if (this->bClouds && !this->bSkyVisible)
					{
						const int wCXOffset = (int)this->fCloudX % this->images[SUNSHINE_SURFACE]->w;
						const int wCYOffset = (int)this->fCloudY % this->images[SUNSHINE_SURFACE]->h;
						src.x += wCXOffset;
						src.y += wCYOffset;
					} else {
						//Cloud shadows move faster over the ground than in the reflection of the sky.
						static const UINT wShadowSpeedMultiplier = 3;
						const int wCXOffset = (this->dwSkyX * wShadowSpeedMultiplier)
							% this->images[SUNSHINE_SURFACE]->w;
						src.x += wCXOffset;
					}
					g_pTheBM->ShadeWithWrappingSurfaceMask(this->images[SUNSHINE_SURFACE], src,
						pDestSurface, dest, pwShadow, bPlatformMoved ? 0 : shadows.size());
				}

				if (this->dwLightning)
					g_pTheBM->LightenTile(pDestSurface,
						this->x + wXDest, this->y + wYDest, fBrilliance);

				//Handles blitting full fog above floor/walls.
				if (this->cFogLayer >= 2)
				{
					const int wXOffset = (int)this->fFogX % this->images[FOG_SURFACE]->w;
					const int wYOffset = (int)this->fFogY % this->images[FOG_SURFACE]->h;

					//Fog with uniform alpha.
					SDL_Rect src = {(int)(wXOffset + wXDest), (int)(wYOffset + wYDest), CX_TILE, CY_TILE};
					SDL_Rect dest = {this->x + wXDest, this->y + wYDest, CX_TILE, CY_TILE};
					SDL_SetAlpha(this->images[FOG_SURFACE], SDL_SRCALPHA, MIN_FOG_OPACITY);
					g_pTheBM->BlitWrappingSurface(this->images[FOG_SURFACE], src, pDestSurface, dest);
				}
			}
		}
	}

	if (this->bClouds)
	{
		for (CCoordSet::const_iterator tile=dirtyTiles.begin(); tile!=dirtyTiles.end(); ++tile)
		{
			UINT wX = tile->wX;
			UINT wY = tile->wY;

			SDL_Rect src = {(int)(wX * CX_TILE), (int)(wY * CY_TILE), CX_TILE, CY_TILE};
			SDL_Rect dest = {this->x + wX * CX_TILE, this->y + wY * CY_TILE, CX_TILE, CY_TILE};
			//If sky is visible, move clouds the same way.
			//Otherwise use the free-moving cloud movement.
			if (this->bSkyVisible)
			{
				static const UINT wCloudSpeedMultiplier = 2;
				const int wXOffset = (this->dwSkyX * wCloudSpeedMultiplier)
					% this->images[CLOUD_SURFACE]->w;
				src.x += wXOffset;
			} else {
				const int wXOffset = (int)this->fCloudX % this->images[CLOUD_SURFACE]->w;
				const int wYOffset = (int)this->fCloudY % this->images[CLOUD_SURFACE]->h;
				src.x += wXOffset;
				src.y += wYOffset;
			}
			g_pTheBM->BlitWrappingSurface(this->images[CLOUD_SURFACE], src, pDestSurface, dest);
		}
	}
}

//*****************************************************************************
void CRoomWidget::RenderFogInPit(SDL_Surface *pDestSurface) //[default=NULL]
{
	if (!pDestSurface)
		pDestSurface = GetDestSurface();

	if (!this->bFog)
		return;

	//Tile-by-tile handling is done here.
	TILEINFO *pbMI = this->pTileInfo;
	const UINT wXEnd = this->wShowCol + CDrodBitmapManager::DISPLAY_COLS;
	const UINT wYEnd = this->wShowRow + CDrodBitmapManager::DISPLAY_ROWS;

	const UINT pit_image_height = static_cast<UINT>(g_pTheDBM->pTextures[PITSIDE_MOSAIC]->h);
	const UINT wMaxPitHeight = (pit_image_height / CY_TILE) +
			((pit_image_height % CY_TILE) > 0 ? 1 : 0);
	const float fFogMultiplier = float(MAX_FOG_OPACITY) / float(wMaxPitHeight);

	for (UINT wY = this->wShowRow; wY < wYEnd; ++wY)
	{
		for (UINT wX = this->wShowCol; wX < wXEnd; ++wX)
		{
			if (this->bAllDirty || pbMI->damaged)
			{
				const UINT wOSquare = this->pRoom->GetOSquare(wX,wY);

				if (bIsPit(wOSquare) || wOSquare == T_PLATFORM_P) //render fog under platform
				{
					//Show fog here.
					const int wXOffset = (int)this->fFogX % this->images[FOG_SURFACE]->w;
					const int wYOffset = (int)this->fFogY % this->images[FOG_SURFACE]->h;
					UINT wPitHeight = this->pbEdges[this->pRoom->ARRAYINDEX(wX,wY)].wPitY;
					if (wPitHeight >= wMaxPitHeight)
					{
						//Fog with uniform alpha.
						SDL_Rect src = {(int)(wXOffset + wX * CX_TILE), (int)(wYOffset + wY * CY_TILE), CX_TILE, CY_TILE};
						SDL_Rect dest = {this->x + wX * CX_TILE, this->y + wY * CY_TILE, CX_TILE, CY_TILE};
						SDL_SetAlpha(this->images[FOG_SURFACE], SDL_SRCALPHA, MAX_FOG_OPACITY);
						g_pTheBM->BlitWrappingSurface(this->images[FOG_SURFACE], src, pDestSurface, dest);
					} else {
						//Fog with graded alpha.
						//Draw one pixel row at a time.
						SDL_Rect src = {(int)(wXOffset + wX * CX_TILE), (int)(wYOffset + wY * CY_TILE), CX_TILE, 1};
						SDL_Rect dest = {this->x + wX * CX_TILE, this->y + wY * CY_TILE, CX_TILE, 1};
						for (UINT row=0; row<(UINT)CY_TILE; ++row)
						{
							const float row_height = wPitHeight + row/float(CY_TILE);
							Uint8 nOpacity = Uint8(row_height * fFogMultiplier);
							if ((this->cFogLayer > 1) && (nOpacity < MIN_FOG_OPACITY))
								nOpacity = MIN_FOG_OPACITY;
							SDL_SetAlpha(this->images[FOG_SURFACE], SDL_SRCALPHA, nOpacity);
							g_pTheBM->BlitWrappingSurface(this->images[FOG_SURFACE], src, pDestSurface, dest);
							++src.y;
							++dest.y;
						}
					}
				}
			}

			++pbMI;
		}
	}
}

//*****************************************************************************
void CRoomWidget::UpdateRoomRects()
//Show room changes on screen.
{
	if (this->bAllDirty)
	{
		//Whole room was redrawn, thus it should be updated on screen.
		UpdateRect();
		return;
	}

	//Don't damage the entire room region.  Damaging only those tiles that
	//we know were actually drawn to will take much less time.
	TILEINFO *pbMI = this->pTileInfo;

	//Count how many runs of rectangles are being blitted.
	UINT wDirtyCount = 0;
	bool bInDirtyRect = false;
	UINT wX, wY;
	for (wY = 0; wY < this->pRoom->wRoomRows; ++wY)
	{
		for (wX = 0; wX < this->pRoom->wRoomCols; ++wX)
		{
			if (pbMI->damaged || pbMI->dirty)
			{
				if (!bInDirtyRect)
				{
					++wDirtyCount;
					bInDirtyRect = true;
				}
			} else {
				bInDirtyRect = false;
			}
			++pbMI;
		}
		bInDirtyRect = false; //row run stops
	}
	if (wDirtyCount > 50)
	{
		//We're gonna be re-blitting a lot of room pieces.  Calling a single
		//update for the whole room will probably be less expensive.
		UpdateRect();
		return;
	}

	//Update tiles where something needs to redrawn on screen.
	//This includes tiles that had old sprites erased this frame (damaged),
	//as well as tiles where something new was drawn this frame (dirty).
	pbMI = this->pTileInfo;
	UINT wStartIndex = 0;
	for (wY = 0; wY < this->pRoom->wRoomRows; ++wY)
	{
		for (wX = 0; wX < this->pRoom->wRoomCols; ++wX)
		{
			if (pbMI->damaged || pbMI->dirty)
			{
				if (!bInDirtyRect)
				{
					wStartIndex = pbMI - this->pTileInfo;
					bInDirtyRect = true;
				}
			} else if (bInDirtyRect) {
				//Damage the run of tiles up until this one.
				const UINT wAfterEndIndex = pbMI - this->pTileInfo;
				UpdateRect(this->x + (wStartIndex%CDrodBitmapManager::DISPLAY_COLS)*CX_TILE,
						this->y + (wStartIndex/CDrodBitmapManager::DISPLAY_COLS)*CY_TILE,
						CX_TILE * (wAfterEndIndex - wStartIndex), CY_TILE);
				bInDirtyRect = false;
			}
			++pbMI;
		}
		if (bInDirtyRect)
		{
			const UINT wAfterEndIndex = pbMI - this->pTileInfo;
			UpdateRect(this->x + (wStartIndex%CDrodBitmapManager::DISPLAY_COLS)*CX_TILE,
					this->y + (wStartIndex/CDrodBitmapManager::DISPLAY_COLS)*CY_TILE,
					CX_TILE * (wAfterEndIndex - wStartIndex), CY_TILE);
			bInDirtyRect = false;
		}
	}
}

//*****************************************************************************
void CRoomWidget::RemoveLastLayerEffectsOfType(
//Removes all last-layer effects of given type in the room.
//
//Params:
	const EffectType eEffectType, //(in) Type of effect to remove.
	const bool bForceClearAll) //if set [default=true], delete all effects,
	                     //including those that request to be retained
{
	ASSERT(this->pLastLayerEffects);
	this->pLastLayerEffects->RemoveEffectsOfType(eEffectType, bForceClearAll);
}

//*****************************************************************************
void CRoomWidget::RemoveMLayerEffectsOfType(
//Removes all m-layer effects of given type in the room.
//
//Params:
	const EffectType eEffectType) //(in) Type of effect to remove.
{
	ASSERT(this->pMLayerEffects);
	this->pMLayerEffects->RemoveEffectsOfType(eEffectType);
}

//*****************************************************************************
void CRoomWidget::RemoveTLayerEffectsOfType(
//Removes all t-layer effects of given type in the room.
//
//Params:
	const EffectType eEffectType) //(in) Type of effect to remove.
{
	ASSERT(this->pTLayerEffects);
	this->pTLayerEffects->RemoveEffectsOfType(eEffectType);
}

void CRoomWidget::RemoveOLayerEffectsOfType(const EffectType eEffectType)
{
	ASSERT(this->pOLayerEffects);
	this->pOLayerEffects->RemoveEffectsOfType(eEffectType);
}

//*****************************************************************************
void CRoomWidget::SetOpacityForMLayerEffectsOfType(
//Set opacity for all m-layer effects of given type in the room.
//
//Params:
	const EffectType eEffectType, //(in) Type of effect to remove.
	float fOpacity)
{
	ASSERT(this->pMLayerEffects);
	this->pMLayerEffects->SetOpacityForEffectsOfType(eEffectType, fOpacity);
}

//*****************************************************************************
void CRoomWidget::GetSquareRect(
//Get rect on screen surface of a specified square.
//
//Params:
	UINT wCol, UINT wRow,   //(in)   Square to get rect for.
	SDL_Rect &SquareRect)   //(out)  Receives rect.
const
{
	ASSERT(IS_COLROW_IN_DISP(wCol, wRow));
	SET_RECT(SquareRect, this->x + (CX_TILE * wCol),
			this->y + (CY_TILE * wRow), CX_TILE,
			CY_TILE);
}

//*****************************************************************************
UINT CRoomWidget::GetOrbMID(const UINT type)
//Returns: messageID corresponding to pressure plate type
{
	switch (type)
	{
		default: return TILE_MID[T_ORB];
		case OT_ONEUSE: return MID_OrbCracked;
		case OT_BROKEN: return MID_OrbBroken;
	}
}

//*****************************************************************************
UINT CRoomWidget::GetPressurePlateMID(const UINT type)
//Returns: messageID corresponding to pressure plate type
{
	switch (type)
	{
		default: return TILE_MID[T_PRESSPLATE];
		case OT_TOGGLE: return MID_PressurePlateToggle;
		case OT_ONEUSE: return MID_PressurePlateOneUse;
		case OT_BROKEN: return MID_PressurePlateBroken;
	}
}

//*****************************************************************************
UINT CRoomWidget::GetTokenMID(const UINT param)
//Returns: messageID corresponding to token type
{
	switch (calcTokenType(param))
	{
		case RotateArrowsCW:
		  return bTokenActive(param) ? MID_TokenRotateCCW : MID_Token;
		case RotateArrowsCCW:
		  return bTokenActive(param) ? MID_Token : MID_TokenRotateCCW;
		case SwitchTarMud: return MID_TokenTarMud;
		case SwitchTarGel: return MID_TokenTarGel;
		case SwitchGelMud: return MID_TokenGelMud;
		case TarTranslucent: return MID_TokenTranslucentTar;
		case PowerTarget: return MID_TokenPowerTarget;
		case SwordDisarm: return MID_TokenSwordDisarm;
		case PersistentCitizenMovement: return MID_TokenCitizen;
		case ConquerToken: return MID_TokenConquer;
		default: ASSERT(!"Unexpected token type."); return 0;
	}
}

//*****************************************************************************
UINT CRoomWidget::SwitchAnimationFrame(
//Switch the monster animation frame at specified tile.
//
//Params:
	const UINT wCol, const UINT wRow)   //(in)   Room square.
{
	ASSERT(wCol < this->pRoom->wRoomCols);
	ASSERT(wRow < this->pRoom->wRoomRows);
	TILEINFO *const pTile = this->pTileInfo + this->pRoom->ARRAYINDEX(wCol,wRow);
	if (++pTile->animFrame >= ANIMATION_FRAMES) //increment frame
		pTile->animFrame = 0;
	return UINT(pTile->animFrame);
}

//
//Protected methods.
//

//*****************************************************************************
CRoomWidget::~CRoomWidget()
//Destructor.  Use Unload() for cleanup.
{
	ASSERT(!this->bIsLoaded);
	delete this->pLastLayerEffects;
	delete this->pMLayerEffects;
	delete this->pOLayerEffects;
	delete this->pTLayerEffects;

	if (this->pRoomSnapshotSurface)
	{
		SDL_FreeSurface(this->pRoomSnapshotSurface);
		this->pRoomSnapshotSurface = NULL;
	}
}

//*****************************************************************************
bool CRoomWidget::Load()
//Load resources for CRoomWidget.
//
//Returns:
//True if successful, false if not.
{
	if (!CWidget::Load())
		return false;

	static Uint32 TransparentColor = SDL_MapRGB(this->images[BOLTS_SURFACE]->format,
			TRANSPARENT_RGB);
	SDL_SetColorKey(this->images[BOLTS_SURFACE], SDL_SRCCOLORKEY, TransparentColor);

	if (this->pCurrentGame)
		if (!LoadFromCurrentGame(this->pCurrentGame))
			return false;

	return true;
}

//*****************************************************************************
void CRoomWidget::Unload()
//Unload resources for CRoomWidget.
{
	UnloadCurrentGame();

	CWidget::Unload();
}

//*****************************************************************************
void CRoomWidget::UnloadCurrentGame()
{
	this->pCurrentGame = NULL;
	this->pRoom = NULL;
	this->dwLastDrawSquareInfoUpdateCount = 0;

	//Force state reload.
	ClearEffects();
	DeleteArrays();
}

//*****************************************************************************
void CRoomWidget::DeleteArrays()
//Deallocate arrays.
{
	delete[] this->pwOSquareTI;
	this->pwOSquareTI = NULL;

	delete[] this->pwFSquareTI;
	this->pwFSquareTI = NULL;

	delete[] this->pwTSquareTI;
	this->pwTSquareTI = NULL;

//	delete[] this->pwMSquareTI;
//	this->pwMSquareTI = NULL;

	delete[] this->pbEdges;
	this->pbEdges = NULL;

	delete[] this->shadows;
	this->shadows = NULL;

	delete[] this->pTileInfo;
	this->pTileInfo = NULL;

	delete[] this->pwShadowTI;
	this->pwShadowTI = NULL;

	delete[] this->psTempLightBuffer;
	this->psTempLightBuffer = NULL;

	delete[] this->psRoomLight;
	this->psRoomLight = NULL;

	delete[] this->psCeilingLight;
	this->psCeilingLight = NULL;

	delete[] this->psPlayerLight;
	this->psPlayerLight = NULL;

	delete[] this->psDisplayedLight;
	this->psDisplayedLight = NULL;
}

//*****************************************************************************
void CRoomWidget::SetFrameVars(const bool bMoveMade)
//Update information that is used for rendering this frame.
{
	static bool bOpacityUp = false;

	//Stationary view.
	if (!this->bAnimateMoves)
	{
		this->dwMovementStepsLeft = 0;
		this->ghostOpacity = 255;
		bOpacityUp = false;
		return;
	}

	//Do player and monsters need to be repainted?
	if (bMoveMade)
	{
		if (this->dwMovementStepsLeft)
		{
			//The previous move was still being rendered -- skip it and begin new one now.
			this->bFinishLastMovementNow = true;
			//A complete refresh must occur to get everything to update correctly.
			//This makes the animation jerk.
			//To prevent it, call FinishMoveAnimation and Paint on this object
			//to force finishing the animation of the last move before this move is made.
			this->bAllDirty = true;
		}

		SetMoveCountText();

		//Ensure sleeping is stopped.
		StopSleeping();

		BetterVisionQuery();
	} else {
		//Handle graphic animations specific to between moves only.
		const UINT dwTimeToSleep = 21000;   //21s -- longer than EXIT_DELAY in GameScreen.cpp
		if (!this->bAllowSleep)
		{
			//This will keep from falling asleep right after bAllowSleep is set.
			if (this->dwCurrentDuration > this->dwMoveDuration)
				this->dwCurrentDuration = this->dwMoveDuration;
			AllowSleep(false);
		} else if (this->dwCurrentDuration >= dwTimeToSleep)
		{
			//Effect shows player asleep after no action for a while in peaceful rooms.
			if (!this->bPlayerSleeping)
			{
				ASSERT(this->pCurrentGame);
				CSwordsman& player = this->pCurrentGame->swordsman;

				this->bPlayerSleeping = true;
				CMoveCoord origin(player.wX, player.wY, N);
				AddMLayerEffect(new CFloatEffect(this, origin, TI_ZSLEEP, 4, 6));
				if (this->pParent && this->pParent->GetType() == WT_Screen)
				{
					CScreen *pScreen = DYN_CAST(CScreen*, CWidget*, this->pParent);
					ASSERT(pScreen);
					if (pScreen->GetScreenType() == SCR_Game)
					{
						CGameScreen *pGameScreen = DYN_CAST(CGameScreen*, CScreen*, pScreen);
						ASSERT(pGameScreen);
						pGameScreen->GetFaceWidget()->SetSleeping();
					}
				}
			}
			if (!g_pTheSound->IsSoundEffectPlaying(SEID_SNORING))
				g_pTheSound->PlaySoundEffect(SEID_SNORING);
		}
	}

	const Uint32 dwNow = SDL_GetTicks();

	//Slayer wisp animation.
	static const Uint32 dwWispAnimationRate = 100;	//ms
	static Uint32 dwLastWispAnimation = 0;
	if (dwNow - dwLastWispAnimation < dwWispAnimationRate)
		this->bNextWispFrame = false;
	else
	{
		this->bNextWispFrame = true;
		dwLastWispAnimation = dwNow;
	}

	//Update wall monster transparency.
	if (bOpacityUp)
	{
		this->ghostOpacity += 3;
		if (this->ghostOpacity > 235)
			bOpacityUp = false;
	} else {
		this->ghostOpacity -= 3;
		if (this->ghostOpacity < 35)
			bOpacityUp = true;
	}

	//Environmental effects.
	//These are mostly frame-based, not time-based, in order to look good
	//while keeping framerate high on older systems.
	const bool bEnvironmentalEffects = g_pTheBM->bAlpha && IsWeatherRendered();
	if (bEnvironmentalEffects) {
		SetFrameVarsForWeather();
	}
}

void CRoomWidget::SetFrameVarsForWeather()
{
	static const float fMaxVelocity = 1 / (float)CDrodBitmapManager::DISPLAY_ROWS;
	static const int HALF_ROWS = CDrodBitmapManager::DISPLAY_ROWS / 2;

	ASSERT(this->pCurrentGame);
	CSwordsman& player = this->pCurrentGame->swordsman;
	const bool bIsPlacingDouble = player.wPlacingDoubleType != 0;
	if (bIsPlacingDouble)
		return;

	const bool bPlayerIsAlive = !this->pCurrentGame->IsPlayerDying();

	const Uint32 dwNow = SDL_GetTicks();

	//1. Lightning.  It comes first because it might cause a full refresh,
	//which affects how other effects are rendered.
	if (!this->bLightning) {
		this->dwLightning = 0;
	} else {
		if (!this->dwLightning && bPlayerIsAlive)
		{
			//Randomly cause a lightning flash.
			if (rand() % 2500 <= 2) //about once every 20 seconds
			{
				this->dwLightning = dwNow;
				this->bAllDirty = true;
				const Uint32 time_of_thunder = dwNow + Uint32(1000 + fRAND_MID(250.0f)); //about a second later
				this->playThunder.push(time_of_thunder);
			}
		} else {
			//Stop lightning after a moment.
			static const Uint32 dwLightningDuration = 350;
			if (this->dwLightning + dwLightningDuration < dwNow || !bPlayerIsAlive)
			{
				this->dwLightning = 0;
				this->bAllDirty = true;
			} else {
				//Change lightning intensity occasionally while flash continues.
				if (RAND(8) == 0)
					this->bAllDirty = true;
			}
		}
	}

	//2. Sunlight through clouds is modeled based on either the movement of the open sky
	//or clouds being explicitly drawn.  The former is given priority, otherwise
	//the free-moving clouds model is used.
	if ((this->bSkyVisible || (this->bSunlight && !this->bClouds)))
	{
		//Update sky state periodically.
		if (!this->redrawingRowForWeather)
		{
			static const Uint32 MAX_SKY_FRAMES_PER_SECOND = 12;
			static const Uint32 MIN_TIME_BETWEEN_SKY_UPDATES = 1000/MAX_SKY_FRAMES_PER_SECOND; //ms
			if (dwNow >= this->time_of_last_sky_move + MIN_TIME_BETWEEN_SKY_UPDATES) {
				this->time_of_last_sky_move = dwNow;

				++this->dwSkyX;
				flag_weather_refresh();
			}
		}
	}

	//3. Fog.
	if (this->bFog)
	{
		//Update fog "wind" velocity.
		this->fFogVX += fRAND_MID(0.001f);
		this->fFogVY += fRAND_MID(0.001f);

		//Speed limit.
		if (this->fFogVX > fMaxVelocity) this->fFogVX = fMaxVelocity;
		else if (this->fFogVX < -fMaxVelocity) this->fFogVX = -fMaxVelocity;
		if (this->fFogVY > fMaxVelocity) this->fFogVY = fMaxVelocity;
		else if (this->fFogVY < -fMaxVelocity) this->fFogVY = -fMaxVelocity;

		this->fFogX += this->fFogVX;
		this->fFogY += this->fFogVY;

		if ((int)this->fFogX != (int)this->fFogOldX ||
				(int)this->fFogY != (int)this->fFogOldY)
		{
			//Fog has noticeably moved -- redraw.
			this->fFogOldX = this->fFogX;
			this->fFogOldY = this->fFogY;
			flag_weather_refresh();
		}
	}

	//4. Clouds and cloud shadows.
	if (this->bClouds || (this->bSunlight && !this->bSkyVisible))
	{
		//Clouds always move along at same speed.
		static const float fCloudVelocity = fMaxVelocity;

		//Update direction of wind.
		this->fCloudAngle += fRAND_MID(0.02f);

		this->fCloudX += cos(this->fCloudAngle) * fCloudVelocity;
		this->fCloudY += sin(this->fCloudAngle) * fCloudVelocity;

		if ((int)this->fCloudX != (int)this->fCloudOldX ||
				(int)this->fCloudY != (int)this->fCloudOldY)
		{
			//Clouds have noticeably moved -- redraw.
			this->fCloudOldX = this->fCloudX;
			this->fCloudOldY = this->fCloudY;
			flag_weather_refresh();
		}
	}

	if (this->bAllDirty)
	{
		//Entire room is being refreshed -- don't do anything here.
		this->redrawingRowForWeather = 0;
		this->need_to_update_room_weather = false;
		this->time_of_last_weather_render = dwNow;
	} else if (bPlayerIsAlive) {
		//Refresh weather periodically as requested.
		if (!this->redrawingRowForWeather && this->need_to_update_room_weather)  {
			static const Uint32 MIN_TIME_BETWEEN_WEATHER_RENDERS = 50; //ms
			if (dwNow >= this->time_of_last_weather_render + MIN_TIME_BETWEEN_WEATHER_RENDERS) {
				this->redrawingRowForWeather = CDrodBitmapManager::DISPLAY_ROWS;
				this->time_of_last_weather_render = dwNow;
				this->need_to_update_room_weather = false;
			}
		}

		//As environment slowly changes, update a bit of the room each frame to reflect changes.
		//This keeps the frame rate from slowing too much on older systems.
		if (this->redrawingRowForWeather) {
			int wRowsRedrawnPerFrame = 2 * g_pTheBM->eyeCandy;
			ASSERT(wRowsRedrawnPerFrame > 0);

			//When redrawing more than half of the room, it will take two updates to draw the whole room.
			//So, just draw half of the room each update for more perceived smoothness.
			if (wRowsRedrawnPerFrame > HALF_ROWS && wRowsRedrawnPerFrame < (int)CDrodBitmapManager::DISPLAY_ROWS) {
				wRowsRedrawnPerFrame = HALF_ROWS;
			}

			//draw top-down
			const UINT current_row = this->wShowRow + CDrodBitmapManager::DISPLAY_ROWS - this->redrawingRowForWeather;
			DirtyTileRect(this->wShowCol, current_row,
					CDrodBitmapManager::DISPLAY_COLS-1, current_row + wRowsRedrawnPerFrame - 1);

			if (this->bSkyVisible) {
				//Room tiles reflecting sky must be re-rendered
				RenderRoom(this->wShowCol, current_row,
						CDrodBitmapManager::DISPLAY_COLS, wRowsRedrawnPerFrame,
						false);
			}

			if (wRowsRedrawnPerFrame >= this->redrawingRowForWeather)
				this->redrawingRowForWeather = 0; //done
			else
				this->redrawingRowForWeather -= wRowsRedrawnPerFrame; //partial update -- do more next time
		}
	}
}

void CRoomWidget::flag_weather_refresh()
{
	this->need_to_update_room_weather = true;
}

//*****************************************************************************
void CRoomWidget::AddObstacleShadowMask(const UINT wCol, const UINT wRow)
//Add obstacle's tile shadow masks to set of pending masks.
{
	const BYTE tParam = this->pRoom->GetTParam(wCol, wRow);
	const BYTE obType = calcObstacleType(tParam);
	if (!obType || !bObstacleLeft(tParam) || !bObstacleTop(tParam))
		return; //this isn't the top-left corner of an obstacle -- do nothing

	//Each obstacle casts a shadow mask 1 tile larger than its size,
	//starting at its upper-left corner.
	UINT wObSizeIndex, wXPos, wYPos, wOTile;
	GetObstacleStats(this->pRoom, wCol, wRow, wObSizeIndex, wXPos, wYPos);
	ASSERT(wXPos == 0);
	ASSERT(wYPos == 0);
	const UINT wObIndex = obstacleIndices[obType][wObSizeIndex];
	ASSERT(wObIndex);
	const UINT wXDim = obstacleDimensions[wObIndex][0];
	const UINT wYDim = obstacleDimensions[wObIndex][1];
	ASSERT(wXDim);
	ASSERT(wYDim);

	const UINT wOTileOrigin = this->pRoom->GetOSquare(wCol, wRow);
	UINT wSX, wSY, x, y, wTileIndex;
	y = wRow;
	for (wSY=0; wSY<=wYDim; ++wSY, ++y)
	{
		x = wCol;
		wTileIndex = this->pRoom->ARRAYINDEX(x,y);
		for (wSX=0; wSX<=wXDim; ++wSX, ++x, ++wTileIndex)
		{
			//Shadow is cast if (x,y) is part of the current obstacle
			//(i.e. its NxN square) OR shows shadows.
			if (this->pRoom->IsValidColRow(x,y))
			{
				wOTile = this->pRoom->GetOSquare(x,y);
				if ((wSX<wXDim && wSY<wYDim) ||
					//if obstacle is placed on wall, it casts shadow everywhere
					bObstacleCastsShadowFrom(wOTileOrigin,wOTile))
				{
					this->shadows[wTileIndex].push_back(
							obstacleShadowTile[wObIndex][wSX][wSY]);
					this->pTileInfo[wTileIndex].dirty = 1;
				}
			}
		}
	}
}

//*****************************************************************************
void CRoomWidget::BoundsCheckRect(
//Performs bounds checking on a room rectangle.
//
//Params:
	int &wCol, int &wRow,         //(in/out)  Top-left tile.
	int &wWidth, int &wHeight)    //(in/out)  dimensions, in tiles
const
{
	ASSERT(wWidth > 0 && wHeight > 0);  //avoid wasteful calls
	ASSERT(wCol + wWidth > 0 && wRow + wHeight > 0);
	//Room dimensions.
	const int wRW = this->pRoom->wRoomCols;
	const int wRH = this->pRoom->wRoomRows;
	ASSERT(wCol < wRW && wRow < wRH);

	//Perform bounds checking.
	if (wCol < 0) {wWidth += wCol; wCol = 0;}
	if (wRow < 0) {wHeight += wRow; wRow = 0;}
	if (wCol + wWidth >= wRW) {wWidth = wRW - wCol;}
	if (wRow + wHeight >= wRH) {wHeight = wRH - wRow;}
}

//*****************************************************************************
void CRoomWidget::BAndWRect(
//Sets a rectangle of tiles in room to black-and-white.
//
//Params:
	SDL_Surface *pDestSurface, //(in)   Surface to draw to.
	int wCol, int wRow,        //(in)   Top-left tile.
	int wWidth, int wHeight)   //(in)   dimensions, in tiles
{
	BoundsCheckRect(wCol,wRow,wWidth,wHeight);

	g_pTheBM->BAndWRect(this->x + wCol * CX_TILE,
			this->y + wRow * CY_TILE,
			wWidth * CX_TILE, wHeight * CY_TILE,
			pDestSurface);
}
//*****************************************************************************
void CRoomWidget::DarkenRect(
//Darkens a rectangle of tiles in room by given percent.
//
//Params:
	SDL_Surface *pDestSurface, //(in)   Surface to draw to.
	const float fLightPercent, //(in)   % of color value to retain
	int wCol, int wRow,        //(in)   Top-left tile.
	int wWidth, int wHeight)   //(in)   dimensions, in tiles
{
	BoundsCheckRect(wCol,wRow,wWidth,wHeight);

	g_pTheBM->DarkenRect(this->x + wCol * CX_TILE, this->y + wRow * CY_TILE,
			wWidth * CX_TILE, wHeight * CY_TILE, fLightPercent, pDestSurface);
}

//*****************************************************************************
void CRoomWidget::ShadeRect(
//Shades a rectangle of tiles in room with given color.
//
//Params:
	SDL_Surface *pDestSurface, //(in)   Surface to draw to.
	const SURFACECOLOR &Color, //(in)   Color to shade with
	int wCol, int wRow,        //(in)   Top-left tile.
	int wWidth, int wHeight)   //(in)   dimensions, in tiles
{
	BoundsCheckRect(wCol,wRow,wWidth,wHeight);

	g_pTheBM->ShadeRect(this->x + wCol * CX_TILE, this->y + wRow * CY_TILE,
			wWidth * CX_TILE, wHeight * CY_TILE, Color, pDestSurface);
}

//*****************************************************************************
void CRoomWidget::DirtyTileRect(
//Mark all tiles between these two points as dirty.
//
//Params:
	const int x1, const int y1, const int x2, const int y2)  //(in)
{
	//Find min/max coords.
	int xMin = min(x1,x2);
	int xMax = max(x1,x2);
	int yMin = min(y1,y2);
	int yMax = max(y1,y2);

	//Only dirty tiles in visible room area.
	if (xMin < 0) xMin = 0;
	if (yMin < 0) yMin = 0;
	if (xMax >= static_cast<int>(CDrodBitmapManager::DISPLAY_COLS)) xMax = CDrodBitmapManager::DISPLAY_COLS-1;
	if (yMax >= static_cast<int>(CDrodBitmapManager::DISPLAY_ROWS)) yMax = CDrodBitmapManager::DISPLAY_ROWS-1;

	this->pLastLayerEffects->DirtyTilesInRect(xMin,yMin,xMax,yMax);
}

//*****************************************************************************
void CRoomWidget::DrawDamagedMonsters(
//Paint monsters whose tiles have been repainted.
//Also check whether monster is raised up and tile above is dirty.
//This signifies the monster's image has been clipped on top
//and must be repainted.
//
//Params:
	SDL_Surface *pDestSurface, const bool bDrawGhosts)
{
	ASSERT(!this->bAllDirty);

	TILEINFO *pbMI;
	UINT wSX, wSY;
	vector<CMonster*> drawnMonsters;
	for (CMonster *pMonster = this->pRoom->pFirstMonster; pMonster != NULL; pMonster = pMonster->pNext)
	{
		//If bDrawGhosts is on, then we only draw Ghost Image characters
		//If bDrawGhosts is off, then we ignore Ghost Image characters
		bool bGhost = false;
		if (pMonster->wType == M_CHARACTER)
		{
			CCharacter *pCharacter = DYN_CAST(CCharacter*, CMonster*, pMonster);
			bGhost = pCharacter->IsGhostImage() && !pCharacter->IsVisible();
		}
		if (bGhost != bDrawGhosts)
			continue;

		bool bDrawMonster = false;

		if ((pMonster->wType == M_SLAYER || pMonster->wType == M_SLAYER2) &&
				this->pCurrentGame) //not in room editor
		{
			//Redraw Slayer's seeker wisp each turn
			bDrawMonster = true;
		} else if (bIsSerpent(pMonster->wType)) {
			//Redraw dirty serpent pieces.  The head only will be redrawn below as applicable.
			for (list<CMonsterPiece*>::iterator piece = pMonster->Pieces.begin();
					piece != pMonster->Pieces.end(); ++piece)
			{
				ASSERT(this->pRoom->IsValidColRow((*piece)->wX, (*piece)->wY));
				pbMI = this->pTileInfo + this->pRoom->ARRAYINDEX((*piece)->wX, (*piece)->wY);
				if (pbMI->dirty || pbMI->damaged)
				{
					//Copied from DrawSerpent().
					DrawTileImage((*piece)->wX, (*piece)->wY, 0, 0,
							GetTileImageForSerpentPiece(pMonster->wType, (*piece)->wTileNo),
							false, pDestSurface, false);
				}
			}
		} else if (pMonster->IsLongMonster()) {
			//If any pieces of a large monster are dirty, redraw the entire monster.
			for (list<CMonsterPiece*>::iterator piece = pMonster->Pieces.begin();
					piece != pMonster->Pieces.end(); ++piece)
			{
				ASSERT(this->pRoom->IsValidColRow((*piece)->wX, (*piece)->wY));
				pbMI = this->pTileInfo + this->pRoom->ARRAYINDEX((*piece)->wX, (*piece)->wY);
				if (pbMI->dirty || pbMI->damaged)
				{
					bDrawMonster = true;
					break;
				}
			}
		}
		//Check whether monster's sword square is dirty also.
		else if (pMonster->GetSwordCoords(wSX, wSY)) {
			if (IS_COLROW_IN_DISP(wSX, wSY))
			{
				ASSERT(this->pRoom->IsValidColRow(wSX, wSY));
				pbMI = this->pTileInfo + this->pRoom->ARRAYINDEX(wSX, wSY);
				if (pbMI->dirty || pbMI->damaged) {
					bDrawMonster = true;
				} else if (wSY > this->wShowRow && DrawRaised(this->pRoom->GetOSquare(pMonster->wX, pMonster->wY)))
				{
					//If it's raised and the monster above it is being redrawn, then it must be too.
					pbMI = this->pTileInfo + this->pRoom->ARRAYINDEX(wSX, wSY-1);
					if (pbMI->dirty || pbMI->damaged)
						bDrawMonster = true;
				}
			}
		}

		if (!bDrawMonster)
		{
			ASSERT(this->pRoom->IsValidColRow(pMonster->wX, pMonster->wY));
			pbMI = this->pTileInfo + this->pRoom->ARRAYINDEX(pMonster->wX, pMonster->wY);
			if (pbMI->dirty || pbMI->damaged) {
				bDrawMonster = true;
			} else if (pMonster->wY > this->wShowRow && DrawRaised(this->pRoom->GetOSquare(pMonster->wX, pMonster->wY)))
			{
				//If it's raised and the monster above it is being redrawn, then it must be too.
				pbMI = this->pTileInfo + this->pRoom->ARRAYINDEX(pMonster->wX, pMonster->wY-1);
				if (pbMI->dirty || pbMI->damaged)
					bDrawMonster = true;
			}
		}

		if (bDrawMonster)
		{
			DrawMonster(pMonster, this->pRoom, pDestSurface, false, false, false);
			drawnMonsters.push_back(pMonster);
		}
	}

	DrawSwordsFor(drawnMonsters, pDestSurface);
}

void CRoomWidget::DrawDamagedMonsterSwords(SDL_Surface *pDestSurface)
{
	for (CMonster *pMonster = this->pRoom->pFirstMonster; pMonster != NULL; pMonster = pMonster->pNext)
	{
		UINT wSX, wSY;
		if (DrawingSwordFor(pMonster) && pMonster->GetSwordCoords(wSX, wSY)) {
			if (IS_COLROW_IN_DISP(wSX, wSY))
			{
				ASSERT(this->pRoom->IsValidColRow(wSX, wSY));
				TILEINFO *pbMI = this->pTileInfo + this->pRoom->ARRAYINDEX(wSX, wSY);
				if (pbMI->dirty || pbMI->damaged) {
					DrawSwordFor(pMonster, pMonster->GetIdentity(), wSX, wSY,
							0, 0, DrawRaised(this->pRoom->GetOSquare(pMonster->wX, pMonster->wY)),
							pDestSurface, false, 255);
				}
			}
		}
	}
}

//*****************************************************************************
void CRoomWidget::DrawPlatforms(
//Draws platforms in the room.
	SDL_Surface *pDestSurface,
	const bool bEditor,           //(in) [default=false]
	const bool bMoveInProgress)   //(in) [default=false]
{
	ASSERT(pDestSurface);

	const float fLightLevel = fRoomLightLevel[this->wDark];
	const bool bAddLight = IsLightingRendered();

	CCoordSet tilesDrawn;

	//Render each platform.
	ASSERT(this->pRoom);
	for (vector<CPlatform*>::const_iterator platformIter = this->pRoom->platforms.begin();
			platformIter != this->pRoom->platforms.end(); ++platformIter)
	{
		const CPlatform& platform = *(*platformIter);
		const bool bPlatformMoved = platform.xDelta || platform.yDelta;
		const bool bPlatformAnimating = bMoveInProgress && bPlatformMoved;
		const bool bRedrawAll = this->bAllDirty || bPlatformAnimating || bEditor;

		//Calculate animation offset (in pixels).
		UINT wXOffset = 0, wYOffset = 0;
		if (this->dwMovementStepsLeft)
		{
			wXOffset = -platform.xDelta * this->dwMovementStepsLeft;
			wYOffset = -platform.yDelta * this->dwMovementStepsLeft;
		}

		CCoordSet tiles;
		platform.GetTiles(tiles);

		//Redraw tiles needing to be repainted.
		for (CCoordSet::const_iterator tileIter = tiles.begin();
				tileIter != tiles.end(); ++tileIter)
		{
			UINT wX = tileIter->wX, wY = tileIter->wY;
			TILEINFO& tileInfo = this->pTileInfo[this->pRoom->ARRAYINDEX(wX,wY)];
			if (bRedrawAll || tileInfo.damaged)
			{
				const UINT oTile = pRoom->GetOSquare(wX, wY);
				UINT wTileImageNo = GetTileImageForTileNo(oTile);
				if (wTileImageNo == CALC_NEEDED)
				{
					wTileImageNo = CalcTileImageFor(this->pRoom, oTile, wX, wY);
					DrawTileImage(wX, wY, wXOffset, wYOffset,
							wTileImageNo, false, pDestSurface,
							bPlatformAnimating); // || wXOffset || wYOffset); -- only needed if platform has jitter independent of movement animation
					tilesDrawn.insert(wX,wY);
					tilesDrawn.insert(wX - platform.xDelta, wY - platform.yDelta); //tiles being moved off of need to have items redrawn too
				}
				//otherwise: we're cutting-and-pasting this platform in the editor, so nothing special should be drawn at present
			}
		}
	}

 	//Re-render room items located on top of each platform tile.
	RenderRoomItemsOnTiles(tilesDrawn, pDestSurface, fLightLevel, bAddLight, bEditor);
}

//*****************************************************************************
void CRoomWidget::RenderRoomItemsOnTiles(
	const CCoordSet& tiles, SDL_Surface *pDestSurface,
	const float fLightLevel, const bool bAddLight, const bool bEditor)
{
	for (CCoordSet::const_iterator tileIter = tiles.begin();
			tileIter != tiles.end(); ++tileIter)
	{
		UINT wX = tileIter->wX, wY = tileIter->wY;

		//Get tile-specific info.
		UINT wOTileNo = this->pRoom->GetOSquare(wX, wY);
		const bool bWater = bIsWater(wOTileNo) || wOTileNo == T_PLATFORM_W || bIsSteppingStone(wOTileNo);
		const bool bAddLightLayers = bWater; //over water
		const bool bPitPlatformTiles = !bAddLightLayers;

		//Determine this tile's darkness.
		float fDark = fLightLevel;
		const UINT darkVal = this->pRoom->tileLights.GetAt(wX, wY);
		if (bIsDarkTileValue(darkVal))
		{
			ASSERT(darkVal - LIGHT_OFF - 1 < NUM_DARK_TYPES);
			fDark *= darkMap[darkVal - LIGHT_OFF - 1];
		}

		int nX = this->x + wX * CX_TILE;
		int nY = this->y + wY * CY_TILE;
		const UINT tileIndex = this->pRoom->ARRAYINDEX(wX, wY);
		UINT wFTI = this->pwFSquareTI[tileIndex];
		UINT wTTI = this->pwTSquareTI[tileIndex];
		vector<UINT> *pShadows = this->shadows + tileIndex;

		LIGHTTYPE *psL = this->psDisplayedLight + tileIndex * wLightValuesPerTile;

		RenderRoomTileObjects(wX, wY, nX, nY, pDestSurface,
				wOTileNo, wFTI, wTTI, pShadows, psL,
				fDark, bAddLight, bAddLightLayers, bEditor, bPitPlatformTiles);
		this->pTileInfo[tileIndex].dirty = 1;
	}
}

//*****************************************************************************
void CRoomWidget::DrawMonsters(
//Draws monsters not on the player -- this case is handled separately.
//
//Params:
	CMonster *const pMonsterList, //(in)   Monsters to draw.
	SDL_Surface *pDestSurface,
	const bool bActionIsFrozen,   //(in)   Whether action is currently stopped.
	const bool bMoveInProgress)   //(in)   [default=false]
{
	CMonster *pMonster;

	//Draw "ghost" NPCs first.
	for (pMonster = pMonsterList; pMonster; pMonster = pMonster->pNext)
	{
		if (pMonster->wType == M_CHARACTER)
		{
			CCharacter *pCharacter = DYN_CAST(CCharacter*, CMonster*, pMonster);
			if (pCharacter->IsGhostImage() && !pCharacter->IsVisible())
				DrawMonster(pMonster, this->pRoom, pDestSurface, bActionIsFrozen, bMoveInProgress);
		}
	}

	//Get player info.
	UINT wPX = UINT(-1), wPY = UINT(-1);
	bool bPlayerIsDying = false;
	if (this->pCurrentGame)
	{
		bPlayerIsDying = this->pCurrentGame->IsPlayerDying();
		this->pCurrentGame->GetSwordsman(wPX, wPY);
	}

	vector<CMonster*> drawnMonsters;
	for (pMonster = pMonsterList; pMonster; pMonster = pMonster->pNext)
	{
		//Don't draw ghost images a second time
		if (pMonster->wType == M_CHARACTER)
		{
			CCharacter *pCharacter = DYN_CAST(CCharacter*, CMonster*, pMonster);
			if (pCharacter->IsGhostImage() && !pCharacter->IsVisible())
				continue;
		}

		//Draw monster if it is not involved in a death sequence.
		if (!bPlayerIsDying || (!IsMonsterInvolvedInDeath(pMonster) && pMonster->bAlive))
		{
			DrawMonster(pMonster, this->pRoom, pDestSurface, bActionIsFrozen, bMoveInProgress);
			drawnMonsters.push_back(pMonster);
		}
	}

	//Redraw monster swords to ensure no other monster is being drawn on top of them.
	if (!(bMoveInProgress || this->dwMovementStepsLeft))
	{
		DrawSwordsFor(drawnMonsters, pDestSurface);
	}
}

void CRoomWidget::DrawSwordsFor(const vector<CMonster*>& drawnMonsters, SDL_Surface *pDestSurface)
{
	for (vector<CMonster*>::const_iterator monster=drawnMonsters.begin();
		monster != drawnMonsters.end(); ++monster)
	{
		CMonster *pMonster = *monster;
		UINT wSX, wSY;
		if (pMonster->GetSwordCoords(wSX, wSY)) {
			const bool draw_raised = DrawRaised(this->pRoom->GetOSquare(pMonster->wX, pMonster->wY));
			DrawSwordFor(pMonster, pMonster->GetIdentity(), wSX, wSY,
					0, 0, draw_raised,
					pDestSurface, false, 255);
		}
	}
}

//*****************************************************************************
void CRoomWidget::AnimateMonsters()
//Randomly change monsters' animation frame.
{
	if (!this->bAnimateMoves) return;

	//Animate monsters in real time.
	const Uint32 dwNow=SDL_GetTicks();
	Uint32 dwTimeElapsed = dwNow - this->dwLastAnimationFrame;
	if (dwTimeElapsed==0)
		dwTimeElapsed=1;
	const Uint32 dwRandScalar=MONSTER_ANIMATION_DELAY * 1000 / dwTimeElapsed;

	const bool bAlpha = g_pTheBM->bAlpha;

	UINT wSX, wSY;
	for (CMonster *pMonster = this->pRoom->pFirstMonster; pMonster != NULL;
			pMonster = pMonster->pNext)
	{
		if (RAND(dwRandScalar) == 0)
		{
			SwitchAnimationFrame(pMonster->wX, pMonster->wY);
			this->pTileInfo[this->pRoom->ARRAYINDEX(pMonster->wX, pMonster->wY)].dirty = 1;
			//Redraw all changing parts of large monsters.
			if (!bIsSerpent(pMonster->wType) && pMonster->IsLongMonster())
			{
				for (list<CMonsterPiece*>::const_iterator piece = pMonster->Pieces.begin();
						piece != pMonster->Pieces.end(); ++piece)
					this->pTileInfo[this->pRoom->ARRAYINDEX((*piece)->wX, (*piece)->wY)].dirty = 1;
			}
		}
		//Redraw transparent monsters each frame.
		else if (this->pCurrentGame &&
				((pMonster->eMovement == WALL && bAlpha) || pMonster->wType == M_SPIDER))
			this->pTileInfo[this->pRoom->ARRAYINDEX(pMonster->wX, pMonster->wY)].dirty = 1;

		//Always redraw all changing parts of large and sworded monsters in fog.
		if (this->cFogLayer > 1)
		{
			if (pMonster->IsLongMonster() && !bIsSerpent(pMonster->wType))
			{
				this->pTileInfo[this->pRoom->ARRAYINDEX(pMonster->wX, pMonster->wY)].dirty = 1;
				for (list<CMonsterPiece*>::const_iterator piece = pMonster->Pieces.begin();
						piece != pMonster->Pieces.end(); ++piece)
					this->pTileInfo[this->pRoom->ARRAYINDEX((*piece)->wX, (*piece)->wY)].dirty = 1;
			}
			if (pMonster->GetSwordCoords(wSX, wSY) && this->pRoom->IsValidColRow(wSX, wSY))
			{
				this->pTileInfo[this->pRoom->ARRAYINDEX(pMonster->wX, pMonster->wY)].dirty = 1;
				this->pTileInfo[this->pRoom->ARRAYINDEX(wSX, wSY)].dirty = 1;
			}
		}
	}

	//Animate non-Beethro player role.
	if (this->pCurrentGame && RAND(dwRandScalar) == 0)
	{
		CSwordsman& player = this->pCurrentGame->swordsman;
		if (!bIsSmitemaster(player.wAppearance))
		{
			SwitchAnimationFrame(player.wX, player.wY);
			this->pTileInfo[this->pRoom->ARRAYINDEX(player.wX, player.wY)].dirty = 1;
		}
	}

	this->dwLastAnimationFrame=dwNow;
}

//*****************************************************************************
void CRoomWidget::BetterVisionQuery()
//Display visual effects for vision power up.
{
	//Evil eye gazes are seen.
	this->bRequestEvilEyeGaze = this->pCurrentGame->pRoom->bBetterVision;

	//Remove old persistent evil eye gazes.
	//Note that normal wake up gaze effects are displayed on the m-layer,
	//not the last layer like these.
	this->pLastLayerEffects->RemoveEffectsOfType(EEVILEYEGAZE);
}

//*****************************************************************************
void CRoomWidget::DrawMonster(
//Draws a monster.
//
//Params:
	const CMonster* pMonster,   //(in) Monster to draw.
	CDbRoom *const pRoom,
	SDL_Surface *pDestSurface,
	const bool bActionIsFrozen, //(in) Whether action is currently stopped.
	const bool bMoveInProgress, //(in) Whether movement animation is not yet completed [default=true]
	const bool bDrawPieces)     //(in) Draw monster pieces [default=true]
{
	bool bDrawRaised =
			DrawRaised(pRoom->GetOSquare(pMonster->wX, pMonster->wY));
	Uint8 opacity = 255;
	switch (pMonster->wType)
	{
		case M_CITIZEN:
			DrawCitizen(DYN_CAST(const CCitizen*, const CMonster*, pMonster), bDrawRaised, pDestSurface, bMoveInProgress);
			break;
		case M_SLAYER:
		case M_SLAYER2:
			DrawSlayerWisp(DYN_CAST(const CPlayerDouble*, const CMonster*, pMonster), pDestSurface);
			//FALL-THROUGH
		case M_MIMIC: case M_CLONE: case M_DECOY:
		case M_GUARD: case M_STALWART: case M_STALWART2:
			DrawDouble(DYN_CAST(const CPlayerDouble*, const CMonster*, pMonster), bDrawRaised, pDestSurface, bMoveInProgress);
			break;
		case M_CHARACTER:
			DrawCharacter(DYN_CAST(const CCharacter*, const CMonster*, pMonster), bDrawRaised, pDestSurface, bMoveInProgress);
			break;
		case M_ROCKGIANT:
			DrawRockGiant(pMonster, bDrawRaised, pDestSurface, bMoveInProgress);
			break;
		case M_SERPENT:
		case M_SERPENTG:
		case M_SERPENTB:
			bDrawRaised = false;
			if (bDrawPieces)
				DrawSerpent(pMonster, pDestSurface, bMoveInProgress);
			//no break
		default:
		{
			const bool bWading = bIsWadingMonsterType(pMonster->wType) && pMonster->IsWading();
			const UINT wFrame = bWading ? (UINT)WADING_FRAME :
				this->pTileInfo[this->pRoom->ARRAYINDEX(pMonster->wX, pMonster->wY)].animFrame;

			//Calculate animation offset for everything except serpent heads.
			UINT wXOffset = 0, wYOffset = 0;
			if (this->dwMovementStepsLeft && !bIsSerpent(pMonster->wType))
			{
				wXOffset = (pMonster->wPrevX - pMonster->wX) * this->dwMovementStepsLeft;
				wYOffset = (pMonster->wPrevY - pMonster->wY) * this->dwMovementStepsLeft;
			}
			UINT wTileImageNo = GetTileImageForEntity(pMonster->wType, pMonster->wO, wFrame);

			int nAddColor = NO_COLOR_INDEX;
			if (pMonster->eMovement == WALL)
			{
				//Monsters in walls are not raised, and are transparent.
				bDrawRaised = false;
				if (!bActionIsFrozen) //only draw transparent when play is in progress
					opacity = this->ghostOpacity;
			}
			else if (bIsMother(pMonster->wType))
				bDrawRaised = false; //tarstuff is never raised
			else switch (pMonster->wType)
			{
				case M_SPIDER:
					if (this->pCurrentGame && !this->pCurrentGame->pRoom->bBetterVision)
					{
						//spiders are about 35% opaque, more when adjacent to player
						const UINT wDistance = pMonster->DistToSwordsman(false);
						switch (wDistance)
						{
							case 0: case 1: opacity = 190; break;
							case 2: opacity = 145; break;
							case 3: opacity = 135; break;
							case 4: opacity = 125; break;
							case 5: opacity = 110; break;
							default: opacity = 95; break;
						}
					}
				break;
				case M_EYE:
				{
					//Draw active eyes differently.  (Only one animation frame currently supported.)
					const CEvilEye *pEye = DYN_CAST(const CEvilEye*, const CMonster*, pMonster);
					if (pEye->IsAggressive())
						wTileImageNo = GetTileImageForEntity(M_EYE_ACTIVE, pMonster->wO, 0);
					else if (this->bRequestEvilEyeGaze) {
						//Show eye beams with vision power-up.
						AddLastLayerEffect(new CEvilEyeGazeEffect(this,pMonster->wX,
								pMonster->wY,pMonster->wO, 0));
					}
				}
				break;
				case M_AUMTLICH:
				{
					const CZombie *pAumtlich = DYN_CAST(const CZombie*, const CMonster*, pMonster);
					if (pAumtlich->bFrozen)
						nAddColor = FROZEN_COLOR_INDEX;
				}
				break;
			}

			DrawTileImage(pMonster->wX, pMonster->wY, wXOffset, wYOffset,
					wTileImageNo, bDrawRaised, pDestSurface, bMoveInProgress, opacity, false, nAddColor);
		}
		break;
	}
}

//*****************************************************************************
bool CRoomWidget::IsMonsterInvolvedInDeath(CMonster *pMonster) const
//Is this monster either killing the player or a critical character,
//or is a critical character being killed?
{
	ASSERT(pMonster);

	//Did this monster collide with the player?
	UINT wPX, wPY;
	this->pCurrentGame->GetSwordsman(wPX, wPY);
	if (pMonster->wX == wPX && pMonster->wY == wPY)
		return true;
	if (pMonster->IsLongMonster())
	{
		//Did a piece of the monster hit the player?
		CMonster *pMonsterOnPlayer = this->pRoom->GetMonsterAtSquare(wPX, wPY);
		if (pMonsterOnPlayer)
		{
			if (pMonsterOnPlayer->IsPiece())
			{
				CMonsterPiece *pPiece = DYN_CAST(CMonsterPiece*, CMonster*, pMonsterOnPlayer);
				pMonsterOnPlayer = pPiece->pMonster;
			}
			if (pMonster == pMonsterOnPlayer)
				return true;
		}
	}

	UINT wSX, wSY;
	switch (pMonster->wType)
	{
		case M_CLONE:
		case M_HALPH: case M_HALPH2:
			//Is critical character being killed (not falling into pit)?
			if (!pMonster->IsAlive() && !bIsPit(this->pRoom->GetOSquare(pMonster->wX, pMonster->wY)))
				return true;
		break;
		case M_CHARACTER:
		{
			CCharacter *pCharacter = DYN_CAST(CCharacter*, CMonster*, pMonster);
			if (!pCharacter->IsVisible())
				break;
			//Is critical character being killed?
			switch (pCharacter->GetImperative())
			{
				case ScriptFlag::MissionCritical:
					if (!pCharacter->IsAlive())
						return true;
				break;
				default:
					//Is sworded character killing player or a critical character?
					if (pCharacter->GetSwordCoords(wSX,wSY))
					{
						if ((wSX == wPX && wSY == wPY) ||
								this->pRoom->IsMonsterOfTypeAt(M_HALPH, wSX, wSY, true, false) ||
								this->pRoom->IsMonsterOfTypeAt(M_HALPH2, wSX, wSY, true, false) ||
								this->pRoom->IsMonsterOfTypeAt(M_CLONE, wSX, wSY, true, false))
							return true;
					}
				break;
			}
		}
		break;
		default:
			//Is sworded monster killing player or critical character?
			if (pMonster->GetSwordCoords(wSX,wSY))
			{
				if ((wSX == wPX && wSY == wPY) ||
						this->pRoom->IsMonsterOfTypeAt(M_HALPH, wSX, wSY, true, false) ||
						this->pRoom->IsMonsterOfTypeAt(M_HALPH2, wSX, wSY, true, false) ||
						this->pRoom->IsMonsterOfTypeAt(M_CLONE, wSX, wSY, true, false))
					return true;
			}
		break;
	}
	return false;
}

//*****************************************************************************
void CRoomWidget::DrawMonsterKillingPlayer(SDL_Surface *pDestSurface)
//Draw one or more monsters killing player or critical character.
{
	ASSERT(pDestSurface);

	//If player is dead, draw monster that made the kill.
	if (this->pCurrentGame->swordsman.IsInRoom())
	{
		DrawMonsterKillingAt(&this->pCurrentGame->swordsman, pDestSurface);
	}

	for (CMonster *pSeek = this->pRoom->pFirstMonster; pSeek != NULL; pSeek = pSeek->pNext)
	{
		//Is critical character being killed?
		bool bCritical = false;
		switch (pSeek->wType)
		{
			case M_CLONE:
			case M_HALPH: case M_HALPH2:
				bCritical = true;
			break;
			case M_CHARACTER:
			{
				CCharacter *pCharacter = DYN_CAST(CCharacter*, CMonster*, pSeek);
				bCritical = pCharacter->IsVisible() && pCharacter->GetImperative() == ScriptFlag::MissionCritical;
			}
			break;
		}
		//Draw critical character if dead and not falling into pit, plus whatever killed it.
		if (bCritical && !pSeek->IsAlive() && !bIsPit(this->pRoom->GetOSquare(pSeek->wX, pSeek->wY)))
		{
			DrawMonster(pSeek, this->pRoom, pDestSurface, true);
			DrawMonsterKilling(pSeek, pDestSurface);
		}
	}
}

//*****************************************************************************
void CRoomWidget::DrawMonsterKilling(
	CCoord *pCoord,
	SDL_Surface *pDestSurface) //(in) Surface to draw on
{
	const UINT wX = pCoord->wX;
	const UINT wY = pCoord->wY;
	for (int dx=-1; dx<=1; ++dx)
		for (int dy=-1; dy<=1; ++dy)
			if ((dx || dy) && this->pRoom->IsValidColRow(wX+dx,wY+dy))
			{
				CMonster *pMonster = this->pRoom->GetMonsterAtSquare(wX+dx,wY+dy);
				if (pMonster && pMonster->HasSwordAt(wX,wY))
				{
					DrawMonster(pMonster, this->pRoom, pDestSurface, true);
					return;
				}
			}
}

//*****************************************************************************
void CRoomWidget::DrawMonsterKillingAt(
//Draw the monster which killed player or critical character.
//Params:
	CCoord *pCoord, //(in) coordinates of dying character
	SDL_Surface *pDestSurface) //(in) Surface to draw on
{
	CMonster *pMonster = this->pRoom->GetMonsterAtSquare(pCoord->wX, pCoord->wY);
	if (pMonster)
	{
		if (pMonster->IsPiece())
		{
			const CMonsterPiece *pPiece = DYN_CAST(const CMonsterPiece*, CMonster*, pMonster);
			pMonster = pPiece->pMonster;
		}
		DrawMonster(pMonster, this->pRoom, pDestSurface, true);
		return;
	}

	DrawMonsterKilling(pCoord, pDestSurface);
}

//*****************************************************************************
void CRoomWidget::DrawDoorFiller(SDL_Surface *pDestSurface, const UINT wX, const UINT wY)
//In cases where doors are 2x2 tiles or thicker, draw filler to
//remove the dimple on the edge.
{
	const UINT oTile = this->pRoom->GetOSquare(wX,wY);
	UINT wFillerTI;
	switch (oTile)
	{
		case T_DOOR_Y: wFillerTI = TI_DOOR_YC; break;
		case T_DOOR_M: wFillerTI = TI_DOOR_GC; break;
		case T_DOOR_C: wFillerTI = TI_DOOR_CC; break;
		case T_DOOR_R: wFillerTI = TI_DOOR_RC; break;
		case T_DOOR_B: wFillerTI = TI_DOOR_BC; break;
		default: return; //no filler for this door type
	}

	bool adj[3][3] = {{false}};

	//mark where adjacent door tiles are
	const UINT endX = wX+2, endY = wY+2;
	UINT x, y;
	for (y=wY-1; y!=endY; ++y)
	{
		if (y >= this->pRoom->wRoomRows)
			continue;
		const int dy = y-wY+1;
		for (x=wX-1; x!=endX; ++x)
		{
			if (x >= this->pRoom->wRoomCols)
				continue;
			const int dx = x-wX+1;
			if (dx == 1 && dy == 1)
				continue;
			adj[dx][dy] = (this->pRoom->GetOSquare(x,y) == oTile);
		}
	}

	int nBaseX = this->x + wX * CX_TILE;
	int nBaseY = this->y + wY * CY_TILE;
	const UINT wHalfTileX = CDrodBitmapManager::CX_TILE/2;
	const UINT wHalfTileY = CDrodBitmapManager::CY_TILE/2;

	if (adj[0][0] && adj[0][1] && adj[1][0]) //upper-left corner
	{
		g_pTheDBM->BlitTileImagePart(wFillerTI,
				nBaseX, nBaseY, wHalfTileX, wHalfTileY,
				wHalfTileX, wHalfTileY, pDestSurface);
	}
	if (adj[0][2] && adj[0][1] && adj[1][2]) //lower-left corner
	{
		g_pTheDBM->BlitTileImagePart(wFillerTI,
				nBaseX, nBaseY + wHalfTileY, wHalfTileX, 0,
				wHalfTileX, wHalfTileY, pDestSurface);
	}
	if (adj[2][0] && adj[2][1] && adj[1][0]) //upper-right corner
	{
		g_pTheDBM->BlitTileImagePart(wFillerTI,
				nBaseX + wHalfTileX, nBaseY, 0, wHalfTileY,
				wHalfTileX, wHalfTileY, pDestSurface);
	}
	if (adj[2][2] && adj[2][1] && adj[1][2]) //lower-right corner
	{
		g_pTheDBM->BlitTileImagePart(wFillerTI,
				nBaseX + wHalfTileX, nBaseY + wHalfTileY, 0, 0,
				wHalfTileX, wHalfTileY, pDestSurface);
	}
}

//*****************************************************************************
void CRoomWidget::DrawTileImage(
//Blits a tile graphic to a specified room position.
//Dirties tiles covered by this blit.
//
//Params:
		const UINT wCol, const UINT wRow,   //(in)   Destination square coords.
		const UINT wXOffset, const UINT wYOffset, //(in)   Animation offset.
		const UINT wTileImageNo,         //(in)   Indicates which tile image to blit.
		const bool bDrawRaised,       //(in)   Draw tile raised above surface?
		SDL_Surface *pDestSurface,    //(in)   Surface to draw to.
		const bool bDirtyTiles, //(in)   Whether tiles should be dirtied
		const Uint8 nOpacity,   //(in)   Tile opacity (0 = transparent, 255 = opaque).
		bool bClipped,    //(in)   Tile image is not fully on screen and must be clipped [default=false]
		const int nAddColor) // Palette color to add to tile [default=-1 (none)]
{
	ASSERT(IS_COLROW_IN_DISP(wCol, wRow) || bClipped);
	if (wTileImageNo == TI_TEMPTY) return; //Wasteful to make this call for empty blit.

	//Determine pixel positions.
	UINT wX = (CX_TILE * wCol) + wXOffset;
	UINT wY = (CY_TILE * wRow) + wYOffset;
	static const UINT CY_RAISED = 4;
	if (bDrawRaised)
	{
		wY -= CY_RAISED;
		if (wRow == 0)
			bClipped = true;
	}
	const int nPixelX = this->x + (int)wX;
	const int nPixelY = this->y + (int)wY;

	if (bClipped)
	{
		//Calculate what part of tile is showing.
		SDL_Rect ClipRect, BlitRect = {0, 0, CX_TILE, CY_TILE};
		GetRect(ClipRect);
		if (nPixelX < ClipRect.x)
		{
			BlitRect.x = ClipRect.x - nPixelX;
			ASSERT(BlitRect.x > 0);
			if (BlitRect.x >= BlitRect.w)
				return; //completely outside rect
			BlitRect.w -= BlitRect.x;
		}
		else if (nPixelX + (int)BlitRect.w >= ClipRect.x + (int)ClipRect.w)
		{
			if (nPixelX >= ClipRect.x + (int)ClipRect.w)
				return; //completely outside rect
			BlitRect.w = (ClipRect.x + ClipRect.w) - nPixelX;
		}
		if (nPixelY < ClipRect.y)
		{
			BlitRect.y = ClipRect.y - nPixelY;
			ASSERT(BlitRect.y > 0);
			if (BlitRect.y >= BlitRect.h)
				return; //completely outside rect
			BlitRect.h -= BlitRect.y;
		}
		else if (nPixelY + (int)BlitRect.h >= ClipRect.y + (int)ClipRect.h)
		{
			if (nPixelY >= ClipRect.y + (int)ClipRect.h)
				return; //completely outside rect
			BlitRect.h = (ClipRect.y + ClipRect.h) - nPixelY;
		}
		ASSERT(BlitRect.w <= CX_TILE);
		ASSERT(BlitRect.h <= CY_TILE);

		g_pTheDBM->BlitTileImagePart(wTileImageNo,
				nPixelX + BlitRect.x, nPixelY + BlitRect.y,
				BlitRect.x, BlitRect.y, BlitRect.w, BlitRect.h,
				pDestSurface, true, nOpacity);

		if (nAddColor >= 0)
			g_pTheBM->LightenRectWithTileMask(pDestSurface,
					nPixelX + BlitRect.x, nPixelY + BlitRect.y,
					BlitRect.w, BlitRect.h,
					1.0f+lightMap[0][nAddColor], 1.0f+lightMap[1][nAddColor], 1.0f+lightMap[2][nAddColor],
					wTileImageNo, BlitRect.x, BlitRect.y);

		//Set to proper light level.
		if (IsLightingRendered() && this->pRoom->IsValidColRow(wCol, wRow))
		{
			//No cast shadows when sprite is partially outside room.

			//Add dark to sprite.
			const UINT val = this->pRoom->tileLights.GetAt(wCol, wRow);
			if (bIsDarkTileValue(val))
			{
				ASSERT(val - LIGHT_OFF - 1 < NUM_DARK_TYPES);
				g_pTheBM->DarkenTileWithMask(wTileImageNo, BlitRect.x, BlitRect.y,
					nPixelX + BlitRect.x, nPixelY + BlitRect.y, BlitRect.w, BlitRect.h,
					pDestSurface, darkMap[(val - LIGHT_OFF - 1) *3/4]); //add 3/4 darkness
			}

			//Add light to sprite.
			//!!not implemented when partially outside of room
		}
	} else {
		//Draw sprite.
		g_pTheDBM->BlitTileImage(wTileImageNo, nPixelX, nPixelY, pDestSurface, true, nOpacity);

		//Add optional color filter.
		if (nAddColor >= 0)
			g_pTheBM->LightenRectWithTileMask(pDestSurface, nPixelX, nPixelY,
					CBitmapManager::CX_TILE, CBitmapManager::CY_TILE,
					1.0f+lightMap[0][nAddColor], 1.0f+lightMap[1][nAddColor], 1.0f+lightMap[2][nAddColor],
					wTileImageNo, 0, 0);

		//Set to proper light level.
		if (IsLightingRendered())
		{
			if (wXOffset || wYOffset)
			{
				//Don't cast shadows -- doesn't look right while moving

				AddLightOffset(pDestSurface, wCol, wRow, wXOffset, wYOffset, wTileImageNo, nOpacity, bDrawRaised ? -int(CY_RAISED) : 0);
			} else {
				//Add cast shadows to sprite.
				vector<UINT> &shadows = this->shadows[this->pRoom->ARRAYINDEX(wCol,wRow)];
				if (shadows.size() > 0)
					g_pTheBM->BlitTileShadowsWithTileMask(&(*shadows.begin()), shadows.size(),
							wTileImageNo, nPixelX, nPixelY, pDestSurface);

				//Add dark to sprite.
				const UINT val = this->pRoom->tileLights.GetAt(wCol, wRow);
				float fDark = 1.0;
				if (bIsDarkTileValue(val))
				{
					ASSERT(val - LIGHT_OFF - 1 < NUM_DARK_TYPES);
					fDark = darkMap[(val - LIGHT_OFF - 1) *3/4]; //add 3/4 darkness
					/*
					g_pTheBM->DarkenTileWithMask(wTileImageNo, 0, 0,
						nPixelX, nPixelY, CBitmapManager::CX_TILE, CBitmapManager::CY_TILE,
						pDestSurface, fDark);
					*/
				}

				//Add light to sprite.
				if (this->pRoom->tileLights.Exists(wCol,wRow) ||
					 this->lightedRoomTiles.has(wCol,wRow) || this->lightedPlayerTiles.has(wCol,wRow))
				{
					LIGHTTYPE *psL = this->psDisplayedLight + (wRow * this->pRoom->wRoomCols + wCol) * wLightValuesPerTile;
					AddLightInterp(pDestSurface, wCol, wRow, psL, fDark, wTileImageNo, nOpacity, bDrawRaised ? -int(CY_RAISED) : 0);
				}
			}
		}
	}

	//Dirty tiles covered by blit.  (There are up to four of them.)
	if (!bDirtyTiles)
	{
		//The 'monster' flag indicates that something was drawn here, and that
		//at the latest the tile should be repainted next turn.
		if (IS_COLROW_IN_DISP(wCol, wRow))
			this->pTileInfo[this->pRoom->ARRAYINDEX(wCol,wRow)].monster = 1;
		return;
	}

	DirtyTilesForSpriteAt(wX, wY);
}

void CRoomWidget::DirtyTilesForSpriteAt(UINT pixel_x, UINT pixel_y)
{
	UINT wMinX = pixel_x/CX_TILE;
	UINT wMinY = pixel_y/CY_TILE;
	const UINT wMaxX = (pixel_x+CX_TILE-1)/CX_TILE;
	const UINT wMaxY = (pixel_y+CY_TILE-1)/CY_TILE;
	if (wMinX > wMaxX) wMinX = wMaxX;   //bounds checking
	if (wMinY > wMaxY) wMinY = wMaxY;

	for (UINT wY = wMinY; wY <= wMaxY; ++wY)
		for (UINT wX = wMinX; wX <= wMaxX; ++wX)
			if (IS_COLROW_IN_DISP(wX, wY))
				this->pTileInfo[this->pRoom->ARRAYINDEX(wX,wY)].dirty = 1;
}

//*****************************************************************************
void CRoomWidget::DrawBoltInRoom(
//Draws a lightning bolt between the given coords.
//
//Params:
	const int xS, const int yS, const int xC, const int yC)
{
	static SDL_Rect RoomRect;
	SDL_Surface *pDestSurface = GetDestSurface();
	GetRect(RoomRect);
	SDL_SetClipRect(pDestSurface, &RoomRect);
	vector<SDL_Rect> dirtyRects;
	DrawBolt(xS, yS, xC, yC, CDrodBitmapManager::DISPLAY_COLS,
			this->images[BOLTS_SURFACE], pDestSurface, dirtyRects);
	this->pLastLayerEffects->DirtyTilesForRects(dirtyRects);

	SDL_SetClipRect(pDestSurface, NULL);
}

//*****************************************************************************
void CRoomWidget::DrawInvisibilityRange(
//Show effective range of invisibility/decoy effect by highlighting area around
//target.
//
//Params:
	int nX, int nY,   //(in)   Center of effect
	SDL_Surface *pDestSurface,    //(in)   Surface to draw to.
	CCoordIndex *pCoordIndex,		//(in/out)  which squares are drawn to [default=NULL]
		//if specified, only drawn to unmarked squares
	const int nRange) //[default=DEFAULT_SMELL_RANGE]
{
	ASSERT(nRange >= 0);
	ASSERT(this->pRoom->IsValidColRow(nX,nY));
	const int numTiles = nRange*2 + 1;
	const int x1 = nX - nRange;
	const int y1 = nY - nRange;
	const int x2 = nX + nRange;
	const int y2 = nY + nRange;
	int wX, wY;

	if (!pDestSurface)
	{
		//Show as lasting effect, rather than just painting this frame.
		static const SURFACECOLOR DecoyColor = {224, 160, 0};
		for (wY = y1; wY <= y2; ++wY)
			for (wX = x1; wX <= x2; ++wX)
			{
				if (!IS_COLROW_IN_DISP(wX,wY)) continue;
				if (pCoordIndex)
				{
					if (pCoordIndex->Exists(wX,wY)) continue;
					else pCoordIndex->Add(wX,wY);
				}
				AddShadeEffect(wX, wY, DecoyColor);
			}
	} else {
		static const float fShadingFactor = 0.80f;
		if (this->bAllDirty && !pCoordIndex)
			DarkenRect(pDestSurface, fShadingFactor, x1, y1, numTiles, numTiles);
		else {
			//Darken only dirty tiles.
			TILEINFO *pTI;
			for (wY = y1; wY <= y2; ++wY)
				for (wX = x1; wX <= x2; ++wX)
				{
					if (!IS_COLROW_IN_DISP(wX,wY)) continue;
					pTI = this->pTileInfo + this->pRoom->ARRAYINDEX((UINT)wX, (UINT)wY);
					if (this->bAllDirty || pTI->dirty || pTI->damaged)
					{
						if (pCoordIndex)
						{
							if (pCoordIndex->Exists(wX,wY)) continue;
							else pCoordIndex->Add(wX,wY);
						}

						g_pTheBM->DarkenTile(this->x + wX * CX_TILE,
								this->y + wY * CY_TILE, fShadingFactor, pDestSurface);
					}
				}
		}
		DirtyTileRect(x1-1,y1-1,x2+1,y2+1); //one tile buffer in case swordsman moved
	}
}

//*****************************************************************************
void CRoomWidget::DrawPlayer(
//Draws player along with his sword (if applicable) to game screen.
//
//Params:
	const CSwordsman &swordsman,  //(in) swordsman data
	SDL_Surface *pDestSurface)    //(in) Surface to draw to.
{
	ASSERT(IS_COLROW_IN_DISP(swordsman.wX, swordsman.wY));
	ASSERT(this->pRoom);

	static Uint8 nOpacity = 255;
	static bool bFadeIn = false;

	//Make sure display orientation is correct for role appearance.
	UINT wO;
	switch (swordsman.wAppearance)
	{
		case M_NONE: return; //Don't show anything if player is not being shown.
		case M_BRAIN: case M_SKIPPERNEST: wO = NO_ORIENTATION; break;
		default: wO = swordsman.wO; break;
	}
	ASSERT(IsValidOrientation(wO));

	const bool bHasSword = swordsman.HasSword();
	const bool bWading = this->pCurrentGame && this->pCurrentGame->IsPlayerWading() && bIsWadingMonsterType(swordsman.wAppearance);

	//Get tile image for player.
	UINT wFrame;
	if (bHasSword || !bEntityHasSword(swordsman.wAppearance)) {
		wFrame = bWading ? (UINT)SWORD_WADING_FRAME :
			this->pTileInfo[this->pRoom->ARRAYINDEX(swordsman.wX, swordsman.wY)].animFrame;
	} else {
		//If player is a swordless sword-wielding character, then try to show the swordless frame.
		wFrame = bWading ? WADING_FRAME : SWORDLESS_FRAME;
	}

	UINT wSManTI = GetCustomEntityTile(swordsman.wIdentity, wO, wFrame);
	UINT wSwordTI = TI_UNSPECIFIED;

	//Get optional custom sword tile.  It overrides other sword display variants below.
	if (this->pCurrentGame)
	{
		ASSERT(this->pCurrentGame->pHold);
		HoldCharacter *pCustomChar = this->pCurrentGame->pHold->GetCharacter(
				swordsman.wIdentity);
		if (pCustomChar)
			wSwordTI = g_pTheBM->GetCustomTileNo(pCustomChar->dwDataID_Tiles,
					GetCustomTileIndex(wO), SWORD_FRAME);
	}

	if (!swordsman.IsVisible() && !swordsman.bIsDying)
	{
		//Player is invisible.
		DrawInvisibilityRange(swordsman.wX, swordsman.wY, pDestSurface);

		//Show invisible player fading in and out.
		const bool bBeethroLike = bIsBeethroDouble(swordsman.wAppearance) &&
				swordsman.wAppearance != M_GUNTHRO;
		if (!this->pRoom->SomeMonsterCanSmellSwordsman() && bBeethroLike)
		{
			//If invisible player is not sensed by any monsters, and is
			//in a role that looks like Beethro, then show "hollow" version.
			//(Custom tiles specified above will override this behavior.)
			if (wSManTI == TI_UNSPECIFIED)
				wSManTI = GetTileImageForEntity(UINT(M_DECOY), wO, wFrame);
			if (wSwordTI == TI_UNSPECIFIED)
				wSwordTI = GetSwordTile(M_DECOY, wO);
		}
	}

	//If no custom or situation-specific player/sword tile is provided, use the stock/default.
	if (wSManTI == TI_UNSPECIFIED)
		wSManTI = GetTileImageForEntity(swordsman.wAppearance == static_cast<UINT>(-1) ?
			static_cast<UINT>(CHARACTER_FIRST) : swordsman.wAppearance, wO, wFrame);
	if (wSwordTI == TI_UNSPECIFIED)
		wSwordTI = GetSwordTile(swordsman.wAppearance, wO);

	if (swordsman.bIsDying)
		nOpacity = 255;
	else if (swordsman.bIsHasted)
	{
		//Speed potion effect: fast flashing.
		if (nOpacity < 20)
			bFadeIn = true;
		else if (nOpacity > 235)
			bFadeIn = false;
		if (bFadeIn)
			nOpacity += 15;
		else
			nOpacity -= 15;
	}
	else if (!swordsman.IsVisible())
	{
		//Invisible.  Fade in and out slowly.
		if (nOpacity < 50)   //don't make too faint
			bFadeIn = true;
		else if (nOpacity > 160)   //don't make too dark
			bFadeIn = false;
		if (bFadeIn)
			nOpacity += 2;
		else
			nOpacity -= 8;
	}
	else nOpacity = 255;

	//Draw swordsman raised above floor?
	const UINT wOSquare = this->pRoom->GetOSquare(swordsman.wX, swordsman.wY);
	const bool bDrawRaised = DrawRaised(wOSquare) && wOSquare != T_WALL_M;

	//Calculate animation offset.
	UINT wXOffset = 0, wYOffset = 0;
	int nSgnX = 0, nSgnY = 0;
	if (this->dwMovementStepsLeft)
	{
		wXOffset = (swordsman.wPrevX - swordsman.wX) * this->dwMovementStepsLeft;
		wYOffset = (swordsman.wPrevY - swordsman.wY) * this->dwMovementStepsLeft;
		nSgnX = swordsman.wPrevX == swordsman.wX ? 0 : swordsman.wPrevX > swordsman.wX ?
			1 : -1;
		nSgnY = swordsman.wPrevY == swordsman.wY ? 0 : swordsman.wPrevY > swordsman.wY ?
			1 : -1;
	}

	//Blit the player.
	const int nAddColor = swordsman.bFrozen ? FROZEN_COLOR_INDEX : NO_COLOR_INDEX;
	DrawTileImage(swordsman.wX, swordsman.wY, wXOffset, wYOffset, wSManTI,
			bDrawRaised, pDestSurface, true, nOpacity, false, nAddColor);

	//Blit the sword.
	if (!bHasSword)
		return;

	//Special case: don't draw sword when going down stairs and sword is occluded.
	if (bIsStairs(wOSquare) &&
			wO != (wOSquare == T_STAIRS ? N : S) &&
			(!this->pRoom->IsValidColRow(swordsman.wSwordX, swordsman.wSwordY) ||
			!bIsStairs(this->pRoom->GetOSquare(swordsman.wSwordX, swordsman.wSwordY))))
		return;	//sword should be hidden

	//See if sword square is within display.
	if (IS_COLROW_IN_DISP(swordsman.wSwordX, swordsman.wSwordY) &&
			IS_COLROW_IN_DISP(swordsman.wSwordX + nSgnX, swordsman.wSwordY + nSgnY))
	{
		DrawTileImage(swordsman.wSwordX, swordsman.wSwordY, wXOffset,
				wYOffset, wSwordTI, bDrawRaised, pDestSurface, true, nOpacity);
	} else {
		//Sword isn't fully in display -- just draw it clipped.
		//(This is needed for when swordsman is stepping onto room edge.)
		DrawTileImage(swordsman.wSwordX, swordsman.wSwordY, wXOffset, wYOffset,
				wSwordTI, bDrawRaised, pDestSurface, true, nOpacity, true);
	}
}

//*****************************************************************************
void CRoomWidget::DrawCharacter(
//Draws character if visible, according to set appearance.
//
//Params:
	const CCharacter *pCharacter, //(in)   Pointer to CCharacter monster.
	const bool bDrawRaised,    //(in)   Draw Character raised above floor?
	SDL_Surface *pDestSurface, //(in)   Surface to draw to.
	const bool bMoveInProgress)
{
	if (!pCharacter->IsVisible() && !pCharacter->IsGhostImage()) return;

	Uint8 nOpacity = 255;
	int nAddColor = NO_COLOR_INDEX;
	UINT wIdentity = pCharacter->GetIdentity();
	if (wIdentity == M_CLONE)
	{
		//Resolve identity of clone to actual role, and give clone color highlighting.
		wIdentity = pCharacter->GetResolvedIdentity();
		nAddColor = CLONE_COLOR_INDEX; //light green-blue to clones

		//Clones are invisible when the player is invisible.
		if (this->pCurrentGame)
		{
			const CSwordsman& player = this->pCurrentGame->swordsman;
			bool bInvisible = player.bIsInvisible;
			if (!bInvisible)
			{
				const UINT wOSquare = this->pRoom->GetOSquare(pCharacter->wX, pCharacter->wY);
				if ((wOSquare == T_SHALLOW_WATER) && (player.GetWaterTraversalState() == WTrv_CanHide))
					bInvisible = true;
			}
			if (bInvisible)
				nOpacity = 128;
		}
	}

	ASSERT(wIdentity < MONSTER_COUNT ||
			(wIdentity >= CHARACTER_FIRST && wIdentity < CHARACTER_TYPES) ||
			wIdentity == static_cast<UINT>(-1));
	const UINT wO = wIdentity == M_BRAIN || wIdentity == M_SKIPPERNEST ?
			NO_ORIENTATION : pCharacter->wO;

	//If a sword-wielding character is swordless, try to get its swordless frame.
	//However, if that frame is not defined or otherwise, then show the current animation frame.
	UINT wSX, wSY;
	const bool bHasSword = pCharacter->GetSwordCoords(wSX, wSY);
	UINT wFrame = bHasSword || !bEntityHasSword(wIdentity) ? this->pTileInfo[
			this->pRoom->ARRAYINDEX(pCharacter->wX, pCharacter->wY)].animFrame :
			(UINT)SWORDLESS_FRAME;
	if (pCharacter->IsWading()) {
		wFrame = bHasSword ? SWORD_WADING_FRAME : WADING_FRAME;
	}
	UINT wTileImageNo = GetEntityTile(wIdentity, pCharacter->wLogicalIdentity, wO, wFrame);

	//Calculate animation offset.
	UINT wXOffset = 0, wYOffset = 0;
	if (this->dwMovementStepsLeft)
	{
		wXOffset = (pCharacter->wPrevX - pCharacter->wX) * this->dwMovementStepsLeft;
		wYOffset = (pCharacter->wPrevY - pCharacter->wY) * this->dwMovementStepsLeft;
	}

	//Draw character.
	DrawTileImage(pCharacter->wX, pCharacter->wY, wXOffset, wYOffset,
			wTileImageNo, bDrawRaised, pDestSurface, bMoveInProgress,
			nOpacity, false, nAddColor);

	//Draw character with sword.
	if (bHasSword)
		DrawSwordFor(pCharacter, wIdentity, wSX, wSY,
				wXOffset, wYOffset, bDrawRaised, pDestSurface, bMoveInProgress);
}

//*****************************************************************************
void CRoomWidget::DrawCitizen(
//Draws character if visible, according to set appearance.
//
//Params:
	const CCitizen *pCitizen,  //(in)   Pointer to CCitizen monster.
	const bool bDrawRaised,    //(in)   Draw raised above floor?
	SDL_Surface *pDestSurface, //(in)   Surface to draw to.
	const bool bMoveInProgress)
{
	const bool bWading = pCitizen->IsWading();
	const UINT wFrame = bWading ? (UINT)WADING_FRAME :
		this->pTileInfo[this->pRoom->ARRAYINDEX(pCitizen->wX, pCitizen->wY)].animFrame;

	const UINT wTileImageNo = GetTileImageForEntity(M_CITIZEN, pCitizen->wO, wFrame);
	//Calculate animation offset.
	UINT wXOffset = 0, wYOffset = 0;
	if (this->dwMovementStepsLeft)
	{
		wXOffset = (pCitizen->wPrevX - pCitizen->wX) * this->dwMovementStepsLeft;
		wYOffset = (pCitizen->wPrevY - pCitizen->wY) * this->dwMovementStepsLeft;
	}

	DrawTileImage(pCitizen->wX, pCitizen->wY, wXOffset, wYOffset,
			wTileImageNo, bDrawRaised, pDestSurface, bMoveInProgress, 255, false,
			pCitizen->StationType());
}

//*****************************************************************************
void CRoomWidget::DrawDouble(
//Draws player double along with his sword to room.
//
//Params:
	const CPlayerDouble *pDouble,    //(in)   Pointer to monster.
	const bool bDrawRaised,    //(in)   Draw double raised above floor?
	SDL_Surface *pDestSurface, //(in)   Surface to draw to.
	const bool bMoveInProgress,
	Uint8 nOpacity)      //(in)   Opacity of double [default=255]
{
	ASSERT(pDouble);
	ASSERT(IsValidOrientation(pDouble->wO));
	ASSERT(IS_COLROW_IN_DISP(pDouble->wX, pDouble->wY));

	//Calculate animation offset.
	UINT wXOffset = 0, wYOffset = 0;
	if (this->dwMovementStepsLeft)
	{
		wXOffset = (pDouble->wPrevX - pDouble->wX) * this->dwMovementStepsLeft;
		wYOffset = (pDouble->wPrevY - pDouble->wY) * this->dwMovementStepsLeft;
	}

	//Blit the double.
	const bool bHasSword = pDouble->HasSword();
	const bool bWading = pDouble->IsWading() && bIsWadingMonsterType(pDouble->wType);
	UINT wFrame = 0;
	if (bWading) {
		wFrame = bHasSword ? SWORD_WADING_FRAME : WADING_FRAME;
	} else if (!bHasSword) {
		wFrame = SWORDLESS_FRAME;
	}

	UINT visualIdentity = pDouble->GetIdentity();
	UINT wDoubleTI = GetTileImageForEntity(visualIdentity, pDouble->wO, wFrame);
	int nAddColor = NO_COLOR_INDEX;
	switch (pDouble->wType)
	{
		case M_CLONE:
		{
			//Clone may also look like a custom player tileset.
			if (this->pCurrentGame)
			{
				CSwordsman& player = this->pCurrentGame->swordsman;
				UINT wSManTI = GetCustomEntityTile(player.wIdentity, pDouble->wO, wFrame);
				if (wSManTI != TI_UNSPECIFIED)
					wDoubleTI = wSManTI;

				//Invisible clones are drawn with transparency
				if (pDouble->IsHiding())
					nOpacity = 128;
			}

			nAddColor = pDouble->bFrozen ? FROZEN_COLOR_INDEX : CLONE_COLOR_INDEX; //light green-blue to clones
		}
		break;
		case M_STALWART: case M_STALWART2:
		case M_MIMIC:
			if (pDouble->bFrozen)
				nAddColor = FROZEN_COLOR_INDEX;
		break;
	}
	DrawTileImage(pDouble->wX, pDouble->wY, wXOffset, wYOffset, wDoubleTI,
			bDrawRaised, pDestSurface, bMoveInProgress, nOpacity, false, nAddColor);

	if (bHasSword)
		DrawSwordFor(pDouble, visualIdentity, pDouble->GetSwordX(), pDouble->GetSwordY(),
				wXOffset, wYOffset, bDrawRaised, pDestSurface, bMoveInProgress, nOpacity);
}

//*****************************************************************************
void CRoomWidget::DrawRockGiant(
//Draws RockGiant monster.
//
//Params:
	const CMonster *pMonster,    //(in)   Pointer to splitter monster.
	const bool /*bDrawRaised*/,    //(in)   never raised
	SDL_Surface *pDestSurface, //(in)   Surface to draw to.
	const bool bMoveInProgress)
{
	ASSERT(pMonster->wType == M_ROCKGIANT);

	//Calculate animation offset.
	UINT wXOffset = 0, wYOffset = 0;
	if (this->dwMovementStepsLeft)
	{
		wXOffset = (pMonster->wPrevX - pMonster->wX) * this->dwMovementStepsLeft;
		wYOffset = (pMonster->wPrevY - pMonster->wY) * this->dwMovementStepsLeft;
	}
	const UINT wFrame = this->pTileInfo[this->pRoom->ARRAYINDEX(pMonster->wX, pMonster->wY)].animFrame;

	const UINT wSplitterTI = GetTileImageForEntity(M_ROCKGIANT, pMonster->wO, wFrame);
	DrawTileImage(pMonster->wX, pMonster->wY, wXOffset, wYOffset,
			wSplitterTI, false, pDestSurface, bMoveInProgress);

	UINT wI = 0;
	for (list<CMonsterPiece*>::const_iterator piece = pMonster->Pieces.begin();
			piece != pMonster->Pieces.end(); ++piece, ++wI)
	{
		ASSERT(this->pRoom->IsValidColRow((*piece)->wX, (*piece)->wY));
		DrawTileImage((*piece)->wX, (*piece)->wY, wXOffset, wYOffset,
				GetTileImageForRockGiantPiece(wI, pMonster->wO, wFrame),
				false, pDestSurface, bMoveInProgress);
	}
}

//*****************************************************************************
void CRoomWidget::DrawSwordFor(
//Draw a sword for the given monster.
//
//Params:
	const CMonster *pMonster,    //(in)   Pointer to monster.
	const UINT wType,									//monster type
	const UINT wMSwordX, const UINT wMSwordY,	//coord
	const UINT wXOffset, const UINT wYOffset,	//pixel offset
	const bool bDrawRaised,    //(in)   Draw mimic raised above floor?
	SDL_Surface *pDestSurface, //(in)   Surface to draw to.
	const bool bMoveInProgress,
	const Uint8 nOpacity)      //(in)   Opacity of sword [default=255]
{
	ASSERT(pMonster);
	ASSERT(IsValidOrientation(pMonster->wO));
	UINT wSwordTI = TI_UNSPECIFIED;

	//Get optional custom sword tile.
	if (this->pCurrentGame)
	{
		//Currently, supported for characters and player clones only.
		ASSERT(this->pCurrentGame->pHold);
		HoldCharacter *pCustomChar = NULL;
		switch (pMonster->wType)
		{
			case M_CHARACTER:
			{
				const CCharacter *pCharacter = DYN_CAST(const CCharacter*, const CMonster*, pMonster);
				pCustomChar = this->pCurrentGame->pHold->GetCharacter(
						pCharacter->wLogicalIdentity);
			}
			break;
			case M_CLONE:
			{
				CSwordsman& player = this->pCurrentGame->swordsman;
				pCustomChar = this->pCurrentGame->pHold->GetCharacter(player.wIdentity);
			}
			break;
			default: break;
		}

		if (pCustomChar)
			wSwordTI = g_pTheBM->GetCustomTileNo(pCustomChar->dwDataID_Tiles,
					GetCustomTileIndex(pMonster->wO), SWORD_FRAME);
	}

	//Calculate monster's default sword tile.
	if (wSwordTI == TI_UNSPECIFIED)
		wSwordTI = GetSwordTile(wType, pMonster->wO);

	DrawSwordFor(pMonster, wMSwordX, wMSwordY, wXOffset, wYOffset,
			bDrawRaised, wSwordTI, pDestSurface, bMoveInProgress, nOpacity);
}

//*****************************************************************************
void CRoomWidget::DrawSwordFor(
//Draw's a sword tile for the given monster.
//
//Params:
	const CMonster *pMonster,    //(in)   Pointer to monster.
	const UINT wMSwordX, const UINT wMSwordY,	//coord
	const UINT wXOffset, const UINT wYOffset,	//pixel offset
	const bool bDrawRaised,    //Draw mimic raised above floor?
	const UINT wSwordTI,			//sword tile
	SDL_Surface *pDestSurface, //(in)   Surface to draw to.
	const bool bMoveInProgress,
	const Uint8 nOpacity)      //(in)   Opacity of sword [default=255]
{
	ASSERT(pMonster);
	ASSERT(IS_COLROW_IN_DISP(pMonster->wX, pMonster->wY));

	//See if its sword square is within display.
	int nSgnX = 0, nSgnY = 0;
	if (this->dwMovementStepsLeft)
	{
		nSgnX = pMonster->wPrevX == pMonster->wX ? 0 :
				pMonster->wPrevX > pMonster->wX ? 1 : -1;
		nSgnY = pMonster->wPrevY == pMonster->wY ? 0 :
				pMonster->wPrevY > pMonster->wY ? 1 : -1;
	}

	if (IS_COLROW_IN_DISP(wMSwordX, wMSwordY) &&
			 IS_COLROW_IN_DISP(wMSwordX + nSgnX, wMSwordY + nSgnY))
	{
		//Get entity sword tile image from array.
		DrawTileImage(wMSwordX, wMSwordY, wXOffset, wYOffset, wSwordTI,
				bDrawRaised, pDestSurface, bMoveInProgress, nOpacity);
	} else {
		//Sword isn't fully in display -- just draw it clipped.
		//(This is needed for when stepping onto room edge.)
		DrawTileImage(wMSwordX, wMSwordY, wXOffset, wYOffset,
				wSwordTI, bDrawRaised, pDestSurface, bMoveInProgress, nOpacity, true);
	}
}

//*****************************************************************************
void CRoomWidget::DrawDoubleCursor(
//Draws a double cursor to a specified square of the room.
//
//Params:
	const UINT wCol, const UINT wRow,      //(in)   Destination square coords.
	SDL_Surface *pDestSurface) //(in)   Surface to draw to.
{
	ASSERT(IS_COLROW_IN_DISP(wCol, wRow));
	ASSERT(this->pCurrentGame);

	const int xPixel = this->x + wCol * CX_TILE;
	const int yPixel = this->y + wRow * CY_TILE;
	const bool bObstacle = this->pRoom->
			DoesSquareContainDoublePlacementObstacle(wCol, wRow,
			this->pCurrentGame->swordsman.wPlacingDoubleType);

	//Draw cursor.
	//Animate mimic cursor movement.
	static UINT wPrevCol = wCol, wPrevRow = wRow;
	if (!this->dwMovementStepsLeft || this->bFinishLastMovementNow)
	{
		wPrevCol = wCol;
		wPrevRow = wRow;
	}

	//Show illegal placement tile.
	if (bObstacle)
	{
		g_pTheBM->ShadeTile(xPixel,yPixel,Red,GetDestSurface());
	} else {
		//Fade in and out.
		static Uint8 nOpacity = 160;
		static bool bFadeIn = false;
		if (nOpacity < 35)   //don't make too faint
			bFadeIn = true;
		else if (nOpacity > 180)   //don't make too dark
			bFadeIn = false;
		if (bFadeIn)
			nOpacity += 7;
		else
			nOpacity -= 5;

		//This double is not in the monster list yet.
		CPlayerDouble *pDouble;
		switch (this->pCurrentGame->swordsman.wPlacingDoubleType)
		{
			case M_CLONE: pDouble = new CClone(this->pCurrentGame); break;
			default: pDouble = new CPlayerDouble(
						this->pCurrentGame->swordsman.wPlacingDoubleType, this->pCurrentGame);\
			break;
		}
		pDouble->wO = this->pCurrentGame->swordsman.wO;
		pDouble->wX = wCol;
		pDouble->wY = wRow;
		pDouble->wPrevX = wPrevCol;
		pDouble->wPrevY = wPrevRow;
		pDouble->SetSwordSheathed();
		DrawDouble(pDouble, true, pDestSurface, true, nOpacity);
		delete pDouble;
	}

	//Draw bolt from player to cursor.
	static const UINT CX_TILE_HALF = CX_TILE / 2;
	static const UINT CY_TILE_HALF = CY_TILE / 2;
	const UINT wSX = this->pCurrentGame->swordsman.wX; //always use player's actual position, since he's the only one who can place doubles
	const UINT wSY = this->pCurrentGame->swordsman.wY;
	const int xS = this->x + wSX * CX_TILE + CX_TILE_HALF;
	const int yS = this->y + wSY * CY_TILE + CY_TILE_HALF;
	const int xC = xPixel + CX_TILE_HALF;
	const int yC = yPixel + CY_TILE_HALF;
	DrawBoltInRoom(xS, yS, xC, yC);
}

//*****************************************************************************************
void CRoomWidget::DrawSerpent(
// Starting after head, traverse and draw room tiles to the tail.
// Assumes a valid serpent.
//
//Params:
	const CMonster *pMonster,  //(in)   Pointer to CSerpent monster.
	SDL_Surface *pDestSurface, //(in)   Surface to draw to.
	const bool bMoveInProgress)
{
	ASSERT(bIsSerpent(pMonster->wType));
	ASSERT(this->pRoom);

	//No animation of pieces.
	for (list<CMonsterPiece*>::const_iterator piece = pMonster->Pieces.begin();
			piece != pMonster->Pieces.end(); ++piece)
	{
		DrawTileImage((*piece)->wX, (*piece)->wY, 0, 0,
				GetTileImageForSerpentPiece(pMonster->wType, (*piece)->wTileNo),
				false, pDestSurface, bMoveInProgress);
	}

	if (this->dwMovementStepsLeft)
	{
		//Erase where tail used to be.
		const CSerpent *pSerpent = DYN_CAST(const CSerpent*, const CMonster*, pMonster);
		this->pTileInfo[this->pRoom->ARRAYINDEX(pSerpent->wOldTailX, pSerpent->wOldTailY)].dirty = 1;
	}
}

//*****************************************************************************
void CRoomWidget::DrawSlayerWisp(
//Draw the wisp coming off of the Slayer.
//
//Params:
	const CPlayerDouble *pDouble,
	SDL_Surface *pDestSurface)
{
	static const UINT NUM_WISP_FRAMES = 4;
	static const UINT wispFrame[NUM_WISP_FRAMES] = {
		TI_WISP1, TI_WISP2, TI_WISP3, TI_WISP4
	};

	for (list<CMonsterPiece*>::const_iterator piece = pDouble->Pieces.begin();
			piece != pDouble->Pieces.end(); ++piece)
	{
		CMonsterPiece *pPiece = *piece;
		ASSERT(this->pRoom->IsValidColRow(pPiece->wX, pPiece->wY));

		//Animate each piece.
		if (!pPiece->wTileNo)
			pPiece->wTileNo = 1 + RAND(NUM_WISP_FRAMES);
		if (this->bNextWispFrame)
		{
			if (++pPiece->wTileNo > NUM_WISP_FRAMES)
				pPiece->wTileNo = 1;
		}

		ASSERT(pPiece->wTileNo-1 < NUM_WISP_FRAMES);
		DrawTileImage(pPiece->wX, pPiece->wY, 0, 0, wispFrame[pPiece->wTileNo-1],
				false, pDestSurface,	false);

		//Must erase tile next frame.
		this->pTileInfo[this->pRoom->ARRAYINDEX(pPiece->wX, pPiece->wY)].dirty = 1;
	}
}

//*****************************************************************************
void CRoomWidget::CastLightOnTile(
//Determines how far light will be cast and what intensity it has at this square.
//If distance limit has not been reached, then light continues from square (wX,wY),
//according to direction light is being cast.
//
//Params:
	const UINT wSX, const UINT wSY,  //(in) position of light source
	const UINT wX, const UINT wY,    //(in) square to place light effect
	Light *pLight,					//(in) light source
	const bool bGeometry)     //if true (default), take room geometry into account, otherwise ignore
{
	ASSERT(pLight);

	if (!this->pRoom->IsValidColRow(wX, wY))
		return;

	const int dx = (int)wX - (int)wSX, dy = (int)wY - (int)wSY;
	ASSERT(abs(dx) <= MAX_LIGHT_DISTANCE);
	ASSERT(abs(dy) <= MAX_LIGHT_DISTANCE);

	//Consider light direction to perform simple optimizations.
	const bool bCenter = !dx && !dy;
	const bool bAxial = !dx || !dy;
	const bool bCorner = abs(dx) == abs(dy);

	const int closerDX = dx-(dx>0?1:dx<0?-1:0); //one coord closer to the light source
	const int closerDY = dy-(dy>0?1:dy<0?-1:0);

	LightedType nextClosestTile = L_Light;
	if (bGeometry)
	{
		nextClosestTile = tileLight[MAX_LIGHT_DISTANCE + closerDX][MAX_LIGHT_DISTANCE + closerDY];
		if (!bCenter && nextClosestTile == L_Dark)
		{
			//Light fully blocked by closer tiles means no light shows here.
			if (bAxial || bCorner)
			{
				tileLight[MAX_LIGHT_DISTANCE + dx][MAX_LIGHT_DISTANCE + dy] = L_Dark;
				return;
			} else {
				const bool bHoriz = abs(dx) > abs(dy);
				if (tileLight[MAX_LIGHT_DISTANCE + (bHoriz?closerDX:dx)][MAX_LIGHT_DISTANCE + (bHoriz?dy:closerDY)] == L_Dark)
				{
					tileLight[MAX_LIGHT_DISTANCE + dx][MAX_LIGHT_DISTANCE + dy] = L_Dark;
					return;
				}
			}
		}
	}

	const float fElevAtLight = bGeometry ? getTileElev(this->pRoom->GetOSquare(wSX,wSY)) : 0;
	const UINT wOTile = this->pRoom->GetOSquare(wX,wY);
	float fElev = getTileElev(wOTile);

	//Is there an obstruction here?
	bool bWallShine = false;
	if (fElevAtLight <= 0.0 && fElev > 0.0)
	{
		if (bIsDoor(wOTile))
		{
			tileLight[MAX_LIGHT_DISTANCE + dx][MAX_LIGHT_DISTANCE + dy] = L_Dark;
			return; //don't show light on door faces at all
		}

		//this wall actually only extends partly back -- so light can shine behind it (in the occluded area)
		bool bNorthernWall = bGeometry ? getTileElev(wX,wY-1) <= 0.0 : true;

		const UINT wTI = this->pwOSquareTI[this->pRoom->ARRAYINDEX(wX,wY)];
		switch (wTI)
		{
			case TI_WALL_SW: case TI_WALL_SW2: case TI_WALL_SE: case TI_WALL_SE1:
			case TI_WALL_NSE: case TI_WALL_SWE: case TI_WALL_NSW: case TI_WALL_NSWE: case TI_WALL_NS:
			case TI_WALL_S: case TI_WALL_S1: case TI_WALL_S2: case TI_WALL_S3:
			case TI_WALL_BSW: case TI_WALL_BSW2: case	TI_WALL_BSE: case	TI_WALL_BSE1:
			case TI_WALL_BNSE: case TI_WALL_BSWE: case TI_WALL_BNSW: case TI_WALL_BNSWE: case TI_WALL_BNS:
			case TI_WALL_BS: case TI_WALL_BS1: case TI_WALL_BS2: case TI_WALL_BS3:
			case TI_WALL_HSW: case TI_WALL_HSW2: case	TI_WALL_HSE: case	TI_WALL_HSE1:
			case TI_WALL_HNSE: case TI_WALL_HSWE: case TI_WALL_HNSW: case TI_WALL_HNSWE: case TI_WALL_HNS:
			case TI_WALL_HS: case TI_WALL_HS1: case TI_WALL_HS2: case TI_WALL_HS3:
				bWallShine = dy < 0; //light propagating north shines on south wall faces
			break;
			default:
				if (!bGeometry) //don't ever shine wall lights on the top of walls
					return;

				if (!bNorthernWall)
				{
					tileLight[MAX_LIGHT_DISTANCE + dx][MAX_LIGHT_DISTANCE + dy] = L_Dark;
					return;	//don't show light on or past this wall at all
				}
				if (!dx)
				{
					tileLight[MAX_LIGHT_DISTANCE + dx][MAX_LIGHT_DISTANCE + dy] = L_Dark;
					return;  //light shining directly north or south won't expand further
				}
			break;
		}
		if (!bWallShine)
		{
			//Tile partially or completely obstructs light.
			tileLight[MAX_LIGHT_DISTANCE + dx][MAX_LIGHT_DISTANCE + dy] =
					bNorthernWall && dy > 0 ? L_Partial : L_Dark;
			return;
		}

		//Prevent light shining on inner wall areas.
		if (!bGeometry && bWallShine)
		{
			if (getTileElev(wX,wY+1) > 0.0)
				return; //(x,y) is not really a south-facing wall
		}
	}

	//If we reached here, the tile probably has light on it.
	bool bFullyLit = true;
	//Did item on tile closer to light partially obstruct the light?
	bool bPartialOcclusion = nextClosestTile == L_PartialItem;
	tileLight[MAX_LIGHT_DISTANCE + dx][MAX_LIGHT_DISTANCE + dy] = L_Light;

	//Is there an object on the current tile that partially obstructs the light?
	bool bSphereItem = false;
	if (bGeometry)
		switch (this->pRoom->GetTSquare(wX, wY))
		{
			case T_ORB: case T_BOMB: bFullyLit = false; bPartialOcclusion = true; bSphereItem = true; break;
			default: break;
		}

	if (bGeometry && bFullyLit && !bCenter)
	{
		//Light shining completely above walls is not occluded.
		if (!(fElevAtLight > 0.0 && fElev > 0.0))
		{
			//If light is shining down from a wall-top to a lower tile, then
			//the wall will partially occlude the light.
			if (fElevAtLight > 0.0 && fElev <= 0.0)
			{
				bFullyLit = false;
				bPartialOcclusion = true;
			}

			//If nothing on this tile stops light, and nothing before the tile stopped the light,
			//then this tile is fully lit also.
			if (tileLight[MAX_LIGHT_DISTANCE + closerDX][MAX_LIGHT_DISTANCE + closerDY] != L_Light ||
				 tileLight[MAX_LIGHT_DISTANCE + dx][MAX_LIGHT_DISTANCE + closerDY] != L_Light ||
				 (tileLight[MAX_LIGHT_DISTANCE + closerDX][MAX_LIGHT_DISTANCE + dy] != L_Light &&
					!bWallShine))   //horizontally adjacent tiles won't occlude a wall shine
				bFullyLit = false; //otherwise there is probably only partial lighting on this tile
		}
	}

	//Distance in tiles from light source to top-left corner of this tile.
	static const float fEpsilon = 0.01f;
	static const float fLightIncrement = (1.0f - 2*fEpsilon)/(float)(LIGHT_SPT_MINUSONE); //slightly less than one to avoid hitting ambiguous tile edges
	const float dxTile = ((int)wX - (int)wSX) - 0.5f - fEpsilon; //slightly less than one-half to avoid ambiguous corners
	const float dyTile = ((int)wY - (int)wSY) - 0.5f - fEpsilon;

	//Light intensity.
	float fFullLight = fMaxLightIntensity[this->wDark];
	const UINT lightTileVal = this->pRoom->tileLights.GetAt(wX, wY);
	if (bIsDarkTileValue(lightTileVal)) //shine brighter in specially marked dark tiles
	{
		ASSERT(lightTileVal - LIGHT_OFF - 1 < NUM_DARK_TYPES);
		fFullLight /= darkMap[lightTileVal - LIGHT_OFF - 1];
	}
	const float fMin = fFullLight / (1.1f*pLight->fMaxDistance);

	//Supersample the tile.
	const UINT wIndex = this->pRoom->ARRAYINDEX(wX,wY);
	const UINT wBaseLightIndex = wIndex * wLightValuesPerTile;
	int i,j;	//sub-tile granularity
	const int minI = dx>=0 ? 0 : LIGHT_SPT_MINUSONE;
	const int minJ = dy>=0 ? 0 : LIGHT_SPT_MINUSONE;
	const int maxI = minI==0 ? LIGHT_SPT : -1;
	const int maxJ = minJ==0 ? LIGHT_SPT : -1;
	const int incI = dx>=0 ? 1 : -1;
	const int incJ = dy>=0 ? 1 : -1;
	bool bIFirst, bJFirst = true;
	bool bSomeLight = false, bSomeDark = false;
	float fZ = max(0.0001f,fElev); //keep off of ground
	float dzSquared = 0.9f - fZ;    //the constant is generally the height of the light
	dzSquared *= dzSquared;
	for (j=minJ; j!=maxJ; j += incJ, bJFirst = false)
	{
		const float yOffset = j*fLightIncrement;

		//Except, that light shines on south wall faces.
		if (bWallShine)
		{
			if (j < LIGHT_SPT/2)
				continue;	//only show light on bottom-half of wall tile
			fZ = 2*(fElev - yOffset);
		}

		//Calculate distance of each sub-tile sample from light source (in pixels).
		const UINT wRowLightIndex = wBaseLightIndex + j*(LIGHT_SPT*LIGHT_BPP); //RGB
		const float dy = dyTile + yOffset;  //top-left corner of sub-tile
		const float dySquared = dy*dy;
		//add small offset to not check for light directly on the corner (and same for fX)
		const float fY = wY + (bWallShine ? 1.001f : yOffset + fEpsilon);
		bIFirst = true;
		for (i=minI; i!=maxI; i += incI, bIFirst = false)
		{
			const float xOffset = i*fLightIncrement;
			const float dx = dxTile + xOffset;  //middle of sub-tile

			if (!bFullyLit)
			{
				//Light probably doesn't shine completely here, so
				//find out where there's an occluding surface in the way.
				if (!bPartialOcclusion && !bIFirst && !bJFirst)
				{
					//When no occluding object is on this tile,
					//and we've already scanned an earlier row and column,
					//then certain deductions can be made:
					const UINT wCount = subtileLight[i-incI][j-incJ] + subtileLight[i-incI][j] + subtileLight[i][j-incJ];
					if (wCount == 3)
					{
						//Known: there is no shadow near this subtile, so there won't
						//be any shadow here either.
						goto AddLightToSubtile;
					}
					if (!wCount	&& fElevAtLight <= 0)
					{
						//Known: when the area in front of this subtile is in shadow,
						//and the light is not shining from above onto this tile,
						//then this further subtile is also in shadow.
						subtileLight[i][j] = L_Dark;
						bSomeDark = true;
						continue;
					}
				}

				//Cast a shadow ray.
				{
					float fZOffset = 0.0f;
					if (bSphereItem)
					{
						//Calculate point in space where light ray would hit sphere.
						static const float PiOverTwoOrbRad = 3.14159265359f / (2.0f*0.22f); //*orbRadius); //why does .22f work?  no idea...
						static const float fOffset = fEpsilon-0.5f;
						static const float fOrbRadiusSq = orbRadius * orbRadius;
						const float fRadSq = (xOffset+fOffset)*(xOffset+fOffset) + (yOffset+fOffset)*(yOffset+fOffset);
						if (fRadSq <= fOrbRadiusSq)
							fZOffset = orbRadius * (1.0f + sin(PiOverTwoOrbRad*(orbRadius - sqrt(fRadSq))));
					}
					Point here(wX + xOffset + fEpsilon, fY, fZ + fZOffset); //add small offset to not check directly on the tile corner
					if (!this->model.lightShinesAt(here, pLight))
					{
						subtileLight[i][j] = L_Dark;
						bSomeDark = true;
						continue;
					}
				}
AddLightToSubtile:
				subtileLight[i][j] = L_Light;
				bSomeLight = true;
			}

			//Calculate light intensity from source at center of this tile.
			//
			//Use linear falloff for light for better look than squared falloff.
			//fMin is the amount of light shown at the perimeter of the light.
			//It is subtracted so that it appears natural that the light has no
			//effect past its enforced maximum distance (wMaxDistance).
			const float fDistSquared = dx * dx + dySquared + dzSquared;
			float fLight = fFullLight / sqrt(fDistSquared) - fMin;

			//Add RGB light to tile.
			if (fLight > 0.0)
			{
				//Emphasize wall shine.
				if (bWallShine)
					fLight *= fWallLightingMultiplier[this->wDark];

				const float sLight = float(fLight * 256.0);
				const UINT wLightIndex = wRowLightIndex + i*LIGHT_BPP; //RGB
				const Color& color = pLight->color;
				UINT val;
				val = this->pActiveLight[wLightIndex+0] + UINT(color.r() * sLight);
				this->pActiveLight[wLightIndex]   = val < LIGHTTYPE(-1) ? val : LIGHTTYPE(-1);
				val = this->pActiveLight[wLightIndex+1] + UINT(color.g() * sLight);
				this->pActiveLight[wLightIndex+1] = val < LIGHTTYPE(-1) ? val : LIGHTTYPE(-1);
				val = this->pActiveLight[wLightIndex+2] + UINT(color.b() * sLight);
				this->pActiveLight[wLightIndex+2] = val < LIGHTTYPE(-1) ? val : LIGHTTYPE(-1);
			}
		}
	}

	//Record what kind of light was actually mapped onto tile.
	if (bGeometry)
	{
		if (bSomeLight && !bSomeDark)
			bFullyLit = true;
		const LightedType tileLightType =
			tileLight[MAX_LIGHT_DISTANCE + dx][MAX_LIGHT_DISTANCE + dy] =
				bWallShine ? L_Dark : //Light shining on a wall is obstructed from further propagation.
				bFullyLit ? L_Light : //Nothing blocked light here.
				bPartialOcclusion ? L_PartialItem :  //An item partially blocked the light (and might not block it further out)
				bSomeLight ? (bSomeDark ? L_Partial : L_Light) :  //Was there some shadow here?
					L_Dark; //There was no light here.

		if (tileLightType == L_Partial || tileLightType == L_PartialItem)
			this->partialLightedTiles.insert(wX,wY); //blur light edge here

		//Set to the max value applied to this tile this frame
		//if the light is not on a wall or other elevated tile.
		//This is to facilitate light blurring when shadows cross floor-level tiles.
		if (fElev <= 0.0 && this->tileLightInfo.GetAt(wX,wY) < tileLightType)
			this->tileLightInfo.Set(wX,wY,tileLightType);
	}

	this->pActiveLightedTiles->insert(wX,wY);
	this->pTileInfo[wIndex].dirty = 1;
}

//*****************************************************************************
void CRoomWidget::ProcessLightmap()
//Initialize the light display buffer.
//1. Light in room layer is blurred and applied to the display buffer.
//2. Ceiling lights are also added.
{
	const UINT wSize = this->pRoom->CalcRoomArea() * wLightValuesPerTile;

	//Render ceiling light map once when requested.
	if (!this->bCeilingLightsRendered)
	{
		//Reset ceiling light buffer.
		memset(this->psCeilingLight, 0, wSize * sizeof(LIGHTTYPE));
		if (!this->pRoom->tileLights.empty())
		{
			for (UINT wY = 0; wY < this->pRoom->wRoomRows; ++wY)
				for (UINT wX = 0; wX < this->pRoom->wRoomCols; ++wX)
				{
					const UINT lightVal = this->pRoom->tileLights.GetAt(wX,wY);
					if (bIsLightTileValue(lightVal))
					{
						//Light this ceiling tile.
						SetCeilingLight(wX,wY);
						this->pTileInfo[this->pRoom->ARRAYINDEX(wX,wY)].dirty = 1;
					}
				}
		}
		this->bCeilingLightsRendered = true;
	}

	LIGHTTYPE *pCeilingDestBuffer = IsPlayerLightRendered() ? this->psRoomLight : this->psDisplayedLight;

	//1. Calculate room lighting buffer.
	if (this->lightedRoomTiles.empty() || this->partialLightedTiles.empty())
	{
		if (this->lightedRoomTiles.empty())
		{
			//If there are no room lights, reset buffer to display ceiling lights only.
			if (!this->pRoom->tileLights.empty() ||
					!this->pCurrentGame) //i.e. in the editor, always refresh
			{
				memcpy(pCeilingDestBuffer, this->psCeilingLight, wSize * sizeof(LIGHTTYPE));
				return;
			}
			//If there are no ceiling lights either, then this buffer can be ignored.
		} else if (!IsPlayerLightRendered()) {
			//If there are room lights but they don't need to be smoothed, just display them.
			memcpy(pCeilingDestBuffer, this->psRoomLight, wSize * sizeof(LIGHTTYPE));
		}
	} else {
		//Perform a low-pass filter on the room layer light map to reduce aliasing.
		//Write results to the display light buffer.
		LIGHTTYPE *pSrc = this->psRoomLight;
		LIGHTTYPE *pDest = this->psDisplayedLight; //result of blurring operation
			//The final display buffer may be used as the result buffer, since
			//a copy operation may be avoided if no more light needs to be layered

		const UINT wLastCol = this->pRoom->wRoomCols-1;
		const UINT wLastRow = this->pRoom->wRoomRows-1;
		for (UINT wY=0; wY<this->pRoom->wRoomRows; ++wY)
		{
			//Process a row of room tiles.
			for (UINT wX=0; wX<this->pRoom->wRoomCols; ++wX)
			{
				//Process a room tile.
				if (this->partialLightedTiles.has(wX,wY))
				{
					//Blurring the light on this tile will have a noticeable effect.

					//Whether there is light at the edges of the adjacent tiles to factor in.
					const bool bUp = wY > 0 && this->tileLightInfo.GetAt(wX,wY-1) != L_Dark;
					const bool bDown = wY < wLastRow && this->tileLightInfo.GetAt(wX,wY+1) != L_Dark;
					const bool bLeft = wX > 0 && this->tileLightInfo.GetAt(wX-1,wY) != L_Dark;
					const bool bRight = wX < wLastCol && this->tileLightInfo.GetAt(wX+1,wY) != L_Dark;

					LowPassLightFilter(pSrc, pDest, bLeft, bRight, bUp, bDown);
				} else {
					//Tile does not need to be blurred.

					//Initialize the display buffer with these values.
					memcpy(pDest, pSrc, wLightBytesPerTile);
				}
				pSrc += wLightValuesPerTile;
				pDest += wLightValuesPerTile;
			}
		}

		ASSERT(pSrc == this->psRoomLight + wSize);
		ASSERT(pDest == this->psDisplayedLight + wSize);

		//If more light must be added to the display buffer,
		//a copy of the final room light buffer must be preserved
		//so it doesn't have to be recalculated each turn.
		//Same thing for ceiling lights below.
		if (IsPlayerLightRendered())
			memcpy(this->psRoomLight, this->psDisplayedLight, wSize * sizeof(LIGHTTYPE));
	}

	//2. Add ceiling light, either to the blurred room light buffer
	//or the display buffer if no other light will be added.
	if (!this->pRoom->tileLights.empty())
	{
		for (UINT wY = 0; wY < this->pRoom->wRoomRows; ++wY)
			for (UINT wX = 0; wX < this->pRoom->wRoomCols; ++wX)
			{
				const UINT lightVal = this->pRoom->tileLights.GetAt(wX,wY);
				if (bIsLightTileValue(lightVal))
				{
					//Add lighting from this tile.
					const UINT wStartIndex = this->pRoom->ARRAYINDEX(wX,wY) * wLightValuesPerTile;
					LIGHTTYPE *pSrc = this->psCeilingLight + wStartIndex;
					LIGHTTYPE *pDest = pCeilingDestBuffer + wStartIndex;
					const LIGHTTYPE *pSrcTileEnd = pSrc + wLightValuesPerTile;
					while (pSrc != pSrcTileEnd)
					{
						*(pDest++) += *(pSrc++);
						*(pDest++) += *(pSrc++);
						*(pDest++) += *(pSrc++);
					}
				}
			}
	}
}

//*****************************************************************************
void CRoomWidget::LowPassLightFilter(
//Blurs a light map using a low-pass filter.
//
//Params:
	LIGHTTYPE *pSrc,  //(in) initial buffer
	LIGHTTYPE *pDest, //(out) blurred buffer
	const bool bLeft, const bool bRight, //(in) whether there is a tile that may
	const bool bUp, const bool bDown)    //be weighed in on this edge
const
{
	//Low-pass filter.
	static const UINT FILTER[9] = {0,5,0, 5,12,5, 0,5,0}; //kernel
	static const UINT FILTER_SUM = 32; //sum to a power of two for speed
	static const UINT OFFSET[9] = { //address offset to these locations in another tile
		0,LIGHT_SPT*LIGHT_BPP,0,
		(LIGHT_SPT*(LIGHT_SPT-1)+1)*LIGHT_BPP,0,(LIGHT_SPT*(LIGHT_SPT-1)+1)*LIGHT_BPP,
		0,LIGHT_SPT*LIGHT_BPP,0
	}; //!!diagonals not implemented

	const UINT wVertOffset = wLightValuesPerTile * (this->pRoom->wRoomCols-1);
	UINT i, j, k;

	for (j=0; j<LIGHT_SPT; ++j)
		for (i=0; i<LIGHT_SPT; ++i)
		{
			//Process one light pixel.
			for (k=3; k--; )
			{
				UINT dwSum = *pSrc * FILTER[4];  //center
				//left+right
				switch (i)
				{
					case 0:
						//mix in adjacent tile if not at edge, otherwise use the center (i.e. current) value
						dwSum += bLeft ? *(pSrc-OFFSET[3]) * FILTER[3] : *pSrc;
						dwSum += *(pSrc+LIGHT_BPP) * FILTER[5];
					break;
					default:
						dwSum += *(pSrc-LIGHT_BPP) * FILTER[3]; //look in same tile
						dwSum += *(pSrc+LIGHT_BPP) * FILTER[5];
					break;
					case LIGHT_SPT-1:
						dwSum += *(pSrc-LIGHT_BPP) * FILTER[3];
						dwSum += bRight ? *(pSrc+OFFSET[5]) * FILTER[5] : *pSrc;
					break;
				}
				//up+down
				switch (j)
				{
					case 0:
						dwSum += bUp ? *(pSrc-OFFSET[1]-wVertOffset) * FILTER[1] : *pSrc;
						dwSum += *(pSrc+LIGHT_BPP*LIGHT_SPT) * FILTER[7];
					break;
					default:
						dwSum += *(pSrc-LIGHT_BPP*LIGHT_SPT) * FILTER[1];
						dwSum += *(pSrc+LIGHT_BPP*LIGHT_SPT) * FILTER[7];
					break;
					case LIGHT_SPT-1:
						dwSum += *(pSrc-LIGHT_BPP*LIGHT_SPT) * FILTER[1];
						dwSum += bDown ? *(pSrc+OFFSET[7]+wVertOffset) * FILTER[7] : *pSrc;
					break;
				}

				dwSum /= FILTER_SUM;
				*(pDest++) = dwSum;
				++pSrc;
			}
		}
}

//*****************************************************************************
float CRoomWidget::getTileElev(const UINT i, const UINT j) const
//Return height info about this tile relevant for modeling.
{
	ASSERT(this->pRoom);
	if (!this->pRoom->IsValidColRow(i,j))
		return 0.0;

	return getTileElev(this->pRoom->GetOSquare(i,j));
}

float CRoomWidget::getTileElev(const UINT wOTile) const
{
	switch (wOTile)
	{
		case T_PIT: case T_PIT_IMAGE:
		case T_PLATFORM_P: //so light isn't drawn where platforms are rendered
			return -2.0f;

		case T_WALL: case T_WALL2:	case T_WALL_IMAGE:
		case T_WALL_B: case T_WALL_H:	case T_WALL_M:
		case T_DOOR_Y:	case T_DOOR_M:	case T_DOOR_C:	case T_DOOR_R:	case T_DOOR_B:
			return 1.0f;

		case T_WATER: case T_SHALLOW_WATER:
		case T_PLATFORM_W:
		case T_STAIRS_UP: case T_STAIRS:
			return 0.0f;

		case T_BRIDGE: case T_BRIDGE_H: case T_BRIDGE_V:
		case T_STEP_STONE:
		case T_GOO: case T_HOT: case T_PRESSPLATE:
		case T_TUNNEL_N: case T_TUNNEL_S: case T_TUNNEL_E: case T_TUNNEL_W:
		case T_DOOR_YO: case T_DOOR_GO: case T_DOOR_CO:	case T_DOOR_RO: case T_DOOR_BO:
		case T_FLOOR_ROAD: case T_FLOOR_GRASS:	case T_FLOOR_DIRT: case T_FLOOR_ALT:
		case T_FLOOR_M: case T_FLOOR:	case T_FLOOR_IMAGE:
		default:
			return 0.0f;
	}
}

//*****************************************************************************
void CRoomWidget::modelVertTileface(
	const float elev, //height above/below floor
	const UINT i, const UINT j,  //where to render
	const bool bXAxis,  //whether tileface is in east-west (x) or north-south (y) direction
	const bool bNorthernWall)
{
	ASSERT(elev > 0.0);  //we're actually modeling a wall

	const Point p1((float)i, j + (bNorthernWall?fNorthWallYCoord:0.0f), 0),
		p2(i + (bXAxis?1.0f:0.0f), j + (bXAxis?(bNorthernWall?fNorthWallYCoord:0.0f):1.0f), elev);
	SceneRect *pTile = new SceneRect(p1,p2);
	this->model.addObject(pTile);
}

//*****************************************************************************
void CRoomWidget::PropagateLight(const int nSX, const int nSY, const UINT tParam)
//Radiate out a light source onto the room tiles from its given position in the room,
//taking room geometry into account.
{
	ASSERT(this->pRoom->IsValidColRow(nSX,nSY));

	if (bLightOff(tParam))
		return; //light is off

	const UINT wLightColor = calcLightType(tParam);

	const float fZ = getTileElev(this->pRoom->GetOSquare(nSX, nSY));
	//Center of tile (slightly offset, to avoid rounding instability)
	const float fX = nSX+0.5f, fY = nSY+0.501f+Y_LIGHT_OFFSET_KLUDGE;
	PointLightObject light(
			Point(fX, fY, max(0.0f,fZ)+0.9f), //never in pit, and close to ceiling
			Color(lightMap[0][wLightColor], lightMap[1][wLightColor], lightMap[2][wLightColor]),
			Color()); //falloff unused

	//Light's radius.
	const int nMaxDistance = 1+calcLightRadius(tParam);
	light.fMaxDistance = (float)nMaxDistance + 0.3f; //vertical component

	//The light source's tile itself.
	CastLightOnTile(nSX, nSY, nSX, nSY, &light);

	//Cast light outward to the maximum range.
	//Process one quadrant at a time in order to maintain a compact area model.
	int dist, rad;
	//NW quadrant
	RenderRoomModel(nSX, nSY, nSX - nMaxDistance, nSY - nMaxDistance);  //model this quadrant
	for (dist=1; dist<=nMaxDistance; ++dist) {
		//1st. Axial.
		CastLightOnTile(nSX, nSY, nSX - dist, nSY, &light);
		CastLightOnTile(nSX, nSY, nSX, nSY - dist, &light);
		//2nd. Diagonal.
		for (rad=1; rad<dist; ++rad) {
			CastLightOnTile(nSX, nSY, nSX - dist, nSY - rad, &light);
			CastLightOnTile(nSX, nSY, nSX - rad, nSY - dist, &light);
		}
		//3rd. Corner.
		CastLightOnTile(nSX, nSY, nSX - dist, nSY - dist, &light);
	}
	//NE quadrant
	RenderRoomModel(nSX, nSY, nSX + nMaxDistance, nSY - nMaxDistance);
	for (dist=1; dist<=nMaxDistance; ++dist) {
		CastLightOnTile(nSX, nSY, nSX + dist, nSY, &light);
		for (rad=1; rad<dist; ++rad) {
			CastLightOnTile(nSX, nSY, nSX + dist, nSY - rad, &light);
			CastLightOnTile(nSX, nSY, nSX + rad, nSY - dist, &light);
		}
		CastLightOnTile(nSX, nSY, nSX + dist, nSY - dist, &light);
	}
	//SW quadrant
	RenderRoomModel(nSX, nSY, nSX - nMaxDistance, nSY + nMaxDistance);
	for (dist=1; dist<=nMaxDistance; ++dist) {
		CastLightOnTile(nSX, nSY, nSX, nSY + dist, &light);
		for (rad=1; rad<dist; ++rad) {
			CastLightOnTile(nSX, nSY, nSX - dist, nSY + rad, &light);
			CastLightOnTile(nSX, nSY, nSX - rad, nSY + dist, &light);
		}
		CastLightOnTile(nSX, nSY, nSX - dist, nSY + dist, &light);
	}
	//SE quadrant
	RenderRoomModel(nSX, nSY, nSX + nMaxDistance, nSY + nMaxDistance);
	for (dist=1; dist<=nMaxDistance; ++dist) {
		for (rad=1; rad<dist; ++rad) {
			CastLightOnTile(nSX, nSY, nSX + dist, nSY + rad, &light);
			CastLightOnTile(nSX, nSY, nSX + rad, nSY + dist, &light);
		}
		CastLightOnTile(nSX, nSY, nSX + dist, nSY + dist, &light);
	}
}

//*****************************************************************************
void CRoomWidget::PropagateLightNoModel(const int nSX, const int nSY, const UINT tParam)
//Radiate out a light source onto the room tiles from its given position in the room.
{
	ASSERT(this->pRoom->IsValidColRow(nSX,nSY));

	if (bLightOff(tParam))
		return; //light is off

	const UINT wLightColor = calcLightType(tParam);

	const float fZ = 0.0; //always on the floor
	//Center of tile (slightly offset, to avoid rounding instability)
	const float fX = nSX+0.5f, fY = nSY+0.501f+Y_LIGHT_OFFSET_KLUDGE;
	PointLightObject light(
			Point(fX, fY, max(0.0f,fZ)+0.9f), //never in pit, and close to ceiling
			Color(lightMap[0][wLightColor], lightMap[1][wLightColor], lightMap[2][wLightColor]),
			Color()); //falloff unused

	//Light's radius.
	const int nMaxDistance = 1+calcLightRadius(tParam);
	light.fMaxDistance = (float)nMaxDistance + 0.3f; //vertical component

	this->model.clear();

	//Cast light outward to the maximum range.
	int nXdist, nYdist;
	for (nYdist=-nMaxDistance; nYdist<=nMaxDistance; ++nYdist)
		for (nXdist=-nMaxDistance; nXdist<=nMaxDistance; ++nXdist)
			CastLightOnTile(nSX, nSY, nSX + nXdist, nSY + nYdist, &light, false);
}

//*****************************************************************************
void CRoomWidget::RemoveHighlight()
//Reset tile marked for custom highlighting.
{
	this->wHighlightX = this->wHighlightY = UINT(-1);
	this->cursorLight.wX = this->cursorLight.wY = UINT(-1);
}

//*****************************************************************************
void CRoomWidget::RenderRoomModel(const int nX1, const int nY1, const int nX2, const int nY2)
//Generate 3d model of room area (i.e. simple obstructing surfaces, like walls and spheres).
{
	int nMinX = min(nX1,nX2), nMinY = min(nY1,nY2);
	int nMaxX = max(nX1,nX2), nMaxY = max(nY1,nY2);
	if (nMinX < 0) nMinX = 0;
	if (nMinY < 0) nMinY = 0;
	if ((UINT)nMaxX >= this->pRoom->wRoomCols) nMaxX = this->pRoom->wRoomCols-1;
	if ((UINT)nMaxY >= this->pRoom->wRoomRows) nMaxY = this->pRoom->wRoomRows-1;

	this->model.init(nMinX, nMinY, nMaxX, nMaxY);

	UINT i,j;
	for (j=nMinY; j<=(UINT)nMaxY; ++j)
	{
		//Render a row.
		float maxElev, elev, northElev, westElev = getTileElev(nMinX-1, j);

		//Northern-facing walls (i.e. there's no more wall to the north past this wall tile)
		//extend only part of a tile back/north.
		bool bNorthernWall, bLastWallIsNorthern;

		for (i=nMinX; i<=(UINT)nMaxX; ++i)
		{
			//Get room tile type and matching texture.
			elev = getTileElev(i, j);

			const UINT wTTile = this->pRoom->GetTSquare(i, j);
			switch (wTTile)
			{
				case T_ORB:
				case T_BOMB:
				{
					Sphere *pOrb = new Sphere(Point(i+0.5f, j+0.5f,
							max(0.0f,elev) + orbRadius), orbRadius);
					this->model.addObject(pOrb);
				}
				break;
			}

			//west/east edge
			if (i>UINT(nMinX))
			{
				if (westElev != elev)
				{
					maxElev = max(westElev, elev);
					if (maxElev > 0.0)
					{
						bNorthernWall =
							(westElev > 0.0 && getTileElev(i-1,j-1) <= 0.0) ||
							(elev > 0.0 && getTileElev(i,j-1) <= 0.0);
						modelVertTileface(maxElev, i, j, false, bNorthernWall);
					}
				} else {
					//If this wall is northern facing and last one is not, or v.v.,
					//then a partial wall piece must be added.
					bLastWallIsNorthern = westElev > 0.0 && getTileElev(i-1,j-1) <= 0.0;
					bNorthernWall = elev > 0.0 && getTileElev(i,j-1) <= 0.0;
					if (bLastWallIsNorthern != bNorthernWall)
					{
						maxElev = max(westElev, elev);
						const Point p1((float)i, (float)j, 0),
							p2((float)i, j + fNorthWallYCoord, maxElev);
						SceneRect *pTile = new SceneRect(p1,p2);
						this->model.addObject(pTile);
					}
				}
			}

			//north/south edge
			if (j>UINT(nMinY))
			{
				northElev = getTileElev(i, j-1);
				if (northElev != elev)
				{
					maxElev = max(northElev, elev);
					if (maxElev > 0.0)
					{
						bNorthernWall = northElev <= 0.0;
						modelVertTileface(maxElev, i, j, true, bNorthernWall);
					}
				}
			}
			westElev = elev;
		}
	}

	this->model.ready();  //done adding to the model
}

//*****************************************************************************
void CRoomWidget::SetCeilingLight(const UINT wX, const UINT wY)
//Set light values on tile (x,y) in the ceiling light map.
{
	const float fLightMultiplier = fMaxLightIntensity[this->wDark] * 256.0f;
	ASSERT(this->pRoom->tileLights.GetAt(wX,wY));
	const UINT wLightIndex = this->pRoom->tileLights.GetAt(wX,wY);
	ASSERT(bIsLightTileValue(wLightIndex)); //this should be a light tile, not a dark tile
	const UINT wLightColor = calcLightType(wLightIndex-1);
	const LIGHTTYPE R = static_cast<LIGHTTYPE>(lightMap[0][wLightColor] * fLightMultiplier);
	const LIGHTTYPE G = static_cast<LIGHTTYPE>(lightMap[1][wLightColor] * fLightMultiplier);
	const LIGHTTYPE B = static_cast<LIGHTTYPE>(lightMap[2][wLightColor] * fLightMultiplier);

	//Add light on edges if another light tile is next that edge.
	bool bWestLight = true, bNorthLight = true, bEastLight = true, bSouthLight = true;
	UINT lightVal;
	if (wX)
	{
		lightVal = this->pRoom->tileLights.GetAt(wX-1,wY);
		bWestLight = bIsLightTileValue(lightVal);
	}
	if (wY)
	{
		lightVal = this->pRoom->tileLights.GetAt(wX,wY-1);
		bNorthLight = bIsLightTileValue(lightVal);
	}
	if (wX<this->pRoom->wRoomCols-1)
	{
		lightVal = this->pRoom->tileLights.GetAt(wX+1,wY);
		bEastLight = bIsLightTileValue(lightVal);
	}
	if (wY<this->pRoom->wRoomRows-1)
	{
		lightVal = this->pRoom->tileLights.GetAt(wX,wY+1);
		bSouthLight = bIsLightTileValue(lightVal);
	}

	LIGHTTYPE *psL = this->psCeilingLight + (wY * this->pRoom->wRoomCols + wX) * wLightValuesPerTile;
	bool bRender = true;
	for (UINT j=0; j<LIGHT_SPT; ++j)
	{
		if (j==0 && !bNorthLight)
		{
			psL += LIGHT_SPT*LIGHT_BPP; //advance to next row
			continue;
		}
		if (j==LIGHT_SPT_MINUSONE && !bSouthLight)
			break; //done
		for (UINT i=0; i<LIGHT_SPT; ++i)
		{
			if (i==0)
			{
				if (!bWestLight)
					bRender = false;
				//Handle west corner values.
				else if (j==0 && wX>0 && wY>0)
				{
					lightVal = this->pRoom->tileLights.GetAt(wX-1,wY-1);
					if (!bIsLightTileValue(lightVal))
						bRender = false;
				}
				else if (j==LIGHT_SPT_MINUSONE && wX>0 && wY<this->pRoom->wRoomRows-1)
				{
					lightVal = this->pRoom->tileLights.GetAt(wX-1,wY+1);
					if (!bIsLightTileValue(lightVal))
						bRender = false;
				}
			}
			else if (i==LIGHT_SPT_MINUSONE)
			{
				if (!bEastLight)
					bRender = false;
				//Handle east corner values.
				else if (j==0 && wX<this->pRoom->wRoomCols-1 && wY>0)
				{
					lightVal = this->pRoom->tileLights.GetAt(wX+1,wY-1);
					if (!bIsLightTileValue(lightVal))
						bRender = false;
				}
				else if (j==LIGHT_SPT_MINUSONE && wX<this->pRoom->wRoomCols-1 &&
						wY<this->pRoom->wRoomRows-1)
				{
					lightVal = this->pRoom->tileLights.GetAt(wX+1,wY+1);
					if (!bIsLightTileValue(lightVal))
						bRender = false;
				}
			}

			if (!bRender)
			{
				psL += LIGHT_BPP; //advance to next pixel
				bRender = true;   //reset for next pixel
			} else {
				*(psL++) = R;
				*(psL++) = G;
				*(psL++) = B;
			}
		}
	}
}

//*****************************************************************************
bool CRoomWidget::SkyWillShow() const
//Returns: true if room contains some tile that shows the sky/ceiling
{
	for (UINT wRow = this->pRoom->wRoomRows; wRow--; )
		for (UINT wCol = this->pRoom->wRoomCols; wCol--; )
		{
			//These tiles show open sky.
			const UINT wOSquare = this->pRoom->GetOSquare(wCol, wRow);
			if (bIsWater(wOSquare) || bIsSteppingStone(wOSquare) || wOSquare == T_PLATFORM_W)
				return true;
		}

	return false;
}

//*****************************************************************************
void CRoomWidget::SynchRoomToCurrentGame()
//Keep room object synched to current game's room object.
{
	if (this->pCurrentGame)
		if (this->pCurrentGame->pRoom != this->pRoom)
		{
			this->pRoom = this->pCurrentGame->pRoom;
			LoadRoomImages();
		}
}

//*****************************************************************************
bool CRoomWidget::UpdateDrawSquareInfo(
//Update square drawing information arrays for a room.
//
//Params:
	const CCoordSet *pSet,	//(in) squares which were replotted [default=NULL]
	const CCoordSet *pGeometryChanges) //(in) tiles where room geometry changed, which affects lighting [default=NULL]
//Returns:
//True if successful, false if not (out of memory error).
{
	if (!this->pRoom)
		return true;

	const UINT wSquareCount = this->pRoom->CalcRoomArea();
	UINT wIndex;

	//If the square count changed, then I need to realloc arrays.
	if (wSquareCount != this->dwLastDrawSquareInfoUpdateCount)
	{
		if (!SetupDrawSquareInfo())
			return false;
		this->dwLastDrawSquareInfoUpdateCount = wSquareCount;
	}

	//Set tile image elements of arrays.
	UINT *pwO = this->pwOSquareTI;
	UINT *pwF = this->pwFSquareTI;
	UINT *pwT = this->pwTSquareTI;
	UINT *pwS = this->pwShadowTI;
	EDGES *pbE = this->pbEdges;
	TILEINFO *pbMI = this->pTileInfo;
	const char *pucO = this->pRoom->pszOSquares;
	const char *pucF = this->pRoom->pszFSquares;
	const char *pucT = this->pRoom->pszTSquares;
	const UINT wRows = this->pRoom->wRoomRows, wCols = this->pRoom->wRoomCols;
	UINT wTileImage, wLightVal, wTile;
	EDGETYPE drawEdge;
	bool bHalfWall;

	//Reset complex tile masks / light values.
	//Note: This can't be done along with the shadow and recalc arrays since these values
	//are not added to the tiles in order.
	for (wIndex = wSquareCount; wIndex--; )
		this->shadows[wIndex].clear();

	//If room viewing method changed, dirty relevant tiles.
	if (this->bLastVision != this->pRoom->bBetterVision)
	{
		this->bLastVision = this->pRoom->bBetterVision;

		//Dirty all tarstuff.
		for (wIndex = wSquareCount; wIndex--; )
			if (bIsTar(pucT[wIndex]))
				pbMI[wIndex].dirty = 1;
	}

	//Determine which tiles needed to be recalculated.
	CCoordIndex recalc(this->pRoom->wRoomCols, this->pRoom->wRoomRows, this->bAllDirty || !pSet ? 1 : 0);
	if (pSet && !this->bAllDirty)
	{
		for (CCoordSet::const_iterator tile = pSet->begin(); tile != pSet->end(); ++tile)
		{
			//Mark this tile for redraw.
			const int nX = tile->wX, nY = tile->wY;
			pbMI[this->pRoom->ARRAYINDEX(nX, nY)].dirty = 1;

			//For each plotted tile, all adjacent tiles within two units possibly
			//need to be refreshed (rerendered).  If this changes in the future,
			//a more sophisticated marking scheme might need to be used.
			int nI, nJ;
			for (nJ = nY-2; nJ <= nY+2; ++nJ)
				for (nI = nX-2; nI <= nX+2; ++nI)
					if (this->pRoom->IsValidColRow(nI, nJ))
						recalc.Add(nI,nJ);
		}
	}

	//If the room geometry possibly changed adjacent to where lights shine,
	//then refresh the room light model.
	//(This set is a subset of 'pSet'.)
	if (pGeometryChanges && !this->bAllDirty)
	{
		for (CCoordSet::const_iterator tile = pGeometryChanges->begin(); tile != pGeometryChanges->end(); ++tile)
		{
			const int nX = tile->wX, nY = tile->wY;

			//Tiles below ground level don't ever affect lighting.
			UINT wOTile = this->pRoom->GetOSquare(nX,nY);
			const bool bNotPit = !(bIsPit(wOTile) || bIsWater(wOTile) ||
					bIsSteppingStone(wOTile) || bIsPlatform(wOTile));
			if (bNotPit)
			{
				int nI, nJ;
				for (nJ = nY-1; nJ <= nY+1; ++nJ)
					for (nI = nX-1; nI <= nX+1; ++nI)
						if (this->pRoom->IsValidColRow(nI, nJ))
						{
							//Tiles below ground level don't ever affect lighting.
							wOTile = this->pRoom->GetOSquare(nI,nJ);
							if (!(bIsPit(wOTile) || bIsWater(wOTile) ||
								bIsSteppingStone(wOTile) || bIsPlatform(wOTile)) &&
									//If light was drawn here last turn, then
									//this plot might have affected the light casting.
									(this->lightedPlayerTiles.has(nI,nJ) || this->lightedRoomTiles.has(nI,nJ)))
								this->bRenderRoomLight = true;
						}
			}
		}
	}

	//Re-model the room lighting when something has changed.
	const bool bRenderLights = this->bRenderRoomLight && IsLightingRendered();
	if (bRenderLights)
	{
		//Erase old room lighting and prepare for new lighting.
		const UINT memSize = wSquareCount * wLightBytesPerTile;
		memset(this->psRoomLight, 0, memSize);
		if (this->bAllDirty)
			memset(this->psPlayerLight, 0, memSize); //reset the old player light buffer

		//Refresh tiles that used to be lit.
		if (!this->lightedRoomTiles.empty())
		{
			if (!this->bAllDirty)
			{
				//Only need to refresh tiles that used to have light drawn on them.
				for (CCoordSet::const_iterator tile = this->lightedRoomTiles.begin();
						tile != this->lightedRoomTiles.end(); ++tile)
					this->pTileInfo[this->pRoom->ARRAYINDEX(tile->wX,tile->wY)].dirty = 1;
			}

			this->lightedRoomTiles.clear();
			this->partialLightedTiles.clear();
			this->tileLightInfo.Clear();
		}

		this->pActiveLight = this->psRoomLight; //write to room light buffer
		this->pActiveLightedTiles = &this->lightedRoomTiles;
	}

	wIndex = 0;
	for (UINT wRow = 0; wRow < wRows; ++wRow)
	{
		for (UINT wCol = 0; wCol < wCols; ++wCol, ++wIndex)
		{
			//Retain old TILEINFO data.  More tiles might be dirtied below.
			if (recalc.Exists(wCol, wRow))
			{

			//Calculate aesthetic tile edges.
			//If existence of an edge changes, tile must be redrawn.
			if (wRow > 0)
			{
				drawEdge = CalcEdge(*pucO, *(pucO - wCols),N);
				if (drawEdge != pbE->north)
				{
					pbMI->dirty = 1;
					pbE->north = drawEdge;
					//Touch up corners.
					if (wCol > 0) (pbMI-1)->dirty = 1;
					if (wCol < wCols-1) (pbMI+1)->dirty = 1;
				}
			}

			if (wCol > 0)
			{
				drawEdge = CalcEdge(*pucO,*(pucO - 1),W);
				if (drawEdge != pbE->west)
				{
					pbMI->dirty = 1;
					pbE->west = drawEdge;
					//Touch up corners.
					if (wRow > 0) (pbMI-wCols)->dirty = 1;
					if (wRow < wRows-1) (pbMI+wCols)->dirty = 1;
				}
			}

			if (wCol < wCols-1)
			{
				drawEdge = CalcEdge(*pucO,*(pucO + 1),E);
				if (drawEdge != pbE->east)
				{
					pbMI->dirty = 1;
					pbE->east = drawEdge;
					//Touch up corners.
					if (wRow > 0) (pbMI-wCols)->dirty = 1;
					if (wRow < wRows-1) (pbMI+wCols)->dirty = 1;
				}
			}

			if (wRow < wRows-1)
			{
				drawEdge = CalcEdge(*pucO,*(pucO + wCols),S);
				if (drawEdge != pbE->south)
				{
					pbMI->dirty = 1;
					pbE->south = drawEdge;
					//Touch up corners.
					if (wCol > 0) (pbMI-1)->dirty = 1;
					if (wCol < wCols-1) (pbMI+1)->dirty = 1;
				}
			}

			//Calculate o-layer tiles
			//If tile changes, it must be redrawn.
			wTile = *pucO;

			//Tiles that aren't drawn to the room snapshot because they are animated.
			if (bIsPlatform(wTile))
				wTile = this->pRoom->coveredOSquares.GetAt(wCol, wRow);

			wTileImage = GetTileImageForTileNo(wTile);
			if (wTileImage == CALC_NEEDED)
				wTileImage = CalcTileImageForOSquare(this->pRoom, wCol, wRow);
			if (wTileImage != *pwO)
			{
				pbMI->dirty = 1;

				//Must "undraw" the trapdoor/bridge edge on the tile below when it falls.
				if ((*pwO == TI_TRAPDOOR || *pwO == TI_TRAPDOOR2 || *pwO == TI_BRIDGE ||
						*pwO == TI_BRIDGE_H || *pwO == TI_BRIDGE_V) && wRow < wRows-1)
					(pbMI+wCols)->dirty = 1;

				*pwO = wTileImage;
			}

			//Calculate whether special textures will be drawn on tile.
			bHalfWall = ((
				(wTileImage >= TI_WALL_SW && wTileImage <= TI_WALL_NS) ||
				(wTileImage >= TI_WALL_S && wTileImage <= TI_WALL_S3)  ||
				(wTileImage >= TI_WALL_BSW && wTileImage <= TI_WALL_BNS) ||
				(wTileImage >= TI_WALL_BS && wTileImage <= TI_WALL_BS3) ||
				(wTileImage >= TI_WALL_HSW && wTileImage <= TI_WALL_HNS) ||
				(wTileImage >= TI_WALL_HS && wTileImage <= TI_WALL_HS3) ) &&
					GetWallTypeAtSquare(this->pRoom, wCol, wRow + 1) == WALL_INNER);
			if (bHalfWall != pbE->bHalfWall)
			{
				pbMI->dirty = 1;
				pbE->bHalfWall = !pbE->bHalfWall;
			}

			//Calculate pit edge info.
			if (bIsPit(*pucO) || *pucO == T_PLATFORM_P)
			{
				const UINT wPitX = pbE->wPitX, wPitY = pbE->wPitY, wPitRem = pbE->wPitRemaining;
				CalcTileCoordForPit(this->pRoom, wCol, wRow, pbE->wPitX, pbE->wPitY, pbE->wPitRemaining);
				if (wPitX != pbE->wPitX || wPitY != pbE->wPitY || wPitRem != pbE->wPitRemaining)
					pbMI->dirty = 1;
			}

			//Calculate f-layer tiles
			//If tile changes, it must be redrawn.
			wTileImage = GetTileImageForTileNo(*pucF);
			ASSERT(wTileImage != CALC_NEEDED);
			if (wTileImage != *pwF)
			{
				pbMI->dirty = 1;
				*pwF = wTileImage;
			}

			//Calculate t-layer tiles
			//If tile changes, it must be redrawn.
			wTileImage = GetTileImageForTileNo(*pucT);
			if (wTileImage == CALC_NEEDED)
				wTileImage = CalcTileImageForTSquare(this->pRoom, wCol, wRow);
			if (wTileImage != *pwT)
			{
				pbMI->dirty = 1;
				*pwT = wTileImage;
			}

			}	//recalc

			//Keep track of where wall shadows are being cast.
			//If shadow changes, tile must be redrawn.
			if (this->wDark || !bShowsShadow(*pucO))	//no shadows in dark rooms
			{
				//These objects show no shadow.
				if (*pwS != TI_UNSPECIFIED)
				{
					pbMI->dirty = 1;
					*pwS = TI_UNSPECIFIED;
				}
			} else {
				//Shadows can fall on this tile.
				UINT wShadow = CalcTileImagesForWallShadow(this->pRoom, wCol, wRow);
				if (wShadow != TI_UNSPECIFIED)
					this->shadows[wIndex].push_back(wShadow);
				if (wShadow != *pwS)
				{
					pbMI->dirty = 1;
					*pwS = wShadow;
				}
			}

			//Compile set of all obstacle shadows.
			if (*pucT == T_OBSTACLE && !this->wDark)	//no shadows in dark rooms
				AddObstacleShadowMask(wCol,wRow);

			//Light sources.
			//T-layer lights and wall lights are handled here.
			if (bRenderLights)
			{
				if (bIsLight(*pucT))
					PropagateLight(wCol, wRow, this->pRoom->GetTParam(wCol, wRow));

				wLightVal = this->pRoom->tileLights.GetAt(wCol, wRow);
				if (bIsWallLightValue(wLightVal))
					PropagateLightNoModel(wCol, wRow, wLightVal);
			}

			//Monster tiles might change, but most of these are taken care of
			//in DirtySpriteTiles() as they move.  Cases not taken care of
			//are (1) when a monster is killed without an effect to dirty its tile,
			//or (2) when an entire long monster (e.g. serpent) is destroyed at once.

			//Advance to next square.
			++pbE;
			++pwO;
			++pwF;
			++pwS;
			++pwT;
			++pbMI;
			++pucO;
			++pucF;
			++pucT;
		}
	}

	//I'm expecting pointers to have traversed entire size of their arrays--
	//no more, no less.
	ASSERT(pbE == this->pbEdges + wSquareCount);
	ASSERT(pwO == this->pwOSquareTI + wSquareCount);
	ASSERT(pwF == this->pwFSquareTI + wSquareCount);
	ASSERT(pwS == this->pwShadowTI + wSquareCount);
	ASSERT(pwT == this->pwTSquareTI + wSquareCount);
	ASSERT(pbMI == this->pTileInfo + wSquareCount);
	ASSERT(pucO == this->pRoom->pszOSquares + wSquareCount);
	ASSERT(pucF == this->pRoom->pszFSquares + wSquareCount);
	ASSERT(pucT == this->pRoom->pszTSquares + wSquareCount);

	if (bRenderLights)
	{
		//Done rendering room lighting.  Prepare for display.
		ProcessLightmap();
		this->bRenderPlayerLight = IsPlayerLightRendered(); //add player light to room display also
		this->bRenderRoomLight = false;

		//If the room currently has no lights turned on, but there were lights
		//that were just turned off, then 'this->psDisplayedLight' will be used for
		//rendering the player light directly.  In this case, we want to reset
		//the light values on the display buffer where the lights that just got
		//turned off used to be shining.
		const bool bRoomHasLighting = !this->pRoom->tileLights.empty() || !this->lightedRoomTiles.empty();
		if (!bRoomHasLighting && this->bRenderPlayerLight)
		{
			CCoordSet clearedTiles;
			for (CCoordSet::const_iterator tile = this->pRoom->disabledLights.begin();
					tile != this->pRoom->disabledLights.end(); ++tile)
			{
				//Just reset everywhere within the disabled light's range.
				const UINT wX = tile->wX, wY = tile->wY;
				const BYTE tParam = this->pRoom->GetTParam(wX,wY);
				const int nMaxDistance = 1+calcLightRadius(tParam);
				for (UINT y=wY-nMaxDistance; y<=wY+nMaxDistance; ++y)
					for (UINT x=wX-nMaxDistance; x<=wX+nMaxDistance; ++x)
						if (this->pRoom->IsValidColRow(x,y) && !clearedTiles.has(x,y))
						{
							const UINT wStartIndex = this->pRoom->ARRAYINDEX(x,y) * wLightValuesPerTile;
							memset(this->psDisplayedLight + wStartIndex, 0, wLightBytesPerTile);
							clearedTiles.insert(x,y);
						}
			}
		}
	}

	this->bRenderRoom = true;  //ready to refresh room image

	return true;
}

bool CRoomWidget::SetupDrawSquareInfo()
{
	const UINT wSquareCount = this->pRoom->CalcRoomArea();

	DeleteArrays();

	//Allocate new tile image arrays.
	this->pwOSquareTI = new UINT[wSquareCount];
	this->pwFSquareTI = new UINT[wSquareCount];
	this->pwTSquareTI = new UINT[wSquareCount];
//		this->pwMSquareTI = new UINT[wSquareCount];
	this->pwShadowTI = new UINT[wSquareCount];
	this->pbEdges = new EDGES[wSquareCount];
	this->shadows = new vector<UINT>[wSquareCount];
	this->pTileInfo = new TILEINFO[wSquareCount];
	this->psTempLightBuffer = new LIGHTTYPE[wSquareCount * wLightValuesPerTile];
	this->psCeilingLight = new LIGHTTYPE[wSquareCount * wLightValuesPerTile];
	this->psRoomLight = new LIGHTTYPE[wSquareCount * wLightValuesPerTile];
	this->psDisplayedLight = new LIGHTTYPE[wSquareCount * wLightValuesPerTile];
	this->psPlayerLight = new LIGHTTYPE[wSquareCount * wLightValuesPerTile];
	if (!(this->pwOSquareTI && this->pwFSquareTI && this->pwTSquareTI && //this->pwMSquareTI &&
			this->pwShadowTI && this->pbEdges &&
			this->psTempLightBuffer && this->psRoomLight &&
			this->psCeilingLight && this->psPlayerLight && this->psDisplayedLight))
	{
		DeleteArrays();
		return false;
	}

	//Init what might not be set below.
	ClearLights();
	memset(this->pwOSquareTI, TI_FLOOR, wSquareCount * sizeof(UINT));
	memset(this->pwFSquareTI, T_EMPTY, wSquareCount * sizeof(UINT));
	memset(this->pwTSquareTI, T_EMPTY, wSquareCount * sizeof(UINT));
//		memset(this->pwMSquareTI, 0, wSquareCount * sizeof(UINT));
	memset(this->pwShadowTI, TI_UNSPECIFIED, wSquareCount * sizeof(UINT));
	//Give each m-layer tile a random animation frame
	for (UINT wIndex = wSquareCount; wIndex--; )
		this->pTileInfo[wIndex].animFrame = RAND(2); //2 possible frames
	this->bAllDirty = true;

	return true;
}
