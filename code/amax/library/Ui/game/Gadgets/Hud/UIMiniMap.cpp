#include "stdafx.h"

#include "UIMiniMap.h"
#include "UiHelpers.h"

#include "UIMessages.h"
#include "UiXtConfig.h"

#include "Game/GameActors/Components/Messages/VehicleLogicMessages.h"
#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_NewScoring.h>

#include <AI/AI_Amax.h>
#include <game/Game.h>
#include <ui/Graphics/SimpleTimer.h>
#include <ui/Game/Gadgets/UIStaticShape.h>

#include<GameLogic\Layers\RaceLogicLayer.h>
#include<GameLogic\Pickups\PickupMineBase.h>

#include "GameLogic/RaceRules/RaceRulesManager.h"
#include <GameLogic/Structures/MpResults.h>
#include <GameLogic/RaceModes/RaceModes.h>
#include <GameLogic/RaceModes/RaceModeDestruction.h>

#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_NewScoring.h>
#include <camera2/stock/DebugFeed.h>
#include "Game/AmaxCam/CameraSystem.h"


#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

XT_TYPE( UI::UIMiniMap, "UIMiniMap" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_fVehicleWidth, "vehicle_width" )
	XT_FIELD( m_fVehicleHeight, "vehicle_height" )
	XT_FIELD( m_fClampFraction, "clamp_fraction" )
	XT_FIELD( m_fFadeFraction, "fade_fraction" )
	XT_FIELD( m_fMapAlpha, "map_alpha" )
	XT_FIELD( m_fBattleViewDist, "battle_view_dist" )
	XT_FIELD( m_fMinViewDist, "min_view_dist" )
	XT_FIELD( m_fMaxViewDist, "max_view_dist" )

	XT_FIELD( m_xtColourYou, "colour_you" )
	XT_FIELD( m_xtColourMap, "colour_map" )
	XT_FIELD( m_xtColourTargeted, "colour_targeted" )
	XT_FIELD( m_xtColourPulseWinner, "colour_pulse_winner" )
	XT_FIELD( m_xtColourPulseRival, "colour_pulse_rival" )
	XT_FIELD( m_xtColourDestructionEasy, "colour_destruction_easy" )
	XT_FIELD( m_xtColourDestructionMedium, "colour_destruction_medium" )
	XT_FIELD( m_xtColourDestructionHard, "colour_destruction_hard" )
	XT_FIELD( m_xtColourDestructionVHard, "colour_destruction_vhard" )

	XT_FIELD( m_xtColourFirst, "colour_first" )
	XT_FIELD( m_xtColourSecond, "colour_second" )
	XT_FIELD( m_xtColourThird, "colour_third" )
	XT_FIELD( m_xtColourNone, "colour_none" )

	XT_FIELD( m_xtColourVP0, "colour_vp0" )
	XT_FIELD( m_xtColourVP1, "colour_vp1" )
	XT_FIELD( m_xtColourVP2, "colour_vp2" )
	XT_FIELD( m_xtColourVP3, "colour_vp3" )
XT_END_TYPE

