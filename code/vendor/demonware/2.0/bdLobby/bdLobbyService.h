// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Constitutes the main entry point for all of the DemonWare Lobby Services.
// This class provides access to the individual services and maintains the connection
// to the gateway.

#ifndef BD_LOBBY_SERVICES_H
#define BD_LOBBY_SERVICES_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyConnection.h>
#include <bdLobby/bdLobbyConnectionListener.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyEventHandler.h>

#define BD_PROTOCOL_VERSION (200)

BD_REFERENCE(bdAuthInfo);
BD_REFERENCE(bdLobbyConnection);

class bdMatchMaking;
class bdStats;
class bdProfiles;
class bdTeams;
class bdFriends;
class bdStorage;
class bdChannelChat;
class bdTitleUtilities;
class bdKeyArchive;
class bdRemoteTaskManager;
class bdMessaging;
class bdContentUnlock;
class bdCounter;
class bdGroup;
class bdContentStreaming;
class bdTags;
class bdVoteRank;
class bdTwitter;
class bdFacebook;
#ifdef BD_PLATFORM_IPHONE
class bdApplePushNotification;
#endif
class bdLinkCode;
class bdDML;

#undef connect

/// Constitutes the main entry point for all of the DemonWare Lobby Services.
/// This class provides access to the individual services and maintains the connection
/// to the gateway.
class bdLobbyService :public bdLobbyConnectionListener
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor. 
		bdLobbyService();

		/// Destructor.
		~bdLobbyService();
		
#if (!defined BD_PLATFORM_XBOX) && (!defined BD_PLATFORM_XENON)

		/// Connects to the Lobby Service Gateway.
		/// \param lobbyServiceAddr[in]		The address of the LSG
		/// \param authInfo[in]				Authentication ticket
		bdBool connect(	bdAddr lobbyServiceAddr, 
						const bdAuthInfo *authInfo,
						const bdBool useEncryption = true);

		/// Connects to the Lobby Service Gateway.
		/// \param lobbyServiceAddr[in]	The address of the LSG
		/// \param authInfo[in]			Authentication ticket, which should be 
		///								obtained from one of the authorize* 
		///								functions in bdAuthService.
		/// \param sendBuffer[in]		A pointer to a chunk of memory to be 
		///								used as send buffer.
		/// \param sendBufSize[in]		size of the send buffer
		/// \param recvBuffer[in]		A pointer to a chunk of memory to be 
		///								used as receive buffer.
		/// \param recvBufSize[in]		size of the receive buffer
		/// \param useEncryption[in]	Flag to indicate whether the connection is to be encrypted.
		bdBool connect(bdAddr lobbyServiceAddr, 
					   const bdAuthInfo *authInfo,
                       void *sendBuffer,
					   bdUWord sendBufSize,
					   void *recvBuffer, 
					   bdUWord recvBufSize,
					   const bdBool useEncryption = true);

#endif
#if (defined BD_PLATFORM_XBOX) && (!defined BD_PLATFORM_XENON)
		/// Xbox-specific version of the connect call, for use when accessing
		/// the Lobby Service Gateway via Microsoft's Live Server Platform (LSP).
		/// \note XNetRegisterKey must be called before connect
		/// is called.
		/// \param	lobbyServiceAddr[in]	The address of the LSG.
		/// \param	id[in]					The security ID of the key previously
		///									registered for accessing LSP.
		/// \param	authInfo[in]			A ticket constructed locally, instead
		///									of via the bdAuthService.
		bdBool connect(	bdCommonAddrRef lobbyServiceAddr,
						bdSecurityID &id,
						const bdAuthInfo *authInfo);

#elif defined BD_PLATFORM_XENON
		/// Xenon-specific version of the connect call, for use when accessing
		/// the Lobby Service Gateway via Microsoft's Live Server Platform (LSP).
		/// \note XNetRegisterKey must be called before connect
		/// is called.
		/// \param	lobbyServiceAddr[in]	The address of the LSG.
		/// \param	authInfo[in]			A ticket constructed locally, instead
		///									of via the bdAuthService.
		bdBool connect(	bdAddr lobbyServiceAddr,
						const bdAuthInfo *authInfo);


		/// Xenon-specific version of the connect call, for use when accessing
		/// the Lobby Service Gateway via Microsoft's Live Server Platform (LSP).
		/// \note XNetRegisterKey must be called before connect
		/// is called.
		/// \param	lobbyServiceAddr[in]	The address of the LSG.
		/// \param	id[in]					The security ID of the key previously
		///									registered for accessing LSP.
		/// \param	authInfo[in]			A ticket constructed locally, instead
		///									of via the bdAuthService.
		/// deprecated - use connect(bdAddr,bdAuthInfo*) instead
		bdBool connect(	bdAddr lobbyServiceAddr,
						bdSecurityID &id,
						const bdAuthInfo *authInfo);
