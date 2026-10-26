#ifndef SYSTEM_PROFILE_MANAGER_H
#define SYSTEM_PROFILE_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Threads.h>
#include <Network/FriendEnumerator.h>
#include <SystemProfile/SystemProfile.h>
#include <Network/Presence.h>
#include <System/Time/Time.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>

#if defined ( PLATFORM_PS3 )
#include "../source/SystemProfile/ps3/NpFriendStorage.h"
#elif defined ( PLATFORM_PC )
#include "../source/SystemProfile/pc/pcFriendStorage.h"
#endif

// forward declerations
namespace NetPlay { struct SessionSecurity; class C_AsyncTask;}

namespace SysProfile
{	
	// forward declarations
	struct GameInvite;
#if defined ( PLATFORM_PS3 )
	class C_NpThreadedFriendEnumerator;
#endif

	/// Gets passed into the Poll function to inform game of any specific notifications that have been received from the OS
	struct Notifications
	{
		Notifications() :
			m_SignInChanged(false), m_DuplicateSignIn(0), m_ProfilesChanged(false), m_ProfileSettingsChanged(false), 
			m_StorageDeviceChanged(false), m_NewContentInstalled(false), m_InputDeviceChanged(false), m_MuteInfoChanged(false),
			m_ServiceConnectionLost(false), m_ServiceConnectionEstablished(false), m_FriendsUpdated(false),m_NetworkCableRemoved(false),
			m_DeviceConfigChanged(0)
		{}
		bool	m_SignInChanged								: 1;		/**< has a system profile been signed in or out. */
		bool	m_DuplicateSignIn							: 1;		/**< profile has been signed in on a remote box which has signed out a profile on the local box. */
		bool	m_ProfilesChanged							: 1;		/**< profile's on this box have changed in some way. */
		bool	m_ProfileSettingsChanged					: 1;		/**< does the game need to re read the profile settings for the active players. */
		bool	m_StorageDeviceChanged						: 1;		/**< MU inserted removed. */
		bool	m_NewContentInstalled						: 1;		/**< a new dlc pack has just been installed. */
		bool	m_InputDeviceChanged						: 1;		/**< pad inserted or removed. */
		bool	m_MuteInfoChanged							: 1;		/**< mute data has changed. */
		bool	m_ServiceConnectionLost						: 1;		/**< profile have lost conenction to the service provider. */
		bool	m_ServiceConnectionEstablished				: 1;		/**< profile have established conenction to the service provider. */
		bool	m_FriendsUpdated							: 1;		/**< this will go true if a friend is added / removed, maybe the ingame friends display should re enumerate. */
		bool	m_NetworkCableRemoved						: 1;		/**< this will go true if the ethernet cable is removed. */
		U8		m_DeviceConfigChanged;									/**< power state changed. */
	};


	/// Main class for managing system profiles (Gamer profiles on xbox360)
	/** Also manages friends enumeration / game invites on PS3.
	 */
	class C_Manager
	{
	public:

		/// Defines the possible connection states to our service provider.
		typedef enum 
		{
			eServiceStateUnknown = 0,	/**< unknown connection state. */
			eServiceStateDisconnected,	/**< not connected to the service provider. */
			eServiceStateConnected,		/**< valid connection to the service provider. */
		}eServiceState;

		// on ps3 we need aceess to the enumerated friends cache.
		friend class NetPlay::C_FriendsEnumerator;
			
		/// get a reference to the static class instance
		static C_Manager& Instance(void)	{return s_ProfileManager;}

		/// startup and allocate resources
		/** for PS3 we need to pass in the age restriction of the game so the profiles get locked out of multiplayer
		 *	if required.
		 *	on PS3 make sure the CELL_SYSMODULE_L10N module is loaded before using this class.
		 */
		bool Initialise(const S32 ageRequirement = -1);

		///  Close down and release resources
		void Shutdown(void);

		/// Try and enter a critical section.. useful if you want to access multiple profiles at once via GetProfile() 
		void EnterCritical(void);

		/// Try an leave the critical section.. Must call after using EnterCritical() or bad stuff will happen !!
		void LeaveCritical(void);

		/// Set which player is falgged as primary in the UI.
		/** On PS3 this controller index will be used to determine which system profile is flagged as primary.
		 */
		void SetPrimaryPadNo(S32 padNo) { m_PrimaryPadNo = padNo; }

		/// Get the current primary controller.
		S32	 GetPrimaryPadNo(void) const { return m_PrimaryPadNo; }

		/// Call once per frame to perform any asynchronous tasks.
		/**	@param outGameInvite if a player ahs accepted a game invite this object will show as valid and the game should act upon it (Xbox360 only)
		 *	@param notifications bunch of notifications that the cam can monitor and take action on if required.
		 */
		void Poll(GameInvite& outGameInvite, Notifications& notifications, const F32 frameTime);