namespace UI
{

#ifndef CFG_GOLD
Debug::Bool g_MiniMapDrawDebug("UI.MiniMap.Render", false );
Debug::Bool g_MiniMapUseCamera("UI.MiniMap.UseCamera", true );
Debug::Bool g_MiniMapUseDebug("UI.MiniMap.UseDebug", false );
//Debug::Float g_MiniMapCentrePointX("UI.MiniMap.CentreX", -80.0f, Debug::Range(-1000.0f,1000.0f) );
//Debug::Float g_MiniMapCentrePointZ("UI.MiniMap.CentreZ", 880.0f, Debug::Range(-1000.0f,1000.0f) );
//Debug::Float g_MiniMapBoundsSizeX("UI.MiniMap.SizeX", 2800.0f, Debug::Range(1.0f,5000.0f) );
Debug::Float g_MiniMapCentrePointX("UI.MiniMap.CentreX", 13.325f, Debug::Range(-1000.0f,1000.0f) );
Debug::Float g_MiniMapCentrePointZ("UI.MiniMap.CentreZ", -833.91f, Debug::Range(-1000.0f,1000.0f) );
Debug::Float g_MiniMapBoundsSizeX("UI.MiniMap.SizeX", 2965.037f,  Debug::Range(1.0f,5000.0f) );
Debug::Float g_MiniMapAspect("UI.MiniMap.Aspect", 720.0f / 1280.0f );
Debug::Float g_MiniMapOffsetU("UI.MiniMap.OffsetU", 0.5f );
Debug::Float g_MiniMapOffsetV("UI.MiniMap.OffsetV", 0.5f );
Debug::Float g_MiniMapViewSize("UI.MiniMap.ViewSize", 1000.0f, Debug::Range(0.0f,3000.0f) );
#endif

Debug::Bool g_MiniMapRivalPulsingEnabled("UI.MiniMap.Rival.RivalPulsingEnabled", true );
Debug::Float g_MiniMapRivalPulseRateMultiplier("UI.MiniMap.Rival.MiniMapRivalPulseRateMultiplier", 1.0f, Debug::Range(0.0f,10.0f) );
Debug::Bool g_MiniMapWinnerPulsingEnabled("UI.MiniMap.Winner.WinnerPulsingEnabled", true );

Debug::Float g_MiniMapRivalBackingSizeMultiplier("UI.MiniMap.Rival.MiniMapRivalBackingSizeMult", 3.9f, Debug::Range(0.0f,10.0f) );

Debug::Float g_MiniMapRivalBackingAlphaMultiplier("UI.MiniMap.Rival.MiniMapRivalBackingAlphaMult", 1.0f, Debug::Range(0.0f,1.0f) );
Debug::Float g_MiniMapRivalBackingIntensityMultiplier("UI.MiniMap.Rival.MiniMapRivalBackingIntensityMult", 1.25f, Debug::Range(0.0f,1.0f) );

Debug::Bool g_MiniMapSinglePlayerPulsingEnabled("UI.MiniMap.SinglePlayerPulsingEnabled", false );
Debug::Bool g_MiniMapSinglePlayerFlagAsWinner("UI.MiniMap.SinglePlayerFlagAsWinner", false );
Debug::Bool g_MiniMapSinglePlayerFlagAsRival("UI.MiniMap.SinglePlayerFlagAsRival", false );

// ----------
// UIMiniMap
// ----------
void UIMiniMap::Init()
{
	C_GadgetBase::Init();

	// Clear
	m_sLocalPlayerIndex = -1;
	m_pDummyScale		= NULL;
	m_pDummyTranslate	= NULL;

	// Defaults
	m_fCurrentViewDist = 500.0f;
	m_fBattleViewDist = 50.0f;
	m_fTopSpeedInMph = 200.0f;
	m_bIsNormalView = true;
	m_bRouteShapeExists = false;
	m_fRivalIntensityTimer = 0.0f;

	// Cache stuff
	m_bSilentRunningHidesPlayersOnRadar = !C_MultiplayerData::Instance().m_pConfig->m_SilentRunningModSettings.m_VisibleOnRadar;


	// messages
	AddSubscriber( E_UiMsg_CtrlButton_RightThumb );
	AddSubscriber( E_UiMsg_FrameUpdate );
	AddSubscriber( E_UiGameMsg_SetLocalPlayerIndex );
}

void UIMiniMap::SetupRTGadgets()
{
	Util::C_String128 gadget_tmp;

	// Grab dummys
	gadget_tmp.Printf( "dmy_MiniMapScale%d", m_sLocalPlayerIndex );
	m_pDummyScale		= gButtons.FindByName( gadget_tmp.Chars() );

	gadget_tmp.Printf( "dmy_MiniMapTranslate%d", m_sLocalPlayerIndex );
	m_pDummyTranslate	= gButtons.FindByName( gadget_tmp.Chars() );

	AmaxRoute *pRoute  = C_GameData::Instance().mRoutes[(GAMEDATA_ID)UI::GetLevelState().mRoute];
	if( pRoute )
	{
		Maths::HVec3 min( pRoute->city->boundsMin.vx, 0.0f, pRoute->city->boundsMin.vy );
		Maths::HVec3 max( pRoute->city->boundsMax.vx, 0.0f, pRoute->city->boundsMax.vy );
		Maths::C_AxisBox box( min, max );

		m_MapOffset = box.Centre();
		m_MapSize = max - min;

		// Stop div0
		if( m_MapSize.X() <= 0.0f || m_MapSize.Z() <= 0.0f )
		{
			LOGWARN( g_Log, DBSTR " : Invalid map size ");
			m_MapSize = Maths::HVec3( 1000.0f, 0.0f, 1000.0f );
		}
		else
		{
			LOGDEBUG( g_Log, DBSTR " : Map is X:%5.2f Z:%5.2f S:%5.2f", (F32)m_MapOffset.X(), (F32)m_MapOffset.Z(), (F32)m_MapSize.X() );
		}
	}

	// Change route shapes
	C_GadgetBase* pGadget;

	Util::C_String128 city_tmp;
	city_tmp.Printf( "%s_game", (const char*)pRoute->city->tag );

	// Main route
	gadget_tmp.Printf( "shp_MiniMapRoad%d", m_sLocalPlayerIndex );
	pGadget = gButtons.FindByName( gadget_tmp.Chars() );

	if( pGadget && pGadget->GetType() == E_UIButtonType_StaticShape )
	{
		UIStaticShape* pShape = ((UIStaticShape*)pGadget);

		// City and route
		if( pShape->ChangeSceneName( city_tmp.Chars() ) && pShape->ChangeObjectName( (const char*)pRoute->tag ) )
			m_bRouteShapeExists = true;
	}

	// Alt route
	gadget_tmp.Printf( "shp_MiniMapRoad_Alt%d", m_sLocalPlayerIndex );
	pGadget = gButtons.FindByName( gadget_tmp.Chars() );

	if( pGadget && pGadget->GetType() == E_UIButtonType_StaticShape )
	{
		UIStaticShape* pShape = ((UIStaticShape*)pGadget);

		Util::C_String altRouteName( (const char*)pRoute->tag );
		altRouteName += "_Alt";

		// City and route
		if( !pShape->ChangeSceneName( city_tmp.Chars() ) || !pShape->ChangeObjectName( altRouteName.Chars() ) )
			pGadget->Activate(false);
	}
}

void UIMiniMap::PostInit()
{
	// Check for height changes & default camera mode
	m_bCheckHeight = false;
	Game::C_RaceBookLayer* pLayer = GetRaceBookLayer();
	if( pLayer && pLayer->GetGameModePtr()->IsMultiplayer() )
	{
		Game::C_RaceModeBase* pRaceMode = pLayer->GetRaceModePtr();
		m_bCheckHeight = ( pRaceMode && pRaceMode->GetID() == Game::E_RaceMode_Multiplayer_Destruction );
		m_bIsNormalView = !m_bCheckHeight;
	}

	m_RivalBackingRenderState = m_RenderState;
	m_RivalBackingRenderState.m_EffectIndex = E_Effect_IntensityAsAlpha;
}

void UIMiniMap::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	Game::Actors::RaceLogicCore* pCore = GetRLC();
	
