#include <Stdafx.h>
#include <Core/Core.h>
#include <network/core.h>
#include <Network/SystemHeaders.h>
#include <Network/SessionDeletion.h>
#include <Network/SessionParameter.h>
#include "DemonwareTests.h"
#include <Network/Session.h>
#include <Network/SessionEnumerator.h>
#include <Network/ScoreboardStructures.h>
#include <Network/ScoreboardWriter.h>
#include <Network/ScoreboardReaderByRank.h>
#include <Network/ScoreboardReaderByUsers.h>
#include <Network/PerUserServerStorageWriter.h>
#include <Network/PerUserServerStorageReader.h>
#include <Network/PlayerSkillReader.h>
#include <Network/PlayerSkillWriter.h>
#include <Network/PlayerSkill.h>
#include <Network/SecureKey.h>  
#include <bdNet/bdNet.h>
#include <bdCore/bdCore.h>
#include <bdExample/bdExampleCommonLobby/bdExampleCommonLobby.h>
#include <bdLobby/bdAuthService.h>
#include <bdLobby/bdLobbyService.h>
#include <bdSocket\bdSecurityKeyMap.h>
#include <algorithm>

#if defined (PLATFORM_PC)
#include <Iphlpapi.h>
#endif

#include <btl/Containers/vector.h>
#include <btl/Containers/List.h>
#include <btl/allocators/pool.h>

#define TITLE_ID  (0x1403)	
#define USERNAME "david_wuz_ere"
//#define USERNAME "hello_world"
#define PASSWORD "bizarre"

#define TEST_AUTH_ADDRESS	"amax-pc.auth.mmp3.demonware.net" 
#define TEST_AUTH_PORT		(3074)

#define TEST_LSG_ADDRESS	"amax-pc.lsg.mmp3.demonware.net"
#define TEST_LSG_PORT		(3074)

typedef struct 
{
	char name[32];
}QosTest;

QosTest gSessionQos;


typedef struct 
{
	char name[32];
	U32	var1;
	U32 var2;
}UserFileTest;
UserFileTest gUserFile;

// if defined then we will request the written scoreboard data by rank.
#define SCOREBOARD_TEST_READ_BY_RANK


C_DemonwareTests::C_DemonwareTests(void) :	m_Started(false), m_PumpLobbyConnection(false),
											m_pSession(NULL), m_pEnumerator(NULL), m_pScoreboardWriter(NULL), m_pScoreboardReaderByRank(NULL), m_pScoreboardReaderByUsers(NULL),
											m_pUserFileWriter(NULL), m_pUserFileReader(NULL), m_pAuthGetHostByName(NULL), m_pLsgGetHostByName(NULL), 
											m_pAuthService(NULL), m_pSkillReader(NULL), m_pSkillWriter(NULL)
{
}

C_DemonwareTests::~C_DemonwareTests(void)
{
}

void C_DemonwareTests::Start(void)
{
	if (m_Started)
		return;

	// setup qos for when session is created.
	strcpy(gSessionQos.name, "Wowzer");

	m_Started = true;
	m_State = eTestStateBegin;
	
	m_pSession =  new NetPlay::C_Session;
	assert(m_pSession);
	const bool ok = m_pSession->Initialise(16, 10);
	assert(ok);

	m_pEnumerator = new NetPlay::C_SessionEnumerator;
	assert(m_pEnumerator);
	const bool eOk = m_pEnumerator->Initialise(10,2,10);
	assert (eOk);

	m_pScoreboardWriter = new Scoreboard::C_Writer;
	assert(m_pScoreboardWriter);
	const bool swOk = m_pScoreboardWriter->Initialise(10,32);
	assert(swOk);

	m_pScoreboardReaderByRank = new Scoreboard::C_ReaderByRank;
	assert(m_pScoreboardReaderByRank);
	const bool sbrbrOk = m_pScoreboardReaderByRank->Initialise();
	assert(sbrbrOk);

	m_pScoreboardReaderByUsers = new Scoreboard::C_ReaderByUsers;
	assert(m_pScoreboardReaderByUsers);
	const bool sbrbuOK = m_pScoreboardReaderByUsers->Initialise();
	assert(sbrbuOK);

	m_pUserFileWriter = new NetPlay::C_PerUserServerStorageWriter;
	assert(m_pUserFileWriter);

	m_pUserFileReader = new NetPlay::C_PerUserServerStorageReader;
	assert(m_pUserFileReader);

	m_pSkillReader = new NetPlay::C_PlayerSkillReader;
	assert(m_pSkillReader);
	const bool skok = m_pSkillReader->Initialise(10);
	assert(skok);

	m_pSkillWriter = new NetPlay::C_PlayerSkillWriter;
	assert(m_pSkillWriter);
	const bool skwrok = m_pSkillWriter->Initialise(10);
	assert(skwrok);

}