		/// Get the maximum number of players we can have active at any one time.
		U32	 MaxUsers(void) const {return m_MaxUsers;}

		/// Is a system profile active for a specific user.
		/**	@param userIndex user index of player to check.
		 *	@param outProfileData if profile is active this will hold the actual profile data upon return.
		 *	@return true if there is a valid system profile active on the specified user index, false is not.
		 */
		bool GetProfile(const U32 userIndex, FullProfile& outProfileData);

		/// Once the game has an active profile we can monitor that profile for changes.
		/** On Xbox360 a profile can go from online to offline without triggering a ProfilesChanged() message since it is the same user just a different internal state.
		 *	@param activeProfile should be an active profile enumerated via GetProfile(). 
		 */
		void MonitorProfile(FullProfile& activeProfile);

		/// Are we currently displaying a system overlay.
		/** This can be used to detect if we need to pause the game because control has been taken out of the players hands.  
		 *	On xbox360 this will be whenever the guide is active.
		 *	On ps3 this will be whenever the OS has intercepted the pad for its own overlays.
		 */
		bool ShowingSystemOverlay(void) const {return m_SystemHudActive;}
		
		/// Setup a profile to be a blagged user, this does not overwrite any internal profile data
		bool CreateBlaggedProfile(const U32 userIndex, const WCHAR* pDefaultName, FullProfile& outProfileData);

		/// when online we can set out presence so it displays on friends lists, On PS3 we are limited to 6 presence updates per minute!.
		/** @param userIndex the local player who we wish to set presence for.
		 *	@param presenceData correctly configured presence data to set.
		 *	@return false if presence was not sent to the platform servers.
		 */
		bool SetPresence(const U32 userIndex, RacerNet::eRichPresence presence, bool joinInProgress);

		/// Are we in the process of sendiing a game invite?
		bool SendingGameInvite(void) const { return m_GameInviteBeingSent; }

		/// Start the asynchronous process of sending a game invite to a list of friends.
		/** The game is in control of making sure the friends are valid and that the local player is actually in the session before sending the invite.
		 *	The game should only call this once it has created an online session that is joinable, on xbox360 that means local player have been added into the session as well.
		 *	@see ContinueSendInvite()
		 *	function will fail if we are in the process of sending an invite via StartSendExternalPartyInvite().
		 *	We can sent to a single invite to multiple friends in 1 call, this is limited to 10 players on PC and PS3.
		 *	@param userIndex the local player who wishes to send the invite.	
		 *	@param pRecipients pointer to a list of players who we wish to invite.
		 *	@param numRecipients how many players are listed n the pRecipients array.
		 *	@param sessionData security information for the session we wish the recipients to join.
		 *	@param inviteText is the string that will show up in the Xbox360 guide when the player goes to accept the invite, it's not used on other platforms.
		 *	@return true if the asynchronous process of sending the invite was started (call ContinueSendGameInvite() until completion) or false if the process failed to start. 
		 */	
		bool StartSendGameInvite(const U32 userIndex, const UserIdentifier* pRecipients, const U32 numRecipients, NetPlay::SessionSecurity& sessionData, WCHAR* inviteText = NULL);

		/// Starts the asynchronous task of inviting players in our external party session into a game session.
		/** This is only applicable for xbox360, no other platform has partys sessions built into the OS.
		 *	This function will fail if we are already sending a game invite vis StartSendgameInvite(). 
		 *	If the specified local player is not a member of the external party session, or there is not enough remote players in that session
		 *	then his friends list will be displayed so they can invite from this.
		 *	Check for send completion via ContinueSendInvite().
		 *	The game must have already created a local matching session and added the local players into it for the guide to allow the invite to be sent.
		 *	@param userIndex the local user index of the player who will be sending the invite. they must be in the external partys session. 
		 *	@param minRemote minimum number of remote players that must be in the external session, no point sending an invite if there is no remote players. 
		 */
		bool StartSendExternalPartyInvite(const U32 userIndex, const U8 minRemote = 1);

		/// Continues the asynchronous process of sending a game invite.
		/** Game must have successfully called StartSendGameInvite() before calling this. 
		 *	Game must also call Poll() while sending the invite as the bulk of the logic happens in there. If we are using demonware we must also pump their lobby connection.
		 *	@param errorOccurred once this function returns true then this variable can be checked to see if the invite was sent ok.
		 *	@return true when the process of sending the invite has finished.
		 */
		bool ContinueSendGameInvite(bool& errorOccurred);

