// ***************************************************************
//  Session   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Online session management
// ***************************************************************

#ifndef SESSION_H
#define SESSION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/Systemheaders.h>
#include <btl/containers/vector.h>
#include <Network/SessionCommandNotification.h>
#include <SystemProfile/SystemProfile.h>
#include <Network/SystemHeaders.h>
#include <bdLobby/bdMatchMaking/bdSessionID.h>

// forward declerations from other namespaces
namespace SysProfile { struct GameInvite; }
namespace Scoreboard { class C_Writer; }

namespace NetPlay
{
	// forward declarations from the Network namespace
	struct ArbitrationNonce;
	struct SessionSecurity;
	struct MigrationInfo;
	struct SessionParameter;
	class C_SessionCommand;
	class C_SessionEnumerator;
	class C_DemonwareSession;

	// or together and pass into Create() in the sessionFlags parameter
#if defined(LIVE_ON_WINDOWS)
	const  U32 SESSION_CREATE_HOST                          =	XSESSION_CREATE_HOST;							// Set only on the host of a multi player session. The user who sets the host flag is the user that interacts with Live
	const  U32 SESSION_CREATE_USES_PRESENCE                 =	XSESSION_CREATE_USES_PRESENCE;					// Session is used across games to keep players together. Advertises state via Presence
	const  U32 SESSION_CREATE_USES_STATS                    =	XSESSION_CREATE_USES_STATS;						// Session is used for stats tracking
	const  U32 SESSION_CREATE_USES_MATCHMAKING				=	XSESSION_CREATE_USES_MATCHMAKING;				// Session is advertised in matchmaking for searching
	const  U32 SESSION_CREATE_USES_ARBITRATION              =	XSESSION_CREATE_USES_ARBITRATION;				// Session stats are arbitrated (and therefore tracked for everyone in the game)
	const  U32 SESSION_CREATE_USES_PEER_NETWORK             =	XSESSION_CREATE_USES_PEER_NETWORK;				// Secure key is registered and parental control settings are enforced
	const  U32 SESSION_CREATE_INVITES_DISABLED              =	XSESSION_CREATE_INVITES_DISABLED;				// Game Invites cannot be sent by the HUD for this session
	const  U32 SESSION_CREATE_JOIN_VIA_PRESENCE_DISABLED	=	XSESSION_CREATE_JOIN_VIA_PRESENCE_DISABLED; 	// Session will not ever be displayed as joinable via Presence
	const  U32 SESSION_CREATE_JOIN_IN_PROGRESS_DISABLED		=	XSESSION_CREATE_JOIN_IN_PROGRESS_DISABLED;		// Session will not be joinable between Start and End
	const  U32 SESSION_CREATE_JOIN_VIA_PRESENCE_FRIENDS_ONLY=	XSESSION_CREATE_JOIN_VIA_PRESENCE_FRIENDS_ONLY;	// Session will only be joinable on presence by friends of the host
#else
	const  U32 SESSION_CREATE_HOST                          =	0x00000001;  // Set only on the host of a multi player session. The user who sets the host flag is the user that interacts with Live
	const  U32 SESSION_CREATE_USES_PRESENCE                 =	0x00000002;  // Session is used across games to keep players together. Advertises state via Presence
	const  U32 SESSION_CREATE_USES_STATS                    =	0x00000004;  // Session is used for stats tracking
	const  U32 SESSION_CREATE_USES_MATCHMAKING				=	0x00000008;  // Session is advertised in matchmaking for searching
	const  U32 SESSION_CREATE_USES_ARBITRATION              =	0x00000010;  // Session stats are arbitrated (and therefore tracked for everyone in the game)
	const  U32 SESSION_CREATE_USES_PEER_NETWORK             =	0x00000020;  // Secure key is registered and parental control settings are enforced
	const  U32 SESSION_CREATE_INVITES_DISABLED              =	0x00000040;  // Game Invites cannot be sent by the HUD for this session
	const  U32 SESSION_CREATE_JOIN_VIA_PRESENCE_DISABLED	=	0x00000080;  // Session will not ever be displayed as joinable via Presence
	const  U32 SESSION_CREATE_JOIN_IN_PROGRESS_DISABLED		=	0x00000100;  // Session will not be joinable between Start and End#endif
	const  U32 SESSION_CREATE_JOIN_VIA_PRESENCE_FRIENDS_ONLY=	0x00000200;	// Session will only be joinable on presence by friends of the host
#endif

