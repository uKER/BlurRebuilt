#include "stdafx.h"

#include "UITargeting.h"
#include "UiHelpers.h"

#include <Ui/UiInterface.h>
#include <Ui/SoundEffects.h>
#include <Ui/Game/Gadgets/UIStaticText.h>
#include "Ui/UiXtConfig.h"
#include "Ui/Game/Gadgets/HUD/UIPerks.h"
#include "UiMessages.h"

#include <Game/Game.h>
#include <Game/GameWorld/GameWorld.h>
#include <ui/Graphics/SimpleTimer.h>
#include <Camera2/Camera.h>
#include <camera2/stock/DebugFeed.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

XT_TYPE( UI::UITargeting, "UITargeting" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_xtColourTargeted, "colour_targeted" )
	XT_FIELD( m_fColourLerp, "colour_lerp" )
	XT_FIELD( m_fScaleMult, "scale_mult" )
	XT_FIELD( m_fTargetedIntensityAdd, "targeted_intensity_add" )
XT_END_TYPE

Debug::Logger &g_uiTargettingLog( Debug::GetLogger("UI.Targetting") );

namespace UI
{

static const char* s_pGadgetNames[ 4 ] = 
{
	"targeting_top",
	"targeting_bottom",
	"targeting_left",
	"targeting_right",
};

static const char* s_pEffectLockedNames[ 6 ] = 
{
	"targeting_top_hard",
	"targeting_bottom_hard",
	"targeting_left_hard",
	"targeting_right_hard",
	"targeting_top_hard_slow",
	"targeting_bottom_hard_slow",
};

// -----------
// UITargeting
// -----------
void UITargeting::Init()
{
	C_GadgetBase::Init();

	m_fColourLerp = 0.0f;
	m_fScaleMult = 0.0f;
	m_fTargetedIntensityAdd = 0.25f;
	m_fMessageTimer = 0.0f;

	m_sVehicleIndexOverride = -1;
	m_PrevTargetted = false;
	m_bPlayerInRangeLocked = false;
	m_bEffectsStartedLocked = false;

	for ( S32 i = 0; i < 4; i++ )
	{
		m_Gadgets[ i ] = NULL;
		m_SegmentActive[ i ] = false;
		m_PlayerInRange[ i ] = false;
		m_EffectsStartedInRange[ i ] = false;
	}

	m_pPerkedText = NULL;

	ResetTargets();

	// messages
	AddSubscriber( E_UiMsg_FrameUpdate );
	AddSubscriber( E_UiGameMsg_VehicleInRangeMessage );
	AddSubscriber( E_UiGameMsg_PlayerInRangeWarningMessage );
	AddSubscriber( E_UiGameMsg_SetLocalPlayerIndex );
	AddSubscriber( E_UiGameMsg_ScoringEventPerk );
}

void UITargeting::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	// resolve colour styles
	m_ColourTargeted.SetByName( m_xtColourTargeted );
}

void UITargeting::PostInit()
{
	ButtonArray childrenMe;
	FindChildren( &childrenMe, true );

	C_GadgetBase* p = NULL;

	p = ArrayFindByName( childrenMe, "txt_PerkedText" );
	if( p && p->GetType() == E_UIButtonType_StaticText )
		m_pPerkedText = (UIStaticText*)p;
}

void UITargeting::SetupGadgets()
{
	// find the segments
	ButtonArray childrenMe;
	FindChildren( &childrenMe, true );

	for ( S32 i = 0; i < 4; i++ )
	{
		m_Gadgets[ i ] = ArrayFindByName( childrenMe, s_pGadgetNames[ i ] );

		if ( !m_Gadgets[ i ] || ( m_Gadgets[ i ]->GetType() != E_UIButtonType_StaticGraphic ) )
		{
			//LOGERROR( g_Log, "Targeting gadget[%d] invalid", i );	// removed as it's happening for ai gadgets which isn't an error
			//m_Activated = false;
			return;
		}
	}
}

