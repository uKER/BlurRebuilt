// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "Simulation.h"
#include <bdLobby/bdLobbyCommon/bdAuthCommon.h>
#include <bdCore/bdUtilities/bdRandom.h>
#include <time.h>
#include <bdLobby/bdTags/bdTag.h>
#include <bdLobby/bdContentStreaming/bdFileID.h>

// Set to use the defined bdDownloadInterceptor and bdUploadInterceptor callback methods
#define USE_CUSTOM_CALLBACKS_FOR_HTTP
#define RATE_LIMIT_HTTP
#define MAX_UPLOAD_RATE_KBYTES_PER_SEC 4		
#define MAX_DOWNLOAD_RATE_KBYTES_PER_SEC 8
#define MAX_UPLOAD_CHUNK_SIZE (1024)

// Useful for testing the maximum values and sizes of parameters
//#define TEST_MAXIMA

#if defined BD_PLATFORM_XENON || defined BD_PLATFORM_PS3 
// Useful for running on remote dev kits - no input required to test
#define RUN_CONTENT_STREAMING_EXAMPLE_AUTOMATEDLY
#endif

#ifdef BD_PLATFORM_XENON

#include <bdExample/bdExampleCommonLobby/LiveServices.h>
#define TITLE_ID			0x14D8
#define LIVE_LSP_SERVICE_ID	0x415607E9 
#define LIVE_TITLE_ID		0xDEADBEEF // INSERT_YOUR_LIVE_TITLE_ID_HERE
#define LSP_PORT			(3017u)

#include <xtitleidbegin.h>
XEX_TITLE_ID (LIVE_TITLE_ID)
#include <xtitleidend.h>
#else
#define TITLE_ID 0x14D7
#endif

#define DEFAULT_USERNAME "CSsample1"
#define DEFAULT_PASSWORD "pass1"
#define DEFAULT_USERNAME2 "CSsample2"
#define DEFAULT_PASSWORD2 "pass2"
#define PROFILE_PIC_FILE_NAME "MyPic"

static char itunes_receipt[]=
"{\n"
"\"signature\"=\"AVsnnFoiR2b5mI55LvQFNg2VKEIWb7cjBtwnyRIQEwQi8VX7T1Y5HsIlWdIOCZN5Q90jSSUOoJzN1SS2QMe76VsX3V5w2kXMrf7y21PxdV/gdf5yFQ+BScOe+E+ECgFz14kbrbMiof5jlK7BYSsffUURi6T4NqitXBcx06PIWhQwMIIDUzCCAjugAwIBAgIIZRSRTdlYBLUwDQYJKoZIhvcNAQEFBQAwfzELMAkGA1UEBhMCVVMxEzARBgNVBAoMCkFwcGxlIEluYy4xJjAkBgNVBAsMHUFwcGxlIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MTMwMQYDVQQDDCpBcHBsZSBpVHVuZXMgU3RvcmUgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMDkwNjE1MjIwNTU2WhcNMTQwNjE0MjIwNTU2WjBkMSMwIQYDVQQDDBpQdXJjaGFzZVJlY2VpcHRDZXJ0aWZpY2F0ZTEbMBkGA1UECwwSQXBwbGUgaVR1bmVzIFN0b3JlMRMwEQYDVQQKDApBcHBsZSBJbmMuMQswCQYDVQQGEwJVUzCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEAytGMXZy3gitJ2JMKFojSDynC/9yYezyn9HBX+u3/3VcpWE2XhcgGKYqNBA1+AewOzrKO774OsokTu4qymEx10ph8UTmsZewB0ESMHBEjF7FN6/HccsQUYC3WagrHnT12HG2Ih0OAm/ZhpWzj0HS4m813LpIyo00sewMvMNL2hkcCAwEAAaNyMHAwDAYDVR0TAQH/BAIwADAfBgNVHSMEGDAWgBQ2HejinYLSARi1MmsO10MLkVhDOjAOBgNVHQ8BAf8EBAMCB4AwHQYDVR0OBBYEFKmDg/IZSMU+ElcIFMzNo36ZXyT1MBAGCiqGSIb3Y2QGBQEEAgUAMA0GCSqGSIb3DQEBBQUAA4IBAQARpJs+O2Y3gL8gHdASkrfZHFpwINd1VcB5VF5LkVpnFz63zylA/3cGIDG91b/d5NIwZjkVt4Bgvd62o/mCbzCsWiNfSKTJVFK1D78BDQoSO2oHTuQuz1BR7xzNHxQZ90zUS6ZX9SC8N3g3A1jEtAyDhZNB+CRBBXLwZdnBUeBsT9QLpjvTnekZcGTnU08zfCjGF3eBJEu9eP6WgexK1xMSp72kEOmYbn6yTi3D4YrcYx4Q3n/57VBP2en8qXWeP5oHDsLTGzLRsWdoB3VxJLrF2ivL8JS8zqC0qyac452pN6xunRuzyyfpaqzQL12BzFEe44xna2byektSbtquA5LNAAAAAA==\";"
"\"purchase-info\" = \"ewoJIml0ZW0taWQiID0gIjM0MDM4NTE2OSI7Cgkib3JpZ2luYWwtdHJhbnNhY3Rpb24taWQiID0gIjEwMDAwMDAwMDAxOTkyMzEiOwoJInB1cmNoYXNlLWRhdGUiID0gIjIwMDktMTItMTUgMTE6MDE6MDAgRXRjL0dNVCI7CgkicHJvZHVjdC1pZCIgPSAiY29tLmlkZWF3b3JrczNkLmtpd2lUZXN0LnZlcnJ1Y2t0IjsKCSJ0cmFuc2FjdGlvbi1pZCIgPSAiMTAwMDAwMDAwMDE5OTIzMSI7CgkicXVhbnRpdHkiID0gIjEiOwoJIm9yaWdpbmFsLXB1cmNoYXNlLWRhdGUiID0gIjIwMDktMTItMTUgMTE6MDE6MDAgRXRjL0dNVCI7CgkiYmlkIiA9ICJjb20uaWRlYXdvcmtzM2Qua2l3aVRlc3QiOwoJImJ2cnMiID0gIjEuMS4wIjsKfQ==\";"
"\"pod\" = \"100\";\n"
"\"signing-status\" = \"0\";\n"
"}\n";

Simulation::Simulation()
: m_taskStatus(READY),
  m_task(START_NETWORK),
  m_changedScreenText(true),
  m_authGetHostByName(BD_NULL),
#ifndef BD_PLATFORM_XENON
  m_authService(BD_NULL),
#endif
  m_lsgGetHostByName(BD_NULL),
  m_remoteTask(BD_NULL),
  m_pumpLobbyConnection(false),
  m_lobbyErrorCode(BD_NO_ERROR),
  m_fileSizeIndex(0),
  m_fileSize(FILE_SIZE_HTTP),
  m_fileSlot(0),
  m_checkFileData(false)
{
	m_uploadedFileID.m_fileID = 0;
	adjustMyFilesPostReset();

	bdSnprintf(m_userName, sizeof(m_userName), m_userName);

	for (bdUInt i=0; i<MAX_NUM_RESULTS; i++)
	{
		m_thumbnails[i].m_data.m_fileData = BD_NULL;
	}

	// create the file data dynamically so as not to overflow stack on limited memory platforms
	m_fileData = new bdUByte8[FILE_SIZE_HTTP];
}

Simulation::~Simulation()
{
	for (bdUInt i=0; i<MAX_NUM_RESULTS; i++)
	{
		if(m_thumbnails[i].m_data.m_fileData != BD_NULL)
		{
			delete static_cast<bdUByte8*>(m_thumbnails[i].m_data.m_fileData);
		}
	}

	delete [] m_fileData;
}

bdBool Simulation::init(bdInt32 argc,
						bdNChar8* argv [])
{
	bdBool ok = true;

	// start logging
	// we want to hear about all warnings and errors
	m_logSubscriber.addChannel("warn");
	m_logSubscriber.addChannel("err");
	m_logSubscriber.addChannel("info");

	// DEFAULTS
	m_titleID = TITLE_ID;

#ifdef BD_PLATFORM_XENON
	m_lsgPort = LSP_PORT;
#else
	m_lsgPort = LSG_PORT;
	bdSnprintf(m_lsgURL,256,"%s",LSG_ADDRESS);
	bdSnprintf(m_authURL,256,"%s",AUTH_ADDRESS);
	m_authPort = AUTH_PORT;
	bdSnprintf(m_userName,BD_MAX_USERNAME_LENGTH,"%s",DEFAULT_USERNAME);
	bdSnprintf(m_password,BD_MAX_USERNAME_LENGTH,"%s",DEFAULT_PASSWORD);
	bdSnprintf(m_userName2,BD_MAX_USERNAME_LENGTH,"%s",DEFAULT_USERNAME2);
	bdSnprintf(m_password2,BD_MAX_USERNAME_LENGTH,"%s",DEFAULT_PASSWORD2);
	m_currentUser = m_userName;
	m_currentPassword = m_password;

#endif

	// parse user args
	bdInt32 numArgs(1);
	if (argc > numArgs)
	{
		m_titleID = atoi(argv[numArgs++]);
	}
#ifndef BD_PLATFORM_XENON
	if (argc > numArgs)
	{
		bdStrlcpy(m_lsgURL,argv[numArgs++],256);
	}
#endif
	if (argc > numArgs)
	{
		m_lsgPort = bdUInt16(atoi(argv[numArgs++]));
	}
#ifndef BD_PLATFORM_XENON
	if (argc > numArgs)
	{
		bdStrlcpy(m_authURL,argv[numArgs++],256);
	}
	if (argc > numArgs)
	{
		m_authPort = atoi(argv[numArgs++]);
	}
	if (argc > numArgs)
	{
		bdStrlcpy(m_userName,argv[numArgs++],BD_MAX_USERNAME_LENGTH);
	}
	if (argc > numArgs)
	{
		bdStrlcpy(m_password,argv[numArgs++],BD_MAX_USERNAME_LENGTH);
	}
	if (argc > numArgs)
	{
		bdStrlcpy(m_userName2,argv[numArgs++],BD_MAX_USERNAME_LENGTH);
	}
	if (argc > numArgs)
	{
		bdStrlcpy(m_password2,argv[numArgs++],BD_MAX_USERNAME_LENGTH);
	}
#endif

	m_lobbyService = new bdLobbyService();

	return ok;
}

bdBool Simulation::step(const bdFloat32)
{
	const bdBool quitKey = getScreen().wasKeyPressed(BD_QUIT);

	TaskStatus oldTaskStatus = m_taskStatus;

	if(m_taskStatus == READY)
	{
		startTask();
	}

	if(m_taskStatus == PENDING || m_taskStatus == DONE || m_taskStatus == FAILED)
	{
		// pump even if the status is DONE or FAILED to read out any results
		// and close any remote task handles
		pumpTask();
	}

	if(m_taskStatus == DONE || m_taskStatus == FAILED || m_taskStatus == SKIPPED)
	{
		// in case the task fails or completes in a single loop
		if(oldTaskStatus == READY)
		{
			oldTaskStatus = m_taskStatus;
		}

		displayTaskResult();
	}

	m_changedScreenText = (oldTaskStatus != m_taskStatus);

	if(m_pumpLobbyConnection)
	{
		bdBool lostConnection = true;
		if(m_lobbyService)
		{
			m_lobbyService->pump();
			const bdLobbyConnection::Status status = m_lobbyService->getStatus();

			bdContentStreaming* const contentStreaming = getContentStreaming();
			if(contentStreaming)
			{
				contentStreaming->pump();
			}

			switch(status)
			{
				case bdConnection::BD_NOT_CONNECTED:
				case bdConnection::BD_CONNECTING:
				case bdConnection::BD_DISCONNECTING:
				case bdConnection::BD_DISCONNECTED:
				{
					lostConnection = true;
				}
				break;
				case bdConnection::BD_CONNECTED:
				{
					lostConnection = false;
				}
				break;
			}
		}

		if(lostConnection)
		{
			// set up for a reconnect
			m_pumpLobbyConnection = false;
			m_task = STOP_NETWORK;
			m_taskStatus = READY;
			m_changedScreenText = true;
			m_lastTaskDescription = "Lost connection to lobby.";
		}
	}

	getScreen().printToScreen(m_screenText);

	if(m_changedScreenText)
	{
		m_screenText = "";
	}

	return !quitKey;
}

