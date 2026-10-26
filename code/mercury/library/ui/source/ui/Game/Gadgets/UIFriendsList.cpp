#include "stdafx.h"

#include <Gamelogic\SocialNetworking\FriendsManager.h>

#include "ui/Game/Gadgets/UIFriendsList.h"
#include <Ui/UiInterface.h>	// [Blur2PC] UI::UiInterface()

#include "ui/Game/UILua.h"
#include "ui/Game/Gadgets/ButtonMessages.h"
#include "ui/Game/Profiles/ProfileInterface.h"
#include "ui/Graphics/SimpleTimer.h"
#include "ui/Graphics/Font/Font.h"
#include "ui/Graphics/Font/FontStyle.h"

#include <lua_shared/LuaHelper.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

#ifndef CFG_GOLD
Debug::Bool g_UiFriendsFakeSome( "UI.Profiles.FakeSomeFriends", false );
#endif

XT_TYPE( UI::UIFriendsList, "UIFriendsList" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_FriendsVisible, "friends_visible" )
	XT_FIELD( m_RowHeight,		"row_height" )
	XT_FIELD( m_StatusSpacing,	"status_spacing" )
	XT_FIELD( m_IconPadding,	"icon_padding" )

	XT_FIELD( m_xtColourFriendCount, "colour_friend_count" )
	XT_FIELD( m_xtColourSelected, "colour_selected" )
	XT_FIELD( m_xtColourUnselected, "colour_unselected" )
	XT_FIELD( m_xtColourOnline, "colour_online" )
	XT_FIELD( m_xtColourOnlineDifferentGame, "colour_online_different_game" )
	XT_FIELD( m_xtColourOffline, "colour_offline" )
	XT_FIELD( m_xtColourPending, "colour_pending" )
XT_END_TYPE

// - depends on the following Sorcery strings
/// PRO_FRIENDS_WORKING	-> "Working - Please Wait ..."
/// PRO_FRIENDS_NO_FRIENDS -> "No Friends - lonely!"
/// PRO_FRIENDS_TITLE -> "Friends List"
/// PRO_FRIENDS_INF_VIEW_GAMERCARD_A > "~~{A}~~ View GamerCard"
/// PRO_FRIENDS_INF_SEND_INVITE_X -> "~~{X}~~ Send Invite"
/// PRO_FRIENDS_INF_JOIN_SESSION_Y -> "~~{Y}~~ Join Session In Progress"
/// PRO_FRIENDS_STATUS_OFFLINE -> "Offline"
/// PRO_FRIENDS_STATUS_ONLINE_NOT_THIS -> "Online But Playing Another Game"
/// PRO_FRIENDS_STATUS_ONLINE -> "Online"
/// PRO_FRIENDS_STATUS_REQUEST_RECEIVED -> "Friend Request Received"
/// PRO_FRIENDS_STATUS_REQUEST_SENT -> "Friend Request Sent"
/// PRO_FRIENDS_ACCEPT_REQUEST_Y -> "~~{Y}~~ Accept Friend Request"
/// PRO_FRIENDS_DECLINE_REQUEST_X -> "~~{X}~~ Decline Friend Request"
/// PRO_FRIENDS_REVOKE_REQUEST_X -> "~~{X}~~ Revoke Friend Request"
/// PRO_FRIENDS_NUM_OF_NUM -> "Friend x/y"

