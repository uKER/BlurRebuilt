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
#include <bdLobby/bdContentStreaming/bdContentStreaming.h>
#include <bdLobby/bdContentStreaming/bdFileID.h>
#include <bdLobby/bdContentStreaming/bdFileMetaData.h>
#include <bdLobby/bdContentStreaming/bdQuotaUsage.h>
#include <bdLobby/bdContentStreaming/bdURL.h>
#include <bdLobby/bdContentStreaming/bdContentStreamingStatsInfo.h>
#include <bdLobby/bdContentStreaming/bdDownloadInterceptor.h>
#include <bdLobby/bdContentStreaming/bdUploadInterceptor.h>
#include <bdLobby/bdStorage/bdGetFilesResult.h>

#include <bdCore/bdCrypto/bdHashTiger192.h>

#define FILE_HASH_SIZE 24
#define FILE_PORTION_SIZE 1000
#define FILE_SIZE_HTTP (20*1000u)
#define INCOMPLETE_FILE_SIZE_HTTP (FILE_SIZE_HTTP / 2)

#define MAX_NUM_DISPLAY_RANKS (10u)
#define MAX_SLOTS_PER_USER 5
#define MAX_NUM_RESULTS 5

static char* taskDescs[] = {
	"START_NETWORK",
	"CHECK_DNS",
	"CREATE_ACCOUNT",
	"AUTHENTICATE_ACCOUNT",
	"LOBBY_CONNECT",
	"GET_QUOTA",
	"GET_FILE_META_DATA",
	"GET_FILE_META_DATA_USER_2",
	"UPLOAD_FILE",
	"DOWNLOAD_FILE_BY_SLOT",
	"DOWNLOAD_FILE_BY_SLOT_USER_2",
	"DOWNLOAD_FILE_BY_ID",
	"DOWNLOAD_FILE_BY_ID_INCOMPLETE",
	"DOWNLOAD_FILE_BY_ID_RESUME",
	"LIST_FILES_BY_OWNER",
	"LIST_FILES_BY_DOWNLOADCOUNT",
	"REPORT_CONTENT",
	"LIST_FILES_BY_OWNER_USER_2",
	"GET_FILES_THUMBNAILS",
	"GET_FILE_META_DATA_BY_ID",
	"LIST_ALL_PUBLISHER_FILES",
	"DOWNLOAD_PUBLISHER_FILE",
	"LIST_ALL_APPLE_PUBLISHER_FILES",
	"DOWNLOAD_APPLE_PURCHASED_FILE",
	"REMOVE_FILE",
	"LOBBY_DISCONNECT",
	"STOP_NETWORK"
};

