#include "stdafx.h"
#include <GameLogic/Layers/RaceBookLayer.h>	// [Blur2PC]

#include "UIAiHud.h"
#include "UiHelpers.h"

#include <ui/Game/Gadgets/ButtonMessages.h>
#include <ui/Game/UILocaliser.h>
#include <Ui/UiInterface.h>
#include <Ui/SoundEffects.h>

#include "GameLogic/RaceRules/RaceRulesManager.h"
#include "GameLogic/Structures/MpResults.h"

#include "UiPerks.h"
#include "UiDamage.h"
#include "UiTargeting.h"
#include "UiXtConfig.h"

#include "UiManager.h"
#include "UiMessages.h"

#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_NewScoring.h>

#include <algorithm>
#include "GameLogic\RaceLogicCore\Core\RaceLogicCore.h"
#include <camera2/Camera.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

Debug::Bool g_UiHudCarInfoBillboardToCarInsteadOfCamera( "UI.HUD.BillboardToCarInsteadOfCamera", true, Debug::Persist() );
Debug::Bool	g_UiHudCarInfoShowVehicleIndex("Perks.Scripts.ShowVehicleIndex", false, Debug::Persist());

XT_TYPE( UI::UIAiHud, "UIAiHud" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_fFadeNearStart, "fade_near_start" )
	XT_FIELD( m_fFadeNearEnd, "fade_near_end" )
	XT_FIELD( m_fFadeStart, "fade_start" )
	XT_FIELD( m_fFadeEnd, "fade_end" )
	XT_FIELD( m_bDisable, "disable" )
	XT_FIELD( m_sMaxDrawCount, "max_draw_count" )
	XT_FIELD( m_fScaleStart, "scale_start" )
	XT_FIELD( m_fScaleMultAddPerMetre, "scale_mult_add_per_metre" )
	XT_FIELD( m_fScaleMax, "scale_max" )
XT_END_TYPE

Debug::Bool g_ForceRivalIndex("UI.HUD.RivalArrow.ForceRivalIndex", false);
Debug::Int g_DebugRivalIndex("UI.HUD.RivalArrow.RivalIndex", -1, Debug::Range(0, 19));

namespace UI
{

// ------------
// C_AiSortInfo
// ------------
bool C_AiSortInfo::operator < ( const C_AiSortInfo& o ) const
{
	return ( m_fDistanceSq < o.m_fDistanceSq );
}

// -------
// UIAiHud
// -------
void UIAiHud::Init()
{
	C_GadgetBase::Init();

	m_fFadeNearStart = 2.0f;
	m_fFadeNearEnd = 4.0f;

	m_fFadeStart = 5.0f;
	m_fFadeEnd = 15.0f;

	m_fScaleStart = 0.0f;
	m_fScaleMultAddPerMetre = 0.0f;
	m_fScaleMax = 1.0f;

	m_fParentScaleStart = 1.0f;

	m_bDisable = false;

	m_sMaxDrawCount = 3;
	m_DoSort = 0;

	for ( S32 i = 0; i < m_Vehicle.Size(); i++ )
		m_Vehicle[i].m_TeamColourSet = false;

	Game::C_LevelState& levelState = GetLevelState();
	m_bShowNames = ShowAiNames();
	m_bShowPerks = ( levelState.mRaceMode != Game::E_RaceMode_Destruction  ) && ( levelState.mRaceMode != Game::E_RaceMode_FriendDemand_Destruction );

	// messages
	AddSubscriber( E_UiGameMsg_SetLocalPlayerIndex );

	// figure out if we have a boss (or two) in our race
	for ( S32 i = 0; i < m_BossVehicles.Size(); i++ )
		m_BossVehicles[ i ] = -1;
	
	if ( m_bShowNames )
		return;

	Game::C_RaceBookLayer* pRaceBookLayer = GetRaceBookLayer();

	if ( !pRaceBookLayer || !pRaceBookLayer->GetGameModePtr() || ( pRaceBookLayer->GetGameModePtr()->GetID() != Game::E_GameMode_SinglePlayer ) )
		return;

	SP_Event* pEvent = SP_GetEventById( SP_GetCurrentSaveGame(), levelState.mSpEventId );

	if ( !pEvent || !pEvent->event )
		return;

	S32 sCount = 0;

	for ( S32 i = 0; i < pEvent->event->opponents.count; i++ )
	{
		if ( !pEvent->event->opponents[ i ].opponent )
			continue;

		if ( pEvent->event->opponents[ i ].opponent->type == E_CHARACTER_TYPE_BOSS )
			m_BossVehicles[ sCount++ ] = i;
	}
}

void UIAiHud::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	Game::Actors::RaceLogicCore* pCore = GetRLC();