namespace UI
{

static const F32 s_fAutoRefresh = 20.0f;
static const F32 s_fNextOpenDelay = 0.5f;

static F32 s_fMiddleGapMult = 1.0f;
static F32 s_fIconMult = 0.85f;
static F32 s_fIconGapMult = 0.25f;
static F32 s_fInviteMult = 0.75f;

// -----------
// C_UIFriends
// -----------
C_UIFriends C_UIFriends::m_sInstance;

void C_UIFriends::Initialise( const S32 sContext, const S32 sActivateButton, const char* pScriptName )
{
	m_sContext = sContext;
	m_sActivateButton = sActivateButton;
	m_sPadNo = -1;

	m_fAutoRefreshTimer = 0.0f;
	m_bAutoRefresh = false;

	m_fNextOpenDelayTimer = 0.0f;

	STRCPY( m_pScriptName, m_uMaxScriptNameLength, pScriptName );

	// set magic button message
	SetSubscriberSystem( &gButtons.GetSubscriberSystem() );
		AddSubscriber( m_sActivateButton );
}

void C_UIFriends::Cancel()
{
	if ( !m_bDisplayed )
		return;

	m_bCancel = true;
}

bool C_UIFriends::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( m_bActive && !m_bDisplayed && ( pMessage->Type() == m_sActivateButton ) && ( m_fNextOpenDelayTimer >= s_fNextOpenDelay ) )
	{
		m_sPadNo = ((C_UiMessagePadData*)pMessage)->pad;

		// store current context
		m_sRestoreContext = C_ScreenControlManager::Instance().Context();

		// magic button has been pressed - activate
		C_ScreenControlManager::Instance().ResetContext( true, m_sContext );

		C_ScreenControlManager::Instance().Get( m_sContext ).SetNextScreen( m_pScriptName );
		C_ScreenControlManager::Instance().Get( m_sContext ).Init();

		m_bDisplayed = true;
	}

	return true;
}

void C_UIFriends::StartEnumeration()
{
	// if we're currently working - do nothing
	if ( Working() )
		return;

	Enumerator().Initialise();
	EnumeratorSent().Initialise();
	EnumeratorReceived().Initialise();

	m_uNumFriends = 0;
	
	SysProfile::FullProfile& primaryProfile = C_ProfileInterface::Instance().GetPadProfile( m_sPadNo );

	// start all the enumerations
	m_bReading  = Enumerator().StartEnumeration( primaryProfile, /*quickEnumeration*/ false );
	m_bReading |= EnumeratorSent().StartEnumeration( primaryProfile, NetPlay::C_PendingFriendsEnumerator::eModeInviteSent );
	m_bReading |= EnumeratorReceived().StartEnumeration( primaryProfile, NetPlay::C_PendingFriendsEnumerator::eModeInviteReceived );
}

void C_UIFriends::StartAnswerFriendRequest( const U32 uIndex, const bool bAccept )
{
	if ( Working() || !IsPendingFriendReceived( uIndex ) )
		return;

	m_bWorking = EnumeratorReceived().StartAnswerFriendRequest( m_PendingFriendsReceived[ AdjustIndexPendingReceived( uIndex ) ], bAccept );
}

void C_UIFriends::StartRevokeFriendRequest( const U32 uIndex )
{
	if ( Working() || !IsPendingFriendSent( uIndex ) )
		return;

	m_bWorking = EnumeratorSent().StartRevokeFriendRequest( m_PendingFriendsSent[ AdjustIndexPendingSent( uIndex ) ] );
}

