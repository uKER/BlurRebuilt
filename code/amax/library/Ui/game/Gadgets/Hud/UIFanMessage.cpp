#include "stdafx.h"
#include <Game/Game.h>	// [Blur2PC]

#include "UIFanMessage.h"
#include "UiHelpers.h"
#include "UIPerks.h" // C_UIPerkInfo
#include "UiXtConfig.h" // C_UIPerkInfo

#include <Ui/Game/Gadgets/ButtonMessages.h>
#include <ui/Game/Gadgets/UIStaticText.h>
#include <ui/Game/Gadgets/UIStaticGraphic.h>
#include <ui/Graphics/SimpleTimer.h>
#include <Ui/UiInterface.h>
#include <Ui/SoundEffects.h>

#include "Game/GameActors/Components/Messages/VehicleLogicMessages.h"
#include "GameLogic/ActorStudioActors/ResourceTypes.h"
#include "RaceBook/Profile/RaceBookProfile.h"
#include "RaceBook/RaceBookManager.h"

#include <Game/GameActors/Crowds/CrowdGameActor.h>

#include "Ui/UiMessages.h"
#include "Ui/HudDataFinder.h"

XT_TYPE( UI::UIFanMessage, "UIFanMessage" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_fFanGapX, "fan_gap_x" )
XT_END_TYPE