void C_DemonwareTests::Poll(const F32 frameTime)
{
	if (!m_Started)
		return;

	switch (m_State)
	{
	case eTestStateBegin:
		NetStartBegin();
		break;
	case eTestStateNetStart:
		NetStartContinue();
		break;
	
	case eTestStateDNSLookupStart:
		DNSLookupBegin();
		break;
	case eTestStateDNSlookupContinue:
		DNSLookupContinue();
		break;
		
	case eTestStateAccountCreationStart:
		AccountCreationBegin();
		break;
	case eTestStateAccountCreationContinue:
		AccountCreationContinue();
		break;

	case eTestStateAuthStart:
		AccountAuthenticationBegin();
		break;
	case eTestStateAuthContinue:
		AccountAuthenticationContinue();
		break;

	case eTestStatelobbyConnectStart:
		LobbyConnectBegin();
		break;
	case eTestStatelobbyConnectContinue:
		LobbyConnectContinue();
		break;

	case eTestStateSessionCreateStart:
		SessionCreateBegin();
		break;
	case eTestStateSessionCreateContinue:
		SessionCreateContinue();
		break;

	case eTestStateSesionEnumerationDelay:
		{
			m_DelayTime -= frameTime;
			if (m_DelayTime <= 0.0f)
				m_State = eTestStateSessionEnumerateStart;
		}
		break;

	case eTestStateSessionEnumerateStart:
		SessionEnumerateBegin();
		break;
	case eTestStateSessionEnumerateContinue:
		SessionEnumerateContinue();
		break;

	case eTestStateScoreboardWriteStart:
		ScoreboardWriteBegin();
		break;
	case eTestStateScoreboardWriteContinue:
		ScoreboardWriteContinue();
		break;

	case eTestStateScoreboardReadStart:
		ScoreboardReadBegin();
		break;
	case eTestStateScoreboardReadContinue:
		ScoreboardReadContinue();
		break;

	case eTestStateUserFileWriteStart:
		UserFileWriteBegin();
		break;
	case eTestStateUserFileWriteContinue:
		UserFileWriteContinue();
		break;
	
	case eTestStateUserFileReadStart:
		UserFileReadBegin();
		break;

	case eTestStateUserFileReadContinue:
		UserFileReadContinue();
		break;

	case eTestStateSkillWriteStart:
		SkillWriteBegin();
		break;

	case eTestStateSkillWriteContinue:
		SkillWriteContinue();
		break;

	case eTestStateSkillReadStart:
		SkillReadBegin();
		break;

	case eTestStateSkillReadContinue:
		SkillReadContinue();
		break;

	case eTestStateDisconnect:
		Disconnect();
		break;

	case eTestStateIdle:
		break;

	default:
		assert(false);
		break;
	}


	// need to Poll socket router when connected to allow QOS probes to be sent / received.
	bdNetImpl *const net = bdNet::getInstance();
	if (net->getStatus() == bdNetImpl::BD_NET_DONE)
	{
		bdSocketRouter* router =  net->getSocketRouter();
		if (router)
			router->pump();

		//net->receiveAll();
		//net->dispatchAll();
		//net->sendAll();
	}

	// poll session
	m_pSession->Poll();

	// once connected we need to constantly pump this to process async tasks
	if (m_PumpLobbyConnection)
	{
		bdLobbyService *const lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if(lobbyService)
		{
			lobbyService->pump();
			const bdLobbyConnection::Status connectionStatus = lobbyService->getStatus();
			switch(connectionStatus)
			{			
			case bdConnection::BD_NOT_CONNECTED:
			case bdConnection::BD_DISCONNECTED:
				m_PumpLobbyConnection = false;
				Disconnect();
				break;
			case bdConnection::BD_DISCONNECTING:
				m_PumpLobbyConnection = true;
				break;
			}
		}
	}
}

void C_DemonwareTests::Stop(void)
{
	if (!m_Started)
		return;

	Disconnect();

	if (m_pSession)
	{
		m_pSession->Delete();
		delete m_pSession;
		m_pSession = NULL;
	}
}