		/// Game has processed the invite received from a friend.
		/** Needs to be called on PS3 and PC when the game has chosen to accept a pending game invite that was sent by a friend.
		 *	Internally the invite will be removed from the friend so the game can't process the invite more than once.
		 *	This function will do nothing on Xbox360 since invites are handled by the guide. 
		 *	@param userIndex the local players index of who has accepted the game invite.
		 *	@param inviteSender the friend who originally sent the invite.	
		 *	@param accepted was the invite accepted or declined.
		 *	@return true if the invite was successfully processed.
		 */
		bool ProcessGameInvite(const U32 userIndex, const UserIdentifier& inviteSender, const bool accepted);

		/// Given a user index of a player who is signed in generate that player demonware user identifier.
		/**	The demonware user identifier is a hash of the players name (on xbox360 it's a copy of the players XUID), this means the player must
		 *	already hbe authenticated to the demonware servers. This function will fail if you pass a guest userIndex. 
		 *	@param userIndex the local players pad index of who to generate the Id for.
		 *	@param demonwareUserId will contain the players demonware identifier if function was successful.
		 *	@return true if the user identifier was generated or false if the player was invalid.
		 */
		bool GetDemonwareUserId(const U32 userIndex, bdOnlineUserID& demonwareUserId);

		/// What is the current status of the connection to the online service provider.
		/** @See eServiceState
		 */
		eServiceState ServiceConnectionState(void) const {return m_LatestServiceState;}

		/// Game needs to call this function when we receive a status change for a demonware player.
		/** On PC this is used to track what friends are online and which are offline. 
		 *	Game needs to derrive a class from bdLobbyEventHandler and register this class with bdLobbyService to start receiving demonware messages.
		 *	This function should be used when we receive bdLobbyEventHandler::onPlayerStatusUpdate()
		 */
		void DemonwarePlayerStatusUpdate(bdOnlineUserID userID, bdNChar8 *userName, bdUByte8 status);

		/// Game needs to call this function when we receive a session invite via demonware.
		/** PS3 and PC will store the invite with the friends data so when the friends are enumerated the invite data will be correct.
		 *	Game needs to derrive a class from bdLobbyEventHandler and register this class with bdLobbyService to start receiving demonware messages.
		 *	This function should be used when we receive bdLobbyEventHandler::onSessionInvite()
		 */
		void DemonwareSessionInvite(bdOnlineUserID senderID, char *senderName, bdSecurityID& secID, void *attachment, U32 attachmentSize);

		/// Game needs to call this function when we receive rich presence data from demonware.
		/** PS3 and PC will store the presence with the friends data so when the friends are enumerated the presence data will be correct.
		 *	Game needs to derrive a class from bdLobbyEventHandler and register this class with bdLobbyService to start receiving demonware messages.
		 *	This function should be used when we receive bdLobbyEventHandler::onRichPresenceUpdate()
		 */
		void DemonwareRichPresenceUpdate(bdOnlineUserID userID, char *userName, void *data, U32 dataSize);

		/// Are the two player currently both flagged as being in external chat.
		/** On Xbox360 if the 2 players are both in an XNE party and its set to party chat then voice data is sent
		 *	by the guide, we can stop the game chat encoding and sending voice to the remote player in this case as it's just wasted
		 *	bandwidth.
		 *	@param local local player UserIdentifier to test against.
		 *	@param remote remote player UserIdentifier to test against.
		 *	@return true if both supplied players are in the same NXE party and both are set to party voice chat.
		 */
		bool IsInExternalChat(const UserIdentifier& local, const UserIdentifier& remote);

		/// Tests to see if any local player is in an external party session.
		/** Applicable to xbox360 when the local players can be in an NXE party as well as game created sessions. 
		 *	@param mimRemote minumum number of remote players that must also be in the external party before function returns true.
		 *	Xbox306 TCR states if we are in the party with 1 other remote player we must send invites to this party and not to friends. 		
		 *	@return true if local player is in an external party and there is at least the number of remote players in that party as well.
		 */
		bool IsInExternalParty(const U8 minRemote = 1);

#if defined ( PLATFORM_PS3 ) 

		/// Has the console user got a valid NP account bound.
		/** We should only offer NP functionality (ie sign in) if the user has a valid NP account.
		 */
		bool NetworkPlatformAccountAvailable(void) const;

		/// Game should register a callback using sceNpManagerRegisterCallback() and call this function from it so we can take what data we need this callback is used for connection changes etc..
		void NpManagerCallback(S32 ev, S32 result, void *arg);

		/// Game should register for Np basic events (friends, presence, messaging etc) via its own callback function and call this function from within, the game can use sceNpBasicRegisterContextSensitiveHandler() to register for the handler on console startup.
		void NpBasicEventHandler(S32 ev, S32 retCode, uint32_t reqId, void *arg);
#endif

#if defined ( PLATFORM_PC )
		/// Add a friend to the local players friends list.
		/** Needs to be called on PC only.
		 *	@param userIndex the local players index of who wants to add the friend.
		 *	@param name the name of the friend we wish to add to our friends list.
		 *	@return true if the asynchronous process of adding a friend was started, if true then call ContinueAddFriend()
		 */
		bool StartAddFriend(const U32 userIndex, const char* name);