	// common sessionFlags for Create()
	const U32 SESSION_CREATE_SINGLEPLAYER_WITH_STATS		=	(SESSION_CREATE_USES_PRESENCE | SESSION_CREATE_USES_STATS | SESSION_CREATE_INVITES_DISABLED | SESSION_CREATE_JOIN_VIA_PRESENCE_DISABLED | SESSION_CREATE_JOIN_IN_PROGRESS_DISABLED);
	const U32 SESSION_CREATE_ONLINE_MULTIPLAYER_STANDARD    =	(SESSION_CREATE_USES_PRESENCE | SESSION_CREATE_USES_STATS | SESSION_CREATE_USES_MATCHMAKING | SESSION_CREATE_USES_PEER_NETWORK);
	const U32 SESSION_CREATE_ONLINE_MULTIPLAYER_RANKED      =	(SESSION_CREATE_ONLINE_MULTIPLAYER_STANDARD | SESSION_CREATE_USES_ARBITRATION);
	const U32 SESSION_CREATE_LAN							=	(SESSION_CREATE_USES_PEER_NETWORK);
	const U32 SESSION_CREATE_GROUP_LOBBY					=	SESSION_CREATE_ONLINE_MULTIPLAYER_STANDARD;//(SESSION_CREATE_USES_PRESENCE | SESSION_CREATE_USES_PEER_NETWORK);
	const U32 SESSION_CREATE_GROUP_LOBBY_NO_PRESENCE		=	(SESSION_CREATE_USES_PEER_NETWORK);

	/// Object for controlling a single on line game session.
	/** Can me used for multiplayer and if on line stats (ie: scoreboards) are required single player sessions also.
	 *	If the game passes in a SessionCommandNotification pointer via any of the functions then this MUST stay valid until either the command completes 
	 *	or the session is deleted.
	 *	When using demonware as the service provider we must configure the server to accept the security key (a data blob of BD_SECURITY_KEY_LENGTH size) as the first parameter in the
	 *	web imp matchmaking attributes list. When we create a web imp query we also need to return the security key (a data blob of BD_SECURITY_KEY_LENGTH size) as the first return parameter.
	 *	When we supply a list of session parameters to the C_Session class the first parameter in this list MUST be the game mode configured as a context, @see bdMatchMakingInfo. 
	 *	This has a similar use to X_CONTEXT_GAME_MODE on xbox 360 on the servers, we can segment sessions based on system defined a game mode field.
	 *	When changing a single matchmaking parameter via Modify() you still need to supply the complete parameter list to update the server with, this helps with cross plaform differences. 
	 */
	class C_Session
	{
	public:
		friend class Scoreboard::C_Writer;
		friend class C_SessionDeletion;
		friend class C_PlayerSkillWriter;

		/// Structure that defines the slot configuration of a session
		/** The game needs to manage which players belong to which slot type
		 */
		struct SlotConfig
		{
			SlotConfig() : m_MaxPublic(0), m_MaxPrivate(0), m_PublicInUse(0), m_PrivateInUse(0)
			{}
			
			// do the slot counts addup
			bool Validate(const U32 maxPlayers) const
			{
				const U32 total = m_MaxPublic+m_MaxPrivate;
				if (0 == total || total > maxPlayers)
					return false;

				const S32 pub = m_MaxPublic - m_PublicInUse;
				const S32 pri = m_MaxPrivate - m_PrivateInUse;

				if (pub < 0 || pri < 0)
					return false;
				return true;
			}

			U16 m_MaxPublic;			// Session will be crated with this number of Public slots
			U16 m_MaxPrivate;			// Session will be crated with this number of Private slots
			U16 m_PublicInUse;			// How many Public slots are actualyl in use by players
			U16 m_PrivateInUse;			// How many Private slots are actualyl in use by players
		};

		/// Structure that defines a single user that will be added to the session
		/** This is used as the base for the SessionUserList, which is used when adding / removing multiple players to the session
		 *	via a single API call.  
		 */
		struct SessionUser
		{
			SessionUser(const SysProfile::UserIdentifier& u, bool p ) : m_User(u), m_PublicSlot(p)
			{}
			SessionUser() : m_PublicSlot(false)
			{}
			SysProfile::UserIdentifier	m_User;
			bool						m_PublicSlot;
		};

