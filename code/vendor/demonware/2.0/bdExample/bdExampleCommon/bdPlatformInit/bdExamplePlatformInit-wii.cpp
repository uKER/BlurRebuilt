// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdExample/bdExampleCommon/bdPlatformInit/bdExamplePlatformInit.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>
#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>
#include <bdNet/bdNet.h>
#include <bdCore/bdMemory/bdDefaultMemory.h>

#include <revolution/ncd.h>
#include <revolution/soex.h>
#include <revolution/net.h>

#define BD_LOG_LEVEL "bdExampleCommon/init"

#define MAX_NUM_LOCAL_ADDRS (4)

void* BD_CALL MEM2AllocRedirect(bdULong, 
								bdLong size)
{
    void *const ptr = bdMemory::alignedAllocate2(size, 32);
	return ptr;	
}

void BD_CALL MEM2FreeRedirect(bdULong, 
							  void* ptr, 
							  bdLong)
{
	bdMemory::alignedDeallocate2(ptr);
}

bdPlatformParams::bdPlatformParams()
{
}

bdInt BD_CALL bdExamplePlatformInit::init(bdNetStartParams &params,
										  bdPlatformParams *)
{
	// get IP address
	bdBool ok = true;

    bdInt32 result = 0;
		
    // Wait for NCD to be ready
	// FIXME
    while(NCDGetLinkStatus() == NCD_RESULT_INPROGRESS)
    {
       bdPlatformTiming::sleep(100);
    }

   // Initialize the socket library by specifying the allocator
	SOLibraryConfig libConfig;
	libConfig.alloc = MEM2AllocRedirect;
	libConfig.free = MEM2FreeRedirect;

	if(ok)
	{
		result = SOInit( &libConfig );
		if(result != SO_SUCCESS)
		{
			ok = false;
			BD_ERR(BD_LOG_LEVEL, "SOInit() failed.(%i)", result);
		}
	}
    
	if(ok)
	{
		// Start the network function of the socket library
		// SOStartup waits for DHCP to be resolved
		// TODO: thread SOStartup.
		result = SOStartup();
		if(result != SO_SUCCESS)
		{
			ok = false;
			BD_ERR(BD_LOG_LEVEL, "SOStartup() failed.(%i)", result);
			BD_ERR(BD_LOG_LEVEL, "Network Error Code is %i", -NETGetStartupErrorCode(result));
		}
	}

    if(ok)
	{
		IPAddrEntry addrTable[MAX_NUM_LOCAL_ADDRS];
		bdMemset(addrTable, BD_NULL, sizeof(addrTable));
		bdInt length = sizeof(addrTable);
		
		// get the number of local addresses
		result = SOGetInterfaceOpt(NULL, SO_SOL_CONFIG, SO_CONFIG_IP_ADDR_TABLE, addrTable, &length);
		if(result != SO_SUCCESS)
		{
			ok = false;
			BD_ERR(BD_LOG_LEVEL, "SOGetInterfaceOpt() failed. (%i)", result);
		}
		else
		{
			const bdUInt numAddrs = length / sizeof(IPAddrEntry);

			if(numAddrs > MAX_NUM_LOCAL_ADDRS)
			{
				BD_WARN(BD_LOG_LEVEL, "Using %u of a total of %u local addresses.", MAX_NUM_LOCAL_ADDRS, numAddrs);
			}
			
			for(bdInt i = 0; (i < numAddrs) && (i < MAX_NUM_LOCAL_ADDRS); i++)
			{
				bdUInt32 addr32 = 0;
				BD_ASSERT(sizeof(addr32) == sizeof(addrTable[i].addr), "IPAddrEntry.addr is not 4 bytes.");
				bdMemcpy(&addr32, addrTable[i].addr, sizeof(addr32));
				
				params.m_localAddresses.pushBack(bdInetAddr(addr32));
			}
		}
	}

	return ok ? 0:-1;
}

void BD_CALL bdExamplePlatformInit::quit()
{
    bdInt32 result = 0;
    /* Stop the network function of the socket library */
    result = SOCleanup();
    if (result < SO_SUCCESS)
    {
		BD_WARN(BD_LOG_LEVEL,"SOCleanup() failed.(%d)", result);
    }
    
    /* Release the socket library resource */
    result = SOFinish();
    if (result < SO_SUCCESS)
    {
        BD_WARN(BD_LOG_LEVEL,"SOFinish() failed.(%d)", result);
    }
}