void Simulation::quit()
{
	m_logSubscriber.removeChannel("warn");
	m_logSubscriber.removeChannel("err");
	m_logSubscriber.removeChannel("info");

	stopNetwork();

	delete m_authGetHostByName;
#ifndef BD_PLATFORM_XENON
	delete m_authService;
#endif
	delete m_lsgGetHostByName;
	
	m_remoteTask = BD_NULL;
	delete m_lobbyService;
}

void Simulation::startTask()
{
	const bdBool skipKey = getScreen().wasKeyPressed(SKIP_KEY);

	if (skipKey)
	{
		m_taskStatus = SKIPPED;
		m_changedScreenText = true;
		m_fileListString = "";
	}
	else
	{
#ifdef RUN_CONTENT_STREAMING_EXAMPLE_AUTOMATEDLY
		const bdBool startKey = true;
#else
		const bdBool startKey = getScreen().wasKeyPressed(START_KEY);
#endif

		bdString taskDescription("Unknown task");

		// default user for all operations unless overridden
		m_currentUserID = m_userID;

		switch(m_task)
		{
			case START_NETWORK:
			{
				startNetworkDescription(taskDescription);
				if(startKey)
				{
					startNetwork();
				}
			}
			break;
#ifndef BD_PLATFORM_XENON
			case CHECK_DNS:
			{
				checkDNSDescription(taskDescription);
				if(startKey)
				{
					startCheckDNS();
				}
			}
			break;
			case CREATE_ACCOUNT:
			{
				createAccountDescription(taskDescription);
				if(startKey)
				{
					startCreateAccount();
				}
			}
			break;
			case AUTHENTICATE_ACCOUNT:
			{
				authenticateAccountDescription(taskDescription);
				if(startKey)
				{
					startAuthenticateAccount();
				}
			}
			break;
#endif
			case LOBBY_CONNECT:
			{
				lobbyServiceConnectDescription(taskDescription);
				if(startKey)
				{
					startLobbyServiceConnect();
				}
			}
			break;
			case GET_FILE_META_DATA_USER_2:
				{
					m_currentUserID = m_userID2;

					generateGetFileDescriptionUser2(taskDescription);
					if(startKey)
					{
						startGetFileMetaData();
					}
				}
				break;
			case GET_FILE_META_DATA:
				{
					generateGetFileDescription(taskDescription);
					if(startKey)
					{
						startGetFileMetaData();
					}
				}
				break;
			case GET_FILE_META_DATA_BY_ID:
				{
					generateGetFileByIDDescription(taskDescription);
					if(startKey)
					{
						startGetFileMetaDataByID();
					}
				}
				break;
			case LIST_FILES_BY_OWNER_USER_2:
				{
					m_currentUserID = m_userID2;
					generateListFilesByOwnerDescriptionUser2(taskDescription);
					if(startKey)
					{
						startListFilesByOwner();
					}
				}
				break;
			case LIST_FILES_BY_OWNER:
				{
					generateListFilesByOwnerDescription(taskDescription);
					if(startKey)
					{
						startListFilesByOwner();
					}
				}
				break;
			case REPORT_CONTENT:
				{
					generateReportContentDescription(taskDescription);
					if(startKey)
					{
						startReportContent();
					}
				}
				break;
			case LIST_FILES_BY_DOWNLOADCOUNT:
				{
					generateListFilesByDownloadCountDescription(taskDescription);
					if(startKey)
					{
						startListFilesByDownloadCount();
					}
				}
				break;
			case GET_FILES_THUMBNAILS:
				{
					generateGetFilesThumbnailsDescription(taskDescription);
					if(startKey)
					{
						startGetFilesThumbnails();
					}
				}
				break;
			case LIST_ALL_PUBLISHER_FILES:
				{
					generateListAllPublisherFilesDescription(taskDescription);
					if(startKey)
					{
						startListAllPublisherFiles(PUBLISHER_FILES_PHOTO);
					}
				}
				break;
			case DOWNLOAD_PUBLISHER_FILE:
				{
					generateDownloadPublisherFileDescription(taskDescription);
					if(startKey)
					{
						startDownloadPublisherFile();
					}
				}
				break;
			case LIST_ALL_APPLE_PUBLISHER_FILES:
				{
					generateListAllPublisherFilesDescription(taskDescription);
					if(startKey)
					{
						startListAllPublisherFiles(PUBLISHER_FILES_APPLE);
					}
				}
				break;
			case DOWNLOAD_APPLE_PURCHASED_FILE:
				{
					generateDownloadApplePurchasedFileDescription(taskDescription);
					if(startKey)
					{
						startDownloadApplePurchasedFile();
					}
				}
				break;
			case GET_QUOTA:
				{
					generateGetQuotaDescription(taskDescription);
					if(startKey)
					{
						startGetQuota();
					}
				}
				break;
			case UPLOAD_FILE:
			{
				generateUploadDescription(taskDescription);
				if(startKey)
				{
					startUpload();
				}
			}
			break;

			case DOWNLOAD_FILE_BY_ID:
			{
				generateDownloadByIDDescription(taskDescription);
				if(startKey)
				{
					startDownloadByID();
				}
			}
			break;

			case DOWNLOAD_FILE_BY_ID_INCOMPLETE:
			{
				generateDownloadByIDDescription_Incomplete(taskDescription);
				if(startKey)
				{
					startDownloadByID_Incomplete();
				}
			}
			break;

			case DOWNLOAD_FILE_BY_ID_RESUME:
			{
				generateDownloadByIDDescription_Resume(taskDescription);
				if(startKey)
				{
					startDownloadByID_Resume();
				}
			}
			break;

			case DOWNLOAD_FILE_BY_SLOT_USER_2:
			{
				m_currentUserID = m_userID2;

				generateDownloadDescriptionUser2(taskDescription);
				if(startKey)
				{
					startDownload();
				}
			}
			break;
			case DOWNLOAD_FILE_BY_SLOT:
			{
				generateDownloadDescription(taskDescription);
				if(startKey)
				{
					startDownload();
				}
			}
			break;
			case REMOVE_FILE:
			{
				generateRemoveFileDescription(taskDescription);
				if(startKey)
				{
					startRemoveFile();
				}
			}
			break;
	
			case LOBBY_DISCONNECT:
			{
				lobbyServiceDisconnectDescription(taskDescription);
				if(startKey)
				{
					startLobbyServiceDisconnect();
				}
			}
			break;

			case STOP_NETWORK:
			{
				stopNetworkDescription(taskDescription);
				if(startKey)
				{
					stopNetwork();
				}
			}
			break;
		}

		if(m_changedScreenText)
		{
			m_screenText = taskDescription;
			m_screenText += "\n" START_TEXT;
			m_screenText += "\n" SKIP_TEXT;

			if(m_lastTaskDescription.getLength())
			{
				m_screenText += "\n\n";
				m_screenText += m_lastTaskDescription;
			}
			
			m_screenText += m_fileListString;
		}
	}
}

void Simulation::pumpTask()
{
	bdString taskDescription("Unknown task");

	// don't pump tasks that have failed to start
	if (m_taskStatus == FAILED)
	{
		BD_ERR("sim","Task failed");
		return;
	}

	switch(m_task)
	{
		case START_NETWORK:
		{
			startNetworkDescription(taskDescription);
			pumpStartNetwork();
		}
		break;
#ifndef BD_PLATFORM_XENON
		case CHECK_DNS:
		{
			checkDNSDescription(taskDescription);
			pumpCheckDNS();
		}
		break;
		case CREATE_ACCOUNT:
		{
			createAccountDescription(taskDescription);
			pumpCreateAccount();
		}
		break;
		break;
		case AUTHENTICATE_ACCOUNT:
		{
			authenticateAccountDescription(taskDescription);
			pumpAuthenticateAccount();
		}
		break;
#endif
		case LOBBY_CONNECT:
		{
			lobbyServiceConnectDescription(taskDescription);
			pumpLobbyServiceConnect();
		}
		break;

		case GET_FILE_META_DATA_USER_2:
			{
				generateGetFileDescriptionUser2(taskDescription);
				pumpGetFileMetaData();
			}
			break;
		case GET_FILE_META_DATA:
			{
				generateGetFileDescription(taskDescription);
				pumpGetFileMetaData();
			}
			break;
		case GET_FILE_META_DATA_BY_ID:
			{
				generateGetFileByIDDescription(taskDescription);
				pumpGetFileMetaDataByID();
			}
			break;
		case LIST_FILES_BY_OWNER_USER_2:
			{
				generateListFilesByOwnerDescriptionUser2(taskDescription);
				pumpListFilesByOwner();
			}
			break;
		case LIST_FILES_BY_OWNER:
			{
				generateListFilesByOwnerDescription(taskDescription);
				pumpListFilesByOwner();
			}
			break;
		case REPORT_CONTENT:
			{
				generateReportContentDescription(taskDescription);
				pumpReportContent();
			}
			break;
		case LIST_FILES_BY_DOWNLOADCOUNT:
			{
				generateListFilesByDownloadCountDescription(taskDescription);
				pumpListFilesByDownloadCount();
			}
			break;
		case GET_FILES_THUMBNAILS:
			{
				generateGetFilesThumbnailsDescription(taskDescription);
				pumpGetFilesThumbnails();
			}
			break;
		case LIST_ALL_PUBLISHER_FILES:
		case LIST_ALL_APPLE_PUBLISHER_FILES:
			{
				generateListAllPublisherFilesDescription(taskDescription);
				pumpListAllPublisherFiles();
			}
			break;
		case DOWNLOAD_PUBLISHER_FILE:
			{
				generateDownloadPublisherFileDescription(taskDescription);
				pumpDownloadPublisherFile();
			}
			break;
		case DOWNLOAD_APPLE_PURCHASED_FILE:
			{
				generateDownloadApplePurchasedFileDescription(taskDescription);
				pumpDownloadApplePurchasedFile();
			}
			break;
		case GET_QUOTA:
			{
				generateGetQuotaDescription(taskDescription);
				pumpGetQuota();
			}
			break;
		case UPLOAD_FILE:
			{
				generateUploadDescription(taskDescription);
				pumpUpload();
			}
			break;
		case DOWNLOAD_FILE_BY_SLOT_USER_2:
			{
				generateDownloadDescriptionUser2(taskDescription);
				pumpDownload();
			}
			break;
		case DOWNLOAD_FILE_BY_SLOT:
			{
				generateDownloadDescription(taskDescription);
				pumpDownload();
			}
			break;
		case DOWNLOAD_FILE_BY_ID:
			{
				generateDownloadByIDDescription(taskDescription);
				pumpDownloadByID();
			}
			break;
		case DOWNLOAD_FILE_BY_ID_INCOMPLETE:
			{
				generateDownloadByIDDescription_Incomplete(taskDescription);
				pumpDownloadByID_Incomplete();
			}
			break;
		case DOWNLOAD_FILE_BY_ID_RESUME:
			{
				generateDownloadByIDDescription_Resume(taskDescription);
				pumpDownloadByID();
			}
			break;
		case REMOVE_FILE:
		{
			generateRemoveFileDescription(taskDescription);
			pumpRemoveFile();
		}
		break;
		case LOBBY_DISCONNECT:
		{
			lobbyServiceDisconnectDescription(taskDescription);
			pumpLobbyServiceDisconnect();
		}
		break;
		case STOP_NETWORK:
		{
			// nothing to to here
		}
		break;
	}

	if(m_changedScreenText)
	{
		m_screenText = taskDescription;
	}
}