		/// defines the maximum number of players we can have in a single api call via AddPlayers() or RemovePlayers().
		typedef Btl::Vector<SessionUser, Btl::VectorParameters_Static<NETPLAY_SESSION_MAX_PLAYERS_ONE_OPERATION> > SessionUserList;

		/// list of different states the session can be in.
		typedef enum
		{
			eStateNotActive=0,			/**< not active */
			eStateCreating,				/**< in process of being created */
			eStateRunning,				/**< running, most probably in lobby */
			eStatePlaying,				/**< game play active */
			eStateError,				/**< error state, no commands will be processed internally */
		}eState;

		explicit C_Session();
		virtual ~C_Session();

		/// Initialize all dynamic data, call once before using another function within this object.
		/** @return true if everything was correctly setup or false if not.
		 */
		bool Initialise(const U32 maxPlayers, const U32 commandQueueSize);

		/// returns the current state of the session.
		eState SessionState(void) const {return m_SessionState;}

		/// Call once per frame to continue asynchronous processing.
		/** @return true if all ok or false if we encountered an error.
		 */
		bool Poll(void);

		/// Attempt to create a new Session, Asynchronous command.
		/**	@param userIndex local user who will own the session.
		 *	@param sessionFlags flags that determine what properties the session will be created with. See SESSION_CREATE_xxx defines.
		 *	@param pSessionParameters pointer to a list of session parameters that define the session to be created. On PS3 and PC the first 
		 *	parameter in the list must be configured as a context and will be used to specify the game type on the demonware session.  
		 *	@param numSessionParameters number of SesionParameter structures pointed to by pSessionParameters.
		 *	@param slotConfig slot configuration we wish to create the session with.		
		 *	@return true if an internal command was created to create the session.
		 */
		bool Create(const U8 userIndex, const U32 sessionFlags, const SessionParameter* pSessionParameters, const U32 numSessionParameters, const SlotConfig& slotConfig);

		/// Attempt to create a new Session when joining an already running session, Asynchronous command.
		/** @param userIndex local user who will own the session.
		*	@param sessionFlags flags that determine what properties the session will be created with. See SESSION_CREATE_xxx defines.
		*	@param pSessionParameters pointer to a list of session parameters that define the session to be created. On PS3 and PC the first 
		*	parameter in the list must be configured as a context and will be used to specify the game type on the demonware session. 		
		*	@param numSessionParameters number of SesionParameter structures pointed to by pSessionParameters.
		*	@param securityData security data of the session that we will join.
		*	@param slotConfig slot configuration we wish to create the session with.
		*	@return true if an internal command was created to create the session.
		*/
		bool CreatePeer(const U8 userIndex, const U32 sessionFlags, const SessionParameter* pSessionParameters, const U32 numSessionParameters, const SessionSecurity& securityData, const SlotConfig& slotConfig);

		/// Attempt to create a new Session when joining an already running session, Asynchronous command.
		/** @param userIndex local user who will own the session.
		 *	@param sessionFlags flags that determine what properties the session will be created with. See SESSION_CREATE_xxx defines.
		 *	@param pSessionParameters pointer to a list of session parameters that define the session to be created. On PS3 and PC the first 
		 *	parameter in the list must be configured as a context and will be used to specify the game type on the demonware session. 		 
		 *	@param numSessionParameters number of SesionParameter structures pointed to by pSessionParameters.
		 *	@param availableSessions C_SesionEnumerator object that contains session from the matching server.
		 *	@param sessionIndexToJoin index of the session within availableSesions which we will attempt to join.
		 *	@param slotConfig slot configuration we wish to create the session with.
		 *	@return true if an internal command was created to create the session.
		 */
		bool CreatePeer(const U8 userIndex, const U32 sessionFlags, const SessionParameter* pSessionParameters, const U32 numSessionParameters, const C_SessionEnumerator& availableSessions, const U32 sessionIndexToJoin, const SlotConfig& slotConfig);

		/// Attempt to create a new Session when joining an already running session, Asynchronous command.
		/** @param userIndex local user who will own the session.
		 *	@param sessionFlags flags that determine what properties the session will be created with. See SESSION_CREATE_xxx defines.
		 *	@param pSessionParameters pointer to a list of session parameters that define the session to be created. On PS3 and PC the first 
		 *	parameter in the list must be configured as a context and will be used to specify the game type on the demonware session. 
		 *	@param numSessionParameters number of SesionParameter structures pointed to by pSessionParameters.
		 *	@param inviteData game invite data received about the session to join.
		 *	@param slotConfig slot configuration we wish to create the session with.
		 *	@return true if an internal command was created to create the session.
		 */
		bool CreatePeer(const U8 userIndex, const U32 sessionFlags, const SessionParameter* pSessionParameters, const U32 numSessionParameters, SysProfile::GameInvite& inviteData, const SlotConfig& slotConfig);

