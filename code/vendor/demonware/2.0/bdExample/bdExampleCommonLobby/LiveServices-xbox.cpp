// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdCore/bdCore.h>
#include <bdExample/bdExampleCommonLobby/LiveServices.h>
#include <bdNet/bdNet.h>

#define BD_LOG_LEVEL "bdExampleCommon/LiveServices"

#pragma pack(push, 1)
struct QueryResult
{
	ULONGLONG qwEntityID;          // title server ID
	WORD      wTitleServerAddress; // (unused)
	TSADDR    TitleServerAddress;  // title server address
	WORD      wSessionID;          // (unused)
	XNKID     SessionID;           // session ID
	WORD      wKeyExchangeKey;     // (unused)
	XNKEY     KeyExchangeKey;      // key exchange key
};
#pragma pack(pop)

LiveServices::LiveServices()
: m_signedOn(false),
  m_userId(0)
{
	m_userName[0] = '\0';
}

bdBool LiveServices::signOn(bdUInt32 titleServerID)
{
	XONLINE_USER aStoredUsers[ XONLINE_MAX_STORED_ONLINE_USERS ];
	DWORD        dwNumStoredUsers;

	HRESULT hrGetUsers = XOnlineGetUsers( aStoredUsers, &dwNumStoredUsers );
	BD_ASSERT(hrGetUsers == S_OK, "Failed to get user accounts.");
	(VOID)hrGetUsers; // avoid compiler warning

	if( dwNumStoredUsers == 0 )
	{
		BD_ERR(BD_LOG_LEVEL, "No user accounts found.");
		return false;
	}

	if( aStoredUsers[0].dwUserOptions & XONLINE_USER_OPTION_REQUIRE_PASSCODE )
	{
		BD_INFO(BD_LOG_LEVEL, "%S has a passcode.", aStoredUsers[0].szGamertag);
	}

	XONLINE_USER aLogonUsers[ XONLINE_MAX_LOGON_USERS ] = { 0 }; // Initially zeroed

	// Sign in the first account on the first controller.
	aLogonUsers[0] = aStoredUsers[0];

	const DWORD X_SERVICES_ARRAY[] =
	{
		XONLINE_QUERY_SERVICE,
		titleServerID
	};
	const DWORD NUM_X_SERVICES = sizeof( X_SERVICES_ARRAY ) /
		sizeof( X_SERVICES_ARRAY[0] );

	HRESULT hrLogon = XOnlineLogon( aLogonUsers, X_SERVICES_ARRAY, NUM_X_SERVICES,
		NULL, &m_logonTask );

	switch( hrLogon )
	{
	case S_OK:
		break;
	case XONLINE_E_LOGON_NO_NETWORK_CONNECTION:
		BD_ERR(BD_LOG_LEVEL, "No network connection detected.");
		return false;
		break;
	default:
		BD_ASSERT(false, "XOnlineLogon returned something strange.");
		return false;
	}

	BD_INFO(BD_LOG_LEVEL, "Signing in...");

	HRESULT hrTask;
	do
	{
		hrTask = XOnlineTaskContinue( m_logonTask );  // Do a small amount of work
	}
	while ( hrTask == XONLINETASK_S_RUNNING );  // As long as there is work to do

	// 1. Check for system authentication errors.
	switch( hrTask )
	{
	case XONLINE_S_LOGON_CONNECTION_ESTABLISHED:
		BD_INFO(BD_LOG_LEVEL, "Connection established");
		break;
	case XONLINE_E_LOGON_CONNECTION_LOST:
		BD_ERR(BD_LOG_LEVEL, "Network connection lost");
		return false;
	case XONLINE_E_LOGON_CANNOT_ACCESS_SERVICE:
		BD_ERR(BD_LOG_LEVEL, "Your Xbox console cannot connect to Xbox Live.");
		return false;
	case XONLINE_E_LOGON_UPDATE_REQUIRED:
		BD_WARN(BD_LOG_LEVEL, "XONLINE_E_LOGON_UPDATE_REQUIRED happened.");
		XOnlineTitleUpdate( 0 );
		BD_ASSERT(false, "Shouldn't happen.");
		return false;
	case XONLINE_E_LOGON_INVALID_USER:
		BD_ERR(BD_LOG_LEVEL, "This account is not current.");
		return false;
	case XONLINE_E_LOGON_SERVERS_TOO_BUSY:
		BD_WARN(BD_LOG_LEVEL, "The XBox Live service is very busy.");
		return false;
	default:
		BD_ERR(BD_LOG_LEVEL, "Login failed with error 0x%x", hrTask);
		return false;
	}

	PXONLINE_USER aUsers = XOnlineGetLogonUsers();

	BD_ASSERT(aUsers != BD_NULL, "Failed to retrieve users.");

	for( DWORD i = 0; i < XONLINE_MAX_LOGON_USERS; ++i )
	{
		if( aUsers[i].xuid.qwUserID != 0 ) // A valid user
		{
			switch( aUsers[i].hr )
			{
			case S_OK:
				BD_INFO(BD_LOG_LEVEL, "%S signed in", aUsers[i].szGamertag );
				break;
			case XONLINE_S_LOGON_USER_HAS_MESSAGE:
				BD_INFO(BD_LOG_LEVEL, "%S signed in, and has messages", aUsers[i].szGamertag );
				break;
			case XONLINE_E_LOGON_USER_ACCOUNT_REQUIRES_MANAGEMENT:
				BD_ERR(BD_LOG_LEVEL, "This %S account requires management", aUsers[i].szGamertag );
				return false;
			default:
				BD_ASSERT(false, "Should never happen.");
				return false;
			}

			m_userId = aUsers[i].xuid.qwUserID;
			bdMemset(m_userName, 0, BD_MAX_USERNAME_LENGTH);
			bdMemcpy(m_userName, aUsers[i].szGamertag,
				BD_MIN(BD_MAX_USERNAME_LENGTH, bdStrlen(aUsers[i].szGamertag)));
		}

	}

	for( DWORD i = 0; i < NUM_X_SERVICES; ++i )
	{
		HRESULT hrGetServiceInfo = XOnlineGetServiceInfo( X_SERVICES_ARRAY[i], NULL );

		switch( hrGetServiceInfo )
		{
		case S_OK:
			BD_INFO(BD_LOG_LEVEL, "Service %lu Available", X_SERVICES_ARRAY[i]);
			break;
		case XONLINE_E_LOGON_SERVICE_NOT_AUTHORIZED:
			BD_ERR(BD_LOG_LEVEL, "Access to service 0x%x is denied", X_SERVICES_ARRAY[i] );
			return false;
		case XONLINE_E_LOGON_SERVICE_TEMPORARILY_UNAVAILABLE:
			BD_ERR(BD_LOG_LEVEL, "Service %lu is unavailable", X_SERVICES_ARRAY[i]);
			return false;
		default:
			BD_ERR(BD_LOG_LEVEL, "Error 0x%x signing onto service %lu", hrGetServiceInfo,
				X_SERVICES_ARRAY[i]);
			return false;
		}
	}

	for( DWORD i = 0; i < XONLINE_MAX_LOGON_USERS; ++i )
	{
		if( aUsers[i].xuid.qwUserID != 0 &&
			!XOnlineIsUserGuest( aUsers[i].xuid.dwUserFlags ) )
		{

			HRESULT hrSetState = XOnlineNotificationSetState(
				i,                               // Controller Index
				XONLINE_FRIENDSTATE_FLAG_ONLINE, // state flags
				// (just stating user
				// is online here)
				XNKID(),                         // this user's current session
				0,                               // buffer size of title-specific
				// info buffer (not being
				// used here)
				NULL                             // buffer for title-specific info
				// (not being used here)
				);

			// Ensure the set state operation succeeded
			BD_ASSERT(hrSetState == S_OK, "Set state operation failed.");
		}
	}

	m_signedOn = true;

	return true;
}

