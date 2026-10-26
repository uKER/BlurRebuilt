#ifndef UIFRIENDSLIST_H
#define UIFRIENDSLIST_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/Gadgets/Buttons.h"
#include "ui/Game/Gadgets/BaseUIList.h"
#include "ui/Util/WideString.h"

#include <Network/FriendEnumerator.h>
#include <Network/PendingFriendEnumerator.h>

#include <misc_shared/Messaging/MessageHandler.h>
#include <btl/Containers/containers.h>

namespace UI
{

// --------------------
// C_UIFriendsInterface
// --------------------
class C_UIFriendsInterface
{
public:
	virtual bool CanInvite( const U32 uIndex )											{ return true; }
	virtual bool CanJoinSessionInProgress( const U32 uIndex )							{ return true; }

	virtual void SendInvite( const U32 uIndex )											{ /**/ }
	virtual void JoinSessionInProgress( const U32 uIndex )								{ /**/ }

	virtual void BuildPresenceString( const U32 uIndex, C_WideString64& string )		{ /**/ }
};

// -----------
// C_UIFriends
// -----------
class C_UIFriends : public Messaging::C_MessageHandler
{
public:
	C_UIFriends()
		: m_bActive( true )
		, m_bDisplayed( false )
		, m_bCancel( false )
		, m_sContext( -1 )
		, m_sRestoreContext( 0 )
		, m_sActivateButton( -1 )
		, m_pEnumeratorOverride( NULL )
		, m_pEnumeratorSentOverride( NULL )
		, m_pEnumeratorReceivedOverride( NULL )
		, m_pInterfaceOverride( NULL )
		, m_bReading( false )
		, m_bWorking( false )
		, m_bError( false )
		, m_uNumFriends( 0 )
		, m_uNumPendingFriendsSent( 0 )
		, m_uNumPendingFriendsReceived( 0 )
	{
		//
	}

public:
	static C_UIFriends&						Instance()							{ return m_sInstance; }

public:
	void									Initialise( const S32 sContext, const S32 sActivateButton, const char* pScriptName = "FriendsList.lua" );

	void									Active( const bool bActive )														{ m_bActive = bActive; }
	bool									Active()																			{ return m_bActive; }

	bool									Displayed()																			{ return m_bDisplayed; }

	virtual bool							MessageHandler( const Messaging::C_MessageBase* pMessage );

	void									SetEnumerator( NetPlay::C_FriendsEnumerator* pEnumeratorOverride )					{ m_pEnumeratorOverride = pEnumeratorOverride; }
	void									SetEnumeratorSent( NetPlay::C_PendingFriendsEnumerator* pEnumeratorOverride )		{ m_pEnumeratorSentOverride = pEnumeratorOverride; }
	void									SetEnumeratorReceived( NetPlay::C_PendingFriendsEnumerator* pEnumeratorOverride )	{ m_pEnumeratorReceivedOverride = pEnumeratorOverride; }

	void									SetInterface( C_UIFriendsInterface* pInterface )									{ m_pInterfaceOverride = pInterface; }

	void									StartEnumeration();
	void									StartAnswerFriendRequest( const U32 uIndex, const bool bAccept );
	void									StartRevokeFriendRequest( const U32 uIndex );

	void									Update();
	void									Cancel();

	bool									Working()							{ return m_bReading || m_bWorking; }
	U32										NumFriends()						{ return m_uNumFriends; }
	S32										Context()							{ return m_sContext; }

	bool									IsPendingFriendSent( const U32 uIndex );
	bool									IsPendingFriendReceived( const U32 uIndex );

	S32										GetPadNo(void) const { return m_sPadNo; }
	SysProfile::BaseProfile&				GetFriendProfile( const U32 uIndex );
	NetPlay::eFriendState					GetFriendState( const U32 uIndex );
	NetPlay::RichPresence&					GetFriendPresence( const U32 uIndex );
	const WCHAR*							GetFriendPresenceString( const U32 uIndex );
	SysProfile::GameInvite&					GetFriendInvite( const U32 uIndex );

	bool									CanViewGamerCards();
	bool									CanInvite( const U32 uIndex );
	bool									CanJoinSessionInProgress( const U32 uIndex );

	void									ViewGamerCard( const U32 uIndex );
	void									SendInvite( const U32 uIndex );
	void									JoinSessionInProgress( const U32 uIndex );

private:
	bool									CanInviteSystem( const U32 uIndex );
	bool									CanJoinSessionInProgressSystem( const U32 uIndex );

