#include "stdafx.h"

#include "UIRaceLine.h"
#include "UiHelpers.h"

#include "UIMessages.h"
#include "UiXtConfig.h"
#include "ui/Graphics/SimpleTimer.h"

#include "Game/AmaxCam/CameraSystem.h"
#include "Game/GameActors/Components/Messages/VehicleLogicMessages.h"

#include <AI/AI_Amax.h>
#include <game/Game.h>
#include <algorithm>

#include<GameLogic\Layers\RaceLogicLayer.h>
#include<GameLogic\Pickups\PickupMineBase.h>

#include "GameLogic/RaceRules/RaceRulesManager.h"
#include <GameLogic/Structures/LevelStateMultiplayerAbilities.h>
#include <camera2/stock/DebugFeed.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

XT_TYPE( UI::UIRaceLine, "UIRaceLine" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_fVehicleWidth, "vehicle_width" )
	XT_FIELD( m_fVehicleHeight, "vehicle_height" )
	XT_FIELD( m_fDotSize, "dot_size" )
	XT_FIELD( m_fAlphaFadeSize, "alpha_fade_size" )
	XT_FIELD( m_xtColourYou, "colour_you" )
	XT_FIELD( m_xtColourOther, "colour_other" )
	XT_FIELD( m_xtColourMine, "colour_mine" )
	XT_FIELD( m_fScaleMin, "scale_min" )
	XT_FIELD( m_xtColourName, "colour_name" )
	XT_FIELD( m_fNameTextHeight, "name_text_height" )
	XT_FIELD( m_fNameGapMult, "name_gap_mult" )
	XT_FIELD( m_FontStylePasses, "styles" )
	XT_FIELD( m_fSpinTime, "spin_time" )
	XT_FIELD( m_fViewDistance, "view_distance" )
	XT_FIELD( m_bUseCameraNotCar, "use_camera_forward_not_car" )
	XT_FIELD( m_fMapSize, "map_size" )
	XT_FIELD( m_fMapScale, "map_scale" )
	XT_FIELD( m_xtColourRadarFill, "colour_radar_fill" )
	XT_FIELD( m_xtColourRadarOuter, "colour_radar_outer" )
	XT_FIELD( m_fTargetingSize, "targeting_size" )
	XT_FIELD( m_fTargetingGapMult, "targeting_gap_mult" )
	XT_FIELD( m_xtColourTargeted, "colour_targeted" )
XT_END_TYPE