void C_UIFriends::Update()
{
	F32 fFrameTime = theFETimer.FrameTime();

	if ( m_bCancel )
	{
		C_ScreenControlManager::Instance().Get( m_sContext ).End();

		C_ScreenControlManager::Instance().ResetContext( true, m_sRestoreContext );

		m_bDisplayed = false;
		m_bCancel = false;

		m_fNextOpenDelayTimer = 0.0f;
	}

	m_fNextOpenDelayTimer += fFrameTime;

	if ( m_bAutoRefresh && m_bDisplayed )
	{
		m_fAutoRefreshTimer += fFrameTime;

		if ( m_fAutoRefreshTimer >= s_fAutoRefresh )
		{
			StartEnumeration();
			m_fAutoRefreshTimer = 0.0f;
		}
	}

	if ( !m_bReading )
	{
		if ( m_bError )
			;	// TODO - handle errors
		else
		{
			int num_friends=m_uNumFriends;

			m_uNumFriends = Enumerator().ExtractFriends( m_Friends, MAX_FRIENDS, /*fullFriendsOnly*/ true );
			
			m_uNumPendingFriendsSent = EnumeratorSent().ExtractPendingFriends( m_PendingFriendsSent, MAX_FRIENDS );
			m_uNumFriends += m_uNumPendingFriendsSent;
			m_uNumPendingFriendsReceived = EnumeratorReceived().ExtractPendingFriends( m_PendingFriendsReceived, MAX_FRIENDS );
			m_uNumFriends += m_uNumPendingFriendsReceived;

			if(num_friends!=m_uNumFriends)
			{
				for( int a=0; a < m_uNumFriends; a++ )
				{
					Game::C_FriendsManager::Instance().AddFriend( m_Friends[ a ].m_Profile );
				}
			}

		#ifndef CFG_GOLD
			if ( g_UiFriendsFakeSome )
			{
				S32 sNumNewFriends = 15;
				S32 sNumNewFriendsSent = 5;
				S32 sNumNewFriendsReceived = 10;
				m_uNumFriends += ( sNumNewFriends + sNumNewFriendsSent + sNumNewFriendsReceived );
				m_uNumPendingFriendsSent += sNumNewFriendsSent;
				m_uNumPendingFriendsReceived += sNumNewFriendsReceived;
			}
		#endif
		}
	}

	if ( !Working() )
		return;

	m_bError = false;
	bool bLocalError = false;

	// reading and working are mutually exclusive

	if ( m_bReading )
	{
		// check all of the enumators
		m_bReading  = !Enumerator().ContinueEnumeration( bLocalError );
		m_bError   |= bLocalError;
		m_bReading |= !EnumeratorSent().ContinueTask( bLocalError );
		m_bError   |= bLocalError;
		m_bReading |= !EnumeratorReceived().ContinueTask( bLocalError );
		m_bError   |= bLocalError;
	}

	if ( m_bWorking )
	{
		m_bWorking  = !EnumeratorSent().ContinueTask( bLocalError );
		m_bError    = bLocalError;
		m_bWorking |= !EnumeratorReceived().ContinueTask( bLocalError );
		m_bError   |= bLocalError;

		if ( !m_bWorking )
		{
			if ( m_bError )
				;	// TODO - handle errors
			else
			{
				// refresh the list
				StartEnumeration();
			}
		}
	}
}

bool C_UIFriends::IsPendingFriendSent( const U32 uIndex )
{
	// sent requests comes after receive requests
	return ( ( !IsPendingFriendReceived( uIndex ) ) && ( uIndex < ( m_uNumPendingFriendsReceived + m_uNumPendingFriendsSent ) ) );
}

bool C_UIFriends::IsPendingFriendReceived( const U32 uIndex )
{
	// received requests are at the top of the list
	return ( uIndex < m_uNumPendingFriendsReceived );
}

U32 C_UIFriends::AdjustIndexPendingSent( const U32 uIndex )
{
	// sent requests comes after receive requests
	return ( uIndex - m_uNumPendingFriendsReceived );
}

U32 C_UIFriends::AdjustIndexPendingReceived( const U32 uIndex )
{
	// received requests are at the top of the list
	return uIndex;
}

U32 C_UIFriends::AdjustIndex( const U32 uIndex )
{
	// accepted friends are displayed last
	return ( uIndex - m_uNumPendingFriendsReceived - m_uNumPendingFriendsSent );
}

SysProfile::BaseProfile& C_UIFriends::GetFriendProfile( const U32 uIndex )
{
	if ( IsPendingFriendSent( uIndex ) )
		return m_PendingFriendsSent[ AdjustIndexPendingSent( uIndex ) ].m_Profile;

	if ( IsPendingFriendReceived( uIndex ) )
		return m_PendingFriendsReceived[ AdjustIndexPendingReceived( uIndex ) ].m_Profile;

	return m_Friends[ AdjustIndex( uIndex ) ].m_Profile;
}

