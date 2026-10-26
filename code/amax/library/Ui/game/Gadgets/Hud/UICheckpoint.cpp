#include "stdafx.h"

#include "UICheckpoint.h"

#include "UiHelpers.h"
#include "UIMessages.h"
#include "ui\UiManager.h"

#include <ui/Game/UIDataFinder.h>

#include <GameLogic/InGameStartLineInfoManager.h>
#include <camera2/Camera.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <Game/ActorDef/GameActorList.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

XT_TYPE( UI::UICheckpoint, "UICheckpoint" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_FontStylePasses, "styles" )
	XT_FIELD( m_fFadeNear, "fade_near" )
	XT_FIELD( m_fFadeFar, "fade_far" )
	XT_FIELD( m_fRedBoundary, "red_boundary" )
	XT_FIELD( m_xtColourRed, "colour_red" )
XT_END_TYPE

namespace UI
{

// ------------
// UICheckpoint
// ------------
void UICheckpoint::Init()
{
	C_GadgetBase::Init();

	m_fFadeNear = 0.0f;
	m_fFadeFar = 0.0f;
	m_fRedBoundary = 10.0f;
}

void UICheckpoint::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	// resolve font styles
	m_FontStylePasses.Setup( pState );

	// resolve colour styles
	m_ColourRed.SetByName( m_xtColourRed );

	m_PositionOriginal = m_Position;
}

void UICheckpoint::Render()
{
	if ( !m_Activated )
		return;

	Game::C_RaceBookLayer* pRaceBookLayer = GetRaceBookLayer();

	if ( !pRaceBookLayer || !pRaceBookLayer->GetRaceModePtr() || ( pRaceBookLayer->GetRaceMode() != Game::E_RaceMode_Checkpoint ) )
		return;

	Game::C_RaceModeCheckpoint* pRace = (Game::C_RaceModeCheckpoint*)pRaceBookLayer->GetRaceModePtr();

	if ( pRace->GetActualCheckpointsPassed() == 0 )
		return;

	bool bDoRed = ( pRace->GetTimeLeft() < m_fRedBoundary );

	Game::C_GameActorList* pActorList = GetActorList();

	if ( !pActorList )
		return;

	S32 sLocalPlayerIndex = 0;	// single player only
	m_Panel = E_GameSpritePanel_VP0 + sLocalPlayerIndex;

	C_GadgetBase::Render();

	Btl::Vector< Game::StartLineInfoType >& store = Game::C_InGameStartLineInfoManager::Instance().GetStore( sLocalPlayerIndex );

	// quick count
	S32 sCheckpointCount = 0;
	
	for ( S32 i = 0; i < store.Size(); i++ )
		if ( store[ i ].mType == Actors::StartInfoType_Checkpoint || store[ i ].mType == Actors::StartInfoType_Checkpoint_StartFinish )
			sCheckpointCount++;

	if ( sCheckpointCount == 0 )
		return;

	// time left
	const WCHAR* pString = C_UIDataFinder::Instance().GetVariableText( "HUD_SP_RACE_CHECKPOINT_TIME_LEFT" );

	// turn on z-buffer
	C_Simple2D::C_StackedRenderState::C_DepthState depth;
	depth.function	= Display::E_CmpFunc_LessEqual;
	depth.write		= Display::E_Bool_False;
	depth.enabled	= Display::E_Bool_True;

	const Cam2::C_Camera& cam = GetPlayerCamera( sLocalPlayerIndex );
	const Maths::HMat43& camMat = cam.Mat43();

	// make camera plane
	Maths::C_Normal cam_n( camMat.AZ() );
	Maths::C_Plane cam_plane( cam_n, camMat.AW() );

	gSimple2D.AddSprite( m_Layer, m_Panel, E_SpriteType_SetRendersStateStack, 1 );
	gSimple2D.AddSprite( m_Layer, m_Panel, E_SpriteType_SetDepthBuffer, *((S32*)&depth) );

	for ( S32 i = 0; i < store.Size(); i++ )
	{
		if ( store[ i ].mType != Actors::StartInfoType_Checkpoint && store[ i ].mType != Actors::StartInfoType_Checkpoint_StartFinish )
			continue;

		const Actor::C_Actor* pActor = pActorList->GetActorPtr( store[ i ].mHandle );

		if ( !pActor || ( pActor->GetActorTypeId() != Game::E_ActorTypeId_StartLineInfo ) )
			continue;

		const Actors::StartLineInfoActor* pStartLineInfoActor = (const Actors::StartLineInfoActor*)pActor;

		if ( !pStartLineInfoActor->GetCreationData() )
			continue;

		Maths::HMat43 worldMat = pStartLineInfoActor->GetCreationData()->GetCreationMatrix();

		Maths::HVec3 worldMatAW = worldMat.AW();
		Maths::HVec3 worldMatAZ = worldMat.AZ();

		if ( (F32)( cam_plane.Distance( worldMatAW ) ) < 0.0f )
			continue;

		C_UIColour colour = bDoRed ? m_ColourRed[ 0 ] : m_SpriteColours[ 0 ];

		// alpha fade on distance
		if ( m_fFadeFar > 0.0f )
		{
			F32 fDiff = ( camMat.AW() - worldMatAW ).Length();

			if ( fDiff <= m_fFadeNear )
				continue;

			if ( fDiff < m_fFadeFar )
				colour.A01( colour.A01() * ( ( fDiff - m_fFadeNear ) / ( m_fFadeFar - m_fFadeNear ) ) );
		}

		F32 fRotY = 0.0f;
		F32 rotY = Maths::RadToDeg( atan2f( worldMatAZ.X(), worldMatAZ.Z() ) );

		Maths::C_Normal n( worldMatAZ );
		Maths::C_Plane plane( n, worldMatAW );

		if ( (F32)( plane.Distance( camMat.AW() ) ) <= 0.0f )
			fRotY = rotY;
		else
		{
			// don't render when looking back at the start line
			if ( pRace->GetCheckpointsPassed() == 0 )
				continue;

			fRotY = rotY + 180.0f;
		}

		m_Rotation.y = fRotY;
		m_Position = m_PositionOriginal + worldMatAW;

		// auto-transform
		C_UIAutoTransform transform( this );

		gFont.FixedSpaceNext( E_FixedSpaceType_Numbers );
		gFont.DrawText( m_Layer, m_Position, m_Justify, pString, colour, m_Size.y, m_FontStylePasses, m_Size.x, m_Panel );

		// matrix needs calculating each time
		DirtySet( E_Dirty_Matrix, true );
	}

	// restore previous render state
	gSimple2D.AddSprite( m_Layer, m_Panel, E_SpriteType_SetRendersStateStack, 0 );
}

}; // namespace UI