namespace UI
{

// ------------
// UIFanMessage
// ------------
void UIFanMessage::Init()
{
	C_GadgetBase::Init();

	m_fFanGapX = 0.0f;
	
	m_sLocalPlayerIndex = -1;
	m_VehicleIndex = -1;

	m_FanBranch = NULL;
	m_FanDemandBranch = NULL;

	m_pFanMessageRoot = NULL;
	m_pFanMessage = NULL;
	m_pFanMessageName = NULL;
	m_pFanMessageNum = NULL;
	m_pFanBackingSingle = NULL;
	m_pFanBackingDouble = NULL;
	m_pFanBackingSingleBlob = NULL;
	m_pFanBackingDoubleBlob = NULL;

	m_pFanBonusMessageRoot = NULL;
	m_pFanBonusMessage = NULL;
	m_pFanBonusMessageNum = NULL;
	m_pFanBonusBackingSingleBlob = NULL;

	m_pFdIcon = NULL;
	m_pFdName = NULL;
	m_pFdAmount = NULL;
	m_pFdBlob = NULL;
	m_pFdWinIcon = NULL;
	m_pFdWinName = NULL;
	m_pFdWinAmount = NULL;
	m_pFdWinBlob = NULL;
	m_pFdFailIcon = NULL;
	m_pFdFailName = NULL;
	m_pFdFailBlob = NULL;

	m_fFdMaxNumLength = 0.0f;
	m_sFdFans = 0;

	m_eRaceMode = GetLevelState().mRaceMode;
	m_bSplitscreen = ( GetLevelState().mNumViewports > 1 );
	
	// messages
	AddSubscriber( E_UiGameMsg_SetLocalPlayerIndex );
	//AddSubscriber( E_VehicleLogicMsg_FiredPerk );
	//AddSubscriber( E_VehicleLogicMsg_HitByPerk );
	AddSubscriber( E_UiGameMsg_ScoringEventChain );
	AddSubscriber( E_UiGameMsg_ScoringEventPerk );
	AddSubscriber( E_UiGameMsg_ScoringEventFans );
	AddSubscriber( E_UiGameMsg_SetLocalPlayerIndex );
	AddSubscriber( E_UiGameMsg_FanDemand );
	AddSubscriber( E_UiGameMsg_ShowFanScoring );
	
	Game::C_LevelState& levelState = GetLevelState();
	m_eRaceMode = levelState.mRaceMode;
}

void UIFanMessage::PostInit()
{
	C_GadgetBase::PostInit();

	FindChildren( &m_Children, true );

	C_GadgetBase* p  = NULL;

	// branches
	p = ArrayFindByName( m_Children, "bra_FanRoot" );
	if( p && p->GetType() == E_UIButtonType_RenderBranch )
		m_FanBranch = (UIRenderBranch*)p;

	p = ArrayFindByName( m_Children, "bra_FanDemandRoot" );
	if( p && p->GetType() == E_UIButtonType_RenderBranch )
		m_FanDemandBranch = (UIRenderBranch*)p;

	// fan scoring
	p = ArrayFindByName( m_Children, "dmy_FanDummy" );
	if( p && p->GetType() == E_UIButtonType_RenderBranch )
		m_pFanMessageRoot = (UIRenderBranch*)p;

	p = ArrayFindByName( m_Children, "txt_FanMessage" );
	if( p && p->GetType() == E_UIButtonType_StaticText )
	{
		m_pFanMessage = (UIStaticText*)p;
		m_pFanMessage->SetDirectText( m_Message.Chars() );
	}

	p = ArrayFindByName( m_Children, "txt_FanMessageName" );
	if( p && p->GetType() == E_UIButtonType_StaticText )
	{
		m_pFanMessageName = (UIStaticText*)p;
		m_pFanMessageName->SetDirectText( m_MessageName.Chars() );
	}

	p = ArrayFindByName( m_Children, "txt_FansAdded" );
	if( p && p->GetType() == E_UIButtonType_StaticText )
	{
		m_pFanMessageNum = (UIStaticText*)p;
		m_pFanMessageNum->SetDirectText( m_FansAdded.Chars() );
	}

	p = ArrayFindByName( m_Children, "bra_FanBlobSingle" );
	if( p && p->GetType() == E_UIButtonType_RenderBranch )
		m_pFanBackingSingle = (UIRenderBranch*)p;

	p = ArrayFindByName( m_Children, "bra_FanBlobDouble" );
	if( p && p->GetType() == E_UIButtonType_RenderBranch )
		m_pFanBackingDouble = (UIRenderBranch*)p;

	p = ArrayFindByName( m_Children, "gfx_FanBlobSingleC" );
	if( p && p->GetType() == E_UIButtonType_StaticGraphic )
		m_pFanBackingSingleBlob = (UIStaticGraphic*)p;

	p = ArrayFindByName( m_Children, "gfx_FanBlobDoubleC" );
	if( p && p->GetType() == E_UIButtonType_StaticGraphic )
		m_pFanBackingDoubleBlob = (UIStaticGraphic*)p;

	// bonus scoring
	p = ArrayFindByName( m_Children, "dmy_FanBonus" );
	if( p && p->GetType() == E_UIButtonType_RenderBranch )
		m_pFanBonusMessageRoot = (UIRenderBranch*)p;

	p = ArrayFindByName( m_Children, "txt_Bonus" );
	if( p && p->GetType() == E_UIButtonType_StaticText )
	{
		m_pFanBonusMessage = (UIStaticText*)p;
		m_pFanBonusMessage->SetDirectText( m_BonusString.Chars() );
	}

	p = ArrayFindByName( m_Children, "txt_BonusFansAdded" );
	if( p && p->GetType() == E_UIButtonType_StaticText )
	{
		m_pFanBonusMessageNum = (UIStaticText*)p;
		m_pFanBonusMessageNum->SetDirectText( m_BonusFansAdded.Chars() );
	}

	p = ArrayFindByName( m_Children, "gfx_FanBlobBonusC" );
	if( p && p->GetType() == E_UIButtonType_StaticGraphic )
		m_pFanBonusBackingSingleBlob = (UIStaticGraphic*)p;

	// fan demands
	p = ArrayFindByName( m_Children, "gfx_FdIcon" );
	if ( p && p->GetType() == E_UIButtonType_StaticGraphic )
		m_pFdIcon = (UIStaticGraphic*)p;

	p = ArrayFindByName( m_Children, "txt_FdName" );
	if ( p && p->GetType() == E_UIButtonType_StaticText )
	{
		m_pFdName = (UIStaticText*)p;
		m_pFdName->SetDirectText( m_FdName.Chars() );
	}

	p = ArrayFindByName( m_Children, "txt_FdAmount" );
	if ( p && p->GetType() == E_UIButtonType_StaticText )
	{
		m_pFdAmount = (UIStaticText*)p;
		m_pFdAmount->SetDirectText( m_FdAmount.Chars() );

		// calc max string size for fan run
		m_FdAmount.Format( L"8888" );
		m_fFdMaxNumLength = m_pFdAmount->GetStringLength();
	}

	p = ArrayFindByName( m_Children, "gfx_FanBlobFdC" );
	if ( p && p->GetType() == E_UIButtonType_StaticGraphic )
		m_pFdBlob = (UIStaticGraphic*)p;

	// fan demand win
	p = ArrayFindByName( m_Children, "gfx_FdIconWin" );
	if ( p && p->GetType() == E_UIButtonType_StaticGraphic )
		m_pFdWinIcon = (UIStaticGraphic*)p;

	p = ArrayFindByName( m_Children, "txt_FdWin" );
	if ( p && p->GetType() == E_UIButtonType_StaticText )
		m_pFdWinName = (UIStaticText*)p;

	p = ArrayFindByName( m_Children, "txt_FdAmountWin" );
	if ( p && p->GetType() == E_UIButtonType_StaticText )
	{
		m_pFdWinAmount = (UIStaticText*)p;
		m_pFdWinAmount->SetDirectText( m_FdAmountWin.Chars() );
	}

	p = ArrayFindByName( m_Children, "gfx_FanBlobFdWinC" );
	if ( p && p->GetType() == E_UIButtonType_StaticGraphic )
		m_pFdWinBlob = (UIStaticGraphic*)p;

	// fan demand fail
	p = ArrayFindByName( m_Children, "gfx_FdIconFail" );
	if ( p && p->GetType() == E_UIButtonType_StaticGraphic )
		m_pFdFailIcon = (UIStaticGraphic*)p;

	p = ArrayFindByName( m_Children, "txt_FdFail" );
	if ( p && p->GetType() == E_UIButtonType_StaticText )
		m_pFdFailName = (UIStaticText*)p;

	p = ArrayFindByName( m_Children, "gfx_FanBlobFdFailC" );
	if ( p && p->GetType() == E_UIButtonType_StaticGraphic )
		m_pFdFailBlob = (UIStaticGraphic*)p;
}

bool UIFanMessage::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated )
		return C_GadgetBase::MessageHandler( pMessage );

	switch ( pMessage->Type() )
	{
		case E_UiGameMsg_SetLocalPlayerIndex:
			if ( m_sLocalPlayerIndex == -1 )
			{
				m_sLocalPlayerIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
				m_VehicleIndex = GetPlayerVehicleIndex( m_sLocalPlayerIndex );
			}
			break;

		case E_UiGameMsg_ScoringEventChain:
			ProcessBonus( Messaging::MessageCast<C_UiScoringEventChain>( pMessage ) );
			return true;

		case E_UiGameMsg_ScoringEventPerk:
			ProcessHitByPerk( Messaging::MessageCast<C_UiScoringEventPerk>( pMessage ) );
			return true;

		case E_UiGameMsg_FanDemand:
			ProcessFanDemand( Messaging::MessageCast<C_UiFanDemand>( pMessage ) );
			return true;

		case E_UiGameMsg_ShowFanScoring:
			{
				const C_UiShowFanScoring* pShowFanMsg = Messaging::MessageCast<C_UiShowFanScoring>( pMessage );
				if( pShowFanMsg->m_Demands )
				{
					if( m_FanDemandBranch )
						m_FanDemandBranch->Activate( pShowFanMsg->m_Show, false );
				}
				else
				{
					if( m_FanBranch )
						m_FanBranch->Activate( pShowFanMsg->m_Show, false );
				}
			}
			break;

		/*
		case E_VehicleLogicMsg_HitByPerk:
		case E_VehicleLogicMsg_FiredPerk:
			CheckPerkEffects( pMessage );
			break;
		*/

		default:
			return C_GadgetBase::MessageHandler( pMessage );
	}

	return true;
}