NetPlay::eFriendState C_UIFriends::GetFriendState( const U32 uIndex )
{
	if ( IsPendingFriendSent( uIndex ) )
		return m_PendingFriendsSent[ AdjustIndexPendingSent( uIndex ) ].m_FriendState;

	if ( IsPendingFriendReceived( uIndex ) )
		return m_PendingFriendsReceived[ AdjustIndexPendingReceived( uIndex ) ].m_FriendState;

	return m_Friends[ AdjustIndex( uIndex ) ].m_FriendState;
}

NetPlay::RichPresence& C_UIFriends::GetFriendPresence( const U32 uIndex )
{
	static NetPlay::RichPresence blank;

	if ( IsPendingFriendSent( uIndex ) || IsPendingFriendReceived( uIndex ) )
		return blank;

	return m_Friends[ AdjustIndex( uIndex ) ].m_RichPresence;
}

const WCHAR* C_UIFriends::GetFriendPresenceString( const U32 uIndex )
{
	static C_TextStringID sentId( "PRO_FRIENDS_STATUS_REQUEST_SENT" );
	static C_TextStringID receivedId( "PRO_FRIENDS_STATUS_REQUEST_RECEIVED" );

	if ( IsPendingFriendSent( uIndex ) )
		return sentId.String();

	if ( IsPendingFriendReceived( uIndex ) )
		return receivedId.String();

	U32 uAdjustedIndex = AdjustIndex( uIndex );

	//if ( m_Presence[ uAdjustedIndex ].Length() == 0 )	// build this every frame now
	{
		// build the string we should render
		Interface().BuildPresenceString( uIndex, m_Presence[ uAdjustedIndex ] );

		if ( m_Presence[ uAdjustedIndex ].Length() == 0 )
		{
			// if we've failed to build a string - check online status and build one ourselves
			static C_TextStringID offlineId( "PRO_FRIENDS_STATUS_OFFLINE" );
			static C_TextStringID onlineNotThisId( "PRO_FRIENDS_STATUS_ONLINE_NOT_THIS" );
			static C_TextStringID onlineId( "PRO_FRIENDS_STATUS_ONLINE" );

			switch ( GetFriendState( uIndex ) )
			{
				case NetPlay::eFriendOffline:						m_Presence[ uAdjustedIndex ] = offlineId.String();			break;
				case NetPlay::eFriendOnlineButPlayingAnotherGame:	m_Presence[ uAdjustedIndex ] = onlineNotThisId.String();	break;
				case NetPlay::eFriendOnline:						m_Presence[ uAdjustedIndex ] = onlineId.String();			break;
				default:																										break;
			}
		}
	}

	return m_Presence[ uAdjustedIndex ].Chars();
}

SysProfile::GameInvite& C_UIFriends::GetFriendInvite( const U32 uIndex )
{
	static SysProfile::GameInvite blank;

	if ( IsPendingFriendSent( uIndex ) || IsPendingFriendReceived( uIndex ) )
		return blank;

	return m_Friends[ AdjustIndex( uIndex ) ].m_GameInvite;
}

bool C_UIFriends::CanViewGamerCards()
{
	return false;
}

bool C_UIFriends::CanInviteSystem( const U32 uIndex )
{
	if ( IsPendingFriendSent( uIndex ) || IsPendingFriendReceived( uIndex ) )
		return false;

	return m_Friends[ AdjustIndex( uIndex ) ].m_CanInvite;
}

bool C_UIFriends::CanInvite( const U32 uIndex )
{
	return ( CanInviteSystem( uIndex ) && Interface().CanInvite( uIndex ) );
}

bool C_UIFriends::CanJoinSessionInProgressSystem( const U32 uIndex )
{
	if ( IsPendingFriendSent( uIndex ) || IsPendingFriendReceived( uIndex ) )
		return false;

	return ( CanInviteSystem( uIndex ) && ( GetFriendInvite( uIndex ).m_Active || GetFriendPresence( uIndex ).m_JoinInProgressPossible ) );
}