	if ( pCore )
	{
		m_Objects.Reserve( pCore->GetNumVehicles() );

		if ( pCore->GetNumVehicles() > 0 )
		{
			S32 sNumSortVehicles = pCore->GetNumVehicles() - 1; // don't want to sort me

			m_SortedVehicles.Reserve( sNumSortVehicles );

			for ( S32 i = 0; i < sNumSortVehicles; i++ )
				m_SortedVehicles.PushBack( C_AiSortInfo() );
		}
	}
}

void UIAiHud::Resolve( C_GadgetBase* pParentClone )
{
	if ( !pParentClone )
		return;

	m_Objects.PushBack( AiGadgets() );

	AiGadgets& gadgets = m_Objects.Back();

	for ( S32 i = 0; i < E_AiHud_Count; i++ )
		gadgets[ i ] = NULL;

	gadgets[ E_AiHud_Parent ] = pParentClone;

	m_fParentScaleStart = pParentClone->m_Scale.x;

	ButtonArray children;
	pParentClone->FindChildren( &children, true );

	for ( S32 i = 0; i < children.Size(); i++ )
	{
		if ( !children[ i ]->m_Name )
			continue;

			 if ( !strcmp( children[ i ]->m_Name, "ai_name" ) )					gadgets[ E_AiHud_Name ] = children[ i ];
		else if ( !strcmp( children[ i ]->m_Name, "ai_position" ) )				gadgets[ E_AiHud_Position ] = children[ i ];
		else if ( !strcmp( children[ i ]->m_Name, "ai_perks" ) )				gadgets[ E_AiHud_Perks ] = children[ i ];
		else if ( !strcmp( children[ i ]->m_Name, "ai_damage" ) )				gadgets[ E_AiHud_Damage ] = children[ i ];
		else if ( !strcmp( children[ i ]->m_Name, "ai_targeting" ) )			gadgets[ E_AiHud_Targeting ] = children[ i ];
		else if ( !strcmp( children[ i ]->m_Name, "ai_targeting_left" ) )		gadgets[ E_AiHud_TargetingLeft ] = children[ i ];
		else if ( !strcmp( children[ i ]->m_Name, "ai_targeting_right" ) )		gadgets[ E_AiHud_TargetingRight ] = children[ i ];
		else if ( !strcmp( children[ i ]->m_Name, "ai_winner_icon" ) )			gadgets[ E_AiHud_WinnerIcon ] = children[ i ];
		else if ( !strcmp( children[ i ]->m_Name, "ai_winner_pos" ) )			gadgets[ E_AiHud_WinnerPos ] = children[ i ];
		else if ( !strcmp( children[ i ]->m_Name, "ai_rival_icon" ) )			gadgets[ E_AiHud_RivalIcon ] = children[ i ];
		else if ( !strcmp( children[ i ]->m_Name, "ai_rival_arrow" ) )			gadgets[ E_AiHud_RivalArrow ] = children[ i ];
		else if ( !strcmp( children[ i ]->m_Name, "ai_team_icon" ) )			gadgets[ E_AiHud_TeamIcon ] = children[ i ];
	}

	for ( S32 i = 0; i < E_AiHud_Count; i++ )
	{
		if ( !gadgets[ i ] )
			continue;
		
		if ( i != E_AiHud_RivalArrow && i != E_AiHud_TargetingLeft && i != E_AiHud_TargetingRight )
			gadgets[ i ]->m_Layer = ( ( i == E_AiHud_Name ) || ( i == E_AiHud_Position ) ) ? m_Layer : m_Layer + 1;

		gadgets[ i ]->m_Panel = m_Panel;

		gadgets[ i ]->RemoveSubscriber( E_UiMsg_FrameUpdate );
		gadgets[ i ]->RemoveSubscriber( E_UiMsg_Render );
	}

	S32 sVehicleIndex = m_Objects.Size() - 1;

	if ( gadgets[ E_AiHud_Perks ] )
		((UIPerks*)gadgets[ E_AiHud_Perks ])->SetVehicleIndexOverride( sVehicleIndex );

	if ( gadgets[ E_AiHud_Damage ] )
		((UIDamage*)gadgets[ E_AiHud_Damage ])->SetVehicleIndexOverride( sVehicleIndex );

	if ( gadgets[ E_AiHud_Targeting ] )
		((UITargeting*)gadgets[ E_AiHud_Targeting ])->SetVehicleIndexOverride( sVehicleIndex );
}

