// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef SIMULATION_H
#define SIMULATION_H

#include <bdExample/bdExampleCommon/bdExampleCommon.h>

#include <bdExample/bdExampleCommonLobby/bdExampleCommonLobby.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdContentUnlock/bdContentUnlock.h>

#define MAX_CONTENT_INFOS 10

class Simulation : public bdDemo
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		Simulation();

		~Simulation();

		virtual bdBool init(bdInt argc,
							bdNChar8* argv[]);

		virtual bdBool step(const bdFloat32 time);
		
		virtual void quit();

	protected:

		enum Task
		{
			START_NETWORK,			
			CHECK_DNS,
			CREATE_ACCOUNT,			
			AUTHENTICATE_ACCOUNT,
			LOBBY_CONNECT,

			// storage tasks
			LIST_CONTENT_BY_KEY,
			UNLOCK_CONTENT,
			LIST_UNLOCKED_CONTENT,

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

		// Create account
		void createAccountDescription(bdString& description);
		void startCreateAccount();
		void pumpCreateAccount();

		// Authenticate account
		void authenticateAccountDescription(bdString& description);
		void startAuthenticateAccount();
		void pumpAuthenticateAccount();

		// Lobby service connect
		void lobbyServiceConnectDescription(bdString& description);
		void startLobbyServiceConnect();
		void pumpLobbyServiceConnect();

		// Get Content By Key
		void listContentByKeyDescription(bdString& description);
		void startListContentByKey();
		void pumpListContentByKey();

		// Unlock Content
		void unlockContentDescription(bdString& description);
		void startUnlockContent();
		void pumpUnlockContent();

		// List unlocked content
		void listUnlockedContentDescription(bdString& description);
		void startListUnlockedContent();
		void pumpListUnlockedContent();

		// Lobby disconnect
		void lobbyServiceDisconnectDescription(bdString& description);
		void startLobbyServiceDisconnect();
		void pumpLobbyServiceDisconnect();

		// Network stop
		void stopNetworkDescription(bdString& description);
		void stopNetwork();
		
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

		bdBool m_pumpLobbyConnection;
		bdLobbyErrorCode m_lobbyErrorCode;

		// Content Unlock specific data
		bdOnlineUserID m_userID;

		bdUnlockContentInfo m_unlockedContentInfo;
		bdUnlockContentInfo m_unlockedContentInfoList[MAX_CONTENT_INFOS];

		bdString m_unlockedContentString;
		bdString m_contentString;
};

#endif // SIMULATION_H