void Simulation::displayTaskResult()
{
	bdString taskDescription("Unknown task result");

	Task nextTaskIfDone = m_task;
	Task nextTaskIfFailed = m_task;
	Task nextTaskIfSkipped = m_task;

	bdBool authStage = false;
	bdBool displayLobbyError = true;

	switch(m_task)
	{
		case START_NETWORK:
		{
			startNetworkDescription(m_lastTaskDescription);
#ifdef BD_PLATFORM_XENON
			nextTaskIfDone = LOBBY_CONNECT;
#else
			nextTaskIfDone = CHECK_DNS;
#endif
			nextTaskIfFailed = START_NETWORK;
			nextTaskIfSkipped = START_NETWORK;
			displayLobbyError = false;
		}
		break;
#ifndef BD_PLATFORM_XENON
		case CHECK_DNS:
		{
			checkDNSDescription(m_lastTaskDescription);
			nextTaskIfDone = AUTHENTICATE_ACCOUNT;
			nextTaskIfFailed = CHECK_DNS;
			nextTaskIfSkipped = AUTHENTICATE_ACCOUNT;
			displayLobbyError = false;
		}
		break;
		case CREATE_ACCOUNT:
		{
			createAccountDescription(m_lastTaskDescription);
			checkAuthServiceError();
			nextTaskIfDone = AUTHENTICATE_ACCOUNT;
			nextTaskIfFailed = CREATE_ACCOUNT;
			nextTaskIfSkipped = AUTHENTICATE_ACCOUNT;
			authStage = true;
		}
		break;
		case AUTHENTICATE_ACCOUNT:
		{
			authenticateAccountDescription(m_lastTaskDescription);
			checkAuthServiceError();
			nextTaskIfDone = LOBBY_CONNECT;
			nextTaskIfFailed = CREATE_ACCOUNT;
			nextTaskIfSkipped = LOBBY_CONNECT;
			authStage = true;
		}
		break;
#endif
		case LOBBY_CONNECT:
		{
			lobbyServiceConnectDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_QUOTA;
			nextTaskIfFailed = LOBBY_CONNECT;
			nextTaskIfSkipped = GET_QUOTA;
		}
		break;
		case GET_QUOTA:
		{
			generateGetQuotaDescription(m_lastTaskDescription);
			nextTaskIfDone = UPLOAD_FILE;
			nextTaskIfFailed = GET_QUOTA;
			nextTaskIfSkipped = UPLOAD_FILE;
		}
		break;
		case UPLOAD_FILE:
		{
			generateUploadDescription(m_lastTaskDescription);
			nextTaskIfDone = DOWNLOAD_FILE_BY_SLOT;
			nextTaskIfFailed = UPLOAD_FILE;
			nextTaskIfSkipped = DOWNLOAD_FILE_BY_SLOT;
		}
		break;
		case DOWNLOAD_FILE_BY_SLOT:
		{
			generateDownloadDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_FILES_THUMBNAILS;
			nextTaskIfFailed = DOWNLOAD_FILE_BY_SLOT;
			nextTaskIfSkipped = GET_FILES_THUMBNAILS;
		}
		break;
		case GET_FILES_THUMBNAILS:
		{
			generateGetFilesThumbnailsDescription(m_lastTaskDescription);
			nextTaskIfDone = LIST_FILES_BY_OWNER;
			nextTaskIfFailed = GET_FILES_THUMBNAILS;
			nextTaskIfSkipped = LIST_FILES_BY_OWNER;
		}
		break;
		case LIST_FILES_BY_OWNER:
		{
			generateListFilesByOwnerDescription(m_lastTaskDescription);
			nextTaskIfDone = REPORT_CONTENT;
			nextTaskIfFailed = LIST_FILES_BY_OWNER;
			nextTaskIfSkipped = REPORT_CONTENT;
		}
		break;
		case REPORT_CONTENT:
		{
			generateReportContentDescription(m_lastTaskDescription);
			nextTaskIfDone = LIST_FILES_BY_DOWNLOADCOUNT;
			nextTaskIfFailed = REPORT_CONTENT;
			nextTaskIfSkipped = LIST_FILES_BY_DOWNLOADCOUNT;
		}
		break;
		case LIST_FILES_BY_DOWNLOADCOUNT:
		{
			generateListFilesByDownloadCountDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_FILE_META_DATA;
			nextTaskIfFailed = LIST_FILES_BY_DOWNLOADCOUNT;
			nextTaskIfSkipped = GET_FILE_META_DATA;
		}
		break;
		case GET_FILE_META_DATA:
		{
			generateGetFileDescription(m_lastTaskDescription);
			nextTaskIfDone = DOWNLOAD_FILE_BY_ID;
			nextTaskIfFailed = GET_FILE_META_DATA;
			nextTaskIfSkipped = DOWNLOAD_FILE_BY_ID;
		}
		break;
		case DOWNLOAD_FILE_BY_ID:
		{
			generateDownloadByIDDescription(m_lastTaskDescription);
			nextTaskIfDone = DOWNLOAD_FILE_BY_ID_INCOMPLETE;
			nextTaskIfFailed = DOWNLOAD_FILE_BY_ID;
			nextTaskIfSkipped = DOWNLOAD_FILE_BY_ID_INCOMPLETE;
		}
		break;
		case DOWNLOAD_FILE_BY_ID_INCOMPLETE:
		{
			generateDownloadByIDDescription(m_lastTaskDescription);
			nextTaskIfDone = DOWNLOAD_FILE_BY_ID_RESUME;
			nextTaskIfFailed = DOWNLOAD_FILE_BY_ID_INCOMPLETE;
			nextTaskIfSkipped = DOWNLOAD_FILE_BY_ID_RESUME;
		}
		break;
		case DOWNLOAD_FILE_BY_ID_RESUME:
		{
			generateDownloadByIDDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_FILE_META_DATA_BY_ID;
			nextTaskIfFailed = DOWNLOAD_FILE_BY_ID_RESUME;
			nextTaskIfSkipped = GET_FILE_META_DATA_BY_ID;
		}
		break;
		case GET_FILE_META_DATA_BY_ID:
		{
			generateGetFileByIDDescription(m_lastTaskDescription);
			nextTaskIfDone = REMOVE_FILE;
			nextTaskIfFailed = GET_FILE_META_DATA_BY_ID;
			nextTaskIfSkipped = REMOVE_FILE;
		}
		break;
		case REMOVE_FILE:
		{
			generateRemoveFileDescription(m_lastTaskDescription);
			nextTaskIfDone = GET_FILE_META_DATA_USER_2;
			nextTaskIfFailed = REMOVE_FILE;
			nextTaskIfSkipped = GET_FILE_META_DATA_USER_2;
		}
		break;
		case GET_FILE_META_DATA_USER_2:
		{
			generateGetFileDescriptionUser2(m_lastTaskDescription);
			nextTaskIfDone = LIST_FILES_BY_OWNER_USER_2;
			nextTaskIfFailed = GET_FILE_META_DATA_USER_2;
			nextTaskIfSkipped = LIST_FILES_BY_OWNER_USER_2;
		}
		break;
		case LIST_FILES_BY_OWNER_USER_2:
		{
			generateListFilesByOwnerDescriptionUser2(m_lastTaskDescription);
			nextTaskIfDone = DOWNLOAD_FILE_BY_SLOT_USER_2;
			nextTaskIfFailed = DOWNLOAD_FILE_BY_SLOT_USER_2; // expected to fail in single user mode
			nextTaskIfSkipped = DOWNLOAD_FILE_BY_SLOT_USER_2;
		}
		break;
		case DOWNLOAD_FILE_BY_SLOT_USER_2:
		{
			generateDownloadDescriptionUser2(m_lastTaskDescription);
			nextTaskIfDone = LIST_ALL_PUBLISHER_FILES;
			nextTaskIfFailed = LIST_ALL_PUBLISHER_FILES; // expected to fail in single user mode
			nextTaskIfSkipped = LIST_ALL_PUBLISHER_FILES;
		}
		break;
		case LIST_ALL_PUBLISHER_FILES:
		{
			generateListAllPublisherFilesDescription(m_lastTaskDescription);
			nextTaskIfDone = DOWNLOAD_PUBLISHER_FILE;
			nextTaskIfFailed = LIST_ALL_PUBLISHER_FILES;
			nextTaskIfSkipped = DOWNLOAD_PUBLISHER_FILE; 
		}
		break;
		case DOWNLOAD_PUBLISHER_FILE:
		{
			generateDownloadPublisherFileDescription(m_lastTaskDescription);
			nextTaskIfDone = LIST_ALL_APPLE_PUBLISHER_FILES;
			nextTaskIfFailed = DOWNLOAD_PUBLISHER_FILE;
			nextTaskIfSkipped = LIST_ALL_APPLE_PUBLISHER_FILES;
		}
		break;
		case LIST_ALL_APPLE_PUBLISHER_FILES:
		{
			generateListAllPublisherFilesDescription(m_lastTaskDescription);
			nextTaskIfDone = DOWNLOAD_APPLE_PURCHASED_FILE;
			nextTaskIfFailed = LIST_ALL_APPLE_PUBLISHER_FILES;
			nextTaskIfSkipped = DOWNLOAD_APPLE_PURCHASED_FILE; 
		}
		break;
		case DOWNLOAD_APPLE_PURCHASED_FILE:
		{
			generateDownloadPublisherFileDescription(m_lastTaskDescription);
			nextTaskIfDone = LOBBY_DISCONNECT;
			nextTaskIfFailed = DOWNLOAD_APPLE_PURCHASED_FILE;
			nextTaskIfSkipped = LOBBY_DISCONNECT;
		}
		break;
		case LOBBY_DISCONNECT:
		{
			lobbyServiceDisconnectDescription(m_lastTaskDescription);
			nextTaskIfDone = STOP_NETWORK;
			nextTaskIfFailed = LOBBY_DISCONNECT;
			nextTaskIfSkipped = STOP_NETWORK;
		}
		break;
		case STOP_NETWORK:
		{
			stopNetworkDescription(m_lastTaskDescription);
			nextTaskIfDone = START_NETWORK;
			nextTaskIfFailed = STOP_NETWORK;
			nextTaskIfSkipped = CREATE_ACCOUNT;
		}
		break;
	}

	switch(m_taskStatus)
	{
		case READY:
		case PENDING:
		{
			BD_WARN(BD_LOG_LEVEL, "Unexpected task status.");
		}
		break;
		case DONE:
		{
			m_task = nextTaskIfDone;
			BD_INFO(BD_LOG_LEVEL, "Done, Starting task %d (%s).", m_task, taskDescs[m_task]);
		}
		break;
		case FAILED:
		{
			m_task = nextTaskIfFailed;
			BD_INFO(BD_LOG_LEVEL, "Failed, Starting task %d (%s).", m_task, taskDescs[m_task]);
		}
		break;
		case SKIPPED:
		{
			m_task = nextTaskIfSkipped;
			BD_INFO(BD_LOG_LEVEL, "Skipped, Starting task %d (%s).", m_task, taskDescs[m_task]);
		}
		break;
	}

	// if task uses sets a lobby error code and has failed
	// display the error description
	if(displayLobbyError && (m_taskStatus == FAILED))
	{
		if(authStage)
		{
#ifndef BD_PLATFORM_XENON
			checkAuthServiceError();
#endif
		}
		else
		{
			if (m_remoteTask != BD_NULL)
			{
				m_lobbyErrorCode = m_remoteTask->getErrorCode();
			}
		}

		bdNChar8 buf[64];
		LobbyErrorCodeToString(m_lobbyErrorCode, buf, sizeof(buf));
		m_lastTaskDescription += "\nError: ";
		m_lastTaskDescription += buf;
		m_lobbyErrorCode = BD_NO_ERROR;
	}

	m_taskStatus = READY;
}