	S32 sNumVehicles = 0;
	if ( pCore )
		sNumVehicles = (S32)pCore->GetNumVehicles();

	m_Sorter.Init( sNumVehicles );

	// Check for div0
	if( m_fFadeFraction <= 0.0f )
		m_fFadeFraction = 1.0f;
}

void UIMiniMap::FrameUpdate()
{
	if ( !m_Activated )
		return;

	// Get logic actor
	Game::Actors::C_VehicleLogicActor* pLogicActorMe = GetVehicleLogicActor( GetPlayerVehicleIndex( m_sLocalPlayerIndex ) );
	if ( !pLogicActorMe )
		return;

	// Get target zoom
	F32 fTargetDistance = 500.0f;

	if( m_bIsNormalView ) // Normal mode
	{
		fTargetDistance = (Maths::Clamp<F32>( pLogicActorMe->GetPositionInfo().mSpeedInMph / m_fTopSpeedInMph, 0.0f, 1.0f )  * (m_fMaxViewDist-m_fMinViewDist)) + m_fMinViewDist;
	}
	else
	{
		fTargetDistance = m_fBattleViewDist;
	}

	m_fCurrentViewDist += (fTargetDistance - m_fCurrentViewDist) * theFETimer.FrameTime() * 4.0f;

#ifndef CFG_GOLD
	if( g_MiniMapDrawDebug )
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(
		Maths::HVec3(	(F32)m_MapOffset.X() - (m_MapSize.X()/2.0f), 
						(F32)0.0f,
						(F32)m_MapOffset.Z() - (m_MapSize.Z()/2.0f)),
						 
		Maths::HVec3(	(F32)m_MapOffset.X() + (m_MapSize.X()/2.0f), 
						(F32)0.0f,
						(F32)m_MapOffset.Z() + (m_MapSize.Z()/2.0f)),
						Maths::Constants<Maths::HMat43>::Identity(),
						Maths::RGBA(0x00, 0x00, 0xff, 0xff) );
	}
#endif

	const Cam2::I_Feed& feed = AmaxCam::CameraSystem().DebugFeed( m_sLocalPlayerIndex );

	F32 viewDistance = m_fCurrentViewDist;