void C_DemonwareTests::NetStartBegin(void)
{
#if defined (PLATFORM_PC)
	bdNetImpl *const net = bdNet::getInstance();
	net->stop();

	// we want online functionality
	bdNetStartParams params;
	params.m_onlineGame = true;

	// build the list of local IP addresses..
	// GetAdaptersInfo treats first argument as a chunk of memory.
	U8* heapOutBuf = NULL;
	U8 stackOutBuf[sizeof(IP_ADAPTER_INFO)];
	ULONG outBufLen = sizeof(IP_ADAPTER_INFO);
	PIP_ADAPTER_INFO adapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(&stackOutBuf);

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the outBufLen variable
	if (GetAdaptersInfo(adapterInfo, &outBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		// need a bigger buffer
		heapOutBuf = new U8[outBufLen];
		assert(heapOutBuf);
		adapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(heapOutBuf);
	}

	//Get info for the adapters
	if (GetAdaptersInfo(adapterInfo, &outBufLen) != ERROR_SUCCESS)
	{
		// no network adapters ?
		if (heapOutBuf)
			delete [] heapOutBuf;
		
		Disconnect();
	}
	else
	{
		S32 index = 0;
		while(adapterInfo)
		{
			PIP_ADDR_STRING ipStr = &adapterInfo->IpAddressList;
			while(ipStr)
			{
				if(strcmp(ipStr->IpAddress.String, "0.0.0.0") != 0)
					params.m_localAddresses.pushBack(bdInetAddr(ipStr->IpAddress.String));

				ipStr = ipStr->Next;
			}

			adapterInfo = adapterInfo->Next;
			index++;
		}
	}

	if (params.m_localAddresses.isEmpty())
	{
		// Machine is not connected to any kind of network. Add loopback as local.
		params.m_localAddresses.pushBack(bdInetAddr("127.0.0.1"));
	}

	if (heapOutBuf)
		delete [] heapOutBuf;

	if (net->start(params))
		m_State = eTestStateNetStart;
	else
	{
		assert(false);
		m_State = eTestStateIdle;
	}
#else
	m_State = eTestStateIdle;
#endif
}

void C_DemonwareTests::NetStartContinue(void)
{
#if defined (PLATFORM_PC)
	bdNetImpl *const net = bdNet::getInstance();

	net->pump();

	if (net->getStatus() == bdNetImpl::BD_NET_PENDING)
		return;

	if (net->getStatus() == bdNetImpl::BD_NET_DONE)
	{
		bdCommonAddrRef myAddr = net->getLocalCommonAddr();
		m_State = eTestStateDNSLookupStart;
	}
	else
		Disconnect();
#else 
	Disconnect();
#endif
}

void C_DemonwareTests::DNSLookupBegin(void)
{
#if defined (PLATFORM_PC)
	const TaskStatus authDNSStatus = StartGetHostByName(TEST_AUTH_ADDRESS, &m_pAuthGetHostByName, m_authAddr);
	const TaskStatus lsgDNSStatus = StartGetHostByName(TEST_LSG_ADDRESS, &m_pLsgGetHostByName, m_lsgAddr);
	if((authDNSStatus == FAILED) || (lsgDNSStatus == FAILED))
	{
		Disconnect();
		return;
	}

	if((authDNSStatus == PENDING) || (lsgDNSStatus == PENDING))
	{
		m_State = eTestStateDNSlookupContinue;
		return;
	}

	if((authDNSStatus == DONE) && (lsgDNSStatus == DONE))
	{
		m_State = eTestStateAccountCreationStart;
		return;
	}
	
	assert(false);
#else
	m_State = eTestStateIdle;
#endif
}

void C_DemonwareTests::DNSLookupContinue(void)
{
#if defined (PLATFORM_PC)
	TaskStatus authDNSStatus = DONE;
	TaskStatus lsgDNSStatus = DONE;

	if(m_pAuthGetHostByName)
	{
		authDNSStatus = PumpGetHostByName(&m_pAuthGetHostByName, m_authAddr);
	}

	if(m_pLsgGetHostByName)
	{
		lsgDNSStatus = PumpGetHostByName(&m_pLsgGetHostByName, m_lsgAddr);
	}

	if((authDNSStatus == FAILED) || (lsgDNSStatus == FAILED))
	{
		Disconnect();
		return;
	}

	if((authDNSStatus == PENDING) || (lsgDNSStatus == PENDING))
		return;

	if((authDNSStatus == DONE) && (lsgDNSStatus == DONE))
	{
		m_State = eTestStateAccountCreationStart;
		return;
	}

	assert(false);
#else
	Disconnect();
#endif
}

void C_DemonwareTests::CreateAuthService(void)
{
#if defined (PLATFORM_PC)
	if (NULL == m_pAuthService)
	{
		const bdAddr authAddr(m_authAddr, TEST_AUTH_PORT);
		m_pAuthService = new bdAuthService(TITLE_ID, authAddr);
	}
#endif
}

void C_DemonwareTests::AccountCreationBegin(void)
{
#if defined (PLATFORM_PC)
	CreateAuthService();

	if (m_pAuthService)
	{
		if (m_pAuthService->createAccount(USERNAME, PASSWORD))
			m_State = eTestStateAccountCreationContinue;
		else
			Disconnect();
	}
	else
		Disconnect();
#else
	Disconnect();
#endif
}

void C_DemonwareTests::AccountCreationContinue(void)
{
#if defined (PLATFORM_PC)
	if (m_pAuthService)
	{
		const bdAuthService::bdStatus status = m_pAuthService->getStatus();
		if(status == bdAuthService::BD_READY)
		{
			const bdLobbyErrorCode errorCode = m_pAuthService->getErrorCode();
			if(BD_AUTH_NO_ERROR == errorCode || BD_AUTH_CREATE_USERNAME_EXISTS == errorCode)
				m_State = eTestStateAuthStart;
			else
				Disconnect();
		}
	}
	else
		Disconnect();
#else
	Disconnect();
#endif
}

void C_DemonwareTests::AccountAuthenticationBegin(void)
{
#if defined (PLATFORM_PC)
	CreateAuthService();

	if (m_pAuthService)
	{
		if (m_pAuthService->authorizeAccount(USERNAME, PASSWORD))
			m_State = eTestStateAuthContinue;
		else
			Disconnect();
	}
	else
		Disconnect();
#else
	Disconnect();
#endif

}

void C_DemonwareTests::AccountAuthenticationContinue(void)
{
#if defined (PLATFORM_PC)
	if (m_pAuthService)
	{
		const bdAuthService::bdStatus status = m_pAuthService->getStatus();
		if(status == bdAuthService::BD_READY)
		{
			const bdLobbyErrorCode errorCode = m_pAuthService->getErrorCode();
			if(BD_AUTH_NO_ERROR == errorCode)
			{
				// store the auth info for lobby connection.
				m_AuthInfo = *m_pAuthService->getAuthInfo();

				// can destroy authentication object now..
				delete m_pAuthService;
				m_pAuthService = NULL;

				m_State = eTestStatelobbyConnectStart;
			}
			else
				Disconnect();
		}
	}
	else
		Disconnect();
#else
	Disconnect();
#endif
}


void C_DemonwareTests::LobbyConnectBegin(void)
{
#if defined (PLATFORM_PC)
	bdLobbyService *const lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
	if(lobbyService)
	{
		const bdAddr lsgAddr(m_lsgAddr, TEST_LSG_PORT);

		if (lobbyService->connect(lsgAddr, &m_AuthInfo))
			m_State = eTestStatelobbyConnectContinue;
		else
			Disconnect();
	}
	else
		Disconnect();
#else
	Disconnect(); 
#endif

}

void C_DemonwareTests::LobbyConnectContinue(void)
{
#if defined (PLATFORM_PC)
	bdLobbyService *const lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
	if(lobbyService)
	{
		lobbyService->pump();
		const bdLobbyConnection::Status connectionStatus = lobbyService->getStatus();
		switch(connectionStatus)
		{			
		case bdConnection::BD_NOT_CONNECTED:
		case bdConnection::BD_DISCONNECTING:
		case bdConnection::BD_DISCONNECTED:
			{
				Disconnect();
			}
			break;
		case bdConnection::BD_CONNECTING:
			{
				return;
			}
			break;
		case bdConnection::BD_CONNECTED:
			{
				m_State = eTestStateSessionCreateStart;
				m_PumpLobbyConnection = true;
			}
			break;			
		}
	}
#else
	Disconnect();
#endif
}


void C_DemonwareTests::SessionCreateBegin(void)
{
#if defined (PLATFORM_PC)
	// kill any old session.
	m_pSession->Delete();

	NetPlay::C_Session::SlotConfig slotConf;
	slotConf.m_MaxPublic = 0;
	slotConf.m_MaxPrivate = 2;
	slotConf.m_PrivateInUse = 1; // local player

	Btl::Vector<NetPlay::SessionParameter, Btl::VectorParameters_Array<3> > params;
	params[0].CreateContext(0, 9);			// game type when using demonware must be first parameter and configured as a context.
	params[1].CreatePropertyS32(0, 100);	// attribute 1 
	params[2].CreatePropertyS32(1, 200);	// attribute 2

	if (m_pSession->Create(0, NetPlay::SESSION_CREATE_HOST, &params[0], 3, slotConf))
		m_State = eTestStateSessionCreateContinue;
	else
		Disconnect();
#else
	Disconnect();
#endif
}

void C_DemonwareTests::SessionCreateContinue(void)
{
#if defined (PLATFORM_PC)
	switch (m_pSession->SessionState())
	{
	case NetPlay::C_Session::eStateNotActive:
	case NetPlay::C_Session::eStateError:
		Disconnect();
		break;

	case NetPlay::C_Session::eStateCreating:
		break;
	
	default:
		{
			NetPlay::SessionSecurity sec;
			m_pSession->GetSecurityInfo(sec);
			m_pSession->EnableQos(16384, reinterpret_cast<U8*> (&gSessionQos), sizeof(gSessionQos));
			
			
			// attempt to add a player to the session.
			NetPlay::C_Session::SessionUserList list;

			NetPlay::C_Session::SessionUser user;
			user.m_PublicSlot = true;
			user.m_User.m_IsGuest = false;
			user.m_User.m_Uid = bdAuthUtility::getUserID("hello_world");

			list.PushBack(user);

			if(m_pSession->AddPlayers(list, NULL))
			{
				/*
				if(m_pSession->RemovePlayers(list, NULL))
				{
					m_DelayTime	 = 3.0f; // 3 second delay before enumeration stats.
					m_State = eTestStateSesionEnumerationDelay;
				}
				else
					Disconnect();
				*/
			}
			else
				Disconnect();

			m_DelayTime	 = 1.0f; // 1 second delay before enumeration stats.
			m_State = eTestStateSesionEnumerationDelay;
		}
	break;
	}
#else
	Disconnect();
#endif
}

void C_DemonwareTests::SessionEnumerateBegin(void)
{
#if defined (PLATFORM_PC)

	SysProfile::FullProfile fp;
	fp.m_State = SysProfile::eProfileStateOnline;

	// query input..
	// to make the server ignore a parameter in the query, set it's value to
	// BD_INT32_NAN for S32 types. look in bdCore\bdContainers\bdBitBufferTypes.h for all other type defines.
	Btl::Vector<NetPlay::SessionParameter, Btl::VectorParameters_Array<3> > params;
	params[0].CreatePropertyS32(0, 9);				// game mode (system parameter)
	params[1].CreatePropertyS32(1, 100);			// server attribute var1
	params[2].CreatePropertyS32(2, 	BD_INT32_NAN);	// server attribute var2 will be ignored by the server if set to this.

	// query return types (ignoring secure key blob).
	NetPlay::SesionQueryReturnTypes returnTypes;
	returnTypes.PushBack(NetPlay::SessionParameter::Int32);	// server attribute var1
	returnTypes.PushBack(NetPlay::SessionParameter::Int32);	// server attribute var2

	if (m_pEnumerator->StartSearch(fp, 1, 10, &params[0], 3, returnTypes, true))
		m_State = eTestStateSessionEnumerateContinue;
	else
		Disconnect();
#else
	Disconnect();
#endif
}

void C_DemonwareTests::SessionEnumerateContinue(void)
{
#if defined (PLATFORM_PC)
	m_pEnumerator->Poll();
	if (m_pEnumerator->IsSearching())
		return;

	int num = m_pEnumerator->NumSessionsEnumerated();

	if (num > 0)
	{
		NetPlay::SessionSecurity sec;
		bdCommonAddrRef addr;
		if (m_pEnumerator->GetHostInfoFromEnumerated(0, sec, addr))
		{
			int i=0;
		}

		U32 open = 0;
		U32 max = 0;
		if (m_pEnumerator->GetSlotInfoFromEnumerated(0, open, max))
		{
			int i=0;
		}

		NetPlay::C_SessionEnumerator::SessionQoSData qos;
		if (m_pEnumerator->GetQoSDataFromEnumerated(0, qos))
		{
			int i = 0;
		}
	}

	m_pEnumerator->KillSearch();

	// delete session now as its no longer required.
	m_pSession->Delete();

	m_State = eTestStateScoreboardWriteStart;
#else
	Disconnect();
#endif
}


void C_DemonwareTests::ScoreboardWriteBegin(void)
{
	m_pScoreboardWriter->StopAndCleanup();
#if defined (PLATFORM_PC)
	Btl::Vector<Scoreboard::ColumnData, Btl::VectorParameters_Array<3> > cols;
	cols[0].SetS32(0, 300);
	cols[1].SetS64(1, 301);

	// add first scorboard
	if (m_pScoreboardWriter->Queue(1,123, Scoreboard::eWriteTypeReplace, &cols[0], 2))
	{
		//add second scoreboard
		//cols[0].SetS32(0, 300);
		//cols[1].SetS64(1, 600);
		//cols[2].SetS32(2, 150);
		//if (m_pScoreboardWriter->Queue(2, 50, Scoreboard::eWriteTypeReplace, &cols[0], 3))
		{
			// blag a user since sysprofiles are not setup in this test app.
			SysProfile::UserIdentifier user;
			user.m_IsGuest = false;
			user.m_Uid = bdAuthUtility::getUserID(USERNAME);

			if (m_pScoreboardWriter->Start(user, *m_pSession))
				m_State = eTestStateScoreboardWriteContinue;
			else
				Disconnect();
		}
		//else
		//	Disconnect();
	}
	else
		Disconnect();
#else
	Disconnect();
#endif
}

void C_DemonwareTests::ScoreboardWriteContinue(void)
{
#if defined (PLATFORM_PC)
	bool error = false;
	if (!m_pScoreboardWriter->Continue(error))
		return;

	m_pScoreboardWriter->StopAndCleanup();

	m_State = eTestStateScoreboardReadStart;
#else
	Disconnect();
#endif
}


void C_DemonwareTests::ScoreboardReadBegin(void)
{
#if defined (PLATFORM_PC)
#if defined (SCOREBOARD_TEST_READ_BY_RANK)

	Scoreboard::ColumnIdentifierList colList;
	colList.PushBack(0);
	colList.PushBack(1);
	//colList.PushBack(2);

	Scoreboard::ColumnTypeList colTypes;
	colTypes.PushBack(Scoreboard::Int32);
	colTypes.PushBack(Scoreboard::Int64);
	//colTypes.PushBack(Scoreboard::Int32);

	if (m_pScoreboardReaderByRank->AddScoreboard(1, colList, colTypes, true))
	{
		if (m_pScoreboardReaderByRank->StartReadByRank(0, 1, 10))
			m_State = eTestStateScoreboardReadContinue;
		else
			Disconnect();
	}
	else
		Disconnect();

#else
	m_pScoreboardReaderByUsers->StopAndCleanup();

	// blag users..
	Btl::Vector<SysProfile::UserIdentifier, Btl::VectorParameters_Array<2> > users;
	users[0].m_IsGuest = false;
	users[0].m_Uid = bdAuthUtility::getUserID("david_wuz_ere");
	users[1].m_IsGuest = false;
	users[1].m_Uid = bdAuthUtility::getUserID("hello_world");

	if (m_pScoreboardReaderByUsers->SetUsers(&users[0], 2))
	{
		Scoreboard::ColumnIdentifierList colList;
		colList.PushBack(0);
		colList.PushBack(1);

		Scoreboard::ColumnTypeList colTypes;
		colTypes.PushBack(Scoreboard::Int32);
		colTypes.PushBack(Scoreboard::Int64);

		if (m_pScoreboardReaderByUsers->AddScoreboard(1, colList, colTypes, true))
		{
			if (m_pScoreboardReaderByUsers->Start(0))
				m_State = eTestStateScoreboardReadContinue;
			else
				Disconnect();
		}
		else
			Disconnect();
	}
	else
		Disconnect();
#endif
#else
	Disconnect();
#endif
}

void C_DemonwareTests::ScoreboardReadContinue(void)
{
#if defined (PLATFORM_PC)
#if defined(SCOREBOARD_TEST_READ_BY_RANK)

	bool error = false;
	if (!m_pScoreboardReaderByRank->Continue(error))
		return;

	if (!error)
	{
		Scoreboard::RowData	rows[10];
		U32 size = 0;
		const U32 numRows = m_pScoreboardReaderByRank->ExtractRows(1, rows, 10, size);
		if (numRows)
		{
		}
	}

	m_pScoreboardReaderByRank->StopAndCleanup();

#else
	bool error = false;
	if (!m_pScoreboardReaderByUsers->Continue(error))
		return;

	if (!error)
	{
		Scoreboard::RowData	rows[10];
		const U32 numRows = m_pScoreboardReaderByUsers->ExtractRows(1, rows, 10);
		if (numRows)
		{
		}
	}

	m_pScoreboardReaderByUsers->StopAndCleanup();
#endif

	m_State = eTestStateUserFileWriteStart;
#else
	Disconnect();
#endif
}

void C_DemonwareTests::UserFileWriteBegin(void)
{
	memset(&gUserFile, 0, sizeof(gUserFile));
	gUserFile.var1 = 5;
	gUserFile.var2 = 3;
	strcpy(gUserFile.name, "ghost");

#if defined (PLATFORM_PC)
	// blag a profile for now..
	SysProfile::FullProfile fp;
	fp.m_State = SysProfile::eProfileStateOnline;
	fp.m_Uid = bdAuthUtility::getUserID(USERNAME);

	if (m_pUserFileWriter->Start(fp, "ghost.1.bin", reinterpret_cast<U8*>(&gUserFile), sizeof(gUserFile)))
		m_State = eTestStateUserFileWriteContinue;
	else
		Disconnect();
#else
	Disconnect();
#endif

}

void C_DemonwareTests::UserFileWriteContinue(void)
{
#if defined (PLATFORM_PC)
	const NetPlay::C_PerUserServerStorageWriter::ePerUserStorageWriterState state = m_pUserFileWriter->Continue();
	switch (state)
	{
	case NetPlay::C_PerUserServerStorageWriter::eWriteNotActive:
	case NetPlay::C_PerUserServerStorageWriter::eWriteFailed:
		Disconnect();
		break;

	case NetPlay::C_PerUserServerStorageWriter::eWriteInProgress:
		return;
		break;
	
	case NetPlay::C_PerUserServerStorageWriter::eWriteSuccessful:
		m_pUserFileWriter->StopAndCleanup();
		m_State = eTestStateUserFileReadStart;
		break;
	}
#else
	Disconnect();
#endif
}
  

void C_DemonwareTests::UserFileReadBegin()
{
#if defined (PLATFORM_PC)
	memset(&gUserFile, 0, sizeof(gUserFile));

	SysProfile::FullProfile fp;
	fp.m_Uid = bdAuthUtility::getUserID(USERNAME);
	fp.m_UserIndex = 0;

	// blag a profile for now..
	SysProfile::UserIdentifier id;
	id.m_Uid = bdAuthUtility::getUserID(USERNAME);

	if (m_pUserFileReader->Start(fp, id, "ghost.1.bin",reinterpret_cast<U8*>(&gUserFile), sizeof(gUserFile) ))
		m_State = eTestStateUserFileReadContinue;
	else
		Disconnect();
#else
	Disconnect();
#endif
}
          
void C_DemonwareTests::UserFileReadContinue()  
{
#if defined (PLATFORM_PC)
	U32 downloadedSize = 0;
	const NetPlay::C_PerUserServerStorageReader::ePerUserStorageReaderState state = m_pUserFileReader->Continue(downloadedSize);
	switch (state)
	{
	case NetPlay::C_PerUserServerStorageReader::eReadNotActive:
	case NetPlay::C_PerUserServerStorageReader::eReadFailed:  
	case NetPlay::C_PerUserServerStorageReader::eReadDownloadBufferToSmall:
		Disconnect();

	case NetPlay::C_PerUserServerStorageReader::eReadInProgress:
		return;
		break;

	case NetPlay::C_PerUserServerStorageReader::eReadFileDoesNotExist:
	case NetPlay::C_PerUserServerStorageReader::eReadSuccessful:
		m_pUserFileReader->StopAndCleanup();
		m_State = eTestStateSkillWriteStart;
		break;
	}
#else
	Disconnect();
#endif
}

void C_DemonwareTests::SkillWriteBegin(void)
{
#if defined (PLATFORM_PC)

	// blag users...
	NetPlay::PlayerSkillSubmission subs[2];
	subs[0].m_User.m_Uid = bdAuthUtility::getUserID("david_wuz_ere");
	subs[0].m_Rating = 1;
	subs[0].m_Team = 1;

	subs[1].m_User.m_Uid = bdAuthUtility::getUserID("hello_world");
	subs[1].m_Rating = 2;
	subs[1].m_Team = 2;

	if (m_pSkillWriter->Start(*m_pSession, &subs[0], 2, 1))
		m_State = eTestStateSkillWriteContinue;
	else
		Disconnect();
#else
	Disconnect();
#endif
}

void C_DemonwareTests::SkillWriteContinue(void)
{
#if defined (PLATFORM_PC)
	bool err = false;
	if (!m_pSkillWriter->Continue(err))
		return;
	if (err)
		Disconnect();
	else
	{
		m_State = eTestStateSkillReadStart;
	}
#else
	Disconnect();
#endif
}

void C_DemonwareTests::SkillReadBegin(void)
{
#if defined (PLATFORM_PC)
	// blag a profile for now..
	SysProfile::UserIdentifier id[2];
	id[0].m_Uid = bdAuthUtility::getUserID("david_wuz_ere");
	id[1].m_Uid = bdAuthUtility::getUserID("hello_world");

	if (m_pSkillReader->Start(&id[0], 2, 1))
		m_State = eTestStateSkillReadContinue;
	else
		Disconnect();

#else
	Disconnect();
#endif
}

void C_DemonwareTests::SkillReadContinue(void)
{
#if defined (PLATFORM_PC)
	bool err = false;
	if (!m_pSkillReader->Continue(err))
		return;
	if (err)
		Disconnect();
	else
	{
		NetPlay::PlayerSkill skill;
		NetPlay::PlayerSkill skill1;

		SysProfile::UserIdentifier id;
		id.m_Uid = bdAuthUtility::getUserID("david_wuz_ere");

		SysProfile::UserIdentifier id1;
		id1.m_Uid = bdAuthUtility::getUserID("hello_world");

		m_pSkillReader->GetSkillFromResults(id, skill);
		m_pSkillReader->GetSkillFromResults(id1, skill1);

		m_pSkillReader->StopAndCleanup();
		m_State = eTestStateIdle;
	}
#else
	Disconnect();
#endif

}


// release everything and stop the demonware service.
void C_DemonwareTests::Disconnect(void)
{
	if (m_pSkillWriter)
	{
		m_pSkillWriter->StopAndCleanup();
		delete m_pSkillWriter;
		m_pSkillWriter = NULL;
	}

	if (m_pSkillReader)
	{
		m_pSkillReader->StopAndCleanup();
		delete m_pSkillReader;
		m_pSkillReader = NULL;
	}

	if (m_pUserFileReader)
	{
		m_pUserFileReader->StopAndCleanup();
		delete m_pUserFileReader;
		m_pUserFileReader = NULL;
	}

	if (m_pUserFileWriter)
	{
		m_pUserFileWriter->StopAndCleanup();
		delete m_pUserFileWriter;
		m_pUserFileWriter = NULL;
	}

	if (m_pScoreboardReaderByUsers)
	{
		m_pScoreboardReaderByUsers->StopAndCleanup();
		delete m_pScoreboardReaderByUsers;
		m_pScoreboardReaderByUsers = NULL;
	}

	if (m_pScoreboardReaderByRank)
	{
		m_pScoreboardReaderByRank->StopAndCleanup();
		delete m_pScoreboardReaderByRank;
		m_pScoreboardReaderByRank = NULL;
	}

	if (m_pScoreboardWriter)
	{
		m_pScoreboardWriter->StopAndCleanup();
		delete m_pScoreboardWriter;
		m_pScoreboardWriter = NULL;
	}

	if (m_pEnumerator)
	{
		m_pEnumerator->KillSearch();
		delete m_pEnumerator;
		m_pEnumerator = NULL;
	}

	if (m_pSession)
		m_pSession->Delete();

	NetPlay::C_SessionDeletion::Instance().Shutdown();
	
	bdLobbyService *const lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
	if(lobbyService)
		lobbyService->disconnect();

	m_PumpLobbyConnection = false;

	if (m_pAuthService)
		delete m_pAuthService;
	m_pAuthService = NULL;

	if (m_pAuthGetHostByName)
	{
#if defined (PLATFORM_PC)
		m_pAuthGetHostByName->quit();
#endif
		delete m_pAuthGetHostByName;
		m_pAuthGetHostByName = NULL;
	}

	if (m_pLsgGetHostByName)
	{
#if defined (PLATFORM_PC)
		m_pLsgGetHostByName->quit();
#endif
		delete m_pLsgGetHostByName;
		m_pLsgGetHostByName = NULL;
	}

	bdNetImpl *const net = bdNet::getInstance();
	net->stop();
	m_State = eTestStateIdle;
}