void Simulation::startNetworkDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Start Network",				// READY
											"Starting Network",				// PENDING
											"Started Network",				// DONE
											"Starting Network Failed",		// FAILED
											"Starting Network Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startNetwork()
{
	bdNetImpl *const net = bdNet::getInstance();
	bdNetStartParams params;
	params.m_onlineGame = false;

	// platform specific startup parameters
	bdPlatformParams platformParams;
	//TODO: check the return value here
	bdExamplePlatformInit::init(params, &platformParams);

	if(net->start(params))
	{
		m_taskStatus = PENDING;
	}
	else
	{
		m_taskStatus = FAILED;
	}
}

void Simulation::pumpStartNetwork()
{
	bdNetImpl *const net = bdNet::getInstance();
	net->pump();

	const bdNetImpl::bdNetStatus networkStatus = net->getStatus();
	switch(networkStatus)
	{
		case bdNetImpl::BD_NET_BIND_FAILED:
		case bdNetImpl::BD_NET_ONLINE_FAILED:
		case bdNetImpl::BD_NET_INIT_FAILED:
		case bdNetImpl::BD_NET_STOPPED:
		case bdNetImpl::BD_NET_PARAMS_CONFIG_ERROR:
		{
			// error conditions
			m_taskStatus = FAILED;
		}
		break;
		case bdNetImpl::BD_NET_PENDING:
		{
			// wait
			m_taskStatus = PENDING;
		}
		break;
		case bdNetImpl::BD_NET_DONE:
		{
			// compl
			m_taskStatus = DONE;
		}
	}
}

#ifndef BD_PLATFORM_XENON

void Simulation::checkDNSDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Start DNS Lookup",			// READY
											"Performing DNS Lookup",	// PENDING
											"DNS Lookup Succeeded",		// DONE
											"DNS Lookup Failed",		// FAILED
											"DNS Lookup Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if(m_taskStatus == DONE)
	{
		bdNChar8 addrString[BD_INET_ADDR_STRING_SIZE];
		m_authAddr.toString(addrString, sizeof(addrString));
		description +=  "\n " AUTH_ADDRESS ":";
		description += addrString;
		description +=  "\n";

		m_lsgAddr.toString(addrString, sizeof(addrString));
		description +=  " " LSG_ADDRESS ":";
		description += addrString;
	}
}

void Simulation::startCheckDNS()
{
	const TaskStatus authDNSStatus = StartGetHostByName(m_authURL, &m_authGetHostByName, m_authAddr);
	const TaskStatus lsgDNSStatus = StartGetHostByName(m_lsgURL, &m_lsgGetHostByName, m_lsgAddr);
	
	if((authDNSStatus == FAILED) || (lsgDNSStatus == FAILED))
	{
		m_taskStatus = FAILED;
	}

	if((authDNSStatus == PENDING) || (lsgDNSStatus == PENDING))
	{
		m_taskStatus = PENDING;
	}

	if((authDNSStatus == DONE) && (lsgDNSStatus == DONE))
	{
		m_taskStatus = DONE;
	}
}

void Simulation::pumpCheckDNS()
{
	TaskStatus authDNSStatus = DONE;
	TaskStatus lsgDNSStatus = DONE;

	if(m_authGetHostByName)
	{
		authDNSStatus = PumpGetHostByName(&m_authGetHostByName, m_authAddr);
	}

	if(m_lsgGetHostByName)
	{
		lsgDNSStatus = PumpGetHostByName(&m_lsgGetHostByName, m_lsgAddr);
	}

	if((authDNSStatus == FAILED) || (lsgDNSStatus == FAILED))
	{
		m_taskStatus = FAILED;
	}

	if((authDNSStatus == PENDING) || (lsgDNSStatus == PENDING))
	{
		m_taskStatus = PENDING;
	}

	if((authDNSStatus == DONE) && (lsgDNSStatus == DONE))
	{
		if (m_authAddr.isValid() && m_lsgAddr.isValid())
		{
			m_taskStatus = DONE;
		}
		else
		{
			m_taskStatus = FAILED;
		}
	}
}

bdAuthService* Simulation::getAuthService()
{
	if(m_authService == BD_NULL)
	{
		const bdAddr authAddr(m_authAddr, m_authPort);
		m_authService = new bdAuthService(m_titleID, authAddr);
	}

	return m_authService;
}


const bdAuthInfo* Simulation::getAuthInfo()
{
	if(m_authService != BD_NULL)
	{
		return m_authService->getAuthInfo();
	}

	return BD_NULL;
}

void Simulation::checkAuthServiceError()
{
	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		m_lobbyErrorCode = authService->getErrorCode();
	}
}

void Simulation::pumpAuthServiceTask()
{
	// fail safe
	m_taskStatus = FAILED;

	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		m_taskStatus = PENDING;
		const bdAuthService::bdStatus status = authService->getStatus();
		if(status == bdAuthService::BD_READY)
		{
			const bdLobbyErrorCode lobbyErrorCode = authService->getErrorCode();
			if(lobbyErrorCode == BD_AUTH_NO_ERROR)
			{
				m_taskStatus = DONE;
			}
			else
			{
				m_taskStatus = FAILED;
			}
		}
	}
}

void Simulation::createAccountDescription(bdString& description)
{
	bdNChar8 description1[256];
	bdSnprintf(description1,256,"Create Account (%s/%s)",m_userName,m_password);

	const bdNChar8 *const descriptions[] = { description1,	// READY
											"Creating Account",								// PENDING
											"Created Account",								// DONE
											"Create Account Failed",						// FAILED
											"Create Account Skipped"};						// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startCreateAccount()
{
	// fail safe
	m_taskStatus = FAILED;

	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		if(authService->createAccount(m_userName, m_password))
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpCreateAccount()
{
	pumpAuthServiceTask();
}

void Simulation::authenticateAccountDescription(bdString& description)
{
	bdNChar8 description1[256];
	bdSnprintf(description1,256,"Create Account (%s/%s)",m_userName,m_password);

	const bdNChar8 *const descriptions[] = { description1,	// READY
											"Authenticating Account",									// PENDING
											"Authenticated Account",									// DONE
											"Authenticate Account Failed",								// FAILED
											"Authenticate Account Skipped"};							// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startAuthenticateAccount()
{
	// fail safe
	m_taskStatus = FAILED;

	bdAuthService *const authService = getAuthService();
	if(authService)
	{
		if(authService->authorizeAccount(m_userName, m_password))
		{
			m_taskStatus = PENDING;
		}
	}
}

void Simulation::pumpAuthenticateAccount()
{
	pumpAuthServiceTask();
}
#endif

void Simulation::lobbyServiceConnectDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Connect to Lobby Service",				// READY
											"Connecting to Lobby Service",			// PENDING
											"Connected to Lobby Service",			// DONE
											"Connect to Lobby Service Failed",		// FAILED
											"Connect to Lobby Service Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

#ifdef BD_PLATFORM_XENON 
void Simulation::startLobbyServiceConnect()
{
	// fail safe
	m_taskStatus = FAILED;

	if(m_lobbyService)
	{
		LiveServices liveServices;
		liveServices.signOn(0);
		bdBool m_errorFlag = false;
		while (	(false == liveServices.isSignedOn()) && 
			(false == m_errorFlag))
		{
			m_errorFlag = !(liveServices.pump());
			bdPlatformTiming::sleep(50);
		}

		if(false == m_errorFlag)
		{
			bdAddr ca;
			bdSecurityID unusedID;
			bdSecurityKey key;
			if (!liveServices.resolveTitleAddr(LIVE_LSP_SERVICE_ID, m_lsgPort, ca, unusedID, key))
			{
				BD_WARN("test", "Failed to resolve LSG address. Failing now.");
				m_errorFlag = true;
			}
			else
			{
				// setup local user
				bdNChar8 buffer [BD_MAX_USERNAME_LENGTH];
				liveServices.getXUserName(buffer);

				BD_INFO("test", "SG Lite address found.");

				// Prepare auth info
				bdAuthTicket ticket;
				ticket.m_userID = liveServices.getXUserId();

				// cache m_userID for use in API calls
				m_userID = ticket.m_userID;

				ticket.m_titleID = int(m_titleID);
				liveServices.getXUserName(ticket.m_username);

				bdAuthInfo authInfo;
				authInfo.m_titleID = m_titleID;
				ticket.serialize(authInfo.m_data);

				BD_INFO("test", "Connecting to matchmaking...");
				m_errorFlag = !(m_lobbyService->connect(ca, &authInfo));

				bdPrintf ("Connecting to MatchMaking+ sandbox...");
			}
		}
		if(false == m_errorFlag)
		{
			m_taskStatus = PENDING;
		}
	}
}
#else
void Simulation::startLobbyServiceConnect()
{
	// fail safe
	m_taskStatus = FAILED;
	if(m_lobbyService)
	{
		const bdAddr lsgAddr(m_lsgAddr, m_lsgPort);
		const bdBool connecting = m_lobbyService->connect(lsgAddr, getAuthInfo());

		if(connecting)
		{
			m_taskStatus = PENDING;
		}
	}
}
#endif

void Simulation::pumpLobbyServiceConnect()
{
	// fail safe
	m_taskStatus = FAILED;

	m_pumpLobbyConnection = false;

	if(m_lobbyService)
	{
		m_lobbyService->pump();
		const bdLobbyConnection::Status connectionStatus = m_lobbyService->getStatus();
		switch(connectionStatus)
		{
			case bdConnection::BD_NOT_CONNECTED:
			case bdConnection::BD_DISCONNECTING:
			case bdConnection::BD_DISCONNECTED:
			{
				m_taskStatus = FAILED;
			}
			break;
			case bdConnection::BD_CONNECTING:
			{
				m_taskStatus = PENDING;
			}
			break;
			case bdConnection::BD_CONNECTED:
			{
				m_taskStatus = DONE;
				m_pumpLobbyConnection = true;
#ifndef BD_PLATFORM_XENON
				m_userID = bdAuthUtility::getUserID(m_userName);
				m_userID2 = bdAuthUtility::getUserID(m_userName2);
				m_currentUserID = m_userID;
#else				
				// init must be called before we call any HTTP related operations
				// ie upload download delete
				m_lobbyService->getContentStreaming()->init(LIVE_LSP_SERVICE_ID);
#endif
			}
			break;
		}
	}
}


void Simulation::generateUploadDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Upload file (via HTTP)",				// READY
		"Uploading file (via HTTP)",			// PENDING
		"File uploaded (via HTTP)",			// DONE
		"File upload failed (via HTTP)",		// FAILED
		"File upload skipped (via HTTP)"		// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if (m_taskStatus == PENDING)
	{
		bdNChar8 buf[40];
		printDataRate(buf, 40);
		description += buf;
	}	
}

void Simulation::startUpload()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_ERR("sim","Could not acquire handle to content streaming service. Are you still connected to LSG?");
		m_taskStatus = FAILED;
		return;
	}

	// Fill file full of the star character (2A in hex)
	bdMemset(m_fileData,'*',m_fileSize); 

	bdUInt resSize = 24;
	m_tigerHash.hash(
		static_cast<bdUByte8*>(m_fileData),
		m_fileSize,
		m_hashFile,
		resSize);

	m_tigerHash.hash(
		static_cast<bdUByte8*>(m_fileData),
		INCOMPLETE_FILE_SIZE_HTTP,
		m_hashIncompleteFile,
		resSize);

#ifdef TEST_MAXIMA
	bdUInt32 numTags = BD_MAX_NUM_TAGS;
	bdTag tags[BD_MAX_NUM_TAGS];
	
	for (bdUInt16 i(0); i<BD_MAX_NUM_TAGS; i++)
	{
		tags[i].set(1,2*i);
	}

	bdUInt32 thumbSize = BD_MAX_THUMB_DATA_SIZE;
	bdUByte8* thumbData = new bdUByte8[thumbSize];
	bdMemset(thumbData,'!',thumbSize);

	bdMemset(m_myFileNameHTTP, 'A', BD_MAX_FILE_NAME_LENGTH);
	bdMemset(m_myFileNameHTTP + BD_MAX_FILE_NAME_LENGTH, 0, 1);
#else
	bdUInt32 numTags = 5;
	bdTag tags[5];
	tags[0].set(1,5);
	tags[1].set(2,3);
	tags[2].set(3,2);
	tags[3].set(4,1);
	tags[4].set(6,4);

	bdUInt32 thumbSize = 16;
	bdUByte8* thumbData = new bdUByte8[thumbSize];
	bdMemset(thumbData,'!',thumbSize);

	bdRandom rand;
	if (rand.nextUInt() % 2 == 0)
	{
		bdSnprintf(m_myFileNameHTTP, sizeof(m_myFileNameHTTP), PROFILE_PIC_FILE_NAME);
	}
	else
	{
		bdSnprintf(m_myFileNameHTTP, sizeof(m_myFileNameHTTP), "File%d\0", rand.nextUInt() % 100);
	}

#endif

	m_fileSlot = m_fileSlot++ == MAX_SLOTS_PER_USER ? 1 : m_fileSlot;
	m_myCategory = PHOTO;

#ifdef USE_CUSTOM_CALLBACKS_FOR_HTTP
	initUpload(m_fileSize);	
	m_remoteTask = contentStreaming->upload(m_fileSlot,	
											this,
											m_fileSize,
											m_myFileNameHTTP, 
											m_myCategory,
											thumbData,
											thumbSize,
											numTags,
											tags,
											&m_uploadedFileID); 
#else
	m_remoteTask = contentStreaming->upload(m_fileSlot,	
											m_fileData,
											m_fileSize,
											m_myFileNameHTTP, 
											m_myCategory,
											thumbData,
											thumbSize,
											numTags,
											tags,
											&m_uploadedFileID); 
#endif

	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	
	if (m_taskStatus == PENDING)
	{
		BD_INFO("sim","Uploading file of size %d commenced to slot %d.", m_fileSize, m_fileSlot);

		bdSnprintf(m_uploadedFileMetaData.m_fileName, BD_MAX_FILE_NAME_LENGTH, m_myFileNameHTTP);
		m_uploadedFileMetaData.m_fileSize = m_fileSize;
		m_uploadedFileMetaData.m_fileSlot = m_fileSlot;
		m_uploadedFileMetaData.m_numTags = numTags;
		//m_uploadedFileMetaData.m_tags = tags;

		resetTimers();
	}
	else
	{
		BD_INFO("sim","Uploading file of size %d failed to start.",m_fileSize);
	}
}