#endif 
		/// Disconnects from the Lobby Services Gateway.
		void disconnect();

		/// Pumps the connection to the lobby server. This functions needs to be called at regular intervals
		/// to maintain the connection to the lobby server and to advance any outstanding tasks.
		void pump();

		/// Gets the status of the underlying connection to the lobby server.
		bdLobbyConnection::Status getStatus();

		/// Register an event handler for intercepting lobby events.
		/// \params eventHandler[in] The event handler to be registered. Set to NULL if no further events
		///							 should be handled.
		void registerEventHandler(bdLobbyEventHandler *eventHandler);

		/// Gets an instance of the messaging lobby service.
		bdMessaging* getMessaging();

		/// Gets an instance of the matchmaking lobby service.
		bdMatchMaking* getMatchMaking();

		/// Gets an instance of the profiles lobby service.
		bdProfiles* getProfiles();

		/// Gets an instance of the stats lobby service.
		bdStats* getStats();

		/// Gets an instance of the teams lobby service.
		bdTeams* getTeams();

		/// Gets an instance of the friends lobby service.
		bdFriends* getFriends();

		/// Gets an instance of the storage lobby service.
		bdStorage* getStorage();

		/// Gets an instance of the storage lobby service.
		bdContentUnlock* getContentUnlock();

		// Gets an instance of the title utilities lobby service.
		bdTitleUtilities* getTitleUtilities();

		// Gets an instance of the key archive lobby service.
		bdKeyArchive* getKeyArchive();

		// Gets an instance of the counter lobby service.
		bdCounter* getCounter();

		// Gets an instance of the group lobby service.
		bdGroup* getGroup();

		/// Gets an instance of the streaming lobby service.
		bdContentStreaming* getContentStreaming();

		/// Removes the ContentStreaming manager and releases associated resources
		void deleteContentStreaming();

		// Gets and instance of the tag lobby service.
		bdTags* getTags();

		// Gets an instance of the VoteRank lobby service.
		bdVoteRank* getVoteRank();

		// Gets an instance of the Twitter lobby service.
		bdTwitter* getTwitter();

		// Gets an instance of the Facebook lobby service.
		bdFacebook* getFacebook();
		
#ifdef BD_PLATFORM_IPHONE
		// Gets an instance of the ApplePushNotification lobby service.
		bdApplePushNotification* getApplePushNotification();
#endif

		// Gets an instance of the LinkCode lobby service.
		bdLinkCode* getLinkCode();

		// Gets an instance of the DML lobby service.
		bdDML* getDML();

		/// Gets the lobby service task manager. Useful for custom services.
		bdRemoteTaskManager* getTaskManager();

		/// Gets the last error code
		bdUInt getErrorCode();


	protected:

		friend class bdLobbyConnection;

		/// Event handler for connection related events
		void onConnect(bdLobbyConnectionRef connection);

		/// Event handler for connection related events
		void onDisconnect(bdLobbyConnectionRef connection);

		void cleanup();

		void handlePushMessage(bdByteBufferRef message);

	protected:

		bdRemoteTaskManager *m_taskManager;

		/// An instance of the user profiles lobby service
		bdProfiles *m_profiles;

		/// An instance of the messaging lobby service
		bdMessaging *m_messaging;

		/// An instance of the matchmaking lobby service
		bdMatchMaking *m_matchMaking;

		/// An instance of the stats lobby service
		bdStats *m_statsManager;

		/// An instance of the friends lobby service
		bdFriends *m_friendsManager;

		/// An instance of the teams lobby service
		bdTeams* m_teamsManager;

		/// An instance of the storage lobby service
		bdStorage *m_storageManager;

		/// An instance of the content unlock service
		bdContentUnlock *m_contentUnlockManager;

		/// An instance of the title utilities lobby service
		bdTitleUtilities *m_titleUtilitiesManager;

		/// An instance of the key archive lobby service
		bdKeyArchive *m_keyArchive;

		/// An instance of the counter lobby service
		bdCounter *m_counter;

		/// An instance of the group lobby service
		bdGroup *m_group;
		/// An instance of the streaming lobby service
		bdContentStreaming *m_contentStreamingManager;

		/// An instance of the tag lobby service.
		bdTags *m_tags;

		/// An instance of the storage lobby service
		bdVoteRank *m_voteRankManager;

		/// An instance of the storage lobby service
		bdTwitter *m_twitter;

		/// An instance of the storage lobby service
		bdFacebook *m_facebook;

#ifdef BD_PLATFORM_IPHONE
		/// An instance of the apple push notification service
		bdApplePushNotification *m_applePushNotification;
#endif

		/// An instance of the LinkCode lobby service
		bdLinkCode *m_linkCode;

		/// An instance of the DML lobby service
		bdDML *m_DML;

		/// Title ID
		bdTitleID m_titleID;

		bdLobbyEventHandler *m_eventHandler;

		/// The connection to the Lobby Services Gateway
		bdLobbyConnectionRef m_lobbyConnection;

		bdAuthInfo m_authInfo;

		bdBool m_encryptedConnection;

		bdUInt	m_errorCode;
		
};


#endif //BD_LOBBY_SERVICES_H