bool UITargeting::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated )
		return C_GadgetBase::MessageHandler( pMessage );

	switch ( pMessage->Type() )
	{
		case E_UiGameMsg_SetLocalPlayerIndex:
			if ( m_sLocalPlayerIndex == -1 )
			{
				m_sLocalPlayerIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
				SetupGadgets();
			}
			break;

		case E_UiGameMsg_VehicleInRangeMessage:
		{
			// a vehicle is being targetted
			if ( m_sVehicleIndexOverride == -1 )
				return true;

			const C_UiVehicleInRangeMessage* pMsg = Messaging::MessageCast<C_UiVehicleInRangeMessage>( pMessage );

			S32 sVehicleIndex = GetPlayerVehicleIndex( m_sLocalPlayerIndex );

			if ( pMsg->m_sPlayerVehicleIndex == sVehicleIndex && pMsg->m_sVehicleIndex == m_sVehicleIndexOverride )
				m_bPlayerInRangeLocked = true;

		#if 0
			// enemy is in range for me to fire
			S32 sVehicleIndex = GetPlayerVehicleIndex( m_sLocalPlayerIndex );

			if ( pMsg->m_sPlayerVehicleIndex == sVehicleIndex )
				m_VehicleInRange[ pMsg->m_sVehicleIndex ] = true;
		#endif

			break;
		}

		case E_UiGameMsg_PlayerInRangeWarningMessage:
		{
			// i am in range of an enemy who is firing
			const C_UiPlayerInRangeWarningMessage* pMsg = Messaging::MessageCast<C_UiPlayerInRangeWarningMessage>( pMessage );

			//if ( !m_PrevTargetted && m_sVehicleIndexOverride == -1 )
				//UiInterface()->PlaySound( E_UiSoundEffect_TargetLocked );

			S32 sVehicleIndex = GetPlayerVehicleIndex( m_sLocalPlayerIndex );

			if ( pMsg->m_sPlayerVehicleIndex == sVehicleIndex && m_sVehicleIndexOverride == -1 )
			{
				m_PlayerInRange[1] = true; //Behind
				m_bPlayerInRangeLocked = true;
			}

			break;
		}

		case E_UiGameMsg_ScoringEventPerk:
			if( m_sVehicleIndexOverride == -1 )
				ProcessHitByPerk( Messaging::MessageCast<C_UiScoringEventPerk>( pMessage ) );
			return true;

		default:
			return C_GadgetBase::MessageHandler( pMessage );
	}

	return true;
}

void UITargeting::ProcessHitByPerk( const C_UiScoringEventPerk* pMessage )
{
	const U8 to = pMessage->m_VehicleIndex;

	if( to != m_sLocalPlayerIndex )
		return;

	C_UIPerkInfo *pPerkInfo = C_UiXtConfig::Instance()->m_PerkDisplayParameters.GetPerkInfo( pMessage->m_PickupType );
	if( !pPerkInfo )
		return;

	if( !m_pPerkedText )
		return;

	return; // Put back in when PK has done us a 'negative' message for this	// Perked
	m_pPerkedText->SetDirectText( pMessage->m_ScoreString );
	m_pPerkedText->DoCalcStringSize();
	m_pPerkedText->Activate( true, true );

	m_fMessageTimer = 2.0f; // TODO : Grab from RLC?
}