void Simulation::pumpUpload()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();
	
	if (contentStreaming == BD_NULL)
	{
		BD_ERR("sim","Could not access bdContentStreaming object");
		return;
	}

	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
	
	switch(m_taskStatus)
	{
		case PENDING:
		{
			contentStreaming->pump();

#ifdef BD_PLATFORM_XENON
			if ( m_stopwatch.getElapsedTimeInSeconds() > m_lastTimeSampled + 1.0f)
			{
				m_lastTimeSampled = m_stopwatch.getElapsedTimeInSeconds();
				bdNChar8 buf[40];
				printDataRate(buf, 40);
				BD_INFO("sim", buf);
			}
#endif
		}
		break;
		case DONE:
		{
			
			m_lobbyService->pump();

			const bdUInt sizeStrLen = 128;
			bdNChar8 fileIDString[sizeStrLen];
			bdSnprintf(fileIDString, sizeStrLen, "Uploaded file ID: %u of size %d to slot %d.", m_uploadedFileID.m_fileID, m_fileSize, m_fileSlot);
			m_fileListString = "";
			m_fileListString += fileIDString;
			
			adjustQuotaPostUpload(m_fileSize);
			adjustMyFilesPostUpload(&m_uploadedFileMetaData);
			regenerateFileListPostUpload();
			regenerateQuota();

			BD_INFO("sim","Uploading file of ID %d complete.", m_uploadedFileID.m_fileID);
			
		}
		break;
		case FAILED:
		{
			BD_WARN("sim","Uploading failed.");
		}
		break;
		default:
			break;
	}
}

void Simulation::generateDownloadDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Download file",			// READY
		"Downloading file",			// PENDING
		"Downloaded file",			// DONE
		"File download failed",		// FAILED
		"File download skipped"		// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if (m_taskStatus == PENDING)
	{
		bdNChar8 buf[40];
		printDataRate(buf, 40);
		description += buf;
	}	
}


void Simulation::generateDownloadDescriptionUser2(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Download friend's file",			// READY
		"Downloading friend's file",			// PENDING
		"Downloaded friend's file",			// DONE
		"friend's File download failed",		// FAILED
		"friend's File download skipped"		// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if (m_taskStatus == PENDING)
	{
		bdNChar8 buf[40];
		printDataRate(buf, 40);
		description += buf;
	}		
}

void Simulation::startDownload()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_ERR("sim","Could not acquire handle to content streaming service. Are you still connected to LSG?");
		m_taskStatus = FAILED;
		return;
	}

	bdUInt16 fileSlot;
	if (m_currentUserID == m_userID)
	{
		// download the file we just uploaded
		fileSlot = m_fileSlot;
		m_checkFileData = true;
	}
	else
	{
		// download the file we got meta data for
		fileSlot = m_listResultsByOwner[0].m_fileSlot;
		m_checkFileData = false;
	}

#ifdef BD_PLATFORM_WIN32
	contentStreaming->enableProgressMeter(true);
	contentStreaming->enableVerboseOutput(true);
#endif

#ifdef USE_CUSTOM_CALLBACKS_FOR_HTTP
	m_bytesDownloaded = 0;
	initDownload();
	m_remoteTask = contentStreaming->downloadFileBySlot(m_currentUserID, fileSlot, this, &m_downloadedFileMetaData);
#else
	initDownload();
	m_remoteTask = contentStreaming->downloadFileBySlot(m_currentUserID, fileSlot, m_fileData, m_fileSize, &m_downloadedFileMetaData);
#endif

	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if (m_taskStatus == PENDING)
	{
		BD_INFO("sim","Downloading file from user %d slot %d of size %d commenced.", m_currentUserID, fileSlot, m_fileSize);
		resetTimers();
	}
	else
	{
		BD_ERR("sim","Downloading file from user %d slot %d of size %d FAILED to start.", m_currentUserID, fileSlot, m_fileSize);
	}
}

void Simulation::pumpDownload()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();
	
	if (contentStreaming == BD_NULL)
	{
		BD_ERR("sim","Could not access bdContentStreaming object");
		return;
	}

	contentStreaming->pump();

#ifdef BD_PLATFORM_XENON
	if ( m_stopwatch.getElapsedTimeInSeconds() > m_lastTimeSampled + 1.0f)
	{
		m_lastTimeSampled = m_stopwatch.getElapsedTimeInSeconds();
		bdNChar8 buf[40];
		printDataRate(buf, 40);
		BD_INFO("sim", buf);
	}
#endif

	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE && m_fileData != BD_NULL && m_checkFileData)
	{
		bdUInt resSize = 24;
		bdUByte8 rtnHashFile[24];
		m_tigerHash.hash(
			static_cast<bdUByte8*>(m_fileData),
			m_fileSize,
			rtnHashFile,
			resSize);

		if (bdMemcmp(m_hashFile, rtnHashFile, 24) != 0)
		{
			BD_INFO("sim","Tiger Hash not correct");
		}
		else
		{
			BD_INFO("sim","Hash of downloaded file data matches that of uploaded file data.");
		}
	}

	if(m_taskStatus == DONE)
	{
		m_fileListString = "";

		adjustQuotaPostDownload(m_fileSize);
		appendFileToList(&m_downloadedFileMetaData);
		regenerateQuota();

		BD_INFO("sim","Downloading of file of size %d bytes complete", m_fileSize);
	}
}

void Simulation::generateDownloadByIDDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Download file by ID",		// READY
		"Downloading file by ID",	// PENDING
		"Downloaded file by ID",	// DONE
		"File download by ID failed",	// FAILED
		"File download by ID skipped"	// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if (m_taskStatus == PENDING)
	{
		bdNChar8 buf[40];
		printDataRate(buf, 40);
		description += buf;
	}	
}

void Simulation::startDownloadByID()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_ERR("sim","Could not acquire handle to content streaming service. Are you still connected to LSG?");
		m_taskStatus = FAILED;
		return;
	}

	// download the file specified by the id in the m_listResultsByOwner we have acquired by calling
	// listFilesByOwner (either in list files by owner / get meta data by name)
#ifdef BD_PLATFORM_WIN32
	contentStreaming->enableProgressMeter(false);
	contentStreaming->enableVerboseOutput(false);
#endif

#ifdef USE_CUSTOM_CALLBACKS_FOR_HTTP
	m_bytesDownloaded = 0;
	initDownload();
	m_remoteTask = contentStreaming->download(m_listResultsByOwner[0].m_fileID, this, &m_downloadedFileMetaData);
#else
	initDownload();
	m_remoteTask = contentStreaming->download(m_listResultsByOwner[0].m_fileID, m_fileData, m_fileSize, &m_downloadedFileMetaData);
#endif

	m_checkFileData = true;

	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if (m_taskStatus == PENDING)
	{
		resetTimers();
		BD_INFO("sim","Downloading file id %d of size %d commenced.", m_listResultsByOwner[0].m_fileID, m_fileSize);
	}
	else
	{
		BD_ERR("sim","Downloading file id %d of size %d FAILED to start.", m_listResultsByOwner[0].m_fileID, m_fileSize);
	}
}

void Simulation::resetTimers()
{
	m_stopwatch.reset();
	m_stopwatch.start();
	m_lastTimeSampled = 0.0f;
}


void Simulation::pumpDownloadByID()
{
	pumpDownload();
}

void Simulation::generateDownloadByIDDescription_Incomplete(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Download (incomplete) file by ID",		// READY
		"Downloading (incomplete) file by ID",	// PENDING
		"Downloaded (incomplete) file by ID",	// DONE
		"File download by ID (incomplete) failed",	// FAILED
		"File download by ID (incomplete) skipped"	// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if (m_taskStatus == PENDING)
	{
		bdNChar8 buf[40];
		printDataRate(buf, 40);
		description += buf;
	}	
}

void Simulation::startDownloadByID_Incomplete()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_ERR("sim","Could not acquire handle to content streaming service. Are you still connected to LSG?");
		m_taskStatus = FAILED;
		return;
	}

	// download the file specified by the id in the m_listResultsByOwner we have acquired by calling
	// listFilesByOwner (either in list files by owner / get meta data by name)
#ifdef BD_PLATFORM_WIN32
	contentStreaming->enableProgressMeter(false);
	contentStreaming->enableVerboseOutput(false);
#endif

	bdUInt32 startByte(0);
	bdUInt32 endByte(INCOMPLETE_FILE_SIZE_HTTP-1);
	initDownload();
	m_remoteTask = contentStreaming->download(m_listResultsByOwner[0].m_fileID, m_fileData, m_fileSize, &m_downloadedFileMetaData, startByte, endByte);

	m_checkFileData = true;
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if (m_taskStatus == PENDING)
	{
		resetTimers();
		BD_INFO("sim","Downloading file id %d of size %d commenced.", m_listResultsByOwner[0].m_fileID, m_fileSize);
	}
	else
	{
		BD_ERR("sim","Downloading file id %d of size %d FAILED to start.", m_listResultsByOwner[0].m_fileID, m_fileSize);
	}
}

