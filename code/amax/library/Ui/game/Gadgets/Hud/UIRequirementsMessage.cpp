#include "UIRequirementsMessage.h"
#include <GameLogic/SinglePlayer/SinglePlayer.h>	// [Blur2PC]
#include "UiHelpers.h"

#include <ui/Game/UILocaliser.h>
#include <ui/Game/Gadgets/UIStaticText.h>
#include <ui/Game/Gadgets/UIStaticGraphic.h>
#include <Ui/Game/Gadgets/UiBox.h>
#include <Ui/Game/Gadgets/UiStaticShape.h>
#include <Ui/Game/Gadgets/ButtonMessages.h>

#include <ui/Graphics/SimpleTimer.h>

#include "Ui/HudDataFinder.h"

#include <Ui/UiInterface.h>
#include <Ui/SoundEffects.h>

XT_TYPE( UI::UIRequirementsMessage, "UIRequirementsMessage" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_fProgressMessageTime, "message_timer" )
XT_END_TYPE

namespace UI
{

// ---------------------
// UIRequirementsMessage
// ---------------------
void UIRequirementsMessage::Init()
{
	C_GadgetBase::Init();
	
	m_sLocalPlayerIndex = -1;
	m_VehicleIndex = -1;

	m_pBranch = NULL;
	m_pBossPic = NULL;
	m_pBossFrame = NULL;
	m_pStickerIcon = NULL;
	m_pChallengeIcon = NULL;
	m_pBar = NULL;
	m_pTick = NULL;
	m_pTitle = NULL;
	m_pStickerNum = NULL;
	m_pDesc = NULL;
	m_pDescComplete = NULL;
	m_pBackingTop = NULL;
	m_pBackingTop2 = NULL;
	m_pBackingMiddle = NULL;
	m_pBackingBottom = NULL;

	m_fProgressMessageTime = 3.5f;
	
	m_fDisplayTime = 0.f;
	m_fBarSizeX = 0.0f;

	m_fChallengeIconU = 0.0f;

	AddSubscriber( E_UiGameMsg_SetLocalPlayerIndex );
	AddSubscriber( E_UiMsg_FrameUpdate );
	AddSubscriber( E_UiGameMsg_ProgressMessage );
	AddSubscriber( E_UiGameMsg_BossDemandIncrement );
}

void UIRequirementsMessage::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	// resolve colours
	m_ColourNormal.SetByName( "Support_4" );
	m_ColourMultiplayer.SetByName( "Main_1" );
	m_ColourPowerup.SetByName( "Support_0" );
	m_ColourBoss.SetByName( "Support_2" );
	m_ColourTextSticker.SetByName( "Support_4" );
	m_ColourTextOther.SetByName( "Main_1" );
}

