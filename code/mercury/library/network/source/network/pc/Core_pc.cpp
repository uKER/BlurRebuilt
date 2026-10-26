#include <StdAfx.h>
#include <Network/Core.h>
#include <Iphlpapi.h>
#include <bdNet\bdNet.h>
#include <bdCore\bdSocket\bdCommonAddr.h>
#include <bdLobby\bdLobbyService.h>
#include <Network\Securekey.h>



namespace NetPlay
{
	// static instance..
	C_Core C_Core::s_NetworkCore;
  
  
	// constructor
	C_Core::C_Core() : 
	m_StartedUp(false),	m_ValidLocalAddress(false), m_ScanAddressTimer(0), 
	m_NetCableConnected(false),m_ScanForLocalAddress(false),m_BitFields(0)
	{
#if defined (LIVE_ON_WINDOWS)
		m_SystemLinkPortSet = false;
#endif
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

		// fire upm demonware, default memory managers can be overriden later by the game code if required.
		bdCore::init(true);

		m_pDemonwareLobby = new bdLobbyService;
		if (NULL == m_pDemonwareLobby)
		{
			Shutdown();
			return;
		}

		// start winsock 2.2
		WSADATA wsda;
		if (WSAStartup(MAKEWORD(2,2), &wsda) == 0)
		{
			m_StartedUp = true;

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
				Shutdown();
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
						{
							m_NetCableConnected = true;
							//params.m_localAddresses.pushBack(bdInetAddr(ipStr->IpAddress.String));
						}
						else
						{
							// invalid address..
						}

						ipStr = ipStr->Next;
					}

					adapterInfo = adapterInfo->Next;
					index++;
				}
			}

			if (heapOutBuf)
				delete [] heapOutBuf;
		}
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

		WSACleanup();
		
		bdCore::quit();

		m_StartedUp = false;	
		m_ValidLocalAddress = false;
		m_ScanAddressTimer = 0; 	
		m_NetCableConnected = false; 
		m_ScanForLocalAddress = false;
		m_BitFields = 0;
	}


	// poll once per frame if possible
	void C_Core::Poll(const bool forceNetworkLinkCheck)
	{
	}

	// force the system to scan for our local network address.. blocks while executing.
	void C_Core::ForceLocalAddressScan(void)
	{

	}

	// generate a unique 64 bit identifier for this console..
	bool C_Core::GenerateMachineIdentifier(U64& machineId)
	{
		machineId = 0;
		bdNetImpl *const net = bdNet::getInstance();
		if (net->getStatus() == bdNetImpl::BD_NET_DONE)
		{
			bdCommonAddrRef addr = net->getLocalCommonAddr();
			if (!addr.isNull())
			{
				machineId = addr->getHash();
				return true;
			}
		}
		return false;
	}
}