void Simulation::pumpDownloadByID_Incomplete()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_ERR("sim","Could not access bdContentStreaming object");
		return;
	}

	contentStreaming->pump();

	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

#ifdef BD_PLATFORM_XENON
	if(m_taskStatus == PENDING)
	{
		if ( m_stopwatch.getElapsedTimeInSeconds() > m_lastTimeSampled + 1.0f)
		{
			m_lastTimeSampled = m_stopwatch.getElapsedTimeInSeconds();
			bdNChar8 buf[40];
			printDataRate(buf, 40);
			BD_INFO("sim", buf);
		}
	}
#endif

	if(m_taskStatus == DONE && m_fileData != BD_NULL && m_checkFileData)
	{
		bdUInt resSize = 24;
		bdUByte8 rtnHashFile[24];
		m_tigerHash.hash(
			static_cast<bdUByte8*>(m_fileData),
			INCOMPLETE_FILE_SIZE_HTTP,
			rtnHashFile,
			resSize);

		if (bdMemcmp(m_hashIncompleteFile, rtnHashFile, 24) != 0)
		{
			BD_INFO("sim","Tiger Hash not correct");
		}
		else
		{
			BD_INFO("sim","Hash of downloaded file data matches that of uploaded file data.");
		}
	}

	if(m_taskStatus == DONE)
	{
		m_fileListString = "";

		adjustQuotaPostDownload(m_fileSize);
		appendFileToList(&m_downloadedFileMetaData);
		regenerateQuota();

		BD_INFO("sim","Downloading of file of size %d bytes complete", m_fileSize);
	}
}

void Simulation::generateDownloadByIDDescription_Resume(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Resume file download by ID",		// READY
		"Resuming download file by ID",	// PENDING
		"Resumed and downloaded file by ID",	// DONE
		"Resume file download ID failed",	// FAILED
		"Resume file download ID skipped"	// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

	if (m_taskStatus == PENDING)
	{
		bdNChar8 buf[40];
		printDataRate(buf, 40);
		description += buf;
	}	
}

void Simulation::startDownloadByID_Resume()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_ERR("sim","Could not acquire handle to content streaming service. Are you still connected to LSG?");
		m_taskStatus = FAILED;
		return;
	}

	// download the file specified by the id in the m_listResultsByOwner we have acquired by calling
	// listFilesByOwner (either in list files by owner / get meta data by name)
#ifdef BD_PLATFORM_WIN32
	contentStreaming->enableProgressMeter(false);
	contentStreaming->enableVerboseOutput(false);
#endif

	bdUInt32 startByte(INCOMPLETE_FILE_SIZE_HTTP);
	bdUInt32 endByte(0);
	initDownload();
	m_remoteTask = contentStreaming->download(m_listResultsByOwner[0].m_fileID, m_fileData, m_fileSize, &m_downloadedFileMetaData, startByte, endByte);

	m_checkFileData = true;

	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if (m_taskStatus == PENDING)
	{
		resetTimers();
		BD_INFO("sim","Downloading file id %d of size %d commenced.", m_listResultsByOwner[0].m_fileID, m_fileSize);
	}
	else
	{
		BD_ERR("sim","Downloading file id %d of size %d FAILED to start.", m_listResultsByOwner[0].m_fileID, m_fileSize);
	}
}

void Simulation::generateRemoveFileDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Remove file",			// READY
		"Removing file",		// PENDING
		"Removed file",			// DONE
		"Remove file failed",	// FAILED
		"Remove file skipped"	// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

}

void Simulation::startRemoveFile()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_ERR("sim","Could not acquire handle to content streaming service. Are you still connected to LSG?");
		m_taskStatus = FAILED;
		return;
	}

	// remove the last file we uploaded
	m_remoteTask = contentStreaming->removeFile(m_fileSlot);
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	// advance the file slot so we can call it repeatedly
	m_fileSlot++;
	if (m_fileSlot > MAX_SLOTS_PER_USER)
	{
		m_fileSlot = 0;
	}

	if(m_taskStatus == PENDING)
	{

		BD_INFO("sim","Removal of file commenced");
	}
	else
	{
		BD_INFO("sim","Removal of file failed to start");
	}
}

void Simulation::pumpRemoveFile()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();
	if (contentStreaming == BD_NULL)
	{
		BD_ERR("sim","Could not access bdContentStreaming object");
		return;
	}

	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	switch(m_taskStatus)
	{
	case PENDING:
		{
			contentStreaming->pump();
		}
		break;
	case DONE:
		{
			BD_INFO("sim","Removal of file complete");
			m_fileListString = "";
			adjustQuotaPostRemove(m_fileSize);
			adjustMyFilesPostRemove(m_fileSlot);
			regenerateMyFileList();
			regenerateQuota();
		}
		break;
	case FAILED:
		BD_INFO("sim","Removal of file failed");
		break;
	default:
		break;
	}
}


void Simulation::generateGetQuotaDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Get Quota",			// READY
		"Getting quota",		// PENDING
		"Got Quota",			// DONE
		"Get quota failed",		// FAILED
		"Get quota skipped"		// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

}

void Simulation::startGetQuota()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_ERR("sim","Could not acquire handle to content streaming service. Are you still connected to LSG?");
		m_taskStatus = FAILED;
		return;
	}

	// update m_usage with server's view of quota
	m_remoteTask = contentStreaming->getQuotaUsage(&m_usage);
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == PENDING)
	{
		BD_INFO("sim","Getting quota commenced");
	}
	else
	{
		BD_INFO("sim","Getting quota failed to start");
	}
}

void Simulation::pumpGetQuota()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();
	if (contentStreaming == BD_NULL)
	{
		BD_ERR("sim","Could not access bdContentStreaming object");
		return;
	}

	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	switch(m_taskStatus)
	{
	case DONE:
		m_fileListString = "";
		regenerateQuota(true);
		BD_INFO("sim","Get Quota complete");
		break;
	case FAILED:
		BD_INFO("sim","Get Quota failed");
		break;
	default:
		break;
	}
}

void Simulation::generateGetFileDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Get file metaData by name",			// READY
		"Get file metaData by name",		// PENDING
		"Got file metaData by name",			// DONE
		"File get metaData failed",	// FAILED
		"File get metaData skipped"	// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

}


void Simulation::generateGetFileDescriptionUser2(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Get friend's file metaData by name",			// READY
		"Get friend's file metaData by name",		// PENDING
		"Got friend's file metaData by name",			// DONE
		"friend's File get metaData failed",	// FAILED
		"friend's File get metaData skipped"	// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);

}

void Simulation::startGetFileMetaData()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		m_taskStatus = FAILED;
		return;
	}

	bdUInt32 startDate = 0;
	bdUInt16 category = PHOTO;
	m_remoteTask = contentStreaming->listFilesByOwner(m_currentUserID, startDate, category, m_listResultsByOwner, MAX_NUM_RESULTS, 0, PROFILE_PIC_FILE_NAME);
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
}

void Simulation::pumpGetFileMetaData()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if (m_taskStatus == DONE)
	{
		m_fileListString = "";

		if (m_currentUserID == m_userID)
		{
			adjustMyFilesPostList(m_remoteTask->getNumResults(), m_listResultsByOwner);
			regenerateMyFileList();
		}
		else
		{
			regenerateFileList(m_listResultsByOwner);
		}
	}
}

void Simulation::generateGetFilesThumbnailsDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Get Files Thumbnails",			// READY
		"Getting Files Thumbnails",		// PENDING
		"Get Files Thumbnails DONE",		// DONE
		"Get Files Thumbnails FAILED",	// FAILED
		"Get Files Thumbnails SKIPPED"	// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}
void Simulation::startGetFilesThumbnails()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_WARN("warn","not connected to ContentStreaming server");
		m_taskStatus = FAILED;
		return;
	}

	bdEntityID thumbnailFileIDs[MAX_NUM_RESULTS];
	
	// only get thumbnails for files we have metadata for (and therefore have the thumbnail fileID)
	bdUInt16 numResults(0);
	for (bdUInt i=0; i<MAX_SLOTS_PER_USER; i++)
	{
		if (m_slotIsUsed[i])
		{
			thumbnailFileIDs[numResults] = m_myFiles[i].m_thumbNailFileID;
			// allocate thumbnail buffers of specified sizes
			bdUByte8* thumbDataBuffer = new bdUByte8[m_myFiles[i].m_thumbNailDataSize];
			m_thumbnails[numResults].setFileData(thumbDataBuffer, m_myFiles[i].m_thumbNailDataSize);

			numResults++;

			BD_INFO(BD_LOG_LEVEL, "Getting Thumbnail for file slot %i by file ID %d of max size %d.", i, m_myFiles[i].m_thumbNailFileID, m_myFiles[i].m_thumbNailDataSize);
		}
	}

	BD_INFO(BD_LOG_LEVEL, "Getting %d Thumbnails", numResults); 
	m_remoteTask = contentStreaming->getThumbNailData(numResults, 
													  thumbnailFileIDs, 
													  m_thumbnails);

	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
}
void Simulation::pumpGetFilesThumbnails()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{
		m_fileListString = "";
	}
}

void Simulation::regenerateFileListPostUpload()
{
	m_fileListString = "";
	const bdUInt metaDataStrLen = 128;
	bdNChar8 metaDataStr[metaDataStrLen];
	bdSnprintf(metaDataStr, metaDataStrLen, "\nName: %s Slot: %u Size: %u" ,
			m_myFileNameHTTP,
			m_fileSlot,
			m_fileSize);

	m_fileListString += metaDataStr;
}

void Simulation::regenerateFileList(bdFileMetaData* metaData)
{
	m_fileListString = "";
	bdUInt i;
	m_numFileResults = m_remoteTask->getNumResults();
	for(i = 0; i < m_numFileResults; i++)
	{
		appendFileToList(&metaData[i]);
	}
}

void Simulation::regenerateFileList(bdStatsInfo* downloadCountStatsInfo)
{
	m_fileListString = "";
	m_numFileResults = m_remoteTask->getNumResults();

	for (bdUInt i(0); i<m_numFileResults; i++)
	{
		const bdUInt metaDataStrLen = 128;
		bdNChar8 metaDataStr[metaDataStrLen];
		bdSnprintf(metaDataStr, metaDataStrLen, "\nfileID: %I64d fileName:(%s) ldrbrd: %d rank: %d rating: %d", 
				   downloadCountStatsInfo[i].m_entityID,
				   downloadCountStatsInfo[i].m_entityName,
				   downloadCountStatsInfo[i].m_leaderboardID,
				   downloadCountStatsInfo[i].m_rank,
				   downloadCountStatsInfo[i].m_rating);
		m_fileListString += metaDataStr;

		BD_INFO(BD_LOG_LEVEL, metaDataStr);
	}
}

void Simulation::regenerateMyFileList()
{
	m_fileListString = "";
	bdUInt i;
	for(i = 0; i < MAX_SLOTS_PER_USER; i++)
	{
		if (m_slotIsUsed[i])
		{
			appendFileToList(&m_myFiles[i]);
		}
	}
}

