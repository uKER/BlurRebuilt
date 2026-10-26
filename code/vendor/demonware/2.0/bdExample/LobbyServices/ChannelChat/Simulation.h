// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef SIMULATION_H
#define SIMULATION_H

#include <bdExample/bdExampleCommonLobby/bdExampleCommonLobby.h>

#define MAX_NUM_DISPLAY_CHANNELS (10u)
#define MAX_NUM_DISPLAY_MEMBERS (10u)
#define MAX_NUM_CHANNELS (5)

class Simulation : public bdDemo, public bdLobbyEventHandler
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		Simulation();

		~Simulation();

		virtual void onChatChannelMessage(bdUInt64 channelID,
										  bdOnlineUserID senderID,
										  bdNChar8 *senderName,
                                          void *message,
                                          bdUInt messageSize);

		virtual void onChatChannelUpdate(bdUInt64 channelID,
										 bdOnlineUserID userID,
										 bdNChar8 *userName,
										 bdUByte8 status);

		virtual bdBool init(bdInt argc,
							bdNChar8* argv[]);

		virtual bdBool step(const bdFloat32 time);
		
		virtual void quit();

	protected:

		enum Task
		{
			START_NETWORK,			
			CHECK_DNS,
			CREATE_PLAYER1,
			CREATE_PLAYER2,
			AUTHENTICATE_PLAYER1,
			AUTHENTICATE_PLAYER2,			
			LOBBY_CONNECT,
			GET_CHANNEL_LIST,
			CREATE_CHANNEL,
			SUBSCRIBE_TO_CHANNEL,
			SEND_TO_CHANNEL,
			GET_CHANNEL_MEMBERS,
			WHISPER_TO_MEMBER,
			UNSUBSCRIBE_FROM_CHANNEL,
			LOBBY_DISCONNECT,
			STOP_NETWORK
		};

		void startTask();

		void pumpTask();

		void displayTaskResult();

		// Network startup

		void startNetworkDescription(bdString& description);

		void startNetwork();

		void pumpStartNetwork();

		// DNS lookup

		void checkDNSDescription(bdString& description);

		void startCheckDNS();

		void pumpCheckDNS();

		// Authentication service 	

		bdAuthService* getAuthService();

		const bdAuthInfo *getAuthInfo();

		void checkAuthServiceError();

		void pumpAuthServiceTask();

		// Create player1
		void createPlayer1Description(bdString& description);
		void startCreatePlayer1();
		void pumpCreatePlayer1();

		// Create player2
		void createPlayer2Description(bdString& description);
		void startCreatePlayer2();
		void pumpCreatePlayer2();

		// Authenticate player1
		void authenticatePlayer1Description(bdString& description);
		void startAuthenticatePlayer1();
		void pumpAuthenticatePlayer1();

		// Authenticate player2
		void authenticatePlayer2Description(bdString& description);
		void startAuthenticatePlayer2();
		void pumpAuthenticatePlayer2();


		// Lobby service connect

		void lobbyServiceConnectDescription(bdString& description);

		void startLobbyServiceConnect();

		void pumpLobbyServiceConnect();

		// Get Channel List

		void getChannelListDescription(bdString& description);

		void startGetChannelList();

		void pumpGetChannelList();


		// Create Channel List

		void getCreateChannelDescription(bdString& description);

		void startCreateChannel();

		void pumpCreateChannel();

		// Subscribe to Channel

		void subscribeToChannelDescription(bdString& description);

		void startSubscribeToChannel();

		void pumpSubscribeToChannel();

		// Send to Channel

		void sendToChannelDescription(bdString& description);

		void startSendToChannel();

		void pumpSendToChannel();

		// Get Channel Members

		void getChannelMembersDescription(bdString& description);

		void startGetChannelMembers();

		void pumpGetChannelMembers();

		// Whisper to channel member

		void whisperToMemberDescription(bdString& description);

		void startWhisperToMember();

		void pumpWhisperToMember();

		// Whisper to channel member

		void unsubscribeFromChannelDescription(bdString& description);

		void startUnsubscribeFromChannel();

		void pumpUnsubscribeFromChannel();

		// Lobby disconnect

		void lobbyServiceDisconnectDescription(bdString& description);

		void startLobbyServiceDisconnect();

		void pumpLobbyServiceDisconnect();

		// Network stop

		void stopNetworkDescription(bdString& description);

		void stopNetwork();

		// A generic pump method, for all channel chat tasks that do not return results.
		void pumpGenericChannelChatTask();
		

	protected:

		// receives log messages
		bdLogSubscriber m_logSubscriber;

		TaskStatus m_taskStatus;
		Task m_task;
		bdString m_lastTaskDescription;

		bdBool m_changedScreen;
		bdString m_screenText;

		bdInetAddr m_authAddr;
		bdGetHostByName *m_authGetHostByName;

		bdAuthService *m_authService;
		bdLobbyService *m_lobbyService;

		bdInetAddr m_lsgAddr;
		bdGetHostByName *m_lsgGetHostByName;

		bdRemoteTaskRef m_remoteTask;

		//PrintableList<bdChannelInfo, MAX_NUM_DISPLAY_CHANNELS> m_channelList;

		bdUInt64 m_subscribingChannelID;
		bdFastArray<bdUInt64> m_subscribedChannels;
		
		bdChannelMember m_channelMembers[MAX_NUM_DISPLAY_MEMBERS];
		bdUInt m_numChannelMembers;

		bdString m_messageText;
		bdStopwatch m_messageTimer;
		bdUInt m_playerNumber;

		// Public channel infos
		bdChannelInfo m_channelInfos[MAX_NUM_CHANNELS];
		bdUInt m_numChatChannels;

		// Private channel info
		bdChannelInfo m_myChannelInfo;

		bdBool m_pumpLobbyConnection;
		bdLobbyErrorCode m_lobbyErrorCode;

		bdUInt64 m_returnedChannelID;
};


#endif // SIMULATION_H