bdBool LiveServices::pump()
{
	HRESULT hrTask = XOnlineTaskContinue( m_logonTask );

	switch( hrTask )
	{
	case XONLINE_S_LOGON_CONNECTION_ESTABLISHED:
		break;
	case XONLINE_E_LOGON_CONNECTION_LOST:
		BD_WARN(BD_LOG_LEVEL, "Connection was lost.");
		return false;
	case XONLINE_E_LOGON_KICKED_BY_DUPLICATE_LOGON:
		BD_WARN(BD_LOG_LEVEL, "Another XBox signed on with the same account.");
		return false;
	default:
		BD_ASSERT(false, "XOnlineTaskContinue returned something strange.");
		return false;
	}

	return true;
}

bdBool LiveServices::isSignedOn()
{
	return m_signedOn;
}

bdBool LiveServices::resolveTitleAddr(bdUInt32 titleServerID,
									  bdUInt16 port,
									  bdCommonAddrRef &addr,
									  bdSecurityID &id,
									  bdSecurityKey &key)
{
	// we'll be making a query for datasets that match four different attribute
	// sets
	const DWORD NUM_ATTRIBUTE_SPECS = 4;
	enum // the number of types should match the number of attribute specs, above
	{
		ATTSPEC_ENTITY_ID,
		ATTSPEC_TS_ADDR,
		ATTSPEC_SESSION_ID,
		ATTSPEC_KEY_EXCHANGE_KEY
	};

	XONLINE_ATTRIBUTE_SPEC aQueryAttrs[NUM_ATTRIBUTE_SPECS];
	XONLINETASK_HANDLE     hTaskQuery;

	TSADDR tsa;

	aQueryAttrs[ATTSPEC_ENTITY_ID].dwType          = X_ATTRIBUTE_DATATYPE_ENTITY_ID;
	aQueryAttrs[ATTSPEC_ENTITY_ID].dwLength        = 0;
	aQueryAttrs[ATTSPEC_TS_ADDR].dwType            = XONLINE_LSP_ATTR_TSADDR;
	aQueryAttrs[ATTSPEC_TS_ADDR].dwLength          = sizeof(tsa);
	aQueryAttrs[ATTSPEC_SESSION_ID].dwType         = XONLINE_LSP_ATTR_XNKID;
	aQueryAttrs[ATTSPEC_SESSION_ID].dwLength       = sizeof(id);
	aQueryAttrs[ATTSPEC_KEY_EXCHANGE_KEY].dwType   = XONLINE_LSP_ATTR_KEK;
	aQueryAttrs[ATTSPEC_KEY_EXCHANGE_KEY].dwLength = sizeof(key);

	const DWORD QUERY_SEARCH_PROCEDURE_INDEX = 1;
	const DWORD RESULT_PAGE_INDEX = 0;
	const DWORD NUM_RESULTS_PER_PAGE = 10; // 255 is the maximum size

	// attempt to query matching attributes
	HRESULT hrQuerySearch = XOnlineQuerySearch(
		XONLINE_LSP_DEFAULT_DATASET_ID, // ID for default LSP dataset
		QUERY_SEARCH_PROCEDURE_INDEX,   // search procedure index
		RESULT_PAGE_INDEX,              // current page number
		NUM_RESULTS_PER_PAGE,           // number of results per page
		NUM_ATTRIBUTE_SPECS,            // number of competition attributes
		//     in the attribute specs array
		aQueryAttrs,                    // competition attribute specs array
		0,                              // number of competition attributes
		//     in the attributes array
		//     (not used here)
		NULL,                           // competition attributes array
		//     (not used here)
		NULL,                           // handle to event related to this task
		//     (not used)
		&hTaskQuery                     // address of query task
		);


	if ( hrQuerySearch != S_OK)
	{
		BD_ERR(BD_LOG_LEVEL, "Failed in attempting to query dataset 0x%x" , XONLINE_LSP_DEFAULT_DATASET_ID );
		return false;
	}

	// Go into a loop, calling XOnlineTaskContinue on the query task
	// until the task completes.
	// In a real title, this would appear inside your game loop.
	HRESULT hrTask;
	do
	{
		hrTask = XOnlineTaskContinue( hTaskQuery );  // Do a small amount of work
	}
	while ( hrTask == XONLINETASK_S_RUNNING );  // As long as there is work to do

	if ( hrTask == S_OK)
	{
		BD_ERR(BD_LOG_LEVEL, "Failed in completing query task with error 0x%x" , hrTask );
		return false;
	}

	// Parse the results
	QueryResult aQueryResults[10];
	DWORD cTotalResults;
	DWORD cResults;
	DWORD cbResults = sizeof(aQueryResults);

	// get the results from the previous XOnlineQuerySearch call
	HRESULT hrGetResults = XOnlineQuerySearchGetResults( hTaskQuery, &cTotalResults,
		&cResults, &cbResults,
		(PBYTE)aQueryResults );

	if ( hrGetResults == S_OK)
	{
		BD_ERR(BD_LOG_LEVEL, "Failed in acquiring query results with error 0x%x" , hrGetResults );
		return false;
	}

	// closes task and its resources
	XOnlineTaskClose( hTaskQuery );
	hTaskQuery = NULL;

	// we can't
	if (cResults == 0)
	{
		BD_WARN(BD_LOG_LEVEL, "No results were returned from query search." );
		return false;
	}

	// output the query results
	for (DWORD i = 0; i < cResults; ++i)
	{
		DWORD ipa = aQueryResults[i].TitleServerAddress.inaOnline.s_addr;
		BD_INFO(BD_LOG_LEVEL, "%d: %I64x %I64x %d.%d.%d.%d", i, aQueryResults[i].qwEntityID,
			*(ULONGLONG *)&aQueryResults[i].SessionID, (ipa) & 0xFF,
			(ipa >> 8) & 0xFF, (ipa >> 16) & 0xFF, (ipa >> 24) & 0xFF );
	}

	// Choose a TSADDR. Note that we're taking a shortcut here by choosing
	// an address from the first page of results. In a real title you
	// would want to choose from the full set to ensure good load balancing.

	DWORD iRandom;
	XNetRandom((BYTE *)&iRandom, sizeof(iRandom));
	iRandom %= min(NUM_RESULTS_PER_PAGE, cResults);

	memcpy(&tsa, &aQueryResults[iRandom].TitleServerAddress, sizeof(tsa) );

	id = bdSecurityID(aQueryResults[iRandom].SessionID);
	key = bdSecurityKey(aQueryResults[iRandom].KeyExchangeKey);

	addr = new bdCommonAddr(bdNet::getInstance()->getLocalCommonAddr(), tsa, port, titleServerID);

	return true;
}

bdUInt64 LiveServices::getXUserId()
{
	return m_userId;
}

void LiveServices::getXUserName(bdNChar8 buffer[BD_MAX_USERNAME_LENGTH])
{
	bdMemcpy(buffer, m_userName, BD_MAX_USERNAME_LENGTH);
}