		/// Continues the asynchrouns task of adding a friend.
		/** Game must have successfully called StartAddFriend() before calling this. 
		 *	Game must also call Poll() while processing as the bulk of the logic happens in there. If we are using demonware we must also pump their lobby connection.
		 *	@param errorOccurred once this function returns true then this variable can be checked to see if the friend was added or not.
		 *	@return true when the asynchronous process has finished.
		 */
		bool ContinueAddFriend(bool& errorOccurred);

		/// Remove a friend from my friends list.
		/** Needs to be called on PC only. This only works if the friend has been accepted onto my full friends list ie: they are not on my pending list.
		 *	@param userIndex the local players index of who wants to remove the friend.
		 *	@param toRemove friend identifier who will be removed.
		 *	@return true if the asynchronous process of removing a friend was started, if true then call ContinueRemoveFriend()
		 */
		bool StartRemoveFriend(const U32 userIndex, const UserIdentifier& toRemove);

		/// Continues the asynchrouns task of removing a friend.
		/** Game must have successfully called StartRemoveFriend() before calling this. 
		 *	Game must also call Poll() while processing as the bulk of the logic happens in there. If we are using demonware we must also pump their lobby connection.
		 *	@param errorOccurred once this function returns true then this variable can be checked to see if the friend was removed or not.
		 *	@return true when the asynchronous process has finished.
		*/
		bool ContinueRemoveFriend(bool& errorOccurred);

		/// Set what user is currently authenticated to demonware.
		/** When we connect / disconect from demonware we need to inform the manager what user was authenticated
		 *	and in what state (online / offline etc). 
		 *	@param pName the name of the user who was authenticated to the demonware service or NULL if no player currently active.
		 *	to GetProfile() will not return any valid users.
		 *	@return true if the supplied data was set correctly.
		 */
		bool SetDemonwareAuthenticatedPlayerData(const char * pName);

		/// Let the profile manager know that the demonware lobby service has informed us that we have signed in on multiple computers usinng the same account.
		/** Next time we call Poll() the notification will be passed to the game.
		 */
		void DemonwareDuplicateLogon(void);
#endif 

	private:
		explicit C_Manager();
		virtual ~C_Manager();
		C_Manager(const C_Manager& original);				// private copy constructor
		C_Manager& operator = (const C_Manager rhs);		// private un coded assignment operator

		void ReadUsers(Notifications& notifications);

	private:
		static C_Manager				s_ProfileManager;
		U64								m_RefCounter;
		bool							m_Initialised;
		bool							m_SystemHudActive;
		U8								m_MaxUsers;
		FullProfile*					m_pProfileData;
		MT::Lock*						m_pCritSection;
		int								m_PrimaryPadNo;
		eServiceState					m_ActiveServiceState;
		eServiceState					m_LatestServiceState;
		S32								m_AgeRequirement;
		bool							m_GameInviteBeingSent;

#if defined (PLATFORM_PS3)
		U32								m_NumNpBasicMessagesWaiting;
		bool							m_InitialPresenceSent;
		bool							m_PresenceDataModified;
		Time::C_Time					m_LastPresenceUpdate;
		NetPlay::RichPresence*			m_pRichPresence;
		bool							m_ForcePresenceToServer;
		NpFriendsList*					m_pFriendsList;
		NpFriendsPool*					m_pFriendsPool;
		C_NpThreadedFriendEnumerator*	m_pThreadedFriendEnumerator;

		// for game invite
		bdRemoteTaskRef			m_GameInviteTask;						
		bool							m_CableState;
		void							CheckCableState( Notifications& notifications );
#elif defined (PLATFORM_PC)
		BaseProfile				m_DemonwareUser;

		bool					m_InitialPresenceSent;
		bool					m_PresenceDataModified;
		bool					m_DuplicateLogon;
		Time::C_Time			m_LastPresenceUpdate;
		NetPlay::RichPresence*	m_pRichPresence;
		bool					m_ForcePresenceToServer;

		bool					m_PresenceUpdateActive;
		bdRemoteTaskRef 		m_PresenceUpdateTask;

		bdRemoteTaskRef 		m_GameInviteTask;
		
		bool					m_GameInviteBeingProcessed;
		bdRemoteTaskRef 		m_GameInviteProcessTask;

		bool					m_FriendshipProposalActive;
		bdRemoteTaskRef 		m_FriendshipProposalTask;

		bool					m_FriendshipRemovalActive;
		bdRemoteTaskRef 		m_FriendshipRemovalTask;

		PcFriendsList*			m_pFriendsList;
		PcFriendsPool*			m_pFriendsPool;
#endif

	};
}; // namespace SystemProfile

#endif