#ifndef CFG_GOLD
	if( g_MiniMapUseDebug )
	{
		m_MapOffset = Maths::HVec3( (F32)g_MiniMapCentrePointX, 0.0f, (F32)g_MiniMapCentrePointZ );
		m_MapSize = Maths::HVec3( (F32)g_MiniMapBoundsSizeX, 0.0f, (F32)g_MiniMapBoundsSizeX * (F32)g_MiniMapAspect );

		viewDistance = (F32)g_MiniMapViewSize;
	}
#endif

	Maths::HVec3 worldScale3( 1.0f / (F32)m_MapSize.X(), 0.0f, 1.0f / -m_MapSize.Z() );
	Maths::HVec3 worldOffset3( (F32)m_MapOffset.X(), 0.0f, (F32)m_MapOffset.Z() );
	
	// Angle
	Maths::HVec3 worldDirMe( feed.Mat43().AZ() );
#ifndef CFG_GOLD
	if( !g_MiniMapUseCamera )
		worldDirMe = pLogicActorMe->GetWorldMatrix().AZ();
#endif
	m_Rotation.z = Maths::RadToDeg( atan2f( worldDirMe.X(), worldDirMe.Z() ) );

	// Position
	m_WorldPosMe = feed.Mat43().AW();
#ifndef CFG_GOLD
	if( !g_MiniMapUseCamera )
		m_WorldPosMe = pLogicActorMe->GetWorldMatrix().AW();
#endif

	Maths::HVec3 uv = (m_WorldPosMe - worldOffset3) * worldScale3;
	if( m_TextureParams.count > 0 )
	{
		C_TextureParam& t = m_TextureParams[0];

		// Scale image to view distance
		t.m_Size.u = (F32)viewDistance / m_MapSize.X();
		t.m_Size.v = (F32)viewDistance / m_MapSize.Z();

		// Offset to position
		t.m_Position.u = uv.X() + ((1.0f - t.m_Size.u)/2.0f);
		t.m_Position.v = uv.Z() + ((1.0f - t.m_Size.v)/2.0f);
	}

	// Update the vector graphic pivots for objects that get drawn into the render target
	if( m_pDummyScale )
	{
		F32 scale = viewDistance / m_MapSize.X();

		// Scale pivot
		m_pDummyScale->m_Scale.x = 1.0f / scale;
		m_pDummyScale->m_Scale.y = 1.0f / scale;
	}
	if( m_pDummyTranslate )
	{
		F32 x = m_WorldPosMe.X();
		F32 z = m_WorldPosMe.Z();

		x -= worldOffset3.X();
		z -= worldOffset3.Z();

		x = x / m_MapSize.X() * 512.0f;
		z = z / m_MapSize.X() * 512.0f;

		// Translation pivot
		m_pDummyTranslate->m_Position.x = -x;
		m_pDummyTranslate->m_Position.y =  z;
	}
}

bool UIMiniMap::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated )
		return C_GadgetBase::MessageHandler( pMessage );

	switch ( pMessage->Type() )
	{
	case E_UiGameMsg_SetLocalPlayerIndex:
		if ( m_sLocalPlayerIndex == -1 )
		{
			m_sLocalPlayerIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];

			// Get top speed of my car
			S32 sVehicleIndexMe = GetPlayerVehicleIndex( m_sLocalPlayerIndex );

			Game::Actors::C_VehicleLogicActor* pLogicActorMe = GetVehicleLogicActor( sVehicleIndexMe );
			if ( pLogicActorMe )
			{
				Game::Actors::C_VehicleActor* pVehicleActor = pLogicActorMe->GetGraphicalActor( sVehicleIndexMe );
				if( pVehicleActor )
				{
					AmaxVehicle* pXtVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)pVehicleActor->GetXtIndex()];
					if( pXtVehicle )
					{
						m_fTopSpeedInMph = (F32)pXtVehicle->m_sUiMaxSpeed;
						if( m_fTopSpeedInMph <= 0.0f )
							m_fTopSpeedInMph = 170.0f;
					}
				}
			}

			m_ColourYou.SetByName( m_xtColourYou );
			m_ColourMap.SetByName( m_xtColourMap );
			m_ColourTargeted.SetByName( m_xtColourTargeted );
			m_ColourPulseWinner.SetByName( m_xtColourPulseWinner );
			m_ColourPulseRival.SetByName( m_xtColourPulseRival );

			m_ColourDestructionEasy.SetByName( m_xtColourDestructionEasy );
			m_ColourDestructionMedium.SetByName( m_xtColourDestructionMedium );
			m_ColourDestructionHard.SetByName( m_xtColourDestructionHard );
			m_ColourDestructionVHard.SetByName( m_xtColourDestructionVHard );

			m_ColourFirst.SetByName( m_xtColourFirst );
			m_ColourSecond.SetByName( m_xtColourSecond );
			m_ColourThird.SetByName( m_xtColourThird );
			m_ColourNone.SetByName( m_xtColourNone );

			m_ColourVP[0].SetByName( m_xtColourVP0 );
			m_ColourVP[1].SetByName( m_xtColourVP1 );
			m_ColourVP[2].SetByName( m_xtColourVP2 );
			m_ColourVP[3].SetByName( m_xtColourVP3 );

			SetupRTGadgets();
		}
		break;

	case E_UiMsg_CtrlButton_RightThumb :
		{
			const C_UiMessagePadData* pUIMessage = (C_UiMessagePadData*)pMessage;

			// Switch view
			if( pUIMessage && pUIMessage->deb && pUIMessage->pad == m_sLocalPlayerIndex )
				m_bIsNormalView = !m_bIsNormalView;
		}
		break;

	default:
		return C_GadgetBase::MessageHandler( pMessage );
	}

	return true;
}