void UIRequirementsMessage::PostInit()
{
	C_GadgetBase::PostInit();

	ButtonArray m_Children;
	FindChildren( &m_Children, true );

	C_GadgetBase* p = NULL;

	p = ArrayFindByName( m_Children, "ProgressRB" );
	if( p && p->GetType() == E_UIButtonType_RenderBranch )
		m_pBranch = (UIRenderBranch*)p;

	p = ArrayFindByName( m_Children, "ProgressBossPic" );
	if( p && p->GetType() == E_UIButtonType_StaticBox )
		m_pBossPic = (UIStaticBox*)p;

	p = ArrayFindByName( m_Children, "ProgressBossFrame" );
	if( p && p->GetType() == E_UIButtonType_StaticBox )
		m_pBossFrame = (UIStaticBox*)p;

	p = ArrayFindByName( m_Children, "ProgressStickerIcon" );
	if( p && p->GetType() == E_UIButtonType_StaticGraphic )
		m_pStickerIcon = (UIStaticGraphic*)p;

	p = ArrayFindByName( m_Children, "ProgressChallengeIcon" );
	if( p && p->GetType() == E_UIButtonType_StaticGraphic )
	{
		m_pChallengeIcon = (UIStaticGraphic*)p;
		m_fChallengeIconU = m_pChallengeIcon->m_TextureParams[ 0 ].m_Position.u;
	}

	p = ArrayFindByName( m_Children, "ProgressGraphic" );
	if( p && p->GetType() == E_UIButtonType_StaticBox )
	{
		m_pBar = (UIStaticBox*)p;
		m_fBarSizeX = m_pBar->m_Size.x;
	}

	p = ArrayFindByName( m_Children, "ProgressAmount" );
	if( p && p->GetType() == E_UIButtonType_StaticText )
		((UIStaticText*)p)->SetDirectText( m_Amount );

	p = ArrayFindByName( m_Children, "ProgressTick" );
	if( p && p->GetType() == E_UIButtonType_StaticGraphic )
		m_pTick = (UIStaticGraphic*)p;

	p = ArrayFindByName( m_Children, "ProgressTitle" );
	if( p && p->GetType() == E_UIButtonType_StaticText )
	{
		m_pTitle = (UIStaticText*)p;
		m_pTitle->SetDirectText( m_Name );
	}

	p = ArrayFindByName( m_Children, "ProgressStickerNum" );
	if( p && p->GetType() == E_UIButtonType_StaticText )
	{
		m_pStickerNum = ((UIStaticText*)p);
		m_pStickerNum->SetDirectText( m_StickerNum );
	}

	p = ArrayFindByName( m_Children, "ProgressDesc" );
	if( p && p->GetType() == E_UIButtonType_StaticText )
	{
		m_pDesc = ((UIStaticText*)p);
		m_pDesc->SetDirectText( m_Desc );
	}

	p = ArrayFindByName( m_Children, "ProgressDescComplete" );
	if( p && p->GetType() == E_UIButtonType_StaticText )
	{
		m_pDescComplete = ((UIStaticText*)p);
		m_pDescComplete->SetDirectText( m_Desc );
	}

	p = ArrayFindByName( m_Children, "ProgressBackingTopR" );
	if( p && p->GetType() == E_UIButtonType_StaticGraphic )
		m_pBackingTop = (UIStaticGraphic*)p;

	p = ArrayFindByName( m_Children, "ProgressBackingTopR2" );
	if( p && p->GetType() == E_UIButtonType_StaticGraphic )
		m_pBackingTop2 = (UIStaticGraphic*)p;

	p = ArrayFindByName( m_Children, "ProgressBackingMiddleR" );
	if( p && p->GetType() == E_UIButtonType_StaticGraphic )
		m_pBackingMiddle = (UIStaticGraphic*)p;

	p = ArrayFindByName( m_Children, "ProgressBackingBottomR" );
	if( p && p->GetType() == E_UIButtonType_StaticGraphic )
		m_pBackingBottom = (UIStaticGraphic*)p;
}

bool UIRequirementsMessage::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated )
		return C_GadgetBase::MessageHandler( pMessage );

	switch ( pMessage->Type() )
	{
		case E_UiGameMsg_SetLocalPlayerIndex:
		{
			if ( m_sLocalPlayerIndex == -1 )
			{
				m_sLocalPlayerIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
				m_VehicleIndex = GetPlayerVehicleIndex( m_sLocalPlayerIndex );
			}
			break;
		}
		case E_UiGameMsg_ProgressMessage:
		{
			const C_UiProgressMessage* pProgressMessage = Messaging::MessageCast<C_UiProgressMessage>( pMessage );

			if ( pProgressMessage->mVehicleIndex == m_VehicleIndex )
				AddMessageToQueue( pProgressMessage );

			break;
		}
		case E_UiGameMsg_BossDemandIncrement:
		{
			S32 sIndexBD = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];

			SP_SaveGame* pSG = SP_GetCurrentSaveGame();

			SP_Event* pEvent = SP_GetEventById( pSG, GetLevelState().mSpEventId );

			if ( !pEvent || !pEvent->tier || !pEvent->tier->boss )
				break;

			SP_BossDemand* pBD = SP_GetBossDemand( pSG, pEvent->tier->boss, sIndexBD );

			if ( !pBD )
				break;

			ProgressMessage msg;

			if ( SP_DoesBossDemandUseFloatValue( pSG, pBD ) )
			{
				msg.m_fProgress = SP_GetBossDemandValueFloat( pSG, pBD );
				msg.m_fGoal = SP_GetBossDemandTargetFloat( pSG, pBD );
			}
			else
			{
				msg.m_fProgress = (F32)SP_GetBossDemandValueInt( pSG, pBD );
				msg.m_fGoal = (F32)SP_GetBossDemandTargetInt( pSG, pBD );
			}

			msg.m_Name = C_TextStringID( pEvent->tier->boss->name.id ).String();

			Btl::String32 desc;
			desc.Printf( "SPL_TIER_%dBD%d_TITLE", pEvent->tier->index, sIndexBD );
			msg.m_Description = C_UIDataFinder::Instance().GetVariableText( desc.Chars() );

			msg.m_bOneShot = SP_IsBossDemandComplete( pSG, pBD );
			msg.m_bBossDemand = true;

			AddMessageToQueue( msg );
		}

		default:
			return C_GadgetBase::MessageHandler( pMessage );
	}

	return true;
}