		/// Try to enable Quality of service for a secure network session.
		/**	@param qosBandwidth how much bandwidth do we set aside for QoS packets.
		 *	@param pData pointer to some data block that gets appended onto the QoS packets.
		 *	@param dataSize how many bytes this QoS data block actually is.
		 *	@return true if QoS was enabled or false if an error occurred.
		 */
		bool EnableQos(const U32 qosBandwidth, U8* pData, const U32 dataSize);

		/// Attempt to disable Quality of Service for a specific network session.
		/** @return true if QoS was disabled for session.
		 */
		bool DisableQos(void);
	
		/// Is QoS currently enabled for this matching session.
		/** Demonware only allows for QoS data to be active for a single matching session at one time.
		 */
		bool QosEnabled(void) const { return m_QoSEnabled; }

		/// Attempt to change the total number of public and private slots available to the session.
		/** Any players currently added to the sesion cant have their slot type changed.
		 *	@param modifiedSessionFlags new session parameters that may have been modified depending on game mode when player joined, if 0 current session flags are not modified.
		 *	@param slotConfig slot configuration we wish to modify the session to.
		 *	@param pSessionParameters pointer to a list of session parameters, this should contain all session parameters not only the ones that were modified.
		 *	@param numSessionParameters number of SesionParameter structures pointed to by pSessionParameters.
		 *	@param pCompleteNotification this is so the game code can get notified when the actual session modification command has been executed. pointer must stay valid until command completes or session is deleted.
		 *	@return true if the command was added to the queue or false if an error occurred.
		 */
		bool Modify(const U32 modifiedSessionFlags, const SlotConfig& slotConfig, const SessionParameter* pSessionParameters, const U32 numSessionParameters, SessionCommandNotification* pCompleteNotification = NULL);

		/// Starts the process of deleting the active session.
		void Delete(void);

		/// Attempt to add a player to the session.
		/** If you need to add multiple player to the session then use AddPlayers() as its more optimal.
		 *	@param player SysProfile::UserIdentifier or local or remote player to add to the session.
		 *	@param usePublicSlot true if this player should occupy a public slot, if false then player will take a private slot.
		 *	@param pCompleteNotification this is so the game code can get notified when the actual add player command has been executed. On xbox360 the game can't send invites until the local players have been added into the active session. pointer must stay valid until command completes or session is deleted.
		 *	@return true if it was possible to add this command to the queue.
		 */
		bool AddPlayer(const SysProfile::UserIdentifier& player, const bool usePublicSlot, SessionCommandNotification* pCompleteNotification = NULL);

		/// Attempt to add multiple player to the session. 
		/**	@param playerList the players that you wish to be added to the session.
		 *	@param pCompleteNotification this is so the game code can get notified when the actual add player command has been executed. On xbox360 the game can't send invites until the local players have been added into the active session. pointer must stay valid until command completes or session is deleted.
		 *	@return true if it was possible to add this command to the queue.
		 */
		bool AddPlayers(const SessionUserList& playerList, SessionCommandNotification* pCompleteNotification = NULL);

		/// Attempt to remove a player from the session.
		/** If you need to remove multiple player from the session then use RemovePlayers() as its more optimal.
		 *	@param pCompleteNotification this is so the game code can get notified when the actual remove player command has been executed. pointer must stay valid until command completes or session is deleted.
		 *	@param player SysProfile::UserIdentifier or local or remote player to remove from the session
		 *	@return true if it was possible to add this command to the queue.
		 */
		bool RemovePlayer(const SysProfile::UserIdentifier& player, SessionCommandNotification* pCompleteNotification = NULL);

		/// Attempt to remove multiple player from the session. 
		/**	@param playerList the players that you wish to be removed from the session.
		 *	@param pCompleteNotification this is so the game code can get notified when the actual remove players command has been executed. pointer must stay valid until command completes or session is deleted.
		 *	@return true if it was possible to add this command to the queue.
		 */
		bool RemovePlayers(const SessionUserList& playerList, SessionCommandNotification* pCompleteNotification = NULL);

