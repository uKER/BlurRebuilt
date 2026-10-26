#ifndef SYSTEM_PROFILE_H
#define SYSTEM_PROFILE_H

#ifdef _MSC_VER
#pragma once
#endif


#if defined PLATFORM_PS3
#include <np.h>
#define BLUR_USER_ID(uid) bdAuthUtility::getUserID((uid).handle.data)	// [Blur2PC]
#define SYSTEM_PROFILE_NAME_LENGTH (16+1)
#elif defined (LIVE_ON_WINDOWS)
#define SYSTEM_PROFILE_NAME_LENGTH (XUSER_NAME_SIZE)
#else
#define SYSTEM_PROFILE_NAME_LENGTH (16+1)
// [Blur2PC] the PC branch below uses bdOnlineUserID; make the header self-contained
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
// [Blur2PC] a few game files call the Xbox Live helper on Demonware ids; on PC ids are plain integers
#ifndef XOnlineAreUsersIdentical
#define XOnlineAreUsersIdentical(a, b) ((a) == (b))
#endif
// [Blur2PC] game code shared between PS3 and PC extracts the Demonware id from the PS3 SceNpId handle;
// on PC the UserIdentifier already holds the bdOnlineUserID.
#define BLUR_USER_ID(uid) (uid)
#endif


namespace SysProfile
{	
	typedef enum
	{
		eProfileStateNotActive = 0,	// no user is signed on with this profile.
		eProfileStateOffline,		// user is signed on but has an offline profile	
		eProfileStateOnline,		// user is signed on with an online enabled profile.
		eProfileStateBlagged,		// user is signed on but without a valid system profile..
	}eState;

	/// Contains the unique user identifier to reference a single player.
	/** Also contains wether this player is considered to be a guest. Guest players
	 *	have much less capabilities than their sponsor account.
	 *	If you need to change the platforms user identifier into the demonware version (bdOnlineUserId) then there use GetDemonwareUserId() within the 
	 *	system profile manager class.
	 */
	struct UserIdentifier
	{
#if defined(LIVE_ON_WINDOWS)
		UserIdentifier()
		{
			m_Uid = 0;
			m_IsGuest = false;

		}
		XUID	m_Uid;
		bool operator == (const UserIdentifier& other) const { return other.m_Uid == m_Uid; } 
		bool operator != (const UserIdentifier& other) const { return other.m_Uid != m_Uid; }

#elif defined (PLATFORM_PS3)
		UserIdentifier()
		{
			memset(&m_Uid, 0, sizeof(m_Uid));
			m_IsGuest = false;
		}
		SceNpId m_Uid;			// Network platform user id. 

		// NOTE -> user comparison will only work if both users are online otherwise sceNpUtilCmpNpId() will fail since there is no
		// sceNpId for offline users.
		bool operator == (const UserIdentifier& other) const { return sceNpUtilCmpNpId(&m_Uid, & other.m_Uid) == 0 ? true : false; }
		bool operator != (const UserIdentifier& other) const { return sceNpUtilCmpNpId(&m_Uid, & other.m_Uid) != 0 ? true : false; }
#else
		UserIdentifier()
		{
			m_Uid = 0;
			m_IsGuest = false;
		}
		bdOnlineUserID m_Uid;	// demonware user id
		bool operator == (const UserIdentifier& other) const { return other.m_Uid == m_Uid; } 
		bool operator != (const UserIdentifier& other) const { return other.m_Uid != m_Uid; }
#endif
		// is this user flagged as a guest.
		bool	m_IsGuest;
	};


	/// contains only minimal data that can go across the network.
	/** we don't need to send all data when talking to remote peers.
	 */
	struct BaseProfile : UserIdentifier
	{
#if defined(LIVE_ON_WINDOWS)
		BaseProfile()
		{
			memset(m_Name, 0, sizeof(m_Name));
		}
#elif defined PLATFORM_PS3
		BaseProfile()
		{
			memset(m_Name, 0, sizeof(m_Name));
		}
#else
		BaseProfile()
		{
			memset(m_Name, 0, sizeof(m_Name));
		}
#endif
		WCHAR*	Name(void) const {return (WCHAR*)m_Name;}
		WCHAR	m_Name[SYSTEM_PROFILE_NAME_LENGTH];
	};


	/// contains specific privilege settings that a user has access too.
	/** basically a load of parental controls.
	 */
	struct Privileges
	{
		Privileges() :	m_ValidPrivileges(false), m_OnlineMultiplayerPrivilege(false), m_ScoreboardPrivilege(false), m_CommunicationsPrivilege(false), m_CommunicationsFriendsOnlyPrivilege(false),
						m_ProfileViewingPrivilege(false), m_ProfileViewingFriendsOnlyPrivilege(false), m_ViewUserContentPrivilege(false), m_ViewUserContentFriendsOnlyPrivilege(false),
						m_PurchaseContentPrivilege(false), m_ListPlayPrivilege(false), m_ParentalLocked(false)
		{
		}

		U16 m_ValidPrivileges:1;						// are the privilege settings valid
		U16 m_OnlineMultiplayerPrivilege:1;				// can this player play over the internet..
		U16	m_ScoreboardPrivilege:1;					// can this player read/write to online scoreboards
		U16 m_CommunicationsPrivilege:1;				// can the player use voice and video comms
		U16 m_CommunicationsFriendsOnlyPrivilege:1;		// can the player use voice and video comms, but only with friends
		U16 m_ProfileViewingPrivilege:1;				// can the player view game profiles
		U16 m_ProfileViewingFriendsOnlyPrivilege:1;		// can the player view game profiles, but only of friends
		U16 m_ViewUserContentPrivilege:1;				// can the player use user created content
		U16 m_ViewUserContentFriendsOnlyPrivilege:1;	// can the player use user created content, but only with friends
		U16 m_PurchaseContentPrivilege:1;				// can the player purchase additional content (DLC)
		U16 m_ListPlayPrivilege:1;						// does the player get access to List play on games for Windows Live
		U16	m_ParentalLocked:1;							// used on ps3 when we have determined some features have been disabled because of parental age restrictions
	};


	/// contains everything retrieved from the system for a specific player.
	struct FullProfile : BaseProfile, Privileges
	{
		FullProfile() : m_Ref(0), m_LastModifiedTick(0), m_State(eProfileStateNotActive), m_UserIndex(0)
		{}

		bool Active(void) const {return eProfileStateNotActive != m_State;}
		bool operator == (const FullProfile& other) const {return other.m_Ref == m_Ref;} 

		U64		m_Ref;
		U32		m_LastModifiedTick;
		eState	m_State;
		U8		m_UserIndex;
	};

}; // namespace SysProfile

#endif