bool C_UIFriends::CanJoinSessionInProgress( const U32 uIndex )
{
	return ( CanJoinSessionInProgressSystem( uIndex ) && Interface().CanJoinSessionInProgress( uIndex ) );
}

void C_UIFriends::ViewGamerCard( const U32 uIndex )
{
	// TODO?
}

void C_UIFriends::SendInvite( const U32 uIndex )
{
	if ( CanInvite( uIndex ) )
		Interface().SendInvite( uIndex );
}

void C_UIFriends::JoinSessionInProgress( const U32 uIndex )
{
	if ( CanJoinSessionInProgress( uIndex ) )
		Interface().JoinSessionInProgress( uIndex );
}

// -------------
// UIFriendsList
// -------------
void UIFriendsList::Init()
{
	C_GadgetBase::Init();

	m_fMaxNameLength = 0.0f;
	m_Pad			 = -1;
	m_CurrentItem	 = 0;
	m_FriendsVisible = 0;
	m_StartItem		 = 0;
	m_EndItem		 = 0;

	// messages
	AddSubscriber( E_UiMsg_CtrlButton_Up );
	AddSubscriber( E_UiMsg_CtrlButton_Down );
	AddSubscriber( E_UiMsg_MenuNext );
	AddSubscriber( E_UiMsg_MenuBack );
	AddSubscriber( E_UiMsg_CtrlButton_X );
	AddSubscriber( E_UiMsg_CtrlButton_Y );

	// kick off a read
	C_UIFriends::Instance().StartEnumeration();
}

void UIFriendsList::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	m_fMaxNameLength = gFont.StringLength( L"WWWWWWWWWWWWWWWW", m_RowHeight, C_FontStyle::Instance() );

	// resolve colour styles
	m_ColourFriendCount.SetByName( m_xtColourFriendCount );
	m_ColourSelected.SetByName( m_xtColourSelected );
	m_ColourUnselected.SetByName( m_xtColourUnselected );
	m_ColourOnline.SetByName( m_xtColourOnline );
	m_ColourOnlineDifferentGame.SetByName( m_xtColourOnlineDifferentGame );
	m_ColourOffline.SetByName( m_xtColourOffline );
	m_ColourPending.SetByName( m_xtColourPending );
}

bool UIFriendsList::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated || C_UIFriends::Instance().Working() )
		return C_GadgetBase::MessageHandler( pMessage );

	// Check if current item has changed.
	S32 item = m_CurrentItem;

	switch ( pMessage->Type() )
	{
		case E_UiMsg_CtrlButton_Up:
		case E_UiMsg_CtrlButton_Down:
		{
			const C_UiMessagePadData* pUIMessage = (C_UiMessagePadData*)pMessage;

			if ( m_Pad == -1 || ( m_Pad == pUIMessage->pad ) )
			{
				if ( pMessage->Type() == E_UiMsg_CtrlButton_Up )
				{
					if ( m_CurrentItem > 0 )
					{
						m_CurrentItem--;

						if ( m_CurrentItem < m_EndItem - m_FriendsVisible )
						{
							m_StartItem--;
							m_EndItem--;
						}
					}
				}
				else
				if ( pMessage->Type() == E_UiMsg_CtrlButton_Down )
				{
					if ( m_CurrentItem < (S32)C_UIFriends::Instance().NumFriends()-1 )
					{
						m_CurrentItem++;

						if ( m_CurrentItem >= m_StartItem + m_FriendsVisible )
						{
							m_StartItem++;
							m_EndItem++;
						}
					}
				}
			}
			break;
		}
		case E_UiMsg_MenuBack:
			C_UIFriends::Instance().Cancel();
			break;

		case E_UiMsg_MenuNext:
			C_UIFriends::Instance().ViewGamerCard( m_CurrentItem );
			break;

		case E_UiMsg_CtrlButton_X:
		{
			if ( C_UIFriends::Instance().IsPendingFriendSent( m_CurrentItem ) )
			{
				C_UIFriends::Instance().StartRevokeFriendRequest( m_CurrentItem );	// revoke
			}
			else 
			if ( C_UIFriends::Instance().IsPendingFriendReceived( m_CurrentItem ) )
			{
				C_UIFriends::Instance().StartAnswerFriendRequest( m_CurrentItem, false );	// decline
			}
			else
			{
				C_UIFriends::Instance().SendInvite( m_CurrentItem );	// invite
			}
			break;
		}

		case E_UiMsg_CtrlButton_Y:
		{
			if ( C_UIFriends::Instance().IsPendingFriendSent( m_CurrentItem ) )
				; // do nothing
			else if ( C_UIFriends::Instance().IsPendingFriendReceived( m_CurrentItem ) )
				C_UIFriends::Instance().StartAnswerFriendRequest( m_CurrentItem, true );	// accept
			else if ( C_UIFriends::Instance().CanJoinSessionInProgress( m_CurrentItem ) )	// only if i can join
			{
				C_UIFriends::Instance().JoinSessionInProgress( m_CurrentItem );	// join
				C_UIFriends::Instance().Cancel(); // shutdown
			}
			break;
		}

		default:
			return C_GadgetBase::MessageHandler( pMessage );
	}

	// Update the items.
	if ( item != m_CurrentItem )
	{
		UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Move ) );
	}

	return true;
}