bool UIAiHud::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated )
		return C_GadgetBase::MessageHandler( pMessage );

	switch ( pMessage->Type() )
	{
		case E_UiGameMsg_SetLocalPlayerIndex:
			if ( m_sLocalPlayerIndex == -1 )
			{
				m_sLocalPlayerIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
				m_Panel = E_GameSpritePanel_VP0 + m_sLocalPlayerIndex;
			}
			break;

		default:
			return C_GadgetBase::MessageHandler( pMessage );
	}

	return true;
}

void UIAiHud::Render()
{
	//PERF_EVENT( "UIAiHud.Render" );

	if ( !m_Activated || m_bDisable || ( m_sLocalPlayerIndex == -1 ) )
		return;

	Game::Actors::RaceLogicCore* pCore = GetRLC();

	if ( !pCore )
		return;

	S32 sPlayerVehicleIndex = GetPlayerVehicleIndex( m_sLocalPlayerIndex );
	Game::Actors::C_VehicleLogicActor* pLogicActorMe = GetVehicleLogicActor( sPlayerVehicleIndex );

	if ( !pLogicActorMe )
		return;

	// sort based on distance
	S32 sCount = 0;

	m_DoSort++;
	if( m_DoSort > 15 )
	{
		for ( S32 i = 0; i < (S32)pCore->GetNumVehicles(); i++ )
		{
			if ( i == sPlayerVehicleIndex )
				continue;

			m_SortedVehicles[ sCount ].m_sVehicleIndex = i;
			m_SortedVehicles[ sCount ].m_fDistanceSq = pLogicActorMe->mVehicleProximities[ i ].mDistanceSquared;

			sCount++;
		}
		std::sort( m_SortedVehicles.Begin(), m_SortedVehicles.End() );

		m_DoSort = 0;
	}

	S32 sDrawCount = 0;

	const Cam2::C_Camera& cam = GetPlayerCamera( m_sLocalPlayerIndex );
	const Maths::HMat43& camMat = cam.Mat43();
	F32 camRotY = Maths::RadToDeg( atan2f( camMat.AZ().X(), camMat.AZ().Z() ) );

	Game::C_LevelState& levelState = GetLevelState();

	if ( levelState.mNumViewports == 1 )
	{
		// turn on z-buffer
		C_Simple2D::C_StackedRenderState::C_DepthState depth;
		depth.function	= Display::E_CmpFunc_LessEqual;
		depth.write		= Display::E_Bool_False;
		depth.enabled	= Display::E_Bool_True;

		gSimple2D.AddSprite( m_Layer,		m_Panel, E_SpriteType_SetRendersStateStack, 1 );
		gSimple2D.AddSprite( m_Layer + 1,	m_Panel, E_SpriteType_SetRendersStateStack, 1 );
		gSimple2D.AddSprite( m_Layer,		m_Panel, E_SpriteType_SetDepthBuffer, *((S32*)&depth) );
		gSimple2D.AddSprite( m_Layer + 1,	m_Panel, E_SpriteType_SetDepthBuffer, *((S32*)&depth) );
	}

	// make camera plane
	Maths::C_Normal cam_n( camMat.AZ() );
	Maths::C_Plane cam_plane( cam_n, camMat.AW() );

	Game::Actors::C_VehicleLogicActor* pLogicActor = NULL;

	Game::Actors::C_VehicleLogicActor* pPlayerLogicActor = GetVehicleLogicActor(sPlayerVehicleIndex);

	for ( S32 i = 0; i < m_SortedVehicles.Size(); i++ )
	{
		S32 sVehicleIndexO = m_SortedVehicles[ i ].m_sVehicleIndex;

		if ( sVehicleIndexO == sPlayerVehicleIndex )
			continue;

		pLogicActor = GetVehicleLogicActor( sVehicleIndexO );

		if ( !pLogicActor ||pLogicActor->IsCloaked() )
			continue;

		Game::Actors::C_VehicleActor* pVehicleActor = pLogicActor->GetGraphicalActor( sVehicleIndexO );

		if ( !pVehicleActor || !pVehicleActor->GetCollisionBody() || !(GetLevelState().mCars[ sVehicleIndexO ].mState == Game::E_VEHICLE_STATE_RACING) )
			continue;

		if ( !pVehicleActor->IsVisibleInMainView() )
			continue;

		// dont crash when going through debug menu
		if(m_Objects.Size() < sVehicleIndexO)
			continue;

		AiGadgets& objects = m_Objects[ sVehicleIndexO ];

		if ( !objects[ E_AiHud_Parent ] )
			continue;

		if ( m_bShowPerks && objects[ E_AiHud_Perks ] )
			objects[ E_AiHud_Perks ]->FrameUpdate();

		if ( objects[ E_AiHud_Damage ] )
			objects[ E_AiHud_Damage ]->FrameUpdate();

		if ( objects[ E_AiHud_Targeting ] )
			objects[ E_AiHud_Targeting ]->FrameUpdate();

		const Maths::HMat43& worldMat = pLogicActor->GetWorldMatrix();

		if ( (F32)( cam_plane.Distance( worldMat.AW() ) ) < 0.0f )
			continue;

		// Make sure we recalc matrix positions
		for( S32 j = 0; j < objects.Size(); j++ )
			if( objects[ j ] )
				objects[ j ]->DirtySet( E_Dirty_Matrix, true );

		F32 fAlpha = 1.0f;

		// position above the car
		Maths::HVec3 posMe = cam.Mat43().AW(); // pCore->GetVehicleWorldPosition( sPlayerVehicleIndex );
		Maths::HVec3 posAI = pVehicleActor->GetCollisionBody()->Position();

		F32 fDistance = ( posMe - posAI ).Length();

		if ( fDistance < m_fFadeNearStart )
			fAlpha = 0.0f;
		else if ( fDistance < m_fFadeNearEnd )
			fAlpha = ( fDistance - m_fFadeNearStart ) / ( m_fFadeNearEnd - m_fFadeNearStart );
		else
		{
			if ( fDistance > m_fFadeEnd )
				fAlpha = 0.0f;
			else if ( fDistance < m_fFadeStart )
				fAlpha = 1.0f;
			else
				fAlpha = 1.0f - ( ( fDistance - m_fFadeStart ) / ( m_fFadeEnd - m_fFadeStart ) );
		}

		bool bWinner = false;
		bool bRival = false;

		if ( levelState.mNetworkRace && ( levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction ) )
			bWinner = ( Game::C_MpResults::Instance().m_SortedVehicleResults[ 0 ].m_LocalIndex == sVehicleIndexO );

		if ( ( levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction ) && ( pPlayerLogicActor->pNewScoringComponent->GetRivalIndex() == sVehicleIndexO ) )
		{
			Game::Actors::C_VehicleLogicActor* pPlayerLogicActor = GetVehicleLogicActor(sVehicleIndexO);
			if(!pPlayerLogicActor->IsGhosted())
			{
				bRival = true;
			}

		}
		
		if(g_ForceRivalIndex && g_DebugRivalIndex == sVehicleIndexO)
			bRival = true;

		if ( bWinner || bRival )
			fAlpha = 1.0f;

		U8 alpha_cache = (U8)(fAlpha * 255.0f);
		const bool alpha_changed = ( m_Vehicle[ sVehicleIndexO ].m_Alpha != alpha_cache );
		m_Vehicle[ sVehicleIndexO ].m_Alpha = alpha_cache;

		// parent
		objects[ E_AiHud_Parent ]->DirtySet( E_Dirty_Alpha, alpha_changed );
		objects[ E_AiHud_Parent ]->m_Alpha = fAlpha;

		F32 fExtentsY = pVehicleActor->GetCollisionBody()->GetLocalBoundingBox().extents.Y();

		static F32 fMult = 1.0f;
		
		posAI.Y( posAI.Y() + fExtentsY * fMult );

		objects[ E_AiHud_Parent ]->m_Position = posAI + m_Position;
#if PLATFORM_PC
		// [Blur2PC] render interpolation: the badges are placed at the car's update pose; each rendered frame
		// moves them with the car to the pose it is drawn at (translation only: they face the camera)
		gSimple2D.Blur2PCSetAnchor( objects[ E_AiHud_Parent ], sVehicleIndexO, false, pVehicleActor->Blur2PCRenderMatrix() );
#endif

		// targeting
		if ( objects[ E_AiHud_Targeting ] )
			objects[ E_AiHud_Targeting ]->Render();

		// targeting left
		if ( objects[ E_AiHud_TargetingLeft ] )
			objects[ E_AiHud_TargetingLeft ]->Render();

		// targeting right
		if ( objects[ E_AiHud_TargetingRight ] )
			objects[ E_AiHud_TargetingRight ]->Render();

		if ( ( sDrawCount >= m_sMaxDrawCount ) && !bWinner && !bRival )
			continue;

		if ( fAlpha == 0.0f )
			continue;

		if ( fAlpha == 1.0f )
			fAlpha = 0.99f;	// gargh!

		if ( g_UiHudCarInfoBillboardToCarInsteadOfCamera && levelState.mRaceMode != Game::E_RaceMode_Multiplayer_Destruction )
		{
			F32 rotY = Maths::RadToDeg( atan2f( worldMat.AZ().X(), worldMat.AZ().Z() ) );
			Maths::C_Normal n( worldMat.AZ() );
			Maths::C_Plane plane( n, worldMat.AW() );

			if ( (F32)( plane.Distance( camMat.AW() ) ) <= 0.0f )
				objects[ E_AiHud_Parent ]->m_Rotation.y = rotY;
			else
				objects[ E_AiHud_Parent ]->m_Rotation.y = rotY + 180.0f;
		}
		else
			objects[ E_AiHud_Parent ]->m_Rotation.y = camRotY;

		objects[ E_AiHud_Parent ]->Render();

		// perks
		if ( m_bShowPerks && objects[ E_AiHud_Perks ] )
			objects[ E_AiHud_Perks ]->Render();
		
		// damage
		if ( objects[ E_AiHud_Damage ] )
		{
			// Only render health if user is not ghosted
			if(!pLogicActor->IsGhosted())
			{
				objects[ E_AiHud_Damage ]->Render();
			}
		}

		// team icon
		if ( levelState.mTeams && objects[ E_AiHud_TeamIcon ] )
		{
			const U8 team = levelState.mCars[ sVehicleIndexO ].mTeam;
			C_UISpriteColours teamColour = team ? C_UiXtConfig::Instance()->m_TeamBColour : C_UiXtConfig::Instance()->m_TeamAColour;

			// Offset text u pos
			C_TextureParam& tex = objects[ E_AiHud_TeamIcon ]->GetTextureParam( 0 );
			tex.m_Position.u = tex.m_Size.u * team;

			objects[ E_AiHud_TeamIcon ]->m_SpriteColours = teamColour;
			objects[ E_AiHud_TeamIcon ]->Render();			
		}

		// name
		// -- exception - in single player we want to show the boss name even if names are off
		if ( ( m_bShowNames || IsBossCar( sVehicleIndexO ) ) && objects[ E_AiHud_Name ] )
		{
			objects[ E_AiHud_Name ]->DirtySet( E_Dirty_Alpha, alpha_changed );
			C_WideString64 string;

			if(g_UiHudCarInfoShowVehicleIndex)
				string.Format( L"%s - %i", levelState.mCars[ sVehicleIndexO ].mNameAboveCar.Chars(), sVehicleIndexO );
			else
				string.Format( L"%s", levelState.mCars[ sVehicleIndexO ].mNameAboveCar.Chars() );

			((UIStaticText*)objects[ E_AiHud_Name ])->SetDirectText( string.Chars() );
			((UIStaticText*)objects[ E_AiHud_Name ])->SetCache( &m_Vehicle[ sVehicleIndexO ].m_TextCache );
			objects[ E_AiHud_Name ]->Render();
		}

		if ( bRival && objects[ E_AiHud_RivalIcon ] && objects[ E_AiHud_RivalArrow ])
		{
			if ( fDistance > m_fScaleStart )
			{
				F32 fScaleMult = Maths::Clamp( 1.0f + ( ( fDistance - m_fScaleStart ) * m_fScaleMultAddPerMetre ), 0.0f, m_fScaleMax );
				F32 fNewScale  = m_fParentScaleStart * fScaleMult;

				objects[ E_AiHud_RivalArrow ]->m_Scale.x = fNewScale;
				objects[ E_AiHud_RivalArrow ]->m_Scale.y = fNewScale;
			}

			objects[ E_AiHud_RivalArrow ]->DirtySet( E_Dirty_Matrix, true );

			objects[ E_AiHud_RivalIcon ]->Render();
			objects[ E_AiHud_RivalArrow ]->Render();
		}

		if ( !levelState.mTeams )
		{
			if ( !bRival && bWinner && objects[ E_AiHud_WinnerIcon ] && objects[ E_AiHud_WinnerPos ] ) // rival overrides winner icon
			{
				objects[ E_AiHud_WinnerIcon ]->Render();
				objects[ E_AiHud_WinnerPos ]->Render();
			}
		}

		sDrawCount++;
	}

	if ( levelState.mNumViewports == 1 )
	{
		// restore previous render state
		gSimple2D.AddSprite( m_Layer,		m_Panel, E_SpriteType_SetRendersStateStack, 0 );
		gSimple2D.AddSprite( m_Layer + 1,	m_Panel, E_SpriteType_SetRendersStateStack, 0 );
	}

	// auto-transform
	C_UIAutoTransform transform( this );

	C_GadgetBase::Render();
}

#if PLATFORM_PC
static bool Blur2PCResolveCar( const S32 sVehicleIndex, Maths::HMat43& renderMatrix )	// [Blur2PC]
{
	Game::Actors::C_VehicleLogicActor* pLogicActor = GetVehicleLogicActor( sVehicleIndex );
	Game::Actors::C_VehicleActor* pVehicleActor = pLogicActor ? pLogicActor->GetGraphicalActor( sVehicleIndex ) : NULL;
	if ( !pVehicleActor )
		return false;
	renderMatrix = pVehicleActor->Blur2PCRenderMatrix();
	return true;
}

void UIAiHud::Blur2PCPlaceCarSprites()
{
	gSimple2D.Blur2PCPlaceAnchored( &Blur2PCResolveCar );
}
#endif

bool UIAiHud::IsBossCar( const S32 sVehicleIndex )
{
	for ( S32 i = 0; i < m_BossVehicles.Size(); i++ )
	{
		if ( m_BossVehicles[ i ] == -1 )
			return false;	// as they're added in order

		if ( m_BossVehicles[ i ] == sVehicleIndex )
			return true;
	}

	return false;
}

}; // namespace UI