namespace UI
{

static C_UIPerkInfo* GetPerkInfo( const Game::Actors::C_VehicleLogicActor* pLogicActor, const E_PICKUP_TYPE eTypeOverride = E_PICKUP_TYPE_NONE )
{
	if ( !C_UiXtConfig::Instance() )
		return NULL;

	if ( !pLogicActor )
		return C_UiXtConfig::Instance()->m_PerkDisplayParameters.GetPerkInfo( eTypeOverride );
	
	if ( ( pLogicActor->GetPickupInfo().mPickupStoreIndex == -1 ) ||
		 ( pLogicActor->GetPickupInfo().mPickupStoreIndex < 0 ) || 
		 ( pLogicActor->GetPickupInfo().mPickupStoreIndex >= pLogicActor->GetPickupInfo().mPickupStore.Size() ) )
		return NULL;

	return C_UiXtConfig::Instance()->m_PerkDisplayParameters.GetPerkInfo( 
		pLogicActor->GetPickupInfo().mPickupStore[ pLogicActor->GetPickupInfo().mPickupStoreIndex ].mType
	);
}

// --------------------
// C_UIRaceLineSortInfo
// --------------------
bool C_UIRaceLineSortInfo::operator < ( const C_UIRaceLineSortInfo& o ) const
{
	return ( m_fValue01 > o.m_fValue01 );
}

// --------------------
// C_UIRaceLineSorter
// --------------------
void C_UIRaceLineSorter::Init( const S32 sNumVehicles )
{
	m_sNumVehicles = sNumVehicles;

	if ( m_sNumVehicles > 0 )
	{
		S32 sNumSortVehicles = m_sNumVehicles - 1;	// don't want to sort me

		m_SortInfo.Reserve( sNumSortVehicles );	

		for ( S32 i = 0; i < sNumSortVehicles; i++ )
			m_SortInfo.PushBack( C_UIRaceLineSortInfo() );
	}
}

C_UIRaceLineSortArray& C_UIRaceLineSorter::GatherSortInfo( const S32 sVehicleIndexMe )
{
	Game::Actors::C_VehicleLogicActor* pLogicActorMe = GetVehicleLogicActor( sVehicleIndexMe );

	if ( !pLogicActorMe )
		return m_SortInfo;

	F32 fLapPosMe01 = pLogicActorMe->GetPositionInfo().mLogicPosition;

	S32 sCount = 0;

	for ( S32 i = 0; i < m_sNumVehicles; i++ )
	{
		if ( i == sVehicleIndexMe )
			continue;

		Game::Actors::C_VehicleLogicActor* pLogicActorOther = GetVehicleLogicActor( i );

		if ( !pLogicActorOther )
			continue;

		F32 fLapPosOther01 = pLogicActorOther->GetPositionInfo().mLogicPosition;
		F32 fDiff = fLapPosOther01 - fLapPosMe01;

		if ( fDiff > 0.5f )
			fDiff -= 1.0f;
		if ( fDiff < -0.5f )
			fDiff += 1.0f;

		m_SortInfo[ sCount ].m_fValue01 = fDiff;
		m_SortInfo[ sCount ].m_pPerkInfo = GetPerkInfo( pLogicActorOther );
		m_SortInfo[ sCount ].m_sVehicleIndex = i;
		m_SortInfo[ sCount ].m_bTracking = Game::C_RaceRulesManager::Instance().TrackVehicle( i );

		Game::Actors::C_VehicleActor* pVehicleActorOther = pLogicActorOther->GetGraphicalActor( i );

		m_SortInfo[ sCount ].m_bActive = (pVehicleActorOther && GetLevelState().mCars[ i ].mState == Game::E_VEHICLE_STATE_RACING) ? true : false;

		sCount++;
	}

	std::sort( m_SortInfo.Begin(), m_SortInfo.End() );

	return m_SortInfo;
}

// ----------
// UIRaceLine
// ----------
void UIRaceLine::Init()
{
	C_GadgetBase::Init();

	m_fVehicleWidth = 10.0f;
	m_fVehicleHeight = 20.0f;
	m_fDotSize = 5.0f;
	m_fAlphaFadeSize = 0.25f;
	m_fScaleMin = 0.5f;
	m_fNameTextHeight = 20.0f;
	m_fNameGapMult = 1.0f;
	m_fSpinTime = 1.0f;
	m_fViewDistance = 100.0f;
	m_bUseCameraNotCar = false;
	m_fMapSize = 80.0f;
	m_fMapScale = 1.0f;
	m_fTargetingSize = 20.0f;
	m_fTargetingGapMult = 0.0f;

	for ( S32 i = 0; i < m_VehicleInRange.Size(); i++ )
		m_VehicleInRange[ i ] = false;

	// messages
	AddSubscriber( E_UiMsg_FrameUpdate );
	AddSubscriber( E_UiGameMsg_SetLocalPlayerIndex );
	AddSubscriber( E_VehicleLogicMsg_HitByPerk );
	AddSubscriber( E_UiGameMsg_VehicleInRangeMessage );
}

void UIRaceLine::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	// resolve colour styles
	m_ColourYou.SetByName( m_xtColourYou );
	m_ColourOther.SetByName( m_xtColourOther );
	m_ColourName.SetByName( m_xtColourName );
	m_ColourMine.SetByName( m_xtColourMine );

	m_ColourRadarFill.SetByName( m_xtColourRadarFill );
	m_ColourRadarOuter.SetByName( m_xtColourRadarOuter );

	m_ColourTargeted.SetByName( m_xtColourTargeted );

	// resolve font styles
	m_FontStylePasses.Setup( pState );

	Game::Actors::RaceLogicCore* pCore = GetRLC();

	S32 sNumVehicles = 0;
	if ( pCore )
		sNumVehicles = (S32)pCore->GetNumVehicles();

	m_EffectInfo.Reserve( sNumVehicles );
	m_Sorter.Init( sNumVehicles );