void UIMiniMap::Render()
{
	if(!m_Activated)
		return;

	// auto-transform
	C_UIAutoTransform transform( this );

	C_GadgetBase::Render();

	// The map!
	{
		C_UISpriteColours colourMap;
		colourMap.Set( m_ColourMap, m_AlphaInherit, m_IntensityInherit );

		gSimple2D.AddSprite(m_Layer, m_Position, m_Size.x, m_Size.y, colourMap, m_Panel, E_SpriteType_Sprite, m_RenderState);
		gSimple2D.AddJustification( m_Justify );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) ); // Map
		gSimple2D.AddTexUVs( 1, GetTextureParam( 1 ) ); // Cutout circle
	}

	// The road render target
	if( m_bRouteShapeExists )
	{
		C_UISpriteColours colours;
		colours = 0xffffffff;
		colours.SetAlpha( (U8)(m_AlphaInherit * 255.0f) );

		gSimple2D.AddSprite(m_Layer, m_Position, m_Size.x, m_Size.y, colours, m_Panel, E_SpriteType_Sprite, m_RenderState);
		gSimple2D.AddJustification( m_Justify );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 5 ) ); // Render target
		gSimple2D.AddTexUVs( 1, GetTextureParam( 1 ) ); // Cutout circle
	}

	Maths::HVec2 texScale;
	if( m_TextureParams.count > 0 )
	{
		C_TextureParam& t = m_TextureParams[0];
		texScale = Maths::HVec2( 1.0f / t.m_Size.u, 1.0f / t.m_Size.v );
	}

	// Gather sorted data of other vehicles
	S32 sVehicleIndexMe = GetPlayerVehicleIndex( m_sLocalPlayerIndex );

	C_UIRaceLineSortArray& sortInfo = m_Sorter.GatherSortInfo( sVehicleIndexMe );
	Game::C_LevelState& levelState	= Game::C_Game::GetInstance()->GetLevelState();	

	bool isDestruction = ( levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction );
	LayerIndex layerPlusOne = isDestruction ? Maths::Min( m_Layer + 1, gSimple2D.TopLayer() ) : m_Layer;
	
	const F32 fClampFraction = m_fClampFraction * 0.5f;
	const Maths::HVec2 worldScale( 1.0f / (F32)m_MapSize.X(), 1.0f / m_MapSize.Z() );

	// Render positions of other drivers
	C_UISpriteColours targetedColour;
	targetedColour.Set( m_ColourTargeted, m_AlphaInherit, m_IntensityInherit );

	// Always increment our Rival intensity timer
	m_fRivalIntensityTimer += theFETimer.FrameTime();

	for ( S32 i = ( sortInfo.Size() - 1 ); i >= 0; i-- )
	{
		if ( !sortInfo[ i ].m_bActive )
			continue;

		F32 fDiff = sortInfo[ i ].m_fValue01;
		S32 sVehicleIndexO = sortInfo[ i ].m_sVehicleIndex;

		if( !IsSilentRunningHidingThisVehicle(levelState, sVehicleIndexO) )
		{
			Game::Actors::C_VehicleLogicActor* pLogicActorOther = GetVehicleLogicActor( sVehicleIndexO );

			if ( !pLogicActorOther || ( ( levelState.mCars[ sVehicleIndexO ].mState != Game::E_VEHICLE_STATE_RACING ) && ( levelState.mCars[ sVehicleIndexO ].mState != Game::E_VEHICLE_STATE_FINISHED ) ) )
				continue;

			Game::Actors::C_VehicleActor* pVehicleActor = pLogicActorOther->GetGraphicalActor( sVehicleIndexO );

			if ( !pVehicleActor->GetVisibility() )
				continue;

			Maths::HVec3 diff( pLogicActorOther->GetWorldMatrix().AW() - m_WorldPosMe );
			Maths::HVec2 mapPoint = Maths::HVec2( diff.X(), diff.Z() ) * worldScale * texScale;

			// Clamp and fade
			F32 length = mapPoint.Length();
			F32 fAlpha = 1.0f;
			S32 carIconIndex = 3;
			bool isClamped = false;
	
			if( length > fClampFraction )
			{
				mapPoint /= length;			// Normalise
				mapPoint *= fClampFraction;	// Scale

				isClamped = true;

				carIconIndex = 4;

				// Do Fade
				length = 1.0f - (length - fClampFraction);
				fAlpha = Maths::Clamp<F32>( length * m_fFadeFraction, 0.0f, 1.0f ) * 0.5f;
			}
			if( fAlpha <= 0.0f )
				continue;

			// Show above or below icon
			bool isSameLevel = true;
			if( m_bCheckHeight && !isClamped )
			{
				const F32 sameLevelRange = 3.0f;
				if( diff.Y() > sameLevelRange )
				{
					carIconIndex = 6;
					isSameLevel = false;
				}
				else
				if( diff.Y() < -sameLevelRange )
				{
					carIconIndex = 7;
					isSameLevel = false;
				}
			}

			Maths::HVec2 drawPoint = Maths::HVec2( m_Position.x + (m_Size.x * mapPoint.X()), m_Position.y + (m_Size.y * mapPoint.Y()) );

			Game::Actors::C_VehicleLogicActor* pLogicActorMe = GetVehicleLogicActor( sVehicleIndexMe );

			bool bIsRival = false;
			bool bIsWinner = false;
			float lerpVal = 1.0f;

			if ( !levelState.mTeams )
			{
				if(pLogicActorMe)
				{
					// Is this our rival?
					bIsRival = GetIsRival(levelState, pLogicActorMe, sVehicleIndexO);
				}

				// Is this the winner?
				bIsWinner = GetIsWinner(levelState, sVehicleIndexO);
			
				// default to multiplier that has no effect
				if((bIsRival &&  g_MiniMapRivalPulsingEnabled) || (bIsWinner && g_MiniMapWinnerPulsingEnabled))
				{
					lerpVal = CalculateLerpValue();
				}
			}

			// Default colour
			C_UISpriteColours colours;

			if ( bIsRival || bIsWinner )
				fAlpha = 1.0f;

			if( levelState.mRaceMode == Game::E_RaceMode_Destruction || levelState.mRaceMode == Game::E_RaceMode_FriendDemand_Destruction )
			{
				const AmaxRaceModeDestruction* destructionSetupData = ((Game::C_RaceModeDestruction*)Game::gRaceModeStore(levelState.mRaceMode))->GetSetupData();

				U32 vehicleID = pLogicActorMe->GetGraphicalActor(sVehicleIndexO)->GetXtIndex();
				if( vehicleID == destructionSetupData->vehicleEasy->vehicle->ID )
				{
					colours.Set( m_ColourDestructionEasy, m_AlphaInherit * fAlpha, m_IntensityInherit );
				}
				else if( vehicleID == destructionSetupData->vehicleMedium->vehicle->ID )
				{
					colours.Set( m_ColourDestructionMedium, m_AlphaInherit * fAlpha, m_IntensityInherit );
				}
				else if( vehicleID == destructionSetupData->vehicleHard->vehicle->ID )
				{
					colours.Set( m_ColourDestructionHard, m_AlphaInherit * fAlpha, m_IntensityInherit );
				}
				else if( vehicleID == destructionSetupData->vehicleVHard->vehicle->ID )
				{
					colours.Set( m_ColourDestructionVHard, m_AlphaInherit * fAlpha, m_IntensityInherit );
				}
				else
				{
					// Default behaviour - White. Only get into this if you change focus to an AI car
					colours.Set( C_UISpriteColours(0xFFFFFFFF), m_AlphaInherit * fAlpha, m_IntensityInherit );
				}
			}
			else
			{
				if ( levelState.mTeams )
				{
					C_UISpriteColours teamColour = levelState.mCars[ sVehicleIndexO ].mTeam ? C_UiXtConfig::Instance()->m_TeamBColour : C_UiXtConfig::Instance()->m_TeamAColour;
					colours.Set( teamColour, m_AlphaInherit * fAlpha, m_IntensityInherit );
				}
				else
				{
					if( C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() == Game::E_GameMode_SplitScreen )
					{
						// Colour based on viewport
						const U8 index = pLogicActorOther->GetVehicleIndex();
						if( index < m_MaxViewports )
							colours.Set( m_ColourVP[ index ], m_AlphaInherit * fAlpha, m_IntensityInherit );
						else
							colours.Set( m_ColourNone, m_AlphaInherit * fAlpha, m_IntensityInherit );
					}
					else
					{
						S32 sPosition = pLogicActorOther->GetPositionInfo().mRacePosition;

						switch ( sPosition )
						{
							case 1:		colours.Set( m_ColourFirst, m_AlphaInherit * fAlpha, m_IntensityInherit );		break;
							case 2:		colours.Set( m_ColourSecond, m_AlphaInherit * fAlpha, m_IntensityInherit );		break;
							case 3:		colours.Set( m_ColourThird, m_AlphaInherit * fAlpha, m_IntensityInherit );		break;
							default:	colours.Set( m_ColourNone, m_AlphaInherit * fAlpha, m_IntensityInherit );		break;
						}
					}
				}
			}
			
			// Finally now we have the base colour.. lerp to 'Rival'/'Winner' colour if this is necessary. 
			LayerIndex layerCar = m_Layer;

			if( bIsRival || bIsWinner)
			{
				PulseColour(bIsWinner,bIsRival, lerpVal, colours);
				layerCar = layerPlusOne;
			}

			F32 vehicleAngle = 180.0f + Maths::RadToDeg( atan2f( pLogicActorOther->GetWorldMatrix().AZ().X(), -pLogicActorOther->GetWorldMatrix().AZ().Z()) );

			// Tracking
			if ( sortInfo[ i ].m_bTracking )
			{
				static F32 fTrackingMult = 1.1f;

				gSimple2D.AddSprite( m_Layer, Maths::HVec3( drawPoint.X(), drawPoint.Y(), m_Position.z ), 
					m_fVehicleWidth * fTrackingMult, m_fVehicleHeight * fTrackingMult, targetedColour, m_Panel, E_SpriteType_Sprite, m_RenderState );
				gSimple2D.AddTexUVs( 0, GetTextureParam( 8 ) );
				gSimple2D.AddJustification( E_Justify_MiddleCentre );
				gSimple2D.AddRotation( 0.0f, 0.0f, 90.0f - m_Rotation.z );
			}

			// If this is the rival.. lets draw the backing
			if( bIsRival )
			{
				// C_UISpriteColours targetedColour; - set alpha and intensity of this?
				C_UISpriteColours rivalBackingColour = m_ColourPulseRival;
				rivalBackingColour.SetAlpha(U8(lerpVal * 255.0f * g_MiniMapRivalBackingAlphaMultiplier));
				rivalBackingColour.SetIntensity(lerpVal * g_MiniMapRivalBackingIntensityMultiplier); 

				F32 sizey = isClamped ? m_fVehicleWidth : m_fVehicleHeight;
				gSimple2D.AddSprite( layerCar, Maths::HVec3( drawPoint.X(), drawPoint.Y(), m_Position.z ), 
				m_fVehicleWidth * g_MiniMapRivalBackingSizeMultiplier, sizey * g_MiniMapRivalBackingSizeMultiplier, rivalBackingColour, m_Panel, E_SpriteType_Sprite, m_RivalBackingRenderState );
				gSimple2D.AddTexUVs( 0, GetTextureParam( 9 ) );
				gSimple2D.AddJustification( E_Justify_MiddleCentre );
				gSimple2D.AddRotation( 0.0f, 0.0f, isSameLevel ? vehicleAngle : -m_Rotation.z );
			}

			// Draw sprite
			F32 sizey = isClamped ? m_fVehicleWidth : m_fVehicleHeight;
			gSimple2D.AddSprite( layerCar, Maths::HVec3( drawPoint.X(), drawPoint.Y(), m_Position.z ), 
				m_fVehicleWidth, sizey, colours, m_Panel, E_SpriteType_Sprite, m_RenderState );
			gSimple2D.AddTexUVs( 0, GetTextureParam( carIconIndex ) );
			gSimple2D.AddJustification( E_Justify_MiddleCentre );
			gSimple2D.AddRotation( 0.0f, 0.0f, isSameLevel ? vehicleAngle : -m_Rotation.z );
			
		}
	}

	// Render you
	Game::Actors::C_VehicleLogicActor* pLogicActorMe = GetVehicleLogicActor( sVehicleIndexMe );
	if( pLogicActorMe )
	{
		C_UISpriteColours colours;
		colours = m_ColourYou;
		colours.SetAlpha( (U8)(m_AlphaInherit * 255.0f) );

		if(g_MiniMapSinglePlayerPulsingEnabled)
		{
			float lerpVal = CalculateLerpValue();
			PulseColour(g_MiniMapSinglePlayerFlagAsWinner,g_MiniMapSinglePlayerFlagAsRival,lerpVal,colours); 
		}

		gSimple2D.AddSprite( m_Layer, m_Position, 
			m_fVehicleWidth, m_fVehicleHeight, colours, m_Panel, E_SpriteType_Sprite, m_RenderState );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 3 ) );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );
		gSimple2D.AddRotation( 0.0f, 0.0f, 180.0f + Maths::RadToDeg( atan2f( pLogicActorMe->GetWorldMatrix().AZ().X(), -pLogicActorMe->GetWorldMatrix().AZ().Z()) ) );
	}
	// The outline
	{
		gSimple2D.AddSprite(m_Layer, m_Position, m_Size.x, m_Size.y, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState);
		gSimple2D.AddTexUVs( 0, GetTextureParam( 2 ) );
		gSimple2D.AddJustification( m_Justify );
	}
}