void UIFanMessage::ProcessBonus( const C_UiScoringEventChain* pMessage )
{
	if ( ( pMessage->m_VehicleIndex != m_VehicleIndex ) || ( pMessage->m_BonusString.Length() == 0 ) )
		return;

	if ( !m_pFanBonusMessageRoot || !m_pFanBonusMessage || !m_pFanBonusMessageNum || !m_pFanBonusBackingSingleBlob )
		return;

	CallChildLine( "fan_bonus", true, 0 );

	m_BonusString = pMessage->m_BonusString;
	// Only show '+ xxx' if actually something to award
	if(pMessage->m_BonusFans != 0 )
	{
		m_BonusFansAdded.Format( L"+%d", pMessage->m_BonusFans );
	
		::Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
		if( levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction )
		{
			if( pMessage->m_BonusFans != 1 )
			{
				m_BonusFansAdded.Format( UI::C_TextStringID("MP_MOTORMASH_POINTS").String(), pMessage->m_BonusFans );
			}
			else // Points mean prizes, but only if pluralised
			{
				m_BonusFansAdded.Format( UI::C_TextStringID("MP_MOTORMASH_POINT").String(), pMessage->m_BonusFans );
			}
		}
	}
	else
	{
		m_BonusFansAdded.Clear(); 
	}


	UiInterface()->PlaySound( E_UiSoundEffect_MessageOtherActionIn );

	// centre text and size box
	F32 fLength = 0.0f;
	fLength += m_pFanBonusMessage->GetStringLength( true );
	fLength += m_pFanBonusMessageNum->m_Position.x;
	fLength += m_pFanBonusMessageNum->GetStringLength( true );

	F32 fOffset = -( fLength / 2.0f );

	m_pFanBonusMessageRoot->DirtyMatrix();
	
	m_pFanBonusMessage->m_Position.x = fOffset;
	m_pFanBonusBackingSingleBlob->m_Size.x = fLength + m_fFanGapX;
}

