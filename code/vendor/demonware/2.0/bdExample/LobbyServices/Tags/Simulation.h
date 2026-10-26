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
#include <bdLobby/bdTags/bdTagsArray.h>

#define MAX_NUM_FILES (50u)

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
			UPLOAD_FILE,
			SET_TAGS_FOR_ENTITYID,
			GET_TAGS_FOR_ENTITYIDS,
			SEARCY_BY_TAGS,
			REMOVE_TAGS_FOR_ENTITYID,
			GET_TAGS_FOR_ENTITYIDS2,
			REMOVE_ALL_TAGS_FOR_ENTITYID,
			GET_TAGS_FOR_ENTITYIDS3,
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

#ifndef BD_PLATFORM_XENON
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
#endif

		// Lobby service connect
		void lobbyServiceConnectDescription(bdString& description);
		void startLobbyServiceConnect();
		void pumpLobbyServiceConnect();

		// Tags
		void printFileIDsList(bdString& string);
		void printFileIDTagsList(bdString& string);

		// Upload File
		void uploadFileDescription(bdString& description);
		void startUploadFile();
		void pumpUploadFile();

		// Set Tags
		void setTagsDescription(bdString& description);
		void startSetTags();
		void pumpSetTags();

		// Get Tags
		void getTagsDescription(bdString& description);
		void startGetTags();
		void pumpGetTags();

		// Search By Tags
		void searchByTagsDescription(bdString& description);
		void startSearchByTags();
		void pumpSearchByTags();

		// Remove Tags
		void removeTagsDescription(bdString& description);
		void startRemoveTags();
		void pumpRemoveTags();

		// Remove All Tags
		void removeAllTagsDescription(bdString& description);
		void startRemoveAllTags();
		void pumpRemoveAllTags();

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

#ifndef BD_PLATFORM_XENON
		bdAuthService *m_authService;
#endif
		bdLobbyService *m_lobbyService;

		bdInetAddr m_lsgAddr;
		bdGetHostByName *m_lsgGetHostByName;
		
		// The remote task handle for the active task
		bdRemoteTaskRef  m_remoteTask;
		
		// Then fileID of the file uploaded by the example upload operation
		bdFileID m_uploadedFileID;

		// Results class for fileID for any file search operation
		bdFileID m_fileIDs[MAX_NUM_FILES];
		
		// The number of fileIDs found in any search operation
		bdUInt m_numFilesIDs;

		// Holds tag results for operations such as bdTags.getTagsForEntityIDs
		bdTagsArray m_tagResults[MAX_NUM_FILES];
		
		// Printable lists, helpers for displaying results of tasks
		PrintableList<bdFileID, MAX_NUM_FILES> m_foundFileIDs;
		PrintableList<bdTagsArray, MAX_NUM_FILES> m_foundTags;

		bdBool m_pumpLobbyConnection;
		bdLobbyErrorCode m_lobbyErrorCode;
};

#endif // SIMULATION_H