void UIRequirementsMessage::FrameUpdate()
{
	if ( !m_Activated )
		return;

	C_GadgetBase::FrameUpdate();

	ProcessMessageQueue();
}

void UIRequirementsMessage::AddMessageToQueue( const C_UiProgressMessage* pMessage )
{
	ProgressMessage msg( pMessage );

	if ( CheckAndUpdateForDupes( msg ) )
		return;

	if ( m_MessageQueue.Size() >= REQ_MSG_QUEUE_MAX )
	{
		LOGWARN( g_Log, "UIRequirementsMessage::AddMessageToQueue > Queue overflow, message ignored" );
		return;
	}

	m_MessageQueue.PushBack( msg );
}

void UIRequirementsMessage::AddMessageToQueue( const ProgressMessage& msg )
{
	if ( CheckAndUpdateForDupes( msg ) )
		return;

	if ( m_MessageQueue.Size() >= REQ_MSG_QUEUE_MAX )
	{
		LOGWARN( g_Log, "UIRequirementsMessage::AddMessageToQueue > Queue overflow, message ignored" );
		return;
	}

	m_MessageQueue.PushBack( msg );
}

void UIRequirementsMessage::ProcessMessageQueue()
{
	if ( !CheckGadgetPointers() )
		return;

	if ( m_fDisplayTime > 0.0f )
	{
		m_fDisplayTime -= theFETimer.FrameTime();

		if ( m_fDisplayTime <= 0.0f )
		{
			m_fDisplayTime = 0.0f;
			m_pBranch->Activate( false, false );
		}
	}
	else
	{
		m_pBranch->Activate( false, false );
	}

	// display next message
	if ( !m_MessageQueue.Empty() && ( m_fDisplayTime == 0.0f ) )
	{
		DisplayMessage( m_MessageQueue.Front() );
		m_MessageQueue.Erase( m_MessageQueue.Begin() );
	}
}

