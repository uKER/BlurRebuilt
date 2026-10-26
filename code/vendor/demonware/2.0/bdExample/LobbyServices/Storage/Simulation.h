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
#include <bdLobby/bdStorage/bdStorage.h>
#include <bdCore/bdCrypto/bdHashTiger192.h>
#include <bdLobby/bdStorage/bdFileData.h>
#include <bdLobby/bdStorage/bdFileInfo.h>

#define STORAGE_EXAMPLE_FILE_SIZE 24
#define FILE_SIZE 1024
#define MAX_FILES 10

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
			UPLOAD_FILE,
			GET_FILE,
			LIST_FILES_BY_OWNER,
			GET_FILE_BY_ID,
			LIST_ALL_PUBLISHER_FILES,
			GET_PUBLISHER_FILE,
			REMOVE_FILE,

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
		
		// upload file
		void generateUploadFileDescription(bdString& description);
		void startUploadFile();
		void pumpUploadFile();
			
		// get file 
		void generateGetFileDescription(bdString& description);
		void startGetFile();
		void pumpGetFile();
	
		// list files by owner
		void generateListFilesByOwnerDescription(bdString& description);
		void startListFilesByOwner();
		void pumpListFilesByOwner();

		// get file by ID
		void generateGetFileByIDDescription(bdString& description);
		void startGetFileByID();
		void pumpGetFileByID();
	
		// list all publisher files
		void generateListAllPublisherFilesDescription(bdString& description);
		void startListAllPublisherFiles();
		void pumpListAllPublisherFiles();
	
		// get publisher file 
		void generateGetPublisherFileDescription(bdString& description);
		void startGetPublisherFile();
		void pumpGetPublisherFile();

		// remove file
		void generateRemoveFileDescription(bdString& description);
		void startRemoveFile();
		void pumpRemoveFile();

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

		// storage specific data
		bdHashTiger192 m_tigerHash;
		bdUByte8 m_hashFile[STORAGE_EXAMPLE_FILE_SIZE];

		bdFileData m_fileData;
		bdFileInfo m_fileInfos[MAX_FILES];
		bdUInt m_numFileInfos;
		bdUByte8 m_data[FILE_SIZE];

		bdNChar8 m_myFileName[100];
		bdOnlineUserID m_UserId;
		bdString m_fileListString;

};

#endif // SIMULATION_H