	for ( S32 i = 0; i < sNumVehicles; i++ )
		m_EffectInfo.PushBack( C_UIRaceLineEffectInfo() );

	m_fTrackLength = AIAmax::AmaxAIR::Instance().DistanceFromStartOfSpline( 1 );
	m_fLogicCap01  = m_fViewDistance / m_fTrackLength;
}

void UIRaceLine::FrameUpdate()
{
	if ( !m_Activated )
		return;

	F32 fFrameTime = theFETimer.FrameTime();

	// update effects
	for ( S32 i = 0; i < m_EffectInfo.Size(); i++ )
	{
		if ( !m_EffectInfo[ i ].m_bActive )
			continue;

		m_EffectInfo[ i ].m_fTime01 += ( ( 1.0f / m_fSpinTime ) * fFrameTime );

		if ( m_EffectInfo[ i ].m_fTime01 >= 1.0f )
			m_EffectInfo[ i ].m_bActive = false;
	}
}

bool UIRaceLine::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated )
		return C_GadgetBase::MessageHandler( pMessage );

	switch ( pMessage->Type() )
	{
		case E_UiGameMsg_SetLocalPlayerIndex:
		{
			if ( m_sLocalPlayerIndex == -1 )
				m_sLocalPlayerIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
			break;
		}

		case E_VehicleLogicMsg_HitByPerk:
		{
			ProcessHitByPerk( pMessage );
			break;
		}

		case E_UiGameMsg_VehicleInRangeMessage:
		{
			// enemy is in range for me to fire
			const C_UiVehicleInRangeMessage* pMsg = Messaging::MessageCast<C_UiVehicleInRangeMessage>( pMessage );

			S32 sVehicleIndex = GetPlayerVehicleIndex( m_sLocalPlayerIndex );

			if ( pMsg->m_sPlayerVehicleIndex == sVehicleIndex )
				m_VehicleInRange[ pMsg->m_sVehicleIndex ] = true;

			break;
		}

		default:
			return C_GadgetBase::MessageHandler( pMessage );
	}

	return true;
}