void UITargeting::FrameUpdate()
{
	if ( !m_Activated || ( m_sLocalPlayerIndex == -1 ) )
		return;

	if( m_fMessageTimer > 0.0f )
	{
		m_fMessageTimer -= theFETimer.FrameTime();
		if( m_fMessageTimer <= 0.0f )
			m_pPerkedText->Activate( true, false );
	}

	S32 sVehicleIndex = GetPlayerVehicleIndex( m_sLocalPlayerIndex );

	const Cam2::C_Camera& cam = GetPlayerCamera( m_sLocalPlayerIndex );
	F32 fViewAngle = -atan2f( cam.Mat43().AZ().X(), cam.Mat43().AZ().Z() );

	Game::Actors::C_VehicleLogicActor* pLogicActor		= GetVehicleLogicActor( sVehicleIndex );
	Game::Actors::C_VehicleLogicActor* pOtherLogicActor = NULL;

	if ( !pLogicActor )
		return;

#if 0
	if ( m_sVehicleIndexOverride == -1 )
	{
		// determine which segments are active
		for ( S32 i = 0; i < m_PlayerInRange.Size(); i++ )
		{
			if ( !m_PlayerInRange[ i ] )
				continue;

			pOtherLogicActor = GetVehicleLogicActor( i );

			if ( !pOtherLogicActor )
				continue;

			Maths::HVec3 diff = pOtherLogicActor->GetWorldMatrix().AW() - pLogicActor->GetWorldMatrix().AW();
			F32 fVehicleAngle = -atan2f( diff.X(), diff.Z() );
			F32 fAngle = fViewAngle - fVehicleAngle;

			if ( Maths::Finite( fAngle ) )
			{
				while ( fAngle < 0.0f )
					fAngle += Maths::Pi2;
				while ( fAngle >= Maths::Pi2 )
					fAngle -= Maths::Pi2;
			}
			else
				LOGERROR( g_uiTargettingLog, "UITargeting::FrameUpdate() - Vehicle positions UNDEFINED!" );
			
			fAngle += ( Maths::Pi2 / m_SegmentActive.Size() ) * 0.5f;

			S32 sSegment = Maths::Clamp( (S32)( fAngle / ( Maths::Pi2 / m_SegmentActive.Size() ) ), 0, m_SegmentActive.Size() - 1 );

			m_SegmentActive[ sSegment ] = true;
		}
	}
	else
	{
		// determine which segments are active
		if ( m_VehicleInRange[ m_sVehicleIndexOverride ] )
		{
			pOtherLogicActor = GetVehicleLogicActor( m_sVehicleIndexOverride );

			if ( !pOtherLogicActor )
				return;

			m_bPlayerInRangeLocked = true;

			Maths::HVec3 diff = pLogicActor->GetWorldMatrix().AW() - pOtherLogicActor->GetWorldMatrix().AW();
			F32 fVehicleAngle = -atan2f( diff.X(), diff.Z() );
			F32 fAngle = fViewAngle - fVehicleAngle;

			if ( Maths::Finite( fAngle ) )
			{
				while ( fAngle < 0.0f )
					fAngle += Maths::Pi2;
				while ( fAngle >= Maths::Pi2 )
					fAngle -= Maths::Pi2;
			}
			else
				LOGERROR( g_uiTargettingLog, "UITargeting::FrameUpdate() - Vehicle positions UNDEFINED!" );

			fAngle += ( Maths::Pi2 / m_SegmentActive.Size() ) * 0.5f;

			S32 sSegment = Maths::Clamp( (S32)( fAngle / ( Maths::Pi2 / m_SegmentActive.Size() ) ), 0, m_SegmentActive.Size() - 1 );

			m_SegmentActive[ sSegment ] = true;
		}
	}
#endif

	if ( m_sVehicleIndexOverride == -1 )
	{
		// player effects
		if ( m_bPlayerInRangeLocked && !m_bEffectsStartedLocked )
		{
			m_bEffectsStartedLocked = true;
			SendPrivateTimelineMessaage( "targeting_on", true, 0.0f );
		}
		else if ( !m_bPlayerInRangeLocked && m_bEffectsStartedLocked )
		{
			m_bEffectsStartedLocked = false;
			SendPrivateTimelineMessaage( "targeting_on", false );
		}

		Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
		bool hasECM = levelState.mCars[m_sLocalPlayerIndex].mUpgrades.GetMultiplayerAbilities()->GetAbility(UI::E_MultiplayerVehicleAbilities_ECM)->GetActive();


		for ( S32 i = 0; i < 4; i++ )
		{
			if ( ( m_bPlayerInRangeLocked && m_PlayerInRange[ i ] ) && !m_EffectsStartedInRange[ i ] )
			{
				if ( m_Gadgets[ i ] )
					m_Gadgets[ i ]->SendPrivateTimelineMessaage( s_pEffectLockedNames[ i <= 1 && hasECM ? i+4 : i ], true, 0.0f );
				m_EffectsStartedInRange[ i ] = true;
			}
			else if ( ( !m_bPlayerInRangeLocked || !m_PlayerInRange[ i ] ) && m_EffectsStartedInRange[ i ] )
			{
				if ( m_Gadgets[ i ] )
					m_Gadgets[ i ]->SendPrivateTimelineMessaage( s_pEffectLockedNames[ i <= 1 && hasECM ? i+4 : i ], false );
				m_EffectsStartedInRange[ i ] = false;
			}
		}
	}
	else
	{
		// ai effects
		if ( m_bPlayerInRangeLocked && !m_bEffectsStartedLocked )
		{
			m_bEffectsStartedLocked = true;
			Util::C_String32 string;
			string.Printf( "targeted_%d_%d", m_sLocalPlayerIndex, m_sVehicleIndexOverride );
			C_UiTimelineMessage msg( string.Chars(), true, 0.0f );
			gButtons.BroadcastMessageNow( 0xffffffff, &msg );
			UiInterface()->PlaySound( E_UiSoundEffect_LockHardAIActionIn );
		}
		else if ( !m_bPlayerInRangeLocked && m_bEffectsStartedLocked )
		{
			m_bEffectsStartedLocked = false;
			Util::C_String32 string;
			string.Printf( "targeted_%d_%d", m_sLocalPlayerIndex, m_sVehicleIndexOverride );
			C_UiTimelineMessage msg( string.Chars(), false );
			gButtons.BroadcastMessageNow( 0xffffffff, &msg );
		}
	}
}