void UIFanMessage::ProcessHitByPerk( const C_UiScoringEventPerk* pMessage )
{
	if ( ( pMessage->m_VehicleIndex != m_VehicleIndex ) || ( pMessage->m_ScoreString.Length() == 0 ) )
		return;

	if ( !m_pFanMessageRoot || !m_pFanMessage || !m_pFanMessageName || !m_pFanMessageNum || !m_pFanBackingSingle || !m_pFanBackingDouble || !m_pFanBackingSingleBlob || !m_pFanBackingDoubleBlob )
		return;

	bool bHasName = ( pMessage->m_ScoreStringName.Length() != 0 );

	CallChildLine( "fan_message", true, 0 );

	m_Message = pMessage->m_ScoreString;
	m_MessageName = pMessage->m_ScoreStringName;

	UiInterface()->PlaySound( E_UiSoundEffect_MessageOtherActionIn );

	if(pMessage->m_FansNumber != 0 )
	{
		m_FansAdded.Format( L"+%d", pMessage->m_FansNumber );

		// Append 'Pts' if we need to
		::Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
		if( levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction )
		{
			if( pMessage->m_FansNumber >= 1 )
			{
				m_FansAdded.Format( UI::C_TextStringID("MP_MOTORMASH_POINTS").String(), pMessage->m_FansNumber );
			}
			else if(pMessage->m_FansNumber < 0 )
			{
				m_FansAdded.Format( UI::C_TextStringID("MP_MOTORMASH_SUICIDE_POINT").String(), pMessage->m_FansNumber );
			}
			else
			{
			
				// Points mean prizes, but only if pluralised
				m_FansAdded.Format( UI::C_TextStringID("MP_MOTORMASH_POINT").String(), pMessage->m_FansNumber );
			}
			
		}
	}
	else
	{
		m_FansAdded.Clear();
	}

	// centre text and size box
	F32 fLength = 0.0f;
	F32 fLengthMessage = 0.0f;
	F32 fLength2 = 0.0f;

	fLengthMessage = m_pFanMessage->GetStringLength( true );

	fLength += fLengthMessage;
	fLength += m_pFanMessageNum->m_Position.x;
	fLength += m_pFanMessageNum->GetStringLength();

	F32 fOffset = -( fLength / 2.0f );

	fLength2 += m_pFanMessageName->GetStringLength();

	if ( bHasName && ( fLength2 > fLength ) )
		fLength = fLength2;

	m_pFanMessageRoot->DirtyMatrix();

	m_pFanMessage->m_Position.x = fOffset;

	m_pFanBackingSingle->Activate( !bHasName, false );
	m_pFanBackingSingleBlob->m_Size.x = fLength + m_fFanGapX;

	m_pFanBackingDouble->Activate( bHasName, false );
	m_pFanBackingDoubleBlob->m_Size.x = fLength + m_fFanGapX;
}