const bool UIMiniMap::IsSilentRunningHidingThisVehicle(Game::C_LevelState& levelState, const S32 vehicleIndex)
{
	if(!m_bSilentRunningHidesPlayersOnRadar)
		return false;

	//Check if they have the silent running mod
	Game::C_LevelStateMultiplayerVehicleAbilities* pOtherVehiclesAbilities = levelState.mCars[vehicleIndex].mUpgrades.GetMultiplayerAbilities();
	BIZ_ASSERT_MSG(pOtherVehiclesAbilities, "UIMiniMap::IsSilentRunningHidingThisVehicle() > Couldn't get a pointer to the other vehicle's abilities");

	Game::C_MultiplayerVehicleAbility* pSilentRunningAbility = pOtherVehiclesAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Silent_Running);
	BIZ_ASSERT_MSG(pSilentRunningAbility, "UIMiniMap::IsSilentRunningHidingThisVehicle() > Couldn't get a pointer to the other vehicle's silent running ability");

	return pOtherVehiclesAbilities->GetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Silent_Running);
}

bool UIMiniMap::GetIsRival( const Game::C_LevelState& levelState, Game::Actors::C_VehicleLogicActor* pLogicActorMe, U8 vehicleIndex ) const
{
	bool bIsRival = false;
	if(levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction )
	{
		// If rival index == this index...
		// BUG: 35504 scoring component may be NULL if we are spectating a non-local vehicle
		// TODO: should probably grab this across the network and show if we have time...
		if(pLogicActorMe && pLogicActorMe->pNewScoringComponent && pLogicActorMe->pNewScoringComponent->GetRivalIndex() == vehicleIndex )
		{
			bIsRival = true;
		}	
	}
	return bIsRival;
}