void UITargeting::Render()
{
	if ( !m_Activated )
		return;

	// Showing message text
	if( m_fMessageTimer > 0.0f )
		return;

#if 0
	bool bTargeted = false;
	
	for ( S32 i = 0; i < m_SegmentActive.Size(); i++ )
		bTargeted |= m_SegmentActive[ i ];

	m_PrevTargetted = bTargeted;

	// PR - don't need to render anything for you anymore
	if ( m_sVehicleIndexOverride != -1 )
	{
		// auto-transform
		C_UIAutoTransform transform( this );

		C_GadgetBase::Render();

		C_UISpriteColours colTargeted;
		colTargeted.Set( m_ColourTargeted, m_AlphaInherit, m_IntensityInherit );

		C_UISpriteColours colTargetedIntensity;
		colTargetedIntensity.Set( m_ColourTargeted, m_AlphaInherit, m_IntensityInherit + m_fTargetedIntensityAdd );

		C_UISpriteColours colTargetedSegment;
		colTargetedSegment.Lerp( colTargeted, colTargetedIntensity, m_fColourLerp );

		static F32 fRadiusMult = 1.0f;
		static F32 fScale = 14.0f;

		if ( ( m_sVehicleIndexOverride != -1 ) && !bTargeted )
			return;	// don't show empty for AI

		for ( S32 i = 0; i < m_SegmentActive.Size(); i++ )
		{
			Maths::HVec3 pos( Maths::HVec3( 0.0f, ( m_Size.y / 2.0f ) * fRadiusMult, 0.0f ) );
			F32 fAngle = ( Maths::Pi2 / m_SegmentActive.Size() ) * i;
			pos = Maths::HMat33( Maths::RotZ( fAngle ) ) * pos;

			F32 fSizeX = m_Size.x * fScale;
			F32 fSizeY = m_Size.y * fScale;

			if ( m_SegmentActive[ i ] )
			{
				fSizeX *= ( m_fScaleMult + 1.0f );
				fSizeY *= ( m_fScaleMult + 1.0f );
			}

	//		C_UISpriteColours& col = bTargeted ? ( m_SegmentActive[ i ] ? colTargetedSegment : colTargeted ) : m_SpriteColours;
			C_UISpriteColours& col = m_PlayerInRange[ i ] ? colTargeted : m_SpriteColours;
			if( col[ 0 ].A() == 0 )
				continue;

			gSimple2D.AddSprite( m_Layer, m_Position + pos, fSizeX, fSizeY, 
				col, 
				m_Panel, E_SpriteType_Sprite, m_RenderState );
			gSimple2D.AddJustification( E_Justify_MiddleCentre );
			gSimple2D.AddRotation( 0.0f, 0.0f, Maths::RadToDeg( -fAngle ) );
			gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
		}

		if ( bTargeted )
		{
			bool bShowText = ( m_sVehicleIndexOverride != -1 );

			// No longer relevant - SB
			//if ( !bShowText )
			//{
			//	const Game::Actors::C_VehicleLogicActor* pLogicActor = GetVehicleLogicActor( GetPlayerVehicleIndex( m_sLocalPlayerIndex ) );
			//	bShowText = ( pLogicActor && ( pLogicActor->GetFanPower() >= 1.0f ) );
			//}

			if ( bShowText )
			{
				static F32 fTextMult = 0.9f;

				C_UISpriteColours colours( 0xffffffff );
				colours.Set( colours, m_AlphaInherit, m_IntensityInherit );

				gFont.DrawText( m_Layer, m_Position, E_Justify_MiddleCentre, 
					( m_sVehicleIndexOverride == -1 ) ? L"~~{X}~~" : L"~~{-A-}~~", 
					colours, m_Size.y * fTextMult, C_FontStyle::Instance(), 0.0f, m_Panel );
			}
		}
	}
#endif

	ResetTargets();	// these are only valid for one frame
}

void UITargeting::ResetTargets()
{
	for ( S32 i = 0; i < m_PlayerInRange.Size(); i++ )
		m_PlayerInRange[ i ] = false;

	for ( S32 i = 0; i < m_VehicleInRange.Size(); i++ )
		m_VehicleInRange[ i ] = false;

	for ( S32 i = 0; i < m_SegmentActive.Size(); i++ )
		m_SegmentActive[ i ] = false;

	m_bPlayerInRangeLocked = false;
}

}; // namespace UI
