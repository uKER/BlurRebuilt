#include <StdAfx.h>
#include <Network/Core.h>
#include <Network/Securekey.h>
#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyService.h>
#include <system/time/time.h>

namespace NetPlay
{
	// static instance..
	C_Core C_Core::s_NetworkCore;

	/// bitfields used when starting up the ps3 netwoking so we know what to shutdown later.
	typedef enum eNpBitfields
	{
		eNetInitializeBit = 0,		// sys_net_initialize_network()
		eNetCtlBit,					// cellNetCtlInit()
		eNetNpInitBit,				// sceNpInit();
		eNetNpLookupInitBit,		// eNetNpLookupInitBit();
		eNetNp2InitBit,				// sceNp2Init();
	};

	// constructor
	C_Core::C_Core() :	m_StartedUp(false), m_ValidLocalAddress(false),m_ScanAddressTimer(0), 
						m_NetCableConnected(false), m_ScanForLocalAddress(false), m_BitFields(0), m_pNetworkPlatformPool(NULL)
	{
	}

	// destructor
	C_Core::~C_Core()
	{
	}

	// Startup network layer
	void C_Core::Initialise(const bool bypassSecurity, const bool networkStackAlreadyStarted)
	{
		if (m_StartedUp)
			return;

		// initialise demonware. default memory manager can be re assigned later if required.
		bdCore::init(true);

		// allocate pool for network platform
		m_pNetworkPlatformPool = new U8[SCE_NP_MIN_POOL_SIZE];
		
		// create demonware lobby;
		m_pDemonwareLobby = new bdLobbyService;

		if (NULL == m_pNetworkPlatformPool || NULL == m_pDemonwareLobby)
		{
			Cleanup();
			return;
		}

		memset(m_pNetworkPlatformPool, 0, SCE_NP_MIN_POOL_SIZE);

		// if the debug tool has already initialised the network layer then don't do it again..
		if (!networkStackAlreadyStarted)
		{
			// init network stack.
			if (sys_net_initialize_network() == SUCCEEDED)
				m_BitFields |= (1<<eNetInitializeBit);
			else
			{
				LOGWARN(g_Log, "sys_net_initialize_network() failed - has PRX module been loaded ?");
				Cleanup();
				return;
			}

			// start network library
			if (cellNetCtlInit() == SUCCEEDED)
				m_BitFields |= (1<<eNetCtlBit);
			else
			{
				LOGWARN(g_Log, "cellNetCtlInit() failed - has PRX module been loaded ?");
				Cleanup();
				return;
			}
		}

		if (sceNpInit(SCE_NP_MIN_POOL_SIZE, m_pNetworkPlatformPool) == SUCCEEDED)
			m_BitFields |= (1<<eNetNpInitBit);
		else
		{
			LOGWARN(g_Log, "sceNpInit() failed - has PRX module been loaded ?");
			Cleanup();
			return;
		}

		if (sceNpLookupInit() == SUCCEEDED)
			m_BitFields |= (1<<eNetNpLookupInitBit);
		else
		{
			LOGWARN(g_Log, "sceNpLookupInit() failed - has PRX module been loaded ?");
			Cleanup();
			return;
		}

		// start up network platform library
		if (sceNp2Init(SCE_NP_MIN_POOL_SIZE, m_pNetworkPlatformPool) == SUCCEEDED)
			m_BitFields |= (1<<eNetNp2InitBit);
		else
		{
			LOGWARN(g_Log, "sceNp2Init() failed - has PRX module been loaded ?");
			Cleanup();
			return;
		}

		m_ValidLocalAddress = false;
		m_NetCableConnected = false;
		m_StartedUp = true;
	}

	// shutdown network layer
	void C_Core::Shutdown(void)
	{
		Cleanup();
	}


	// private function used when closing down the system..
	void C_Core::Cleanup(void)
	{
		if (NULL != m_pDemonwareLobby)
		{
			m_pDemonwareLobby->disconnect();
			delete m_pDemonwareLobby;
			m_pDemonwareLobby = NULL;
		}

		// if we initialised the net then release any net thread ciontent memory that may be active.
		if (m_BitFields&(1<<eNetInitializeBit))
			sys_net_free_thread_context(0, SYS_NET_THREAD_ALL);

		if (m_BitFields&(1<<eNetNp2InitBit))
			sceNp2Term();

		if (m_BitFields&(1<<eNetNpLookupInitBit))
			sceNpLookupTerm();

		if (m_BitFields&(1<<eNetNpInitBit))
			sceNpTerm();

		if (m_BitFields&(1<<eNetCtlBit))
			cellNetCtlTerm();

		if (m_BitFields&(1<<eNetInitializeBit))
			sys_net_finalize_network();
		

		// release network platform memory pool
		if (m_pNetworkPlatformPool)
			delete [] m_pNetworkPlatformPool;
		m_pNetworkPlatformPool = NULL;

		bdCore::quit();

		m_StartedUp = false;	
		m_ValidLocalAddress = false;
		m_ScanAddressTimer = 0; 	
		m_NetCableConnected = false; 
		m_ScanForLocalAddress = false;
		m_BitFields	= 0;
	}

	// poll once per frame if possible
	void C_Core::Poll(const bool forceNetworkLinkCheck)
	{
		if (!m_StartedUp)
			return;

		bool timedRead = false;
		const U64 ticks = Time::GetPerformanceCounter(); // microseconds

		if (m_ScanAddressTimer == 0 || ticks >= m_ScanAddressTimer || forceNetworkLinkCheck)
		{
			m_ScanAddressTimer = ticks + (500*1000);	// 500ms second update.
			timedRead = true;
		}

		if (timedRead || forceNetworkLinkCheck)
		{
			S32 status = 0;
			if (cellNetCtlGetState(&status) != 0)
			{
				// error
				m_ValidLocalAddress = false;
				m_NetCableConnected = false;
			}
			else
			{
				switch (status)
				{
				case CELL_NET_CTL_STATE_IPObtained:
					m_NetCableConnected = true;
					m_ValidLocalAddress = true;
					break;

				default:
					// anything other than IP obtained means game cant use net functionality yet..
					m_ValidLocalAddress = false;
					m_NetCableConnected = false;
					break;
				}
			}
		}
	}

	// force the system to scan for our local network address.. blocks while executing.
	void C_Core::ForceLocalAddressScan(void)
	{
		if (m_StartedUp)
		{
		}
	}
		
	// generate a unique 64 bit identifier for this console..
	bool C_Core::GenerateMachineIdentifier(U64& machineId)
	{
		machineId = 0;
		if (!m_StartedUp)
			return false;

		if (!m_ValidLocalAddress)
			return false;

		// get MAC address..
		CellNetCtlInfo ethernetInfo;
		if (cellNetCtlGetInfo(CELL_NET_CTL_INFO_ETHER_ADDR, &ethernetInfo) != 0)
			return false;

		U64 temp;
		machineId = ethernetInfo.ether_addr.data[0];

		temp = ethernetInfo.ether_addr.data[1];
		machineId |= (temp<<8);
		
		temp = ethernetInfo.ether_addr.data[2];
		machineId |= (temp<<16);

		temp = ethernetInfo.ether_addr.data[3];
		machineId |= (temp<<24);

		temp = ethernetInfo.ether_addr.data[4];
		machineId |= (temp<<32);

		temp = ethernetInfo.ether_addr.data[5];
		machineId |= (temp<<40);

		temp = rand()%(256); // bit of randomness
		machineId |= (temp<<48);

		temp = rand()%(256); // bit of randomness
		machineId |= (temp<<56);

		return true;
	}
}