void UIRequirementsMessage::DisplayMessage( const ProgressMessage& msg, const bool bUpdate )
{
	if ( !CheckGadgetPointers() )
		return;

	if ( !bUpdate )
	{
		UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_HudProgressSlideActionIn );

		m_pBranch->Activate( true );
		m_pBranch->SendPrivateTimelineMessaage( "show", true, 0.0f );

		m_Name = msg.m_Name;
		m_Desc = msg.m_Description;

		m_StickerNum.Clear();

		m_pBossPic->Activate( msg.m_bBossDemand, false );
		m_pBossFrame->Activate( msg.m_bBossDemand, false );

		bool bIsSticker = ( msg.m_eStickerCategory != Game::BLUR_ACHIEVEMENT_CATEGORY_NONE );
		bool bIsChallenge = ( msg.m_eChallengePack != E_ChallengePack_None );

		m_pStickerIcon->Activate( bIsSticker, false );
		m_pChallengeIcon->Activate( bIsChallenge, false );

		if ( bIsSticker )
		{
			m_pTitle->m_SpriteColours = m_ColourTextSticker;

			/*
				 if ( msg.m_eStickerCategory == Game::BLUR_ACHIEVEMENT_CATEGORY_NORMAL )		m_pStickerIcon->m_SpriteColours = m_ColourNormal;
			else if ( msg.m_eStickerCategory == Game::BLUR_ACHIEVEMENT_CATEGORY_MULTIPLAYER )	m_pStickerIcon->m_SpriteColours = m_ColourMultiplayer;
			else if ( msg.m_eStickerCategory == Game::BLUR_ACHIEVEMENT_CATEGORY_POWERUP )		m_pStickerIcon->m_SpriteColours = m_ColourPowerup;
			else if ( msg.m_eStickerCategory == Game::BLUR_ACHIEVEMENT_CATEGORY_BOSS )			m_pStickerIcon->m_SpriteColours = m_ColourBoss;
			*/

			if ( msg.m_sStickerPart == 4 )
				m_StickerNum.Format( L"IV" );
			else
			{
				for ( S32 i = 0; i < msg.m_sStickerPart; i++ )
					m_StickerNum.Add( L"I" );
			}
		}
		else
			m_pTitle->m_SpriteColours = m_ColourTextOther;
			

		if ( bIsChallenge )
			m_pChallengeIcon->m_TextureParams[ 0 ].m_Position.u = m_fChallengeIconU + ( m_pChallengeIcon->m_TextureParams[ 0 ].m_Size.u * msg.m_eChallengePack );

		m_fDisplayTime = m_fProgressMessageTime;

		// resize background box
		F32 fLength1 = m_pTitle->GetStringLength( true );
		fLength1 += m_pStickerNum->GetStringLength();

		F32 fLength2 = m_pDesc->GetStringLength();

		F32 fLength = Maths::Max( fLength1, fLength2 );

		static F32 s_fBarLength = 15.5f;
		fLength = Maths::Max( fLength, s_fBarLength );	// bleh

		m_pBackingTop->m_Size.x = fLength;
		m_pBackingTop2->m_Size.x = fLength;
		m_pBackingMiddle->m_Size.x = fLength;
		m_pBackingBottom->m_Size.x = fLength;

		m_pBranch->DirtyMatrix();
	}

	m_pBar->m_Size.x = Maths::Clamp01( msg.m_fProgress / msg.m_fGoal ) * m_fBarSizeX;

	bool bComplete = ( ( msg.m_fProgress >= msg.m_fGoal ) || msg.m_bOneShot );

	if ( bComplete )
		m_Amount.Format( UI::C_TextStringID( "ACH_UI_COMPLETE" ).String() );
	else
	{
		C_WideString32 str1 = C_UILocaliser::Instance().ConvertNumberToCommas( (S32)msg.m_fProgress );
		C_WideString32 str2 = C_UILocaliser::Instance().ConvertNumberToCommas( (S32)msg.m_fGoal );

		m_Amount.Format( UI::C_TextStringID( "ACH_HUD_MSG_PROGRESS" ).String(), str1.Chars(), str2.Chars() );
	}

	m_pTick->Activate( bComplete, false );
	m_pDesc->Activate( !bComplete, false );
	m_pDescComplete->Activate( bComplete, false );

	// store currently displayed
	m_MessageDisplayed = msg;
}

bool UIRequirementsMessage::CheckGadgetPointers()
{
	return ( m_pBranch && m_pBossPic && m_pBossFrame && m_pStickerIcon && m_pChallengeIcon && m_pBar && m_pTick && m_pTitle && m_pDesc && m_pDescComplete && m_pBackingTop && m_pBackingTop2 && m_pBackingMiddle && m_pBackingBottom );
}

bool UIRequirementsMessage::CheckAndUpdateForDupes( const ProgressMessage& msg )
{
	if ( m_fDisplayTime == 0.0f )
		return false;	// nothing currently active

	if ( ( m_MessageDisplayed.m_Name == msg.m_Name ) && ( m_MessageDisplayed.m_Description == msg.m_Description ) )
	{
		// update the currently displayed
		DisplayMessage( msg, true );
		return true;
	}

	// check the list for dupes
	for ( S32 i = 0; i < m_MessageQueue.Size(); i++ )
	{
		if ( ( m_MessageQueue[ i ].m_Name == msg.m_Name ) && ( m_MessageQueue[ i ].m_Description == msg.m_Description ) )
		{
			// update with new message
			m_MessageQueue[ i ] = msg;
			return true;
		}
	}

	return false;
}

}; // namespace UI