class Simulation : public bdDemo, bdDownloadInterceptor, bdUploadInterceptor
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
			GET_QUOTA,
			GET_FILE_META_DATA,
			GET_FILE_META_DATA_USER_2,
			UPLOAD_FILE,
			DOWNLOAD_FILE_BY_SLOT,
			DOWNLOAD_FILE_BY_SLOT_USER_2,
			DOWNLOAD_FILE_BY_ID,
			DOWNLOAD_FILE_BY_ID_INCOMPLETE,
			DOWNLOAD_FILE_BY_ID_RESUME,
			LIST_FILES_BY_OWNER,
			LIST_FILES_BY_DOWNLOADCOUNT,
			REPORT_CONTENT,
			LIST_FILES_BY_OWNER_USER_2,
			GET_FILES_THUMBNAILS,
			GET_FILE_META_DATA_BY_ID,
			LIST_ALL_PUBLISHER_FILES,
			DOWNLOAD_PUBLISHER_FILE,
			LIST_ALL_APPLE_PUBLISHER_FILES,
			DOWNLOAD_APPLE_PURCHASED_FILE,
			REMOVE_FILE,
			LOBBY_DISCONNECT,
			STOP_NETWORK
		};

		void startTask();
		void pumpTask();
		void displayTaskResult();
		bdContentStreaming* getContentStreaming();

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

		// Lobby disconnect
		void lobbyServiceDisconnectDescription(bdString& description);
		void startLobbyServiceDisconnect();
		void pumpLobbyServiceDisconnect();

		// Network stop
		void stopNetworkDescription(bdString& description);
		void stopNetwork();

		TaskStatus CheckContentStreamingTaskStatus(bdUInt contentStreamingStatus);

		// regular MMP methods
		// These methods function is the same fashion as other lobby methods
		// such as friends and matchmaking

		// get file meta data
		void generateGetFileDescriptionUser2(bdString& description);
		void generateGetFileDescription(bdString& description);
		void startGetFileMetaData();
		void pumpGetFileMetaData();

		// list files by owner
		void generateListFilesByOwnerDescriptionUser2(bdString& description);
		void generateListFilesByOwnerDescription(bdString& description);
		void startListFilesByOwner();
		void pumpListFilesByOwner();
		
		// report dodgy content
		void generateReportContentDescription(bdString& description);
		void startReportContent();
		void pumpReportContent();
		
		// List files by download count
		void generateListFilesByDownloadCountDescription(bdString& description);
		void startListFilesByDownloadCount();
		void pumpListFilesByDownloadCount();

		// get thumbnail files for metadata
		void generateGetFilesThumbnailsDescription(bdString& description);
		void startGetFilesThumbnails();
		void pumpGetFilesThumbnails();

		// get file by ID
		void generateGetFileByIDDescription(bdString& description);
		void startGetFileMetaDataByID();
		void pumpGetFileMetaDataByID();

		// list all publisher files
		void generateListAllPublisherFilesDescription(bdString& description);
		void startListAllPublisherFiles(bdUInt16 category);
		void pumpListAllPublisherFiles();

		// download publisher file
		void generateDownloadPublisherFileDescription(bdString& description);
		void startDownloadPublisherFile();
		void pumpDownloadPublisherFile();
		
		// download apple purchsed file
		void generateDownloadApplePurchasedFileDescription(bdString& description);
		void startDownloadApplePurchasedFile();
		void pumpDownloadApplePurchasedFile();

		// get quota
		void generateGetQuotaDescription(bdString& description);
		void startGetQuota();
		void pumpGetQuota();

		// file IO methods 
		// these methods create a fresh connection to the HTTP server 
		// upon each invocation

		// upload file by http
		void generateUploadDescription(bdString& description);
		void startUpload();
		void pumpUpload();

		// get file by http
		void generateDownloadDescriptionUser2(bdString& description);
		void generateDownloadDescription(bdString& description);
		void startDownload();
		void pumpDownload();

		// get file by http
		void generateDownloadByIDDescription(bdString& description);
		void startDownloadByID();
		void pumpDownloadByID();

		// get file by http - incomplete
		void generateDownloadByIDDescription_Incomplete(bdString& description);
		void startDownloadByID_Incomplete();
		void pumpDownloadByID_Incomplete();

		// get file by http - download resume
		void generateDownloadByIDDescription_Resume(bdString& description);
		void startDownloadByID_Resume();
		//void pumpDownloadByID();

		// remove file
		void generateRemoveFileDescription(bdString& description);
		void startRemoveFile();
		void pumpRemoveFile();

		void initUpload(bdUInt32 uploadSize);
		void initDownload();
		
		virtual bdUInt handleDownload(void* data, bdUInt32 dataSize);
		virtual bdUInt handleUpload(void* uploadBuffer, bdUInt32 bufferSize, bdUInt32 bytesUploaded);

		// print to screen helpers
		void regenerateFileList(bdFileMetaData *metaData);
		void regenerateFileList(bdStatsInfo* downloadCountStatsInfo);
		void regenerateMyFileList();
		void appendFileToList(bdFileMetaData *metaData);
		void regenerateQuota(bdBool printLimits = false);
		void regenerateFileListPostUpload();
		
		// these methods demonstrate best practise quota tracking
		void adjustQuotaPostUpload(bdUInt32 bytesUploaded,bdUInt32 previousFileSize = 0);
		void adjustQuotaPostDownload(bdUInt32 bytesDownloaded);
		void adjustQuotaPostRemove(bdUInt32 bytesRemoved);

		// these methods demonstrate best client side tracking of a users slot usage
		void adjustMyFilesPostReset();
		void adjustMyFilesPostUpload(bdFileMetaData *uploadedFile);
		void adjustMyFilesPostRemove(bdUInt32 deletedSlot);
		void adjustMyFilesPostList(bdUInt numMetaDatas, bdFileMetaData *metaDatas);

		void printDataRate(bdNChar8 *buf, bdUInt32 bufSize);
		void resetTimers();

	protected:

		// config
		bdTitleID m_titleID;
		bdNChar8 m_lsgURL[256];
		bdNChar8 m_authURL[256];
		bdUInt16 m_lsgPort;
		bdUInt32 m_authPort;

		// receives log messages
		bdLogSubscriber m_logSubscriber;

		TaskStatus m_taskStatus;
		Task m_task;
		bdString m_lastTaskDescription;

		bdBool m_changedScreenText;
		bdString m_screenText;

		bdInetAddr m_authAddr;
		bdGetHostByName *m_authGetHostByName;

		// the authentication service