		/// If the original host has left the session then one of the peers can migrate himself to be the ownner (ie: new host) of the session.
		/** Process is as follows	:	1) Game decides original host is left and decides on which peer should take ownership.
		 *							:	2) This peer calls Migrate() and waits for the boolean pointed to by pCompleteNotification to go true
		 *							:	3) This peer then gets the new secure key and sends to all other peers via GetMigrationInfo()
		 *							:	4) All other peers call then MigratePeer() this time passing in the supplied migration info
		 *	@param pCompleteNotification pointer to a Notification structure that will be modified when this command has completed. pointer must stay valid until command completes or session is deleted.
		 *	@param pSessionParameters pointer to a list of session parameters that define the session to be created.
		 *	@param numSessionParameters number of SesionParameter structures pointed to by pSessionParameters.
		 *	@param slotConfig slot configuration we wish to Migrate the session to.
		 *	@return true if the command was successfully added to the internal queue for processing.
		 */
		bool Migrate(SessionCommandNotification* pCompleteNotification, const SessionParameter* pSessionParameters, const U32 numSessionParameters, const SlotConfig& slotConfig);

		/// Once a new host has migrated via Migtate() he can use this function to get the data required to send to the peers so they can call MigtatePeer()
		/**	@return true if outInfo is valid.
		 */
		bool GetMigrationInfo(MigrationInfo& outInfo);

		/// If the original host has left the session then one of the peers can migrate himself to be the ownner (ie: new host) of the session.
		/** Process is as follows		1) Game decides original host is left and decides on which peer should take ownership.
		 *								2) This peer calls Migrate() and waits for the boolean pointed to by pCompleteNotification to go true.
	 	 *								3) This peer then gets the new secure key and sends to all other peers.
		 *								4) All other peers call then MigratePeer() this time passing in the supplied key as pNewSessionKey.
		 *	@param migrationInfo migration session data received from the new host.
		 *	@param pCompleteNotification pointer to a SessionCommandNotification structure that will be modified when this command has completed. pointer must stay valid until command completes or session is deleted.
		 *	@param slotConfig slot configuration we wish to Migrate the session to.
		 *	@return true of the command was successfully added to the internal queue for processing.
		 */
		bool MigratePeer(const MigrationInfo& migrationInfo, SessionCommandNotification* pCompleteNotification, const SlotConfig& slotConfig);

		/// Given a list of players in the session this will update the sessions trueSkill to that of an average to the specified players
		/**	Only for Xbox360
		 *	NOTE : Only 1 of these commands can be added into the command queue at any one time, attempting to add another before the previous one has completed will cause this function to fail
		 *	@param pCompleteNotification pointer to a Notification structure that will be modified when this command has completed. pointer must stay valid until command completes or session is deleted.
		 *	@param numPlayers how many players does pPlayers point to
		 *	@param pPlayers pointer to an array of users who are to be considered in the trueSkill of the session.
		 *	@return true of the   command was successfully added to the internal queue for processing.
		 */
		bool ModifySkill(SessionCommandNotification* pCompleteNotification, const U8 numPlayers, const SysProfile::UserIdentifier* pPlayers);

		/// Attempt to start registering for arbitration.
		/**	Arbitration process is as follows	1) Host gets his ArbitrationNonce from the session security data. @see GetSecurityInfo()
		 *										2: Hosts ArbitrationNonce is sent to peers.
		 *										3) Peers call this function and wait for completion. Once complete they inform the host thay have finished registration.
		 *										4) Once all peers have finished registration (or a time out period has passed) the host also calls this function.
		 *										5) Host waits for completion then checks all peers to be sure they were accepted for arbitration. @see CheckPlayerIsInArbitrationList().		
		 *										6) Host disconnects all peers who didn't register correctly. Game starts.
		 *	@param arbitrationNonce arbitrationNonce data supplied by the host.
		 *	@param pCompleteNotification pointer to a Notification structure that will be set once the asynchronous arbitration request has finished. pointer must stay valid until command completes or session is deleted.
		 *	@return true if the command has been added to the Queue or false if not.
		 */
		bool RegisterForArbitration(const ArbitrationNonce& arbitrationNonce, SessionCommandNotification* pCompleteNotification);