void UIFanMessage::ProcessFanDemand( const C_UiFanDemand* pMessage )
{
	if ( pMessage->m_VehicleIndex != m_VehicleIndex )
		return;

	if ( !m_FanDemandBranch || !m_FanBranch || !m_pFdIcon || !m_pFdName || !m_pFdAmount || !m_pFdBlob || !m_pFdWinIcon || !m_pFdWinName || !m_pFdWinAmount || !m_pFdWinBlob || !m_pFdFailIcon || !m_pFdFailName || !m_pFdFailBlob )
		return;

	m_FdName = pMessage->m_String.Chars();

	if ( !m_FanDemandBranch->IsActivated() || ( pMessage->m_JustUpdate == false ) )
	{
		// switch branches
		m_FanDemandBranch->Activate( true, false );
		m_FanBranch->Activate( false, false );

		// reset
		static char* s_pResetMessagesFD[ 4 ] = { "fan_demand_win", "fan_demand_win_on", "fan_demand_fail", "fan_demand_fail_on", };

		for ( S32 i = 0; i < 4; i++ )
		{
			C_UiTimelineMessage msg_reset( s_pResetMessagesFD[ i ], false, 0.0f );
			gButtons.BroadcastMessageNow( 0xffffffff, &msg_reset );
		}

		// start new
		C_UiTimelineMessage msg_start( "fan_demand_start", true, 0.0f );
		gButtons.BroadcastMessageNow( 0xffffffff, &msg_start );

		m_sFdFans = 0;

		// shift info and size box
		F32 fLength = 0.0f;
		fLength += m_pFdName->GetStringLength( true );
		fLength += m_pFdIcon->m_Position.x;
		fLength += m_pFdIcon->m_Size.x;
		fLength += m_pFdAmount->m_Position.x;
		fLength += m_fFdMaxNumLength;

		m_FanDemandBranch->DirtyMatrix();

		m_pFdName->m_Position.x = -( fLength / 2.0f );

		m_pFdBlob->m_Size.x = fLength + m_fFanGapX;
	}

	S32 sFans = Maths::Max( (S32)pMessage->m_FanAmount, 0 );
	m_FdAmount.Format( L"%d", sFans );
	m_FdAmountWin.Format( L"+%d", sFans );

	if ( ( m_sFdFans != 0 ) && ( sFans > m_sFdFans ) )
	{
		m_pFdAmount->DoCalcStringSize( NULL, true );
		_UI_MESSAGE( UI::E_UiGameMsg_SpFanDemandFansAdded );
	}

	m_sFdFans = sFans;

	if ( pMessage->m_AmountLeft == 0 )
	{
		// failed
		C_UiTimelineMessage msg_fail( "fan_demand_fail", true, 0.0f );
		gButtons.BroadcastMessageNow( 0xffffffff, &msg_fail );

		if ( Game::C_CrowdGameManager::Instance() )
			Game::C_CrowdGameManager::Instance()->OnFanDemand( false );

		m_pFdFailName->ChangeText( C_TextStringID( pMessage->m_FanRun ? "SCR_NEW_AMAX_SCORING_FAN_RUN_FAILED" : "SCR_NEW_AMAX_SCORING_FAN_DEMAND_FAILED" ) );

		// shift info and size box
		F32 fLength = 0.0f;
		fLength += m_pFdFailIcon->m_Size.x;
		fLength += m_pFdFailName->m_Position.x;
		fLength += m_pFdFailName->GetStringLength();

		m_FanDemandBranch->DirtyMatrix();

		m_pFdFailIcon->m_Position.x = -( fLength / 2.0f );
		m_pFdFailBlob->m_Size.x = fLength + m_fFanGapX;
	}
	else
	if ( pMessage->m_AmountLeft == -1 )
	{
		// success
		C_UiTimelineMessage msg_win( "fan_demand_win", true, 0.0f );
		gButtons.BroadcastMessageNow( 0xffffffff, &msg_win );

		if ( Game::C_CrowdGameManager::Instance() )
			Game::C_CrowdGameManager::Instance()->OnFanDemand( true );

		m_pFdWinName->ChangeText( C_TextStringID( pMessage->m_FanRun ? "SCR_NEW_AMAX_SCORING_FAN_RUN_COMPLETE" : "SCR_NEW_AMAX_SCORING_FAN_DEMAND_COMPLETE" ) );

		// shift info and size box
		F32 fLength = 0.0f;
		fLength += m_pFdWinName->GetStringLength( true );
		fLength += m_pFdWinIcon->m_Position.x;
		fLength += m_pFdWinIcon->m_Size.x;
		fLength += m_pFdWinAmount->m_Position.x;
		fLength += m_pFdWinAmount->GetStringLength( true );

		m_FanDemandBranch->DirtyMatrix();

		m_pFdWinName->m_Position.x = -( fLength / 2.0f );
		m_pFdWinBlob->m_Size.x = fLength + m_fFanGapX;
	}
}

