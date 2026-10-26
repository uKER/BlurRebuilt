#ifndef PROFILEINTERFACEMESSAGES_H
#define PROFILEINTERFACEMESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/MessageBase.h>
#include <misc_shared/UID/FourCC.h>
#include <SystemProfile/SystemProfile.h>
#include <SystemProfile/GameInvite.h>

namespace UI
{

enum E_ProfileNotifications
{
	E_ProfMsg_Start = FOURCC_HI20('UIPN'), // Profile notification Message Type

	E_ProfMsg_GameInvitePending,
	E_ProfMsg_SignInChanged,
	E_ProfMsg_DuplicateSignIn,
	E_ProfMsg_ProfilesChanged,
	E_ProfMsg_ProfileSettingsChanged,
	E_ProfMsg_StorageDeviceChanged,
	E_ProfMsg_NewContentInstalled,
	E_ProfMsg_InputDeviceChanged,
	E_ProfMsg_MuteInfoChanged,
	E_ProfMsg_ServiceConnectionLost,
	E_ProfMsg_ServiceConnectionLostXLSP,
	E_ProfMsg_ServiceConnectionEstablished,
	E_ProfMsg_LobbyConnectionLost,
	E_ProfMsg_NetworkCableRemoved,
	E_ProfMsg_FriendRequestPending,
	E_ProfMsg_FriendSignedIn,
	E_ProfMsg_DeviceConfigChanged,
	E_ProfMsg_SignedIn,
	E_ProfMsg_SignedOut,

	E_ProfMsg_Count,
};

class C_ProfMsg_GameInvitePending : public Messaging::C_MessageDerived<C_ProfMsg_GameInvitePending, E_ProfMsg_GameInvitePending>
{
public:
	C_ProfMsg_GameInvitePending( const SysProfile::GameInvite& invite )
		: m_Invite( invite ) {  }
public:
	SysProfile::GameInvite	m_Invite;
};

class C_ProfMsg_SignInChanged : public Messaging::C_MessageDerived<C_ProfMsg_SignInChanged, E_ProfMsg_SignInChanged> {  };
class C_ProfMsg_DuplicateSignIn : public Messaging::C_MessageDerived<C_ProfMsg_DuplicateSignIn, E_ProfMsg_DuplicateSignIn> {  };
class C_ProfMsg_ProfilesChanged : public Messaging::C_MessageDerived<C_ProfMsg_ProfilesChanged, E_ProfMsg_ProfilesChanged> {  };
class C_ProfMsg_ProfileSettingsChanged : public Messaging::C_MessageDerived<C_ProfMsg_ProfileSettingsChanged, E_ProfMsg_ProfileSettingsChanged> {  };
class C_ProfMsg_StorageDeviceChanged : public Messaging::C_MessageDerived<C_ProfMsg_StorageDeviceChanged, E_ProfMsg_StorageDeviceChanged> {  };
class C_ProfMsg_NewContentInstalled : public Messaging::C_MessageDerived<C_ProfMsg_NewContentInstalled, E_ProfMsg_NewContentInstalled> {  };
class C_ProfMsg_InputDeviceChanged : public Messaging::C_MessageDerived<C_ProfMsg_InputDeviceChanged, E_ProfMsg_InputDeviceChanged> {  };
class C_ProfMsg_MuteInfoChanged : public Messaging::C_MessageDerived<C_ProfMsg_MuteInfoChanged, E_ProfMsg_MuteInfoChanged> {  };
class C_ProfMsg_ServiceConnectionLost : public Messaging::C_MessageDerived<C_ProfMsg_ServiceConnectionLost, E_ProfMsg_ServiceConnectionLost> {  };
class C_ProfMsg_ServiceConnectionLostXLSP : public Messaging::C_MessageDerived<C_ProfMsg_ServiceConnectionLostXLSP, E_ProfMsg_ServiceConnectionLostXLSP> {  };
class C_ProfMsg_ServiceConnectionEstablished : public Messaging::C_MessageDerived<C_ProfMsg_ServiceConnectionEstablished, E_ProfMsg_ServiceConnectionEstablished> {  };
class C_ProfMsg_LobbyConnectionLost : public Messaging::C_MessageDerived<C_ProfMsg_LobbyConnectionLost, E_ProfMsg_LobbyConnectionLost> {  };
class C_ProfMsg_NetworkCableRemoved : public Messaging::C_MessageDerived<C_ProfMsg_NetworkCableRemoved, E_ProfMsg_NetworkCableRemoved> {  };
class C_ProfMsg_FriendRequestPending : public Messaging::C_MessageDerived<C_ProfMsg_FriendRequestPending, E_ProfMsg_FriendRequestPending> {  };
class C_ProfMsg_FriendSignedIn : public Messaging::C_MessageDerived<C_ProfMsg_FriendSignedIn, E_ProfMsg_FriendSignedIn> {  };

class C_ProfMsg_DeviceConfigChanged: public Messaging::C_MessageDerived<C_ProfMsg_DeviceConfigChanged, E_ProfMsg_DeviceConfigChanged>
{
public:
	C_ProfMsg_DeviceConfigChanged( U8 uDeviceConfigChanged )
		: m_uDeviceConfigChanged( uDeviceConfigChanged ) {  }
public:
	U8	m_uDeviceConfigChanged;
};

class C_ProfMsg_SignedIn: public Messaging::C_MessageDerived<C_ProfMsg_SignedIn, E_ProfMsg_SignedIn>
{
public:
	C_ProfMsg_SignedIn( S32 pad )
		: m_Pad( pad ) {  }
public:
	S32	m_Pad;
};

class C_ProfMsg_SignedOut: public Messaging::C_MessageDerived<C_ProfMsg_SignedOut, E_ProfMsg_SignedOut>
{
public:
	C_ProfMsg_SignedOut( S32 pad )
		: m_Pad( pad ) {  }
public:
	S32	m_Pad;
};

}; // namespace UI

// macro for registering the debug info for profile messages

#ifndef CFG_GOLD

#define _UI_PROFILE_MESSAGE_DEBUG( msg ) \
	Messaging::RegisterDebugInfo( UI::E_ProfMsg_##msg, "E_ProfMsg_"#msg ); \
	uNumMessagesRegistered++;

#endif

#endif /* PROFILEINTERFACEMESSAGES_H */
