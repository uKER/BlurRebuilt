// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdExample/bdExampleCommon/bdPlatformInit/bdExamplePlatformInit.h>
#include <bdNet/bdNet.h>

#if !defined (_WIN32_WINNT)
#	define _WIN32_WINNT 0x400
#endif

# if !defined (_INC_WINDOWS)     // Already include windows.h ?
// Must define strict before including windows.h !
#  if !defined (STRICT)
#   define STRICT 1
#  endif // STRICT
#  if !defined (WIN32_LEAN_AND_MEAN)
#   define WIN32_LEAN_AND_MEAN
#  endif // !defined (WIN32_LEAN_AND_MEAN)
# endif // !defined (_INC_WINDOWS)

// winsock will include windows.h if it hasn't been included all ready

# pragma warning( push )
# pragma warning( disable: 4127) // warning in windows headers
#  include <winsock2.h>
# pragma warning( pop )

# if defined (_MSC_VER)
#  pragma comment(lib, "ws2_32.lib")
# endif // _MSC_VER

#include<Ipexport.h>
#include<Iptypes.h>
#include<Iphlpapi.h>

#define BD_LOG_LEVEL "bdExampleCommon/init"

bdPlatformParams::bdPlatformParams()
{
}

bdInt BD_CALL bdExamplePlatformInit::init(bdNetStartParams &params,
										  bdPlatformParams* )
{
	
	// TODO: On Windows XP and later:  Use the GetAdaptersAddresses function instead of GetAdaptersInfo.
	// from : http://msdn.microsoft.com/en-us/library/aa365917(VS.85).aspx

	// start winsock 2.2
	WSADATA wsda;
	const bdInt val = WSAStartup(MAKEWORD(2,2), &wsda);

	BD_ASSERT(val == 0, "Failed to start winsock!");

	// GetAdaptersInfo treats first argument as a chunk of memory.
	bdByte8* heapOutBuf = BD_NULL;
	bdByte8 stackOutBuf[sizeof(IP_ADAPTER_INFO)];
	bdULong outBufLen = sizeof(IP_ADAPTER_INFO);
	PIP_ADAPTER_INFO adapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(&stackOutBuf);

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the outBufLen variable
	if (GetAdaptersInfo(adapterInfo, &outBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		// need a bigger buffer
		heapOutBuf = bdAllocate<bdByte8>(outBufLen);
		adapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(heapOutBuf);
	}

	//Get info for the adapters
	if( GetAdaptersInfo(adapterInfo, &outBufLen) != ERROR_SUCCESS )
	{
		//Call failed
		BD_ASSERT(false, "Unable to get adapter info!");
	}
	else
	{
		bdInt index = 0;
		while(adapterInfo)
		{
			BD_INFO(BD_LOG_LEVEL, "Adapter %u: %s", index,
													adapterInfo->Description);

			BD_INFO(BD_LOG_LEVEL, "Adapter %u: default gw: %s", index,
																adapterInfo->GatewayList.IpAddress.String);

			PIP_ADDR_STRING ipStr = &adapterInfo->IpAddressList;
			while(ipStr)
			{
				if(bdStrcmp(ipStr->IpAddress.String, "0.0.0.0") != 0)
				{
					BD_INFO(BD_LOG_LEVEL, "Adapter %u: %s", index, ipStr->IpAddress.String);

					params.m_localAddresses.pushBack(bdInetAddr(ipStr->IpAddress.String));
				}
				else
				{
					BD_INFO(BD_LOG_LEVEL, "Adapter %u: skipping (invalid address)", index, ipStr->IpAddress.String);
				}

				ipStr = ipStr->Next;
			}

			adapterInfo = adapterInfo->Next;
			index++;
		}

		BD_INFO(BD_LOG_LEVEL, "using %u valid network adapter(s) out of %u total.", params.m_localAddresses.getSize(), index);
		
	}
	
	if (params.m_localAddresses.isEmpty())
	{
		// Machine is not connected to any kind of network. Add loopback as local.
		BD_INFO(BD_LOG_LEVEL, "No valid network adapters. Using loopback as local address.");
		params.m_localAddresses.pushBack(bdInetAddr("127.0.0.1"));
	}
	
	bdDeallocate<bdByte8>(heapOutBuf);
	return val;
}


void BD_CALL bdExamplePlatformInit::quit()
{
	WSACleanup();
}