void UIFanMessage::CallChildLine( const char* name, const bool active, const F32 time, const bool reset )
{
	C_UiTimelineMessage msg( name, active, time, reset );

	for( S32 i = 0; i < m_Children.Size(); i++ )
		m_Children[ i ]->MessageHandler( &msg );
}

/*
void UIFanMessage::CheckPerkEffects( const Messaging::C_MessageBase* pMessage )
{
	if ( pMessage->Type() == E_VehicleLogicMsg_FiredPerk )
	{
		const C_VehicleLogicMsg_FiredPerkMsg* pMsg = Messaging::MessageCast< C_VehicleLogicMsg_FiredPerkMsg >( pMessage );

		if ( pMsg->mVehicleIndex != m_VehicleIndex )
			return;

		if ( pMsg->mPickupType == E_PICKUP_TYPE_NITROUS && !m_bSplitscreen )
			DoTimeLine( "perk_nitro" );
		else if ( pMsg->mPickupType == E_PICKUP_TYPE_SHIELD )
			DoTimeLine( "perk_shield" );
	}
	else if ( pMessage->Type() == E_VehicleLogicMsg_HitByPerk )
	{
		const C_VehicleLogicMsg_HitByPerkMsg* pMsg = Messaging::MessageCast< C_VehicleLogicMsg_HitByPerkMsg >( pMessage );

		if ( pMsg->mVehicleIndex != m_VehicleIndex )
			return;

		if ( pMsg->mPickupType == E_PICKUP_TYPE_SHOCK )
			DoTimeLine( "perk_shock" );
		else if ( pMsg->mPickupType == E_PICKUP_TYPE_BLINDFIRE && !m_bSplitscreen )
			DoTimeLine( "perk_blindfire" );
		else if ( pMsg->mPickupType == E_PICKUP_TYPE_SHUNT && !m_bSplitscreen )
			DoTimeLine( "perk_shunt" );
		else if ( pMsg->mPickupType == E_PICKUP_TYPE_BOMB && !m_bSplitscreen )
			DoTimeLine( "perk_mine" );
		else if ( pMsg->mPickupType == E_PICKUP_TYPE_BARGE && !m_bSplitscreen )
		{
			Game::Actors::C_VehicleLogicActor* pAI = GetVehicleLogicActor( pMsg->mFiringVehicleIndex );
			Game::Actors::C_VehicleLogicActor* pMe = GetVehicleLogicActor( m_VehicleIndex );

			if ( !pAI || !pMe )
				return;

			const Maths::HMat43& mAI = pAI->GetWorldMatrix();
			const Maths::HMat43& mMe = pMe->GetWorldMatrix();

			// which side is the AI on?
			Maths::C_Normal n( mMe.AX() );
			Maths::C_Plane plane( n, mMe.AW() );
			
			if ( (F32)( plane.Distance( mAI.AW() ) ) <= 0.0f )
				DoTimeLine( "perk_barge_left" );
			else
				DoTimeLine( "perk_barge_right" );
		}
	}
}

void UIFanMessage::DoTimeLine( const char* pName, const bool bActive, const F32 fTime )
{
	if ( !pName || !pName[ 0 ] )
		return;

	Util::C_String32 string;
	string.Printf( "%s_%d", pName, m_sLocalPlayerIndex );

	C_UiTimelineMessage msg( string.Chars(), bActive, fTime );
	gButtons.BroadcastMessageNow( 0xffffffff, &msg );
}
*/

}; // namespace UI
