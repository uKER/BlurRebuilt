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
#include <bdLobby/bdContentStreaming/bdFileID.h>
#include <bdLobby/bdFacebook/bdFacebookIsRegistered.h>


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
			REGISTER_ACCOUNT,
			IS_REGISTERED,
			POST,
			UPLOAD_PHOTO_TO_CONTENT_SERVER,
			UPLOAD_PHOTO,
			UNREGISTER_ACCOUNT,
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

		// ----------------------------------------------------------------------------------------
		void checkLobbyError();

		// registerAccount
		void getRegisterAccountDescription(bdString& description);

		void startRegisterAccount();

		void pumpRegisterAccount();

		// isRegistered
		void getIsRegisteredDescription(bdString& description);

		void startIsRegistered();

		void pumpIsRegistered();

		// post
		void getPostDescription(bdString& description);

		void startPost();

		void pumpPost();

		// uploadPhotoToContentServer
		void getUploadPhotoToContentServerDescription(bdString& description);

		void startUploadPhotoToContentServer();

		void pumpUploadPhotoToContentServer();

		// uploadPhoto
		void getUploadPhotoDescription(bdString& description);

		void startUploadPhoto();

		void pumpUploadPhoto();

		// unregisterAccount
		void getUnregisterAccountDescription(bdString& description);

		void startUnregisterAccount();

		void pumpUnregisterAccount();

		// Lobby disconnect

		void lobbyServiceDisconnectDescription(bdString& description);

		void startLobbyServiceDisconnect();

		void pumpLobbyServiceDisconnect();

		// Network stop

		void stopNetworkDescription(bdString& description);

		void stopNetwork();


		void resetTimers();
		
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

		bdFileID m_uploadedFileID;

		bdStopwatch m_stopwatch;
		bdFloat32 m_lastTimeSampled;

		bdFacebookIsRegistered m_facebookIsRegistered;
};

#endif // SIMULATION_H