void UIRaceLine::Render()
{
	if ( ( m_Timeline.count < 1 ) || ( m_Timeline[ 0 ].m_Effects.count < 1 ) )
		return;

	if ( !m_Activated )
		return;

	Game::Actors::RaceLogicCore* pCore = GetRLC();

	if ( !pCore )
		return;

	S32 sVehicleIndexMe = GetPlayerVehicleIndex( m_sLocalPlayerIndex );

	Game::Actors::C_VehicleLogicActor* pLogicActorMe = GetVehicleLogicActor( sVehicleIndexMe );

	if ( !pLogicActorMe )
		return;

	bool bSplitscreen = ( GetLevelState().mNumViewports > 1 );

	// auto-transform
	C_UIAutoTransform transform( this );

	C_GadgetBase::Render();

	C_TextureParam& dotTex = GetTextureParam( 2 );
	C_TextureParam& vehicleTex = GetTextureParam( 1 );

	static F32 fRadarMultIntensity = 0.3f;

	F32 fIntensityInheritRadar = m_IntensityInherit;

	if ( fIntensityInheritRadar > 1.0f )
		fIntensityInheritRadar = 1.0f + ( fIntensityInheritRadar - 1.0f ) * fRadarMultIntensity;

	C_UISpriteColours colourRadarFill;
	colourRadarFill.Set( m_ColourRadarFill, m_AlphaInherit, fIntensityInheritRadar );

	C_UISpriteColours colourRadarOuter;
	colourRadarOuter.Set( m_ColourRadarOuter, m_AlphaInherit, fIntensityInheritRadar );

	C_UISpriteColours colourTargeted;
	colourTargeted.Set( m_ColourTargeted, m_AlphaInherit, m_IntensityInherit );

	if ( !bSplitscreen )
		RenderLine();

	// render map
	F32 fMapSizeDraw = m_fMapSize * m_fMapScale;

	gSimple2D.AddSprite( m_Layer, m_Position, fMapSizeDraw, fMapSizeDraw, 
		colourRadarFill, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 3 ) );
	gSimple2D.AddJustification( E_Justify_MiddleCentre );

	gSimple2D.AddSprite( m_Layer, m_Position, fMapSizeDraw, fMapSizeDraw, 
		colourRadarOuter, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 4 ) );
	gSimple2D.AddJustification( E_Justify_MiddleCentre );

	/*
	// draw you as a dot
	static F32 fYouSize = 0.3f;

	gSimple2D.AddSprite( m_Layer, m_Position, fYouSize, fYouSize,
		m_ColourYou, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, dotTex );
	gSimple2D.AddJustification( E_Justify_MiddleCentre );
	*/

	C_UIRaceLineSortArray& sortInfo = m_Sorter.GatherSortInfo( GetPlayerVehicleIndex( m_sLocalPlayerIndex ) );

	// need to render them backwards - just cos i've sorted them that way!

	Game::C_LevelState& levelState	= Game::C_Game::GetInstance()->GetLevelState();	

	C_RenderState renderStateTracking = m_RenderState;
	renderStateTracking.m_EffectIndex = E_Effect_IntensityAsAlpha;

	static F32 fTrackingMult = 1.25f;

	// cant cache pointer to mpabilities as testing against diff leveStateCars each time, so cache once per update here
	Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = levelState.mCars[sVehicleIndexMe].mUpgrades.GetMultiplayerAbilities();
	Game::C_MultiplayerVehicleAbility* radarAbility = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Advanced_Radar);
	Game::C_MultiplayerVehicleAbility* silentRunningAbility = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Silent_Running);

	// render dots outside of the radar view range
	if ( !bSplitscreen )
	{
		for ( S32 i = ( sortInfo.Size() - 1 ); i >= 0; i-- )
		{
			if ( !sortInfo[ i ].m_bActive )
				continue;

			F32 fDiff = sortInfo[ i ].m_fValue01;
			S32 sVehicleIndexO = sortInfo[ i ].m_sVehicleIndex;

			if(   radarAbility->GetActive() ||
				! silentRunningAbility->GetActive())
			{
				
				if ( Maths::Abs<F32>( fDiff ) < m_fLogicCap01 )
					continue;

				F32 fDiff2 = ( ( Maths::Abs<F32>( fDiff ) * 2.0f ) - ( m_fLogicCap01 / 2.0f ) ) / ( 1.0f - ( m_fLogicCap01 / 2.0f ) );
				F32 fOffsetY = ( m_fMapSize / 2.0f ) + ( ( m_Size.y / 2.0f ) - ( m_fMapSize / 2.0f ) ) * fDiff2;

				F32 fScaleMult = Maths::Lerp( 1.0f, m_fScaleMin, fDiff2 );

				// re-calc
				fDiff2 = fOffsetY / ( m_Size.y / 2.0f );

				if ( fDiff > 0.0f )
					fOffsetY *= -1.0f;

				//  alpha out same as line
				F32 fAlpha = 0.0f;

				if ( fDiff2 < 1.0f )
					fAlpha = ( fDiff2 > ( 1.0f - m_fAlphaFadeSize ) ) 
						? ( 1.0f - ( ( fDiff2 - ( 1.0f - m_fAlphaFadeSize ) ) / m_fAlphaFadeSize ) ) 
						: 1.0f;

				if ( fAlpha <= 0.0f )
					continue;

				// colour based on perk
				C_UIPerkInfo* pPerkInfo = sortInfo[ i ].m_pPerkInfo;

				
				C_UISpriteColours colours;
				
				Game::C_LevelState& levelState = GetLevelState();
				Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = levelState.mCars[sVehicleIndexMe].mUpgrades.GetMultiplayerAbilities();
				Game::C_MultiplayerVehicleAbility* radarAbility = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Advanced_Radar);

				if(pPerkInfo && (!C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsOnline() || 
				   radarAbility->GetActive()))
				{
					colours.Set( pPerkInfo->m_ColourFill, m_AlphaInherit * fAlpha, m_IntensityInherit );
				}
				else
				{
					colours.Set( m_ColourOther, m_AlphaInherit * fAlpha, m_IntensityInherit );
				}

				static F32 fMultVeh = 0.75f;

				if ( sortInfo[ i ].m_bTracking )
				{
					gSimple2D.AddSprite( m_Layer, m_Position + Maths::HVec3( -m_fVehicleWidth * 1.5f * fScaleMult * fMultVeh, fOffsetY * Up(), 0.0f ),
						m_fVehicleHeight * fScaleMult * fMultVeh * fTrackingMult, m_fVehicleHeight * fScaleMult * fMultVeh * fTrackingMult, 
						colourTargeted, m_Panel, E_SpriteType_Sprite, renderStateTracking );
					gSimple2D.AddTexUVs( 0, GetTextureParam( 7 ) );
					gSimple2D.AddJustification( E_Justify_MiddleCentre );
					gSimple2D.AddRotation( 0.0f, 0.0f, -90.0f );
				}

				gSimple2D.AddSprite( m_Layer, m_Position + Maths::HVec3( 0.0f, fOffsetY * Up(), 0.0f ), 
					m_fVehicleWidth * fScaleMult * fMultVeh, m_fVehicleHeight * fScaleMult * fMultVeh, 
					colours, m_Panel, E_SpriteType_Sprite, m_RenderState );
				gSimple2D.AddTexUVs( 0, vehicleTex );
				gSimple2D.AddJustification( E_Justify_MiddleCentre );

				if ( m_EffectInfo[ sVehicleIndexO ].m_bActive )
				{
					F32 fEval01 = m_Timeline[ 0 ].m_Effects[ 0 ].Evaluate( m_EffectInfo[ sVehicleIndexO ].m_fTime01 );
					gSimple2D.AddRotation( 0.0f, 0.0f, 360.0f * fEval01 );
				}
			}
		}
	}

	// render the radar
	Maths::HVec3 worldPosMe = pLogicActorMe->GetWorldMatrix().AW();

	const Cam2::I_Feed& feed = AmaxCam::CameraSystem().GameFeed( m_sLocalPlayerIndex );

	F32 fViewAngle = 0.0f;

	if ( m_bUseCameraNotCar )
		fViewAngle = -atan2f( feed.Mat43().AZ().X(), feed.Mat43().AZ().Z() );
	else
		fViewAngle = -atan2f( pLogicActorMe->GetWorldMatrix().AZ().X(), pLogicActorMe->GetWorldMatrix().AZ().Z() );

	Maths::HMat33 viewRotationMat( Maths::C_MatrixRotateRPH( 0.0f, 0.0f, fViewAngle ) );
	Maths::HVec2  viewScale( 1.0f / m_fViewDistance, -1.0f / m_fViewDistance ); // -z to go down in y
	
	// vehicles in radar range
	for ( S32 i = ( sortInfo.Size() - 1 ); i >= 0; i-- )
	{
		if ( !sortInfo[ i ].m_bActive )
			continue;

		F32 fDiff = sortInfo[ i ].m_fValue01;

		if ( Maths::Abs<F32>( fDiff ) >= m_fLogicCap01 )
			continue;

		C_UIPerkInfo* pPerkInfo = sortInfo[ i ].m_pPerkInfo;
		S32 sVehicleIndexO = sortInfo[ i ].m_sVehicleIndex;

		Game::C_LevelState& levelState = GetLevelState();
		Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = levelState.mCars[sVehicleIndexMe].mUpgrades.GetMultiplayerAbilities();
		Game::C_MultiplayerVehicleAbility* radarAbility = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Advanced_Radar);
		Game::C_MultiplayerVehicleAbility* silentRunningAbility = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Silent_Running);
		if(   radarAbility->GetActive() ||
			! silentRunningAbility->GetActive())
		{

			Game::Actors::C_VehicleLogicActor* pLogicActorOther = GetVehicleLogicActor( sVehicleIndexO );

			if ( !pLogicActorOther )
				continue;
			
			Maths::HVec3 worldPosOther = pLogicActorOther->GetWorldMatrix().AW();

			// world-space position in relation to the player's vehicle
			Maths::HVec3 diffW = ( worldPosOther - worldPosMe );
			Maths::HVec3 pos = diffW * viewRotationMat;
			F32 fDiffLength = Maths::HVec2( pos.X(), pos.Z() ).Length();

			bool bOutOfRange = ( fDiffLength > m_fViewDistance );

			// distance from player in 0-1 world space * the "zoom" factor
			Maths::HVec2 mapPoint = Maths::HVec2( pos.X(), pos.Z() ) * viewScale * 0.5f;

			// spread them out in x a bit more - but not at y extremes as much
			F32 fOriginalX = mapPoint.X();
			F32 fNewX = fOriginalX;

			if ( fOriginalX < 0.0f )
				fNewX = ( Maths::Sqrt( Maths::Abs<F32>( fOriginalX * 2.0f ) ) * -1.0f ) / 2.0f;
			else
				fNewX = Maths::Sqrt( fOriginalX * 2.0f ) / 2.0f;

			fNewX = Maths::Lerp( fOriginalX, fNewX, 
				Maths::Clamp01( 1.0f - ( Maths::Abs<F32>( mapPoint.Y() * 2.0f ) ) ) );

			mapPoint.X( fNewX );

			// middle-centre
			Maths::HVec2 drawPoint = Maths::HVec2( 
				m_Position.x + ( m_fMapSize * mapPoint.X() ), 
				m_Position.y + ( m_fMapSize * mapPoint.Y() * Up() )
			);

			if ( bOutOfRange )
			{
				// edge clamp
				Maths::HVec2 diff = drawPoint - Maths::HVec2( m_Position.x, m_Position.y );
				mapPoint *= ( m_fViewDistance / ( Maths::HVec2( diffW.X(), diffW.Z() ).Length() ) );	// ratio to centre

				drawPoint = Maths::HVec2( 
					m_Position.x + ( m_fMapSize * mapPoint.X() ), 
					m_Position.y + ( m_fMapSize * mapPoint.Y() * Up() )
				);

				//fAngleZ = atan2f( diff.X(), -diff.Y() );
			}

			// colour based on perk
			C_UISpriteColours colours;
				

			if(pPerkInfo && (!C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsOnline() ||
				radarAbility->GetActive()))
			{
				colours.Set( pPerkInfo->m_ColourFill, m_AlphaInherit, m_IntensityInherit );
			}
			else
			{
				colours.Set( m_ColourOther, m_AlphaInherit, m_IntensityInherit );
			}


			if ( bOutOfRange )
			{
				// TODO - draw a rotated arrow or something?
			}

			if ( sortInfo[ i ].m_bTracking )
			{
				gSimple2D.AddSprite( m_Layer, Maths::HVec3( drawPoint.X() - m_fVehicleWidth * 1.5f, drawPoint.Y(), m_Position.z ), 
					m_fVehicleHeight * fTrackingMult, m_fVehicleHeight * fTrackingMult, colourTargeted, m_Panel, E_SpriteType_Sprite, renderStateTracking );
				gSimple2D.AddTexUVs( 0, GetTextureParam( 7 ) );
				gSimple2D.AddJustification( E_Justify_MiddleCentre );
				gSimple2D.AddRotation( 0.0f, 0.0f, -90.0f );
			}

			gSimple2D.AddSprite( m_Layer, Maths::HVec3( drawPoint.X(), drawPoint.Y(), m_Position.z ), 
				m_fVehicleWidth, m_fVehicleHeight, colours, m_Panel, E_SpriteType_Sprite, m_RenderState );
			gSimple2D.AddTexUVs( 0, GetTextureParam( 1 ) );
			gSimple2D.AddJustification( E_Justify_MiddleCentre );

			//C_UISpriteColours coloursName = m_ColourName;

			if ( m_EffectInfo[ sVehicleIndexO ].m_bActive )
			{
				F32 fEval01 = m_Timeline[ 0 ].m_Effects[ 0 ].Evaluate( m_EffectInfo[ sVehicleIndexO ].m_fTime01 );
				gSimple2D.AddRotation( 0.0f, 0.0f, 360.0f * fEval01 );

				//C_UIPerkInfo* pPerkInfoHit = GetPerkInfo( NULL, m_EffectInfo[ sVehicleIndexO ].m_ePerkType );

				//if ( pPerkInfoHit )
				//	coloursName = pPerkInfoHit->m_ColourFill;
			}

			// TODO - but the names back in?

			//coloursName.SetAlpha( colours[ 0 ].A() );

			//gFont.DrawText( m_Layer, pos + Maths::HVec3( fOffsetX - m_fVehicleWidth * m_fNameGapMult * fScaleMult, fOffsetY, 0.0f ),
			//	E_Justify_MiddleRight, GetLevelState().mCars[ sVehicleIndexO ].mNameAboveCar.Chars(), coloursName, m_fNameTextHeight, 
			//	m_FontStylePasses, 0.0f, m_Panel );

			// targeting
			if ( m_VehicleInRange[ sVehicleIndexO ] )
			{
				F32 fGap = m_fTargetingGapMult * m_fTargetingSize * 0.5f;

				C_RenderState renderState = m_RenderState;
				renderState.m_EffectIndex = E_Effect_IntensityAsAlpha;

				// left
				gSimple2D.AddSprite( m_Layer, Maths::HVec3( drawPoint.X() - fGap, drawPoint.Y(), m_Position.z ), 
					m_fTargetingSize, m_fTargetingSize, colourTargeted, m_Panel, E_SpriteType_Sprite, renderState );
				gSimple2D.AddTexUVs( 0, GetTextureParam( 5 ) );
				gSimple2D.AddJustification( E_Justify_MiddleRight );
				// right
				gSimple2D.AddSprite( m_Layer, Maths::HVec3( drawPoint.X() + fGap, drawPoint.Y(), m_Position.z ), 
					m_fTargetingSize, m_fTargetingSize, colourTargeted, m_Panel, E_SpriteType_Sprite, renderState );
				gSimple2D.AddTexUVs( 0, GetTextureParam( 6 ) );
				gSimple2D.AddJustification( E_Justify_MiddleLeft );
			}
		}
	}

	//Render Mines on the radar if the player has got the advanced radar tracking upgrade
	//if(radarAbility->GetActive())
	//{
	//	Game::C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	//	for(S32 i = 0; i < pLayer->mLiveMines.Size(); i++)
	//	{
	//		Maths::HVec3 worldPosMine = pLayer->mLiveMines[i]->mMatrix.AW();

	//		// world-space position in relation to the player's vehicle
	//		Maths::HVec3 diffW = ( worldPosMine - worldPosMe );
	//		Maths::HVec3 pos = diffW * viewRotationMat;
	//		F32 fDiffLength = Maths::HVec2( pos.X(), pos.Z() ).Length();

	//		bool bOutOfRange = ( fDiffLength > m_fViewDistance );

	//		// distance from player in 0-1 world space * the "zoom" factor
	//		Maths::HVec2 mapPoint = Maths::HVec2( pos.X(), pos.Z() ) * viewScale * 0.5f;

	//		// spread them out in x a bit more - but not at y extremes as much
	//		F32 fOriginalX = mapPoint.X();
	//		F32 fNewX = fOriginalX;

	//		if ( fOriginalX < 0.0f )
	//			fNewX = ( Maths::Sqrt( Maths::Abs<F32>( fOriginalX * 2.0f ) ) * -1.0f ) / 2.0f;
	//		else
	//			fNewX = Maths::Sqrt( fOriginalX * 2.0f ) / 2.0f;

	//		fNewX = Maths::Lerp( fOriginalX, fNewX, 
	//			Maths::Clamp01( 1.0f - ( Maths::Abs<F32>( mapPoint.Y() * 2.0f ) ) ) );

	//		mapPoint.X( fNewX );

	//		// middle-centre
	//		Maths::HVec2 drawPoint = Maths::HVec2( 
	//			m_Position.x + ( m_fMapSize * mapPoint.X() ), 
	//			m_Position.y + ( m_fMapSize * mapPoint.Y() * Up() )
	//		);

	//		if ( bOutOfRange )
	//		{
	//			continue;
	//		}

	//		// colour based on perk - SHOULD BE ORANGE
	//		C_UISpriteColours colours;			
	//		colours.Set( m_ColourMine, m_AlphaInherit, m_IntensityInherit );


	//		gSimple2D.AddSprite( m_Layer, Maths::HVec3( drawPoint.X(), drawPoint.Y(), m_Position.z ), 
	//			m_fDotSize, m_fDotSize, colours, m_Panel, E_SpriteType_Sprite, m_RenderState );
	//		gSimple2D.AddTexUVs( 0, GetTextureParam( 3 ) ); //using the dot texture
	//		gSimple2D.AddJustification( E_Justify_MiddleCentre );
	//	}
	//}

	// render you
	C_UIPerkInfo* pPerkInfoYou = GetPerkInfo( pLogicActorMe );

	C_UISpriteColours coloursYou;
				
	if(pPerkInfoYou && (!C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsOnline() || radarAbility->GetActive()))
	{
		coloursYou.Set( pPerkInfoYou->m_ColourFill, m_AlphaInherit, m_IntensityInherit );
	}
	else
	{
		coloursYou.Set( m_ColourOther, m_AlphaInherit, m_IntensityInherit );
	}


	gSimple2D.AddSprite( m_Layer, m_Position, m_fVehicleWidth, m_fVehicleHeight, 
		coloursYou, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, vehicleTex );
	gSimple2D.AddJustification( E_Justify_MiddleCentre );

	// these are only valid for one frame
	for ( S32 i = 0; i < m_VehicleInRange.Size(); i++ )
		m_VehicleInRange[ i ] = false;
}