void UIFriendsList::Render()
{
	if ( !m_Activated )
		return;

	// auto-transform
	C_UIAutoTransform transform( this );
	C_GadgetBase::Render();

	// background
	F32 fX = m_Position.x + ( gSimple2D.GetJustifiedCorner( m_Justify, 0, m_Panel ).X() * m_Size.x );

	if ( C_UIFriends::Instance().Working() )
	{
		static C_TextStringID workingId( "PRO_FRIENDS_WORKING" );
		gFont.DrawText( m_Layer, Maths::HVec3( fX, m_Position.y, m_Position.z ), E_Justify_TopLeft, workingId.String(), 
			m_SpriteColours, m_RowHeight, C_FontStyle::Instance(), 0.0f, m_Panel );

		// Reset these
		m_StartItem = m_EndItem = m_CurrentItem = 0;
		return;
	}

	U32 uNumFriends = C_UIFriends::Instance().NumFriends();

	if ( uNumFriends == 0 )
	{
		static C_TextStringID noFriendsId( "PRO_FRIENDS_NO_FRIENDS" );
		gFont.DrawText( m_Layer, Maths::HVec3( fX, m_Position.y, m_Position.z ), E_Justify_TopLeft, noFriendsId.String(), 
			m_SpriteColours, m_RowHeight, C_FontStyle::Instance(), 0.0f, m_Panel );
	}
	else
	{
		F32 fY = m_Position.y;

		if ( m_EndItem == 0 ) // This will need resetting after we have read the friends
			m_EndItem = Maths::Clamp( m_StartItem + m_FriendsVisible, 0, (S32)C_UIFriends::Instance().NumFriends() );

		// Render each friend
		for ( S32 i = m_StartItem; i < m_EndItem; i++ )
		{
			// status
			C_UISpriteColours col;

			switch ( C_UIFriends::Instance().GetFriendState( i ) )
			{
				case NetPlay::eFriendOnline:						col = m_ColourOnline;				break;
				case NetPlay::eFriendOnlineButPlayingAnotherGame:	col = m_ColourOnlineDifferentGame;	break;
				case NetPlay::eFriendOffline:						col = m_ColourOffline;				break;
				case NetPlay::eFriendRequestReceived:				col = m_ColourPending;				break;
				case NetPlay::eFriendRequestSent:					col = m_ColourPending;				break;
				default:											col = m_ColourOffline;				break;
			}

			// state icon
			gSimple2D.AddSprite( m_Layer, Maths::HVec3( fX - m_IconPadding, fY + ( m_RowHeight / 2.0f )+1.0f, m_Position.z ), m_RowHeight * s_fIconMult, m_RowHeight * s_fIconMult, col, 
				m_Panel, E_SpriteType_Sprite, m_RenderState );
			gSimple2D.AddJustification( E_Justify_MiddleLeft );
			gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );

			// name
			gFont.DrawText( m_Layer, Maths::HVec3( fX, fY, m_Position.z ), E_Justify_TopLeft, 
				C_UIFriends::Instance().GetFriendProfile( i ).m_Name, 
				( i == m_CurrentItem ) ? m_ColourSelected : m_ColourUnselected, m_RowHeight, C_FontStyle::Instance(), 0.0f, m_Panel );

			// invite icon
			if ( C_UIFriends::Instance().GetFriendInvite( i ).m_Active )
			{
				gSimple2D.AddSprite( m_Layer, Maths::HVec3( fX + m_StatusSpacing - 2.0f, fY + ( m_RowHeight / 2.0f ), m_Position.z ), m_RowHeight * s_fInviteMult, m_RowHeight * s_fInviteMult, 
					( i == m_CurrentItem ) ? m_ColourSelected : m_ColourUnselected, 
					m_Panel, E_SpriteType_Sprite, m_RenderState );
				gSimple2D.AddJustification( E_Justify_MiddleRight );
				gSimple2D.AddTexUVs( 0, GetTextureParam( 1 ) );
			}

			// presence
			gFont.DrawText( m_Layer, Maths::HVec3( fX + m_StatusSpacing, fY, m_Position.z ), E_Justify_TopLeft, 
				C_UIFriends::Instance().GetFriendPresenceString( i ), 
				( i == m_CurrentItem ) ? m_ColourSelected : m_ColourUnselected, m_RowHeight, C_FontStyle::Instance(), 0.0f, m_Panel );

			fY += m_RowHeight;
		}

		// current count
		static C_TextStringID numFriendsId( "PRO_FRIENDS_NUM_OF_NUM" );
		m_NumString.Format( numFriendsId.String(), ( m_CurrentItem + 1 ), C_UIFriends::Instance().NumFriends() );

		gFont.FixedSpaceNext( E_FixedSpaceType_Numbers );
		gFont.DrawText( m_Layer, Maths::HVec3( fX + ( m_Size.x * 0.5f ), m_Position.y + m_Size.y + ( m_RowHeight / 2.0f ), m_Position.z ), E_Justify_TopCentre, m_NumString.Chars(), 
				m_ColourFriendCount, m_RowHeight, C_FontStyle::Instance(), 0.0f, m_Panel );
	}

	// info lines
	bool bCanSelect = ( uNumFriends > 0 );

	bool bInfoGamerCards = false;
	bool bInfoRevokeRequest = false;
	bool bDeclineAcceptRequest = false;
	bool bCanInvite = false;
	bool bCanJoinInProgress = false;

	if ( bCanSelect )
	{
		bInfoGamerCards = C_UIFriends::Instance().CanViewGamerCards();

		bInfoRevokeRequest = C_UIFriends::Instance().IsPendingFriendSent( m_CurrentItem );

		bDeclineAcceptRequest  = !bInfoRevokeRequest;
		bDeclineAcceptRequest &= C_UIFriends::Instance().IsPendingFriendReceived( m_CurrentItem );

		bCanInvite  = !bDeclineAcceptRequest;
		bCanInvite &= C_UIFriends::Instance().CanInvite( m_CurrentItem );

		bCanJoinInProgress  = !bDeclineAcceptRequest;
		bCanJoinInProgress &= C_UIFriends::Instance().CanJoinSessionInProgress( m_CurrentItem );
	}

	Lua::Call( g_pUILua->GetState(), "Friends_SetupInfoLine", "bbbbbb", 
		bInfoGamerCards, bInfoRevokeRequest, bDeclineAcceptRequest, bCanInvite, bCanJoinInProgress, bCanSelect );
}

};	// namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
