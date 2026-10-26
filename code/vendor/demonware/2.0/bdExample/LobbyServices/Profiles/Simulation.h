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


#include "UserInfo.h"

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
			PROFILE_CREATE,			
			PROFILE_GET_PUBLIC_INFO,	
			PROFILE_GET_PRIVATE_INFO,	
			PROFILE_SET_PUBLIC_INFO,	
			PROFILE_SET_PRIVATE_INFO,	
			PROFILE_DELETE,						
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

		// Profiles
		
		void checkProfilesError();
		
		// Create Profile

		void createProfileDescription(bdString& description);

		void startCreateProfile();

		void pumpCreateProfile();
		
		// Get public Info

		void getPublicInfoDescription(bdString& description);

		void startGetPublicInfo();

		void pumpGetPublicInfo();

		// Get private info
		void getPrivateInfoDescription(bdString& description);

		void startGetPrivateInfo();

		void pumpGetPrivateInfo();

		// Set public Info

		void setPublicInfoDescription(bdString& description);

		void startSetPublicInfo();

		void pumpSetPublicInfo();

		// Set private info
		void setPrivateInfoDescription(bdString& description);

		void startSetPrivateInfo();

		void pumpSetPrivateInfo();

		
		// Delete Profile 
		void deleteProfileInfoDescription(bdString& description);

		void startDeleteProfile();

		void pumpDeleteProfile();

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

		PublicProfileInfo m_publicProfileInfos[2];

		PrivateProfileInfo m_privateProfileInfo;

		bdBool m_changedScreen;
		bdString m_screenText;

		bdInetAddr m_authAddr;
		bdGetHostByName *m_authGetHostByName;

		bdAuthService *m_authService;
		bdLobbyService *m_lobbyService;

		bdInetAddr m_lsgAddr;
		bdGetHostByName *m_lsgGetHostByName;

		bdRemoteTaskRef  m_remoteTask;

		bdBool m_pumpLobbyConnection;
		bdLobbyErrorCode m_lobbyErrorCode;
};

#endif // SIMULATION_H