		/// Check a specific player registeded for arbitration.
		/** Once we all consoles have registered for arbitration the Host can iterate through the arbitration results list and check
		 *	what clients successfully registered. Any clients who did not register should be removed from the session before starting to play.
		 *	@param playeridentifier specifie player (local or remote) to check for in the arbitration results.
		 *	@return true if the specified player has successfully registered for arbitration or false if not.
		 */
		bool CheckPlayerIsInArbitrationList(const SysProfile::UserIdentifier& playeridentifier);

		/// Release dynamic arbitration registration buffer.
		/** Once players have registered for arbitration and the host has verified what players can start play then all players can call this to
		 *	to release the internal dynamic buffers. If this function is not called then the buffers will be released when play starts or the session is deleted.
		 *	@return true if the buffers where succesfully released or false if not.
		 */
		bool ReleaseArbitrationRegistrationBuffer(void);

		/// Stop addition / removal of players from the session.
		/** Once an arebitrated session has started playing the current players are locked, so if they quit the game we can still post results to the server.
		 *	@return true if the function succeeded to lock the current players or false if not.
		 */
		bool LockCurrentPlayers(const bool lock);

		/// Attempt to put the session into Playing round state, this should only be called when the game transitions from the lobby to the actual game play.
		/** @return true if command was added to queue or false if an error occurred.
		*/
		bool StartPlay(void);

		/// Attempt to end a game play session. This should only be called one all game play has ended and the player is returning back to the lobby.
		/**	For every 1 call of StartPlay() you need a corresponding call to EndPlay()
		 *	@return true of the command was added to the queue or false if an error occurred.
		 */
		bool EndPlay(void);

		/// Once session is running we can get access to the security data.
		/**	@return true if outKey is valid on function exit.
		 */
		bool GetSecurityInfo(SessionSecurity& outKey);

		/// An Idle session means there is no server requests active or pending.
		/** @return true if no server commands are active or pending.
		 */
		bool IsIdle(void);

		/// Returns the current parameters used when the session was created or last modified.
		U32 GetSessionParameters(void) const {return m_SessionParameters;}

		/// After completing a session we can show a Player review UI to allow the local player to post feedback
		/** Only required on Xbox360 and PC Games for windows live.
		 *	@param userindex user index of local player who wants to post feedback.
		 *	@param reviewTarget user data for the remote player who we will be posting feedback about.
		 */
		void ShowPlayerReviewUI(const U8 userindex, const SysProfile::UserIdentifier& reviewTarget);

	private:
		C_Session(const C_Session& original);				// private copy constructor
		C_Session& operator = (const C_Session rhs);		// private un coded assignment operator

		bool SetParameter(const U8 userIndex, const SessionParameter& data);

		void CleanUp(void);

	private:
		bool				m_Initialised;				// has object been initialized, ie. all dynamic resources allocated.
		eState				m_SessionState;				// what state is the session currently in.

		U32					m_SessionParameters;		// What parameters did we create the session with.. this holds all the SESSION_CREATE_xxx bits
		U32					m_MaxPlayers;				// how many players can we track, we can create sessions with less players though..
		U32					m_PublicSlots;				// how many public players can join the session.
		U32					m_PrivateSlots;				// how many private players can join the session.
		U32					m_UserIndex;				// Which local user is in charge of the session.
		U32					m_CommandStorageSize;		// how many commands have we allocated for normal commands
		bool				m_SessionCreated;			// goes true when the session has been successfully created; 
		bool				m_Locked;					// after arbitration registration session is locked for adding or removing players.
		bool				m_QoSEnabled;				// has the Quality of Service listener been enabled for this session ?
				
		// for normal command data
		C_SessionCommand*	m_pCommandStorage;			// holds an array of command objects..
		void*				m_pCommandPool;				// linked list of available command objects
		void*				m_pCommandList;				// linked list of all commands currently awaiting processing.

		bool				m_Migrating;
		// platform specific data..
#if defined(LIVE_ON_WINDOWS)
		HANDLE				m_SessionHandle;			// These gets initialized when we create the session.
		XSESSION_INFO		m_SessionInfo;
		XSESSION_INFO		m_MigrationInfo;
		U64					m_SessionNonce;
		U8*					m_pArbitrationRegistrationResults;
		XUID*				m_pSkillModifiers;
		
#elif defined(PLATFORM_PS3) || defined(PLATFORM_PC)
		C_DemonwareSession*	m_pDemonwareSession;
		bdSessionID			m_SessionId;
#endif
	};
}



#endif