void Simulation::appendFileToList(bdFileMetaData *metaData)
{
	const bdUInt metaDataStrLen = 128;
	bdNChar8 metaDataStr[metaDataStrLen];
	if (metaData->m_fileID >= 0)
	{
		bdUInt offset = bdSnprintf(metaDataStr, metaDataStrLen, "\nSlot: %u Name: %s Size: %u, Owner: %I64d", 
								   metaData->m_fileSlot,
								   metaData->m_fileName,
								   metaData->m_fileSize,
								   metaData->m_ownerID);

		for (bdUInt t=0; t<metaData->m_numTags && offset < metaDataStrLen; t++)
		{
			if (t == 0)
			{
				offset += bdSnprintf(metaDataStr+offset, metaDataStrLen-offset, "\nTags: (%u,%u) ", 
					metaData->m_tags[t].m_priTag,
					metaData->m_tags[t].m_secTag);
			}
			else if (t%5 == 0)
			{
				offset += bdSnprintf(metaDataStr+offset, metaDataStrLen-offset, "(%u,%u)\n", 
					metaData->m_tags[t].m_priTag,
					metaData->m_tags[t].m_secTag);
			}
			else
			{
				offset += bdSnprintf(metaDataStr+offset, metaDataStrLen-offset, "(%u,%u) ", 
					metaData->m_tags[t].m_priTag,
					metaData->m_tags[t].m_secTag);
			}
		}
	}
	else
	{
		bdSnprintf(metaDataStr, metaDataStrLen, "Slot: %u Name: %s Size: %u\n", 
			metaData->m_fileSlot,
			metaData->m_fileName,
			metaData->m_fileSize);
	}

	m_fileListString += metaDataStr;
}


void Simulation::regenerateQuota(bdBool printLimits)
{
	m_fileListString += '\n';

	// header
	{
		bdNChar8 buf[128];
		bdSnprintf(buf, 128, "\nQuota: %10s, %10s, %10s, %10s", 
			"Space",
			"Files",
			"Upload",
			"Download");

		m_fileListString += buf;
	}

	// usage
	{
		bdNChar8 buf[128];
		bdSnprintf(buf, 128, "\nUsage: %10.d, %10.d, %10.d, %10.d", 
			m_usage.m_usedStorageSpace, 
			m_usage.m_usedNumFiles,
			m_usage.m_usedDailyUploadBW,
			m_usage.m_usedDailyDownloadBW);

		m_fileListString += buf;
	}

	// limits
	if (printLimits)
	{
		bdNChar8 buf[128];
		bdSnprintf(buf, 128, "\nLimit: %10.d, %10.d, %10.d, %10.d", 
			m_usage.m_maxStorageSpace,
			m_usage.m_maxNumFiles,
			m_usage.m_maxDailyUploadBW,
			m_usage.m_maxDailyDownloadBW);

		m_fileListString += buf;
	}
}

void Simulation::adjustQuotaPostUpload(bdUInt32 bytesUploaded,bdUInt32 previousFileSize)
{
	m_usage.m_usedNumFiles += 1;
	m_usage.m_usedStorageSpace += (bytesUploaded-previousFileSize);
	m_usage.m_usedDailyUploadBW += bytesUploaded;
}

void Simulation::adjustQuotaPostDownload(bdUInt32 bytesDownloaded)
{
	m_usage.m_usedDailyDownloadBW += bytesDownloaded;
}

void Simulation::adjustQuotaPostRemove(bdUInt32 bytesRemoved)
{
	m_usage.m_usedNumFiles -= 1;
	m_usage.m_usedStorageSpace -= bytesRemoved;

	if (m_usage.m_usedStorageSpace < 0)
	{
		m_usage.m_usedStorageSpace = 0;
	}
}

void Simulation::adjustMyFilesPostUpload(bdFileMetaData *uploadedFile)
{
	bdUInt32 slotToUse = uploadedFile->m_fileSlot;
	if (slotToUse < MAX_SLOTS_PER_USER)
	{	
		if (!m_slotIsUsed[slotToUse])
		{
			m_slotIsUsed[slotToUse] = true;
			BD_INFO("info","Uploaded to unused slot");
		}
		else
		{
			BD_INFO("info","Uploaded to slot in use (file update)");
		}

		m_myFiles[slotToUse] = *uploadedFile;
	}
}

void Simulation::adjustMyFilesPostRemove(bdUInt32 deletedSlot)
{
	if (deletedSlot < MAX_SLOTS_PER_USER)
	{
		m_slotIsUsed[deletedSlot] = false;
	}
}

void Simulation::adjustMyFilesPostReset()
{
	for (bdUInt i=0; i<MAX_SLOTS_PER_USER; i++)
	{
		// hacky way of detecting null slots
		m_slotIsUsed[i] = false;
	}
}

void Simulation::adjustMyFilesPostList(bdUInt numMetaDatas, bdFileMetaData *metaDatas)
{
	for (bdUInt i=0; i<numMetaDatas; i++)
	{
		 bdUInt16 slot = metaDatas[i].m_fileSlot;
		 m_myFiles[slot] = metaDatas[i]; // copy
		 m_slotIsUsed[slot] = true;
	}
}


void Simulation::generateListFilesByOwnerDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"List files by owner (List my files)",	// READY
		"Listing files by owner",		// PENDING
		"Files by owner listed",		// DONE
		"List files by owner failed",	// FAILED
		"List files by owner skipped"	// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::generateListFilesByOwnerDescriptionUser2(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"List files by owner (List friend's files)",	// READY
		"Listing friend's files",		// PENDING
		"Files by User2 listed",		// DONE
		"List friend's files failed",	// FAILED
		"List friend's files skipped"	// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startListFilesByOwner()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_WARN("warn","not connected to ContentStreaming server");
		m_taskStatus = FAILED;
		return;
	}

	bdUInt32 startDate = 0;
	bdUInt16 category = PHOTO;
	bdUInt16 offset = 0;

	// n.b. it's possible to specify the category here and only return files of that category. The following specifies category PHOTO.
	m_remoteTask = contentStreaming->listFilesByOwner(m_currentUserID, startDate, category, m_listResultsByOwner, MAX_SLOTS_PER_USER, offset, BD_NULL);
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
}

void Simulation::pumpListFilesByOwner()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if (m_taskStatus == DONE)
	{
		m_fileListString = "";

		if (m_currentUserID == m_userID)
		{
			adjustMyFilesPostList(m_remoteTask->getNumResults(), m_listResultsByOwner);
			regenerateMyFileList();
		}
		else
		{
			regenerateFileList(m_listResultsByOwner);
		}
	}
}


void Simulation::generateReportContentDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Report dodgy content by fileID",	// READY
		"Reporting dodgy content by fileID",		// PENDING
		"Reported dodgy content by fileID",		// DONE
		"Reported dodgy content by fileID FAILED",	// FAILED
		"Reported dodgy content by fileID SKIPPED"	// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}
void Simulation::startReportContent()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_WARN("warn","not connected to ContentStreaming server");
		m_taskStatus = FAILED;
		return;
	}

	// m_listResultsByOwner[0].m_fileID should have been filled out in ListFileByOwner.
	// report our own file!
	m_remoteTask = contentStreaming->reportContent(m_uploadedFileID.m_fileID);
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
}
void Simulation::pumpReportContent()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if (m_taskStatus == DONE)
	{
		m_fileListString = "";

		if (m_currentUserID == m_userID)
		{
			adjustMyFilesPostList(m_remoteTask->getNumResults(), m_listResultsByOwner);
			regenerateMyFileList();
		}
		else
		{
			regenerateFileList(m_listResultsByOwner);
		}
	}
}

void Simulation::generateListFilesByDownloadCountDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"List Files by Download Count",				// READY
		"Listing Files by Download Count",			// PENDING
		"Listed Files by Download Count",			// DONE
		"Listed Files by Download Count FAILED",	// FAILED
		"Listed Files by Download Count SKIPPED"	// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}
void Simulation::startListFilesByDownloadCount()
{
	bdStats *const stats = m_lobbyService->getStats();
	bdUInt firstRank = 0;
	m_remoteTask = stats->readStatsByRank(DownloadCountLeaderboard_PHOTO, 
										  firstRank, 
										  m_contentStreamingStatsInfo, 
										  MAX_NUM_DISPLAY_RANKS);
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
}
void Simulation::pumpListFilesByDownloadCount()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{
		m_taskStatus = DONE;

		bdUInt numResults = m_remoteTask->getNumResults();
		if (numResults == 0)
		{
			BD_WARN(BD_LOG_LEVEL, "readStatsByRank(BD_DOWNLOADCOUNT_LBID_PHOTO) returned no results");
			m_fileListString = "";
		}
		else
		{
			// todo: causing issue on p3s - review
			//regenerateFileList(m_contentStreamingStatsInfo);
		}

		// todo: causing issue on p3s - review
		//for (bdUInt i(0); i<numResults; i++)
		//{
		//	bdNChar8 entityName[64];
		//	if (bdStrcmp(m_contentStreamingStatsInfo[i].m_entityName, ""))
		//	{
		//		bdSnprintf(entityName, 64, "none");
		//	}
		//	else
		//	{
		//		bdSnprintf(entityName, 64, m_contentStreamingStatsInfo->m_entityName);
		//	}

		//	bdUInt leaderboardID = m_contentStreamingStatsInfo[i].m_leaderboardID;
		//	bdUInt rank = m_contentStreamingStatsInfo[i].m_rank;
		//	bdUInt rating = m_contentStreamingStatsInfo[i].m_rating;
		//	
		//	BD_INFO(BD_LOG_LEVEL, "entity: %I64d (%s) ldrbrd: %d rank: %d rating: %d, ownerData(%I64d, %s)", 
		//			m_contentStreamingStatsInfo[i].m_entityID, 
		//			entityName,
		//			leaderboardID,
		//			rank,
		//			rating,
		//			m_contentStreamingStatsInfo[i].m_fileOwnerID,
		//			m_contentStreamingStatsInfo[i].m_fileOwnerName);
		//}
		
	}
	if((m_taskStatus == DONE) || (m_taskStatus == FAILED))
	{
		m_remoteTask = BD_NULL;
	}
}


// get file description
void Simulation::generateGetFileByIDDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Get file metaData by ID",			// READY
		"Getting file metaData by ID",		// PENDING
		"Get file metaData by ID finished",		// DONE
		"Get file metaData by ID failed",	// FAILED
		"Get file metaData by ID skipped"	// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startGetFileMetaDataByID()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_WARN("warn","not connected to ContentStreaming server");
		m_taskStatus = FAILED;
		return;
	}

	if (contentStreaming == BD_NULL)
	{
		m_taskStatus = FAILED;
		return;
	}
	
	bdUInt numFileIDs = 1;
	bdUInt64 fileIDs[1];
	fileIDs[0] = m_uploadedFileID.m_fileID;

	m_remoteTask = contentStreaming->getFileMetaDataByID(numFileIDs,
														 fileIDs,
														 m_listResultsByID);

	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

}

void Simulation::pumpGetFileMetaDataByID()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{
		m_fileListString = "";
		regenerateFileList(m_listResultsByID);
	}
}


void Simulation::generateListAllPublisherFilesDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"List all publisher files",			// READY
		"Listing all publisher files",		// PENDING
		"All publisher files listed",		// DONE
		"List all publisher files failed",	// FAILED
		"List all publisher files skipped"	// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startListAllPublisherFiles(bdUInt16 category)
{
	bdContentStreaming *const contentStreaming = getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_WARN("warn","not connected to ContentStreaming server");
		m_taskStatus = FAILED;
		return;
	}

	bdUInt32 startDate = 0;
	bdUInt16 maxResults = MAX_NUM_RESULTS;
	m_fileListString = "";
	
	// n.b. it's possible to specify the category here and only return files of that category
	m_remoteTask = contentStreaming->listAllPublisherFiles(startDate, category, m_publisherFiles, maxResults);
	
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);
}

void Simulation::pumpListAllPublisherFiles()
{
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if (m_taskStatus == DONE)
	{
		m_fileListString = "";
		regenerateFileList(m_publisherFiles);
	}
}

void Simulation::startDownloadPublisherFile()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_ERR("sim","Could not acquire handle to content streaming service. Are you still connected to LSG?");
		m_taskStatus = FAILED;
		return;
	}

	bdUInt64 fileID = m_publisherFiles[0].m_fileID;