void UIRaceLine::ProcessHitByPerk( const Messaging::C_MessageBase* pMessage )
{
	const C_VehicleLogicMsg_HitByPerkMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_HitByPerkMsg>( pMessage );
	S32 sIndex = pMsg->mVehicleIndex;

	if ( !m_EffectInfo[ sIndex ].m_bActive )
	{
		m_EffectInfo[ sIndex ].m_bActive = true;
		m_EffectInfo[ sIndex ].m_fTime01 = 0.0f;
		m_EffectInfo[ sIndex ].m_ePerkType = pMsg->mPickupType;
	}
}

void UIRaceLine::RenderLine()
{
	// this always needs to be the same drawn y size
	// -- shift uvs depending on scale instead
	F32 fSizeX = m_Size.x;
	F32 fSizeY = m_Size.y;

	F32 fInvScaleY = ( 1.0f / m_Scale.y );
	fSizeY *= fInvScaleY;

	C_TextureParam tex0 = GetTextureParam( 0 );

	// middle
	C_UISpriteColours colour;
	colour.Set( m_SpriteColours, m_AlphaInherit, m_IntensityInherit );

	C_TextureParam texM = GetTextureParam( 0 );
	texM.m_Position.v += ( texM.m_Size.v * m_fAlphaFadeSize );
	texM.m_Size.v *= ( 1.0f - ( m_fAlphaFadeSize * 2.0f ) );
	texM.m_Position.v += texM.m_Size.v * ( 1.0f - fInvScaleY ) * 0.5f;
	texM.m_Size.v *= fInvScaleY;
	
	/*
	gSimple2D.AddSprite( m_Layer, m_Position, 
		fSizeX, fSizeY * ( 1.0f - ( m_fAlphaFadeSize * 2.0f ) * Up() ), 
		colour, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, texM );
	gSimple2D.AddJustification( E_Justify_MiddleCentre );
	*/

	// top
	colour = m_SpriteColours;
	colour[ 0 ].A( 0 );
	colour[ 1 ].A( 0 );

	C_TextureParam texT = tex0;
	texT.m_Size.v *= m_fAlphaFadeSize * fInvScaleY;
	texT.m_Position.v = texM.m_Position.v - texT.m_Size.v;

	gSimple2D.AddSprite( m_Layer, m_Position + Maths::HVec3( 0.0f, fSizeY * -( 0.5f - m_fAlphaFadeSize ) * Up(), 0.0f ), 
		fSizeX, fSizeY * m_fAlphaFadeSize, 
		colour, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, texT );
	gSimple2D.AddJustification( E_Justify_BottomCentre );

	// bottom
	colour = m_SpriteColours;
	colour[ 2 ].A( 0 );
	colour[ 3 ].A( 0 );

	C_TextureParam texB = tex0;
	texB.m_Position.v = texM.m_Position.v + texM.m_Size.v;
	texB.m_Size.v *= m_fAlphaFadeSize * fInvScaleY;

	gSimple2D.AddSprite( m_Layer, m_Position + Maths::HVec3( 0.0f, fSizeY * ( 0.5f - m_fAlphaFadeSize ) * Up(), 0.0f ), 
		fSizeX, fSizeY * m_fAlphaFadeSize, 
		colour, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, texB );
	gSimple2D.AddJustification( E_Justify_TopCentre );
}

}; // namespace UI