	U32										AdjustIndexPendingSent( const U32 uIndex );
	U32										AdjustIndexPendingReceived( const U32 uIndex );
	U32										AdjustIndex( const U32 uIndex );

	NetPlay::C_FriendsEnumerator&			Enumerator()						{ return ( m_pEnumeratorOverride ? *m_pEnumeratorOverride : m_Enumerator ); }
	NetPlay::C_PendingFriendsEnumerator&	EnumeratorSent()					{ return ( m_pEnumeratorSentOverride ? *m_pEnumeratorSentOverride : m_EnumeratorSent ); }
	NetPlay::C_PendingFriendsEnumerator&	EnumeratorReceived()				{ return ( m_pEnumeratorReceivedOverride ? *m_pEnumeratorReceivedOverride : m_EnumeratorReceived ); }

	C_UIFriendsInterface&					Interface()							{ return ( m_pInterfaceOverride ? *m_pInterfaceOverride : m_Interface ); }

private:
	static C_UIFriends						m_sInstance;

private:
	bool									m_bActive;
	bool									m_bDisplayed;
	bool									m_bCancel;

	S32										m_sPadNo;

	S32										m_sContext;
	S32										m_sRestoreContext;
	S32										m_sActivateButton;
	static const U32						m_uMaxScriptNameLength = 32;
	char									m_pScriptName[ m_uMaxScriptNameLength ];

	bool									m_bReading;
	bool									m_bWorking;
	bool									m_bError;

	NetPlay::C_FriendsEnumerator			m_Enumerator;
	NetPlay::C_FriendsEnumerator*			m_pEnumeratorOverride;
	NetPlay::C_PendingFriendsEnumerator		m_EnumeratorSent;
	NetPlay::C_PendingFriendsEnumerator*	m_pEnumeratorSentOverride;
	NetPlay::C_PendingFriendsEnumerator		m_EnumeratorReceived;
	NetPlay::C_PendingFriendsEnumerator*	m_pEnumeratorReceivedOverride;

	C_UIFriendsInterface					m_Interface;
	C_UIFriendsInterface*					m_pInterfaceOverride;

	U32										m_uNumFriends;
	U32										m_uNumPendingFriendsSent;
	U32										m_uNumPendingFriendsReceived;

	NetPlay::FriendProfile					m_Friends[ MAX_FRIENDS ];
	NetPlay::PendingFriendProfile			m_PendingFriendsSent[ MAX_FRIENDS ];
	NetPlay::PendingFriendProfile			m_PendingFriendsReceived[ MAX_FRIENDS ];

	C_WideString64							m_Presence[ MAX_FRIENDS ];

	bool									m_bAutoRefresh;
	F32										m_fAutoRefreshTimer;

	F32										m_fNextOpenDelayTimer;
};

// -------------
// UIFriendsList
// -------------
class UIFriendsList : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIButtonType_FriendsList, "FRIENDS_LIST" )

public:
	void 			Init();
	void 			Setup( lua_State* pState );
	bool 			MessageHandler( const Messaging::C_MessageBase* pMessage );
	void 			Render();

	void			SetPad( const S32 pad )		{ m_Pad = pad; }

public: // exposed for Xt
	XtSz8				m_xtColourFriendCount;
	XtSz8				m_xtColourSelected;
	XtSz8				m_xtColourUnselected;
	XtSz8				m_xtColourOnline;
	XtSz8				m_xtColourOnlineDifferentGame;
	XtSz8				m_xtColourOffline;
	XtSz8				m_xtColourPending;

	S32					m_Pad;
	F32					m_RowHeight;
	F32					m_StatusSpacing;
	F32					m_IconPadding;
	S32					m_FriendsVisible;

private:
	S32					m_CurrentItem;
	S32					m_StartItem;
	S32					m_EndItem;

	F32					m_fMaxNameLength;
	C_WideString32		m_NumString;

	C_UISpriteColours	m_ColourFriendCount;
	C_UISpriteColours	m_ColourSelected;
	C_UISpriteColours	m_ColourUnselected;
	C_UISpriteColours	m_ColourOnline;
	C_UISpriteColours	m_ColourOnlineDifferentGame;
	C_UISpriteColours	m_ColourOffline;
	C_UISpriteColours	m_ColourPending;
};

};	// namespace UI

#endif /* UIFRIENDSLIST_H */