#ifndef BD_PLATFORM_XENON
		bdAuthService *m_authService;
#endif
		bdLobbyService *m_lobbyService;

		// the LSG service
		bdInetAddr m_lsgAddr;
		bdGetHostByName *m_lsgGetHostByName;
		
		bdRemoteTaskRef m_remoteTask;

		bdBool m_pumpLobbyConnection;
		bdLobbyErrorCode m_lobbyErrorCode;

		bdHashTiger192 m_tigerHash;
		bdUByte8 m_hashFile[FILE_HASH_SIZE];
		bdUByte8 m_hashIncompleteFile[FILE_HASH_SIZE];
		
		bdNChar8 m_userName[BD_MAX_USERNAME_LENGTH];
		bdNChar8 m_password[BD_MAX_USERNAME_LENGTH];
		bdNChar8 m_userName2[BD_MAX_USERNAME_LENGTH];
		bdNChar8 m_password2[BD_MAX_USERNAME_LENGTH];
		bdOnlineUserID m_userID;
		bdOnlineUserID m_userID2;
		bdOnlineUserID m_currentUserID;

		bdNChar8 *m_currentUser;
		bdNChar8 *m_currentPassword;

		// tmp info
		bdFileID m_uploadedFileID;
		bdNChar8 m_myFileNameHTTP[BD_MAX_FILE_NAME_LENGTH];
		bdUInt16 m_fileSizeIndex;
		bdUInt32 m_fileSize;
		bdUInt16 m_fileSlot;
		bdUByte8 *m_fileData;
		bdUInt32 m_bytesDownloaded;
		bdUInt32 m_bytesUploaded;
		bdUInt32 m_uploadSize;


		// The available categories of the file to be uploaded. Category specified on upload.
		enum bdCategory
		{
			UNSPECIFIED = 0,
			PHOTO = 1,
			EVENT = 2
		};

		// The available categories of the file to be downloaded from the publisher
		enum bdPublisherCategory
		{
			PUBLISHER_FILES_UNSPECIFIED = 0,
			PUBLISHER_FILES_PHOTO = 1,
			PUBLISHER_FILES_APPLE = 500
		};

		// Note: There is a unique DownloadCount leaderboard per category.
		// In future this LB_ID creation and code generation will be handled by WebIMP.
		// Until then, please send DemonWare the list of categories you wish to use, and DemonWare will generate 
		// the necessary leaderboards on the server, and return the below enum to you.
		// bdDownloadCountLBID's enum will also be provided to the studio when a list of categories is sent to DemonWare.
		//
		// bdLobby\bdStats\bdStatsInfo is the class for deserializing bdNumDownloads leaderboards.
		//enum bdDownloadCountLBID
		enum bdContentStreamingLeaderboardIDs
		{
			//VoteRankLeaderboard_Unspecified = 50023,
			VoteRankLeaderboard_PHOTO = 50024,
			VoteRankLeaderboard_EVENT = 50025,
			//DownloadCountLeaderboard_Unspecified = 50026,
			DownloadCountLeaderboard_PHOTO = 50027,
			DownloadCountLeaderboard_EVENT = 50028,
			//RecentFilesLeaderboard_Unspecified = 50029,
			RecentFilesLeaderboard_PHOTO = 50030,
			RecentFilesLeaderboard_EVENT = 50031,
			ReportContentLeaderboard = 50032
		};

		// The category of the file to be uploaded.
		bdCategory m_myCategory;

		bdString m_fileListString;

		// result info
		bdUInt32 m_numFileResults;
		bdGetFilesResult m_thumbnails[MAX_NUM_RESULTS];
		bdFileMetaData m_listResultsByOwner[MAX_SLOTS_PER_USER];
		bdFileMetaData m_listResultsByID[MAX_NUM_RESULTS];
		bdFileMetaData m_publisherFiles[MAX_NUM_RESULTS];

		// locally modified results info
		bdQuotaUsage m_usage;
		bdFileMetaData m_myFiles[MAX_SLOTS_PER_USER];
		bdBool m_slotIsUsed[MAX_SLOTS_PER_USER];
		bdFileMetaData m_uploadedFileMetaData;
		bdFileMetaData m_downloadedFileMetaData;

		bdStopwatch m_stopwatch;
		bdFloat32 m_lastTimeSampled;
		bdBool m_checkFileData;

		bdStopwatch m_dataRateStopwatch;
		bdFloat32 m_prevCallbackTime;
		
		// results class for list files by download count call
		bdContentStreamingStatsInfo m_contentStreamingStatsInfo[MAX_NUM_DISPLAY_RANKS];
};

#endif // SIMULATION_H