bool UIMiniMap::GetIsWinner( const Game::C_LevelState& levelState, U8 vehicleIndex ) const
{
	bool bWinner = false;
	if (  levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction  )
	{
		bWinner = ( Game::C_MpResults::Instance().m_SortedVehicleResults[ 0 ].m_LocalIndex == vehicleIndex );
	}

	return bWinner;
}

float UIMiniMap::CalculateLerpValue() 
{
	float timeVal = m_fRivalIntensityTimer * g_MiniMapRivalPulseRateMultiplier; 
	float fraction = timeVal - Maths::Floor(timeVal); 
	float lerpVal = 0.0f;
	if(m_Timeline.count > 0 && m_Timeline[ 0 ].m_Effects.count > 0)
	{
		lerpVal = m_Timeline[ 0 ].m_Effects[ 0 ].Evaluate( fraction );
	}
	return lerpVal;
}


void UIMiniMap::PulseColour(const bool bIsWinner, const bool bIsRival, const float lerpVal,C_UISpriteColours& colours)
{
	if( bIsRival && g_MiniMapRivalPulsingEnabled )
	{
		// 'rival' pulse colour
		colours.Lerp(colours, m_ColourPulseRival, lerpVal);
	}
	else
	{
		// If not rival.. check for winner (rival always shows over winner)
		if( bIsWinner && g_MiniMapWinnerPulsingEnabled )
		{
			// 'winner' pulse colour
			colours.Lerp(colours,m_ColourPulseWinner, lerpVal);
		}
	}
}

}; // namespace UI