#ifdef BD_PLATFORM_WIN32
	contentStreaming->enableProgressMeter(true);
	contentStreaming->enableVerboseOutput(true);
#endif

#ifdef USE_CUSTOM_CALLBACKS_FOR_HTTP
	m_bytesDownloaded = 0;
	initDownload();
	m_remoteTask = contentStreaming->downloadPublisherFile(fileID, this, &m_downloadedFileMetaData);
#else
	initDownload();
	m_remoteTask = contentStreaming->downloadPublisherFile(fileID, m_fileData, m_fileSize, &m_downloadedFileMetaData);
#endif
	
	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if (m_taskStatus == PENDING)
	{
		BD_INFO("sim","Downloading publisher file %d of size %d commenced.", fileID, m_fileSize);
		resetTimers();
	}
	else
	{
		BD_ERR("sim","Downloading publisher file %d of size %d FAILED to start.", fileID, m_fileSize);
	}
}

void Simulation::pumpDownloadPublisherFile()
{
	m_checkFileData	= false;
	pumpDownload();
}

void Simulation::generateDownloadPublisherFileDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Download publisher file",			// READY
		"Downloading publisher file",		// PENDING
		"Downloaded publisher file",		// DONE
		"Downloading publisher file failed",	// FAILED
		"Downloading publisher file skipped"	// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}


void Simulation::startDownloadApplePurchasedFile()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_ERR("sim","Could not acquire handle to content streaming service. Are you still connected to LSG?");
		m_taskStatus = FAILED;
		return;
	}

	bdUInt64 fileID = m_publisherFiles[0].m_fileID;
	m_fileSize = FILE_SIZE_HTTP;

#ifdef BD_PLATFORM_WIN32
	contentStreaming->enableProgressMeter(true);
	contentStreaming->enableVerboseOutput(true);
#endif

#ifdef USE_CUSTOM_CALLBACKS_FOR_HTTP
	m_bytesDownloaded = 0;
	initDownload();
	m_remoteTask = contentStreaming->downloadApplePurchasedFile(fileID, itunes_receipt, bdStrlen(itunes_receipt), this, &m_downloadedFileMetaData);
#else
	initDownload();
	m_remoteTask = contentStreaming->downloadApplePurchasedFile(fileID, itunes_receipt, bdStrlen(itunes_receipt), m_fileData, m_fileSize, &m_downloadedFileMetaData);
#endif

	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if (m_taskStatus == PENDING)
	{
		BD_INFO("sim","Downloading publisher file %d of size %d commenced.", fileID, m_fileSize);
		resetTimers();
	}
	else
	{
		BD_ERR("sim","Downloading publisher file %d of size %d FAILED to start.", fileID, m_fileSize);
	}
}

void Simulation::pumpDownloadApplePurchasedFile()
{
	bdContentStreaming *const contentStreaming = getContentStreaming();

	if (contentStreaming == BD_NULL)
	{
		BD_ERR("sim","Could not access bdContentStreaming object");
		return;
	}

	contentStreaming->pump();

#ifdef BD_PLATFORM_XENON
	if ( m_stopwatch.getElapsedTimeInSeconds() > m_lastTimeSampled + 1.0f)
	{
		m_lastTimeSampled = m_stopwatch.getElapsedTimeInSeconds();
		bdNChar8 buf[40];
		printDataRate(buf, 40);
		BD_INFO("sim", buf);
	}
#endif

	m_taskStatus = CheckRemoteTaskStatus(m_remoteTask);

	if(m_taskStatus == DONE)
	{
		m_fileListString = "";

		adjustQuotaPostDownload(m_fileSize);
		appendFileToList(&m_downloadedFileMetaData);
		regenerateQuota(); 

		BD_INFO("sim","Downloading publisher file of size %d bytes complete", m_fileSize);
	}
}

void Simulation::generateDownloadApplePurchasedFileDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] =
	{
		"Download apple purchased file",			// READY
		"Downloading apple purchased file",		// PENDING
		"Downloaded apple purchased file",		// DONE
		"Downloading apple purchased file failed",	// FAILED
		"Downloading apple purchased file skipped"	// SKIPPED
	};

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::lobbyServiceDisconnectDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Disconnect from Lobby Service",			// READY
											"Disconnecting from Lobby Service",			// PENDING
											"Disconnected from Lobby Service",			// DONE
											"Disconnect from Lobby Service Failed",		// FAILED
											"Disconnect from Lobby Service Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::startLobbyServiceDisconnect()
{
	// fail safe
	m_taskStatus = FAILED;
	if(m_lobbyService)
	{
		m_lobbyService->disconnect();
		m_taskStatus = PENDING;
	}
}

void Simulation::pumpLobbyServiceDisconnect()
{
	// fail safe
	m_taskStatus = FAILED;
	if(m_lobbyService)
	{
		const bdLobbyConnection::Status status = m_lobbyService->getStatus();
		switch(status)
		{
			case bdConnection::BD_DISCONNECTED:
			case bdConnection::BD_NOT_CONNECTED:
			{
				m_taskStatus = DONE;
				m_userID = 0;
			}
			break;
			case bdConnection::BD_CONNECTING:
			case bdConnection::BD_CONNECTED:
			{
				m_taskStatus = FAILED;
			}
			break;
			case bdConnection::BD_DISCONNECTING:
			{
				m_taskStatus = PENDING;
			}
			break;
		}
	}
}
void Simulation::stopNetworkDescription(bdString& description)
{
	const bdNChar8 *const descriptions[] = {"Stop Network",					// READY
											"Stopping Network",				// PENDING
											"Stopped Network",				// DONE
											"Stopping Network Failed",		// FAILED
											"Stopping Network Skipped"};	// SKIPPED

	const bdUInt numDescriptions = sizeof(descriptions) / sizeof(bdNChar8*);
	IndexStrings(descriptions, numDescriptions, m_taskStatus, description);
}

void Simulation::stopNetwork()
{
	bdNetImpl *const net = bdNet::getInstance();
	net->stop();
	if(net->getStatus() == bdNetImpl::BD_NET_STOPPED)
	{
		bdExamplePlatformInit::quit();
		m_taskStatus = DONE;
	}
	else
	{
		m_taskStatus = FAILED;
	}
}

bdContentStreaming* Simulation::getContentStreaming()
{
	if (m_lobbyService != BD_NULL)
	{
		return m_lobbyService->getContentStreaming();
	}

	return BD_NULL;
}

void Simulation::printDataRate(bdNChar8 *buf, bdUInt32 bufSize)
{
	bdContentStreaming *const contentStreaming = getContentStreaming();
	bdUInt32 bytesTransfered;
	bdFloat32 dataRate;
	contentStreaming->checkProgress(bytesTransfered,dataRate);
	bytesTransfered /= 1024;
	dataRate /= 1024.0f;

	bdSnprintf(buf, bufSize, "\n %d KB (rate: %.1f KB/sec)", bytesTransfered, dataRate);

	// force screen text update:
	m_changedScreenText = true;
}

void Simulation::initUpload(bdUInt32 uploadSize)
{
	m_bytesUploaded = 0;
	m_uploadSize = uploadSize;
	m_dataRateStopwatch.start();
}

void Simulation::initDownload()
{
	m_dataRateStopwatch.start();
}

#ifdef RATE_LIMIT_HTTP

bdUInt Simulation::handleDownload(void* data, bdUInt32 dataSize)
{
	if (m_bytesDownloaded == 0)
	{
		m_dataRateStopwatch.start();
	}

	if (m_bytesDownloaded + dataSize > m_fileSize)
	{
		BD_ERR(BD_LOG_LEVEL, "Not enough buffer space to save file. Check FILE_SIZE_HTTP");
		// signal that we're done by returning 0
		return 0;
	}

	bdMemcpy(m_fileData + m_bytesDownloaded, data, dataSize);
	m_bytesDownloaded += dataSize;

	// limit the rate of the download by sleeping until we are ready for more data
	bdFloat32 desiredRate = static_cast<bdFloat32>(MAX_DOWNLOAD_RATE_KBYTES_PER_SEC * 1024);
	bdFloat32 actualRate = m_bytesDownloaded / m_dataRateStopwatch.getElapsedTimeInSeconds();

	bdFloat32 desiredElapsedTime = static_cast<bdFloat32>(m_bytesDownloaded) / desiredRate;
	bdFloat32 actualElapsedTime = m_dataRateStopwatch.getElapsedTimeInSeconds();

	BD_INFO(BD_LOG_LEVEL, "downloading - rate: %.1f KBps desired rate: %.1f KBps total: %i bytes", 
		actualRate / 1024.0, desiredRate / 1024.0, m_bytesDownloaded);

	if (desiredElapsedTime > actualElapsedTime)
	{
		bdUInt32 waitTime = static_cast<bdUInt32>(1000.0 * (desiredElapsedTime-actualElapsedTime));
		bdPlatformTiming::sleep(waitTime);
	}

	return dataSize;
}

bdUInt Simulation::handleUpload(void* uploadBuffer, bdUInt32 bufferSize, bdUInt32 bytesUploaded)
{
	if (bytesUploaded == 0)
	{
		m_prevCallbackTime = 0;
	}

	bdUInt32 bytesRemaining = m_uploadSize - bytesUploaded;

	// copy to buffer
	bdUInt32 dataSize = BD_MIN(bufferSize, bytesRemaining);
	// we can make our data rate throttling more accurate by selecting a lower value of
	// MAX_UPLOAD_CHUNK_SIZE
	dataSize = BD_MIN(dataSize, MAX_UPLOAD_CHUNK_SIZE);
	if (dataSize > 0)
	{
		bdMemcpy(uploadBuffer, m_fileData + bytesUploaded, dataSize);
		m_bytesUploaded = bytesUploaded;
	}

	// limit the rate of the upload by sleeping until we are ready for more data
	bdFloat32 desiredRate = static_cast<bdFloat32>(MAX_UPLOAD_RATE_KBYTES_PER_SEC * 1024);
	bdFloat32 actualRate = bytesUploaded / m_dataRateStopwatch.getElapsedTimeInSeconds();

	bdFloat32 desiredElapsedTime = static_cast<bdFloat32>(bytesUploaded) / desiredRate;
	bdFloat32 actualElapsedTime = m_dataRateStopwatch.getElapsedTimeInSeconds();

	BD_INFO(BD_LOG_LEVEL, "uploading - rate: %.1f KBps desired rate: %.1f KBps total: %i bytes", 
		actualRate / 1024.0, desiredRate / 1024.0, bytesUploaded);

	if (desiredElapsedTime > actualElapsedTime)
	{
		bdUInt32 waitTime = static_cast<bdUInt32>(1000.0 * (desiredElapsedTime-actualElapsedTime));
		bdPlatformTiming::sleep(waitTime);
	}

	return dataSize;
}

#else

bdUInt Simulation::handleDownload(void* data, bdUInt32 dataSize)
{
	if (m_bytesDownloaded + dataSize > m_fileSize)
	{
		BD_ERR(BD_LOG_LEVEL, "Not enough buffer space to save file. Check FILE_SIZE_HTTP");
		// signal that we're done by returning 0
		return 0;
	}

	bdMemcpy(m_fileData + m_bytesDownloaded, data, dataSize);
	m_bytesDownloaded += dataSize;
	return dataSize;
}

bdUInt Simulation::handleUpload(void* uploadBuffer, bdUInt32 bufferSize, bdUInt32 bytesUploaded)
{
	bdUInt32 bytesRemaining = m_uploadSize - bytesUploaded;

	bdUInt32 dataSize = BD_MIN(bytesRemaining, bufferSize);
	if (dataSize > 0)
	{
		bdMemcpy(uploadBuffer, m_fileData + m_bytesUploaded, dataSize);
		return dataSize;
	}
	else
	{
		// signal that we're done by returning 0
		return 0;
	}
}

#endif // RATE_LIMIT_HTTP

